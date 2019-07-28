#include "uigeopackagereader.h"

#include "bufstringset.h"
#include "errmsg.h"
#include "survinfo.h"
#include "coordsystem.h"
#include "crssystem.h"
#include "crsproj.h"

#include "ogrsf_frmts.h"
#include "ogr_spatialref.h"

uiGeopackageReader::uiGeopackageReader()
: gdalDS_(nullptr), gdalLayer_(0)
{
}

uiGeopackageReader::~uiGeopackageReader()
{
    close();
}

bool uiGeopackageReader::open( const char* filename )
{
    if (filename_ != filename)
        close();
    
    if (!gdalDS_) {
        const char* papszAllowedDrivers[] = { "GPKG", NULL };
        GDALAllRegister();
        gdalDS_ = GDALDataset::Open(filename, GDAL_OF_VECTOR, papszAllowedDrivers, NULL, NULL);
        if (!gdalDS_) {
            ErrMsg("uiGeopackageReader::open - cannot open input file.");
            return false;
        }
        filename_ = filename;
    }
    return true;
}

void uiGeopackageReader::close()
{
    if (gdalDS_) 
        GDALClose( gdalDS_ );
    gdalDS_ = 0;
    gdalLayer_ = 0;
    filename_.setEmpty();
    layername_.setEmpty();
}

void uiGeopackageReader::getLayers(BufferStringSet& layernms)
{
    layernms.erase();
    if (gdalDS_) {
        for( auto&& poLayer: gdalDS_->GetLayers() ) {
            layernms.add(poLayer->GetName());
        }
    }
}

bool uiGeopackageReader::setLayer(const char* name)
{
    if (gdalDS_) {
        gdalLayer_ = gdalDS_->GetLayerByName(name);
        if (gdalLayer_) {
            layername_ = name;
            gdalLayer_->ResetReading();
            return true;
        }
    }
    return false;
}

bool uiGeopackageReader::isSameCRS(BufferString& errmsg)
{
    if (gdalLayer_) {
        const OGRSpatialReference* poFileSRS = gdalLayer_->GetSpatialRef();
        if (!poFileSRS) {
            errmsg += "Retrieving CRS from Geopackage file failed \n";
            return false;
        }
        if (poFileSRS->IsProjected() && SI().getCoordSystem()->isProjection()) {
            const Coords::ProjectionBasedSystem* const proj = dynamic_cast<const Coords::ProjectionBasedSystem* const>(SI().getCoordSystem().ptr());
            BufferString odtAuthCode;
            odtAuthCode += proj->getProjection()->authCode().id();
            BufferString fileAuthCode = poFileSRS->GetAuthorityCode("PROJCS");
            errmsg += "Geopackage SRS: ";
            errmsg += fileAuthCode;
            errmsg += " ODT SRS: ";
            errmsg += odtAuthCode;
            return odtAuthCode == fileAuthCode;
        }
    }
    return false;
}

bool uiGeopackageReader::getNextFeature(ManagedObjectSet<ODPolygon<Pos::Ordinate_Type>>& poly)
{
    bool res = false;
    poly.erase();
    if (gdalLayer_) {
        OGRFeature* poFeature = gdalLayer_->GetNextFeature();
        if (poFeature) {
            OGRGeometry* poGeom = poFeature->GetGeometryRef();
            if (poGeom) {
                OGRwkbGeometryType wkbType = wkbFlatten(poGeom->getGeometryType());
                if (wkbType == wkbLineString) {
                    ODPolygon<Pos::Ordinate_Type>* tmp = new ODPolygon<Pos::Ordinate_Type>;
                    OGRLineString* poLine = (OGRLineString*) poGeom;
                    for (int ip=0; ip<poLine->getNumPoints(); ip++)
                        tmp->add(Coord(poLine->getX(ip), poLine->getY(ip)));
                    tmp->setClosed(false);
					tmp->keepBendPoints(1.0);
                    poly += tmp;
                    res = true;
                } else if (wkbType == wkbMultiLineString) {
                    OGRMultiLineString* poMultiLine = (OGRMultiLineString*) poGeom;
                    for (int il=0; il<poMultiLine->getNumGeometries(); il++) {
                        OGRLineString* poLine = (OGRLineString*)poMultiLine->getGeometryRef(il);
                        ODPolygon<Pos::Ordinate_Type>* tmp = new ODPolygon<Pos::Ordinate_Type>;
                        for (int ip=0; ip<poLine->getNumPoints(); ip++)
                            tmp->add(Coord(poLine->getX(ip), poLine->getY(ip)));
                        tmp->setClosed(false);
						tmp->keepBendPoints(1.0);
                        poly += tmp;
                    }
                    res = true;
                } else if (wkbType == wkbPolygon) {
                    OGRPolygon* poPoly = (OGRPolygon*) poGeom;
                    OGRLinearRing* poExtRing = poPoly->getExteriorRing();
                    ODPolygon<Pos::Ordinate_Type>* tmp = new ODPolygon<Pos::Ordinate_Type>;
                    for (int ip=0; ip<poExtRing->getNumPoints(); ip++)
                        tmp->add(Coord(poExtRing->getX(ip), poExtRing->getY(ip)));
                    tmp->setClosed(true);
                    poly += tmp;
                    for (int il=0; il<poPoly->getNumInteriorRings(); il++) {
                        OGRLinearRing* poRing = poPoly->getInteriorRing(il);
                        tmp = new ODPolygon<Pos::Ordinate_Type>;
                        for (int ip=0; ip<poRing->getNumPoints(); ip++)
                            tmp->add(Coord(poRing->getX(ip), poRing->getY(ip)));
                        tmp->setClosed(true);
                        poly += tmp;
                    }
                    res = true;
                } else if (wkbType == wkbMultiPolygon) {
                    ErrMsg("uiGeopackageReader::getNextFeature - reading MultiPolygon geometries not supported"); 
                    res = false;
                }
            }
      }
        OGRFeature::DestroyFeature(poFeature);
    }
    return res;
}


