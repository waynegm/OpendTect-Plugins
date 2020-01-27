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
#include "uiext_stepoutsel.h"
#include "externalattrib.h"
#include "extproc.h"

#include "attribdesc.h"
#include "attribdescset.h"
#include "attribparam.h"
#include "filepath.h"
#include "pythonaccess.h"
#include "settings.h"
#include "uiattribfactory.h"
#include "uiattrsel.h"
#include "uifileinput.h"
#include "uilabel.h"
#include "uimsg.h"
#include "uigeninput.h"
#include "uistepoutsel.h"
#include "uitable.h"
#include "uitoolbutton.h"
#include "uidesktopservices.h"
#include "uistrings.h"
#include "datainpspec.h"

#include "wmplugins.h"

using namespace Attrib;

uiExternalAttribInp::uiExternalAttribInp( uiParent* p, const uiTable::Setup& su )
: uiTable(p, su, "")
{

}

uiExternalAttribInp::~uiExternalAttribInp()
{}


mInitAttribUI(uiExternalAttrib,ExternalAttrib,"External Attribute",wmPlugins::sKeyWMPlugins())

uiExternalAttrib::uiExternalAttrib( uiParent* p, bool is2d )
: uiAttrDescEd(p,is2d, HelpKey("wgm", "ext")), extproc_(NULL)
, uiinp_(nullptr)
, outputfld_(nullptr)
, zmarginfld_(nullptr)
, stepoutfld_(nullptr)
, selectfld_(nullptr)

{
    uiFileInput::Setup su;
    su.defseldir("").forread(true);
    #ifdef __win__
	su.filter("*.exe");
    #endif
    interpfilefld_ = new uiFileInput( this, tr("Interpreter (optional)"), su );
	interpfilefld_->setFileName(ExternalAttrib::getPythonPath().fullPath());
    interpfilefld_->valuechanged.notify(mCB(this,uiExternalAttrib,exfileChanged) );

    exfilefld_ = new uiFileInput( this, tr("External File"), uiFileInput::Setup(uiFileDialog::Gen).forread( true ).defseldir(ExternalAttrib::exdir_) );
    exfilefld_->valuechanged.notify(mCB(this,uiExternalAttrib,exfileChanged) );
    exfilefld_->attach(alignedBelow, interpfilefld_);

    CallBack cb = mCB(this,uiExternalAttrib,doHelp);
    help_ = new uiToolButton( this, "contexthelp", uiStrings::sHelp(), cb );
    help_->attach (rightTo, exfilefld_);
    help_->display(false);

    makeUI();

    setHAlignObj( uiinp_ );
}

uiExternalAttrib::~uiExternalAttrib()
{
    if (extproc_)
	delete extproc_;

    detachAllNotifiers();
}


