#include "uiseis2dlineselgrp.h"

#include "bufstringset.h"
#include "multiid.h"
#include "ctxtioobj.h"
#include "uilistbox.h"
#include "uilistboxfilter.h"
#include "uilistboxchoiceio.h"
#include "seisioobjinfo.h"
#include "uiobjbody.h"

uiSeis2DLineSelGrp::uiSeis2DLineSelGrp( uiParent* p, OD::ChoiceMode cm )
: uiGroup(p, "Linechooser"), lbchoiceio_(nullptr)
{
    init(cm);
    listfld_->setName("Line(s)");
    BufferStringSet lnms;
    TypeSet<Pos::GeomID> geomid;
    SeisIOObjInfo::getLinesWithData( lnms, geomid );
    setInput(lnms, geomid);
}

uiSeis2DLineSelGrp::uiSeis2DLineSelGrp( uiParent* p, OD::ChoiceMode cm, const BufferStringSet& lnms,const TypeSet<Pos::GeomID>& geomids)
: uiGroup( p, "Line chooser" ), lbchoiceio_(nullptr)
{
    init(cm);
    listfld_->setName("Line(s)");
    setInput( lnms, geomids );
}

uiSeis2DLineSelGrp::~uiSeis2DLineSelGrp()
{
    delete lbchoiceio_;
}

void uiSeis2DLineSelGrp::init( OD::ChoiceMode cm )
{
    listfld_ = new uiListBox(this, "Lines", cm);
    listfld_->setHSzPol(uiObject::MedVar);
    filtfld_ = new uiListBoxFilter( *listfld_ );
    filtfld_->setItems( lnms_ );
    if ( isMultiChoice(cm) ) {
        lbchoiceio_ = new uiListBoxChoiceIO( *listfld_, "Geometry" );
        lbchoiceio_->readDone.notify(mCB(this,uiSeis2DLineSelGrp,readChoiceDone));
        lbchoiceio_->storeRequested.notify(mCB(this,uiSeis2DLineSelGrp,writeChoiceReq));
    }
    setHAlignObj( listfld_ );
}
    
void uiSeis2DLineSelGrp::setInput( const BufferStringSet& lnms, const TypeSet<Pos::GeomID>& geomid )
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

int uiSeis2DLineSelGrp::nrChosen() const
{
    TypeSet<int> chidxs;
    filtfld_->getChosen(chidxs);
    return chidxs.size();
}

void uiSeis2DLineSelGrp::getChosen( TypeSet<Pos::GeomID>& chids ) const
{
    chids.setEmpty();
    TypeSet<int> chidxs; 
    filtfld_->getChosen( chidxs );
    for ( int idx=0; idx<chidxs.size(); idx++ )
        chids += geomids_[ chidxs[idx] ];
}

void uiSeis2DLineSelGrp::getChosen( BufferStringSet& chnms ) const
{
    chnms.setEmpty();
    TypeSet<int> chidxs;
    filtfld_->getChosen( chidxs );
    for ( int idx=0; idx<chidxs.size(); idx++ )
        chnms.add( lnms_.get(chidxs[idx]) );
}

void    uiSeis2DLineSelGrp::setChosen(const TypeSet<Pos::GeomID>& geomid)
{
    filtfld_->setFilter(0);
    listfld_->chooseAll(false);
    for ( int idx=0; idx<geomid.size(); idx++ ) {
        const int idxof = geomids_.indexOf( geomid[idx] );
        if ( idxof >= 0 )
            listfld_->setChosen( idxof, true );
    }
}

void    uiSeis2DLineSelGrp::setChosen(const BufferStringSet& nms)
{
    filtfld_->setFilter( 0 );
    listfld_->chooseAll( false );
    for ( int idx=0; idx<nms.size(); idx++ ) {
        const int idxof = lnms_.indexOf( nms.get(idx) );
        if ( idxof >= 0 )
            listfld_->setChosen( idxof, true );
    }
}

void   uiSeis2DLineSelGrp::chooseAll(bool yn)
{
    if ( yn )
        filtfld_->setFilter( 0 );
    listfld_->chooseAll( yn );
}



void uiSeis2DLineSelGrp::clear()
{
    lnms_.erase();
    geomids_.erase();
    chooseAll(false);
    filtfld_->setItems(lnms_);
}

void uiSeis2DLineSelGrp::readChoiceDone( CallBacker* )
{
    TypeSet<Pos::GeomID> gids;
    for ( int idx=0; idx<lbchoiceio_->chosenKeys().size(); idx++ )
    {
        const MultiID mid = lbchoiceio_->chosenKeys().get(idx).buf();
        gids += Pos::GeomID( mid.ID(1) );
    }
    
    setChosen( gids );
}


void uiSeis2DLineSelGrp::writeChoiceReq( CallBacker* )
{
    MultiID mid = IOObjContext::getStdDirData(IOObjContext::Geom)->id_;
    mid.add( 0 );
    
    lbchoiceio_->keys().setEmpty();
    for ( int idx=0; idx<listfld_->size(); idx++ )
    {
        const int idxof = lnms_.indexOf( listfld_->textOfItem(idx) );
        if ( idxof < 0 )
        { pErrMsg("Huh"); continue; }
        
        mid.setID( 1, geomids_[idxof] );
        lbchoiceio_->keys().add( mid.buf() );
    }
}
