#include "avopolar.h"

#include "attribdataholder.h"
#include "attribdesc.h"
#include "attribdescset.h"
#include "attribfactory.h"
#include "attribparam.h"
#include "attribparamgroup.h"
#include "survinfo.h"
#include <math.h>
#include "arrayndimpl.h"
#include "errmsg.h"
#include "bufstring.h"

#include "arrayfire.h"
#include "Eigen/Core"

#include "windowedops.h"
#include "windowedops_eigen.h"

namespace Attrib {

mAttrDefCreateInstance(AVOPolar)

const char* AVOPolar::MethodNames[] =
{
        "Normal",
        "Eigen",
        "ArrayFire",
        0
};

void AVOPolar::initClass()
{
    mAttrStartInitClass
    ZGateParam* gateBG = new ZGateParam( gateBGStr() );
    gateBG->setLimits( Interval<float>(-1000,1000) );
    Interval<float> defgate( -100, 100 );
    gateBG->setDefaultValue( defgate );
    desc->addParam( gateBG );
 
    BinIDParam* stepout = new BinIDParam( stepoutStr() );
    stepout->setDefaultValue( BinID(3,3) );
    stepout->setLimits(Interval<int>(1,50),Interval<int>(1,50)),
    desc->addParam( stepout );
    
    EnumParam* method = new EnumParam( methodStr() );
    for (int idx=0; MethodNames[idx]; idx++)
        method->addEnum(MethodNames[idx]);
    method->setDefaultValue(0);
    desc->addParam( method );

    BoolParam* doparallel = new BoolParam( parallelStr(), false, false );
    desc->addParam( doparallel );
    
    desc->addInput( InputSpec("Intercept",true) );
    desc->addInput( InputSpec("Gradient",true) );
    
    desc->setNrOutputs( Seis::UnknowData, 1 );
    desc->setLocality(Desc::MultiTrace);
    
    mAttrEndInitClass
}

void AVOPolar::updateDefaults( Desc& desc )
{
    ValParam* paramgate = desc.getValParam( gateBGStr() );
    mDynamicCastGet(ZGateParam*,zgate,paramgate)
    float roundedzstep = SI().zStep()*SI().zDomain().userFactor();
    if ( roundedzstep > 0 )
        roundedzstep = Math::Floor( roundedzstep );
    zgate->setDefaultValue( Interval<float>(-roundedzstep*25, roundedzstep*25) );
}

AVOPolar::AVOPolar( Desc& desc )
: Provider( desc )
{
    if ( !isOK() ) return;
    
    mGetFloatInterval( gateBG_, gateBGStr() );
    gateBG_.scale( 1.f/zFactor() );
    mGetBinID( stepout_, stepoutStr() );
    
    mGetEnum( method_, methodStr());
    mGetBool( doparallel_, parallelStr() );
    
    if (method_== ArrayFire)
       ErrMsg(af::infoString()); 
    
    intercept_.allowNull( true );
    gradient_.allowNull( true );
    
    getTrcPos();
}

bool AVOPolar::getTrcPos()
{
    trcpos_.erase();
    BinID bid;
    int trcidx = 0;
    centertrcidx_ = 0;
    for ( bid.inl()=-stepout_.inl(); bid.inl()<=stepout_.inl(); bid.inl()++ )
    {
        for ( bid.crl()=-stepout_.crl(); bid.crl()<=stepout_.crl(); bid.crl()++ )
        {
            if ( !bid.inl() && !bid.crl() )
                centertrcidx_ = trcidx;
            trcpos_ += bid;
            trcidx++;
        }
    }
    
    return true;
}

void AVOPolar::prepPriorToBoundsCalc()
{
    sampgateBG_ = Interval<int>( mNINT32(gateBG_.start/refstep_), mNINT32(gateBG_.stop/refstep_) );
    Provider::prepPriorToBoundsCalc();
}

const BinID* AVOPolar::desStepout( int inp, int outp ) const
{
    return inp ? 0: &stepout_;
}

const Interval<int>* AVOPolar::desZSampMargin( int inp, int outp ) const
{
    return inp ? 0: &sampgateBG_;
}

bool AVOPolar::getInputData( const BinID& relpos, int zintv )
{
    intercept_.erase();
    gradient_.erase();
    
    while ( intercept_.size() < trcpos_.size() ) {
        intercept_ += 0;
        gradient_ += 0;
    }
    
    const BinID bidstep = inputs_[0]->getStepoutStep();
    if ( bidstep != inputs_[1]->getStepoutStep() ) {
        ErrMsg("getInputData - mismatch in stepout step between input attributes.");
        return false;
    }

    for ( int idx=0; idx<trcpos_.size(); idx++ ) {
        BinID pos = relpos + trcpos_[idx] * bidstep;
        const DataHolder* data1 = inputs_[0]->getData( pos, zintv );
        const DataHolder* data2 = inputs_[1]->getData( pos, zintv );
        if ( !data1 ) {
            pos = relpos + trcpos_[centertrcidx_]*bidstep;
            data1 = inputs_[0]->getData( pos, zintv );
            if ( !data1 ) return false;
        }
        intercept_.replace( idx, data1 );

        if ( !data2 ) {
            pos = relpos + trcpos_[centertrcidx_]*bidstep;
            data2 = inputs_[1]->getData( pos, zintv );
            if ( !data2 ) return false;
        }
        gradient_.replace( idx, data2 );
    }
    intercept_idx_ = getDataIndex(0);
    gradient_idx_ = getDataIndex(1);
    
    return true;
}

bool AVOPolar::computeData( const DataHolder& output, const BinID& relpos, int z0, int nrsamples, int threadid) const
{
    if (method_ == Normal)
        return computeDataNormal(output, relpos, z0, nrsamples, threadid);
    else if (method_ == Eigen)
        return computeDataEigen(output, relpos, z0, nrsamples, threadid);
    else if (method_ == ArrayFire)
        return computeDataArrayFire(output, relpos, z0, nrsamples, threadid);
    
    return false;
}

bool AVOPolar::computeDataNormal( const DataHolder& output, const BinID& relpos, int z0, int nrsamples, int threadid) const
{
    const int sz = sampgateBG_.width() + nrsamples;
    const int ntraces = trcpos_.size();
    Array2DImpl<float> intercept(ntraces, sz);
    for (int trcidx=0; trcidx<ntraces; trcidx++) {
        const DataHolder* data = intercept_[trcidx];
        for (int idx=0; idx<sz; idx++) {
            float val = getInputValue(*data, intercept_idx_, sampgateBG_.start+idx, z0);
            intercept.set(trcidx, idx, mIsUdf(val)?0.0f:val);
        }
    }
    
    Array2DImpl<float> gradient(ntraces, sz);
    for (int trcidx=0; trcidx<ntraces; trcidx++) {
        const DataHolder* data = gradient_[trcidx];
        for (int idx=0; idx<sz; idx++) {
            float val = getInputValue(*data, gradient_idx_, sampgateBG_.start+idx, z0);
            gradient.set(trcidx, idx, mIsUdf(val)?0.0f:val);
        }
    }
    Array1DImpl<float> result(0);
    computeBackgroundAngle( intercept, gradient, result );
    for (int idx=0; idx<nrsamples; idx++)
        setOutputValue(output, 0, idx, z0, result[idx-sampgateBG_.start]);
    
    return true;
}

void AVOPolar::computeBackgroundAngle( const Array2DImpl<float>& A, 
                                         const Array2DImpl<float>& B,
                                         Array1DImpl<float>& result ) const
{
    int ntraces = A.info().getSize(0);
    int sz = A.info().getSize(1);
    result.setSize(sz);

    Array1DImpl<double> A2(sz);
    Array1DImpl<double> B2(sz);
    Array1DImpl<double> AB(sz);
    
    for (int idx=0; idx<sz; idx++) {
        double A2v = 0.0;
        double B2v = 0.0;
        double ABv = 0.0;
        for (int trcidx=0; trcidx<ntraces; trcidx++) {
            A2v += (double) A.get(trcidx, idx) * (double) A.get(trcidx, idx);
            ABv += A.get(trcidx, idx) * B.get(trcidx, idx);
            B2v += B.get(trcidx, idx) * B.get(trcidx, idx);
        }
        A2.set(idx, A2v);
        B2.set(idx, B2v);
        AB.set(idx, ABv);
    }

    Array1DImpl<double> A2win(sz);
    windowedOps::sum( A2, sampgateBG_.width(), A2win );
    Array1DImpl<double> B2win(sz);
    windowedOps::sum( B2, sampgateBG_.width(), B2win );
    Array1DImpl<double> ABwin(sz);
    windowedOps::sum( AB, sampgateBG_.width(), ABwin );
    
    for (int idx=0; idx<sz; idx++) {
        double ABv = ABwin.get(idx);
        double A2mB2 = A2win.get(idx) - B2win.get(idx);
        double d = sqrt(4.0*ABv*ABv + A2mB2*A2mB2);
        result.set(idx, atan2(2.0*ABv, A2mB2+d)/M_PI*180.0);
    }
}    

bool AVOPolar::computeDataEigen( const DataHolder& output, const BinID& relpos, int z0, int nrsamples, int threadid) const
{
    const int sz = sampgateBG_.width() + nrsamples;
    const int ntraces = trcpos_.size();
    Eigen::ArrayXXd A(sz, ntraces);
    for (int trcidx=0; trcidx<ntraces; trcidx++) {
        const DataHolder* data = intercept_[trcidx];
        for (int idx=0; idx<sz; idx++) {
            float val = getInputValue(*data, intercept_idx_, sampgateBG_.start+idx, z0);
            A(idx, trcidx) = mIsUdf(val)?0.0f:val;
        }
    }
    
    Eigen::ArrayXXd B(sz, ntraces);
    for (int trcidx=0; trcidx<ntraces; trcidx++) {
        const DataHolder* data = gradient_[trcidx];
        for (int idx=0; idx<sz; idx++) {
            float val = getInputValue(*data, gradient_idx_, sampgateBG_.start+idx, z0);
            B(idx, trcidx) = mIsUdf(val)?0.0f:val;
        }
    }
    
    Eigen::ArrayXd A2 = A.square().rowwise().sum();
    Eigen::ArrayXd B2 = B.square().rowwise().sum();
    Eigen::ArrayXd AB = (A*B).rowwise().sum();
    
    Eigen::ArrayXd A2win(sz);
    windowedOpsEigen::sum( A2, sampgateBG_.width(), A2win );
    Eigen::ArrayXd B2win(sz);
    windowedOpsEigen::sum( B2, sampgateBG_.width(), B2win );
    Eigen::ArrayXd ABwin(sz);
    windowedOpsEigen::sum( AB, sampgateBG_.width(), ABwin );
    
    Eigen::ArrayXd A2mB2 = A2win - B2win;
    
    Eigen::ArrayXd result = (2.0*ABwin/(A2mB2 + (4.0*ABwin.square() + A2mB2.square()).sqrt())).atan()/M_PI*180.0;
    for (int idx=0; idx<nrsamples; idx++)
        setOutputValue(output, 0, idx, z0, result(idx-sampgateBG_.start));
    
    return true;
}

bool AVOPolar::computeDataArrayFire( const DataHolder& output, const BinID& relpos, int z0, int nrsamples, int threadid) const
{
    const int sz = sampgateBG_.width() + nrsamples;
    const int ntraces = trcpos_.size();
    
    double* buffer = new double[ sz * ntraces];
    for (int trcidx=0; trcidx<ntraces; trcidx++) {
        const DataHolder* data = intercept_[trcidx];
        for (int idx=0; idx<sz; idx++) {
            float val = getInputValue(*data, intercept_idx_, sampgateBG_.start+idx, z0);
            buffer[idx+trcidx*sz] = mIsUdf(val)?0.0:val;
        }
    }
    af::array A( sz, ntraces, buffer );
    
    for (int trcidx=0; trcidx<ntraces; trcidx++) {
        const DataHolder* data = gradient_[trcidx];
        for (int idx=0; idx<sz; idx++) {
            float val = getInputValue(*data, gradient_idx_, sampgateBG_.start+idx, z0);
            buffer[idx+trcidx*sz] = mIsUdf(val)?0.0:val;
        }
    }
    af::array B( sz,ntraces, buffer );
    delete [] buffer;
    
    af::array A2 = af::sum(A*A,1);
    af::array B2 = af::sum(B*B,1);
    af::array AB = af::sum(A*B,1);
    
    af::array kernel = af::constant( (double) 1.0, sampgateBG_.width(),1);
    af::array A2win = af::convolve1(A2, kernel);
    af::array B2win = af::convolve1(B2, kernel);
    af::array ABwin = af::convolve1(AB, kernel);
    af::array A2mB2 = A2win - B2win;

    af::array result = af::atan2(2.0*ABwin, A2mB2 + af::sqrt(4.0*ABwin*ABwin + A2mB2*A2mB2))/M_PI*180.0;
    
    double* resbuf = result.host<double>();
    for (int idx=0; idx<nrsamples; idx++)
        setOutputValue(output, 0, idx, z0, resbuf[idx-sampgateBG_.start]);
        
    af::freeHost(resbuf);
    return true;
}



}

        
        


    
