#include "uigeopackageexportmainwin.h"

#include "string.h"
#include "file.h"
#include "filepath.h"
#include "oddirs.h"
#include "strmprov.h"
#include "timer.h"

#include "survinfo.h"
#include "uicoordsystem.h"
#include "uifileinput.h"
#include "uiprogressbar.h"
#include "uibutton.h"
#include "uitabstack.h"
#include "uimsg.h"
#include "seisioobjinfo.h"
#include "ctxtioobj.h"
#include "randomlinetr.h"
#include "welltransl.h"
#include "picksettr.h"
#include "emsurfacetr.h"
#include "iodir.h"
#include "iodirentry.h"
#include "bufstringset.h"

#include "uisurveygrp.h"
#include "ui2dlinesgrp.h"
#include "uirandomlinesgrp.h"
#include "uiwellsgrp.h"
#include "uipolylinesgrp.h"
#include "uihorizongrp.h"
#include "uigeopackagewriter.h"


uiGeopackageExportMainWin::uiGeopackageExportMainWin( uiParent* p )
    : uiDialog(p,uiDialog::Setup(getCaptionStr(),mNoDlgTitle,HelpKey("wgm","geopkg")).modal(false) ),
    linesgrp_(nullptr), randomgrp_(nullptr), wellsgrp_(nullptr), polygrp_(nullptr), horgrp_(nullptr)
{
    setCtrlStyle( OkAndCancel );
    setOkText( uiStrings::sExport() );
    setShrinkAllowed(true);

    tabstack_ = new uiTabStack( this, "Tab" );
    tabstack_->selChange().notify( mCB(this,uiGeopackageExportMainWin,tabSel) );
    tabstack_->attach(hCentered);
    
    uiParent* tabparent = tabstack_->tabGroup();
    if (SI().has3D()) {
        surveygrp_ = new uiSurveyGrp( tabparent );
        surveygrp_->attach(hCentered);
        tabstack_->addTab( surveygrp_ );
    }
    
    if (SI().has2D()) {
        BufferStringSet lnms;
        TypeSet<Pos::GeomID> geomids;
        SeisIOObjInfo::getLinesWithData( lnms, geomids );
        if (lnms.size() > 0 ) {
            linesgrp_ = new ui2DLinesGrp( tabparent );
            tabstack_->addTab( linesgrp_ );
        }
    }
    
    if (SI().has3D()) {
        CtxtIOObj ctio(RandomLineSetTranslatorGroup::ioContext());
        const IODir iodir( ctio.ctxt_.getSelKey() );
        const IODirEntryList entries( iodir, ctio.ctxt_ );
        if (entries.size() > 0) {
            randomgrp_ = new uiRandomLinesGrp( tabparent );
            tabstack_->addTab( randomgrp_ );
        }
    }
    
    {
        CtxtIOObj ctio(WellTranslatorGroup::ioContext());
        const IODir iodir( ctio.ctxt_.getSelKey() );
        const IODirEntryList entries( iodir, ctio.ctxt_ );
        if (entries.size() > 0) {
            wellsgrp_ = new uiWellsGrp( tabparent );
            tabstack_->addTab( wellsgrp_ );
        }
    }
    
    {
        CtxtIOObj ctio(PickSetTranslatorGroup::ioContext());
        ctio.ctxt_.toselect_.require_.set( sKey::Type(), sKey::Polygon() );
        const IODir iodir( ctio.ctxt_.getSelKey()  );
        const IODirEntryList entries( iodir, ctio.ctxt_ );
        if (entries.size() > 0) {
            polygrp_ = new uiPolyLinesGrp( tabparent );
            tabstack_->addTab( polygrp_ );
        }
    }
    
    {
        CtxtIOObj ctio2D(EMHorizon2DTranslatorGroup::ioContext());
        const IODir iodir2D( ctio2D.ctxt_.getSelKey() );
        const IODirEntryList entries2D( iodir2D, ctio2D.ctxt_ );
        bool has2Dhorizon = SI().has2D() && entries2D.size()>0;
        CtxtIOObj ctio3D(EMHorizon3DTranslatorGroup::ioContext());
        const IODir iodir3D( ctio3D.ctxt_.getSelKey() );
        const IODirEntryList entries3D( iodir3D, ctio3D.ctxt_ );
        bool has3Dhorizon = SI().has3D() && entries3D.size()>0;
        if (has2Dhorizon || has3Dhorizon) {
            horgrp_= new uiHorizonGrp( tabparent, has2Dhorizon, has3Dhorizon);
            tabstack_->addTab( horgrp_ );
        }
    }
    
    BufferString defseldir = FilePath(GetDataDir()).add("Misc").fullPath();
    filefld_ = new uiFileInput( this, "Output file",
                                uiFileInput::Setup(uiFileDialog::Gen)
                                .forread(false).filter("*.gpkg").defseldir(defseldir).allowallextensions(false) );
    filefld_->setDefaultExtension( "gpkg" );
    filefld_->attach( stretchedBelow, tabstack_ );
    
    appendfld_ = new uiCheckBox( this, tr("Append to Output") );
    appendfld_->attach(alignedBelow, filefld_);
    appendfld_->setChecked(false);
    
    tabSel(0);
}

