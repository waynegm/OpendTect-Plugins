#include "uimenu.h"
#include "uiodmain.h"
#include "odplugin.h"
#include "uimsg.h"
#include "uimenu.h"
#include "uiodmenumgr.h"
#include "ioman.h"
#include "survinfo.h"

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
    void doMistieCorrection2D(CallBacker*);
    void doMistieCorrection3D(CallBacker*);
    void surveyChgCB(CallBacker*);

protected:
    uiODMain*   appl_;
    uiMistieAnalysisMainWin*    mistiedlg_;
    uiMistieCorrMainWin*	corrdlg_;
    uiMistieCorrHorDlg*		corrhor2ddlg_;
    uiMistieCorrHorDlg*		corrhor3ddlg_;


};

uiMistieMgr::uiMistieMgr( uiODMain* a )
    : appl_(a)
    , mistiedlg_(nullptr)
    , corrdlg_(nullptr)
    , corrhor2ddlg_(nullptr)
    , corrhor3ddlg_(nullptr)
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
    mnumgr.getBaseMnu( uiODApplMgr::Man )->insertAction( newitem );

    newitem = new uiAction( tr("Mistie Analysis"), mCB(this,uiMistieMgr,doMistieAnalysis));
    mnumgr.analMnu()->insertAction( newitem );

    uiActionSeparString gridprocstr( "Create Horizon Output" );
    uiAction* itm = mnumgr.procMnu()->findAction( gridprocstr );
    if ( !itm || !itm->getMenu() ) return;
    uiMenu* miscor = new uiMenu(appl_, tr("Apply Mistie Corrections"));

    if (SI().has2D())
	miscor->insertAction(new uiAction(m3Dots(uiStrings::s2D()),mCB(this,uiMistieMgr,doMistieCorrection2D)));
    if (SI().has3D())
	miscor->insertAction(new uiAction(m3Dots(uiStrings::s3D()),mCB(this,uiMistieMgr,doMistieCorrection3D)));

    itm->getMenu()->addMenu(miscor);
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

void uiMistieMgr::doMistieCorrection2D( CallBacker* )
{
    if ( !corrhor2ddlg_ )
	corrhor2ddlg_ = new uiMistieCorrHorDlg(appl_, true);
    corrhor2ddlg_->show();
    corrhor2ddlg_->raise();
}

void uiMistieMgr::doMistieCorrection3D( CallBacker* )
{
    if ( !corrhor3ddlg_ )
	corrhor3ddlg_ = new uiMistieCorrHorDlg(appl_, false);
    corrhor3ddlg_->show();
    corrhor3ddlg_->raise();
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
    if (corrhor2ddlg_) {
	corrhor2ddlg_->close();
	deleteAndZeroPtr( corrhor2ddlg_);
    }
    if (corrhor3ddlg_) {
	corrhor3ddlg_->close();
	deleteAndZeroPtr( corrhor3ddlg_);
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
