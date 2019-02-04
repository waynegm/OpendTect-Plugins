#ifndef uigeopackagewriter_h
#define uigeopackagewriter_h

class GDALDataset;
class OGRSpatialReference;

class uiGeopackageWriter
{
public:
//    static void initClass();
    
    uiGeopackageWriter( const char* filename=0, bool append=true);
    ~uiGeopackageWriter();
    
    bool    open(const char* filename, bool append);
    void    close();
    
    void    writeSurvey();
    
protected:
    GDALDataset*    gdalDS_;
    OGRSpatialReference* poSRS_;
};



#endif
