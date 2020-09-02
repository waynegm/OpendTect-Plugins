#include "uimenu.h"
#include "uiodmain.h"
#include "odplugin.h"
#include "uimsg.h"
#include "uimenu.h"
#include "uiodmenumgr.h"
#include "ioman.h"

#include "uimistieapplier.h"
#include "uimistiecorrmainwin.h"
#include "uimistieanalysismainwin.h"
#include "uimistiecorrhordlg.h"
#include "wmplugins.h"

mDefODPluginInfo(uiMistie)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
	"Mistie analysis and correction (UI)",
	wmPlugins::sKeyWMPlugins(),
	wmPlugins::sKeyWMPluginsAuthor(),
	wmPlugins::sKeyWMPluginsVersion(),
	"Mistie analysis and correction for OpendTect") );
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
    void doMistieCorrection(CallBacker*);
    void surveyChgCB(CallBacker*);
    
protected:
    uiODMain*   appl_;
    uiMistieAnalysisMainWin*    mistiedlg_;
    uiMistieCorrMainWin*	corrdlg_;
    uiMistieCorrHorDlg*		corrhordlg_;
    
    
};

uiMistieMgr::uiMistieMgr( uiODMain* a )
    : appl_(a)
    , mistiedlg_(nullptr)
    , corrdlg_(nullptr)
    , corrhordlg_(nullptr)
{
    mAttachCB( appl_->menuMgr().dTectMnuChanged, uiMistieMgr::updateMenu );
    mAttachCB(IOM().surveyChanged, uiMistieMgr::surveyChgCB);
    updateMenu(nullptr);
}

uiMistieMgr::~uiMistieMgr()
{
    detachAllNotifiers();
}

void uiMistieMgr::updateMenu( CallBacker* )
{
    uiODMenuMgr& mnumgr = appl_->menuMgr();
    uiAction* newitem = new uiAction( tr("Mistie Corrections"), mCB(this,uiMistieMgr,doCorrectionEdit));
    mnumgr.getBaseMnu( uiODApplMgr::Man )->insertItem( newitem );

    newitem = new uiAction( tr("Mistie Analysis"), mCB(this,uiMistieMgr,doMistieAnalysis));
    mnumgr.analMnu()->insertItem( newitem );

    uiActionSeparString gridprocstr( "Create Horizon Output" );
    uiAction* itm = mnumgr.procMnu()->findAction( gridprocstr );
    if ( !itm || !itm->getMenu() ) return;
    itm->getMenu()->insertItem( new uiAction("Apply Mistie Corrections ...",mCB(this,uiMistieMgr,doMistieCorrection)));
}

void uiMistieMgr::doCorrectionEdit( CallBacker* )
{
    if ( !corrdlg_ )
	corrdlg_ = new uiMistieCorrMainWin( appl_ );
    corrdlg_->show();
    corrdlg_->raise();
}

void uiMistieMgr::doMistieAnalysis( CallBacker* )
{
    if ( !mistiedlg_ )
	mistiedlg_ = new uiMistieAnalysisMainWin( appl_ );
    mistiedlg_->show();
    mistiedlg_->raise();
}

void uiMistieMgr::doMistieCorrection( CallBacker* )
{
    if ( !corrhordlg_ )
	corrhordlg_ = new uiMistieCorrHorDlg( appl_ );
    corrhordlg_->show();
    corrhordlg_->raise();
}

void uiMistieMgr::surveyChgCB( CallBacker* )
{
    if ( mistiedlg_ ) {
        mistiedlg_->close();
        deleteAndZeroPtr( mistiedlg_ );
    }
    if ( corrdlg_ ) {
	corrdlg_->close();
	deleteAndZeroPtr( corrdlg_ );
    }
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
