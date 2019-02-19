#include "uigeopackagewriter.h"

#include "survinfo.h"
#include "survgeom2d.h"
#include "posinfo2d.h"
#include "coordsystem.h"
#include "crssystem.h"
#include "crsproj.h"
#include "bufstring.h"
#include "bufstringset.h"
#include "string2.h"
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
#include "emhorizon3d.h"
#include "emhorizon2d.h"
#include "emmanager.h"
#include "emsurfaceiodata.h"
#include "emsurfacetr.h"
#include "emioobjinfo.h"

#include "ogrsf_frmts.h"
#include "ogr_spatialref.h"

uiGeopackageWriter::uiGeopackageWriter( const char* filename, bool append )
: gdalDS_(nullptr), poSRS_(nullptr), append_(append)
{
    if (SI().getCoordSystem()->isProjection()) {
        const Coords::ProjectionBasedSystem* const proj = dynamic_cast<const Coords::ProjectionBasedSystem* const>(SI().getCoordSystem().ptr());
        poSRS_= new OGRSpatialReference();
        poSRS_->importFromEPSG(proj->getProjection()->authCode().id());
        open(filename);
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

bool uiGeopackageWriter::open( const char* filename )
{
    const char* papszAllowedDrivers[] = { "GPKG", NULL };
    
    GDALAllRegister();
    
    if (append_) {
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
    
        OGRLayer* poLayer = nullptr;
        if (append_)
            poLayer = gdalDS_->GetLayerByName( "Survey" );

        if (poLayer == nullptr) {
            poLayer = gdalDS_->CreateLayer( "Survey", poSRS_, wkbPolygon, NULL );
            if (poLayer == nullptr) {
                ErrMsg("uiGeopackageWriter::writeSurvey - creation of Survey layer failed");
                return;
            }
            OGRFieldDefn oField( "Name", OFTString );
            oField.SetWidth(32);
            if( poLayer->CreateField( &oField ) != OGRERR_NONE ) {
                ErrMsg("uiGeopackageWriter::writeSurvey - creating Name field failed" );
                return;
            }
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
        OGRLayer* poLayer = nullptr;
        if (append_)
            poLayer = gdalDS_->GetLayerByName( "2DLines" );

        if (poLayer == nullptr) {
            poLayer = gdalDS_->CreateLayer( "2DLines", poSRS_, wkbLineString, NULL );
            if (poLayer == nullptr) {
                ErrMsg("uiGeopackageWriter::write2DLines - creation of 2DLines layer failed");
                return;
            }
            OGRFieldDefn oField( "LineName", OFTString );
            oField.SetWidth(32);
            if( poLayer->CreateField( &oField ) != OGRERR_NONE ) {
                ErrMsg("uiGeopackageWriter::write2DLines - creation of LineName field failed" );
                return;
            }
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
        OGRLayer* poLayer = nullptr;
        if (append_)
            poLayer = gdalDS_->GetLayerByName( "2DStations" );

        if (poLayer == nullptr) {
            poLayer = gdalDS_->CreateLayer( "2DStations", poSRS_, wkbPoint, NULL );
            if (poLayer == nullptr) {
                ErrMsg("uiGeopackageWriter::write2DStations - creation of 2DStations layer failed");
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
        }
        
        for ( int idx=0; idx<geomids.size(); idx++ ) {
            mDynamicCastGet( const Survey::Geometry2D*, geom2d, Survey::GM().getGeometry(geomids[idx]) );
            if ( !geom2d )
                continue;
            const PosInfo::Line2DData& geom = geom2d->data();
            const TypeSet<PosInfo::Line2DPos>& posns = geom.positions();
            
            if (gdalDS_->StartTransaction() == OGRERR_FAILURE) {
                ErrMsg("uiGeopackageWriter::write2DStations - starting transaction for writing 2DStation layer failed" );
                continue;
            }
            for ( int tdx=0; tdx<posns.size(); tdx++ ) {
                OGRFeature feature( poLayer->GetLayerDefn() );
                feature.SetField("LineName", geom2d->getName());
                feature.SetField("Station", posns[tdx].nr_);
                
                OGRPoint pt;
                Coord pos = posns[tdx].coord_;
                pt.setX(pos.x);
                pt.setY(pos.y);
                feature.SetGeometry( &pt );
                if (poLayer->CreateFeature( &feature ) != OGRERR_NONE) {
                    ErrMsg("uiGeopackageWriter::write2DStations - creating feature failed" );
                    gdalDS_->RollbackTransaction();
                    break;
                }
            }
            if (gdalDS_->CommitTransaction() == OGRERR_FAILURE) {
                ErrMsg("uiGeopackageWriter::write2DStations - transaction commit for 2DStations layer failed" );
                return;
            }
        }
    }
}

void uiGeopackageWriter::writeRandomLines( TypeSet<MultiID>& lineids )
{
    if (gdalDS_ != nullptr) {
        OGRLayer* poLayer = nullptr;
        if (append_)
            poLayer = gdalDS_->GetLayerByName( "RandomLines" );

        if (poLayer == nullptr) {
            poLayer = gdalDS_->CreateLayer( "RandomLines", poSRS_, wkbLineString, NULL );
            if (poLayer == nullptr) {
                ErrMsg("uiGeopackageWriter::writeRandomLines - creation of RandomLines layer failed");
                return;
            }
            OGRFieldDefn oField( "LineName", OFTString );
            oField.SetWidth(32);
            if( poLayer->CreateField( &oField ) != OGRERR_NONE ) {
                ErrMsg("uiGeopackageWriter::writeRandomLines - creating Line Name field failed" );
                return;
            }
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
        OGRLayer* poLayer = nullptr;
        if (append_)
            poLayer = gdalDS_->GetLayerByName( "Wells" );
        
        if (poLayer == nullptr) {
            poLayer = gdalDS_->CreateLayer( "Wells", poSRS_, wkbPoint, NULL );
            if (poLayer == nullptr) {
                ErrMsg("uiGeopackageWriter::writeWells - creation of Wells layer failed");
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
        if (append_) {
            poLayerLines = gdalDS_->GetLayerByName( "PolyLines" );
            poLayerPolygons = gdalDS_->GetLayerByName( "Polygons" );
        }
            
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
                        ErrMsg("uiGeopackageWriter::writePolyLines - creation of Polygons layer failed");
                        return;
                    }
                    if( poLayerPolygons->CreateField( &oField ) != OGRERR_NONE ) {
                        ErrMsg("uiGeopackageWriter::writePolyLines - creating Name field in Polygons layer failed" );
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
                        ErrMsg("uiGeopackageWriter::writePolyLines - creation of PolyLines layer failed");
                        return;
                    }
                    if( poLayerLines->CreateField( &oField ) != OGRERR_NONE ) {
                        ErrMsg("uiGeopackageWriter::writePolyLines - creating Name field in PolyLines layer failed" );
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

void uiGeopackageWriter::writeHorizon( const char* layerName, const MultiID& hor2Dkey, const char* attrib2D, const TypeSet<Pos::GeomID>& geomids, 
                                       const MultiID& hor3Dkey, const char* attrib3D, const TrcKeyZSampling& cs  )
{
    if (gdalDS_ != nullptr) {
        BufferString attrib("Z Values");
        if (!hor2Dkey.isUdf() && attrib2D != nullptr)
            attrib = attrib2D;
        if (!hor3Dkey.isUdf() && attrib3D != nullptr) {
            if (attrib2D == nullptr)
                attrib = attrib3D;
            else if (!caseInsensitiveEqual(attrib2D, attrib3D)) {
                attrib += "_";
                attrib += attrib3D;
            }
        }

        OGRLayer* poLayer = nullptr;
        if (append_)
            poLayer = gdalDS_->GetLayerByName( layerName );
        
        if (poLayer == nullptr) {
            poLayer = gdalDS_->CreateLayer( layerName, poSRS_, wkbPoint, NULL );
            if (poLayer == nullptr) {
                BufferString tmp("uiGeopackageWriter::writeHorizon - creation of ");
                tmp += layerName;
                tmp += " layer failed";
                ErrMsg(tmp);
                return;
            }

            OGRFieldDefn oZField( attrib, OFTReal );
            if( poLayer->CreateField( &oZField ) != OGRERR_NONE ) {
                BufferString tmp("uiGeopackageWriter::writeHorizon - creation of ");
                tmp += attrib;
                tmp += " field failed";
                ErrMsg(tmp);
                return;
            }
        }
        
        const float zfac = SI().zIsTime() ? 1000 : 1;
        
        if (!hor2Dkey.isUdf() && geomids.size()>0) {
            EM::EMObject* obj = EM::EMM().loadIfNotFullyLoaded(hor2Dkey);
            if (obj==nullptr) {
                ErrMsg("uiGeopackageWriter::writeHorizon - loading 2D horizon failed");
                return;
            }
            obj->ref();
            mDynamicCastGet(EM::Horizon2D*,hor,obj);
            if (hor==nullptr) {
                ErrMsg("uiGeopackageWriter::writeHorizon - casting 2D horizon failed");
                obj->unRef();
                return;
            }
            for (int idx=0; idx<geomids.size(); idx++) {
                const StepInterval<int> trcrg = hor->geometry().colRange( geomids[idx] );
                mDynamicCastGet(const Survey::Geometry2D*,survgeom2d,Survey::GM().getGeometry(geomids[idx]))
                if (!survgeom2d || trcrg.isUdf() || !trcrg.step)
                    continue;
                
                TrcKey tk( geomids[idx], -1 );
                Coord crd;
                int spnr = mUdf(int);

                if (gdalDS_->StartTransaction() == OGRERR_FAILURE) {
                    ErrMsg("uiGeopackageWriter::writeHorizon - starting transaction for writing 2D horizon failed" );
                    obj->unRef();
                    continue;
                }
                for ( int trcnr=trcrg.start; trcnr<=trcrg.stop; trcnr+=trcrg.step ) {
                    tk.setTrcNr( trcnr );
                    const float z = hor->getZ( tk );
                    if (mIsUdf(z))
                        continue;
                    const float scaledZ = z*zfac;
                    
                    survgeom2d->getPosByTrcNr( trcnr, crd, spnr );
                    
                    OGRFeature feature( poLayer->GetLayerDefn() );
                    feature.SetField(attrib, scaledZ);
                    OGRPoint pt(crd.x, crd.y);
                    feature.SetGeometry( &pt );
                    if (poLayer->CreateFeature( &feature ) != OGRERR_NONE) {
                        ErrMsg("uiGeopackageWriter::writeHorizon - creating point for 2D horizon failed" );
                        gdalDS_->RollbackTransaction();
                        obj->unRef();
                        break;
                    }
                }
                if (gdalDS_->CommitTransaction() == OGRERR_FAILURE) {
                    ErrMsg("uiGeopackageWriter::writeHorizon - transaction commit for 2D horizon failed" );
                    obj->unRef();
                    return;
                }
            }
            obj->unRef();
        }
        
        if (!hor3Dkey.isUdf()) {
            EM::EMObject* obj = EM::EMM().loadIfNotFullyLoaded(hor3Dkey);
            if (obj==nullptr) {
                ErrMsg("uiGeopackageWriter::writeHorizon - loading 3D horizon failed");
                return;
            }
            obj->ref();
            mDynamicCastGet(EM::Horizon3D*,hor,obj);
            if (hor==nullptr) {
                ErrMsg("uiGeopackageWriter::writeHorizon - casting 3D horizon failed");
                obj->unRef();
                return;
            }

            if (gdalDS_->StartTransaction() == OGRERR_FAILURE) {
                ErrMsg("uiGeopackageWriter::writeHorizon - starting transaction for writing 3D horizon failed" );
                obj->unRef();
                return;
            }
            TrcKeySampling expSel = cs.hsamp_;
            for (int iln=expSel.start_.inl(); iln<=expSel.stop_.inl(); iln+=expSel.step_.inl()) {
                for (int xln=expSel.start_.crl(); xln<=expSel.stop_.crl(); xln+=expSel.step_.crl()) {
                    BinID bid(iln,xln);
                    TrcKey tk(bid);
                    const float z = hor->getZ( tk );
                    if (mIsUdf(z))
                        continue;
                    const float scaledZ = z*zfac;
                    
                    Coord coord;
                    coord = SI().transform(bid);
                    OGRFeature feature( poLayer->GetLayerDefn() );
                    feature.SetField(attrib, scaledZ);
                    OGRPoint pt(coord.x, coord.y);
                    feature.SetGeometry( &pt );
                    if (poLayer->CreateFeature( &feature ) != OGRERR_NONE) {
                        ErrMsg("uiGeopackageWriter::writeHorizon - creating point for 3D horizon failed" );
                        gdalDS_->RollbackTransaction();
                        obj->unRef();
                        break;
                    }
                }
            }
            if (gdalDS_->CommitTransaction() == OGRERR_FAILURE) {
                ErrMsg("uiGeopackageWriter::writeHorizon - transaction commit for 3D horizon failed" );
                obj->unRef();
                return;
            }
            obj->unRef();
        }
    }
}
