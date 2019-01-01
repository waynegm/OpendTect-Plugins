#include "uist_mai_sao.h"
#include "st_mai_sao.h"

#include "attribdesc.h"
#include "attribdescset.h"
#include "attribparam.h"
#include "uiattribfactory.h"
#include "uiattrsel.h"
#include "uigeninput.h"

using namespace Attrib;

mInitAttribUI(uiST_MAI_SAO,ST_MAI_SAO,"ST_MAI_SAO",sKeyBasicGrp())

uiST_MAI_SAO::uiST_MAI_SAO( uiParent* p, bool is2d )
: uiAttrDescEd(p,is2d, mNoHelpKey )
{
    inpfld_1_ = createInpFld( is2d, "Input_1" );

    inpfld_2_ = createInpFld( is2d, "Input_2" );
    inpfld_2_->attach( rightTo, inpfld_1_ );
    
    inpfld_3_ = createInpFld( is2d, "Input_3" );
    inpfld_3_->attach( rightTo, inpfld_2_ );
    
    gatefld_ = new uiGenInput( this, gateLabel(), FloatInpIntervalSpec().setName("Z start",0).setName("Z stop",1) );
    gatefld_->attach( alignedBelow, inpfld_1_ );
    
    setHAlignObj( inpfld_1_ );
}

bool uiST_MAI_SAO::setParameters( const Desc& desc )
{
    if ( desc.attribName() != ST_MAI_SAO::attribName() )
        return false;
    
    mIfGetFloatInterval( ST_MAI_SAO::gateStr(), gate, gatefld_->setValue(gate) );
    
    return true;
}

bool uiST_MAI_SAO::setInput( const Desc& desc )
{
    putInp( inpfld_1_, desc, 0 );
    putInp( inpfld_2_, desc, 1 );
    putInp( inpfld_3_, desc, 2 );
    
    return true;
}

bool uiST_MAI_SAO::getParameters( Desc& desc )
{
    if ( desc.attribName() != ST_MAI_SAO::attribName() )
        return false;

    mSetFloatInterval( ST_MAI_SAO::gateStr(), gatefld_->getFInterval() );

    return true;
}

bool uiST_MAI_SAO::getInput( Desc& desc )
{
    fillInp( inpfld_1_, desc, 0 );
    fillInp( inpfld_2_, desc, 1 );
    fillInp( inpfld_3_, desc, 2 );
    return true;
}

void uiST_MAI_SAO::getEvalParams( TypeSet<EvalParam>& params ) const
{
    params += EvalParam( timegatestr(), ST_MAI_SAO::gateStr() );
    
    EvalParam ep( "Output" ); ep.evaloutput_ = true;
    params += ep;
}

