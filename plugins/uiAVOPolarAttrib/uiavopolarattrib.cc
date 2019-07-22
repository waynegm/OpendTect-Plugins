#include "uiavopolarattrib.h"
#include "avopolarattrib.h"

#include "attribdesc.h"
#include "attribdescset.h"
#include "attribparam.h"
#include "uiattribfactory.h"
#include "uiattrsel.h"
#include "uigeninput.h"
#include "uistepoutsel.h"

using namespace Attrib;

static const char* OutputNames[] =
{
    "Background Polarization Angle",
    "Event Polarization Angle",
    "Polarization Angle Difference",
    "Strength",
    "Polarization Product",
    "Quality",
    0
};

mInitAttribUI(uiAVOPolarAttrib,AVOPolarAttrib,"AVOPolarAttrib",sKeyBasicGrp())

uiAVOPolarAttrib::uiAVOPolarAttrib( uiParent* p, bool is2d )
: uiAttrDescEd(p,is2d, HelpKey("wgm","avop") )
{
    interceptfld_ = createInpFld( is2d, "Intercept" );
    gradientfld_ = createInpFld( is2d, "Gradient" );
    gradientfld_->attach( alignedBelow, interceptfld_ );
    
    outputfld_ = new uiGenInput( this, uiStrings::sOutput(), StringListInpSpec(OutputNames) );
    CallBack cboutsel = mCB(this, uiAVOPolarAttrib, outSel);
    outputfld_->valuechanged.notify(cboutsel);
    outputfld_->attach( alignedBelow, gradientfld_ );
    
    gateBGfld_ = new uiGenInput( this, zDepLabel(tr("Background "), tr("gate")), FloatInpIntervalSpec().setName("Z start",0).setName("Z stop",1) );
    gateBGfld_->attach( alignedBelow, outputfld_ );
    
    stepoutfld_ = new uiStepOutSel( this, is2d );
    stepoutfld_->setFieldNames( "Stepout Inl", "Stepout Crl" );
    stepoutfld_->attach( rightTo, gateBGfld_ );
    
    gatefld_ = new uiGenInput( this, zDepLabel(tr("Event "), tr("gate")), FloatInpIntervalSpec().setName("Z start",0).setName("Z stop",1) );
    gatefld_->attach( alignedBelow, gateBGfld_ );
    
    outSel(0);
    setHAlignObj( interceptfld_ );
}

void uiAVOPolarAttrib::outSel( CallBacker* cb )
{
    const int outval = outputfld_->getIntValue();
    const bool need_BG = outval==AVOPolarAttrib::BGAngle 
                         || outval==AVOPolarAttrib::Difference 
                         || outval==AVOPolarAttrib::Product;
    const bool need_evangle = outval==AVOPolarAttrib::EventAngle
                                || outval==AVOPolarAttrib::Difference
                                || outval==AVOPolarAttrib::Strength
                                || outval==AVOPolarAttrib::Product
                                || outval==AVOPolarAttrib::Quality;
    gateBGfld_->display(need_BG);
    stepoutfld_->display(need_BG);
    gatefld_->display(need_evangle);
}

bool uiAVOPolarAttrib::setParameters( const Desc& desc )
{
    if ( desc.attribName() != AVOPolarAttrib::attribName() )
        return false;
    
    mIfGetFloatInterval( AVOPolarAttrib::gateBGStr(), gateBG, gateBGfld_->setValue(gateBG) );
    mIfGetBinID( AVOPolarAttrib::stepoutStr(), stepout, stepoutfld_->setBinID(stepout) );
    mIfGetFloatInterval( AVOPolarAttrib::gateStr(), gate, gatefld_->setValue(gate) );
    outSel(0);
    return true;
}

bool uiAVOPolarAttrib::setInput( const Desc& desc )
{
    putInp( interceptfld_, desc, 0 );
    putInp( gradientfld_, desc, 1 );
    
    return true;
}

bool uiAVOPolarAttrib::setOutput( const Desc& desc )
{
    outputfld_->setValue( desc.selectedOutput() );
    outSel(0);
    return true;
}

bool uiAVOPolarAttrib::getParameters( Desc& desc )
{
    if ( desc.attribName() != AVOPolarAttrib::attribName() )
        return false;

    mSetFloatInterval( AVOPolarAttrib::gateBGStr(), gateBGfld_->getFInterval() );
    BinID stepout( stepoutfld_->getBinID() );
    mSetBinID( AVOPolarAttrib::stepoutStr(), stepout );
    mSetFloatInterval( AVOPolarAttrib::gateStr(), gatefld_->getFInterval() );
    
    return true;
}

bool uiAVOPolarAttrib::getInput( Desc& desc )
{
    fillInp( interceptfld_, desc, 0 );
    fillInp( gradientfld_, desc, 1 );
    return true;
}

bool uiAVOPolarAttrib::getOutput( Desc& desc )
{
    fillOutput( desc, outputfld_->getIntValue() );
    return true;
}

void uiAVOPolarAttrib::getEvalParams( TypeSet<EvalParam>& params ) const
{
    params += EvalParam( timegatestr(), AVOPolarAttrib::gateBGStr() );
    params += EvalParam( stepoutstr(), AVOPolarAttrib::stepoutStr() );
    params += EvalParam( timegatestr(), AVOPolarAttrib::gateStr() );
    
    EvalParam ep( "Output" ); ep.evaloutput_ = true;
    params += ep;
}

