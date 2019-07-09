#ifndef uigeopackagetreeitem_h
#define uigeopackagetreeitem_h

/*
 *   uiGeopackageExport Plugin
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

#include "uioddatatreeitem.h"
#include "color.h"

namespace visSurvey { class HorizonDisplay; }
namespace visBase
{
    class DrawStyle;
    class PolyLine;
    class Material;
}

class uiGeopackageReader;

/*!\brief Tree item for Geopackage display on 3D horizons */

mClass(uiGeopackageExport) uiGeopackageTreeItem: public uiODDataTreeItem
{ mODTextTranslationClass(uiGeopackageTreeItem);
public:
    static void                 initClass();
    uiGeopackageTreeItem(const char* parenttype);
    ~uiGeopackageTreeItem();
    
    static uiODDataTreeItem*    create(const Attrib::SelSpec&,const char*);
    void                        showPropertyDlg();
    
    static const char*          sKeyGeopackageDefString();
    
    
private:
    virtual bool            init();
    virtual uiString        createDisplayName() const;
    virtual void            checkCB(CallBacker*);
    virtual bool            doubleClick(uiTreeViewItem*);    
    virtual void            createMenu(MenuHandler*,bool istb);
    virtual void            handleMenuCB(CallBacker*);
    
    void                    prepareForShutdown();
    void                    removeAll();
    
    void                    visClosingCB(CallBacker*);
    
    void                    updateColumnText(int);
    void                    showLayer();
    bool                    createPolyLines();
    void                    removeOldLinesFromScene();
    
    visSurvey::HorizonDisplay*  getHorDisp();
    
    uiGeopackageReader*     reader_;
    Color                   color_;
    int                     linewidth_;
    visBase::PolyLine*      lines_;
    visBase::DrawStyle*     drawstyle_;
    visBase::Material*      material_;
    
    MenuItem                optionsmenuitem_;
};

#endif
