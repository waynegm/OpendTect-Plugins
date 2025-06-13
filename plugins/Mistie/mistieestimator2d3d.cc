#include "mistieestimator2d3d.h"

#include "bufstring.h"
#include "ctxtioobj.h"
#include "emhorizon2d.h"
#include "emhorizon3d.h"
#include "emmanager.h"
#include "emobject.h"
#include "geom2dintersections.h"
#include "iodir.h"
#include "ioman.h"
#include "linerectangleclipper.h"
#include "posinfo2d.h"
#include "seisioobjinfo.h"
#include "seisread.h"
#include "seisselectionimpl.h"
#include "seistrc.h"
#include "seistrctr.h"
#include "seistype.h"
#include "survgeom.h"
#include "survgeom2d.h"
#include "survinfo.h"

#include "unsupported/Eigen/FFT"
#include "trcanalysis.h"

Line3DOverlapFinder::Line3DOverlapFinder(const IOObj* ioobj3D, const ObjectSet<BendPoints>& bpoints)
    : bpoints_(bpoints)
{
    SeisIOObjInfo info(ioobj3D);
    TrcKeyZSampling tkz;
    info.getRanges(tkz);
    setBounds(tkz.hsamp_);
}

Line3DOverlapFinder::Line3DOverlapFinder(const EM::Horizon3D* hor3D, const ObjectSet<BendPoints>& bpoints)
: bpoints_(bpoints)
{
    setBounds(hor3D->range());
}

void Line3DOverlapFinder::setBounds(TrcKeySampling tks )
{
    tks.normalize();
    bounds3d_.setLeft(tks.inlRange().start);
    bounds3d_.setRight(tks.inlRange().stop);
    bounds3d_.setTop(tks.crlRange().stop);
    bounds3d_.setBottom(tks.crlRange().start);
}

od_int64 Line3DOverlapFinder::nrIterations() const
{
    return bpoints_.size();
}

uiString Line3DOverlapFinder::uiMessage() const
{
    return tr("Finding 2D to 3D intersections");
}

uiString Line3DOverlapFinder::uiNrDoneText() const
{
    return tr("Lines done");
}

bool Line3DOverlapFinder::doWork(od_int64 start, od_int64 stop, int tid)
{
    for (int idx=mCast(int,start); idx<=stop && shouldContinue(); idx++, addToNrDone(1)) {
        const BendPoints* bps = bpoints_[idx];
        if (!bps)
            continue;
        int bpssize = bps->idxs_.size();
        mDynamicCastGet(const Survey::Geometry2D*,geom2d,Survey::GM().getGeometry(bps->geomid_));
        if (!geom2d)
            continue;
        const TypeSet<PosInfo::Line2DPos>& linepos = geom2d->data().positions();
        TypeSet<Geom::Point2D<Pos::Ordinate_Type>> polyline;
        for (int bpid=0; bpid<bpssize; bpid++) {
            int posidx = bps->idxs_[bpid];
            if (linepos.validIdx(posidx)) {
                Coord crd = SI().binID2Coord().transformBackNoSnap(linepos[posidx].coord_);
                polyline += Geom::Point2D<Pos::Ordinate_Type>(crd.x, crd.y);
            }
        }
        ObjectSet<TypeSet<Geom::Point2D<Pos::Ordinate_Type>>> trimmedLine;
        clipPolyLine(bounds3d_, polyline, trimmedLine);
        for (int id=0; id<trimmedLine.size(); id++) {
            Coord crd1 = SI().binID2Coord().transform(trimmedLine[id]->first());
            Coord crd2 = SI().binID2Coord().transform(trimmedLine[id]->last());
            int trc1, trc2;
	    float sp;
            geom2d->getPosByCoord(crd1, trc1, sp);
            geom2d->getPosByCoord(crd2, trc2, sp);
            Seis::RangeSelData* range = new Seis::RangeSelData();
	    range->setGeomID(bps->geomid_);
	    range->cubeSampling().hsamp_.setInlRange(Interval<int>(0,0));
            range->cubeSampling().hsamp_.setCrlRange(trc1<trc2 ? Interval<int>(trc1, trc2) : Interval<int>(trc2, trc1));
	    Threads::Locker lckr( lock_ );
	    selranges_ += range;
        }
    }
    return true;
}

bool Line3DOverlapFinder::doFinish(bool success)
{
    return true;
}

MistieEstimatorFromSeismic2D3D::MistieEstimatorFromSeismic2D3D(const IOObj* ioobj3D, const IOObj* ioobj2D,
							       const ManagedObjectSet<Seis::RangeSelData>& selranges,
							       ZGate window, float maxshift,
							       int trcstep, bool allEst)
    : window_(window)
    , maxshift_(maxshift)
    , ioobj2d_(ioobj2D)
    , ioobj3d_(ioobj3D)
    , selranges_(selranges)
    , trcstep_(trcstep)
    , allest_(allEst)
{
    BufferString lineA, lineB;
    int trcA = 0;
    int trcB = 0;
    Coord pos;
    lineA += "3D_";
    lineA += ioobj3d_->uiName().getFullString();
    for (int idx=0; idx<selranges_.size(); idx++) {
        lineB = Survey::GM().getName(selranges_[idx]->geomID());
        trcA = selranges[idx]->crlRange().start;
        trcB = selranges[idx]->crlRange().stop;
	Threads::Locker lckr( lock_ );
        misties_.add(lineA, trcA, lineB, trcB, pos);
    }
}

