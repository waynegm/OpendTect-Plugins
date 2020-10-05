#include "uihorizongrp.h"

#include "uibutton.h"
#include "uigeninput.h"
#include "uicombobox.h"
#include "uiioobjsel.h"
#include "uimsg.h"
#include "uipossubsel.h"
#include "uiioobjmanip.h"
#include "uiseis2dlineselgrp.h"
#include "bufstringset.h"
#include "emsurfacetr.h"
#include "emioobjinfo.h"
#include "survgeom2d.h"
#include "trckeyzsampling.h"

uiHorizonGrp::uiHorizonGrp( uiParent* p, bool has2Dhorizon, bool has3Dhorizon )
: uiDlgGroup(p, tr("Horizon")), exp2D_(nullptr), hor2Dfld_(nullptr), lines2Dfld_(nullptr),
  exp3D_(nullptr), hor3Dfld_(nullptr), subsel3Dfld_(nullptr)
{
    namefld_ = new uiGenInput(this, tr("Output Layer") );
    uiObject* lastfld = (uiObject*) namefld_;
    if (has2Dhorizon) {
        exp2D_ = new uiCheckBox(this, uiStrings::phrExport(uiStrings::s2DHorizon()));
        exp2D_->attach(alignedBelow, namefld_);
        exp2D_->setChecked(false);
        exp2D_->activated.notify(mCB(this, uiHorizonGrp, exp2Dsel));
        
        hor2Dfld_ = new uiIOObjSel(this, EMHorizon2DTranslatorGroup::ioContext(), uiStrings::s2DHorizon());
        hor2Dfld_->attach(alignedBelow, exp2D_);
        hor2Dfld_->selectionDone.notify( mCB(this, uiHorizonGrp, hor2Dsel));
        
        lines2Dfld_ = new WMLib::uiSeis2DLineSelGrp( this, OD::ChooseZeroOrMore );
        lines2Dfld_->attach( alignedBelow, hor2Dfld_ );
        
//        attrib2Dfld_ = new uiLabeledComboBox( this, uiStrings::s2D().append(uiStrings::sAttribute()) );
//        attrib2Dfld_->attach(alignedBelow, lines2Dfld_);

        hor2Dsel(0);
        exp2Dsel(0);
//        lastfld = (uiObject*) attrib2Dfld_;
        lastfld = (uiObject*) lines2Dfld_;
    }
    if (has3Dhorizon) {
	exp3D_ = new uiCheckBox(this, uiStrings::phrExport(uiStrings::s3DHorizon()));
        exp3D_->attach(alignedBelow, lastfld);
        exp3D_->setChecked(false);
        exp3D_->activated.notify(mCB(this, uiHorizonGrp, exp3Dsel));

	hor3Dfld_ = new uiIOObjSel(this, EMHorizon3DTranslatorGroup::ioContext(), uiStrings::s3DHorizon());
        hor3Dfld_->attach(alignedBelow, exp3D_);
        hor3Dfld_->selectionDone.notify( mCB(this, uiHorizonGrp, hor3Dsel));
        
        subsel3Dfld_ = new uiPosSubSel( this, uiPosSubSel::Setup(false,false) );
        subsel3Dfld_->attach( alignedBelow, hor3Dfld_ );

//        attrib3Dfld_ = new uiLabeledComboBox( this, uiStrings::s3D().append(uiStrings::sAttribute()) );
//        attrib3Dfld_->attach(alignedBelow, subsel3Dfld_);
        hor3Dsel(0);
        exp3Dsel(0);
    }
}

bool uiHorizonGrp::doHorizonExport()
{
    MultiID hor2Did, hor3Did;
    getHorIds(hor2Did, hor3Did);
    if (!hor3Did.isUdf() && exp3D_->isChecked())
        return true;
    if (!hor2Did.isUdf() && exp2D_->isChecked())
        return true;
    return false;
}

const char* uiHorizonGrp::outputName()
{
    return namefld_->text("");
}

void uiHorizonGrp::getHorIds( MultiID& hor2Did, MultiID& hor3Did )
{
    hor2Did.setUdf();
    hor3Did.setUdf();
    if (hor2Dfld_!=nullptr && exp2D_->isChecked()) {
        const IOObj* horObj = hor2Dfld_->ioobj(true);
        if (horObj!=nullptr)
            hor2Did = horObj->key();
    }
    if (hor3Dfld_!=nullptr && exp3D_->isChecked()) {
        const IOObj* horObj = hor3Dfld_->ioobj(true);
        if (horObj!=nullptr)
            hor3Did = horObj->key();
    }
}

int uiHorizonGrp::num2DLinesChosen()
{
    if (hor2Dfld_!=nullptr && exp2D_->isChecked() && lines2Dfld_!=nullptr)
        return lines2Dfld_->nrChosen();
    else
        return 0;
}

