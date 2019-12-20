/*Copyright (C) 2015 Wayne Mogg. All rights reserved.

This file may be used either under the terms of:

1. The GNU General Public License version 3 or higher, as published by
the Free Software Foundation, or

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

/*+
________________________________________________________________________

 Author:        Wayne Mogg
 Date:          November 2015
 _______________________________________________________________________

-*/

#include "uigradientattrib.h"
#include "gradientattrib.h"

#include "attribdesc.h"
#include "attribparam.h"
#include "uiattribfactory.h"
#include "uiattrsel.h"
#include "uigeninput.h"

#include "wmplugins.h"

using namespace Attrib;

static const char* opstr[] =
{
	"Kroon 3x3x3",
	"Farid 5x5x5",
	"Farid 7x7x7",
	0
};
static const char* outstr[] =
{
    "Inline",
    "Crossline",
    "Z",
    0
};

mInitAttribUI(uiGradientAttrib,GradientAttrib,"Gradient Attribute",wmPlugins::sKeyWMPlugins())


uiGradientAttrib::uiGradientAttrib( uiParent* p, bool is2d )
: uiAttrDescEd(p,is2d,HelpKey("wgm", "grad"))

{
    inpfld_ = createInpFld( is2d );
 
    outfld_ = new uiGenInput( this, tr("Output Gradient"), StringListInpSpec(outstr) );
    outfld_->attach( alignedBelow, inpfld_ );
	
	operatorfld_ = new uiGenInput( this, tr("Operator"), StringListInpSpec(opstr) );
    operatorfld_->attach( alignedBelow, outfld_ );
    
	setHAlignObj( outfld_ );
}




bool uiGradientAttrib::setParameters( const Attrib::Desc& desc )
{
    if ( desc.attribName() != GradientAttrib::attribName() )
	return false;

    mIfGetEnum(GradientAttrib::outputStr(),output, outfld_->setValue(output) )
    mIfGetEnum(GradientAttrib::operatorStr(), opert, operatorfld_->setValue(opert))
    return true;
}


bool uiGradientAttrib::setInput( const Attrib::Desc& desc )
{
    putInp( inpfld_, desc, 0 );
    return true;
}


bool uiGradientAttrib::getParameters( Attrib::Desc& desc )
{
    if ( desc.attribName() != GradientAttrib::attribName() )
	return false;

    mSetEnum( GradientAttrib::outputStr(), outfld_->getIntValue() );
    mSetEnum( GradientAttrib::operatorStr(), operatorfld_->getIntValue() );
    
    return true;
}


bool uiGradientAttrib::getInput( Attrib::Desc& desc )
{
    inpfld_->processInput();
    fillInp( inpfld_, desc, 0 );
    return true;
}



