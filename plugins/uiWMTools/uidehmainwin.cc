#include "uidehmainwin.h"
/*
 *   Data extent horizon generator class
 *   Copyright (C) 2019  Wayne Mogg
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

#include "uigeninput.h"
#include "uiseparator.h"
#include "uilabel.h"
#include "bufstringset.h"
#include "survinfo.h"
#include "seisioobjinfo.h"
#include "uibutton.h"
#include "uimsg.h"
#include "ioobj.h"
#include "emmanager.h"
#include "emioobjinfo.h"
#include "emhorizon3d.h"
#include "uiiosurface.h"
#include "survgeom2d.h"
#include "uitaskrunner.h"
#include "uiodmain.h"
#include "uiodscenemgr.h"

#include "uiseis2dlineselgrp.h"
#include "ui3drangegrp.h"



uidehMainWin::uidehMainWin( uiParent* p )
    : uiDialog(p,uiDialog::Setup(getCaptionStr(),mNoDlgTitle,HelpKey("wgm","deh")).modal(false))
    , lineselfld_(nullptr)
    , include3Dfld_(nullptr)
{
    setCtrlStyle( OkAndCancel );
    setOkText( tr("Create") );
    setShrinkAllowed(true);

    uiString lbl = tr("Z Value ");
    lbl.append( SI().getUiZUnitString() );
    zfld_ = new uiGenInput( this, lbl, FloatInpSpec(SI().zRange(true).start*SI().zDomain().userFactor()) );

    uiObject* lastfld = (uiObject*) zfld_;
    uiSeparator* horsepxt = new uiSeparator(this, "Hor Sep Data Extent");
    horsepxt->attach(stretchedBelow, zfld_);
    uiLabel* lblxt = new uiLabel(this, tr("Data Extent"));
    lblxt->attach(leftBorder);
    lblxt->attach(ensureBelow, horsepxt);

    if (SI().has2D()) {
        BufferStringSet lnms;
        TypeSet<Pos::GeomID> geomids;
        SeisIOObjInfo::getLinesWithData(lnms, geomids);
        if (lnms.size() > 0 ) {
            lineselfld_ = new WMLib::uiSeis2DLineSelGrp(this, OD::ChooseZeroOrMore);
            lineselfld_->attach( alignedBelow, zfld_ );
            lineselfld_->attach(ensureBelow, lblxt);
            lastfld = (uiObject*) lineselfld_;
            mAttachCB(lineselfld_->selectionDone, uidehMainWin::lineselCB);
        }
    }

    if (SI().has3D()) {
        include3Dfld_ = new uiCheckBox(this, tr("Include 3D survey area"));
        include3Dfld_->attach(alignedBelow, lastfld);
        include3Dfld_->attach(ensureBelow, lblxt);
        include3Dfld_->setChecked(true);
        lastfld = (uiObject*) include3Dfld_;
        mAttachCB(include3Dfld_->activated, uidehMainWin::include3DCB);
    }

    uiSeparator* outsep = new uiSeparator(this, "Output");
    outsep->attach(stretchedBelow, lastfld);
    uiLabel* outlbl = new uiLabel(this, tr("Output Horizon"));
    outlbl->attach(leftBorder);
    outlbl->attach(ensureBelow, outsep);
    uiSurfaceWrite::Setup swsu(EM::Horizon3D::typeStr(), EM::Horizon3D::userTypeStr());
    swsu.withsubsel(false);
    outfld_ = new uiSurfaceWrite(this, swsu);
    outfld_->attach(stretchedBelow, lastfld);
    outfld_->attach(ensureBelow, outlbl);
    enableSaveButton(tr("Display after create"));

    rangefld_ = new WMLib::ui3DRangeGrp(this, tr("Area Selection"), true);
    rangefld_->attach(alignedBelow, outfld_);

    include3DCB(0);
}

uidehMainWin::~uidehMainWin()
{
    detachAllNotifiers();
}

void uidehMainWin::lineselCB(CallBacker*)
{
    if (!lineselfld_)
        return;
    TypeSet<Pos::GeomID> geomids;
    lineselfld_->getChosen(geomids);
    if (geomids.size()==0) {
        TrcKeySampling hs;
        rangefld_->setTrcKeySampling(hs);
        include3DCB(nullptr);
        return;
    }
    Interval<float> inlrg, crlrg;
    inlrg.setUdf();
    crlrg.setUdf();
    for (int idx=0; idx<geomids.size(); idx++) {
        mDynamicCastGet( const Survey::Geometry2D*, geom2d, Survey::GM().getGeometry(geomids[idx]) );
        if (!geom2d)
            continue;
        const PosInfo::Line2DData& geom = geom2d->data();
        const TypeSet<PosInfo::Line2DPos>& posns = geom.positions();
        Coord pos = SI().binID2Coord().transformBackNoSnap(posns[0].coord_);
        inlrg.isUdf()? inlrg.set(pos.x,pos.x) : inlrg.include(pos.x);
        crlrg.isUdf()? crlrg.set(pos.y,pos.y) : crlrg.include(pos.y);
        pos = SI().binID2Coord().transformBackNoSnap(posns[posns.size()-1].coord_);
        inlrg.include(pos.x);
        crlrg.include(pos.y);
    }
    TrcKeySampling hs = rangefld_->getTrcKeySampling();
    hs.setInlRange(Interval<int>(Math::Floor(inlrg.start), Math::Ceil(inlrg.stop)));
    hs.setCrlRange(Interval<int>(Math::Floor(crlrg.start), Math::Ceil(crlrg.stop)));
    rangefld_->setTrcKeySampling(hs);
}

void uidehMainWin::include3DCB(CallBacker*)
{
    if (include3Dfld_ && include3Dfld_->isChecked()) {
        TrcKeySampling hs = rangefld_->getTrcKeySampling();
        hs.include(SI().sampling(false).hsamp_, false);
        rangefld_->setTrcKeySampling(hs);
    }
}

bool uidehMainWin::acceptOK( CallBacker*)
{
    float zval = zfld_->getFValue();
    if (mIsUdf(zval)) {
        uiMSG().error( tr("Z value is undefined. Please enter a valid value") );
        return false;
    }

    zval /= SI().zDomain().userFactor();
    if (!SI().zRange(false).includes(zval,false)) {
        const bool res = uiMSG().askContinue( tr("Z Value is outside survey Z range") );
        if ( !res ) return false;
    }

    EM::IOObjInfo eminfo( outfld_->selIOObj()->key() );
    if (eminfo.isOK()) {
        uiString msg = tr("Horizon: %1\nalready exists."
                      "\nDo you wish to overwrite it?").arg(eminfo.name());
        if ( !uiMSG().askOverwrite(msg) )
            return false;
    }

    RefMan<EM::Horizon3D> hor3d = EM::Horizon3D::createWithConstZ(zval, rangefld_->getTrcKeySampling());
    if (!hor3d) {
        ErrMsg("uidehMainWin::acceptOK - creation of output horizon failed");
        return false;
    }

    {
        uiTaskRunner uitr(this);
        hor3d->setMultiID(outfld_->selIOObj()->key());
        PtrMan<Executor> saver = hor3d->saver();
        if (!saver || !TaskRunner::execute(&uitr, *saver)) {
            ErrMsg("uidehMainWin::acceptOK - saving output horizon failed");
            return false;
        }
    }
    if (saveButtonChecked()) {
        int displayid = ODMainWin()->sceneMgr().getIDFromName(hor3d->name());
        if (displayid != -1)
            ODMainWin()->sceneMgr().removeTreeItem(displayid);
        ODMainWin()->sceneMgr().addEMItem(EM::EMM().getObjectID(hor3d->multiID()));
        ODMainWin()->sceneMgr().updateTrees();
    }

    return true;
}


uiString uidehMainWin::getCaptionStr() const
{
    return tr("Create Horizon Covering 2D-3D Data Extent");
}
