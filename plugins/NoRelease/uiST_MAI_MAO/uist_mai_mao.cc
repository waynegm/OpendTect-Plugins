#include "uist_mai_mao.h"
#include "st_mai_mao.h"

#include "attribdesc.h"
#include "attribdescset.h"
#include "attribparam.h"
#include "uiattribfactory.h"
#include "uiattrsel.h"
#include "uigeninput.h"

using namespace Attrib;

static const char* outpstrs[] = 
{
    "Output_1",
    "Output_2",
    "Output_3",
    0
};

mInitAttribUI(uiST_MAI_MAO,ST_MAI_MAO,"ST_MAI_MAO",sKeyBasicGrp())

uiST_MAI_MAO::uiST_MAI_MAO( uiParent* p, bool is2d )
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
    
    setHAlignObj( inpfld_1_ );
}

bool uiST_MAI_MAO::setParameters( const Desc& desc )
{
    if ( desc.attribName() != ST_MAI_MAO::attribName() )
        return false;
    
    mIfGetFloatInterval( ST_MAI_MAO::gateStr(), gate, gatefld_->setValue(gate) );
    
    return true;
}

bool uiST_MAI_MAO::setInput( const Desc& desc )
{
    putInp( inpfld_1_, desc, 0 );
    putInp( inpfld_2_, desc, 1 );
    putInp( inpfld_3_, desc, 2 );
    
    return true;
}

bool uiST_MAI_MAO::setOutput( const Desc& desc )
{
    outpfld_->setValue( desc.selectedOutput() );
    return true;
}

bool uiST_MAI_MAO::getParameters( Desc& desc )
{
    if ( desc.attribName() != ST_MAI_MAO::attribName() )
        return false;

    mSetFloatInterval( ST_MAI_MAO::gateStr(), gatefld_->getFInterval() );

    return true;
}

bool uiST_MAI_MAO::getInput( Desc& desc )
{
    fillInp( inpfld_1_, desc, 0 );
    fillInp( inpfld_2_, desc, 1 );
    fillInp( inpfld_3_, desc, 2 );
    return true;
}

bool uiST_MAI_MAO::getOutput( Desc& desc )
{
    fillOutput( desc, outpfld_->getIntValue() );
    return true;
}

void uiST_MAI_MAO::getEvalParams( TypeSet<EvalParam>& params ) const
{
    params += EvalParam( timegatestr(), ST_MAI_MAO::gateStr() );
    
    EvalParam ep( "Output" ); ep.evaloutput_ = true;
    params += ep;
}

