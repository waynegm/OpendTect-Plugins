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
#include "uitable.h"
#include "uilabel.h"
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

	int lstfld = 0;
	for (int i=0; i<cNrInputs; i++) {
		BufferString bfs = "Input Data"; bfs += i+1;
		uiAttrSel* tmp = createInpFld( is2d, bfs );
		if (i==0) {
			tmp->attach(alignedBelow, exfilefld_);
			lstfld = 0;
		}	else if (i%2)
			tmp->attach(rightTo, inpflds_[i-1]);
		else {
			tmp->attach(alignedBelow, inpflds_[i-2]);
			lstfld = i;
		}
		tmp->display(false);
		inpflds_ += tmp;
	}

	outputfld_ = new uiGenInput( this, "Output", StringListInpSpec() );
	outputfld_->attach(alignedBelow, inpflds_[lstfld]);
	outputfld_->display(false);

	zmarginfld_ = new uiGenInput( this, "Z Window (samples)___", IntInpIntervalSpec().setName("Samples after",0).setName("Samples before",1) );
	zmarginfld_->attach( alignedBelow, outputfld_ );
	zmarginfld_->valuechanged.notify(mCB(this, uiExternalAttrib,doZmarginSymmetry) );
	zmarginfld_->display(false);
	
	stepoutfld_ = new uiStepOutSel( this, is2d );
	stepoutfld_->attach( rightTo, zmarginfld_ );
	stepoutfld_->setFieldNames( "Inl Stepout", "Crl Stepout" );
	stepoutfld_->display(false);
	

	selectfld_ = new uiGenInput( this, "Selection_______________", StringListInpSpec() );
	selectfld_->attach(alignedBelow, zmarginfld_);
	selectfld_->display(false);

	for (int i=0; i<cNrParams; i++) {
		uiGenInput* tmp = new uiGenInput( this, "Par____________________", FloatInpSpec() );
		if (i==0) 
			tmp->attach( alignedBelow, selectfld_ );
		else if (i%2)
			tmp->attach( rightTo, parflds_[i-1] );
		else
			tmp->attach( alignedBelow, parflds_[i-2] );
		tmp->setValue(0);
		tmp->display(false);
		parflds_ += tmp;
	}

	CallBack cb = mCB(this,uiExternalAttrib,doHelp);
	help_ = new uiToolButton( this, "contexthelp", "Help", cb );
	help_->attach (rightTo, exfilefld_);
	help_->display(false);
	
	
	setHAlignObj( interpfilefld_ );
}

uiExternalAttrib::~uiExternalAttrib()
{
	if (extproc_)
		delete extproc_;
}

