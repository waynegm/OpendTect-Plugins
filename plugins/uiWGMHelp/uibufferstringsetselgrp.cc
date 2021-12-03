#include "uibufferstringsetselgrp.h"

#include "bufstringset.h"
#include "uilistbox.h"
#include "uilistboxfilter.h"
#include "uilistboxchoiceio.h"
#include "uiobjbody.h"
#include "uimsg.h"

WMLib::uiBufferStringSetSelGrp::uiBufferStringSetSelGrp( uiParent* p, OD::ChoiceMode cm )
    : uiGroup(p, "String chooser")
    , lbchoiceio_(nullptr)
    , selectionDone(this)
{
    init(cm);
    listfld_->setName("String(s)");
    BufferStringSet lnms;
    setInput(lnms);
    mAttachCB(listfld_->selectionChanged, uiBufferStringSetSelGrp::selChgCB);
}

WMLib::uiBufferStringSetSelGrp::uiBufferStringSetSelGrp( uiParent* p, OD::ChoiceMode cm, const BufferStringSet& lnms)
    : uiGroup( p, "String chooser" )
    , lbchoiceio_(nullptr)
    , selectionDone(this)
{
    init(cm);
    listfld_->setName("String(s)");
    setInput( lnms );
    mAttachCB(listfld_->selectionChanged, uiBufferStringSetSelGrp::selChgCB);
}

WMLib::uiBufferStringSetSelGrp::~uiBufferStringSetSelGrp()
{
    detachAllNotifiers();
    delete lbchoiceio_;
}

void WMLib::uiBufferStringSetSelGrp::init( OD::ChoiceMode cm )
{
    listfld_ = new uiListBox(this, "Strings", cm);
    listfld_->setHSzPol(uiObject::MedVar);
    filtfld_ = new uiListBoxFilter( *listfld_ );
    filtfld_->setItems( lnms_ );
    if ( isMultiChoice(cm) ) {
        lbchoiceio_ = new uiListBoxChoiceIO( *listfld_, "Name" );
    }
    setHAlignObj( listfld_ );
}

void WMLib::uiBufferStringSetSelGrp::setInput( const BufferStringSet& lnms )
{
    clear();
    const int* idxs = lnms.getSortIndexes( false );
    if ( !idxs ) {
        lnms_ = lnms;
    } else {
        const int sz = lnms.size();
        for ( int idx=0; idx<sz; idx++ ) {
            lnms_.add( lnms[ idxs[idx] ]->buf() );
        }
    }
    filtfld_->setItems(lnms_);
}

int WMLib::uiBufferStringSetSelGrp::nrChosen() const
{
    TypeSet<int> chidxs;
    filtfld_->getChosen(chidxs);
    return chidxs.size();
}

void WMLib::uiBufferStringSetSelGrp::getChosen( BufferStringSet& chnms ) const
{
    chnms.setEmpty();
    TypeSet<int> chidxs;
    filtfld_->getChosen( chidxs );
    for ( int idx=0; idx<chidxs.size(); idx++ )
        chnms.add( lnms_.get(chidxs[idx]) );
}

void WMLib::uiBufferStringSetSelGrp::setChosen(const BufferStringSet& nms)
{
    filtfld_->setFilter( 0 );
    listfld_->chooseAll( false );
    for ( int idx=0; idx<nms.size(); idx++ ) {
        const int idxof = lnms_.indexOf( nms.get(idx) );
        if ( idxof >= 0 )
            listfld_->setChosen( idxof, true );
    }
}

void WMLib::uiBufferStringSetSelGrp::chooseAll(bool yn)
{
    if ( yn )
        filtfld_->setFilter( 0 );
    listfld_->chooseAll( yn );
}



void WMLib::uiBufferStringSetSelGrp::clear()
{
    lnms_.erase();
    chooseAll(false);
    filtfld_->setItems(lnms_);
}

void WMLib::uiBufferStringSetSelGrp::selChgCB(CallBacker*)
{
    selectionDone.trigger();
}

