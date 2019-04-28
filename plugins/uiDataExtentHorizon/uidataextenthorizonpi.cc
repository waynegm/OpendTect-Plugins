#include "uidataextenthorizonmod.h"

#include "callback.h"
#include "uimenu.h"
#include "uiodmenumgr.h"
#include "uimenuhandler.h"
#include "menuhandler.h"
#include "uiodscenemgr.h"
#include "uiodtreeitem.h"
#include "uiodhortreeitem.h"
#include "uitreeitemmanager.h"
#include "uitoolbar.h"
#include "uistring.h"
#include "odplugin.h"
#include "survinfo.h"
#include "uimsg.h"

#include "uidehmainwin.h"


mDefODPluginInfo(uiDataExtentHorizon)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
    "Create 3D horizon covering extent of 2D and 3D data",
    "",
    "Wayne Mogg",
    "1.0",
    "Create 3D horizon covering extent of 2D and 3D data."));
    return &retpi;
}

class uiDataExtentHorizonMgr :  public CallBacker
{ mODTextTranslationClass(uiDataExtentHorizonMgr)
public:
			uiDataExtentHorizonMgr(uiODMain*);
			~uiDataExtentHorizonMgr();

    uiODMain*		appl_;
    uidehMainWin*	dlg_;

    void		updateMenu(CallBacker*);
    void		dehDialog(CallBacker*);
};


uiDataExtentHorizonMgr::uiDataExtentHorizonMgr( uiODMain* a )
	: appl_(a)
	, dlg_(nullptr)
{
    mAttachCB( appl_->menuMgr().dTectMnuChanged, uiDataExtentHorizonMgr::updateMenu );
}


uiDataExtentHorizonMgr::~uiDataExtentHorizonMgr()
{
    detachAllNotifiers();
}

void uiDataExtentHorizonMgr::updateMenu( CallBacker* )
{
    if (dlg_!=nullptr) {
        delete dlg_;
        dlg_ = nullptr;
    }

    uiODMenuMgr& mnumgr = appl_->menuMgr();
    uiActionSeparString dehprocstr( "Create Horizon Output" );
    uiAction* itm = mnumgr.procMnu()->findAction( dehprocstr );
    if ( !itm || !itm->getMenu() ) return;
    
    itm->getMenu()->insertItem( new uiAction(m3Dots(tr("Create Data Extent Horizon")),mCB(this,uiDataExtentHorizonMgr,dehDialog)) );
}    

void uiDataExtentHorizonMgr::dehDialog(CallBacker*)
{
    if ( dlg_==nullptr ) {
        dlg_ = new uidehMainWin( appl_ );
    }
    dlg_->show();
    dlg_->raise();
}


mDefODInitPlugin(uiDataExtentHorizon)
{
    mDefineStaticLocalObject( PtrMan<uiDataExtentHorizonMgr>, theinst_, = 0 );
    if ( theinst_ ) return 0;
    
    theinst_ = new uiDataExtentHorizonMgr( ODMainWin() );
    if ( !theinst_ )
        return "Cannot instantiate DataExtentHorizon plugin";
    
     return 0;
}
