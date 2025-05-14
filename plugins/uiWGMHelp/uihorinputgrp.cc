#include "uihorinputgrp.h"

#include "uibutton.h"
#include "uigeninput.h"
#include "uicombobox.h"
#include "uiioobjsel.h"
#include "uimsg.h"
#include "uipossubsel.h"
#include "uiioobjmanip.h"
#include "uiseis2dlineselgrp.h"
#include "bufstringset.h"
#include "ctxtioobj.h"
#include "emsurfacetr.h"
#include "emioobjinfo.h"
#include "emmanager.h"
#include "emhorizon3d.h"
#include "emhorizon2d.h"
#include "emsurfacegeometry.h"
#include "iodir.h"
#include "iodirentry.h"
#include "typeset.h"
#include "ranges.h"
#include "survinfo.h"

#include "survgeom2d.h"
#include "trckeyzsampling.h"

using namespace WMLib;

uiHorInputGrp::uiHorInputGrp( uiParent* p, bool has2Dhorizon, bool has3Dhorizon, bool show3Dsubsel )
: uiDlgGroup(p, tr("Input Data"))
{
    uiObject* lastfld = nullptr;

    if (has2Dhorizon) {
        hor2Dfld_ = new uiIOObjSel(this, EMHorizon2DTranslatorGroup::ioContext(), uiStrings::s2DHorizon());
        mAttachCB(hor2Dfld_->selectionDone, uiHorInputGrp::hor2DselCB);

        lines2Dfld_ = new WMLib::uiSeis2DLineSelGrp( this, OD::ChooseZeroOrMore );
        lines2Dfld_->attach( alignedBelow, hor2Dfld_ );

        lastfld = (uiObject*) lines2Dfld_;
    }
    if (has3Dhorizon) {
	if (has2Dhorizon) {
	    exp3D_ = new uiCheckBox(this, tr("Include 3D horizon"));
	    if (lastfld)
		exp3D_->attach(alignedBelow, lastfld);
	    exp3D_->setChecked(true);
	    mAttachCB(exp3D_->activated, uiHorInputGrp::exp3DselCB);
	    lastfld = (uiObject*) exp3D_;
	}

        hor3Dfld_ = new uiIOObjSel(this, EMHorizon3DTranslatorGroup::ioContext(), uiStrings::s3DHorizon());
        if (lastfld)
	    hor3Dfld_->attach(alignedBelow, lastfld);
        mAttachCB(hor3Dfld_->selectionDone, uiHorInputGrp::hor3DselCB);

        subsel3Dfld_ = new uiPosSubSel( this, uiPosSubSel::Setup(false,false) );
        subsel3Dfld_->attach( alignedBelow, hor3Dfld_ );
	subsel3Dfld_->display(show3Dsubsel);

    }
    mAttachCB( postFinalize(), uiHorInputGrp::initGrp );
}

uiHorInputGrp::~uiHorInputGrp()
{
    detachAllNotifiers();
}

bool uiHorInputGrp::has2Dhorizons()
{
    CtxtIOObj ctio2D(EMHorizon2DTranslatorGroup::ioContext());
    const IODir iodir2D( ctio2D.ctxt_.getSelKey() );
    const IODirEntryList entries2D( iodir2D, ctio2D.ctxt_ );
    return SI().has2D() && entries2D.size()>0;
}

bool uiHorInputGrp::has3Dhorizons()
{
    CtxtIOObj ctio3D(EMHorizon3DTranslatorGroup::ioContext());
    const IODir iodir3D( ctio3D.ctxt_.getSelKey() );
    const IODirEntryList entries3D( iodir3D, ctio3D.ctxt_ );
    return SI().has3D() && entries3D.size()>0;
}

void uiHorInputGrp::exp3DselCB(CallBacker*)
{
    if (!hor3Dfld_ || !subsel3Dfld_)
	return;

    bool use3d = true;
    if (hor2Dfld_ && exp3D_)
	use3d = exp3D_->isChecked();
    hor3Dfld_->setChildrenSensitive(use3d);
    subsel3Dfld_->setChildrenSensitive(use3d);
}

