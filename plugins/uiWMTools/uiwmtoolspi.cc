/*
 *   uiWMTools Plugin - miscellaneous OpendTect tools
 *  Copyright (C) 2019  Wayne Mogg
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "uiwmtoolsmod.h"

#include "uiodmain.h"
#include "uimenu.h"
#include "uiodmenumgr.h"
#include "uiodscenemgr.h"
#include "uiodstdmenu.h"
#include "uiodhortreeitem.h"
#include "uiodpicksettreeitem.h"
#include "uitoolbar.h"
#include "uistring.h"
#include "odplugin.h"
#include "survinfo.h"
#include "uimsg.h"
#include "ioman.h"
#include "uiioobj.h"
#include "uimenuhandler.h"
#include "uitreeview.h"
#include "commontypes.h"
#include "ctxtioobj.h"

#include "pickset.h"
#include "picksetmgr.h"
#include "uipickpartserv.h"
#include "picksettr.h"
#include "uicreatepicks.h"
#include "uipicksetmgr.h"

#include "uidehmainwin.h"
#include "uiconvexhull.h"
#include "uifaultpoly.h"
#include "uicontourpoly.h"
#include "uiwgmhelp.h"
#include "wmplugins.h"

template <class T> class ODPolygon;


mDefODPluginInfo(uiWMTools)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
	"uiWMTools plugin",
	wmPlugins::sKeyWMPlugins(),
	wmPlugins::sKeyWMPluginsAuthor(),
	wmPlugins::sKeyWMPluginsVersion(),
	"Miscellaneous tools to make life with OpendTect easier.") );
    return &retpi;
}

class uiWMToolsMgr :  public CallBacker
{ mODTextTranslationClass(uiWMToolsMgr)
public:
                uiWMToolsMgr(uiODMain*);
                ~uiWMToolsMgr();

    void        updateMenu(CallBacker*);
    void        setupDEHMenu();

    void        surveyChangeCB(CallBacker*);
    void	dataExtentHorizonCB(CallBacker*);
    void	convexHullCB(CallBacker*);
    void	faultPolyCB(CallBacker*);
    void	contourPolyCB(CallBacker*);

    void	addPolygon(Pick::Set*);
    void 	polygonParentMenuCB(CallBacker*);
    void	horizonParentMenuCB(CallBacker*);
    void	contourPolyChangeCB(CallBacker*);

    uiODMain*   		appl_;
    Pick::SetMgr&		psmgr_;
    uiPickSetMgr&		uipsmgr_;
    RefObjectSet<Pick::Set>	contours_;
    TypeSet<float>		contour_levels_;


    uidehMainWin*		dehdlg_ = nullptr;
    uiContourPoly*		contourdlg_ = nullptr;
    uiODPolygonParentTreeItem*	polytreeparent_ = nullptr;
};

uiWMToolsMgr::uiWMToolsMgr( uiODMain* a )
	: appl_(a)
	, psmgr_(Pick::Mgr())
	, uipsmgr_(*new uiPickSetMgr(a,psmgr_))
{
    mAttachCB( appl_->menuMgr().dTectMnuChanged, uiWMToolsMgr::updateMenu );
    mAttachCB(IOM().surveyChanged, uiWMToolsMgr::surveyChangeCB);
    mAttachCB( uiODPolygonParentTreeItem::showMenuNotifier(), uiWMToolsMgr::polygonParentMenuCB );
    mAttachCB( uiODHorizonParentTreeItem::showMenuNotifier(), uiWMToolsMgr::horizonParentMenuCB );
    mAttachCB(Pick::Mgr().locationChanged, uiWMToolsMgr::contourPolyChangeCB);

    updateMenu(nullptr);
}


uiWMToolsMgr::~uiWMToolsMgr()
{
    detachAllNotifiers();
}

void uiWMToolsMgr::updateMenu( CallBacker* )
{
    setupDEHMenu();
}

void uiWMToolsMgr::horizonParentMenuCB( CallBacker* cb )
{
    mCBCapsuleUnpack(uiMenu*,mnu,cb);
    uiMenu* wmtmnu = new uiMenu(tr("WMPlugins"));
    wmtmnu->insertAction( new uiAction(m3Dots(tr("Add Data Extent Horizon")), mCB(this,uiWMToolsMgr,dataExtentHorizonCB)) );
    mnu->addMenu( wmtmnu );
}

void uiWMToolsMgr::polygonParentMenuCB( CallBacker* cb )
{
    mCBCapsuleUnpack(uiMenu*,mnu,cb);
    uiMenu* wmtmnu = new uiMenu(tr("WMPlugins"));

    wmtmnu->insertAction( new uiAction(m3Dots(tr("Add Convex Hull")), mCB(this,uiWMToolsMgr,convexHullCB)) );
    wmtmnu->insertAction( new uiAction(m3Dots(tr("Add Fault Polygons/Polylines")), mCB(this,uiWMToolsMgr,faultPolyCB)) );
    wmtmnu->insertAction( new uiAction(m3Dots(tr("Add Constant Z Polyline")), mCB(this,uiWMToolsMgr,contourPolyCB)) );
    mnu->addMenu( wmtmnu );
    auto* pitm = sCast(uiODPolygonParentTreeItem*,cbcaps->caller);
    if ( !pitm )
	return;

    polytreeparent_ = pitm;
}

void uiWMToolsMgr::setupDEHMenu()
{
    uiODMenuMgr& mnumgr = appl_->menuMgr();
    uiActionSeparString dehprocstr( "Create Horizon Output" );
    const uiAction* itm = mnumgr.procMnu()->findAction( dehprocstr );
    if ( !itm || !itm->getMenu() ) return;

    uiMenu* prochormenu = const_cast<uiMenu*>(itm->getMenu());
    prochormenu->insertAction( new uiAction(m3Dots(tr("Create Data Extent Horizon")),mCB(this,uiWMToolsMgr,dataExtentHorizonCB)) );
}

void uiWMToolsMgr::dataExtentHorizonCB(CallBacker* cb)
{
    if ( !dehdlg_ ) {
	dehdlg_ = new uidehMainWin( appl_ );
    }
    dehdlg_->show();
    dehdlg_->raise();
}

void uiWMToolsMgr::addPolygon(Pick::Set* ps)
{
    if ( !ps || !polytreeparent_ )
	return;

    uiODDisplayTreeItem* item = new uiODPolygonTreeItem( VisID::udf(), *ps );
    polytreeparent_->addChild( item, false );
}

void uiWMToolsMgr::convexHullCB(CallBacker*)
{
    uiConvexHull convexhulldlg( appl_ );
    if ( !convexhulldlg.go() )
	return;

    RefMan<Pick::Set> ps = convexhulldlg.getPolygonPickSet();
    if ( ps && ps->size() && uipsmgr_.storeNewSet(*ps, false) )
    {
	RefMan<Pick::Set> newps = psmgr_.get( psmgr_.size()-1 );
	addPolygon( newps );
	contours_ += newps;
	contour_levels_ +=  convexhulldlg.getZ();
    }
}

void uiWMToolsMgr::faultPolyCB(CallBacker*)
{
    uiFaultPoly faultpolydlg( appl_ );
    if ( !faultpolydlg.go() )
	return;

    if (polytreeparent_)
    {
	const int nfaults = faultpolydlg.nrFaults();
	for ( int idx=0; idx<nfaults; idx++ )
	{
	    RefMan<Pick::Set> ps = faultpolydlg.getPolyForFault(idx);
	    if ( ps && ps->size() && uipsmgr_.storeNewSet(*ps, true) )
	    {
		RefMan<Pick::Set> newps = psmgr_.get( psmgr_.size()-1 );
		addPolygon( newps );
	    }
	}
    }
}

void uiWMToolsMgr::contourPolyCB(CallBacker*)
{
    uiContourPoly dlg( appl_ );
    if ( !dlg.go() )
	return;

    RefMan<Pick::Set> ps = dlg.getPolygonPickSet();
    if ( ps && 	uipsmgr_.storeNewSet(*ps, false) )
    {
	RefMan<Pick::Set> newps = psmgr_.get( psmgr_.size()-1 );
    	addPolygon( newps );
	contours_ += newps;
	contour_levels_ += dlg.getZ();
    }
}

void uiWMToolsMgr::contourPolyChangeCB(CallBacker* cb)
{
    mDynamicCastGet(Pick::SetMgr::ChangeData*,cd,cb);
    if ( !cd || !cd->set_ )
	return;

    const int idx = contours_.indexOf(cd->set_);
    if (idx==-1 || !contour_levels_.validIdx(idx))
	return;

    const_cast<Pick::Set*>(cd->set_)->setZ(cd->loc_, contour_levels_[idx]);
}

void uiWMToolsMgr::surveyChangeCB( CallBacker* )
{
    if (dehdlg_) {
	dehdlg_->close();
	deleteAndZeroPtr(dehdlg_);
    }

    if (contourdlg_) {
	contourdlg_->close();
	deleteAndZeroPtr(contourdlg_);
    }

}

mDefODInitPlugin(uiWMTools)
{
    mDefineStaticLocalObject( PtrMan<uiWMToolsMgr>, theinst_, = 0 );
    if ( theinst_ ) return 0;

    theinst_ = new uiWMToolsMgr( ODMainWin() );
    if ( !theinst_ )
        return "Cannot instantiate WMTools plugin";

     return 0;
}