void uiExternalAttrib::makeUI()
{
    if ( !uiinp_ ) {
	uiinp_ = new uiExternalAttribInp( this,
				uiTable::Setup().rowgrow(false)
						.defrowlbl(false)
						.defcollbl(false)
						.mincolwdt(3.f*uiObject::baseFldSize())
						.maxcolwdt(4.f*uiObject::baseFldSize())
						.minrowhgt(1.5)
						.maxrowhgt(1.8)
	);
	uiinp_->setNrCols( 1 );
	uiinp_->setStretch(2,2);
	uiinp_->showGrid(false);
	uiinp_->setTopHeaderHidden(true);
	uiinp_->setLeftHeaderHidden(true);
	uiinp_->setBackgroundColor( this->backgroundColor() );
	uiinp_->setRowResizeMode( uiTable::ResizeToContents );
//	uiinp_->showOuterBorder( false );
	uiinp_->attach(alignedBelow, exfilefld_);
    }

    if ( !extproc_ )
	return;

    uiinp_->clearTable();
    uiinp_->setNrRows(0);
    inpflds_.erase();
    detachAllNotifiers();
    const uiAttrSelData asd( is2D(), false );
    if (extproc_->hasInput() || extproc_->hasInputs()) {
	int nIn = extproc_->numInput();
	for (int i=0; i<nIn; i++) {
	    uiAttrSel* tmp = new uiAttrSel( 0, extproc_->inputName(i).str(), asd );
	    tmp->setDescSet( ads_ );
	    tmp->setBorder(0);
	    tmp->display(true);
	    inpflds_ += tmp;
	    uiinp_->insertRows(uiinp_->nrRows(), 1);
	    uiinp_->setCellGroup( RowCol(uiinp_->nrRows()-1,0), tmp );
	    uiinp_->setColor( RowCol(uiinp_->nrRows()-1,0), this->backgroundColor() );
	}
    }

    if (extproc_->hasOutput()) {
	int nout = extproc_->numOutput();
	BufferStringSet nms;
	for (int i = 0; i<nout; i++) {
	    BufferString nm = extproc_->outputName(i);
	    nms.add(nm.buf());
	}
	outputfld_ = new uiGenInput( 0, uiStrings::sOutput(), StringListInpSpec(nms) );
	outputfld_->display(true);
	uiinp_->insertRows(uiinp_->nrRows(), 1);
	uiinp_->setCellGroup( RowCol(uiinp_->nrRows()-1,0), outputfld_ );
    }

    if (extproc_->hasZMargin() && !extproc_->hideZMargin()) {
	zmarginfld_ = new uiGenInput( 0, "Z Window (samples)", IntInpIntervalSpec().setName("Samples after",1).setName("Samples before",0) );
	mAttachCB(zmarginfld_->valuechanging, uiExternalAttrib::doZmarginCheck);

	Interval<int> val = extproc_->zmargin();
	val.start = -val.start;
	zmarginfld_->setValue(val);
	zmarginfld_->display(true);
	if (extproc_->zSymmetric()) {
	    zmarginfld_->displayField(false, 1, 0);
	    zmarginfld_->setTitleText(tr("Z Window (+/-samples)"));
	} else {
	    zmarginfld_->displayField(true, 1, 0);
	    zmarginfld_->setTitleText(tr("Z Window (samples)"));
	}
	uiinp_->insertRows(uiinp_->nrRows(), 1);
	uiinp_->setCellGroup( RowCol(uiinp_->nrRows()-1,0), zmarginfld_ );
    }

    if (extproc_->hasStepOut() && !extproc_->hideStepOut()) {
	stepoutfld_ = new uiExt_StepOutSel( 0, is2D() );
	mAttachCB(stepoutfld_->valueChanging,uiExternalAttrib::doStepOutCheck);
	stepoutfld_->setFieldNames( "Inl Stepout", "Crl Stepout" );
	stepoutfld_->display(true);
	stepoutfld_->setBinID(extproc_->stepout());
	stepoutfld_->displayFld2(!extproc_->so_same());
	uiinp_->insertRows(uiinp_->nrRows(), 1);
	uiinp_->setCellGroup( RowCol(uiinp_->nrRows()-1,0), stepoutfld_ );
    }

    if (extproc_->hasSelect()) {
	int nsel = extproc_->numSelect();
	BufferStringSet nms;
	for (int i = 0; i<nsel; i++) {
	    BufferString nm = extproc_->selectOpt(i);
	    nms.add(nm.buf());
	}
	selectfld_ = new uiGenInput( 0, tr("Selection"), StringListInpSpec(nms) );
	selectfld_->setTitleText(extproc_->selectName());
	selectfld_->setValue(extproc_->selectValue());
	selectfld_->display(true);
	uiinp_->insertRows(uiinp_->nrRows(), 1);
	uiinp_->setCellGroup( RowCol(uiinp_->nrRows()-1,0), selectfld_ );
    }

    parflds_.erase();
    for (int i=0; i<cNrParams; i++) {
	if (extproc_->hasParam(i)) {
	    uiGenInput* tmp = new uiGenInput( 0, extproc_->paramName(i), FloatInpSpec() );
	    tmp->setTitleText(extproc_->paramName(i));
	    tmp->setValue(extproc_->paramValue(i));
	    tmp->display(true);
	    parflds_ += tmp;
	    uiinp_->insertRows(uiinp_->nrRows(), 1);
	    uiinp_->setCellGroup( RowCol(uiinp_->nrRows()-1,0), tmp );
	}
    }

    uiinp_->selectRow(0);

    if (extproc_->hasHelp())
	help_->display(true);
    else
	help_->display(false);

}

void uiExternalAttrib::exfileChanged( CallBacker* )
{
    BufferString iname(interpfilefld_->fileName());
    BufferString fname(exfilefld_->fileName());
    if (!fname.isEmpty()) {
	if ( !extproc_ )
	    extproc_ = new ExtProc(fname.str(), iname.str());
	else if ( fname != extproc_->getFile() ) {
	    delete extproc_;
	    extproc_ = new ExtProc(fname.str(), iname.str());
	}

	makeUI();
    }
}