bool uiHorInputGrp::fillPar( IOPar& par ) const
{
    MultiID hor2Did, hor3Did;
    getHorIds(hor2Did, hor3Did);
    if (!hor2Did.isUdf() && lines2Dfld_ && lines2Dfld_->nrChosen()) {
        TypeSet<Pos::GeomID> mids;
        getGeoMids(mids);
        if (mids.size()>0) {
            par.set( sKey2DHorizonID(), hor2Did );
            par.set( sKey2DLineIDNr(), mids.size() );
            for (int idx=0; idx<mids.size(); idx++)
                par.set(IOPar::compKey(sKey2DLineID(), idx), mids[idx]);
        }
    }
    if (!hor3Did.isUdf() && (!hor2Dfld_ || (exp3D_ && exp3D_->isChecked()))) {
        par.set( sKey3DHorizonID(), hor3Did );
        TrcKeyZSampling tkz;
        get3Dsel(tkz);
        tkz.hsamp_.fillPar(par);
    }
    return true;
}

void uiHorInputGrp::usePar( const IOPar& par )
{
    MultiID hor2Did, hor3Did;

    hor3Did.setUdf();
    if (exp3D_)
	exp3D_->setChecked(false);
    if (par.get(sKey3DHorizonID(), hor3Did))
        if (hor3Dfld_ && subsel3Dfld_) {
            hor3Dfld_->setInput(hor3Did);
            TrcKeyZSampling tkz;
            tkz.usePar(par);
            subsel3Dfld_->setInput(tkz);
	    if (exp3D_)
		exp3D_->setChecked(true);
        }

    hor2Did.setUdf();
    if (par.get(sKey2DHorizonID(), hor2Did)) {
        if (hor2Dfld_ && lines2Dfld_) {
            int nlines = 0;
            par.get(sKey2DLineIDNr(), nlines);
            if (nlines>0) {
                TypeSet<Pos::GeomID> mids;
                for (int idx=0; idx<nlines; idx++) {
                    Pos::GeomID id;
                    if (par.get(IOPar::compKey(sKey2DLineID(),idx), id))
                        mids += id;
                }
                lines2Dfld_->setChosen(mids);
            }
        }
    }
}

void uiHorInputGrp::getHorIds( MultiID& hor2Did, MultiID& hor3Did ) const
{
    hor2Did.setUdf();
    hor3Did.setUdf();
    if (hor2Dfld_ && lines2Dfld_ && lines2Dfld_->nrChosen()) {
        const IOObj* horObj = hor2Dfld_->ioobj(true);
        if (horObj)
            hor2Did = horObj->key();
    }
    if (hor3Dfld_ && (!hor2Dfld_ || (exp3D_ && exp3D_->isChecked()))) {
        const IOObj* horObj = hor3Dfld_->ioobj(true);
        if (horObj)
            hor3Did = horObj->key();
    }
}

void uiHorInputGrp::getInputRange( Interval<int>& inlrg, Interval<int>& crlrg )
{
    MultiID hor2Did, hor3Did;
    getHorIds( hor2Did, hor3Did );
    inlrg.setUdf();
    crlrg.setUdf();

    if (!hor2Did.isUdf() && num2DLinesChosen()>0) {
        EM::IOObjInfo eminfo(hor2Did);
        if (!eminfo.isOK()) {
            ErrMsg("uiHorInputGrp::getInputRange - reading 2D horizon failed");
            return;
        }
        TypeSet<StepInterval<int>> trcRanges;
        TypeSet<Pos::GeomID> horGeomids;
        if (!eminfo.getTrcRanges(trcRanges) || !eminfo.getGeomIDs(horGeomids)){
            ErrMsg("uiHorInputGrp::getInputRange - reading 2D horizon trace ranges failed");
            return;
        }
        TypeSet<Pos::GeomID> geomids;
        getGeoMids(geomids);
        for (int idx=0; idx<geomids.size(); idx++) {
            mDynamicCastGet( const Survey::Geometry2D*, geom2d, Survey::GM().getGeometry(geomids[idx]) );
            if ( !geom2d )
                continue;
            int gidx = horGeomids.indexOf(geomids[idx]);
            StepInterval<int> trcrng = trcRanges[gidx];
            Coord pos;
            float spnr;
            geom2d->getPosByTrcNr(trcrng.start_, pos, spnr);
            BinID bin = SI().transform(pos);
            if (idx==0) {
                inlrg.start_ = bin.inl();
                inlrg.stop_ = bin.inl();
                crlrg.start_ = bin.crl();
                crlrg.stop_ = bin.crl();
            } else {
                inlrg.start_ = bin.inl()<inlrg.start_ ? bin.inl() : inlrg.start_;
                inlrg.stop_ = bin.inl()>inlrg.stop_ ? bin.inl() : inlrg.stop_;
                crlrg.start_ = bin.crl()<crlrg.start_ ? bin.crl() : crlrg.start_;
                crlrg.stop_ = bin.crl()>crlrg.stop_ ? bin.crl() : crlrg.stop_;
            }
            geom2d->getPosByTrcNr(trcrng.stop_, pos, spnr);
            bin = SI().transform(pos);
            inlrg.start_ = bin.inl()<inlrg.start_ ? bin.inl() : inlrg.start_;
            inlrg.stop_ = bin.inl()>inlrg.stop_ ? bin.inl() : inlrg.stop_;
            crlrg.start_ = bin.crl()<crlrg.start_ ? bin.crl() : crlrg.start_;
            crlrg.stop_ = bin.crl()>crlrg.stop_ ? bin.crl() : crlrg.stop_;
        }
    }

    if (!hor3Did.isUdf()) {
        EM::EMObject* obj = EM::EMM().loadIfNotFullyLoaded(hor3Did);
        if (!obj) {
            ErrMsg("uiHorInputGrp::getInputRange - loading 3D horizon failed");
            return;
        }
        obj->ref();
        mDynamicCastGet(EM::Horizon3D*,hor,obj);
        if (!hor) {
            ErrMsg("uiHorInputGrp::getInputRange - casting 3D horizon failed");
            obj->unRef();
            return;
        }
        mDynamicCastGet( const EM::RowColSurfaceGeometry*, rcGeom, &hor->geometry() );
        StepInterval<int> rR = rcGeom->rowRange();
        StepInterval<int> cR = rcGeom->colRange();
        if (inlrg.isUdf()) {
            inlrg.start_ = rR.start_;
            inlrg.stop_ = rR.stop_;
            crlrg.start_ = cR.start_;
            crlrg.stop_ = cR.stop_;
        } else {
            inlrg.include(rR);
            crlrg.include(cR);
        }
    }
}

