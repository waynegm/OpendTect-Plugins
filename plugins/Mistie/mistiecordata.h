#ifndef mistiecordata_h
#define mistiecordata_h

#include "mistiemod.h"
#include "bufstringset.h"
#include "typeset.h"

mClass(Mistie) MistieCorrectionData
{
public:
    MistieCorrectionData();
    ~MistieCorrectionData();
    
    static const char* extStr()     { return "mst"; }
    static const char* filtStr()    { return "*.mst"; }
    static const char* defDirStr()  { return "Misc"; }
    
    bool            read( const char* filename );
    bool            write( const char* filename ) const;
    int             size() const;
    void            erase();
    
    BufferString    getDataName( int idx ) const;
    float           getZCor( int idx ) const;
    float           getPhaseCor( int idx ) const;
    float           getAmpCor( int idx ) const;
    
    bool            get( const char* dataname, float& shift, float& phase, float& amp ) const;
    void            set( const char* dataname, float shift, float phase, float amp );
    
protected:
    BufferStringSet     datanames_;
    TypeSet<float>      shifts_;
    TypeSet<float>      phases_;
    TypeSet<float>      amps_;
};

#endif
