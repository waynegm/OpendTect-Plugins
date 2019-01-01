#include "mt_mai_mao.h"

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

namespace Attrib {

mAttrDefCreateInstance(MT_MAI_MAO)

void MT_MAI_MAO::initClass()
{
    mAttrStartInitClass
    
    ZGateParam* gate = new ZGateParam( gateStr() );
    gate->setLimits( Interval<float>(-1000,1000) );
    Interval<float> defgate( -8, 8 );
    gate->setDefaultValue( defgate );
    desc->addParam( gate );
 
    BinIDParam* stepout = new BinIDParam( stepoutStr() );
    stepout->setDefaultValue( BinID(3,3) );
    stepout->setLimits(Interval<int>(1,50),Interval<int>(1,50)),
    desc->addParam( stepout );
    

    desc->addInput( InputSpec("Input data 1",true) );
    desc->addInput( InputSpec("Input data 2",true) );
    desc->addInput( InputSpec("Input data 3",true) );   // Add more/less as required
    
    desc->setNrOutputs( Seis::UnknowData, 3 );
    desc->setLocality(Desc::MultiTrace);
    
    mAttrEndInitClass
}

void MT_MAI_MAO::updateDefaults( Desc& desc )
{
    ValParam* paramgate = desc.getValParam( gateStr() );
    mDynamicCastGet(ZGateParam*,zgate,paramgate)
    float roundedzstep = SI().zStep()*SI().zDomain().userFactor();
    if ( roundedzstep > 0 )
        roundedzstep = Math::Floor( roundedzstep );
    zgate->setDefaultValue( Interval<float>(-roundedzstep*2, roundedzstep*2) );
}

MT_MAI_MAO::MT_MAI_MAO( Desc& desc )
: Provider( desc )
{
    if ( !isOK() ) return;
    
    mGetFloatInterval( gate_, gateStr() );
    gate_.scale( 1.f/zFactor() );
    mGetBinID( stepout_, stepoutStr() );
    
    input_1_.allowNull( true );
    input_2_.allowNull( true );
    input_3_.allowNull( true ); // Add more/less as required
    
    getTrcPos();
}

bool MT_MAI_MAO::getTrcPos()
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

void MT_MAI_MAO::prepPriorToBoundsCalc()
{
    sampgate_ = Interval<int>( mNINT32(gate_.start/refstep_), mNINT32(gate_.stop/refstep_) );
    Provider::prepPriorToBoundsCalc();
}

const BinID* MT_MAI_MAO::desStepout( int inp, int outp ) const
{
    return inp ? 0: &stepout_;
}

const Interval<int>* MT_MAI_MAO::desZSampMargin( int inp, int outp ) const
{
    return inp ? 0: &sampgate_;
}

bool MT_MAI_MAO::getInputData( const BinID& relpos, int zintv )
{
    input_1_.erase();
    input_2_.erase();
    input_3_.erase();   // Add more/less as required
    
    while ( input_1_.size() < trcpos_.size() ) {
        input_1_ += 0;
        input_2_ += 0;
        input_3_ += 0;  // Add more/less as required
    }
    
    const BinID bidstep = inputs_[0]->getStepoutStep();
    if ( bidstep != inputs_[1]->getStepoutStep() ) {
        ErrMsg("getInputData - mismatch in stepout step between input attributes.");
        return false;
    }
    if ( bidstep != inputs_[2]->getStepoutStep() ) {
        ErrMsg("getInputData - mismatch in stepout step between input attributes.");
        return false;
    }

    for ( int idx=0; idx<trcpos_.size(); idx++ ) {
        BinID pos = relpos + trcpos_[idx] * bidstep;
        const DataHolder* data1 = inputs_[0]->getData( pos, zintv );
        const DataHolder* data2 = inputs_[1]->getData( pos, zintv );
        const DataHolder* data3 = inputs_[2]->getData( pos, zintv );
        if ( !data1 ) {
            pos = relpos + trcpos_[centertrcidx_]*bidstep;
            data1 = inputs_[0]->getData( pos, zintv );
            if ( !data1 ) return false;
        }
        input_1_.replace( idx, data1 );

        if ( !data2 ) {
            pos = relpos + trcpos_[centertrcidx_]*bidstep;
            data2 = inputs_[1]->getData( pos, zintv );
            if ( !data2 ) return false;
        }
        input_2_.replace( idx, data2 );

        if ( !data3 ) {             // Add more/less as required
            pos = relpos + trcpos_[centertrcidx_]*bidstep;
            data3 = inputs_[2]->getData( pos, zintv );
            if ( !data3 ) return false;
        }
        input_3_.replace( idx, data3 );
    }
    input_1_idx_ = getDataIndex(0);
    input_2_idx_ = getDataIndex(1);
    input_3_idx_ = getDataIndex(2); // Add more/less as required
    
    return true;
}

bool MT_MAI_MAO::computeData( const DataHolder& output, const BinID& relpos, int z0, int nrsamples, int threadid) const
{
    if ( input_1_. isEmpty() || input_2_.isEmpty() || input_3_.isEmpty() )
            return false;
        
        const int sz = sampgate_.width() + nrsamples;
        const int ntraces = trcpos_.size();
        
        Array2DImpl<float> inp1(ntraces, sz);
        for (int trcidx=0; trcidx<ntraces; trcidx++) {
            const DataHolder* data = input_1_[trcidx];
            for (int idx=0; idx<sz; idx++) {
                float val = getInputValue(*data, input_1_idx_, sampgate_.start+idx, z0);
                inp1.set(trcidx, idx, mIsUdf(val)?0.0f:val);
            }
        }
        
        Array2DImpl<float> inp2(ntraces, sz);
        for (int trcidx=0; trcidx<ntraces; trcidx++) {
            const DataHolder* data = input_2_[trcidx];
            for (int idx=0; idx<sz; idx++) {
                float val = getInputValue(*data, input_2_idx_, sampgate_.start+idx, z0);
                inp2.set(trcidx, idx, mIsUdf(val)?0.0f:val);
            }
        }
        
        Array2DImpl<float> inp3(ntraces, sz);   // Add more/less as required
        for (int trcidx=0; trcidx<ntraces; trcidx++) {
            const DataHolder* data = input_3_[trcidx];
            for (int idx=0; idx<sz; idx++) {
                float val = getInputValue(*data, input_3_idx_, sampgate_.start+idx, z0);
                inp3.set(trcidx, idx, mIsUdf(val)?0.0f:val);
            }
        }
        
        Array1DImpl<float> result(0);
        if (isOutputEnabled(Outputs::Output_1)) {
            computeOutput_1( inp1, inp2, inp3, result );
            for (int idx=0; idx<nrsamples; idx++)
                setOutputValue(output,Outputs::Output_1, idx, z0, result[idx-sampgate_.start]);
        }
        
        if (isOutputEnabled(Outputs::Output_2)) {
            computeOutput_2( inp1, inp2, inp3, result );
            for (int idx=0; idx<nrsamples; idx++)
                setOutputValue(output,Outputs::Output_2, idx, z0, result[idx-sampgate_.start]);
        }
        
        if (isOutputEnabled(Outputs::Output_3)) {
            computeOutput_3( inp1, inp2, inp3, result );
            for (int idx=0; idx<nrsamples; idx++)
                setOutputValue(output,Outputs::Output_3, idx, z0, result[idx-sampgate_.start]);
        }
        
        return true;
}

void MT_MAI_MAO::computeOutput_1( const Array2DImpl<float>& inp1, 
                                  const Array2DImpl<float>& inp2,
                                  const Array2DImpl<float>& inp3,
                                  Array1DImpl<float>& result ) const
{
    int ntraces = inp1.info().getSize(0);
    int sz = inp1.info().getSize(1);
    result.setSize(sz);
    
    for (int idx=0; idx<sz; idx++) {
        result.set(idx, inp2.get(centertrcidx_, idx));
    }
}    
    
void MT_MAI_MAO::computeOutput_2( const Array2DImpl<float>& inp1, 
                                  const Array2DImpl<float>& inp2,
                                  const Array2DImpl<float>& inp3,
                                  Array1DImpl<float>& result ) const
{
    int ntraces = inp1.info().getSize(0);
    int sz = inp1.info().getSize(1);
    result.setSize(sz);
    
    for (int idx=0; idx<sz; idx++) {
        result.set(idx, inp3.get(centertrcidx_, idx));
    }
}    

void MT_MAI_MAO::computeOutput_3( const Array2DImpl<float>& inp1, 
                                  const Array2DImpl<float>& inp2,
                                  const Array2DImpl<float>& inp3,
                                  Array1DImpl<float>& result ) const
{
    int ntraces = inp1.info().getSize(0);
    int sz = inp1.info().getSize(1);
    result.setSize(sz);
    
    for (int idx=0; idx<sz; idx++) {
        result.set(idx, inp1.get(centertrcidx_, idx));
    }
}    

}

        
        


    
