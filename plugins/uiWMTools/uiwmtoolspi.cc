/*
 *   uiWMTools Plugin
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
#include "uitoolbar.h"
#include "uistring.h"
#include "odplugin.h"
#include "survinfo.h"
#include "uimsg.h"



mDefODPluginInfo(uiWMTools)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
    "WMTools plugin",
    "",
    "Wayne Mogg",
    "1.0",
    "Miscellaneous tools to make life with OpendTect easier."));
    return &retpi;
}

class uiWMToolsMgr :  public CallBacker
{ mODTextTranslationClass(uiWMToolsMgr)
public:
                uiWMToolsMgr(uiODMain*);
                ~uiWMToolsMgr();

    void        updateMenu(CallBacker*);
    void        segyscanCB(CallBacker*);
    void        segyfixCB(CallBacker*);

    uiODMain*   appl_;
};


uiWMToolsMgr::uiWMToolsMgr( uiODMain* a )
	: appl_(a)
{
    mAttachCB( appl_->menuMgr().dTectMnuChanged, uiWMToolsMgr::updateMenu );
    updateMenu(0);
}


uiWMToolsMgr::~uiWMToolsMgr()
{
    detachAllNotifiers();
}

void uiWMToolsMgr::updateMenu( CallBacker* )
{
    uiMenu* wmtools = new uiMenu(appl_, tr("WM Tools"));
    appl_->menuMgr().toolsMnu()->insertItem( wmtools );
//    wmtools->insertItem( new uiAction( tr("SEG-Y Scan"), mCB(this, uiWMToolsMgr, segyscanCB)) );
    wmtools->insertItem( new uiAction( tr("SEG-Y Fix"), mCB(this, uiWMToolsMgr, segyfixCB)) );
}

void uiWMToolsMgr::segyscanCB( CallBacker* )
{
}

void uiWMToolsMgr::segyfixCB( CallBacker* )
{
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
