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
#include "file.h"
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
#include "oddirs.h"

#include "uiwgmhelp.h"
#include "wmplugins.h"

using namespace Attrib;

uiExternalAttribInp::uiExternalAttribInp(uiParent* p)
: uiTable(p, uiTable::Setup().rowgrow(false)
			     .defrowlbl(false)
			     .defcollbl(false)
			     .mincolwdt(3.f*uiObject::baseFldSize())
			     .maxcolwdt(4.f*uiObject::baseFldSize())
			     .minrowhgt(1.5)
			     .maxrowhgt(1.8), "")
{
    setNrCols( 1 );
    setStretch(2,2);
    showGrid(false);
    setTopHeaderHidden(true);
    setLeftHeaderHidden(true);
    setBackgroundColor( this->backgroundColor() );
    showOuterFrame( false );
    setRowResizeMode( uiTable::ResizeToContents );
    setColumnResizeMode( uiTable::ResizeToContents );
}

uiExternalAttribInp::~uiExternalAttribInp()
{
    detachAllNotifiers();
    delete extproc_;
}

void uiExternalAttribInp::clearUI()
{
    detachAllNotifiers();
    clearTable();
    setNrRows(0);
}

void uiExternalAttribInp::makeUI(const Attrib::DescSet* ads, bool is2d)
{
    clearUI();
    makeInputUI(ads, is2d);
    makeOutputUI();
    makeStepoutUI(is2d);
    makeZSamplingUI();
    makeNewUI();
    makeLegacyUI();
}

void uiExternalAttribInp::addField(const char* key, uiGenInput* field)
{
    field->display(true);
    fields_[key] = field;
    insertRows(nrRows(), 1);
    field->preFinalize().trigger();
    setCellGroup( RowCol(nrRows()-1,0), field );
}

void uiExternalAttribInp::makeNewUI()
{
    if (!extproc_)
	return;

    BufferStringSet keys = extproc_->paramKeys();
    fields_.clear();
    for (auto* key : keys) {
	BufferString keytype = extproc_->getParamType(key->str());
	uiGenInput* field;
	if (keytype=="Number") {
	    field = new uiGenInput(nullptr, toUiString(key->str()), FloatInpSpec());
	    field->setValue(extproc_->getParamFValue(key->str()));
	} else if (keytype=="Text") {
	    field = new uiGenInput(nullptr, toUiString(key->str()), StringInpSpec());
	    field->setText(extproc_->getParamStrValue(key->str()));
	} else if (keytype=="File") {
	    BufferString mode = extproc_->getParamStrValue( key->str(), "Mode" );
	    uiFileInput::Setup su;
	    su.defseldir("");
	    su.forread( mode=="FileOut" ? false : true);
	    su.directories( mode=="Dir" ? true : false );
	    uiFileInput* filefld = new uiFileInput(0, toUiString(key->str()), su);
	    FilePath fp(extproc_->getParamStrValue(key->str()));
	    if (fp.isAbsolute())
		filefld->setDefaultSelectionDir(fp.pathOnly());
	    else {
		FilePath wfp(GetDataDir(), fp.pathOnly());
		filefld->setDefaultSelectionDir(wfp.fullPath());
	    }
	    if (fp.fileName().contains('*'))
		filefld->setFilter(fp.fileName());
	    else if (!fp.fileName().isEmpty())
		filefld->setFileName(fp.fileName());
	    field = filefld;
	} else if (keytype=="Select") {
	    BufferStringSet options = extproc_->getParamStrLstValue(key->str(),"Options");
	    field = new uiGenInput(nullptr, toUiString(key->str()), StringListInpSpec(options));
	    field->setText(extproc_->getParamStrValue(key->str()));
	    field->setTitleText(toUiString(key->str()));
	} else {
	    BufferString msg("Unknown Keytype: ");
	    msg.add(keytype);
	    ErrMsg(msg);
	    continue;
	}
	addField(key->str(), field);
    }
}

