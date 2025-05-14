#include "uigrid2d3dhorizonmainwin.h"

#include "uiodmain.h"
#include "uiodscenemgr.h"
#include "survinfo.h"
#include "uigeninput.h"
#include "ctxtioobj.h"
#include "emsurfacetr.h"
#include "iodir.h"
#include "filepath.h"
#include "iodirentry.h"
#include "uimsg.h"
#include "ptrman.h"
#include "emmanager.h"
#include "emhorizon3d.h"
#include "emioobjinfo.h"
#include "executor.h"
#include "uiiosurface.h"
#include "uitaskrunner.h"
#include "paralleltask.h"
#include "oddirs.h"

#include "wmgridder2d.h"
#include "uiinputgrp.h"
#include "uigridgrp.h"


uiGrid2D3DHorizonMainWin::uiGrid2D3DHorizonMainWin( uiParent* p )
    : uiDialog(p,uiDialog::Setup(getCaptionStr(),mNoDlgTitle,HelpKey("wgm","grid2d3d")).modal(false))
{
    setCtrlStyle( OkAndCancel );
    setOkText( tr("Grid") );
    setShrinkAllowed(true);

    tabstack_ = new uiTabStack( this, "Tab" );
    mAttachCB(tabstack_->selChange(), uiGrid2D3DHorizonMainWin::tabSelCB);
    tabstack_->attach(hCentered);
    uiParent* tabparent = tabstack_->tabGroup();

    {
        CtxtIOObj ctio2D(EMHorizon2DTranslatorGroup::ioContext());
        const IODir iodir2D( ctio2D.ctxt_.getSelKey() );
        const IODirEntryList entries2D( iodir2D, ctio2D.ctxt_ );
        bool has2Dhorizon = entries2D.size()>0;

        CtxtIOObj ctio3D(EMHorizon3DTranslatorGroup::ioContext());
        const IODir iodir3D( ctio3D.ctxt_.getSelKey() );
        const IODirEntryList entries3D( iodir3D, ctio3D.ctxt_ );
        bool has3Dhorizon = entries3D.size()>0;

        if (has2Dhorizon || has3Dhorizon) {
            inputgrp_ = new uiInputGrp( tabparent, has2Dhorizon, has3Dhorizon );
            tabstack_->addTab( inputgrp_, inputgrp_->getCaption() );
        }
    }

    gridgrp_ = new uiGridGrp( tabparent );
    tabstack_->addTab( gridgrp_, gridgrp_->getCaption() );

    uiSurfaceWrite::Setup swsu(EM::Horizon3D::typeStr(), EM::Horizon3D::userTypeStr());
    swsu.withsubsel(false);
    outfld_ = new uiSurfaceWrite(this, swsu);
    outfld_->attach(stretchedBelow, tabstack_);
    enableSaveButton(tr("Display after create"));

    IOPar par;
    if (par.read(getParFileName(),"grid2d3d")) {
        if (inputgrp_)
	    inputgrp_->usePar( par );
        if (gridgrp_)
	    gridgrp_->usePar( par );
    }
    tabSelCB(0);
}

uiGrid2D3DHorizonMainWin::~uiGrid2D3DHorizonMainWin()
{
    detachAllNotifiers();
}

BufferString uiGrid2D3DHorizonMainWin::getParFileName()
{
    FilePath fp(GetDataDir(), "Misc", "grid2d3d.par");
    ErrMsg(fp.fullPath());
    return fp.fullPath();
}

void uiGrid2D3DHorizonMainWin::tabSelCB( CallBacker* )
{
    if ( !tabstack_ )
        return;

    uiGroup* grp = tabstack_->currentPage();
    if ( !grp ) return;
    if (inputgrp_ && grp->name() == "Input Data")
        inputgrp_->update();
    else
        gridgrp_->update();
}

bool uiGrid2D3DHorizonMainWin::acceptOK( CallBacker*)
{
    IOPar par;
    if (inputgrp_)
	inputgrp_->fillPar( par );

    gridgrp_->fillPar( par );
    par.write(getParFileName(), "grid2d3d");

    BufferString method = par.find(IOPar::compKey(wmGridder2D::sKeyGridDef(), wmGridder2D::sKeyMethod()));
    PtrMan<wmGridder2D> interpolator = wmGridder2D::create( method );
    if ( !interpolator ) {
        ErrMsg("uiGrid2D3DHorizonMainWin::acceptOK - selected interpolation method not found.");
        return false;
    }
    if (!interpolator->usePar(par)) {
        ErrMsg("uiGrid2D3DHorizonMainWin::acceptOK - error in interpolation parameters.");
        return false;
    }

    EM::IOObjInfo eminfo( outfld_->selIOObj()->key() );
    if (eminfo.isOK()) {
        uiString msg = tr("Horizon: %1\nalready exists."
                      "\nDo you wish to overwrite it?").arg(eminfo.name());
        if ( !uiMSG().askOverwrite(msg) )
            return false;
    }

    uiUserShowWait uisw(this, tr("Loading data"));
    if (!interpolator->prepareForGridding(this))
	return false;

    interpolator->executeGridding(this);

    RefMan<EM::Horizon3D> hor3d = EM::Horizon3D::createWithConstZ(0.0, interpolator->getTrcKeySampling());
    if (!hor3d) {
        ErrMsg("uiGrid2D3DHorizonMainWin::acceptOK - creation of output horizon failed");
        return false;
    }
    if (!interpolator->saveGridTo(hor3d.ptr())) {
        ErrMsg("uiGrid2D3DHorizonMainWin::acceptOK - error converting grid to horizon");
        return false;
    }

    {
        uiTaskRunner uitr(this);
        hor3d->setMultiID(outfld_->selIOObj()->key());
        PtrMan<Executor> saver = hor3d->saver();
        if (!saver || !TaskRunner::execute(&uitr, *saver)) {
            ErrMsg("uiGrid2D3DHorizonMainWin::acceptOK - saving output horizon failed");
            return false;
        }
    }
    if (saveButtonChecked()) {
        VisID displayid = ODMainWin()->sceneMgr().getIDFromName(hor3d->name());
        if (!displayid.isUdf())
            ODMainWin()->sceneMgr().removeTreeItem(displayid);
        ODMainWin()->sceneMgr().addEMItem(EM::EMM().getObjectID(hor3d->multiID()));
        ODMainWin()->sceneMgr().updateTrees();
    }
    return true;
}


uiString uiGrid2D3DHorizonMainWin::getCaptionStr() const
{
    return tr("Grid 2D-3D Horizon");
}
