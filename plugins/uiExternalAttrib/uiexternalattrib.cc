/*Copyright (C) 2014 Wayne Mogg. All rights reserved.

This file may be used either under the terms of:

1. The GNU General Public License version 3 or higher, as published by
the Free Software Foundation, or

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

/*+
________________________________________________________________________

 Author:        Wayne Mogg
 Date:          December 2014
 _______________________________________________________________________

-*/

#include "uiexternalattrib.h"
#include "externalattrib.h"
#include "extproc.h"

#include "attribdesc.h"
#include "attribparam.h"
#include "uiattribfactory.h"
#include "uiattrsel.h"
#include "uifileinput.h"
#include "uimsg.h"
#include "uigeninput.h"
#include "uistepoutsel.h"
#include "uitoolbutton.h"
#include "uidesktopservices.h"

using namespace Attrib;


mInitAttribUI(uiExternalAttrib,ExternalAttrib,"External Attribute",sKeyBasicGrp())


uiExternalAttrib::uiExternalAttrib( uiParent* p, bool is2d )
    : uiAttrDescEd(p,is2d,"mToDoHelpID"), extproc_(NULL)

{
	#ifdef __win__
	interpfilefld_ = new uiFileInput( this, "Interpreter (optional)", uiFileInput::Setup(uiFileDialog::Gen).forread( true ).filter("*.exe") );
	#else
	interpfilefld_ = new uiFileInput( this, "Interpreter (optional)", uiFileInput::Setup(uiFileDialog::Gen).forread( true ));
	#endif

	exfilefld_ = new uiFileInput( this, "External File", uiFileInput::Setup(uiFileDialog::Gen).forread( true ).filter("*.py") );
	exfilefld_->valuechanged.notify(mCB(this,uiExternalAttrib,exfileChanged) );
	exfilefld_->attach(alignedBelow, interpfilefld_);
	
	inpfld_ = createInpFld( is2D() );
	inpfld_->attach( alignedBelow, exfilefld_ );

	outputfld_ = new uiGenInput( this, "Output", StringListInpSpec() );
	outputfld_->attach(rightTo, inpfld_);
	outputfld_->display(false);

	zmarginfld_ = new uiGenInput( this, "Z Window (samples)", IntInpIntervalSpec().setName("Samples before",0).setName("Samples after",1) );
	zmarginfld_->attach( alignedBelow, inpfld_ );
	zmarginfld_->display(false);
	
	stepoutfld_ = new uiStepOutSel( this, is2d );
	stepoutfld_->attach( rightTo, zmarginfld_ );
	stepoutfld_->setFieldNames( "Inl Stepout", "Crl Stepout" );
	stepoutfld_->display(false);
	

	selectfld_ = new uiGenInput( this, "Selection_______________________________", StringListInpSpec() );
	selectfld_->attach(alignedBelow, zmarginfld_);
	selectfld_->display(false);
	
	par0fld_ = new uiGenInput( this, "Par________________________________________", FloatInpSpec() );
	par0fld_->attach( alignedBelow, selectfld_ );
	par0fld_->setValue(0);
	par0fld_->display(false);

	par1fld_ = new uiGenInput( this, "Par________________________________________", FloatInpSpec() );
	par1fld_->attach( alignedBelow, par0fld_ );
	par1fld_->setValue(0);
	par1fld_->display(false);
	
	par2fld_ = new uiGenInput( this, "Par________________________________________", FloatInpSpec() );
	par2fld_->attach( alignedBelow, par1fld_ );
	par2fld_->setValue(0);
	par2fld_->display(false);

	par3fld_ = new uiGenInput( this, "Par________________________________________", FloatInpSpec() );
	par3fld_->attach( alignedBelow, par2fld_ );
	par3fld_->setValue(0);
	par3fld_->display(false);

	par4fld_ = new uiGenInput( this, "Par________________________________________", FloatInpSpec() );
	par4fld_->attach( alignedBelow, par3fld_ );
	par4fld_->setValue(0);
	par4fld_->display(false);
	
	CallBack cb = mCB(this,uiExternalAttrib,doHelp);
	help_ = new uiToolButton( this, "contexthelp", "Help", cb );
	help_->attach (rightTo, exfilefld_);
	help_->display(false);
	
	
	setHAlignObj( exfilefld_ );
}

