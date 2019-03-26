#include "uigrid2d3dhorizonmainwin.h"

#include "survinfo.h"
#include "uigeninput.h"
#include "ctxtioobj.h"
#include "emsurfacetr.h"
#include "iodir.h"
#include "iodirentry.h"
#include "uihorsavefieldgrp.h"
#include "uimsg.h"
#include "ptrman.h"

#include "wmgridder2d.h"
#include "uiinputgrp.h"
#include "uigridgrp.h"



uiGrid2D3DHorizonMainWin::uiGrid2D3DHorizonMainWin( uiParent* p )
    : uiDialog(p,uiDialog::Setup(getCaptionStr(),mNoDlgTitle,HelpKey("wgm","grid2d3d")).modal(false) )
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
    
    savefldgrp_ = new uiHorSaveFieldGrp( this, nullptr, false, false );
    savefldgrp_->setSaveFieldName( "Save gridded horizon" );
    savefldgrp_->attach( stretchedBelow, tabstack_ );
    
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
    gridgrp_->fillPar( par );
    
    FixedString method = par.find( wmGridder2D::sKeyMethod() );
    PtrMan<wmGridder2D> interpolator = wmGridder2D::create( method );
    if ( !interpolator ) {
        ErrMsg("uiGrid2D3DHorizonMainWin::acceptOK - selected interpolation method not found.");
        return false;
    }
    if (!interpolator->usePar(par)) {
        ErrMsg("uiGrid2D3DHorizonMainWin::acceptOK - error in interpolation parameters.");
        return false;
    }
    BufferString tmp;
    par.putTo(tmp);
    uiMSG().message(tmp);
        
    return true;
}


uiString uiGrid2D3DHorizonMainWin::getCaptionStr() const
{
    return tr("Grid 2D-3D Horizon");
}
