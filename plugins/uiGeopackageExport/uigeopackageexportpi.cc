#include "uigeopackageexportmod.h"

#include "envvars.h"
#include "filepath.h"
#include "oddirs.h"
#include "ioman.h"
#include "sharedlibs.h"
#include "uimenu.h"
#include "uiodmenumgr.h"
#include "uitoolbar.h"
#include "uistring.h"
#include "odplugin.h"
#include "survinfo.h"
#include "uimsg.h"
#include "uistring.h"
#include "coordsystem.h"
#include "ctxtioobj.h"
#include "emmanager.h"
#include "emsurfacetr.h"
#include "emhorizon3d.h"
#include "iodir.h"
#include "iodirentry.h"
#include "vishorizondisplay.h"
#include "uivismenuitemhandler.h"
#include "uivispartserv.h"
#include "uiodscenemgr.h"

#include "uigeopackageexportmainwin.h"
#include "uigeotiffexportmainwin.h"
#include "gpkgio.h"
//#include "uigeopackagetreeitem.h"
#include "wmplugins.h"

mDefODPluginInfo(uiGeopackageExport)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
	"Geopackage Export plugin",
	wmPlugins::sKeyWMPlugins(),
	wmPlugins::sKeyWMPluginsAuthor(),
	wmPlugins::sKeyWMPluginsVersion(),
	"Export OpendTect data to a Geopackage database."));
    return &retpi;
}

class uiGeopackageExportMgr :  public CallBacker
{ mODTextTranslationClass(uiGeopackageExportMgr)
public:
			uiGeopackageExportMgr(uiODMain*);
			~uiGeopackageExportMgr();

    uiODMain*		appl_;
    uiGeopackageExportMainWin*	gpxdlg_;
    uiGeotiffExportMainWin*     gtifdlg_;
//    uiVisMenuItemHandler        geopmnuitemhndlr_;

    void	updateToolBar(CallBacker*);
    void	updateMenu(CallBacker*);
    void	gpxDialog(CallBacker*);
    void	gtifDialog(CallBacker*);
//    void        doDisplayCB(CallBacker*);
    void	surveyChgCB(CallBacker*);

    bool        hasCRSdefined();
    bool        has3DHorizons();
};


uiGeopackageExportMgr::uiGeopackageExportMgr( uiODMain* a )
    : appl_(a)
    , gpxdlg_(nullptr)
    , gtifdlg_(nullptr)
//    , geopmnuitemhndlr_(visSurvey::HorizonDisplay::sFactoryKeyword(), *a->applMgr().visServer(), tr("Geopackage Display"), mCB(this, uiGeopackageExportMgr, doDisplayCB), "Add", 994)
{
    mAttachCB( appl_->menuMgr().dTectMnuChanged, uiGeopackageExportMgr::updateMenu );
    mAttachCB( IOM().surveyChanged, uiGeopackageExportMgr::surveyChgCB );
    updateMenu(nullptr);
}


uiGeopackageExportMgr::~uiGeopackageExportMgr()
{
    detachAllNotifiers();
}

bool uiGeopackageExportMgr::hasCRSdefined()
{
    SurveyInfo* si = const_cast<SurveyInfo*>( &SI() );
    return (si->getCoordSystem() && si->getCoordSystem()->isProjection());
}

bool uiGeopackageExportMgr::has3DHorizons()
{
    CtxtIOObj ctio3D(EMHorizon3DTranslatorGroup::ioContext());
    const IODir iodir3D( ctio3D.ctxt_.getSelKey() );
    const IODirEntryList entries3D( iodir3D, ctio3D.ctxt_ );
    return entries3D.size()>0;
}


void uiGeopackageExportMgr::updateMenu( CallBacker* )
{
    deleteAndNullPtr( gpxdlg_ );
    uiAction* newitem = new uiAction( tr("Geopackage Export"), mCB(this,uiGeopackageExportMgr,gpxDialog));
    appl_->menuMgr().getBaseMnu( uiODApplMgr::Exp )->insertAction( newitem );

    deleteAndNullPtr(gtifdlg_);
    newitem = new uiAction( tr("Geotiff Export"), mCB(this,uiGeopackageExportMgr,gtifDialog));
    appl_->menuMgr().getBaseMnu( uiODApplMgr::Exp )->insertAction( newitem );


}