MistieEstimatorFromSeismic2D3D::~MistieEstimatorFromSeismic2D3D()
{}

od_int64 MistieEstimatorFromSeismic2D3D::nrIterations() const
{
    return misties_.size();
}

uiString MistieEstimatorFromSeismic2D3D::uiMessage() const
{
    return tr("Estimating 2D to 3D misties");
}

uiString MistieEstimatorFromSeismic2D3D::uiNrDoneText() const
{
    return tr("Lines done");
}

bool MistieEstimatorFromSeismic2D3D::doWork( od_int64 start, od_int64 stop, int threadid )
{
    BufferString lineA, lineB;
    int trc1, trc2;
    SeisTrc trcA, trcB;
    for (int idx=mCast(int,start); idx<=stop && shouldContinue(); idx++, addToNrDone(1)) {
        float zdiff = 0.0;
        float phasediff = 0.0;
        std::complex<float> cPhasediff(0.0, 0.0);
        float ampdiff = 0.0;
        float quality = 0.0;

        if (!misties_.get(idx, lineA, trc1, lineB, trc2)) {
            BufferString tmp("MistieEstimator2D3D::doWork - could not get intersection details at index: ");
            tmp += idx;
            ErrMsg(tmp);
            continue;
        }
        StepInterval<int> traces(trc1, trc2, trcstep_);
        TypeSet<int> trcnums;
        if (traces.nrSteps()>=1) {
            for (int it=0; it<traces.nrSteps(); it++)
                trcnums += traces.atIndex(it);
        } else
            trcnums += traces.center();
        int count = 0;
        for (int it=0; it<trcnums.size(); it++) {
            if(get2DTrc(lineB, trcnums[it], trcB)) {
                Coord pos(trcB.info().getValue(SeisTrcInfo::CoordX), trcB.info().getValue(SeisTrcInfo::CoordY));
                IdxPair bid = SI().binID2Coord().transformBack(pos);
                if (get3DTrc(bid.first, bid.second, trcA)) {
                    if (trcA.info().sampling.step > trcB.info().sampling.step) {
                        trcA.info().pick = 0.0;
                        SeisTrc* tmp = trcA.getRelTrc( window_, trcB.info().sampling.step );
                        trcA = *tmp;
                    } else if (trcA.info().sampling.step < trcB.info().sampling.step) {
                        trcB.info().pick = 0.0;
                        SeisTrc* tmp = trcB.getRelTrc( window_, trcA.info().sampling.step );
                        trcB = *tmp;
                    }
                    float zd = 0.0;
                    float pd = 0.0;
                    float ad = 1.0;
                    float q = 0.0;
		    if (allest_ ? computeMistie( trcA, trcB, maxshift_, zd, pd, ad, q ) : computeMistie( trcA, trcB, maxshift_, zd, q )) {
                        count++;
                        zdiff += zd;
                        cPhasediff +=  std::complex<float>(cos(Math::toRadians(pd)),sin(Math::toRadians(pd)));
                        ampdiff += ad;
                        quality += q;
                    }
                } else {
                    BufferString tmp("MistieEstimator2D3D::doWork - could not get 3D trace at Inl/Crl: ");
                    tmp += bid.first; tmp+= "/"; tmp += bid.second;
                    ErrMsg(tmp);
                }
            } else {
                BufferString tmp("MistieEstimator2D3D::doWork - could not get 2D trace data for line: ");
                tmp += lineB; tmp+= " Trace: "; tmp += trcnums[it];
                ErrMsg(tmp);
            }
        }
        if (count) {
            zdiff /= count;
            ampdiff /= count;
            quality /= count;
            phasediff = Math::toDegrees(std::arg(cPhasediff));
        }
        misties_.set(idx, zdiff, phasediff, ampdiff, quality);
    }

    return true;
}

bool MistieEstimatorFromSeismic2D3D::get2DTrc( BufferString line, int trcnr, SeisTrc& trc )
{
    Pos::GeomID geomid = Survey::GM().getGeomID(line);

    Seis::RangeSelData range;
    range.setGeomID(geomid);
    range.setZRange(window_);
    range.cubeSampling().hsamp_.setInlRange(Interval<int>(0,0));
    range.cubeSampling().hsamp_.setCrlRange(Interval<int>(trcnr, trcnr));

    SeisTrcReader rdr(*ioobj2d_);
    rdr.setSelData(range.clone());
    rdr.prepareWork();
    rdr.get(trc);

    return (!trc.isNull());
}

