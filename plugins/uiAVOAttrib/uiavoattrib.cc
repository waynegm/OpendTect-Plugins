/*Copyright (C) 2014 Wayne Mogg. All rights reserved.
 * 
 T hi*s file may be used either under the terms of:
 
 1. The GNU General Public License version 3 or higher, as published by
 the Free Software Foundation, or
 
 This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*+
 _______________________________________________________________________
 
 Author:        Wayne Mogg
 Date:          January 2014
 _______________________________________________________________________
 
 -*/

#include "uiavoattrib.h"
#include "avoattrib.h"

#include "attribdesc.h"
#include "attribparam.h"
#include "attribparamgroup.h"
#include "uiattribfactory.h"
#include "uiattrsel.h"
#include "uigeninput.h"

#include "wmplugins.h"

using namespace Attrib;


static const char* outputstr[] =
{
    "Fluid Factor",
    "Lithology Factor",
	"Porosity Factor",
    "Crossplot Angle",
	"Crossplot Deviation",
	"AVO Class",
    0
};


mInitAttribUI(uiAVOAttrib,AVOAttrib,"AVO Attributes",wmPlugins::sKeyWMPlugins())


uiAVOAttrib::uiAVOAttrib( uiParent* p, bool is2d )
	: uiAttrDescEd(p,is2d,HelpKey("wgm", "avo"))
{
	inp_interceptfld_ = createInpFld( is2d, "Intercept Volume" );
	inp_gradientfld_ = createInpFld( is2d, "Gradient Volume" );
	inp_gradientfld_->attach( alignedBelow, inp_interceptfld_ );
	
    outputfld_ = new uiGenInput( this, uiStrings::sOutput(), StringListInpSpec(outputstr) );
    outputfld_->valuechanged.notify( mCB(this,uiAVOAttrib,actionSel) );
    outputfld_->attach( alignedBelow, inp_gradientfld_ );

    slopefld_ = new uiGenInput( this, tr("Crossplot Slope"), FloatInpSpec() );
    slopefld_->attach( alignedBelow, outputfld_ );

    intercept_devfld_ = new uiGenInput( this, tr("Intercept Standard Deviation"), FloatInpSpec() );
    intercept_devfld_->attach( alignedBelow, slopefld_ );
	intercept_devfld_->display(false);
	
	gradient_devfld_ = new uiGenInput( this, tr("Gradient Standard Deviation"), FloatInpSpec() );
	gradient_devfld_->attach( alignedBelow, intercept_devfld_ );
	gradient_devfld_->display(false);
	
	correlationfld_ = new uiGenInput( this, tr("Correlation Coefficient"), FloatInpSpec() );
	correlationfld_->attach( alignedBelow, gradient_devfld_ );
	correlationfld_->display(false);

	class2fld_ = new uiGenInput( this, tr("Class 2 Intercept Halfwidth"), FloatInpSpec() );
	class2fld_->attach( alignedBelow, correlationfld_ );
	class2fld_->display(false);
	
	actionSel(0);

    setHAlignObj( inp_interceptfld_ );
}


void uiAVOAttrib::actionSel( CallBacker* )
{
    const int outval = outputfld_->getIntValue();

    intercept_devfld_->display( outval==AVOAttrib::CrossplotAngle || outval==AVOAttrib::CrossplotDeviation );
	gradient_devfld_->display( outval==AVOAttrib::CrossplotAngle || outval==AVOAttrib::CrossplotDeviation );
	correlationfld_->display( outval==AVOAttrib::CrossplotDeviation );
	class2fld_->display( outval==AVOAttrib::AVOClass );
}


bool uiAVOAttrib::setParameters( const Desc& desc )
{
    if ( desc.attribName() != AVOAttrib::attribName() )
	return false;

    mIfGetEnum( AVOAttrib::outputStr(), output, outputfld_->setValue(output) );
    mIfGetFloat( AVOAttrib::slopeStr(), slope, slopefld_->setValue(slope) );
    mIfGetFloat( AVOAttrib::intercept_devStr(), intercept_dev, intercept_devfld_->setValue(intercept_dev) );
	mIfGetFloat( AVOAttrib::gradient_devStr(), gradient_dev, gradient_devfld_->setValue(gradient_dev) );
	mIfGetFloat( AVOAttrib::correlationStr(), correlation, correlationfld_->setValue(correlation) );
	mIfGetFloat( AVOAttrib::class2Str(), class2width, class2fld_->setValue(class2width) );
	actionSel(0);

    return true;
}


bool uiAVOAttrib::setInput( const Desc& desc )
{
    putInp( inp_interceptfld_, desc, 0 );
    putInp( inp_gradientfld_, desc, 1 );
    return true;
}


bool uiAVOAttrib::getParameters( Desc& desc )
{
    if ( desc.attribName() != AVOAttrib::attribName() )
	return false;

	const int outval = outputfld_->getIntValue();
	
    mSetEnum( AVOAttrib::outputStr(), outputfld_->getIntValue() );
	mSetFloat( AVOAttrib::slopeStr(), slopefld_->getfValue() );
	if ( outval == AVOAttrib::CrossplotAngle )
    {
    	mSetFloat( AVOAttrib::intercept_devStr(), intercept_devfld_->getfValue() );
    	mSetFloat( AVOAttrib::gradient_devStr(), gradient_devfld_->getfValue() );
    }
    if ( outval == AVOAttrib::CrossplotDeviation )
	{
		mSetFloat( AVOAttrib::intercept_devStr(), intercept_devfld_->getfValue() );
		mSetFloat( AVOAttrib::gradient_devStr(), gradient_devfld_->getfValue() );
		mSetFloat( AVOAttrib::correlationStr(), correlationfld_->getfValue() );
	}
	if ( outval == AVOAttrib::AVOClass )
		mSetFloat( AVOAttrib::class2Str(), class2fld_->getfValue() );
	
    return true;
}


bool uiAVOAttrib::getInput( Desc& desc )
{
    fillInp( inp_interceptfld_, desc, 0 );
    fillInp( inp_gradientfld_, desc, 1 );
    return true;
}


