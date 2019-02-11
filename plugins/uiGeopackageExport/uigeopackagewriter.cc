#include "uigeopackagewriter.h"

#include "survinfo.h"
#include "survgeom2d.h"
#include "posinfo2d.h"
#include "coordsystem.h"
#include "crssystem.h"
#include "crsproj.h"
#include "bufstring.h"
#include "bufstringset.h"
#include "uistring.h"
#include "uimsg.h"
#include "errmsg.h"
#include "ctxtioobj.h"
#include "ioman.h"
#include "randomlinetr.h"
#include "pickset.h"
#include "picksettr.h"
#include "welldata.h"
#include "wellman.h"
#include "randomlinegeom.h"

#include "ogrsf_frmts.h"
#include "ogr_spatialref.h"

uiGeopackageWriter::uiGeopackageWriter( const char* filename, bool append )
: gdalDS_(nullptr), poSRS_(nullptr)
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
    if (poSRS_ != nullptr)
        delete poSRS_;
}

bool uiGeopackageWriter::open( const char* filename, bool append )
{
    const char* papszAllowedDrivers[] = { "GPKG", NULL };
    
    GDALAllRegister();
    
    if (append) {
        gdalDS_ = GDALDataset::Open(filename, GDAL_OF_VECTOR || GDAL_OF_UPDATE, papszAllowedDrivers, nullptr, nullptr);
        if (gdalDS_ == nullptr) {
            ErrMsg("uiGeopackageWriter::open - cannot open output file for appending.");
            return false;
        }
    } else {
        GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName(papszAllowedDrivers[0]);
        if (poDriver == nullptr) {
            ErrMsg("uiGeopackageWriter::open - GPKG driver not available");
            return false;
        }
        gdalDS_ = poDriver->Create(filename, 0, 0, 0, GDT_Unknown, nullptr);
        if (gdalDS_ == nullptr) {
            ErrMsg("uiGeopackageWriter::open - cannot create output file.");
            return false;
        }
    }
    return true;
}

void uiGeopackageWriter::close()
{
    if (gdalDS_ != nullptr)
        GDALClose( gdalDS_ );
}

