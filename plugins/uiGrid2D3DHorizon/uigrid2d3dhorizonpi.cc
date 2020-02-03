#include "uigrid2d3dhorizonmod.h"

#include "ioman.h"
#include "uimenu.h"
#include "uiodmenumgr.h"
#include "uitoolbar.h"
#include "uistring.h"
#include "odplugin.h"
#include "survinfo.h"
#include "uimsg.h"

#include "uigrid2d3dhorizonmainwin.h"
#include "wmplugins.h"


mDefODPluginInfo(uiGrid2D3DHorizon)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
	"Grid 2D and 3D horizon plugin",
	wmPlugins::sKeyWMPlugins(),
	wmPlugins::sKeyWMPluginsAuthor(),
	wmPlugins::sKeyWMPluginsVersion(),
	"Grid 2D and 3D OpendTect horizon data."));
    return &retpi;
}

class uiGrid2D3DHorizonMgr :  public CallBacker
{ mODTextTranslationClass(uiGrid2D3DHorizonMgr)
public:
			uiGrid2D3DHorizonMgr(uiODMain*);
			~uiGrid2D3DHorizonMgr();

    uiODMain*			appl_;
    uiGrid2D3DHorizonMainWin*	dlg_;

    void		updateToolBar(CallBacker*);
    void		updateMenu(CallBacker*);
    void		gridDialog(CallBacker*);
    void		surveyChgCB(CallBacker*);
};


uiGrid2D3DHorizonMgr::uiGrid2D3DHorizonMgr( uiODMain* a )
	: appl_(a)
	, dlg_(nullptr)
{
    mAttachCB( appl_->menuMgr().dTectTBChanged, uiGrid2D3DHorizonMgr::updateToolBar );
    mAttachCB( appl_->menuMgr().dTectMnuChanged, uiGrid2D3DHorizonMgr::updateMenu );
    mAttachCB(IOM().surveyChanged, uiGrid2D3DHorizonMgr::surveyChgCB);
    updateToolBar(nullptr);
    updateMenu(nullptr);
}


uiGrid2D3DHorizonMgr::~uiGrid2D3DHorizonMgr()
{
    detachAllNotifiers();
}


void uiGrid2D3DHorizonMgr::updateToolBar( CallBacker* )
{
}


void uiGrid2D3DHorizonMgr::updateMenu( CallBacker* )
{
    deleteAndZeroPtr( dlg_ );

    uiODMenuMgr& mnumgr = appl_->menuMgr();
    uiActionSeparString gridprocstr( "Create Horizon Output" );
    uiAction* itm = mnumgr.procMnu()->findAction( gridprocstr );
    if ( !itm || !itm->getMenu() ) return;
    
    itm->getMenu()->insertItem( new uiAction("Grid 2D-3D Horizon ...",mCB(this,uiGrid2D3DHorizonMgr,gridDialog)) );
}


void uiGrid2D3DHorizonMgr::gridDialog( CallBacker* )
{
    if ( !dlg_ ) {
        dlg_ = new uiGrid2D3DHorizonMainWin( appl_ );
    }
    dlg_->show();
    dlg_->raise();
}

void uiGrid2D3DHorizonMgr::surveyChgCB( CallBacker* )
{
    if ( dlg_ ) {
	dlg_->close();
	deleteAndZeroPtr( dlg_ );
    }
}

mDefODInitPlugin(uiGrid2D3DHorizon)
{
    mDefineStaticLocalObject( PtrMan<uiGrid2D3DHorizonMgr>, theinst_, = 0 );
    if ( theinst_ ) return 0;
    
    theinst_ = new uiGrid2D3DHorizonMgr( ODMainWin() );
    if ( !theinst_ )
        return "Cannot instantiate Grid 2D-3D Horizon plugin";
    
     return 0;
}