bool uiExternalAttribInp::setNewParams(const BufferString& encodedstr)
{
//Decode
    if (encodedstr.isEmpty())
	return true;

    if (extproc_ && extproc_->setParamsEncodedStr(encodedstr)) {
	for (const auto& kv : fields_) {
	    const BufferString key(kv.first.c_str());
	    uiGenInput* field = kv.second;
	    const BufferString keytype = extproc_->getParamType(key);
	    if (field && keytype=="Number")
		field->setValue(extproc_->getParamFValue(key));
	    else if (field && keytype=="Text")
		field->setText(extproc_->getParamStrValue(key));
	    else if (field && keytype=="File") {
		mDynamicCastGet(uiFileInput*,filefld,field);
		FilePath fp(extproc_->getParamStrValue(key));
		if (fp.isAbsolute())
		    filefld->setDefaultSelectionDir(fp.pathOnly());
		else {
		    FilePath wfp(GetDataDir(), fp.pathOnly());
		    filefld->setDefaultSelectionDir(wfp.fullPath());
		}
		if (fp.fileName().contains('*'))
		    filefld->setFilter(fp.fileName());
		else if (!fp.fileName().isEmpty())
		    filefld->setFileName(fp.fileName());
	    } else if (field && keytype=="Select") {
		field->setText(extproc_->getParamStrValue(key));
	    }

	}
	return true;
    }
    return false;
}

BufferString uiExternalAttribInp::getNewParams(Attrib::Desc& desc, ChangeTracker& chtr_)
{
    if (!extproc_)
	return BufferString::empty();

    for (const auto& kv : fields_) {
	const BufferString key(kv.first.c_str());
	uiGenInput* field = kv.second;
	const BufferString keytype = extproc_->getParamType(key);
	if (field && keytype=="Number")
	    extproc_->setParamFValue(key, field->getFValue());
	if (field && keytype=="Text")
	    extproc_->setParamStrValue(key, field->text());
	else if (field && keytype=="File") {
	    mDynamicCastGet(uiFileInput*,filefld,field);
	    extproc_->setParamStrValue(key, filefld->fileName());
	} else if (field && keytype=="Select")
	    extproc_->setParamStrValue(key, field->text());
    }
    return extproc_->getParamsEncodedStr();
}

void uiExternalAttribInp::makeInputUI(const Attrib::DescSet* ads,
				      bool is2d)
{
    inpflds_.erase();
    const uiAttrSelData asd( is2d, false );
    if (!extproc_)
	return;

    BufferStringSet inputs = extproc_->getInputNames();
    for (auto* input : inputs) {
	uiAttrSel* tmp = new uiAttrSel( 0, input->str(), asd );
	tmp->setDescSet( ads );
	tmp->setBorder(0);
	tmp->display(true);
	inpflds_ += tmp;
	tmp->preFinalize().trigger();
	insertRows(nrRows(), 1);
	setCellGroup( RowCol(nrRows()-1,0), tmp );
	setColor( RowCol(nrRows()-1,0), parent()->backgroundColor() );
    }
}

void uiExternalAttribInp::makeOutputUI()
{
    if (!extproc_)
	return;

    BufferStringSet outputs = extproc_->getOutputNames();
    if (outputs.isEmpty())
	return;
    outputfld_ = new uiGenInput( 0, uiStrings::sOutput(), StringListInpSpec(outputs) );
    outputfld_->display(true);
    outputfld_->preFinalize().trigger();
    insertRows(nrRows(), 1);
    setCellGroup( RowCol(nrRows()-1,0), outputfld_ );
}

