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

    void        handleMenuCB(CallBacker*);
    void		updateTreeMenu(CallBacker*);
    void		dehDialog();
    
    MenuItem    dehItem_;
};


uiDataExtentHorizonMgr::uiDataExtentHorizonMgr( uiODMain* a )
	: appl_(a)
    , dehItem_(m3Dots(tr("Covering 2D-3D Data Extent")))
	, dlg_(nullptr)
{
    mAttachCB( appl_->justBeforeGo, uiDataExtentHorizonMgr::updateTreeMenu );
}


uiDataExtentHorizonMgr::~uiDataExtentHorizonMgr()
{
    detachAllNotifiers();
}


void uiDataExtentHorizonMgr::handleMenuCB(CallBacker* cb)
{
    mCBCapsuleUnpack(int, mnuid, cb);
    if (mnuid==dehItem_.id)
        dehDialog();
 }

void uiDataExtentHorizonMgr::updateTreeMenu( CallBacker* )
{
    if (dlg_!=nullptr) {
        delete dlg_;
        dlg_ = nullptr;
    }
    
    uiODTreeTop* sceneTT = appl_->sceneMgr().getTreeItemMgr(appl_->sceneMgr().getActiveSceneID());
    if (!sceneTT) {
        uiMSG().message("Bad Scene TreeTop pointer");
        return;
    }    
    uiTreeItem* tree3D = sceneTT->findChild("3D Horizon");
    if (!tree3D) {
        uiMSG().message("Bad TreeItem pointer");
        return;
    }
    mDynamicCastGet(uiODHorizonParentTreeItem*, horParentTI, tree3D);
    horParentTI->newmenu_.addItem(&dehItem_, false);
    mAttachCB(horParentTI->handleMenu, uiDataExtentHorizonMgr::handleMenuCB);
}    

void uiDataExtentHorizonMgr::dehDialog()
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