void uiGeopackageExportMgr::gpxDialog( CallBacker* )
{
    if ( !gpxdlg_ ) {
        gpxdlg_ = new uiGeopackageExportMainWin( appl_ );
        if (!hasCRSdefined()) {
            SurveyInfo* si = const_cast<SurveyInfo*>( &SI() );
            uiString msg( tr("Geopackage Export requires a projected Coordinate Reference System (CRS)."
            " The survey '%1' currently has a CRS set to: '%2' which is not a projected CRS."
            " You can set the survey CRS using the Survey-Select/Setup menu item.").arg(si->name()).arg(si->getCoordSystem()->factoryKeyword()) );
            uiMSG().message(msg);
            gpxdlg_->close();
            return;
        }
    }
    gpxdlg_->show();
    gpxdlg_->raise();
}

void uiGeopackageExportMgr::gtifDialog( CallBacker* )
{
    if ( !gtifdlg_ ) {
        gtifdlg_ = new uiGeotiffExportMainWin( appl_ );
        if (!has3DHorizons()) {
            uiString msg( tr("The current project does not contain any 3D horizons that can be exported to Geotiff.") );
            uiMSG().message(msg);
            gtifdlg_->close();
            return;
        }
    }
    gtifdlg_->show();
    gtifdlg_->raise();
}

/*void uiGeopackageExportMgr::doDisplayCB( CallBacker* )
{
    const auto displayid = geopmnuitemhndlr_.getDisplayID();
    uiVisPartServer* visserv = appl_->applMgr().visServer();
    mDynamicCastGet(visSurvey::HorizonDisplay*,hd,visserv->getObject(displayid))
    if ( !hd ) return;

    EM::EMObject* emobj = EM::EMM().getObject( hd->getObjectID() );
    mDynamicCastGet(EM::Horizon3D*,hor,emobj)
    if ( !hor ) { uiMSG().error(tr("Internal: cannot find horizon")); return; }

    uiTreeItem* parent = appl_->sceneMgr().findItem( displayid );
    if ( !parent )
        return;

    const uiTreeItem* item = parent->findChild("Geopackage");
    if (item) {
	mDynamicCastGet(const uiGeopackageTreeItem*,gpitm,item);
	if ( gpitm )
	    return;
    }

    const int attrib = visserv->addAttrib( displayid );
    Attrib::SelSpec spec( "Geopackage", Attrib::SelSpec::cAttribNotSel(), false, 0 );
    spec.setDefString( uiGeopackageTreeItem::sKeyGeopackageDefString() );
    visserv->setSelSpec( displayid, attrib, spec );

    uiGeopackageTreeItem* newitem = new uiGeopackageTreeItem(typeid(*parent).name());
    if (newitem) {
        parent->addChild(newitem, false);
        newitem->showPropertyDlg();
    }
}*/

void uiGeopackageExportMgr::surveyChgCB( CallBacker* )
{
    if (gpxdlg_) {
	gpxdlg_->close();
	deleteAndNullPtr( gpxdlg_ );
    }
    if (gtifdlg_) {
	gtifdlg_->close();
	deleteAndNullPtr( gtifdlg_ );
    }
}

mDefODInitPlugin(uiGeopackageExport)
{
    mDefineStaticLocalObject( PtrMan<uiGeopackageExportMgr>, theinst_, = 0 );
    if ( theinst_ ) return 0;

    theinst_ = new uiGeopackageExportMgr( ODMainWin() );
    if ( !theinst_ )
        return "Cannot instantiate Geopackage Export plugin";

    BufferString libnm( 256, false );
    SharedLibAccess::getLibName("gpkg", libnm.getCStr(), libnm.bufSize());
    if ( __islinux__ )
	libnm.add(".0");

    FilePath libfp(GetLibPlfDir(), libnm);
    if ( !libfp.exists() && GetEnvVar("OD_USER_PLUGIN_DIR") )
	libfp = FilePath(GetEnvVar("OD_USER_PLUGIN_DIR"), "bin", GetPlfSubDir(),
			 GetBinSubDir(), libnm);

    if ( libfp.exists() )
	GeopackageIO::setGPKGlib_location(libfp.fullPath());
    else
    {
	BufferString msg("uiGeopackageExport: cannot find: ", libnm);
	return msg.buf();
    }

//    uiGeopackageTreeItem::initClass();

    return 0;
}
