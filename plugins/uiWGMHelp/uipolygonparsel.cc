#include "uipolygonparsel.h"

#include "uigeninput.h"
#include "uibutton.h"
#include "uigroup.h"
#include "uimsg.h"
#include "ioobj.h"
#include "ioman.h"
#include "ctxtioobj.h"
#include "picksettr.h"
#include "uiioobjseldlg.h"
#include "uiioobjselgrp.h"
#include "ptrman.h"
#include "uilistbox.h"
#include "callback.h"

WMLib::uiPolygonParSel::uiPolygonParSel( uiParent* p, const uiString& caption, bool multisel )
    : uiCompoundParSel(p, caption)
    , selChange(this)
    , multisel_(multisel)
{
    butPush.notify( mCB(this, uiPolygonParSel, doDlg) );
    
    clearbut_ = new uiPushButton(this, uiStrings::sClear(), true);
    clearbut_->activated.notify( mCB(this, uiPolygonParSel, clearPush) );
    clearbut_->attach( rightOf, selbut_ );
    
    txtfld_->setElemSzPol( uiObject::Wide );
    setHAlignObj( txtfld_ );
}

WMLib::uiPolygonParSel::~uiPolygonParSel()
{
    detachAllNotifiers();
}

void WMLib::uiPolygonParSel::hideClearButton( bool yn )
{
    clearbut_->display( false, true );
}

void WMLib::uiPolygonParSel::setSelectedPolygons( const TypeSet<MultiID>& ids )
{
    selpolygonids_.erase();
    selpolygonnms_.erase();
    for ( int idx=0; idx<ids.size(); idx++ )
    {
        PtrMan<IOObj> ioobj = IOM().get( ids[idx] );
        if ( !ioobj ) continue;
        
        selpolygonnms_.add( ioobj->name() );
        selpolygonids_ += ids[idx];
    }
    updSummary(0);
    selChange.trigger();
}

void WMLib::uiPolygonParSel::clearPush( CallBacker* )
{
    selpolygonnms_.erase();
    selpolygonids_.erase();
    updSummary(0);
    selChange.trigger();
}

void WMLib::uiPolygonParSel::doDlg( CallBacker* )
{
    PtrMan<CtxtIOObj> ctio = mMkCtxtIOObj(PickSet);
    ctio->ctxt_.toselect_.require_.set( sKey::Type(), sKey::Polygon() );
    uiIOObjSelDlg::Setup sdsu( tr("Select Polygons") );
    sdsu.multisel( multisel_ );
    uiIOObjSelDlg dlg( this, sdsu, *ctio );
    dlg.selGrp()->getListField()->setChosen( selpolygonnms_ );
    if ( !dlg.go() ) return;
        
    selpolygonnms_.erase();
    selpolygonids_.erase();
    uiIOObjSelGrp* selgrp = dlg.selGrp();
    selgrp->getListField()->getChosen( selpolygonnms_ );
    for ( int idx=0; idx<selpolygonnms_.size(); idx++ )
        selpolygonids_ += selgrp->chosenID(idx);
    
    selChange.trigger();
}

void WMLib::uiPolygonParSel::setEmpty()
{ clearPush( 0 ); }

BufferString WMLib::uiPolygonParSel::getSummary() const
{
    BufferString summ;
    for ( int idx=0; idx<selpolygonnms_.size(); idx++ )
    {
        summ += selpolygonnms_.get(idx);
        summ += idx == selpolygonnms_.size()-1 ? "." : ", ";
    }
    return summ.isEmpty() ? BufferString(" - ") : summ;
}
