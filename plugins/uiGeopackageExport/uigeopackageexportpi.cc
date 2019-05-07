#include "uigeopackageexportmod.h"

#include "uimenu.h"
#include "uiodmenumgr.h"
#include "uitoolbar.h"
#include "uistring.h"
#include "odplugin.h"
#include "survinfo.h"
#include "uimsg.h"
#include "coordsystem.h"
#include "ctxtioobj.h"
#include "emsurfacetr.h"
#include "iodir.h"
#include "iodirentry.h"

#include "uigeopackageexportmainwin.h"
#include "uigeotiffexportmainwin.h"


mDefODPluginInfo(uiGeopackageExport)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
    "Geopackage Export plugin",
    "",
    "Wayne Mogg",
    "1.0",
    "Export OpendTect survey information to a Geopackage database."));
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

    void		updateToolBar(CallBacker*);
    void		updateMenu(CallBacker*);
    void		gpxDialog(CallBacker*);
    void		gtifDialog(CallBacker*);
    
    bool        hasCRSdefined();
    bool        has3DHorizons();
};


uiGeopackageExportMgr::uiGeopackageExportMgr( uiODMain* a )
	: appl_(a)
	, gpxdlg_(0)
    , gtifdlg_(0)
{
    mAttachCB( appl_->menuMgr().dTectMnuChanged, uiGeopackageExportMgr::updateMenu );
    updateMenu(0);
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
    if (gpxdlg_) {
        delete gpxdlg_;
        gpxdlg_ = 0;
    }
    uiAction* newitem = new uiAction( tr("Geopackage Export"), mCB(this,uiGeopackageExportMgr,gpxDialog));
    appl_->menuMgr().getBaseMnu( uiODApplMgr::Exp )->insertItem( newitem );

    if (gtifdlg_) {
        delete gtifdlg_;
        gtifdlg_ = 0;
    }
    newitem = new uiAction( tr("Geotiff Export"), mCB(this,uiGeopackageExportMgr,gtifDialog));
    appl_->menuMgr().getBaseMnu( uiODApplMgr::Exp )->insertItem( newitem );
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

mDefODInitPlugin(uiGeopackageExport)
{
    mDefineStaticLocalObject( PtrMan<uiGeopackageExportMgr>, theinst_, = 0 );
    if ( theinst_ ) return 0;

    theinst_ = new uiGeopackageExportMgr( ODMainWin() );
    if ( !theinst_ )
        return "Cannot instantiate Geopackage Export plugin";
    
     return 0;
}
