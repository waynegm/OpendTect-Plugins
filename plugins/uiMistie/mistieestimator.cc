#include "mistieestimator.h"

#include "geom2dintersections.h"
#include "survgeom.h"
#include "survgeom2d.h"
#include "seisread.h"
#include "seisioobjinfo.h"
#include "seistrc.h"
#include "seistype.h"
#include "bufstring.h"
#include "seisselectionimpl.h"
#include "ctxtioobj.h"
#include "ioman.h"
#include "seistrctr.h"
#include "iodir.h"
#include "survinfo.h"

#include "unsupported/Eigen/FFT"

MistieEstimator::MistieEstimator(const IOObj* ioobj, Line2DInterSectionSet& intset, ZGate window, float maxshift)
    : window_(window)
    , maxshift_(maxshift)
    , ioobj_(ioobj)
{
    BufferString lineA, lineB;
    int trcA, trcB, spnr;
    Coord pos;
    for (int idx=0; idx<intset.size(); idx++) {
        Pos::GeomID geomidA = intset[idx]->geomID();
        mDynamicCastGet( const Survey::Geometry2D*, geom2d, Survey::GM().getGeometry(geomidA) );
        lineA = Survey::GM().getName(geomidA);
        for (int idp=0; idp<intset[idx]->size(); idp++) {
            Line2DInterSection::Point pint = intset[idx]->getPoint(idp);
            lineB = Survey::GM().getName(pint.line);
            trcA = pint.mytrcnr;
            trcB = pint.linetrcnr;
            geom2d->getPosByTrcNr(trcA, pos, spnr);
            misties_.add(lineA, trcA, lineB, trcB, pos);
        }
    }
}

MistieEstimator::~MistieEstimator()
{}

od_int64 MistieEstimator::nrIterations() const
{
    return misties_.size();
}

uiString MistieEstimator::uiMessage() const
{
    return tr("Estimating misties");
}

uiString MistieEstimator::uiNrDoneText() const
{
    return tr("Intersections done");
}

