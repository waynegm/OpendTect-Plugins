#ifndef uigeotiffwriter_h
#define uigeotiffwriter_h

#include "typeset.h"

class GDALDataset;
class OGRSpatialReference;
class BufferStringSet;
class TrcKeyZSampling;
class uiTaskRunner;

class uiGeotiffWriter
{ 
public:
    uiGeotiffWriter( const char* filename=0 );
    ~uiGeotiffWriter();
    
    void    setFileName( const char* filename ); 
    
    bool    writeHorizon( uiTaskRunner& taskrunner, const MultiID& hor3Dkey, bool exportZ, const BufferStringSet& attribs );
    
protected:
    void    close();

    GDALDataset*            gdalDS_;
    OGRSpatialReference*    poSRS_;
    BufferString            filename_;
};

#endif
