#include "uimt_mai_mao.h"
#include "mt_mai_mao.h"

#include "attribdesc.h"
#include "attribdescset.h"
#include "attribparam.h"
#include "uiattribfactory.h"
#include "uiattrsel.h"
#include "uigeninput.h"
#include "uistepoutsel.h"

using namespace Attrib;

static const char* outpstrs[] = 
{
    "Output_1",
    "Output_2",
    "Output_3",
    0
};

mInitAttribUI(uiMT_MAI_MAO,MT_MAI_MAO,"MT_MAI_MAO",sKeyBasicGrp())

uiMT_MAI_MAO::uiMT_MAI_MAO( uiParent* p, bool is2d )
: uiAttrDescEd(p,is2d, mNoHelpKey )
{
    inpfld_1_ = createInpFld( is2d, "Input_1" );

    inpfld_2_ = createInpFld( is2d, "Input_2" );
    inpfld_2_->attach( rightTo, inpfld_1_ );
    
    inpfld_3_ = createInpFld( is2d, "Input_3" );
    inpfld_3_->attach( rightTo, inpfld_2_ );
    
    outpfld_ = new uiGenInput( this, uiStrings::sOutput(), StringListInpSpec(outpstrs) );
    outpfld_->attach( alignedBelow, inpfld_1_ );
    
    gatefld_ = new uiGenInput( this, gateLabel(), FloatInpIntervalSpec().setName("Z start",0).setName("Z stop",1) );
    gatefld_->attach( alignedBelow, outpfld_ );
    
    stepoutfld_ = new uiStepOutSel( this, is2d );
    stepoutfld_->setFieldNames( "Stepout Inl", "Stepout Crl" );
    stepoutfld_->attach( rightTo, gatefld_ );
    
    setHAlignObj( inpfld_1_ );
}

bool uiMT_MAI_MAO::setParameters( const Desc& desc )
{
    if ( desc.attribName() != MT_MAI_MAO::attribName() )
        return false;
    
    mIfGetFloatInterval( MT_MAI_MAO::gateStr(), gate, gatefld_->setValue(gate) );
    mIfGetBinID( MT_MAI_MAO::stepoutStr(), stepout, stepoutfld_->setBinID(stepout) );
    
    return true;
}

bool uiMT_MAI_MAO::setInput( const Desc& desc )
{
    putInp( inpfld_1_, desc, 0 );
    putInp( inpfld_2_, desc, 1 );
    putInp( inpfld_3_, desc, 2 );
    
    return true;
}

bool uiMT_MAI_MAO::setOutput( const Desc& desc )
{
    outpfld_->setValue( desc.selectedOutput() );
    return true;
}

bool uiMT_MAI_MAO::getParameters( Desc& desc )
{
    if ( desc.attribName() != MT_MAI_MAO::attribName() )
        return false;

    mSetFloatInterval( MT_MAI_MAO::gateStr(), gatefld_->getFInterval() );

    BinID stepout( stepoutfld_->getBinID() );
    mSetBinID( MT_MAI_MAO::stepoutStr(), stepout );
    
    return true;
}

bool uiMT_MAI_MAO::getInput( Desc& desc )
{
    fillInp( inpfld_1_, desc, 0 );
    fillInp( inpfld_2_, desc, 1 );
    fillInp( inpfld_3_, desc, 2 );
    return true;
}

bool uiMT_MAI_MAO::getOutput( Desc& desc )
{
    fillOutput( desc, outpfld_->getIntValue() );
    return true;
}

void uiMT_MAI_MAO::getEvalParams( TypeSet<EvalParam>& params ) const
{
    params += EvalParam( timegatestr(), MT_MAI_MAO::gateStr() );
    params += EvalParam( stepoutstr(), MT_MAI_MAO::stepoutStr() );
    
    EvalParam ep( "Output" ); ep.evaloutput_ = true;
    params += ep;
}