void uiHorizonGrp::getGeoMids( TypeSet<Pos::GeomID>& geomids )
{
    geomids.erase();
    if (lines2Dfld_!=nullptr)
        lines2Dfld_->getChosen(geomids);
}

void uiHorizonGrp::get3Dsel( TrcKeyZSampling& envelope )
{
    envelope.setEmpty();
    if (subsel3Dfld_!=nullptr)
        envelope = subsel3Dfld_->envelope();
}

const char* uiHorizonGrp::attrib2D()
{
/*
    if (attrib2Dfld_!=nullptr)
        return attrib2Dfld_->box()->text();
    return nullptr;
*/
    return "Z values";
}

const char* uiHorizonGrp::attrib3D()
{
/*
    if (attrib3Dfld_!=nullptr)
        return attrib3Dfld_->box()->text();
    return nullptr;
*/
return "Z values";
}

void uiHorizonGrp::update()
{
    if (hor2Dfld_!=nullptr) {
        hor2Dsel(0);
        exp2Dsel(0);
    }
    if (hor3Dfld_!=nullptr) {
        hor3Dsel(0);
        exp3Dsel(0);
    }
}

void uiHorizonGrp::hor2Dsel(CallBacker* )
{
    const IOObj* horObj = hor2Dfld_->ioobj(true);
    if (horObj==nullptr) {
        lines2Dfld_->clear();
//        attrib2Dfld_->setChildrenSensitive(false);
    } else {
        EM::IOObjInfo eminfo(horObj->key());
        if (!eminfo.isOK()) {
            BufferString tmp("uiHorizonGrp::hor2Dsel - cannot read ");
            tmp += horObj->name();
            ErrMsg( tmp );
            return;
        }
        BufferStringSet lnms;
        TypeSet<Pos::GeomID> geomids; 
        eminfo.getLineNames(lnms);
        eminfo.getGeomIDs(geomids);
        lines2Dfld_->setInput( lnms, geomids );
/*        
        uiString msg;
        EM::SurfaceIOData emdata;
        if (!eminfo.getSurfaceData( emdata, msg )) {
            BufferString tmp("uiHorizonGrp::hor2Dsel - cannot read surface data for ");
            tmp += horObj->name();
            tmp += " - ";
            tmp += msg.getOriginalString();
            ErrMsg( tmp );
            return;
        }
        attrib2Dfld_->box()->setEmpty();
        attrib2Dfld_->box()->addItem( tr("Z values") );
        if (emdata.valnames.isEmpty() )
            attrib2Dfld_->setChildrenSensitive( false );
        else {
            attrib2Dfld_->box()->addItems(emdata.valnames);
            attrib2Dfld_->setChildrenSensitive(true);
        }
*/
    }
}

void uiHorizonGrp::hor3Dsel(CallBacker*)
{
    const IOObj* horObj = hor3Dfld_->ioobj(true);
    if (horObj==nullptr) {
//        attrib3Dfld_->setChildrenSensitive(false);
    } else {
        EM::IOObjInfo eminfo(horObj->key());
        if (!eminfo.isOK()) {
            BufferString tmp("uiHorizonGrp::hor3Dsel - cannot read ");
            tmp += horObj->name();
            ErrMsg( tmp );
            return;
        }
        uiString msg;
        EM::SurfaceIOData emdata;
        if (!eminfo.getSurfaceData( emdata, msg )) {
            BufferString tmp("uiHorizonGrp::hor3Dsel - cannot read surface data for ");
            tmp += horObj->name();
            tmp += " - ";
            tmp += msg.getOriginalString();
            ErrMsg( tmp );
            return;
        }
        TrcKeyZSampling cs;
        cs.hsamp_ = emdata.rg;
        subsel3Dfld_->setInput( cs );
        
/*
        attrib3Dfld_->box()->setEmpty();
        attrib3Dfld_->box()->addItem( tr("Z values") );
        if (emdata.valnames.isEmpty() )
            attrib3Dfld_->setChildrenSensitive( false );
        else {
            attrib3Dfld_->box()->addItems(emdata.valnames);
            attrib3Dfld_->setChildrenSensitive(true);
        }
*/
    }
}

void uiHorizonGrp::exp2Dsel(CallBacker*)
{
    hor2Dfld_->setChildrenSensitive(exp2D_->isChecked());
    lines2Dfld_->setChildrenSensitive(exp2D_->isChecked());
//    attrib2Dfld_->setChildrenSensitive(exp2D_->isChecked());
}

void uiHorizonGrp::exp3Dsel(CallBacker*)
{
    hor3Dfld_->setChildrenSensitive(exp3D_->isChecked());
    subsel3Dfld_->setChildrenSensitive(exp3D_->isChecked());
//    attrib3Dfld_->setChildrenSensitive(exp3D_->isChecked());
    
}
