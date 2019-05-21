#include "uimistieapplier.h"
#include "mistieapplier.h"

#include "attribdesc.h"
#include "attribdescset.h"
#include "attribparam.h"
#include "uiattribfactory.h"
#include "uiattrsel.h"
#include "uibuttongroup.h"
#include "uibutton.h"
#include "uifileinput.h"
#include "uilabel.h"
#include "oddirs.h"
#include "filepath.h"

using namespace Attrib;

mInitAttribUI(uiMistieApplier,MistieApplier,"Mistie Application",sKeyBasicGrp())

uiMistieApplier::uiMistieApplier( uiParent* p, bool is2d )
    : uiAttrDescEd(p,is2d, HelpKey("wgm", "mistie" ))
{
    inpfld_ = createInpFld( is2d );
    
    BufferString defseldir = FilePath(GetDataDir()).add("Misc").fullPath();
    mistiefilefld_ = new uiFileInput( this, tr("Mistie Database"), uiFileInput::Setup(uiFileDialog::Gen)
                                                                    .forread(true)
                                                                    .defseldir(defseldir)
                                                                    .filter("mtd") );
    mistiefilefld_->attach(alignedBelow, inpfld_ );
    
    actiongrp_ = new uiButtonGroup( this, "", OD::Horizontal );
    actiongrp_->setExclusive( false );
    actiongrp_->attach( alignedBelow, mistiefilefld_ );
    shiftbut_ = new uiCheckBox( actiongrp_, tr("Timeshift") );
    phasebut_ = new uiCheckBox( actiongrp_, tr("Phase") );
    ampbut_ = new uiCheckBox( actiongrp_, tr("Amplitude") );
    uiLabel* lbl = new uiLabel( this, tr("Apply Corrections for") );
    lbl->attach( centeredLeftOf, actiongrp_ );
    
    setHAlignObj( inpfld_ );
}

bool uiMistieApplier::setParameters( const Desc& desc )
{
    if ( desc.attribName() != MistieApplier::attribName() )
        return false;
    
    mIfGetString(MistieApplier::mistieFileStr(),mistiefile, mistiefilefld_->setFileName(mistiefile) )
    mIfGetBool( MistieApplier::shiftStr(), shift, shiftbut_->setChecked(shift) );
    mIfGetBool( MistieApplier::phaseStr(), phase, phasebut_->setChecked(phase) );
    mIfGetBool( MistieApplier::ampStr(), amp, ampbut_->setChecked(amp) );
    
    return true;
}

bool uiMistieApplier::setInput( const Desc& desc )
{
    putInp( inpfld_, desc, 0 );
    
    return true;
}

bool uiMistieApplier::getParameters( Desc& desc )
{
    if ( desc.attribName() != MistieApplier::attribName() )
        return false;

    mSetString( MistieApplier::mistieFileStr(), mistiefilefld_->fileName() );
    mSetBool( MistieApplier::shiftStr(), shiftbut_->isChecked() );
    mSetBool( MistieApplier::phaseStr(), phasebut_->isChecked() );
    mSetBool( MistieApplier::ampStr(), ampbut_->isChecked() );

    return true;
}

bool uiMistieApplier::getInput( Desc& desc )
{
    fillInp( inpfld_, desc, 0 );
    return true;
}

