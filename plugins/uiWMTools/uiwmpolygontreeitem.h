#ifndef uiwmpolygontreeitem_h
#define uiwmpolygontreeitem_h
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

#include "uiwmtoolsmod.h"

#include "uiodpicksettreeitem.h"


mExpClass(uiWMTools) uiWMPolygonParentTreeItem: public uiODPolygonParentTreeItem
{ mODTextTranslationClass(uiWMPolygonParentTreeItem)
public:
    uiWMPolygonParentTreeItem();
    ~uiWMPolygonParentTreeItem();
    
    bool showSubMenu();
    bool addNewPolygon(Pick::Set*, bool warnifexist=false);
    void addPolygon(Pick::Set*);
    
    CNotifier<uiWMPolygonParentTreeItem,int>	handleMenu;
    MenuItem		toolsmenu_;
    
};

mExpClass(uiWMTools) uiWMPolygonTreeItemFactory : public uiODTreeItemFactory
{ mODTextTranslationClass(uiWMPolygonTreeItemFactory)
public:
    
    const char*		name() const { return typeid(*this).name(); }
    uiTreeItem*		create() const { return new uiWMPolygonParentTreeItem; }
    uiTreeItem*		createForVis(int visid, uiTreeItem* ) const;
};

mExpClass(uiODMain) uiWMPolygonTreeItem : public uiODPolygonTreeItem
{ mODTextTranslationClass(uiWMPolygonTreeItem)
public:
    uiWMPolygonTreeItem(int dispid,Pick::Set&);
    virtual		~uiWMPolygonTreeItem();

protected:
    virtual const char*	parentType() const
    { return typeid(uiWMPolygonParentTreeItem).name(); }
    
};

#endif

