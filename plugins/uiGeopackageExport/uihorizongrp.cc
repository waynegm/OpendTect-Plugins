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
#include "iodir.h"
#include "iodirentry.h"
#include "ioman.h"
#include "survgeom2d.h"
#include "trckeyzsampling.h"

uiHorizonGrp::uiHorizonGrp( uiParent* p, bool has2Dhorizon, bool has3Dhorizon )
: uiDlgGroup(p, tr("Horizon")), hor2Dfld_(nullptr), lines2Dfld_(nullptr),
  hor3Dfld_(nullptr), subsel3Dfld_(nullptr)
{
    namefld_ = new uiGenInput(this, tr("Output Layer") );

    uiIOObjSel::Setup su;
    su.optional( true );

    su.seltxt( uiStrings::s2DHorizon() );
    hor2Dfld_ = new uiIOObjSel(this, EMHorizon2DTranslatorGroup::ioContext(), su);
    hor2Dfld_->attach(alignedBelow, namefld_);
    mAttachCB(hor2Dfld_->selectionDone, uiHorizonGrp::hor2Dsel);

    lines2Dfld_ = new WMLib::uiSeis2DLineSelGrp( this, OD::ChooseZeroOrMore );
    lines2Dfld_->attach( alignedBelow, hor2Dfld_ );

    hor2Dfld_->setSensitive( has2Dhorizon );
    hor2Dfld_->setChecked( false );
    lines2Dfld_->setSensitive( has2Dhorizon );

//        attrib2Dfld_ = new uiLabeledComboBox( this, uiStrings::s2D().append(uiStrings::sAttribute()) );
//        attrib2Dfld_->attach(alignedBelow, lines2Dfld_);

    su.seltxt( uiStrings::s3DHorizon() );
    hor3Dfld_ = new uiIOObjSel(this, EMHorizon3DTranslatorGroup::ioContext(), su);
    hor3Dfld_->attach(alignedBelow, lines2Dfld_);
    mAttachCB(hor3Dfld_->selectionDone, uiHorizonGrp::hor3Dsel);

    subsel3Dfld_ = new uiPosSubSel( this, uiPosSubSel::Setup(false,false) );
    subsel3Dfld_->attach( alignedBelow, hor3Dfld_ );

    hor3Dfld_->setSensitive( has3Dhorizon );
    hor3Dfld_->setChecked( false );
    subsel3Dfld_->setSensitive( has3Dhorizon );

//        attrib3Dfld_ = new uiLabeledComboBox( this, uiStrings::s3D().append(uiStrings::sAttribute()) );
//        attrib3Dfld_->attach(alignedBelow, subsel3Dfld_);

    mAttachCB(hor2Dfld_->optionalChecked, uiHorizonGrp::updateUICB);
    mAttachCB(hor3Dfld_->optionalChecked, uiHorizonGrp::updateUICB);
    mAttachCB(IOM().entryAdded, uiHorizonGrp::updateUICB);
    mAttachCB(IOM().entryRemoved, uiHorizonGrp::updateUICB);

    update();
}

uiHorizonGrp::~uiHorizonGrp()
{
    detachAllNotifiers();
}

bool uiHorizonGrp::doHorizonExport()
{
    MultiID hor2Did, hor3Did;
    getHorIds(hor2Did, hor3Did);
    if (!hor3Did.isUdf())
        return true;
    if (!hor2Did.isUdf())
        return true;
    return false;
}

const char* uiHorizonGrp::outputName()
{
    return namefld_->text(nullptr);
}

void uiHorizonGrp::getHorIds( MultiID& hor2Did, MultiID& hor3Did )
{
    hor2Did.setUdf();
    hor3Did.setUdf();
    if (hor2Dfld_->isChecked())
    {
        const IOObj* horObj = hor2Dfld_->ioobj(true);
        if (horObj)
            hor2Did = horObj->key();
    }
    if (hor3Dfld_->isChecked())
    {
        const IOObj* horObj = hor3Dfld_->ioobj(true);
        if (horObj)
            hor3Did = horObj->key();
    }
}

int uiHorizonGrp::num2DLinesChosen()
{
    if (hor2Dfld_->isChecked())
        return lines2Dfld_->nrChosen();
    else
        return 0;
}

void uiHorizonGrp::getGeoMids( TypeSet<Pos::GeomID>& geomids )
{
    geomids.erase();
    if (hor2Dfld_->isChecked())
        lines2Dfld_->getChosen(geomids);
}

void uiHorizonGrp::get3Dsel( TrcKeyZSampling& envelope )
{
    envelope.setEmpty();
    if (hor3Dfld_->isChecked())
        envelope = subsel3Dfld_->envelope();
}

const char* uiHorizonGrp::attrib2D()
{
/*
    if (attrib2Dfld_!=nullptr)
        return attrib2Dfld_->box()->text();
    return nullptr;
*/
    return "zvals";
}

const char* uiHorizonGrp::attrib3D()
{
/*
    if (attrib3Dfld_!=nullptr)
        return attrib3Dfld_->box()->text();
    return nullptr;
*/
return "zvals";
}

void uiHorizonGrp::hor2Dsel(CallBacker* )
{
    const IOObj* horObj = hor2Dfld_->ioobj(true);
    if (!horObj)
    {
        lines2Dfld_->clear();
//        attrib2Dfld_->setChildrenSensitive(false);
    }
    else
    {
        EM::IOObjInfo eminfo(horObj->key());
        if (!eminfo.isOK())
	{
            BufferString tmp("uiHorizonGrp::hor2Dsel - cannot read ");
            tmp += horObj->name();
            ErrMsg( tmp );
            return;
	}
	TypeSet<Pos::GeomID> prevmids;
        getGeoMids(prevmids);

        BufferStringSet lnms;
        TypeSet<Pos::GeomID> geomids;
        eminfo.getLineNames(lnms);
        eminfo.getGeomIDs(geomids);
        lines2Dfld_->setInput( lnms, geomids );
        lines2Dfld_->setChosen(prevmids);
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
    if (horObj)
    {
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
            tmp += msg.getString();
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

void uiHorizonGrp::update()
{
    if (hor2Dfld_->isChecked())
        hor2Dsel(nullptr);

    if (hor3Dfld_->isChecked())
        hor3Dsel(nullptr);
}

void uiHorizonGrp::updateUICB(CallBacker*)
{
    CtxtIOObj ctio2D(EMHorizon2DTranslatorGroup::ioContext());
    const IODir iodir2D( ctio2D.ctxt_.getSelKey() );
    const IODirEntryList entries2D( iodir2D, ctio2D.ctxt_ );
    bool has2Dhorizon = entries2D.size()>0;

    CtxtIOObj ctio3D(EMHorizon3DTranslatorGroup::ioContext());
    const IODir iodir3D( ctio3D.ctxt_.getSelKey() );
    const IODirEntryList entries3D( iodir3D, ctio3D.ctxt_ );
    bool has3Dhorizon = entries3D.size()>0;

    hor2Dfld_->setSensitive( has2Dhorizon );
    hor2Dfld_->updateInput();
    lines2Dfld_->setSensitive( has2Dhorizon && hor2Dfld_->isChecked() );

    hor3Dfld_->setSensitive( has3Dhorizon );
    hor3Dfld_->updateInput();
    subsel3Dfld_->setSensitive( has3Dhorizon && hor3Dfld_->isChecked() );
}
