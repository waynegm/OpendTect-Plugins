#include "st_mai_sao.h"

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

mAttrDefCreateInstance(ST_MAI_SAO)

void ST_MAI_SAO::initClass()
{
    mAttrStartInitClass
    
    ZGateParam* gate = new ZGateParam( gateStr() );
    gate->setLimits( Interval<float>(-1000,1000) );
    Interval<float> defgate( -8, 8 );
    gate->setDefaultValue( defgate );
    desc->addParam( gate );
 
    desc->addInput( InputSpec("Input data 1",true) );
    desc->addInput( InputSpec("Input data 2",true) );
    desc->addInput( InputSpec("Input data 3",true) );   // Add more/less as required
    
    desc->setNrOutputs( Seis::UnknowData, 1 );
    desc->setLocality(Desc::SingleTrace);
    
    mAttrEndInitClass
}

void ST_MAI_SAO::updateDefaults( Desc& desc )
{
    ValParam* paramgate = desc.getValParam( gateStr() );
    mDynamicCastGet(ZGateParam*,zgate,paramgate)
    float roundedzstep = SI().zStep()*SI().zDomain().userFactor();
    if ( roundedzstep > 0 )
        roundedzstep = Math::Floor( roundedzstep );
    zgate->setDefaultValue( Interval<float>(-roundedzstep*2, roundedzstep*2) );
}

ST_MAI_SAO::ST_MAI_SAO( Desc& desc )
: Provider( desc )
{
    if ( !isOK() ) return;
    
    mGetFloatInterval( gate_, gateStr() );
    gate_.scale( 1.f/zFactor() );
}

void ST_MAI_SAO::prepPriorToBoundsCalc()
{
    sampgate_ = Interval<int>( mNINT32(gate_.start/refstep_), mNINT32(gate_.stop/refstep_) );
    Provider::prepPriorToBoundsCalc();
}

const Interval<int>* ST_MAI_SAO::desZSampMargin( int inp, int outp ) const
{
    return inp ? 0: &sampgate_;
}

bool ST_MAI_SAO::getInputData( const BinID& relpos, int zintv )
{
    input_1_ = inputs_[0]->getData( relpos, zintv );
    input_2_ = inputs_[1]->getData( relpos, zintv );
    input_3_ = inputs_[0]->getData( relpos, zintv );   // Add more/less as required
    
    input_1_idx_ = getDataIndex(0);
    input_2_idx_ = getDataIndex(1);
    input_3_idx_ = getDataIndex(2); // Add more/less as required
    
    return true;
}

bool ST_MAI_SAO::computeData( const DataHolder& output, const BinID& relpos, int z0, int nrsamples, int threadid) const
{
    if ( !input_1_ || input_1_->isEmpty() )
        return false;
    if ( !input_2_ || input_2_->isEmpty() )
        return false;
    if ( !input_3_ || input_3_->isEmpty() )
        return false;
        
    const int sz = sampgate_.width() + nrsamples;
        
    Array1DImpl<float> inp1(sz);
    for (int idx=0; idx<sz; idx++) {
        float val = getInputValue(*input_1_, input_1_idx_, sampgate_.start+idx, z0);
        inp1.set(idx, mIsUdf(val)?0.0f:val);
    }
        
    Array1DImpl<float> inp2(sz);
    for (int idx=0; idx<sz; idx++) {
        float val = getInputValue(*input_2_, input_2_idx_, sampgate_.start+idx, z0);
        inp2.set(idx, mIsUdf(val)?0.0f:val);
    }
        
    Array1DImpl<float> inp3(sz);   // Add more/less as required
    for (int idx=0; idx<sz; idx++) {
        float val = getInputValue(*input_3_, input_3_idx_, sampgate_.start+idx, z0);
        inp3.set(idx, mIsUdf(val)?0.0f:val);
    }
        
    Array1DImpl<float> result(0);
    computeOutput( inp1, inp2, inp3, result );
    for (int idx=0; idx<nrsamples; idx++)
        setOutputValue(output, 0, idx, z0, result[idx-sampgate_.start]);

    return true;
}

void ST_MAI_SAO::computeOutput( const Array1DImpl<float>& inp1, 
                                const Array1DImpl<float>& inp2,
                                const Array1DImpl<float>& inp3,
                                Array1DImpl<float>& result ) const
{
    int sz = inp1.info().getSize(0);
    result.setSize(sz);
    
    for (int idx=0; idx<sz; idx++) {
        result.set(idx, inp2.get(idx));
    }
}    

}

        
        


    