int uiHorInputGrp::num2DLinesChosen()
{
    if (hor2Dfld_ && lines2Dfld_)
        return lines2Dfld_->nrChosen();
    else
        return 0;
}

void uiHorInputGrp::getGeoMids( TypeSet<Pos::GeomID>& geomids ) const
{
    geomids.erase();
    if (lines2Dfld_)
        lines2Dfld_->getChosen(geomids);
}

void uiHorInputGrp::get3Dsel( TrcKeyZSampling& envelope ) const
{
    envelope.setEmpty();
    if (subsel3Dfld_)
        envelope = subsel3Dfld_->envelope();
}

void uiHorInputGrp::hor2DselCB(CallBacker* )
{
    if (!hor2Dfld_ || !lines2Dfld_)
	return;

    const IOObj* horObj = hor2Dfld_->ioobj(true);
    if (!horObj) {
	lines2Dfld_->clear();
    } else {
        EM::IOObjInfo eminfo(horObj->key());
        if (!eminfo.isOK()) {
            BufferString tmp("uiHorInputGrp::hor2DselCB - cannot read ");
            tmp += horObj->name();
            ErrMsg( tmp );
            return;
        }
        TypeSet<Pos::GeomID> mids;
        getGeoMids(mids);

        BufferStringSet lnms;
        TypeSet<Pos::GeomID> geomids;
        eminfo.getLineNames(lnms);
        eminfo.getGeomIDs(geomids);
        lines2Dfld_->setInput( lnms, geomids );

        lines2Dfld_->setChosen(mids);
    }
}

void uiHorInputGrp::hor3DselCB(CallBacker*)
{
    if (!hor3Dfld_ || !subsel3Dfld_)
	return;

    const IOObj* horObj = hor3Dfld_->ioobj(true);
    if (horObj) {
        EM::IOObjInfo eminfo(horObj->key());
        if (!eminfo.isOK()) {
            BufferString tmp("uiHorInputGrp::hor3DselCB - cannot read ");
            tmp += horObj->name();
            ErrMsg( tmp );
            return;
        }
        uiString msg;
        EM::SurfaceIOData emdata;
        if (!eminfo.getSurfaceData( emdata, msg )) {
            BufferString tmp("uiHorInputGrp::hor3DselCB - cannot read surface data for ");
            tmp += horObj->name();
            tmp += " - ";
            tmp += msg.getString();
            ErrMsg( tmp );
            return;
        }
        TrcKeyZSampling cs;
        cs.hsamp_ = emdata.rg;
        subsel3Dfld_->setInput( cs );
    }
}

void uiHorInputGrp::initGrp(CallBacker*)
{
    if (hor2Dfld_)
        hor2DselCB(nullptr);
    if (hor3Dfld_) {
        hor3DselCB(nullptr);
        exp3DselCB(nullptr);
    }
}