void uiExternalAttribInp::makeZSamplingUI()
{
    if (!extproc_ || !extproc_->hasZMargin() || extproc_->hideZMargin())
	return;

    zmarginfld_ = new uiGenInput(nullptr,
				 tr("Z Window (samples)"),
				 IntInpIntervalSpec().setName("Samples after",1)
				 .setName("Samples before",0));
    mAttachCB(zmarginfld_->valuechanging, uiExternalAttribInp::doZmarginCheck);

    Interval<int> val = extproc_->zmargin();
    val.start = -val.start;
    zmarginfld_->setValue(val);
    zmarginfld_->display(true);
    zmarginfld_->preFinalize().trigger();
    if (extproc_->zSymmetric()) {
	zmarginfld_->displayField(false, 1, 0);
	zmarginfld_->setTitleText(tr("Z Window (+/-samples)"));
    } else {
	zmarginfld_->displayField(true, 1, 0);
	zmarginfld_->setTitleText(tr("Z Window (samples)"));
    }
    insertRows(nrRows(), 1);
    setCellGroup( RowCol(nrRows()-1,0), zmarginfld_ );
}

void uiExternalAttribInp::makeStepoutUI(bool is2d)
{
    if (!extproc_ ||!extproc_->hasStepOut() || extproc_->hideStepOut())
	return;

    stepoutfld_ = new uiExt_StepOutSel(nullptr, is2d||extproc_->so_same());
    mAttachCB(stepoutfld_->valueChanging,uiExternalAttribInp::doStepOutCheck);
    stepoutfld_->display(true);
    stepoutfld_->setBinID(extproc_->stepout());
//    stepoutfld_->displayFld2(!extproc_->so_same());

    insertRows(nrRows(), 1);
    setCellGroup( RowCol(nrRows()-1,0), stepoutfld_ );
}

void uiExternalAttribInp::makeLegacyUI()
{
    if (!extproc_)
	return;

    if (extproc_->hasSelect()) {
	int nsel = extproc_->numSelect();
	BufferStringSet nms;
	for (int i = 0; i<nsel; i++) {
	    BufferString nm = extproc_->selectOpt(i);
	    nms.add(nm.buf());
	}
	selectfld_ = new uiGenInput( 0, tr("Selection"), StringListInpSpec(nms) );
	selectfld_->setTitleText(toUiString(extproc_->selectName()));
	selectfld_->setValue(extproc_->selectValue());
	selectfld_->display(true);
	insertRows(nrRows(), 1);
	selectfld_->preFinalize().trigger();
	setCellGroup( RowCol(nrRows()-1,0), selectfld_ );
    }

    parflds_.erase();
    for (int i=0; i<cNrParams; i++) {
	if (extproc_->hasParam(i)) {
	    uiGenInput* tmp = new uiGenInput( 0, toUiString(extproc_->paramName(i)), FloatInpSpec() );
	    tmp->setTitleText(toUiString(extproc_->paramName(i)));
	    tmp->setValue(extproc_->paramValue(i));
	    tmp->display(true);
	    parflds_ += tmp;
	    insertRows(nrRows(), 1);
	    tmp->preFinalize().trigger();
	    setCellGroup(RowCol(nrRows()-1,0), tmp);
	}
    }

}

void uiExternalAttribInp::setExtProc(ExtProc* extproc)
{
    if (extproc && extproc_)
	delete extproc_;
    extproc_ = extproc;
}

const ExtProc* uiExternalAttribInp::getExtProc()
{
    return extproc_;
}

void uiExternalAttribInp::doZmarginCheck( CallBacker* cb )
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

void uiExternalAttribInp::doStepOutCheck( CallBacker* cb )
{
	if (!extproc_)
	    return;

	BinID val = stepoutfld_->getBinID();
	if ( extproc_->so_same() )
	    val.inl() = val.crl();

	if (extproc_->hasStepOut()) {
		BinID minval = extproc_->so_minimum();
		int inl = (val.inl() < minval.inl())? minval.inl():val.inl();
		int crl = (val.crl() < minval.crl())? minval.crl():val.crl();
		stepoutfld_->setBinID(BinID(inl,crl));
	}
}

bool uiExternalAttribInp::setParameters(const Attrib::Desc& desc)
{
    if (!extproc_)
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

    mIfGetString(ExternalAttrib::encodedParStr(),encodedstr, setNewParams(encodedstr));
    return true;
}