uiGeopackageExportMainWin::~uiGeopackageExportMainWin()
{
    
}

void uiGeopackageExportMainWin::tabSel( CallBacker* )
{
    if ( !tabstack_ )
	return;

    uiGroup* grp = tabstack_->currentPage();
    if ( !grp ) return;
    if (grp->name() == "Horizon")
        horgrp_->update();

}

bool uiGeopackageExportMainWin::checkCRSdefined()
{
    SurveyInfo* si = const_cast<SurveyInfo*>( &SI() );
    return (si->getCoordSystem() && si->getCoordSystem()->isProjection());
}

bool uiGeopackageExportMainWin::acceptOK( CallBacker*)
{
    if (!strlen(filefld_->fileName())) {
        uiMSG().error( tr("Please specify an output file") );
        return false;
    }
    if (horgrp_!=nullptr && horgrp_->doHorizonExport()) {
        MultiID hor2Did, hor3Did;
        horgrp_->getHorIds(hor2Did, hor3Did);
        if (!hor2Did.isUdf() && horgrp_->num2DLinesChosen()==0) {
            tabstack_->setCurrentPage( horgrp_ );
            uiMSG().error( tr("Please select the 2D line(s) to export") );
            return false;
        }
        if (strlen(horgrp_->outputName())==0) {
            tabstack_->setCurrentPage( horgrp_ );
            uiMSG().error( tr("Please specify an output layer name for the horizon") );
            return false;
        }
    }
        
    uiGeopackageWriter gpgWriter(filefld_->fileName(), appendfld_->isChecked());
    if (SI().has3D()) 
        if( surveygrp_->doExport() ) {
            setCaption(tr("Exporting 3D Survey"));
            gpgWriter.writeSurvey();
        }

    if (SI().has2D() && linesgrp_!=nullptr) {
        TypeSet<Pos::GeomID> geomids;
        linesgrp_->getGeoMids( geomids );
        if ( linesgrp_->doLineExport() ) {
            setCaption(tr("Exporting 2D Lines"));
            gpgWriter.write2DLines( geomids );
        }
        if (linesgrp_->doStationExport() ) {
            setCaption(tr("Exporting 2D Stations"));
            gpgWriter.write2DStations( geomids );
        }
    }
        
    if (randomgrp_!=nullptr) {
        if (randomgrp_->doLineExport()) {
            setCaption(tr("Exporting Random Lines"));
            TypeSet<MultiID> lineids;
            randomgrp_->getLineIds( lineids );
            gpgWriter.writeRandomLines( lineids );
        }
    }

    if (wellsgrp_!=nullptr) {
        if (wellsgrp_->doWellExport()) {
            setCaption(tr("Exporting wells"));
            TypeSet<MultiID> wellids;
            wellsgrp_->getWellIds( wellids);
            gpgWriter.writeWells( wellids );
        }
    }

    if (polygrp_!=nullptr) {
        if (polygrp_->doLineExport()) {
            setCaption(tr("Exporting polylines"));
            TypeSet<MultiID> lineids;
            polygrp_->getLineIds( lineids );
            gpgWriter.writePolyLines( lineids );
        }
    }
        
    if (horgrp_!=nullptr) {
        if (horgrp_->doHorizonExport()) {
            setCaption(tr("Exporting horizon"));
            MultiID hor2Did, hor3Did;
            horgrp_->getHorIds(hor2Did, hor3Did);
            TypeSet<Pos::GeomID> geomids;
            horgrp_->getGeoMids(geomids);
            TrcKeyZSampling env;
            horgrp_->get3Dsel(env);
            gpgWriter.writeHorizon(horgrp_->outputName(), hor2Did, horgrp_->attrib2D(), geomids, hor3Did, horgrp_->attrib3D(), env); 
        }
    }
    
    return true;
}


uiString uiGeopackageExportMainWin::getCaptionStr() const
{
    return tr("Geopackage Export");
}
