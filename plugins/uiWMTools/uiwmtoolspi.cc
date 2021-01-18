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
#include "oscommand.h"
#include "pythonaccess.h"
#include "survinfo.h"
#include "uimsg.h"
#include "ioman.h"
#include "uimenuhandler.h"
#include "uitreeview.h"
#include "pickset.h"
#include "picksetmgr.h"
#include "uipickpartserv.h"
#include "commontypes.h"

#include "uidehmainwin.h"
#include "uiwmpolygontreeitem.h"
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
    void 	treeToBeAddedCB(CallBacker*);
    void 	treeAddedCB(CallBacker*);
    void        setupDEHMenu();

    void        surveyChangeCB(CallBacker*);
    void	dataExtentHorizonCB(CallBacker*);
    void	convexHullCB(CallBacker*);
    void	faultPolyCB(CallBacker*);
    void	contourPolyCB(CallBacker*);
    void	treeMenuCB(CallBacker*);

    uiODMain*   		appl_;

    uidehMainWin*		dehdlg_ = 0;
    uiWMPolygonParentTreeItem*	polytreeparent_ = 0;

    MenuItem		dehitem_;
    int			dehmenuid_;

    MenuItem		convexpolyitem_;
    int 		convexpolymenuid_;
    MenuItem		faultpolyitem_;
    int 		faultpolymenuid_;
    MenuItem		contourpolyitem_;
    int			contourpolymenuid_;

};

uiWMToolsMgr::uiWMToolsMgr( uiODMain* a )
	: appl_(a)
	, dehitem_(m3Dots(tr("Create Data Extent Horizon")))
	, dehmenuid_(200)
	, convexpolyitem_(m3Dots(tr("New Convex Hull")))
	, convexpolymenuid_(201)
	, faultpolyitem_(m3Dots(tr("Create Fault Polygons/Polylines")))
	, faultpolymenuid_(202)
	, contourpolyitem_(m3Dots(tr("New Constant Z Polyline")))
	, contourpolymenuid_(203)
{
    mAttachCB( appl_->menuMgr().dTectMnuChanged, uiWMToolsMgr::updateMenu );
    mAttachCB( appl_->sceneMgr().treeToBeAdded, uiWMToolsMgr::treeToBeAddedCB );
    mAttachCB( appl_->sceneMgr().treeAdded, uiWMToolsMgr::treeAddedCB );
    mAttachCB(IOM().surveyChanged, uiWMToolsMgr::surveyChangeCB);

    dehitem_.id = dehmenuid_;
    convexpolyitem_.id = convexpolymenuid_;
    faultpolyitem_.id = faultpolymenuid_;
    contourpolyitem_.id = contourpolymenuid_;

    updateMenu(0);

}


uiWMToolsMgr::~uiWMToolsMgr()
{
    detachAllNotifiers();
}

void uiWMToolsMgr::updateMenu( CallBacker* )
{
    setupDEHMenu();
}

void uiWMToolsMgr::treeToBeAddedCB(CallBacker*)
{
    uiTreeFactorySet* tifs = appl_->sceneMgr().treeItemFactorySet();
    BufferString tmp;
    for (int i=0; i<tifs->nrFactories(); i++) {
	tmp = tifs->getFactory(i)->name();
	if (tmp.findLast("uiODPolygonTreeItemFactory")) {
	    tifs->remove(tmp);
	    tifs->addFactory( new uiWMPolygonTreeItemFactory, 8500,
			      SurveyInfo::Both2DAnd3D );
	    break;
	}
    }
}

void uiWMToolsMgr::treeAddedCB(CallBacker* cb)
{
    mCBCapsuleUnpack(int, sceneid, cb);
    uiODTreeTop* treetop = appl_->sceneMgr().getTreeItemMgr(sceneid);
    if (!treetop)
	return;

    for (int idx=0; idx<treetop->nrChildren(); idx++) {
	mDynamicCastGet(uiODHorizonParentTreeItem*,pitm,treetop->getChild(idx))
	if ( !pitm ) continue;
	pitm->newmenu_.addItem( &dehitem_ );
	mAttachCB(pitm->handleMenu, uiWMToolsMgr::treeMenuCB);
    }
    for (int idx=0; idx<treetop->nrChildren(); idx++) {
	mDynamicCastGet(uiWMPolygonParentTreeItem*,pitm,treetop->getChild(idx))
	if ( pitm )
	    polytreeparent_ = pitm;
    }
    if ( polytreeparent_ ) {
	polytreeparent_->toolsmenu_.addItem( &convexpolyitem_ );
	polytreeparent_->toolsmenu_.addItem( &contourpolyitem_ );
	polytreeparent_->toolsmenu_.addItem( &faultpolyitem_ );
	mAttachCB(polytreeparent_->handleMenu, uiWMToolsMgr::treeMenuCB);
    }
}

void uiWMToolsMgr::setupDEHMenu()
{
    uiODMenuMgr& mnumgr = appl_->menuMgr();
    uiActionSeparString dehprocstr( "Create Horizon Output" );
    uiAction* itm = mnumgr.procMnu()->findAction( dehprocstr );
    if ( !itm || !itm->getMenu() ) return;

    itm->getMenu()->insertAction( new uiAction(m3Dots(tr("Create Data Extent Horizon")),mCB(this,uiWMToolsMgr,dataExtentHorizonCB)) );
}

void uiWMToolsMgr::treeMenuCB(CallBacker* cb)
{
    mCBCapsuleUnpack(int, menuid, cb);
    if (menuid == dehmenuid_) {
	dataExtentHorizonCB(0);
    } else if (menuid == convexpolymenuid_) {
	convexHullCB(0);
    } else if (menuid == faultpolymenuid_) {
	faultPolyCB(0);
    } else if (menuid == contourpolymenuid_) {
	contourPolyCB(0);
    }
}

void uiWMToolsMgr::dataExtentHorizonCB(CallBacker* cb)
{
    if ( !dehdlg_ ) {
	dehdlg_ = new uidehMainWin( appl_ );
    }
    dehdlg_->show();
    dehdlg_->raise();
}

void uiWMToolsMgr::convexHullCB(CallBacker*)
{
    uiConvexHull convexhulldlg( appl_ );
    if ( !convexhulldlg.go() )
	return;

    Pick::Set* ps = convexhulldlg.getPolygonPickSet();
    if ( polytreeparent_ )
	polytreeparent_->addNewPolygon( ps );
}

void uiWMToolsMgr::faultPolyCB(CallBacker*)
{
    uiFaultPoly faultpolydlg( appl_ );
    if ( !faultpolydlg.go() )
	return;

    int nfaults = faultpolydlg.nrFaults();
    for ( int idx=0; idx<nfaults; idx++ ) {
	Pick::Set* ps = faultpolydlg.getPolyForFault(idx);

	if ( polytreeparent_ && ps && ps->size() )
	    polytreeparent_->addNewPolygon( ps );
    }
}

void uiWMToolsMgr::contourPolyCB(CallBacker*)
{
    uiContourPoly contourpolydlg( appl_ );
    if ( !contourpolydlg.go() )
	return;

    Pick::Set* ps = contourpolydlg.getPolygonPickSet();
    if ( polytreeparent_ ) {
	polytreeparent_->addNewPolygon( ps );
	if (polytreeparent_->lastAddedChild)
	    polytreeparent_->lastAddedChild->setProperty("Z value", contourpolydlg.getZ());
    }
}

void uiWMToolsMgr::surveyChangeCB( CallBacker* )
{
    if (dehdlg_) {
	dehdlg_->close();
	deleteAndZeroPtr(dehdlg_);
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
