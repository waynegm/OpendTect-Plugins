#include "uigrid2d3dhorizonmainwin.h"

#include "survinfo.h"
#include "uigeninput.h"
#include "ctxtioobj.h"
#include "emsurfacetr.h"
#include "iodir.h"
#include "iodirentry.h"
#include "uimsg.h"
#include "ptrman.h"
#include "emhorizon3d.h"
#include "uiiosurface.h"
#include "uitaskrunner.h"

#include "wmgridder2d.h"
#include "uiinputgrp.h"
#include "uigridgrp.h"
#include "uiwmhorsavefieldgrp.h"



uiGrid2D3DHorizonMainWin::uiGrid2D3DHorizonMainWin( uiParent* p )
    : uiDialog(p,uiDialog::Setup(getCaptionStr(),mNoDlgTitle,HelpKey("wgm","grid2d3d")).modal(false))
    , ready(this)
//    , horReadyForDisplay(this)
//    , finished(this)
{
    setCtrlStyle( OkAndCancel );
    setOkText( tr("Grid") );
    setShrinkAllowed(true);
    
    tabstack_ = new uiTabStack( this, "Tab" );
    tabstack_->selChange().notify( mCB(this,uiGrid2D3DHorizonMainWin,tabSelCB) );
    tabstack_->attach(hCentered);
    uiParent* tabparent = tabstack_->tabGroup();
    
    {
        CtxtIOObj ctio2D(EMHorizon2DTranslatorGroup::ioContext());
        const IODir iodir2D( ctio2D.ctxt_.getSelKey() );
        const IODirEntryList entries2D( iodir2D, ctio2D.ctxt_ );
        bool has2Dhorizon = SI().has2D() && entries2D.size()>0;
        
        CtxtIOObj ctio3D(EMHorizon3DTranslatorGroup::ioContext());
        const IODir iodir3D( ctio3D.ctxt_.getSelKey() );
        const IODirEntryList entries3D( iodir3D, ctio3D.ctxt_ );
        bool has3Dhorizon = SI().has3D() && entries3D.size()>0;

        if (has2Dhorizon || has3Dhorizon) {
            inputgrp_ = new uiInputGrp( tabparent, has2Dhorizon, has3Dhorizon );
            tabstack_->addTab( inputgrp_ );
        }
    }
    
    gridgrp_ = new uiGridGrp( tabparent );
    tabstack_->addTab( gridgrp_ );
    
    uiSurfaceWrite::Setup swsu(EM::Horizon3D::typeStr(), EM::Horizon3D::userTypeStr());
    swsu.withsubsel(false);
    outfld_ = new uiSurfaceWrite(this, swsu);
    outfld_->attach(stretchedBelow, tabstack_);
    enableSaveButton(tr("Display after create"));
//    savefldgrp_ = new uiwmHorSaveFieldGrp( this );
//    savefldgrp_->setSaveFieldName( "Save gridded horizon" );
//    savefldgrp_->attach( stretchedBelow, tabstack_ );
    
    tabSelCB(0);
}

uiGrid2D3DHorizonMainWin::~uiGrid2D3DHorizonMainWin()
{
    
}

void uiGrid2D3DHorizonMainWin::tabSelCB( CallBacker* )
{
    if ( !tabstack_ )
        return;
    
    uiGroup* grp = tabstack_->currentPage();
    if ( !grp ) return;
    if (grp->name() == "Input Data")
        inputgrp_->update();
}

bool uiGrid2D3DHorizonMainWin::acceptOK( CallBacker*)
{
    IOPar par;
    inputgrp_->fillPar( par );
    gridgrp_->fillPar( par );
    
    FixedString method = par.find( wmGridder2D::sKeyMethod() );
    PtrMan<wmGridder2D> interpolator = wmGridder2D::create( method );
    if ( interpolator==nullptr ) {
        ErrMsg("uiGrid2D3DHorizonMainWin::acceptOK - selected interpolation method not found.");
        return false;
    }
    if (!interpolator->usePar(par)) {
        ErrMsg("uiGrid2D3DHorizonMainWin::acceptOK - error in interpolation parameters.");
        return false;
    }
//    if (!savefldgrp_->createNewHorizon())
//        return false;
//    EM::Horizon* hor = savefldgrp_->getNewHorizon();
//    mDynamicCastGet(EM::Horizon3D*,hor3d,hor)
//    if (!hor3d) {
//        ErrMsg("uiGrid2D3DHorizonMainWin::acceptOK - error creating output horizon");
//        return false;
//    }
    
    MouseCursorManager::setOverride(MouseCursor::Wait);
    if (!interpolator->grid()) {
        ErrMsg("uiGrid2D3DHorizonMainWin::acceptOK - error during gridding");
        MouseCursorManager::restoreOverride();
        return false;
    }

//    Interval<int> inlrg, crlrg;
//    interpolator->getHorRange(inlrg,crlrg);
//    if (!savefldgrp_->setHorRange(inlrg, crlrg)) {
//        ErrMsg("uiGrid2D3DHorizonMainWin::acceptOK - setting output horizon extent failed.");
//        return false;
//    }
    RefMan<EM::Horizon3D> hor3d = EM::Horizon3D::createWithConstZ(0.0, interpolator->getTrcKeySampling());
    if (!hor3d) {
        ErrMsg("uiGrid2D3DHorizonMainWin::acceptOK - creation of output horizon failed");
        return false;
    }
    hor3d->setMultiID(outfld_->selIOObj()->key());
    if (!interpolator->saveGridTo(hor3d)) {
        ErrMsg("uiGrid2D3DHorizonMainWin::acceptOK - error converting grid to horizon");
        return false;
    }
    MouseCursorManager::restoreOverride();
    
//    if (savefldgrp_->displayNewHorizon())
//        horReadyForDisplay.trigger();
//    if (!savefldgrp_->saveHorizon()) {
//        ErrMsg("uiGrid2D3DHorizonMainWin::acceptOK - error during horizon save");
//        return false;
//    }
//    finished.trigger();
//    uiMSG().message(tr("Horizon successfully gridded"));
      uiTaskRunner uitr(this);
      PtrMan<Executor> saver = hor3d->saver();
      if (!saver || !uitr.execute(*saver)) {
          ErrMsg("uiGrid2D3DHorizonMainWin::acceptOK - saving output horizon failed");
          return false;
      }
      ready.trigger();
    return true;
}


uiString uiGrid2D3DHorizonMainWin::getCaptionStr() const
{
    return tr("Grid 2D-3D Horizon");
}
