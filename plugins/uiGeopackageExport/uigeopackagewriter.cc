#include "uigeopackagewriter.h"

#include "survinfo.h"
#include "coordsystem.h"
#include "crssystem.h"
#include "crsproj.h"
#include "bufstring.h"
#include "uistring.h"
#include "uimsg.h"
#include "errmsg.h"
#include "ogrsf_frmts.h"
#include "ogr_spatialref.h"

uiGeopackageWriter::uiGeopackageWriter( const char* filename, bool append )
: gdalDS_(NULL), poSRS_(NULL)
{
    if (SI().getCoordSystem()->isProjection()) {
        const Coords::ProjectionBasedSystem* const proj = dynamic_cast<const Coords::ProjectionBasedSystem* const>(SI().getCoordSystem().ptr());
        poSRS_= new OGRSpatialReference();
        poSRS_->importFromEPSG(proj->getProjection()->authCode().id());
        open(filename, append);
    } else {
        BufferString crsSummary("uiGeopackageWriter::uiGeopackageWriter - unrecognised CRS: ");
        crsSummary += SI().getCoordSystem()->summary();
        ErrMsg(crsSummary);
    }
}

uiGeopackageWriter::~uiGeopackageWriter()
{
    close();
    if (poSRS_ != NULL)
        delete poSRS_;
}

bool uiGeopackageWriter::open( const char* filename, bool append )
{
    const char* papszAllowedDrivers[] = { "GPKG", NULL };
    
    GDALAllRegister();
    
    if (append) {
        gdalDS_ = GDALDataset::Open(filename, GDAL_OF_VECTOR || GDAL_OF_UPDATE, papszAllowedDrivers, NULL, NULL);
        if (gdalDS_ == NULL) {
            ErrMsg("uiGeopackageWriter::open - cannot open output file for appending.");
            return false;
        }
    } else {
        GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName(papszAllowedDrivers[0]);
        if (poDriver == NULL) {
            ErrMsg("uiGeopackageWriter::open - GPKG driver not available");
            return false;
        }
        gdalDS_ = poDriver->Create(filename, 0, 0, 0, GDT_Unknown, NULL);
        if (gdalDS_ == NULL) {
            ErrMsg("uiGeopackageWriter::open - cannot create output file.");
            return false;
        }
    }
    return true;
}

void uiGeopackageWriter::close()
{
    if (gdalDS_)
        GDALClose( gdalDS_ );
}

void uiGeopackageWriter::writeSurvey()
{
    if (gdalDS_) {
        SurveyInfo* si = const_cast<SurveyInfo*>( &SI() );
    
        OGRLayer* poLayer = gdalDS_->CreateLayer( "Survey", poSRS_, wkbPolygon, NULL );
        if (poLayer == NULL) {
            ErrMsg("uiGeopackageWriter::writeSurvey - layer creation failed");
            return;
        }
    
        OGRFieldDefn oField( "Name", OFTString );
        oField.SetWidth(32);
        if( poLayer->CreateField( &oField ) != OGRERR_NONE ) {
            ErrMsg("uiGeopackageWriter::writeSurvey - creating Name field failed" );
            return;
        }
    
        OGRFeature feature( poLayer->GetLayerDefn() );
        feature.SetField("Name", GetSurveyName());
    
        OGRLinearRing ring;
    
        const StepInterval<int> inlrg = si->inlRange( false );
        const StepInterval<int> crlrg = si->crlRange( false );
        Coord coord;
        coord = si->transform(BinID(inlrg.start,crlrg.start));
        ring.addPoint( coord.x, coord.y );
        coord = si->transform(BinID(inlrg.start,crlrg.stop));
        ring.addPoint( coord.x, coord.y );
        coord = si->transform(BinID(inlrg.stop,crlrg.stop));
        ring.addPoint( coord.x, coord.y );
        coord = si->transform(BinID(inlrg.stop,crlrg.start));
        ring.addPoint( coord.x, coord.y );
        coord = si->transform(BinID(inlrg.start,crlrg.start));
        ring.addPoint( coord.x, coord.y );

        OGRPolygon survpoly;
        survpoly.addRing(&ring);
        feature.SetGeometry( &survpoly );
        if (poLayer->CreateFeature( &feature ) != OGRERR_NONE)
            ErrMsg("uiGeopackageWriter::writeSurvey - create feature failed" );
    }
}
