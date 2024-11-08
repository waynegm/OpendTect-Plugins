#pragma once

#include "mistiemod.h"
#include "bufstringset.h"
#include "typeset.h"

class MistieData;

mExpClass(Mistie) MistieCorrectionData
{
public:
    MistieCorrectionData();
    ~MistieCorrectionData();

    static const char* extStr()     { return "miscor"; }
    static const char* filtStr()    { return "*.miscor"; }
    static const char* defDirStr()  { return "Misc"; }

    bool            read( const char* filename );
    bool            write( const char* filename ) const;
    int             size() const;
    void            erase();

    int             getIndex( const char* dataname) const;
    BufferString    getDataName( int idx ) const;
    float           getZCor( int idx ) const;
    float           getPhaseCor( int idx ) const;
    float           getAmpCor( int idx ) const;

    void            setDataName( int idx, const char* dataname );
    void            setZCor( int idx, float zcor );
    void            setZCor( const char* dataname, float zcor );
    void            setPhaseCor( int idx, float phasecor );
    void            setPhaseCor( const char* dataname, float phasecor );
    void            setAmpCor( int idx, float ampcor );
    void            setAmpCor( const char* dataname, float ampcor );

    bool            get( const char* dataname, float& shift, float& phase, float& amp ) const;
    void            set( const char* dataname, float shift, float phase, float amp );

    float           computeZCor( const MistieData& misties, const BufferStringSet& reference, float minQuality=0.5, int maxIter=20, float damping=0.75, float delta=0.01 );
    float           computePhaseCor( const MistieData& misties, const BufferStringSet& reference, float minQuality=0.5, int maxIter=20, float damping=0.75, float delta=0.001 );
    float           computeAmpCor( const MistieData& misties, const BufferStringSet& reference, float minQuality=0.5, int maxIter=20, float damping=0.75, float delta=0.001 );

protected:
    BufferStringSet     datanames_;
    TypeSet<float>      shifts_;
    TypeSet<float>      phases_;
    TypeSet<float>      amps_;
};