void uiExternalAttrib::exfileChanged( CallBacker* )
{
//	ErrMsg("uiExternalAttrib::exfileChanged - enter");
	BufferString iname(interpfilefld_->fileName());
	BufferString fname(exfilefld_->fileName());
	if (!fname.isEmpty()) {
		if (extproc_ == NULL) 
			extproc_ = new ExtProc(fname.str(), iname.str());
		else if ( fname != extproc_->getFile() ) {
			delete extproc_;
			extproc_ = new ExtProc(fname.str(), iname.str());
		}
		if (extproc_ == NULL)
			return;
		if (extproc_->hasInput() || extproc_->hasInputs()) {
			int nIn = extproc_->numInput();
			for (int i=0; i<cNrInputs; i++) {
				if (i<nIn) {
					inpflds_[i]->setLabelText(extproc_->inputName(i).str());
					inpflds_[i]->display(true);
				} else 
					inpflds_[i]->display(false);
			}
		}
		if (extproc_->hasZMargin()) {
			Interval<int> val = extproc_->zmargin();
			if (extproc_->zSymmetric()) {
				val.start = val.stop;
				val.stop = -val.start;
			}
			zmarginfld_->setValue(val);
			if (extproc_->hideZMargin())
				zmarginfld_->display(false);
			else
				zmarginfld_->display(true);
				if (extproc_->zSymmetric()) {
					zmarginfld_->displayField(false, 1, 0);
					zmarginfld_->setTitleText("Z Window (+/-samples)");
				} else {
					zmarginfld_->displayField(true, 1, 0);
					zmarginfld_->setTitleText("Z Window (samples)");
				}
		} else
			zmarginfld_->display(false);

		if (extproc_->hasStepOut()) {
			stepoutfld_->setBinID(extproc_->stepout());
			if (extproc_->hideStepOut()) 
				stepoutfld_->display(false);
			else
				stepoutfld_->display(true);
		} else 
			stepoutfld_->display(false);

		if (extproc_->hasOutput()) {
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
		if (extproc_->hasSelect()) {
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
		for (int i=0; i<cNrParams; i++) {
			if (extproc_->hasParam(i)) {
				parflds_[i]->setTitleText(extproc_->paramName(i));
				parflds_[i]->setValue(extproc_->paramValue(i));
				parflds_[i]->display(true);
			} else {
				parflds_[i]->display(false);
			}
		}
		if (extproc_->hasHelp())
			help_->display(true);
		else
			help_->display(false);
	}
//	ErrMsg("uiExternalAttrib::exfileChanged - leave");

}



bool uiExternalAttrib::setParameters( const Attrib::Desc& desc )
{
//	ErrMsg("uiExternalAttrib::setParameters - enter");

    if ( desc.attribName() != ExternalAttrib::attribName())
	return false;

	
	mIfGetString(ExternalAttrib::interpFileStr(),interpfile, interpfilefld_->setFileName(interpfile) )
	mIfGetString(ExternalAttrib::exFileStr(),exfile, exfilefld_->setFileName(exfile) )
	exfileChanged(NULL);
	if (extproc_==NULL)
		return false;
	
	if (extproc_->hasZMargin()) {
		mIfGetIntInterval( ExternalAttrib::zmarginStr(), zmargin, zmarginfld_->setValue(zmargin) )
		if (extproc_->zSymmetric()) {
			Interval<int> val = zmarginfld_->getIInterval();
			val.start = val.stop;
			val.stop = -val.start;
			zmarginfld_->setValue( val );
		}
	}
	if (extproc_->hasStepOut()) {
		mIfGetBinID( ExternalAttrib::stepoutStr(), stepout, stepoutfld_->setBinID(stepout) )
	}
	if (extproc_->hasSelect()) {
		mIfGetInt( ExternalAttrib::selectStr(), select, selectfld_->setValue(select));
	}
	for (int i=0; i<cNrParams; i++) {
		if (extproc_->hasParam(i)) {
			BufferString s(ExternalAttrib::parStr());
			s.add(i);
			mIfGetFloat( s, par, parflds_[i]->setValue(par) )
		}
	}
//	ErrMsg("uiExternalAttrib::setParameters - leave");

    return true;
}

bool uiExternalAttrib::setInput( const Attrib::Desc& desc )
{
//	ErrMsg("uiExternalAttrib::setInput - enter");
	if (extproc_!=NULL) {
		const int nrflds = extproc_->numInput();
		for (int i=0; i<nrflds; i++)
			putInp( inpflds_[i], desc, i );
	}
//	ErrMsg("uiExternalAttrib::setInput - leave");
    return true;
}

bool uiExternalAttrib::setOutput( const Attrib::Desc& desc )
{
//	ErrMsg("uiExternalSttrib::setOutput - enter");

	if (extproc_ && extproc_->hasOutput()) 
		outputfld_->setValue(desc.selectedOutput());
//	ErrMsg("uiExternalAttrib::setOutput - leave");
	return true;
}

bool uiExternalAttrib::getParameters( Attrib::Desc& desc )
{
//	ErrMsg("uiExternalSttrib::getParameters - enter");
    if ( desc.attribName() != ExternalAttrib::attribName())
	return false;

	mSetString( ExternalAttrib::interpFileStr(), interpfilefld_->fileName() );
	mSetString( ExternalAttrib::exFileStr(), exfilefld_->fileName() );
	if (extproc_==NULL ) {
		ErrMsg("uiExternalAttrib::getParameters - extproc_ is NULL");
		return false;
	}
	if (extproc_->hasZMargin()) {
		Interval<int> val = zmarginfld_->getIInterval();
		if (extproc_->zSymmetric()) {
			val.stop = val.start;
			val.start = -val.stop;
		}
		mSetIntInterval( ExternalAttrib::zmarginStr(), val );
	}
	if (extproc_->hasStepOut())  {
		mSetBinID(ExternalAttrib::stepoutStr(), stepoutfld_->getBinID());
	}
	if (extproc_->hasSelect())  {
		mSetInt( ExternalAttrib::selectStr(), selectfld_->getIntValue() );
	}
	for (int i=0; i<cNrParams; i++) {
		if (extproc_->hasParam(i))  {
			BufferString s(ExternalAttrib::parStr());
			s.add(i);
			ErrMsg(s);
			mSetFloat( s, (parflds_[i])->getfValue() );
		}
	}
//	ErrMsg("uiExternalSttrib::getParameters - leave");
	
    return true;
}


bool uiExternalAttrib::getInput( Attrib::Desc& desc )
{
//	ErrMsg("uiExternalSttrib::getInput - enter");

	for (int i=0; i<inpflds_.size(); i++) {
		inpflds_[i]->processInput();
		fillInp( inpflds_[i], desc, i );
	}
//	ErrMsg("uiExternalSttrib::getInput - leave");

	return true;
}

bool uiExternalAttrib::getOutput( Desc& desc )
{
//	ErrMsg("uiExternalSttrib::getOutput - enter");
	if (extproc_ && extproc_->hasOutput()) 
		fillOutput( desc, outputfld_->getIntValue() );
//	ErrMsg("uiExternalSttrib::getOutput - leave");
	return true;
}

void uiExternalAttrib::doHelp( CallBacker* cb )
{
	if (extproc_ && extproc_->hasHelp())
		uiDesktopServices::openUrl(extproc_->helpValue());
	
}

void uiExternalAttrib::doZmarginSymmetry( CallBacker* cb )
{
	if (extproc_ && extproc_->zSymmetric()) {
		Interval<int> val = zmarginfld_->getIInterval();
		val.stop = -val.start;
		zmarginfld_->setValue( val );
	}
}