bool MistieEstimatorFromSeismic2D3D::get3DTrc( int inl, int crl, SeisTrc& trc )
{
    Seis::RangeSelData range;
    range.setZRange(window_);
    range.cubeSampling().hsamp_.setInlRange(Interval<int>(inl, inl));
    range.cubeSampling().hsamp_.setCrlRange(Interval<int>(crl, crl));

    SeisTrcReader rdr(*ioobj3d_);
    rdr.setSelData(range.clone());
    rdr.prepareWork();
    rdr.get(trc);

    return (!trc.isNull());
}

bool MistieEstimatorFromSeismic2D3D::doFinish( bool success )
{
    return true;
}

MistieEstimatorFromHorizon2D3D::MistieEstimatorFromHorizon2D3D(MultiID hor3Did, MultiID hor2Did,
							       const ManagedObjectSet<Seis::RangeSelData>& selranges,
							       int trcstep)
: hor3did_(hor3Did)
, hor2did_(hor2Did)
, selranges_(selranges)
, trcstep_(trcstep)
{
    BufferString lineA, lineB;
    int trcA = 0;
    int trcB = 0;
    Coord pos;
    lineA += "3D";
    for (int idx=0; idx<selranges_.size(); idx++) {
	lineB = Survey::GM().getName(selranges_[idx]->geomID());
	trcA = selranges[idx]->crlRange().start;
	trcB = selranges[idx]->crlRange().stop;
	misties_.add(lineA, trcA, lineB, trcB, pos);
    }
}

MistieEstimatorFromHorizon2D3D::~MistieEstimatorFromHorizon2D3D()
{}

od_int64 MistieEstimatorFromHorizon2D3D::nrIterations() const
{
    return misties_.size();
}

uiString MistieEstimatorFromHorizon2D3D::uiMessage() const
{
    return tr("Estimating 2D to 3D misties from Horizon");
}

uiString MistieEstimatorFromHorizon2D3D::uiNrDoneText() const
{
    return tr("Lines done");
}

bool MistieEstimatorFromHorizon2D3D::doPrepare(int nrthreads)
{
    if (hor2did_.isUdf() || hor3did_.isUdf())
	return false;
    EM::EMObject* obj2d = EM::EMM().loadIfNotFullyLoaded(hor2did_);
    if (!obj2d) {
	ErrMsg("MistieEstimatorFromHorizon2D3D::doPrepare - loading 2D input horizon failed");
	return false;
    }
    EM::EMObject* obj3d = EM::EMM().loadIfNotFullyLoaded(hor3did_);
    if (!obj3d) {
	ErrMsg("MistieEstimatorFromHorizon2D3D::doPrepare - loading 3D input horizon failed");
	return false;
    }
    obj2d->ref();
    obj3d->ref();
    mDynamicCast(EM::Horizon3D*,hor3d_,obj3d);
    mDynamicCast(EM::Horizon2D*,hor2d_,obj2d);
    if (!hor3d_ || !hor2d_) {
	ErrMsg("MistieEstimatorFromHorizon2D3D::doPrepare - casting horizons failed");
	obj2d->unRef();
	obj3d->unRef();
	return false;
    }

    return true;
}

bool MistieEstimatorFromHorizon2D3D::doWork( od_int64 start, od_int64 stop, int threadid )
{
    BufferString lineA, lineB;
    int trc1, trc2;
    for (int idx=mCast(int,start); idx<=stop && shouldContinue(); idx++, addToNrDone(1)) {
	float zdiff = 0.0;
	float phasediff = 0.0;
	float ampdiff = 1.0;
	float quality = 1.0;

	if (!misties_.get(idx, lineA, trc1, lineB, trc2)) {
	    BufferString tmp("MistieEstimatorFromHorizon2D3D::doWork - could not get intersection details at index: ");
	    tmp += idx;
	    ErrMsg(tmp);
	    continue;
	}
	StepInterval<int> traces(trc1, trc2, trcstep_);
	TypeSet<int> trcnums;
	if (traces.nrSteps()>=1) {
	    for (int it=0; it<traces.nrSteps(); it++)
		trcnums += traces.atIndex(it);
	} else
	    trcnums += traces.center();
	int count = 0;
	for (int it=0; it<trcnums.size(); it++) {
	    const TrcKey tk2d(Survey::GM().getGeomID(lineB), trcnums[it]);
	    const Coord3 pos2d = hor2d_->getCoord(tk2d);
	    float z3d = hor3d_->getZ(SI().transform(pos2d.coord()));
	    if (!mIsUdf(z3d) && !mIsUdf(pos2d.z))
	    {
		zdiff += (pos2d.z - z3d) * SI().showZ2UserFactor();
		count++;
	    }
	}
	if (count)
	    zdiff /= count;
	Threads::Locker lckr( lock_ );
	misties_.set(idx, zdiff, phasediff, ampdiff, quality);
    }

    return true;
}

bool MistieEstimatorFromHorizon2D3D::doFinish( bool success )
{
    if (hor2d_)
	hor2d_->unRef();
    if (hor3d_)
	hor3d_->unRef();

    return true;
}



