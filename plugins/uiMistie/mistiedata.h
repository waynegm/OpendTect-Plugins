#ifndef mistiedata_h
#define mistiedata_h

#include "mistiemod.h"
#include "bufstringset.h"
#include "typeset.h"
#include "coord.h"

mClass(Mistie) MistieData
{
public:
    MistieData();
    MistieData( const MistieData& d) { *this = d; } 
    ~MistieData();
    
    MistieData& operator=(const MistieData&);
    
    static const char* extStr()     { return "misties"; }
    static const char* filtStr()    { return "*.misties"; }
    static const char* defDirStr()  { return "Misc"; }
    
    bool            read( const char* filename, bool merge=false );
    bool            write( const char* filename ) const;
    int             size() const;
    void            erase();
    
    bool            add( const char* dataA, int trcA, const char* dataB, int trcB, Coord pos, float zdiff, float phasediff, float ampdiff , float quality );
    bool            add( const char* dataA, int trcA, const char* dataB, int trcB, Coord pos );

    bool            get( int idx, BufferString& dataA, int& trcA, BufferString& dataB, int& trcB, Coord& pos, float& zdiff, float& phasediff, float& ampdiff, float& quality ) const;
    bool            get( int idx, BufferString& dataA, int& trcA, BufferString& dataB, int& trcB ) const;

    bool            set( int idx, const char* dataA, int trcA, const char* dataB, int trcB, Coord pos, float zdiff, float phasediff, float ampdiff , float quality );
    bool            set( int idx, float zdiff, float phasediff, float ampdiff, float quality );
    
protected:
    BufferStringSet     dataA_;
    TypeSet<int>        trcA_;
    BufferStringSet     dataB_;
    TypeSet<int>        trcB_;
    TypeSet<Coord>      pos_;
    TypeSet<float>      zdiff_;
    TypeSet<float>      phasediff_;
    TypeSet<float>      ampdiff_;
    TypeSet<float>      quality_;
};

#endif
