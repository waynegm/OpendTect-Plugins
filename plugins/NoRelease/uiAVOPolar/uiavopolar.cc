#include "uiavopolar.h"
#include "avopolar.h"

#include "attribdesc.h"
#include "attribdescset.h"
#include "attribparam.h"
#include "uiattribfactory.h"
#include "uiattrsel.h"
#include "uigeninput.h"
#include "uistepoutsel.h"

using namespace Attrib;

mInitAttribUI(uiAVOPolar,AVOPolar,"AVOPolar",sKeyBasicGrp())

uiAVOPolar::uiAVOPolar( uiParent* p, bool is2d )
: uiAttrDescEd(p,is2d, mNoHelpKey )
{
    intercept_ = createInpFld( is2d, "Intercept" );
    gradient_ = createInpFld( is2d, "Gradient" );
    gradient_->attach( rightTo, intercept_ );
    
    gateBGfld_ = new uiGenInput( this, zDepLabel(tr("Background "), tr("gate")), FloatInpIntervalSpec().setName("Z start",0).setName("Z stop",1) );
    gateBGfld_->attach( alignedBelow, intercept_ );
    
    stepoutfld_ = new uiStepOutSel( this, is2d );
    stepoutfld_->setFieldNames( "Stepout Inl", "Stepout Crl" );
    stepoutfld_->attach( rightTo, gateBGfld_ );
    
    methodfld_ = new uiGenInput( this, "Method", StringListInpSpec(AVOPolar::MethodNames) );
    methodfld_->attach( alignedBelow, gateBGfld_ );
    
    parallelfld_ = new uiGenInput( this, "Parallel Processing", BoolInpSpec(false));
    parallelfld_->attach( alignedBelow, methodfld_);
    
    
    setHAlignObj( intercept_ );
}

bool uiAVOPolar::setParameters( const Desc& desc )
{
    if ( desc.attribName() != AVOPolar::attribName() )
        return false;
    
    mIfGetFloatInterval( AVOPolar::gateBGStr(), gateBG, gateBGfld_->setValue(gateBG) );
    mIfGetBinID( AVOPolar::stepoutStr(), stepout, stepoutfld_->setBinID(stepout) );

    mIfGetEnum( AVOPolar::methodStr(), method, methodfld_->setText(AVOPolar::MethodNames[method]) )
    mIfGetBool( AVOPolar::parallelStr(), doparallel, parallelfld_->setValue(doparallel));
    return true;
}

bool uiAVOPolar::setInput( const Desc& desc )
{
    putInp( intercept_, desc, 0 );
    putInp( gradient_, desc, 1 );
    
    return true;
}

bool uiAVOPolar::getParameters( Desc& desc )
{
    if ( desc.attribName() != AVOPolar::attribName() )
        return false;

    mSetFloatInterval( AVOPolar::gateBGStr(), gateBGfld_->getFInterval() );

    BinID stepout( stepoutfld_->getBinID() );
    mSetBinID( AVOPolar::stepoutStr(), stepout );

    BufferStringSet strs( AVOPolar::MethodNames );
    const char* method = methodfld_->text();
    mSetEnum( AVOPolar::methodStr(), strs.indexOf(method) );
    mSetBool( AVOPolar::parallelStr(), parallelfld_->getBoolValue());
    
    return true;
}

bool uiAVOPolar::getInput( Desc& desc )
{
    fillInp( intercept_, desc, 0 );
    fillInp( gradient_, desc, 1 );
    return true;
}

void uiAVOPolar::getEvalParams( TypeSet<EvalParam>& params ) const
{
    params += EvalParam( timegatestr(), AVOPolar::gateBGStr() );
    params += EvalParam( stepoutstr(), AVOPolar::stepoutStr() );
    
    EvalParam ep( "Output" ); ep.evaloutput_ = true;
    params += ep;
}

