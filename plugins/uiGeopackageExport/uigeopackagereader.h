#pragma once

#include "bufstring.h"
#include "manobjectset.h"
#include "polygon.h"

class GDALDataset;
class OGRSpatialReference;
class OGRLayer;
class BufferStringSet;
class BufferString;

class uiGeopackageReader
{
public:
    uiGeopackageReader();
    ~uiGeopackageReader();

    bool            open(const char* filename);
    void            close();

    const char*     fileName() const { return filename_; }
    const char*     layerName() const { return layername_; }
    bool            setLayer(const char* name);

    bool            isSameCRS(BufferString& errmsg);
    void            getLayers(BufferStringSet&);

    bool            getNextFeature(ManagedObjectSet<ODPolygon<Pos::Ordinate_Type>>& poly);
//    bool            getNextFeature(Coord& point);

protected:
    GDALDataset*            gdalDS_;
    OGRLayer*               gdalLayer_;

    BufferString            filename_;
    BufferString            layername_;
};
