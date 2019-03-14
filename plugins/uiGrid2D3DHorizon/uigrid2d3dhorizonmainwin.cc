#include "uigrid2d3dhorizonmainwin.h"

#include "survinfo.h"
#include "uimsg.h"
#include "uigeninput.h"
#include "uiioobjsel.h"
#include "uiseis2dlineselgrp.h"
#include "uipossubsel.h"
#include "ctxtioobj.h"
#include "picksettr.h"
#include "emsurfacetr.h"
#include "iodir.h"
#include "iodirentry.h"
#include "bufstringset.h"



uiGrid2D3DHorizonMainWin::uiGrid2D3DHorizonMainWin( uiParent* p )
    : uiDialog(p,uiDialog::Setup(getCaptionStr(),mNoDlgTitle,HelpKey("wgm","grid2d3d")).modal(false) ),
    hor2Dfld_(nullptr)
{
    setCtrlStyle( OkAndCancel );
    setOkText( tr("Grid") );
    setShrinkAllowed(true);
    
    uiStringSet scopes;
    scopes += tr("Bounding box");
    scopes += tr("Convex hull");
    scopes += tr("Polygon");
    filltypefld_ = new uiGenInput(this, tr("Scope"), StringListInpSpec(scopes));
    filltypefld_->setValue( 1 );
    filltypefld_->valuechanged.notify( mCB(this,uiGrid2D3DHorizonMainWin,scopeChgCB));
    
    {
        CtxtIOObj ctio(PickSetTranslatorGroup::ioContext());
        ctio.ctxt_.toselect_.require_.set( sKey::Type(), sKey::Polygon() );
        polyfld_ = new uiIOObjSel( this,ctio.ctxt_,uiIOObjSel::Setup(uiStrings::sEmptyString()) );
        polyfld_->setCaption( uiStrings::sEmptyString() );
        polyfld_->attach( rightOf, filltypefld_ );
        polyfld_->setHSzPol( uiObject::SmallVar );
    }
    
    PositionInpSpec::Setup setup;
    PositionInpSpec spec( setup );
    stepfld_ = new uiGenInput( this, tr("Inl/Crl Step"), spec );
    stepfld_->setValue( BinID(SI().inlStep(),SI().crlStep()) );
    stepfld_->attach( alignedBelow, filltypefld_ );
    
    uiObject* lastfld = (uiObject*) stepfld_;
    {
        CtxtIOObj ctio2D(EMHorizon2DTranslatorGroup::ioContext());
        const IODir iodir2D( ctio2D.ctxt_.getSelKey() );
        const IODirEntryList entries2D( iodir2D, ctio2D.ctxt_ );
        bool has2Dhorizon = SI().has2D() && entries2D.size()>0;
        if (has2Dhorizon) {
            hor2Dfld_ = new uiIOObjSel(this, EMHorizon2DTranslatorGroup::ioContext(), "2D Horizon");
            hor2Dfld_->attach(alignedBelow, stepfld_);
            hor2Dfld_->selectionDone.notify( mCB(this, uiGrid2D3DHorizonMainWin, hor2DselCB));

            lines2Dfld_ = new uiSeis2DLineSelGrp( this, OD::ChooseZeroOrMore );
            lines2Dfld_->attach( alignedBelow, hor2Dfld_ );
            
            lastfld = (uiObject*) lines2Dfld_;
            hor2DselCB(0);
        }
    }
    
    {
        CtxtIOObj ctio3D(EMHorizon3DTranslatorGroup::ioContext());
        const IODir iodir3D( ctio3D.ctxt_.getSelKey() );
        const IODirEntryList entries3D( iodir3D, ctio3D.ctxt_ );
        bool has3Dhorizon = SI().has3D() && entries3D.size()>0;
        if (has3Dhorizon) {
            hor3Dfld_ = new uiIOObjSel(this, EMHorizon3DTranslatorGroup::ioContext(), "3D Horizon");
            hor3Dfld_->attach(alignedBelow, lastfld);
            hor3Dfld_->selectionDone.notify( mCB(this, uiGrid2D3DHorizonMainWin, hor3DselCB));
            
            subsel3Dfld_ = new uiPosSubSel( this, uiPosSubSel::Setup(false,false) );
            subsel3Dfld_->attach( alignedBelow, hor3Dfld_ );
            
            lastfld = (uiObject*) subsel3Dfld_;
            hor3DselCB(0);
        }
    }
    
    
    scopeChgCB(0);
}

uiGrid2D3DHorizonMainWin::~uiGrid2D3DHorizonMainWin()
{
    
}

void uiGrid2D3DHorizonMainWin::scopeChgCB( CallBacker* )
{
}

void uiGrid2D3DHorizonMainWin::hor2DselCB( CallBacker* )
{
}

void uiGrid2D3DHorizonMainWin::hor3DselCB( CallBacker* )
{
}

bool uiGrid2D3DHorizonMainWin::acceptOK( CallBacker*)
{
    
    return true;
}


uiString uiGrid2D3DHorizonMainWin::getCaptionStr() const
{
    return tr("Grid 2D-3D Horizon");
}