void uiExternalAttrib::setExFileName( const char* fname )
{
	BufferString tmp(fname);
	tmp.replace("%OD_EX_DIR%", ExternalAttrib::exdir_);
#ifdef __win__
	tmp.replace("/","\\");
#endif
	exfilefld_->setFileName(tmp);
}

BufferString uiExternalAttrib::getExFileName()
{
	BufferString tmp = exfilefld_->fileName();
	tmp.replace( ExternalAttrib::exdir_, "%OD_EX_DIR%" );
#ifdef __win__
	tmp.replace("\\","/");
#endif
	return tmp;
}

bool uiExternalAttrib::setParameters( const Attrib::Desc& desc )
{
    if ( desc.attribName() != ExternalAttrib::attribName())
	return false;
	
    mIfGetString(ExternalAttrib::interpFileStr(),interpfile, interpfilefld_->setFileName(interpfile) )
    mIfGetString(ExternalAttrib::exFileStr(),exfile, setExFileName(exfile) )
    exfileChanged(NULL);
    if (extproc_==NULL)
	return false;
	
    if (extproc_->hasZMargin() && !extproc_->hideZMargin()) {
	mIfGetIntInterval( ExternalAttrib::zmarginStr(), zmargin, zmarginfld_->setValues(-zmargin.start, zmargin.stop) )
    }

    if (extproc_->hasStepOut() && !extproc_->hideStepOut()) {
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
    return true;
}

bool uiExternalAttrib::setInput( const Attrib::Desc& desc )
{
    if (extproc_!=NULL) {
	const int nrflds = extproc_->numInput();
	for (int i=0; i<nrflds; i++) {
	    putInp( inpflds_[i], desc, i );
	}
    }
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
    mSetString( ExternalAttrib::exFileStr(), getExFileName() );
    if (extproc_==NULL ) {
	ErrMsg("uiExternalAttrib::getParameters - extproc_ is NULL");
	return false;
    }

    if (extproc_->hasZMargin()) {
	Interval<int> val;
	if ( extproc_->hideZMargin() )
	    val = extproc_->zmargin();
	else
	    val = zmarginfld_->getIInterval();

	if (extproc_->zSymmetric())
	    val.stop = val.start;
	val.start = -val.start;
	mSetIntInterval( ExternalAttrib::zmarginStr(), val );
    }

    if (extproc_->hasStepOut()) {
	if (!extproc_->hideStepOut()) {
	    mSetBinID(ExternalAttrib::stepoutStr(), stepoutfld_->getBinID());
	} else {
	    mSetBinID(ExternalAttrib::stepoutStr(), extproc_->stepout());
	}
    }

    if (extproc_->hasSelect())  {
	mSetInt( ExternalAttrib::selectStr(), selectfld_->getIntValue() );
    }

	for (int i=0; i<cNrParams; i++) {
		if (extproc_->hasParam(i))  {
			BufferString s(ExternalAttrib::parStr());
			s.add(i);
			mSetFloat( s, (parflds_[i])->getfValue() );
		}
	}
    return true;
}


bool uiExternalAttrib::getInput( Attrib::Desc& desc )
{
	if (extproc_!=NULL) {
		const int nrflds = extproc_->numInput();
		for (int i=0; i<nrflds; i++) {
			inpflds_[i]->processInput();
			fillInp( inpflds_[i], desc, i );
		}
	}
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

void uiExternalAttrib::doZmarginCheck( CallBacker* cb )
{
	if (extproc_ && extproc_->hasZMargin()) {
		Interval<int> val = zmarginfld_->getIInterval();
		Interval<int> minval = extproc_->z_minimum();

		val.stop = (val.stop < minval.stop)? minval.stop:val.stop;
		val.start = (val.start < -minval.start)? -minval.start:val.start;
		if (extproc_->zSymmetric())
			val.stop = -val.start;

		zmarginfld_->setValue( val );
	}
}

void uiExternalAttrib::doStepOutCheck( CallBacker* cb )
{
	BinID val = stepoutfld_->getBinID();
	if (extproc_ && extproc_->hasStepOut()) {
		BinID minval = extproc_->so_minimum();
		int inl = (val.inl() < minval.inl())? minval.inl():val.inl();
		int crl = (val.crl() < minval.crl())? minval.crl():val.crl();
		stepoutfld_->setBinID(BinID(inl,crl));
		if (extproc_->so_same())
			crl = inl; 
		stepoutfld_->setBinID(BinID(inl,crl));
	}
}