void uiGeopackageWriter::writeSurvey()
{
    if (gdalDS_ != nullptr) {
        SurveyInfo* si = const_cast<SurveyInfo*>( &SI() );
    
        OGRLayer* poLayer = gdalDS_->CreateLayer( "Survey", poSRS_, wkbPolygon, NULL );
        if (poLayer == nullptr) {
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
            ErrMsg("uiGeopackageWriter::writeSurvey - creating feature failed" );
    }
}

void uiGeopackageWriter::write2DLines( TypeSet<Pos::GeomID>& geomids )
{
    if (gdalDS_ != nullptr) {
        OGRLayer* poLayer = gdalDS_->CreateLayer( "2DLines", poSRS_, wkbLineString, NULL );
        if (poLayer == nullptr) {
            ErrMsg("uiGeopackageWriter::write2DLines - layer creation failed");
            return;
        }

        OGRFieldDefn oField( "LineName", OFTString );
        oField.SetWidth(32);
        if( poLayer->CreateField( &oField ) != OGRERR_NONE ) {
            ErrMsg("uiGeopackageWriter::write2DLines - creating Line Name field failed" );
            return;
        }
        
        for ( int idx=0; idx<geomids.size(); idx++ ) {
            mDynamicCastGet( const Survey::Geometry2D*, geom2d, Survey::GM().getGeometry(geomids[idx]) );
            if ( !geom2d )
                continue;
            
            const PosInfo::Line2DData& geom = geom2d->data();
            const TypeSet<PosInfo::Line2DPos>& posns = geom.positions();
            
            OGRFeature feature( poLayer->GetLayerDefn() );
            feature.SetField("LineName", geom2d->getName());
            
            OGRLineString line;
            for ( int tdx=0; tdx<posns.size(); tdx++ ) {
                Coord pos = posns[tdx].coord_;
                line.addPoint( pos.x, pos.y );
            }
            feature.SetGeometry( &line );
            if (poLayer->CreateFeature( &feature ) != OGRERR_NONE)
                ErrMsg("uiGeopackageWriter::write2DLines - creating feature failed" );
        }
    }
}

void uiGeopackageWriter::write2DStations( TypeSet<Pos::GeomID>& geomids )
{
    if (gdalDS_ != nullptr) {
        OGRLayer* poLayer = gdalDS_->CreateLayer( "2DStations", poSRS_, wkbPoint, NULL );
        if (poLayer == nullptr) {
            ErrMsg("uiGeopackageWriter::write2DStations - layer creation failed");
            return;
        }
        
        OGRFieldDefn oLineField( "LineName", OFTString );
        oLineField.SetWidth(32);
        if( poLayer->CreateField( &oLineField ) != OGRERR_NONE ) {
            ErrMsg("uiGeopackageWriter::write2DStations - creating Line Name field failed" );
            return;
        }

        OGRFieldDefn oStationField( "Station", OFTInteger );
        if( poLayer->CreateField( &oStationField ) != OGRERR_NONE ) {
            ErrMsg("uiGeopackageWriter::write2DStations - creating Station field failed" );
            return;
        }
        
        for ( int idx=0; idx<geomids.size(); idx++ ) {
            mDynamicCastGet( const Survey::Geometry2D*, geom2d, Survey::GM().getGeometry(geomids[idx]) );
            if ( !geom2d )
                continue;
            
            const PosInfo::Line2DData& geom = geom2d->data();
            const TypeSet<PosInfo::Line2DPos>& posns = geom.positions();
            
            for ( int tdx=0; tdx<posns.size(); tdx++ ) {
                OGRFeature feature( poLayer->GetLayerDefn() );
                feature.SetField("LineName", geom2d->getName());
                feature.SetField("Station", posns[tdx].nr_);
                
                OGRPoint pt;
                Coord pos = posns[tdx].coord_;
                pt.setX(pos.x);
                pt.setY(pos.y);
                feature.SetGeometry( &pt );
                if (poLayer->CreateFeature( &feature ) != OGRERR_NONE)
                    ErrMsg("uiGeopackageWriter::write2DStations - creating feature failed" );
            }
        }
    }
}

void uiGeopackageWriter::writeRandomLines( TypeSet<MultiID>& lineids )
{
    if (gdalDS_ != nullptr) {
        OGRLayer* poLayer = gdalDS_->CreateLayer( "RandomLines", poSRS_, wkbLineString, NULL );
        if (poLayer == nullptr) {
            ErrMsg("uiGeopackageWriter::writeRandomLines - layer creation failed");
            return;
        }
        
        OGRFieldDefn oField( "LineName", OFTString );
        oField.SetWidth(32);
        if( poLayer->CreateField( &oField ) != OGRERR_NONE ) {
            ErrMsg("uiGeopackageWriter::writeRandomLines - creating Line Name field failed" );
            return;
        }
        
        for ( int idx=0; idx<lineids.size(); idx++ ) {
            Geometry::RandomLineSet inprls;
            BufferString msg;
            IOObj* ioobj = IOM().get(lineids[idx]);
            if ( ioobj == nullptr ) {
                ErrMsg("uiGeopackageWriter::writeRandomLines - cannot get ioobj" );
                continue;
            }
            
            if (!RandomLineSetTranslator::retrieve( inprls, ioobj, msg )) {
                BufferString tmp("uiGeopackageWriter::writeRandomLines - error reading random line - ");
                tmp += msg;
                ErrMsg(tmp);
                return;
            }
            
            for (int rdx=0; rdx<inprls.size(); rdx++) {
                const Geometry::RandomLine* rdl = inprls.lines()[rdx];
                if ( rdl == nullptr ) {
                    ErrMsg("uiGeopackageWriter::writeRandomLines - cannot get random line geometry" );
                    continue;
                }
                OGRFeature feature( poLayer->GetLayerDefn() );
                feature.SetField("LineName", rdl->name());
            
                OGRLineString line;
                for ( int tdx=0; tdx<rdl->nrNodes(); tdx++ ) {
                    Coord pos = SI().transform( rdl->nodePosition(tdx) );;
                    line.addPoint( pos.x, pos.y );
                }
                feature.SetGeometry( &line );
                if (poLayer->CreateFeature( &feature ) != OGRERR_NONE)
                    ErrMsg("uiGeopackageWriter::writeRandomLines - creating feature failed" );
            }
        }
    }    
}

void uiGeopackageWriter::writeWells( TypeSet<MultiID>& wellids )
{
    if (gdalDS_ != nullptr) {
        OGRLayer* poLayer = gdalDS_->CreateLayer( "Wells", poSRS_, wkbPoint, NULL );
        if (poLayer == nullptr) {
            ErrMsg("uiGeopackageWriter::writeWells - layer creation failed");
            return;
        }
        
        OGRFieldDefn oNameField( "WellName", OFTString );
        oNameField.SetWidth(32);
        if( poLayer->CreateField( &oNameField ) != OGRERR_NONE ) {
            ErrMsg("uiGeopackageWriter::writeWells - creating WellName field failed" );
            return;
        }
        OGRFieldDefn ouwidField( "UWID", OFTString );
        ouwidField.SetWidth(32);
        if( poLayer->CreateField( &ouwidField ) != OGRERR_NONE ) {
            ErrMsg("uiGeopackageWriter::writeWells - creating UWID field failed" );
            return;
        }
        OGRFieldDefn oStatusField( "Status", OFTInteger );
        if( poLayer->CreateField( &oStatusField ) != OGRERR_NONE ) {
            ErrMsg("uiGeopackageWriter::writeWells - creating Status field failed" );
            return;
        }
        
        for ( int idx=0; idx<wellids.size(); idx++ ) {
            Well::Data* wd = Well::MGR().get(wellids[idx]);
            if ( wd == nullptr ) {
                ErrMsg("uiGeopackageWriter::writeWells - unable to read well data");
                continue;
            }
            Well::Info& wdinfo = wd->info();
            
            OGRFeature feature( poLayer->GetLayerDefn() );
            feature.SetField("WellName", wdinfo.name());
            feature.SetField("UWID", wdinfo.uwid);
            feature.SetField("Status", wdinfo.welltype_);
            
            OGRPoint pt;
            pt.setX(wdinfo.surfacecoord.x);
            pt.setY(wdinfo.surfacecoord.y);
            feature.SetGeometry( &pt );
            if (poLayer->CreateFeature( &feature ) != OGRERR_NONE)
                ErrMsg("uiGeopackageWriter::writeWells - creating feature failed" );
            
        }
    }
}

void uiGeopackageWriter::writePolyLines( TypeSet<MultiID>& lineids )
{
    if (gdalDS_ != nullptr) {
        OGRLayer* poLayerLines = nullptr;
        OGRLayer* poLayerPolygons = nullptr;

        OGRFieldDefn oField( "Name", OFTString );
        oField.SetWidth(32);
        
        for ( int idx=0; idx<lineids.size(); idx++ ) {
            Pick::Set ps;
            BufferString msg;
            IOObj* ioobj = IOM().get(lineids[idx]);
            if ( ioobj == nullptr ) {
                ErrMsg("uiGeopackageWriter::writePolyLines - cannot get ioobj" );
                continue;
            }
            if (!PickSetTranslator::retrieve( ps, ioobj, true, msg )) {
                BufferString tmp("uiGeopackageWriter::writePolyLines - error reading polyline - ");
                tmp += msg;
                ErrMsg(tmp);
                return;
            }
            if (ps.disp_.connect_ == Pick::Set::Disp::Close ) {
                if (poLayerPolygons == nullptr) {
                    poLayerPolygons = gdalDS_->CreateLayer( "Polygons", poSRS_, wkbPolygon, NULL );
                    if (poLayerPolygons == nullptr) {
                        ErrMsg("uiGeopackageWriter::writePolyLines - polygon layer creation failed");
                        return;
                    }
                    if( poLayerPolygons->CreateField( &oField ) != OGRERR_NONE ) {
                        ErrMsg("uiGeopackageWriter::writePolyLines - creating Name field in polygons layer failed" );
                        return;
                    }
                }
                OGRLinearRing ring;
                OGRFeature feature( poLayerPolygons->GetLayerDefn() );
                feature.SetField("Name", ps.name());
                for (int rdx=0; rdx<ps.size(); rdx++) {
                    const Coord3 pos = ps[rdx].pos();
                    ring.addPoint( pos.x, pos.y );
                }
                ring.addPoint(ps[0].pos().x, ps[0].pos().y);
                
                OGRPolygon poly;
                poly.addRing(&ring);
                feature.SetGeometry( &poly );
                if (poLayerPolygons->CreateFeature( &feature ) != OGRERR_NONE)
                    ErrMsg("uiGeopackageWriter::writePolyLines - creating polygon feature failed" );
                
            } else {
                if (poLayerLines == nullptr) {
                    poLayerLines = gdalDS_->CreateLayer( "PolyLines", poSRS_, wkbLineString, NULL );
                    if (poLayerLines == nullptr) {
                        ErrMsg("uiGeopackageWriter::writePolyLines - polyline layer creation failed");
                        return;
                    }
                    if( poLayerLines->CreateField( &oField ) != OGRERR_NONE ) {
                        ErrMsg("uiGeopackageWriter::writePolyLines - creating Name field in polylines layer failed" );
                        return;
                    }
                }
                OGRLineString line;
                OGRFeature feature( poLayerLines->GetLayerDefn() );
                feature.SetField("Name", ps.name());
                for (int rdx=0; rdx<ps.size(); rdx++) {
                    const Coord3 pos = ps[rdx].pos();
                    line.addPoint( pos.x, pos.y );
                }
                
                feature.SetGeometry( &line );
                if (poLayerLines->CreateFeature( &feature ) != OGRERR_NONE)
                    ErrMsg("uiGeopackageWriter::writePolyLines - creating polyline feature failed" );
            }
        }
    }    
}
