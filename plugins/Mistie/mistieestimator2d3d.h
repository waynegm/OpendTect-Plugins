#ifndef mistieestimator2d3d_h
#define mistieestimator2d3d_h

#include "paralleltask.h"
#include "threadlock.h"
#include "ranges.h"


#include "mistiemod.h"
#include "mistiedata.h"

class SeisTrc;
class IOObj;
namespace Survey { class Geometry2D; }
namespace Seis { class RangeSelData; }
class BendPoints;

mExpClass(Mistie) Line3DOverlapFinder : public ParallelTask
{ mODTextTranslationClass(Line3DOverlapFinder)
public:
    Line3DOverlapFinder(const IOObj* ioobj3D, const ObjectSet<BendPoints>&);

    od_int64    nrIterations() const;
    uiString    uiMessage() const;
    uiString    uiNrDoneText() const;
    
    const ManagedObjectSet<Seis::RangeSelData>& selData() const	{ return selranges_; }
    
protected:
    const IOObj*                               ioobj3d_;
    const ObjectSet<BendPoints>&               bpoints_;
    Geom::PosRectangle<Pos::Ordinate_Type>     bounds3d_;
    ManagedObjectSet<Seis::RangeSelData>       selranges_;
    Threads::Lock lock_;    
    
    bool        doWork(od_int64 start, od_int64 stop, int threadis);
    bool        doFinish(bool success);
    
};

mClass(Mistie) MistieEstimator2D3D : public ParallelTask
{ mODTextTranslationClass(MistieEstimator2D3D)
public:
    MistieEstimator2D3D(const IOObj* ioobj3D, const IOObj* ioobj2D, const ManagedObjectSet<Seis::RangeSelData>& selranges, ZGate window, float maxshift, int trcstep=100);
    ~MistieEstimator2D3D();

    od_int64    nrIterations() const;
    uiString    uiMessage() const;
    uiString    uiNrDoneText() const;
    
    const MistieData& getMisties() const { return misties_; }
    
protected:
    const IOObj*                        ioobj2d_;
    const IOObj*                        ioobj3d_;
    const ManagedObjectSet<Seis::RangeSelData>& selranges_;
    int                                 trcstep_;
    
    MistieData                  misties_;
    ZGate                       window_;
    float                       maxshift_;
    Threads::Lock               lock_;
    int                         counter_;
    
    bool        doWork(od_int64 start, od_int64 stop, int threadis);
    bool        doFinish(bool success);
    
    bool        get2DTrc( BufferString line, int trcnr, SeisTrc& trc );
    bool        get3DTrc( int inl, int crl, SeisTrc& trc );
};

#endif
