#include "uiseis2dlineselgrp.h"

#include "bufstringset.h"
#include "multiid.h"
#include "ctxtioobj.h"
#include "uilistbox.h"
#include "uilistboxfilter.h"
#include "uilistboxchoiceio.h"
#include "seisioobjinfo.h"
#include "uiobjbody.h"
#include "uimsg.h"

WMLib::uiSeis2DLineSelGrp::uiSeis2DLineSelGrp( uiParent* p, OD::ChoiceMode cm )
    : uiGroup(p, "Linechooser")
    , selectionDone(this)
{
    init(cm);
    listfld_->setName("Line(s)");
    BufferStringSet lnms;
    TypeSet<Pos::GeomID> geomid;
    SeisIOObjInfo::getLinesWithData( lnms, geomid );
    setInput(lnms, geomid);
    mAttachCB(listfld_->selectionChanged, uiSeis2DLineSelGrp::selChgCB);
}

WMLib::uiSeis2DLineSelGrp::uiSeis2DLineSelGrp( uiParent* p, OD::ChoiceMode cm, const BufferStringSet& lnms,const TypeSet<Pos::GeomID>& geomids)
    : uiGroup( p, "Line chooser" )
    , selectionDone(this)
{
    init(cm);
    listfld_->setName("Line(s)");
    setInput( lnms, geomids );
    mAttachCB(listfld_->selectionChanged, uiSeis2DLineSelGrp::selChgCB);
}

WMLib::uiSeis2DLineSelGrp::~uiSeis2DLineSelGrp()
{
    detachAllNotifiers();
    delete lbchoiceio_;
}

void WMLib::uiSeis2DLineSelGrp::init( OD::ChoiceMode cm )
{
    listfld_ = new uiListBox(this, "Lines", cm);
    listfld_->setHSzPol(uiObject::MedVar);
    filtfld_ = new uiListBoxFilter( *listfld_ );
    filtfld_->setItems( lnms_ );
    if ( isMultiChoice(cm) ) {
        lbchoiceio_ = new uiListBoxChoiceIO( *listfld_, "Geometry" );
        mAttachCB(lbchoiceio_->readDone, uiSeis2DLineSelGrp::readChoiceDone);
        mAttachCB(lbchoiceio_->storeRequested, uiSeis2DLineSelGrp::writeChoiceReq);
    }
    setHAlignObj( listfld_ );
}

void WMLib::uiSeis2DLineSelGrp::setInput( const BufferStringSet& lnms, const TypeSet<Pos::GeomID>& geomid )
{
    clear();
    const int* idxs = lnms.getSortIndexes( false );
    if ( !idxs ) {
        lnms_ = lnms;
        geomids_ = geomid;
    } else {
        const int sz = lnms.size();
        for ( int idx=0; idx<sz; idx++ ) {
            lnms_.add( lnms[ idxs[idx] ]->buf() );
            geomids_.add( geomid[ idxs[idx] ] );
        }
    }
    filtfld_->setItems(lnms_);
}

int WMLib::uiSeis2DLineSelGrp::nrChosen() const
{
    TypeSet<int> chidxs;
    filtfld_->getChosen(chidxs);
    return chidxs.size();
}

void WMLib::uiSeis2DLineSelGrp::getChosen( TypeSet<Pos::GeomID>& chids ) const
{
    chids.setEmpty();
    TypeSet<int> chidxs;
    filtfld_->getChosen( chidxs );
    for ( int idx=0; idx<chidxs.size(); idx++ )
        chids += geomids_[ chidxs[idx] ];
}

void WMLib::uiSeis2DLineSelGrp::getChosen( BufferStringSet& chnms ) const
{
    chnms.setEmpty();
    TypeSet<int> chidxs;
    filtfld_->getChosen( chidxs );
    for ( int idx=0; idx<chidxs.size(); idx++ )
        chnms.add( lnms_.get(chidxs[idx]) );
}

void WMLib::uiSeis2DLineSelGrp::setChosen(const TypeSet<Pos::GeomID>& geomid)
{
    filtfld_->setFilter(0);
    listfld_->chooseAll(false);
    for ( int idx=0; idx<geomid.size(); idx++ ) {
        const int idxof = geomids_.indexOf( geomid[idx] );
        if ( idxof >= 0 )
            listfld_->setChosen( idxof, true );
    }
}

void WMLib::uiSeis2DLineSelGrp::setChosen(const BufferStringSet& nms)
{
    filtfld_->setFilter( 0 );
    listfld_->chooseAll( false );
    for ( int idx=0; idx<nms.size(); idx++ ) {
        const int idxof = lnms_.indexOf( nms.get(idx) );
        if ( idxof >= 0 )
            listfld_->setChosen( idxof, true );
    }
}

void WMLib::uiSeis2DLineSelGrp::chooseAll(bool yn)
{
    if ( yn )
        filtfld_->setFilter( 0 );
    listfld_->chooseAll( yn );
}



void WMLib::uiSeis2DLineSelGrp::clear()
{
    lnms_.erase();
    geomids_.erase();
    chooseAll(false);
    filtfld_->setItems(lnms_);
}

void WMLib::uiSeis2DLineSelGrp::selChgCB(CallBacker*)
{
    selectionDone.trigger();
}

void WMLib::uiSeis2DLineSelGrp::readChoiceDone( CallBacker* )
{
    TypeSet<Pos::GeomID> gids;
    for ( int idx=0; idx<lbchoiceio_->chosenKeys().size(); idx++ )
    {
        const MultiID mid(lbchoiceio_->chosenKeys().get(idx));
        gids += Pos::GeomID( mid.ID(1) );
    }
    setChosen( gids );
}


void WMLib::uiSeis2DLineSelGrp::writeChoiceReq( CallBacker* )
{
    MultiID mid = IOObjContext::getStdDirData(IOObjContext::Geom)->id_;

    lbchoiceio_->keys().setEmpty();
    for ( int idx=0; idx<listfld_->size(); idx++ )
    {
        const int idxof = lnms_.indexOf( listfld_->textOfItem(idx) );
        if ( idxof < 0 )
        { pErrMsg("Huh"); continue; }

        mid.setObjectID( geomids_[idxof].asInt() );
	lbchoiceio_->keys().add( mid );
    }
}
