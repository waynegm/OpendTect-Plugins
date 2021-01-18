/*
 *   An override of the OD Polygon TreeItem class
 *   Copyright (C) 2019  Wayne Mogg
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "uiwmpolygontreeitem.h"

#include "uiodpicksettreeitem.h"
#include "uiodapplmgr.h"
#include "uiodscenemgr.h"
#include "uivispartserv.h"
#include "uipickpartserv.h"
#include "vissurvobj.h"
#include "vispicksetdisplay.h"
#include "vispolylinedisplay.h"
#include "uistrings.h"
#include "uimenu.h"
#include "uimsg.h"
#include "picksettr.h"
#include "ptrman.h"
#include "pickset.h"
#include "uiioobj.h"
#include "ioman.h"


uiWMPolygonParentTreeItem::uiWMPolygonParentTreeItem()
    : uiODPolygonParentTreeItem()
    , toolsmenu_(uiStrings::sTools())
    , handleMenu(this)
{
}

uiWMPolygonParentTreeItem::~uiWMPolygonParentTreeItem()
{
    detachAllNotifiers();
}

#define mLoadPolyIdx	11
#define mNewPolyIdx	12
#define mSavePolyIdx	13
#define mOnlyAtPolyIdx	14
#define mAlwaysPolyIdx	15

bool uiWMPolygonParentTreeItem::addNewPolygon(Pick::Set* ps, bool warnifexist)
{
    PtrMan<CtxtIOObj> ctio = mMkCtxtIOObj(PickSet);
    ctio->setName( ps->name() );
    BufferString errmsg;
    if ( uiIOObj::fillCtio(*ctio, warnifexist) )
    {
	PtrMan<IOObj> ioobj = ctio->ioobj_;
	IOM().commitChanges( *ioobj );
	if ( !PickSetTranslator::store( *ps, ioobj, errmsg ) ) {
	    uiMSG().error(tr("%1").arg(errmsg));
	    return false;
	}
	Pick::Mgr().set( ioobj->key(), ps );
	Pick::Set& polyps = Pick::Mgr().get( Pick::Mgr().size()-1 );
	addPolygon( &polyps );
	return true;
    }
    delete ps;
    ps = 0;
    return false;
}

void uiWMPolygonParentTreeItem::addPolygon(Pick::Set* ps )
{
    if ( !ps ) return;
    uiWMPolygonTreeItem* item = new uiWMPolygonTreeItem( -1, *ps );
    addChild( item, true );
    lastAddedChild = item;
}

bool uiWMPolygonParentTreeItem::showSubMenu()
{
    mDynamicCastGet(visSurvey::Scene*,scene,applMgr()->visServer()->getObject(sceneID()));
    const bool hastransform = scene && scene->getZAxisTransform();

    uiMenu mnu( getUiParent(), uiStrings::sAction() );
    mnu.insertAction( new uiAction(m3Dots(uiStrings::sAdd())), mLoadPolyIdx );
    mnu.insertAction( new uiAction(m3Dots(uiStrings::sNew())), mNewPolyIdx );
    if (toolsmenu_.nrItems()) {
	uiMenu* toolmenu = new uiMenu(toolsmenu_);
	mnu.addMenu(toolmenu);
    }

    if ( children_.size() > 0 )
    {
	mnu.insertSeparator();
	uiAction* filteritem =
	new uiAction( tr("Display Only at Sections") );
	mnu.insertAction( filteritem, mOnlyAtPolyIdx );
	filteritem->setEnabled( !hastransform );
	uiAction* shwallitem = new uiAction( tr("Display in Full") );
	mnu.insertAction( shwallitem, mAlwaysPolyIdx );
	shwallitem->setEnabled( !hastransform );
	mnu.insertSeparator();
	mnu.insertAction( new uiAction(tr("Save Changes")), mSavePolyIdx );
    }

    addStandardItems( mnu );

    const int mnuid = mnu.exec();
    handleMenu.trigger( mnuid );
    if ( mnuid<0 )
	return false;

    if ( mnuid==mLoadPolyIdx )
    {
	TypeSet<MultiID> mids;
	if ( !applMgr()->pickServer()->loadSets(mids,true) )
	    return false;

	for ( int idx=0; idx<mids.size(); idx++ )
	{
	    Pick::Set& ps = Pick::Mgr().get( mids[idx] );
	    addPolygon( &ps );
	}
    }
    else if ( mnuid==mNewPolyIdx )
    {
	if ( !applMgr()->pickServer()->createEmptySet(true) )
	    return false;

	Pick::Set& ps = Pick::Mgr().get( Pick::Mgr().size()-1 );
	addPolygon( &ps );
    }
    else if ( mnuid==mSavePolyIdx )
    {
	for (int idx=0; idx<children_.size(); idx++) {
	    mDynamicCastGet(uiWMPolygonTreeItem*,itm,children_[idx])
	    if (itm)
		itm->updateZ();
	}

	if ( !applMgr()->pickServer()->storePickSets() )
	    uiMSG().error( tr("Problem saving changes. "
	    "Check write protection.") );
    }
    else if ( mnuid==mAlwaysPolyIdx || mnuid==mOnlyAtPolyIdx )
    {
	const bool showall = mnuid==mAlwaysPolyIdx;
	for ( int idx=0; idx<children_.size(); idx++ )
	{
	    mDynamicCastGet(uiODPolygonTreeItem*,itm,children_[idx])
	    if ( !itm ) continue;

	    itm->setOnlyAtSectionsDisplay( !showall );
	    itm->updateColumnText( uiODSceneMgr::cColorColumn() );
	}
    }
    else
	handleStandardItems( mnuid );

    return true;
}

uiTreeItem* uiWMPolygonTreeItemFactory::createForVis(int visid, uiTreeItem*) const
{
    mDynamicCastGet(visSurvey::PickSetDisplay*,psd,
		    ODMainWin()->applMgr().visServer()->getObject(visid));
    if ( !psd || !psd->isPolygon() )
	return 0;

    Pick::Set* pickset = psd->getSet();
    return new uiWMPolygonTreeItem(visid,*pickset);
}

uiWMPolygonTreeItem::uiWMPolygonTreeItem(int dispid,Pick::Set& ps)
    : uiODPolygonTreeItem(dispid, ps)
    {}

uiWMPolygonTreeItem::~uiWMPolygonTreeItem()
{}

void uiWMPolygonTreeItem::handleMenuCB( CallBacker* cb )
{
    mCBCapsuleUnpackWithCaller( int, mnuid, caller, cb );
    mDynamicCastGet( MenuHandler*, menu, caller );
    if ( menu->isHandled() || mnuid==-1 )
	return;

    if ( menu->menuID()!=displayID() )
	return;

    if ( mnuid==storemnuitem_.id || mnuid==storeasmnuitem_.id )
	updateZ();
    uiODPolygonTreeItem::handleMenuCB(cb);
}

void uiWMPolygonTreeItem::updateZ()
{
    float zval;
    if ( getProperty( "Z value", zval ) ) {
	for (int idx=0; idx<set_.size(); idx++)
	    set_.get(idx).setZ(zval);
	applMgr()->storePickSet( set_ );
    }
}