uiExternalAttrib::~uiExternalAttrib()
{
	if (extproc_)
		delete extproc_;
}

void uiExternalAttrib::exfileChanged( CallBacker* )
{
	BufferString iname(interpfilefld_->fileName());
	BufferString fname(exfilefld_->fileName());
	if (!fname.isEmpty()) {
		if (extproc_ == NULL) 
			extproc_ = new ExtProc(fname.str(), iname.str());
		else if ( fname != extproc_->getFile() ) {
			delete extproc_;
			extproc_ = new ExtProc(fname.str(), iname.str());
		}

		if (extproc_ && extproc_->hasInput()) {
			inpfld_->setLabelText(extproc_->inputName().str());
		}
		if (extproc_ && extproc_->hasZMargin()) {
			zmarginfld_->setValue(extproc_->zmargin());
			if (extproc_->hideZMargin())
				zmarginfld_->display(false);
			else
				zmarginfld_->display(true);
		} else
			zmarginfld_->display(false);

		if (extproc_ && extproc_->hasStepOut()) {
			stepoutfld_->setBinID(extproc_->stepout());
			if (extproc_->hideStepOut()) 
				stepoutfld_->display(false);
			else
				stepoutfld_->display(true);
		} else 
			stepoutfld_->display(false);

		if (extproc_ && extproc_->hasOutput()) {
			int nout = extproc_->numOutput();
			BufferStringSet nms;
			for (int i = 0; i<nout; i++) {
				BufferString nm = extproc_->outputName(i);
				nms.add(nm.buf());
			}
			StringListInpSpec spec(nms);
			outputfld_->newSpec(spec, 0);
			outputfld_->display(true);
		} else {
			outputfld_->display(false);
		}
		if (extproc_ && extproc_->hasSelect()) {
			selectfld_->setTitleText(extproc_->selectName());
			int nsel = extproc_->numSelect();
			BufferStringSet nms;
			for (int i = 0; i<nsel; i++) {
				BufferString nm = extproc_->selectOpt(i);
				nms.add(nm.buf());
			}
			StringListInpSpec spec(nms);
			selectfld_->newSpec(spec, 0);
			selectfld_->setValue(extproc_->selectValue());
			selectfld_->display(true);
		} else {
			selectfld_->display(false);
		}
		if (extproc_ && extproc_->hasParam(0)) {
			par0fld_->setTitleText(extproc_->paramName(0));
			par0fld_->setValue(extproc_->paramValue(0));
			par0fld_->display(true);
		} else {
			par0fld_->display(false);
		}
		if (extproc_ && extproc_->hasParam(1)) {
			par1fld_->setTitleText(extproc_->paramName(1));
			par1fld_->setValue(extproc_->paramValue(1));
			par1fld_->display(true);
		} else {
			par1fld_->display(false);
		}
		if (extproc_ && extproc_->hasParam(2)) {
			par2fld_->setTitleText(extproc_->paramName(2));
			par2fld_->setValue(extproc_->paramValue(2));
			par2fld_->display(true);
		} else {
			par2fld_->display(false);
		}
		if (extproc_ && extproc_->hasParam(3)) {
			par3fld_->setTitleText(extproc_->paramName(3));
			par3fld_->setValue(extproc_->paramValue(3));
			par3fld_->display(true);
		} else {
			par3fld_->display(false);
		}
		if (extproc_ && extproc_->hasParam(4)) {
			par4fld_->setTitleText(extproc_->paramName(4));
			par4fld_->setValue(extproc_->paramValue(4));
			par4fld_->display(true);
		} else {
			par4fld_->display(false);
		}
		if (extproc_ && extproc_->hasHelp())
			help_->display(true);
		else
			help_->display(false);
	}
}



