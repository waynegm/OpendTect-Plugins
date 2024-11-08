#pragma once

#include "mistiemod.h"
#include "bufstringset.h"
#include "typeset.h"
#include "coord.h"

class MistieCorrectionData;
mExpClass(Mistie) MistieData
{
public:
    MistieData();
    MistieData( const MistieData& d) { *this = d; }
    ~MistieData();

    MistieData& operator=(const MistieData&);

    static const char* extStr()     { return "mistie"; }
    static const char* filtStr()    { return "*.mistie"; }
    static const char* defDirStr()  { return "Misc"; }

    bool            read( const char* filename, bool merge=false, bool replace=false );
    bool            write( const char* filename ) const;
    int             size() const;
    void            erase();

    void            add( const MistieData& other );
    bool            add( const char* dataA, int trcA, const char* dataB, int trcB, Coord pos,
                         float zdiff, float phasediff, float ampdiff , float quality, bool replace=false );
    bool            add( const char* dataA, int trcA, const char* dataB, int trcB, Coord pos );

    float           getZMistie( int idx ) const;
    float           getZMistieWith( const MistieCorrectionData& corrections, int idx ) const;
    float           getPhaseMistie( int idx ) const;
    float           getPhaseMistieWith( const MistieCorrectionData& corrections, int idx ) const;
    float           getAmpMistie( int idx ) const;
    float           getAmpMistieWith( const MistieCorrectionData& corrections, int idx ) const;
    float           getQuality( int idx ) const;
    Coord           getPos( int idx ) const;

    void            getAllLines( BufferStringSet& lnms ) const;
    bool            get( int idx, BufferString& dataA, int& trcA, BufferString& dataB, int& trcB ) const;
    bool            getLines( int idx, BufferString& dataA, BufferString& dataB ) const;
    bool            get( int idx, BufferString& dataA, BufferString& dataB, float& zdiff, float& phasediff, float& ampdiff ) const;
    bool            get( int idx, BufferString& dataA, int& trcA, BufferString& dataB, int& trcB, Coord& pos,
                         float& zdiff, float& phasediff, float& ampdiff, float& quality ) const;
    bool            getWith( const MistieCorrectionData& corrections, int idx,
                                          float& zdiff, float& phasediff, float& ampdiff ) const;

    bool            setPhaseMistie(int idx, float phasediff);
    bool            set( int idx, const char* dataA, int trcA, const char* dataB, int trcB, Coord pos,
                         float zdiff, float phasediff, float ampdiff , float quality );
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
