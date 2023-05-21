#include "mistieapplytohorizon.h"

#include "arraynd.h"
#include "emmanager.h"
#include "emobject.h"
#include "emhorizon2d.h"
#include "emhorizon3d.h"
#include "executor.h"
#include "randcolor.h"
#include "survgeom2d.h"
#include "survinfo.h"
#include "trckeyzsampling.h"

MistieApplyToHorizon2D::MistieApplyToHorizon2D(const MultiID oldhor2did,
					       const MultiID newhor2did,
					       const TypeSet<Pos::GeomID>& geomids,
					       const BufferString mistiefile
  					    )
: mistiefile_(mistiefile)
, oldhor2did_(oldhor2did)
, newhor2did_(newhor2did)
, geomids_(geomids)
{
    if (!corrections_.read( mistiefile_ )) {
	ErrMsg("MistieApplyToHorizon2D::MistieApplyToHorizon2D - error reading mistie correction file");
    }
}

MistieApplyToHorizon2D::~MistieApplyToHorizon2D()
{}

od_int64 MistieApplyToHorizon2D::nrIterations() const
{
    return geomids_.size();
}

uiString MistieApplyToHorizon2D::uiMessage() const
{
    return tr("Applying mistie corrections");
}

uiString MistieApplyToHorizon2D::uiNrDoneText() const
{
    return tr("Lines done");
}

bool MistieApplyToHorizon2D::doPrepare(int nrthreads)
{
    if (oldhor2did_.isUdf() || newhor2did_.isUdf())
	return false;
    EM::EMObject* oldobj = EM::EMM().loadIfNotFullyLoaded(oldhor2did_);
    if (!oldobj) {
	ErrMsg("MistieApplyToHorizon2D::doPrepare - loading 2D input horizon failed");
	return false;
    }
    EM::EMObject* newobj = EM::EMM().createTempObject(EM::Horizon2D::typeStr());
    if (!newobj) {
	ErrMsg("MistieApplyToHorizon2D::doPrepare - creating 2D output horizon failed");
	return false;
    }
    oldobj->ref();
    newobj->ref();
    mDynamicCast(EM::Horizon2D*,inphor_,oldobj);
    mDynamicCast(EM::Horizon2D*,outhor_,newobj);
    if (!inphor_ || !outhor_) {
	ErrMsg("MistieApplyToHorizon2D - casting 2D horizon failed");
	oldobj->unRef();
	newobj->unRef();
	return false;
    }
    outhor_->setFullyLoaded(true);

    return true;
}

bool MistieApplyToHorizon2D::doWork( od_int64 start, od_int64 stop, int threadid )
{
    if (!geomids_.validIdx(start) || !geomids_.validIdx(stop))
	return false;

    for (int idx=mCast(int,start); idx<=stop && shouldContinue(); idx++, addToNrDone(1)) {
	const StepInterval<int> trcrg = inphor_->geometry().colRange( geomids_[idx] );
	mDynamicCastGet(const Survey::Geometry2D*,survgeom2d,Survey::GM().getGeometry(geomids_[idx]))
	if (!survgeom2d || trcrg.isUdf() || !trcrg.step)
	{
	    BufferString tmp("MistieApplyToHorizon2D::doWork - geometry error for: ");
	    tmp += geomids_[idx].toString();
	    ErrMsg(tmp);
	    continue;
	}
	{
	    Threads::Locker lckr( lock_ );
	    if (!outhor_->geometry().addLine(geomids_[idx]))
	    {
		BufferString tmp("MistieApplyToHorizon2D::doWork - error adding line: ");
		tmp += survgeom2d->getName();
		ErrMsg(tmp);
		continue;
	    }
	}

	BufferString line = survgeom2d->getName();
	float zcor = corrections_.getZCor(corrections_.getIndex(line))/SI().showZ2UserFactor();
	TrcKey tk( geomids_[idx], -1 );
	for ( int trcnr=trcrg.start; trcnr<=trcrg.stop; trcnr+=trcrg.step ) {
	    tk.setTrcNr(trcnr);
	    float z = inphor_->getZ(tk);
	    if (!mIsUdf(z))
		z += zcor;
	    Threads::Locker lckr( lock_ );
	    outhor_->setZ(tk, z, false);
	}
    }

    return true;
}

