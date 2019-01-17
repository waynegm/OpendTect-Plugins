#include "avopolarattrib.h"

#include "attribdataholder.h"
#include "attribdesc.h"
#include "attribdescset.h"
#include "attribfactory.h"
#include "attribparam.h"
#include "attribparamgroup.h"
#include "survinfo.h"
#include <math.h>
#include "errmsg.h"
#include "bufstring.h"


#include "windowedops_eigen.h"

namespace Attrib {

mAttrDefCreateInstance(AVOPolarAttrib)

const char* AVOPolarAttrib::OutputNames[] =
{
        "Background Polarization Angle",
        "Event Polarization Angle",
        "Polarization Angle Difference",
        "Strength",
        "Polarization Product",
        "Quality",
        0
};

void AVOPolarAttrib::initClass()
{
    mAttrStartInitClass
    ZGateParam* gateBG = new ZGateParam( gateBGStr() );
    gateBG->setLimits( Interval<float>(-1000,1000) );
    Interval<float> defgateBG( -200, 200 );
    gateBG->setDefaultValue( defgateBG );
    desc->addParam( gateBG );
 
    BinIDParam* stepout = new BinIDParam( stepoutStr() );
    stepout->setDefaultValue( BinID(1,1) );
    stepout->setLimits(Interval<int>(1,10),Interval<int>(1,10)),
    desc->addParam( stepout );
    
    ZGateParam* gate = new ZGateParam( gateStr() );
    gate->setLimits( Interval<float>(-100,100) );
    Interval<float> defgate( -20, 20 );
    gate->setDefaultValue( defgate );
    desc->addParam( gate );
    
    desc->addInput( InputSpec("Intercept",true) );
    desc->addInput( InputSpec("Gradient",true) );
    
    desc->setNrOutputs( Seis::UnknowData, 6 );
    desc->setLocality(Desc::MultiTrace);
    
    mAttrEndInitClass
}

void AVOPolarAttrib::updateDefaults( Desc& desc )
{
    ValParam* paramgateBG = desc.getValParam( gateBGStr() );
    mDynamicCastGet(ZGateParam*,zgateBG,paramgateBG)
    float roundedzstep = SI().zStep()*SI().zDomain().userFactor();
    if ( roundedzstep > 0 )
        roundedzstep = Math::Floor( roundedzstep );
    zgateBG->setDefaultValue( Interval<float>(-roundedzstep*50, roundedzstep*50) );
    
    ValParam* paramgate = desc.getValParam( gateStr() );
    mDynamicCastGet(ZGateParam*,zgate,paramgate)
    zgate->setDefaultValue( Interval<float>(-roundedzstep*5, roundedzstep*5) );
}

AVOPolarAttrib::AVOPolarAttrib( Desc& desc )
: Provider( desc )
{
    if ( !isOK() ) return;
    
    mGetFloatInterval( gateBG_, gateBGStr() );
    gateBG_.scale( 1.f/zFactor() );
    mGetBinID( stepout_, stepoutStr() );
    
    mGetFloatInterval( gate_, gateStr() );
    gate_.scale( 1.f/zFactor() );
    
    intercept_.allowNull( true );
    gradient_.allowNull( true );
    
    getTrcPos();
}

bool AVOPolarAttrib::getTrcPos()
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

void AVOPolarAttrib::prepPriorToBoundsCalc()
{
    sampgateBG_ = Interval<int>( mNINT32(gateBG_.start/refstep_), mNINT32(gateBG_.stop/refstep_) );
    sampgate_ = Interval<int>( mNINT32(gate_.start/refstep_), mNINT32(gate_.stop/refstep_) );
    Provider::prepPriorToBoundsCalc();
}

const BinID* AVOPolarAttrib::desStepout( int inp, int outp ) const
{
    return inp ? 0: &stepout_;
}

const Interval<int>* AVOPolarAttrib::desZSampMargin( int inp, int outp ) const
{
    return inp ? 0: &sampgateBG_;
}

bool AVOPolarAttrib::getInputData( const BinID& relpos, int zintv )
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

bool AVOPolarAttrib::computeData( const DataHolder& output, const BinID& relpos, int z0, int nrsamples, int threadid) const
{
    if ( intercept_.isEmpty() || gradient_.isEmpty() || output.isEmpty() )
        return false;

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
    
    Eigen::ArrayXd bgAngle(0);
    Eigen::ArrayXd evAngle(0);
    Eigen::ArrayXd quality(0);
    Eigen::ArrayXd angleDiff(0);
    Eigen::ArrayXd strength(0);
    
    if (isOutputEnabled(BGAngle)) {
        computeBackgroundAngle( A, B, bgAngle );
        for (int idx=0; idx<nrsamples; idx++)
            setOutputValue(output,BGAngle, idx, z0, bgAngle[idx-sampgateBG_.start]);
    }
    
    if (isOutputEnabled(EventAngle)) {
        computeEventAngle( A, B, evAngle, quality );
        for (int idx=0; idx<nrsamples; idx++)
            setOutputValue(output,EventAngle, idx, z0, evAngle[idx-sampgateBG_.start]);
    }
    
    if (isOutputEnabled(Difference)) {
        if (bgAngle.size()==0)
            computeBackgroundAngle( A, B, bgAngle );
        if (evAngle.size()==0)
            computeEventAngle( A, B, evAngle, quality );
        angleDiff = evAngle - bgAngle;
        for (int idx=0; idx<nrsamples; idx++)
            setOutputValue(output,Difference, idx, z0, angleDiff[idx-sampgateBG_.start]);
    }
    
    if (isOutputEnabled(Strength)) {
        computeStrength( A, B, strength );
        for (int idx=0; idx<nrsamples; idx++)
            setOutputValue(output,Strength, idx, z0, strength[idx-sampgateBG_.start]);
    }
    
    if (isOutputEnabled(Product)) {
        if (strength.size()==0)
            computeStrength( A, B, strength );
        if (angleDiff.size()==0) {
            if (bgAngle.size()==0)
                computeBackgroundAngle( A, B, bgAngle );
            if (evAngle.size()==0)
                computeEventAngle( A, B, evAngle, quality );
            angleDiff = evAngle - bgAngle;
        }
        Eigen::ArrayXd prod = angleDiff*strength;
        for (int idx=0; idx<nrsamples; idx++)
            setOutputValue(output,Product, idx, z0, prod[idx-sampgateBG_.start]);
    }
    
    if (isOutputEnabled(Quality)) {
        if (evAngle.size()==0)
            computeEventAngle( A, B, evAngle, quality );
        for (int idx=0; idx<nrsamples; idx++)
            setOutputValue(output,Quality, idx, z0, quality[idx-sampgateBG_.start]);
    }
    
    return true;
}

void AVOPolarAttrib::computeEventAngle(  const Eigen::ArrayXXd& A, const Eigen::ArrayXXd& B, Eigen::ArrayXd& angle, Eigen::ArrayXd& qual) const
{
    const int sz = A.rows();
    angle.resize(sz);
    qual.resize(sz);
    
    Eigen::ArrayXd A2 = A.col(centertrcidx_)*A.col(centertrcidx_);
    Eigen::ArrayXd B2 = B.col(centertrcidx_)*B.col(centertrcidx_);
    Eigen::ArrayXd AB = A.col(centertrcidx_)*B.col(centertrcidx_);
    
    Eigen::ArrayXd A2win(sz);
    windowedOpsEigen::sum( A2, sampgate_.width(), A2win );
    Eigen::ArrayXd B2win(sz);
    windowedOpsEigen::sum( B2, sampgate_.width(), B2win );
    Eigen::ArrayXd ABwin(sz);
    windowedOpsEigen::sum( AB, sampgate_.width(), ABwin );
    
    Eigen::ArrayXd A2mB2 = A2win - B2win;
    Eigen::ArrayXd del = (4.0*ABwin.square() + A2mB2.square()).sqrt().eval();
    
    angle = (2.0*ABwin/(A2mB2 + del)).atan()/M_PI*180.0;
    qual = del / (A2win + B2win);
}

void AVOPolarAttrib::computeBackgroundAngle(  const Eigen::ArrayXXd& A, const Eigen::ArrayXXd& B, Eigen::ArrayXd& result ) const
{
    const int sz = A.rows();
    result.resize(sz);
    
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
    
    result = (2.0*ABwin/(A2mB2 + (4.0*ABwin.square() + A2mB2.square()).sqrt())).atan()/M_PI*180.0;
}

void AVOPolarAttrib::computeStrength(  const Eigen::ArrayXXd& A, const Eigen::ArrayXXd& B, Eigen::ArrayXd& result ) const
{        
    const int sz = A.rows();
    result.resize(sz);
    
    Eigen::ArrayXi imax(sz);
    Eigen::ArrayXd Amax(sz);
    windowedOpsEigen::max( A.col(centertrcidx_), sampgate_.width(), Amax, imax); 
    Eigen::ArrayXi imin(sz);
    Eigen::ArrayXd Amin(sz);
    windowedOpsEigen::min( A.col(centertrcidx_), sampgate_.width(), Amin, imin); 
    
    for (int idx=0; idx<sz;idx++) {
        double bmin = B(imin[idx],centertrcidx_);
        double bmax = B(imax[idx],centertrcidx_);
        result[idx] = sqrt(Amin[idx]*Amin[idx]+bmin*bmin) + sqrt(Amax[idx]*Amax[idx]+bmax*bmax);
    }
}
} // Attrib
    
    
    


    
