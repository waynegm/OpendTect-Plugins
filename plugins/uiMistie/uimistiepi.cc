#include "uimenu.h"
#include "uiodmain.h"
#include "odplugin.h"
#include "uimsg.h"
#include "uimenu.h"
#include "uiodmenumgr.h"

#include "uimistieapplier.h"
#include "uimistiecorrmainwin.h"
#include "uimistieanalysismainwin.h"

mDefODPluginInfo(uiMistie)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
    "Mistie analysis and correction (UI) plugin",
    "OpendTect",
    "Wayne Mogg",
    "1.0",
    "A plugin for determining seismic misties, computing corrections and applying them to seismic data") );
    return &retpi;
}

class uiMistieMgr : public CallBacker
{ mODTextTranslationClass(uiMistieMgr);
public:
    uiMistieMgr(uiODMain*);
    ~uiMistieMgr();

    void updateMenu(CallBacker*);
    void doCorrectionEdit(CallBacker*);
    void doMistieAnalysis(CallBacker*);
    
protected:
    uiODMain*   appl_;
    
    
};

uiMistieMgr::uiMistieMgr( uiODMain* a )
    : appl_(a)
{
    mAttachCB( appl_->menuMgr().dTectMnuChanged, uiMistieMgr::updateMenu );
    updateMenu(0);
}

uiMistieMgr::~uiMistieMgr()
{
    detachAllNotifiers();
}

void uiMistieMgr::updateMenu( CallBacker* )
{
    uiAction* newitem = new uiAction( tr("Mistie Corrections"), mCB(this,uiMistieMgr,doCorrectionEdit));
    appl_->menuMgr().getBaseMnu( uiODApplMgr::Man )->insertItem( newitem );

    newitem = new uiAction( tr("Mistie Analysis"), mCB(this,uiMistieMgr,doMistieAnalysis));
    appl_->menuMgr().analMnu()->insertItem( newitem );
}

void uiMistieMgr::doCorrectionEdit( CallBacker* )
{
    uiMistieCorrMainWin* dlg = new uiMistieCorrMainWin( appl_ );
    dlg->show();
    dlg->raise();
}

void uiMistieMgr::doMistieAnalysis( CallBacker* )
{
    uiMistieAnalysisMainWin* dlg = new uiMistieAnalysisMainWin( appl_ );
    dlg->show();
    dlg->raise();
}

mDefODInitPlugin(uiMistie)
{
    mDefineStaticLocalObject( PtrMan<uiMistieMgr>, theinst_, = 0 );
    if ( theinst_ ) return 0;
    
    theinst_ = new uiMistieMgr( ODMainWin() );
    if ( !theinst_ )
        return "Cannot instantiate Mistie plugin";
    
    uiMistieApplier::initClass();

    return 0;
}