bool MistieEstimator::doWork( od_int64 start, od_int64 stop, int threadid )
{
    BufferString lineA, lineB;
    int trcnrA, trcnrB;
    SeisTrc trcA, trcB;
    Eigen::FFT<double> fft;
    
    for (int idx=mCast(int,start); idx<=stop && shouldContinue(); idx++, addToNrDone(1)) {
        float zdiff = 0.0;
        float phasediff = 0.0;
        float ampdiff = 1.0;
        float quality = 1.0;
        
        if (!misties_.get(idx, lineA, trcnrA, lineB, trcnrB)) {
            BufferString tmp("MistieEstimator::doWork - could not get intersection details at index: ");
            tmp += idx;
            ErrMsg(tmp);
            continue;
        }
        if (get2DTrc(lineA, trcnrA, trcA) && get2DTrc(lineB, trcnrB, trcB)) {
            if (trcA.info().sampling.step > trcB.info().sampling.step) {
                trcA.info().pick = 0.0;
                SeisTrc* tmp = trcA.getRelTrc( window_, trcB.info().sampling.step );
                trcA = *tmp;
            } else if (trcA.info().sampling.step < trcB.info().sampling.step) {
                trcB.info().pick = 0.0;
                SeisTrc* tmp = trcB.getRelTrc( window_, trcA.info().sampling.step );
                trcB = *tmp;
            }
            int n = trcA.size();
            int nfft = 2*n;
            float dt = trcA.info().sampling.step;
            
            Eigen::VectorXd A, B;
            A.setZero(nfft);
            B.setZero(nfft);
            for (int idt=0; idt<n; idt++) {
                float valA = mIsUdf(trcA.get(idt,0)) ? 0.0 : trcA.get(idt,0);
                float valB = mIsUdf(trcB.get(idt,0)) ? 0.0 : trcB.get(idt,0);
                A(idt) = valA;
                B(idt) = valB;
            }
            double sumAA = A.head(n).norm();
            double sumBB = B.head(n).norm();
            ampdiff = mIsZero(sumAA, mDefEps) ? 1.0 : sumBB/sumAA;
            
            Eigen::VectorXcd Af, Bf, CCf, AS;
            Eigen::VectorXd CC, H;
            H.setConstant(nfft, 2.0);
            H(0) = 1.0;
            H(n) = 1.0;
            H.segment(n+1, n-1) *= 0.0;
            fft.fwd(Af,A);
            fft.fwd(Bf,B);
            CCf = (Bf.array() * Eigen::conj(Af.array()) * H.array()).matrix();
            fft.inv(AS,CCf);
            CC = Eigen::abs2(AS.array()).sqrt().matrix();
            Eigen::MatrixXd::Index maxIndex;
            int maxlag = maxshift_/dt;
            CC.segment(maxlag,nfft-2*maxlag) *= 0.0;
            CC.maxCoeff(&maxIndex);
            int il = maxIndex-1;
            int ir = maxIndex+1;
            if (maxIndex==0)
                il = nfft-1;
            else if (maxIndex==nfft-1)
                ir = 0;
            float cp = (CC(il)-CC(ir))/(2.0*CC(il)-4.0*CC(maxIndex)+2.0*CC(ir));
            quality = (CC(maxIndex) - 0.25 * (CC(il) - CC(ir)) * cp)/(sumAA*sumBB);
            quality = quality > 1.0 ? 1.0 : quality;
            zdiff = (maxIndex < n ? float(maxIndex)+cp : float(maxIndex-nfft)+cp)*dt*SI().showZ2UserFactor();;
            float p0 = std::arg(AS(maxIndex));
            float p1 = cp>=0.0 ? std::arg(AS(ir)) : std::arg(AS(il));
            p1 = p1-p0>M_PI ? p1-M_2PI : p1-p0<-M_PI ? p1+M_2PI : p1;
            phasediff = Math::toDegrees(cp>=0.0 ? (p1-p0)*cp+p0 :(p0-p1)*cp+p0) ;
            BufferString tmp;
            tmp += lineA; tmp += " "; tmp += lineB; tmp += "\n";
            tmp += il; tmp += " "; tmp += maxIndex; tmp += " "; tmp += ir;  tmp += "\n";
            tmp += CC(il)/(sumAA*sumBB); tmp += " "; tmp += CC(maxIndex)/(sumAA*sumBB); tmp += " "; tmp += CC(ir)/(sumAA*sumBB);  tmp += "\n";
            tmp += cp; tmp += " zdiff: "; tmp += zdiff; tmp += " quality: "; tmp += quality;  tmp += "\n";
            tmp += std::arg(AS(il)); tmp += " "; tmp += p0; tmp += " "; tmp += std::arg(AS(ir)); tmp += "\n";
            tmp += "p1: "; tmp += p1; tmp += " phasediff: "; tmp += phasediff; tmp += "\n";
            ErrMsg(tmp);
        } else {
            BufferString tmp("MistieEstimator::doWork - could not get trace data for: ");
            tmp += lineA; tmp+= " "; tmp += lineB;
            ErrMsg(tmp);
        }
        misties_.set(idx, zdiff, phasediff, ampdiff, quality);
    }
    return true;
}

bool MistieEstimator::get2DTrc( BufferString line, int trcnr, SeisTrc& trc )
{
    Pos::GeomID geomid = Survey::GM().getGeomID(line);
    
    Seis::RangeSelData range;
    range.setZRange(window_);
    range.cubeSampling().hsamp_.setInlRange(Interval<int>(0,0));
    range.cubeSampling().hsamp_.setCrlRange(Interval<int>(trcnr, trcnr));
    range.setGeomID(geomid);
    
    SeisTrcReader rdr(ioobj_);
    rdr.setSelData(range.clone());
    rdr.prepareWork();
    rdr.get(trc);

    return (!trc.isNull());
}

void MistieEstimator::calcZandPhase( SeisTrc& trcA, SeisTrc& trcB, float& zdiff, float& phasediff, float& quality )
{
    
}

float MistieEstimator::calcAmpScalar( SeisTrc& trcA, SeisTrc& trcB )
{
    double sumsqA = 0.0;
    double sumsqB = 0.0;
    for (int idx=0; idx<trcA.size(); idx++) {
        float valA = mIsUdf(trcA.get(idx,0)) ? 0.0 : trcA.get(idx,0);
        float valB = mIsUdf(trcB.get(idx,0)) ? 0.0 : trcB.get(idx,0);
        sumsqA += valA * valA;
        sumsqB += valB * valB;
    }
    
    double scale = mIsZero(sumsqA, mDefEps) ? 1.0 : sqrt(sumsqB/sumsqA);
    return (float) scale;
}

bool MistieEstimator::doFinish( bool success )
{
    return true;
}

