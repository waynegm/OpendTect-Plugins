/*
 *   uiEFDModesAttrib Plugin -  Empirical Fourier Decomposition Modes UI
 *   Copyright (C) 2021  Wayne Mogg
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "uiefdmodesattrib.h"
#include "efdmodesattrib.h"

#include "attribdesc.h"
#include "attribparam.h"
#include "survinfo.h"
#include "uiattribfactory.h"
#include "uiattrsel.h"
#include "uibutton.h"
#include "uispinbox.h"
#include "trckeyzsampling.h"
#include "uimsg.h"

#include "wmplugins.h"

using namespace Attrib;

mInitAttribUI(uiEFDModesAttrib,EFDModesAttrib,"EFD Modes",wmPlugins::sKeyWMPlugins())


uiEFDModesAttrib::uiEFDModesAttrib( uiParent* p, bool is2d )
: uiAttrDescEd(p,is2d,HelpKey("wgm", "efd"))
, TestPanelAdaptor()
{
    inpfld_ = createInpFld( is2d );

    nrmodesfld_ = new uiLabeledSpinBox( this, tr("Number of Modes") );
    nrmodesfld_->box()->setInterval(1, 100);
    nrmodesfld_->box()->setValue(5);
    nrmodesfld_->attach( alignedBelow, inpfld_ );
    mAttachCB(nrmodesfld_->box()->valueChanged,uiEFDModesAttrib::nrmodesChgCB);

    outmodefld_ = new uiLabeledSpinBox(this, tr("Output Mode"));
    outmodefld_->attach( alignedBelow, nrmodesfld_ );
    outmodefld_->box()->setInterval(0, nrmodesfld_->box()->getIntValue()-1);
    outmodefld_->box()->setValue(0);

    uiString butstr = tr("Display EFD Modes panel");
    modepanelbut_ = new uiPushButton( this, butstr, mCB(this, uiEFDModesAttrib, showPosDlgCB), true );
    modepanelbut_->attach( alignedBelow, outmodefld_ );

    nrmodesChgCB(nullptr);
    setHAlignObj( inpfld_ );
}

uiEFDModesAttrib::~uiEFDModesAttrib()
{
    detachAllNotifiers();
}

void uiEFDModesAttrib::nrmodesChgCB( CallBacker* )
{
    const int mxmodes = nrmodesfld_->box()->getIntValue();
    const int selmode = mMIN(mxmodes-1, outmodefld_->box()->getIntValue());
    outmodefld_->box()->setInterval( 0, mxmodes-1);
    outmodefld_->box()->setValue( selmode);
}

bool uiEFDModesAttrib::setParameters( const Attrib::Desc& desc )
{
    if ( desc.attribName() != EFDModesAttrib::attribName() )
	return false;

    mIfGetInt(EFDModesAttrib::nrmodesStr(), nrmodes, nrmodesfld_->box()->setValue(nrmodes));
    nrmodesChgCB(nullptr);
    return true;
}


bool uiEFDModesAttrib::setInput( const Attrib::Desc& desc )
{
    putInp( inpfld_, desc, 0 );
    return true;
}

bool uiEFDModesAttrib::setOutput( const Desc& desc )
{
    outmodefld_->box()->setValue(desc.selectedOutput());
    return true;
}

bool uiEFDModesAttrib::getParameters( Attrib::Desc& desc )
{
    if ( desc.attribName() != EFDModesAttrib::attribName() )
	return false;

    mSetInt( EFDModesAttrib::nrmodesStr(), nrmodesfld_->box()->getIntValue() );
    return true;
}

bool uiEFDModesAttrib::getInput( Attrib::Desc& desc )
{
    inpfld_->processInput();
    fillInp( inpfld_, desc, 0 );
    return true;
}

bool uiEFDModesAttrib::getOutput( Desc& desc )
{
    const int modeidx = outmodefld_->box()->getIntValue();
    fillOutput( desc, modeidx );
    return true;
}

void uiEFDModesAttrib::getEvalParams( TypeSet<EvalParam>& params ) const
{
    EvalParam ep( "Mode" ); ep.evaloutput_ = true;
    params += ep;
    params += EvalParam( "Number of Modes", EFDModesAttrib::nrmodesStr() );
}

void uiEFDModesAttrib::showPosDlgCB( CallBacker* )
{
    if ( inpfld_->attribID() == DescID::undef() )
    {
	uiMSG().error( tr("Please fill in the Input Data field") );
	return;
    }
    delete( testpanel_ );
    testpanel_ = new uiAttribTestPanel<uiEFDModesAttrib>( *this,
				     "Compute all modes for a single trace",
				     "Mode Decomposition",
				     "Empirical Fourier Decomposition",
				     "Mode", true );
    testpanel_->showPosDlg();
}

#define mSetParam( type, nm, str, fn )\
{ \
    mDynamicCastGet(type##Param*, nm, desc->getValParam(str))\
    nm->setValue( fn );\
}

void uiEFDModesAttrib::fillTestParams( Attrib::Desc* desc ) const
{
    mSetParam(Int,nrmodes, EFDModesAttrib::nrmodesStr(), nrmodesfld_->box()->getIntValue())
}