bool uiExternalAttribInp::getParameters(Attrib::Desc& desc, ChangeTracker& chtr_)
{
    if (!extproc_) {
	ErrMsg("uiExternalAttribInp::getParameters - extproc is nullptr");
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
	    mSetFloat( s, (parflds_[i])->getFValue() );
	}
    }

    mSetString( ExternalAttrib::encodedParStr(), getNewParams(desc, chtr_).getCStr() );
    return true;
}

bool uiExternalAttribInp::setOutput( const Attrib::Desc& desc )
{
    if (!extproc_)
	return false;

    if (extproc_->hasOutput())
	outputfld_->setValue(desc.selectedOutput());
    return true;
}

mInitAttribUI(uiExternalAttrib,ExternalAttrib,"External Attribute",wmPlugins::sKeyWMPlugins())

uiExternalAttrib::uiExternalAttrib( uiParent* p, bool is2d )
    : uiAttrDescEd(p,is2d, HelpKey("wgm", "ext"))
    , uiinp_(nullptr)
{
    uiFileInput::Setup su;
    su.defseldir("").forread(true);
    #ifdef __win__
	su.filter("*.exe");
    #endif
    interpfilefld_ = new uiFileInput( this, tr("Interpreter (optional)"), su );
    interpfilefld_->setFileName(uiWGMHelp::GetPythonInterpPath().fullPath());
    mAttachCB(interpfilefld_->valuechanged, uiExternalAttrib::exfileChanged);

    CallBack cb1 = mCB(this,uiExternalAttrib,updateinterpCB);
    refinterp_ = new uiToolButton( this, "refresh", tr("Reset default interpreter"), cb1 );
    refinterp_->attach (rightTo, interpfilefld_);
    refinterp_->display(true);

    exfilefld_ = new uiFileInput( this, tr("External File"), uiFileInput::Setup(uiFileDialog::Gen).forread( true ).defseldir(ExternalAttrib::exdir_) );
    mAttachCB(exfilefld_->valuechanged, uiExternalAttrib::exfileChanged);
    exfilefld_->attach(alignedBelow, interpfilefld_);

    CallBack cb2 = mCB(this,uiExternalAttrib,exfileRefresh);
    auto* exfileref = new uiToolButton( this, "refresh", tr("Force script reload"), cb2 );
    exfileref->attach (rightTo, exfilefld_);
    exfileref->display(true);

    CallBack cb3 = mCB(this,uiExternalAttrib,doHelp);
    help_ = new uiToolButton( this, "contexthelp", uiStrings::sHelp(), cb3 );
    help_->attach (rightTo, exfileref);
    help_->display(false);

    makeUI();
    setHAlignObj( uiinp_ );
    mAttachCB(postFinalize(), uiExternalAttrib::initGrp);
}

uiExternalAttrib::~uiExternalAttrib()
{
    detachAllNotifiers();
}

void uiExternalAttrib::initGrp( CallBacker* )
{
    updateinterpCB(nullptr);
}

void uiExternalAttrib::updateinterpCB(CallBacker*)
{
    BufferString newinterpfile = uiWGMHelp::GetPythonInterpPath().fullPath();
    if (newinterpfile!=interpfilefld_->fileName()) {
	interpfilefld_->setFileName(newinterpfile);
	exfileChanged(nullptr);
    }
}

void uiExternalAttrib::makeUI()
{
    if ( !uiinp_ ) {
	uiinp_ = new uiExternalAttribInp(this);
	uiinp_->attach(alignedBelow, exfilefld_);
    }
    const auto* extproc = uiinp_->getExtProc();
    if ( !extproc )
	return;
    if (extproc->isOK()) {
	uiinp_->makeUI(ads_, is2D());

	uiinp_->selectRow(0);
	if (uiinp_->getExtProc()->hasHelp())
	    help_->display(true);
	else
	    help_->display(false);
    } else {
	uiMSG().errorWithDetails( extproc->errMsg(),
				 tr("Error encountered running script file"));
	uiinp_->clearUI();
    }
}

