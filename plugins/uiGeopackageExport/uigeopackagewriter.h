#ifndef uigeopackagewriter_h
#define uigeopackagewriter_h

#include "typeset.h"

class GeopackageIO;
namespace Coords{ class AuthorityCode; }
class TrcKeyZSampling;

class uiGeopackageWriter
{
public:
    uiGeopackageWriter( const char* filename=0, bool append=false);
    ~uiGeopackageWriter();

    bool    open(const char* filename);
    void    close();

    void    writeSurvey();
    void    write2DLines( TypeSet<Pos::GeomID>& geomids );
    void    write2DStations( TypeSet<Pos::GeomID>& geomids );
    void    writeRandomLines( TypeSet<MultiID>& lineids );
    void    writeWells( TypeSet<MultiID>& wellids );
    void    writeWellTracks( TypeSet<MultiID>& wellids );
    void    writeWellMarkers( TypeSet<MultiID>& wellids, bool inFeet=false );
    void    writePolyLines( TypeSet<MultiID>& lineids );
    void    writeHorizon( const char* layerName, const MultiID& hor2Dkey, const char* attrib2D, const TypeSet<Pos::GeomID>& geomids,
                          const MultiID& hor3Dkey, const char* attrib3D, const TrcKeyZSampling& cs  );

protected:
    GeopackageIO*		gpkg_ = nullptr;
    Coords::AuthorityCode*	srs_ = nullptr;
    bool			append_;
};

#endif
