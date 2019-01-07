#include "uimt_mai_sao.h"
#include "mt_mai_sao.h"

#include "attribdesc.h"
#include "attribdescset.h"
#include "attribparam.h"
#include "uiattribfactory.h"
#include "uiattrsel.h"
#include "uigeninput.h"
#include "uistepoutsel.h"

using namespace Attrib;

mInitAttribUI(uiMT_MAI_SAO,MT_MAI_SAO,"MT_MAI_SAO",sKeyBasicGrp())

uiMT_MAI_SAO::uiMT_MAI_SAO( uiParent* p, bool is2d )
: uiAttrDescEd(p,is2d, mNoHelpKey )
{
    inpfld_1_ = createInpFld( is2d, "Input_1" );

    inpfld_2_ = createInpFld( is2d, "Input_2" );
    inpfld_2_->attach( rightTo, inpfld_1_ );
    
    inpfld_3_ = createInpFld( is2d, "Input_3" );
    inpfld_3_->attach( rightTo, inpfld_2_ );
    
    gatefld_ = new uiGenInput( this, gateLabel(), FloatInpIntervalSpec().setName("Z start",0).setName("Z stop",1) );
    gatefld_->attach( alignedBelow, inpfld_1_ );
    
    stepoutfld_ = new uiStepOutSel( this, is2d );
    stepoutfld_->setFieldNames( "Stepout Inl", "Stepout Crl" );
    stepoutfld_->attach( rightTo, gatefld_ );
    
    setHAlignObj( inpfld_1_ );
}

bool uiMT_MAI_SAO::setParameters( const Desc& desc )
{
    if ( desc.attribName() != MT_MAI_SAO::attribName() )
        return false;
    
    mIfGetFloatInterval( MT_MAI_SAO::gateStr(), gate, gatefld_->setValue(gate) );
    mIfGetBinID( MT_MAI_SAO::stepoutStr(), stepout, stepoutfld_->setBinID(stepout) );
    
    return true;
}

bool uiMT_MAI_SAO::setInput( const Desc& desc )
{
    putInp( inpfld_1_, desc, 0 );
    putInp( inpfld_2_, desc, 1 );
    putInp( inpfld_3_, desc, 2 );
    
    return true;
}

bool uiMT_MAI_SAO::getParameters( Desc& desc )
{
    if ( desc.attribName() != MT_MAI_SAO::attribName() )
        return false;

    mSetFloatInterval( MT_MAI_SAO::gateStr(), gatefld_->getFInterval() );

    BinID stepout( stepoutfld_->getBinID() );
    mSetBinID( MT_MAI_SAO::stepoutStr(), stepout );
    
    return true;
}

bool uiMT_MAI_SAO::getInput( Desc& desc )
{
    fillInp( inpfld_1_, desc, 0 );
    fillInp( inpfld_2_, desc, 1 );
    fillInp( inpfld_3_, desc, 2 );
    return true;
}

void uiMT_MAI_SAO::getEvalParams( TypeSet<EvalParam>& params ) const
{
    params += EvalParam( timegatestr(), MT_MAI_SAO::gateStr() );
    params += EvalParam( stepoutstr(), MT_MAI_SAO::stepoutStr() );
    
    EvalParam ep( "Output" ); ep.evaloutput_ = true;
    params += ep;
}

