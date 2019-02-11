#ifndef uigeopackagewriter_h
#define uigeopackagewriter_h

#include "typeset.h"

class GDALDataset;
class OGRSpatialReference;
class BufferStringSet;

class uiGeopackageWriter
{
public:
    uiGeopackageWriter( const char* filename=0, bool append=true);
    ~uiGeopackageWriter();
    
    bool    open(const char* filename, bool append);
    void    close();
    
    void    writeSurvey();
    void    write2DLines( TypeSet<Pos::GeomID>& geomids );
    void    write2DStations( TypeSet<Pos::GeomID>& geomids );
    void    writeRandomLines( TypeSet<MultiID>& lineids );
    void    writeWells( TypeSet<MultiID>& wellids );
    void    writePolyLines( TypeSet<MultiID>& lineids );
    
protected:
    GDALDataset*    gdalDS_;
    OGRSpatialReference* poSRS_;
};

#endif