void uiExternalAttrib::exfileRefresh( CallBacker* )
{
    BufferString iname(interpfilefld_->fileName());
    BufferString fname(exfilefld_->fileName());
    if (iname.isEmpty()) {
	uiMSG().message(tr("Please select an interpreter"));
	uiinp_->clearUI();
	return;
    }
    if (fname.isEmpty()) {
	uiMSG().message(tr("Please select a script file"));
	uiinp_->clearUI();
	uiinp_->setExtProc( nullptr );
	return;
    }

    uiinp_->setExtProc(new ExtProc(fname.str(), iname.str()));
    makeUI();
}

void uiExternalAttrib::exfileChanged( CallBacker* )
{
    BufferString iname(interpfilefld_->fileName());
    BufferString fname(exfilefld_->fileName());
    if (iname.isEmpty()) {
	uiMSG().message(tr("Please select an interpreter"));
	uiinp_->clearUI();
	return;
    }
    if (fname.isEmpty()) {
	uiMSG().message(tr("Please select a script file"));
	uiinp_->clearUI();
	uiinp_->setExtProc( nullptr );
	return;
    }

    const auto* xp = uiinp_->getExtProc();
    if (!xp || fname != xp->getFile() || iname != xp->getInterpStr())
	exfileRefresh(nullptr);
    else
	makeUI();
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

    mIfGetString(ExternalAttrib::interpFileStr(),interpfile, interpfilefld_->setFileName(interpfile))
    if (!File::isExecutable(interpfilefld_->fileName()))
	interpfilefld_->setFileName(uiWGMHelp::GetPythonInterpPath().fullPath());

    mIfGetString(ExternalAttrib::exFileStr(),exfile, setExFileName(exfile))
    if (!File::isReadable(exfilefld_->fileName()))
	exfilefld_->setFileName(BufferString::empty());

    exfileChanged(nullptr);
    BufferString exfilenm = exfilefld_->fileName();
    return exfilenm.isEmpty() ? false : uiinp_->setParameters(desc);
}

bool uiExternalAttrib::setInput( const Attrib::Desc& desc )
{
    const ExtProc* extproc = uiinp_->getExtProc();
    if (!extproc || !extproc->isOK())
	return false;

    const int nrflds = extproc->numInput();
    for (int i=0; i<nrflds; i++) {
	putInp( uiinp_->inpflds_[i], desc, i );
    }
    return true;
}

bool uiExternalAttrib::setOutput( const Attrib::Desc& desc )
{
    return uiinp_->setOutput(desc);
}

bool uiExternalAttrib::getParameters( Attrib::Desc& desc )
{
    if ( desc.attribName() != ExternalAttrib::attribName())
	return false;
    mSetString( ExternalAttrib::interpFileStr(), interpfilefld_->fileName() );
    mSetString( ExternalAttrib::exFileStr(), getExFileName().getCStr() );

    return uiinp_->getParameters(desc, chtr_);
}


bool uiExternalAttrib::getInput( Attrib::Desc& desc )
{
    const ExtProc* extproc = uiinp_->getExtProc();
    if (!extproc|| !extproc->isOK())
	return false;

    const int nrflds = extproc->numInput();
    for (int i=0; i<nrflds; i++) {
	uiinp_->inpflds_[i]->processInput();
	fillInp( uiinp_->inpflds_[i], desc, i );
    }
    return true;
}

bool uiExternalAttrib::getOutput( Desc& desc )
{
    const ExtProc* extproc = uiinp_->getExtProc();
    if (!extproc|| !extproc->isOK())
	return false;

    if (extproc->hasOutput())
	fillOutput( desc, uiinp_->outputfld_->getIntValue() );
    return true;
}

void uiExternalAttrib::doHelp( CallBacker* cb )
{
    const ExtProc* extproc = uiinp_->getExtProc();
    if (extproc && extproc->isOK() && extproc->hasHelp())
	uiDesktopServices::openUrl(extproc->helpValue());

}