bool MistieApplyToHorizon2D::doFinish(bool success)
{
    bool res = false;
    if (inphor_)
	inphor_->unRef();
    if (outhor_)
    {
	outhor_->setPreferredColor(OD::getRandomColor());
	outhor_->setMultiID(newhor2did_);
	PtrMan<Executor> saver = outhor_->saver();
	if (saver)
	    res = saver->execute();
	outhor_->unRef();
    }
    return res;
}


MistieApplyToHorizon3D::MistieApplyToHorizon3D(const MultiID oldhor3did,
					       const MultiID newhor3did,
					       const BufferString mistiefile
)
: mistiefile_(mistiefile)
, oldhor3did_(oldhor3did)
, newhor3did_(newhor3did)
{
    if (oldhor3did_.isUdf() || newhor3did_.isUdf())
	return;
    EM::EMObject* oldobj = EM::EMM().loadIfNotFullyLoaded(oldhor3did_);
    if (!oldobj) {
	ErrMsg("MistieApplyToHorizon3D - loading 3D input horizon failed");
	return;
    }
    EM::EMObject* newobj = EM::EMM().createTempObject(EM::Horizon3D::typeStr());
    if (!newobj) {
	ErrMsg("MistieApplyToHorizon3D - creating 3D output horizon failed");
	return;
    }
    oldobj->ref();
    newobj->ref();
    mDynamicCast(EM::Horizon3D*,inphor_,oldobj);
    mDynamicCast(EM::Horizon3D*,outhor_,newobj);
    if (!inphor_ || !outhor_) {
	ErrMsg("MistieApplyToHorizon3D - casting 3D horizon failed");
	oldobj->unRef();
	newobj->unRef();
	return;
    }

    arr2d_ = inphor_->createArray2D();
    nlocs_ = arr2d_->totalSize();
    BufferString tmp;
    outhor_->setFullyLoaded(true);
}

MistieApplyToHorizon3D::~MistieApplyToHorizon3D()
{
    deleteAndZeroPtr( arr2d_ );
}

od_int64 MistieApplyToHorizon3D::nrIterations() const
{
    return nlocs_;
}

uiString MistieApplyToHorizon3D::uiMessage() const
{
    return tr("Applying mistie corrections");
}

uiString MistieApplyToHorizon3D::uiNrDoneText() const
{
    return tr("Locations done");
}

bool MistieApplyToHorizon3D::doPrepare(int nrthreads)
{
    if (!corrections_.read( mistiefile_ )) {
	ErrMsg("MistieApplyToHorizon3D - error reading mistie correction file");
	return false;
    }
    if (!inphor_ || !outhor_)
	return false;

    return true;
}

bool MistieApplyToHorizon3D::doWork( od_int64 start, od_int64 stop, int threadid )
{
    int zidx = corrections_.getIndex("3D");
    if (zidx==-1)
	return false;

    float zcor = corrections_.getZCor(zidx)/SI().showZ2UserFactor();
    int pos[2];

    for (od_int64 idx=start; idx<=stop && shouldContinue(); idx++, addToNrDone(1)) {
	if (arr2d_->info().getArrayPos(idx, pos))
	{
	    float zval = arr2d_->get(pos[0], pos[1]);
	    if (!mIsUdf(zval))
		arr2d_->set(pos[0], pos[1], zval+zcor);
	}
    }

    return true;
}

bool MistieApplyToHorizon3D::doFinish(bool success)
{
    if (outhor_ && inphor_)
    {
	outhor_->setPreferredColor(OD::getRandomColor());
	outhor_->setMultiID(newhor3did_);
	BinID start( inphor_->geometry().rowRange().start,
		     inphor_->geometry().colRange().start );
	BinID step( inphor_->geometry().step().row(),
		    inphor_->geometry().step().col() );
	outhor_->setArray2D( arr2d_, start, step, true );
	arr2d_ = nullptr;

	PtrMan<Executor> saver = outhor_->saver();
	if (saver)
	    return saver->execute();
	outhor_->unRef();
    }
    if (inphor_)
	inphor_->unRef();

    return false;
}
