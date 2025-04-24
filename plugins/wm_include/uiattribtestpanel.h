#pragma once
/*Copyright (C) 2021 Wayne Mogg. All rights reserved.
 *
 * This file may be used either under the terms of:
 *
 * 1. The GNU General Public License version 3 or higher, as published by
 * the Free Software Foundation, or
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "iopar.h"
#include "uiattribpanel.h"
#include "attribdesc.h"
#include "attribdescset.h"
#include "attribfactory.h"
#include "flatposdata.h"
#include "flatview.h"
#include "position.h"
#include "survinfo.h"
#include "trckeyzsampling.h"
#include "uiattrdesced.h"
#include "uicombobox.h"
#include "uiflatviewer.h"
#include "uiflatviewmainwin.h"
#include "uiflatviewstdcontrol.h"
#include "uispinbox.h"
#include "uitrcpositiondlg.h"

namespace Attrib {
    class Desc;
    class DescSet;
};


class TestPanelAdaptor
{
public:
			TestPanelAdaptor()	{}
    virtual		~TestPanelAdaptor()	{}
    virtual void	fillTestParams(Attrib::Desc*) const = 0;
};

class uiTestPanel : public uiAttribPanel
{ mODTextTranslationClass(uiTestPanel)
public:
				uiTestPanel( uiParent* p, const char* procname,
					    const char* packname,
					    const char* panelname,
					    const uiString axisnm, bool wva )
				    : uiAttribPanel( p )
				    , procname_(procname)
				    , packname_(packname)
				    , panelname_(panelname)
				    , axisnm_(axisnm)
				    , wva_(wva)	{}

    BufferString		panelname_;

protected:
    BufferString		procname_, packname_;
    uiString 			axisnm_;
    bool			wva_;
    virtual const char*		getProcName()	{ return procname_; }
    virtual const char*		getPackName()	{ return packname_; }
    virtual const char*		getPanelName()	{ return panelname_; }
    void			createAndDisplay2DViewer(FlatDataPack*);
};

inline void uiTestPanel::createAndDisplay2DViewer( FlatDataPack* fdpack )
{
    if ( !fdpack )
	return;

    RefMan<FlatDataPack> fdp = fdpack;
    if ( flatvwin_ )
	flatvwin_->viewer().setPack(
		    wva_ ? FlatView::Viewer::WVA : FlatView::Viewer::VD, fdp );
    else
    {
	flatvwin_ = new uiFlatViewMainWin( parent_,
			uiFlatViewMainWin::Setup(toUiString(getPanelName())));
	uiFlatViewer& vwr = flatvwin_->viewer();
	vwr.setInitialSize( uiSize(400,600) );
	FlatView::Appearance& app = vwr.appearance();
	app.annot_.setAxesAnnot( true );
	app.annot_.x1_.sampling_ = fdp->posData().range(true);
	app.annot_.x2_.sampling_ = fdp->posData().range(false);
	app.annot_.x1_.name_ = axisnm_;
	if ( wva_ )
	    app.annot_.x1_.annotinint_ = true;
	app.setDarkBG( false );
	app.setGeoDefaults( true );
	app.ddpars_.show( wva_, true );
	vwr.setPack( wva_ ? FlatView::Viewer::WVA : FlatView::Viewer::VD,
		     fdp );
	flatvwin_->addControl( new uiFlatViewStdControl(vwr,
		uiFlatViewStdControl::Setup(nullptr).isvertical(true)) );
	flatvwin_->setDeleteOnClose( false );
    }

    flatvwin_->show();
}

template <class T> class uiAttribTestPanel : public CallBacker
{
public:
    uiAttribTestPanel(T&, const char*, const char*, const char*,
		      const uiString axisnm=uiStrings::sFrequency(), bool wva=false);
    ~uiAttribTestPanel();

    void				showPosDlg();
protected:
    T&					uiattrib_;
    uiTestPanel*			testpanel_;
    uiTrcPositionDlg*			posdlg_ =  nullptr;
    IOPar				prevpar_;

    MultiID				getInputMID() const;
    void				showPanelCB(CallBacker*);
    void				setPrevSel();
    void				getPrevSel();
    Attrib::DescID			createTestDesc(Attrib::DescSet*) const;
    Attrib::Desc*			createNewDescFromDP(Attrib::DescSet*) const;
    Attrib::Desc*			createNewDesc(Attrib::DescSet*,Attrib::DescID) const;
};



template<class T>
uiAttribTestPanel<T>::uiAttribTestPanel( T& uiattrib, const char* procname,
					 const char* packname,
					 const char* panelname,
					 const uiString axisnm, bool wva )
    : uiattrib_(uiattrib)
    , testpanel_(new uiTestPanel(&uiattrib, procname, packname, panelname, axisnm, wva))
{}

template<class T>
uiAttribTestPanel<T>::~uiAttribTestPanel()
{
    detachAllNotifiers();
    delete testpanel_;
}

template<class T>
MultiID uiAttribTestPanel<T>::getInputMID() const
{
    MultiID mid;
    if ( !uiattrib_.is2D() )
	return mid;

    Desc* tmpdesc = uiattrib_.ads_ ? uiattrib_.ads_->getDesc( uiattrib_.inpfld_->attribID() ) : nullptr;
    if ( !tmpdesc ) return mid;

    mid = MultiID( tmpdesc->getStoredID().buf() );
    return mid;
}

template<class T>
void uiAttribTestPanel<T>::showPosDlg()
{
    if ( posdlg_ )
    {
	mDetachCB(posdlg_->windowClosed, uiAttribTestPanel<T>::showPanelCB);
	delete posdlg_;
    }

    if ( !uiattrib_.dpfids_.size() )
    {
	MultiID mid = getInputMID();
	TrcKeyZSampling cs;
	uiattrib_.inpfld_->getRanges( cs );
	posdlg_ = new uiTrcPositionDlg( &uiattrib_, cs, uiattrib_.is2D(), mid );
    }
    else
    {
	DataPack::FullID dpfid;
	uiattrib_.getInputDPID( uiattrib_.inpfld_, dpfid );
	posdlg_ = new uiTrcPositionDlg( &uiattrib_, dpfid );
    }

    setPrevSel();
    posdlg_->show();
    mAttachCB(posdlg_->windowClosed, uiAttribTestPanel<T>::showPanelCB);
}

template<class T>
void uiAttribTestPanel<T>::getPrevSel()
{
    if ( uiattrib_.dpfids_.size() ) return; //TODO implement for dps

    prevpar_.setEmpty();
    if ( !posdlg_ )
	return;

    if ( uiattrib_.is2D() )
    {
	const char* sellnm = posdlg_->linesfld_->box()->text();
	prevpar_.set( sKey::LineName(), sellnm );
	prevpar_.set( sKey::TraceNr(), posdlg_->trcnrfld_->box()->getIntValue() );
	return;
    }

    BinID bid;
    bid.inl() = posdlg_->inlfld_->box()->getIntValue();
    bid.crl() = posdlg_->crlfld_->getIntValue();
    prevpar_.set( sKey::ID(), bid );
}

template<class T>
void uiAttribTestPanel<T>::setPrevSel()
{
    if ( uiattrib_.dpfids_.size() ) return; //TODO implement for dps
    if ( !posdlg_ )
    {
	prevpar_.setEmpty();
	return;
    }

    if ( prevpar_.isEmpty() )
	return;

    if ( uiattrib_.is2D() )
    {
	BufferString lnm;
	prevpar_.get( sKey::LineName(), lnm );
	posdlg_->linesfld_->box()->setText( lnm );
	posdlg_->linesfld_->box()->selectionChanged.trigger();
	int trcnr;
	prevpar_.get( sKey::TraceNr(), trcnr );
	posdlg_->trcnrfld_->box()->setValue( trcnr );
	return;
    }

    BinID bid;
    prevpar_.get( sKey::ID(), bid );
    posdlg_->inlfld_->box()->setValue( bid.inl() );
    posdlg_->crlfld_->setValue( bid.crl() );
}

template<class T>
void uiAttribTestPanel<T>::showPanelCB( CallBacker* )
{
    const int res = posdlg_->uiResult();
    if ( !res )
	return;

    getPrevSel();
    Attrib::DescSet* dset = uiattrib_.ads_ ? new DescSet( *uiattrib_.ads_ ) : new DescSet( uiattrib_.is2D() );
    Attrib::DescID testid = createTestDesc( dset );
    const TrcKeyZSampling cs( posdlg_->getTrcKeyZSampling() );

    testpanel_->compAndDispAttrib( dset, testid, posdlg_->getTrcKeyZSampling() );
}

template<class T>
Attrib::DescID uiAttribTestPanel<T>::createTestDesc( DescSet* dset ) const
{
    DescID inpid;
    Desc* newdesc = 0;
    if ( uiattrib_.dpfids_.size() )
	newdesc = createNewDescFromDP( dset );
    else
    {
	uiattrib_.inpfld_->processInput();
	inpid = uiattrib_.inpfld_->attribID();
	newdesc = createNewDesc( dset, inpid );
    }

    if ( !newdesc )
	return DescID::undef();

    uiattrib_.fillTestParams( newdesc );
    newdesc->updateParams();
    newdesc->setUserRef( testpanel_->panelname_ );
    return dset->addDesc( newdesc );
}

template<class T>
Attrib::Desc* uiAttribTestPanel<T>::createNewDescFromDP( Attrib::DescSet* dset ) const
{
    Desc* newdesc = PF().createDescCopy( uiattrib_.attribName() );
    newdesc->selectOutput( 0 );
    Desc* inpdesc= uiattrib_.getInputDescFromDP( uiattrib_.inpfld_ );
    inpdesc->setDescSet( dset );
    dset->addDesc( inpdesc );
    newdesc->setInput( 0, inpdesc );
    newdesc->selectOutput( 0 );
    newdesc->setHidden( true );
    BufferString usrref = "_"; usrref += inpdesc->userRef();
    newdesc->setUserRef( usrref );
    return newdesc;
}

template<class T>
Attrib::Desc* uiAttribTestPanel<T>::createNewDesc( Attrib::DescSet* descset, Attrib::DescID inpid ) const
{
    Desc* inpdesc = descset->getDesc( inpid );
    Desc* newdesc = PF().createDescCopy( uiattrib_.attribName() );
    if ( !newdesc || !inpdesc )
	return nullptr;

    newdesc->selectOutput( 0 );
    newdesc->setInput( 0, inpdesc );
    newdesc->setHidden( true );
    BufferString usrref = "_"; usrref += inpdesc->userRef();
    newdesc->setUserRef( usrref );
    return newdesc;
}