bool uiExternalAttrib::setParameters( const Attrib::Desc& desc )
{
    if ( desc.attribName() != ExternalAttrib::attribName())
	return false;
	
	mIfGetString(ExternalAttrib::interpFileStr(),interpfile, interpfilefld_->setFileName(interpfile) )
	mIfGetString(ExternalAttrib::exFileStr(),exfile, exfilefld_->setFileName(exfile) )
	exfileChanged(NULL);
	
	if (extproc_ && extproc_->hasZMargin()) {
		mIfGetIntInterval( ExternalAttrib::zmarginStr(), zmargin, zmarginfld_->setValue(zmargin) )
	}
	if (extproc_ && extproc_->hasStepOut()) {
		mIfGetBinID( ExternalAttrib::stepoutStr(), stepout, stepoutfld_->setBinID(stepout) )
	}
	if (extproc_ && extproc_->hasSelect()) {
		mIfGetInt( ExternalAttrib::selectStr(), select, selectfld_->setValue(select));
	}
	if (extproc_ && extproc_->hasParam(0))  {
		mIfGetFloat( ExternalAttrib::par0Str(), par0, par0fld_->setValue(par0) )
	}
	if (extproc_ && extproc_->hasParam(1)) {
		mIfGetFloat( ExternalAttrib::par1Str(), par1, par1fld_->setValue(par1) )
	}
	if (extproc_ && extproc_->hasParam(2)) {
		mIfGetFloat( ExternalAttrib::par2Str(), par2, par2fld_->setValue(par2) )
	}
	if (extproc_ && extproc_->hasParam(3)) {
		mIfGetFloat( ExternalAttrib::par3Str(), par3, par3fld_->setValue(par3) )
	}
	if (extproc_ && extproc_->hasParam(4)) {
		mIfGetFloat( ExternalAttrib::par4Str(), par4, par4fld_->setValue(par4) )
	}
	
    return true;
}

bool uiExternalAttrib::setInput( const Attrib::Desc& desc )
{
	putInp( inpfld_, desc, 0 );
    return true;
}

bool uiExternalAttrib::setOutput( const Attrib::Desc& desc )
{
	if (extproc_ && extproc_->hasOutput()) 
		outputfld_->setValue(desc.selectedOutput());
	return true;
}

bool uiExternalAttrib::getParameters( Attrib::Desc& desc )
{
    if ( desc.attribName() != ExternalAttrib::attribName())
	return false;

	mSetString( ExternalAttrib::interpFileStr(), interpfilefld_->fileName() );
	mSetString( ExternalAttrib::exFileStr(), exfilefld_->fileName() );
	if (extproc_ && extproc_->hasZMargin()) {
		mSetIntInterval( ExternalAttrib::zmarginStr(), zmarginfld_->getIInterval() );
	}
	if (extproc_ && extproc_ && extproc_->hasStepOut())  {
		mSetBinID(ExternalAttrib::stepoutStr(), stepoutfld_->getBinID());
	}
	if (extproc_ && extproc_->hasSelect())  {
		mSetInt( ExternalAttrib::selectStr(), selectfld_->getIntValue() );
	}
	if (extproc_ && extproc_->hasParam(0))  {
		mSetFloat( ExternalAttrib::par0Str(), par0fld_->getfValue() );
	}
	if (extproc_ && extproc_->hasParam(1))  {
		mSetFloat( ExternalAttrib::par1Str(), par1fld_->getfValue() );
	}
	if (extproc_ && extproc_->hasParam(2)) {
		mSetFloat( ExternalAttrib::par2Str(), par2fld_->getfValue() );
	}
	if (extproc_ && extproc_->hasParam(3)) {
		mSetFloat( ExternalAttrib::par3Str(), par3fld_->getfValue() );
	}
	if (extproc_ && extproc_->hasParam(4)) {
		mSetFloat( ExternalAttrib::par4Str(), par4fld_->getfValue() );
	}
	
	
    return true;
}


bool uiExternalAttrib::getInput( Attrib::Desc& desc )
{
	inpfld_->processInput();
    fillInp( inpfld_, desc, 0 );
    return true;
}

bool uiExternalAttrib::getOutput( Desc& desc )
{
	if (extproc_ && extproc_->hasOutput()) 
		fillOutput( desc, outputfld_->getIntValue() );
	return true;
}

void uiExternalAttrib::doHelp( CallBacker* cb )
{
	if (extproc_ && extproc_->hasHelp())
		uiDesktopServices::openUrl(extproc_->helpValue());
	
}
