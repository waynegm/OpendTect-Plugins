#ifndef mistieestimator_h
#define mistieestimator_h

#include "paralleltask.h"
#include "threadlock.h"
#include "ranges.h"


#include "mistiemod.h"
#include "mistiedata.h"

class Line2DInterSectionSet;
class SeisTrc;
class IOObj;

mClass(uiMistie) MistieEstimator : public ParallelTask
{ mODTextTranslationClass(MistieEstimator)
public:
    MistieEstimator(const IOObj* ioobj, Line2DInterSectionSet& intset, ZGate window, float maxshift);
    ~MistieEstimator();

    od_int64    nrIterations() const;
    uiString    uiMessage() const;
    uiString    uiNrDoneText() const;
    
    const MistieData& getMisties() const { return misties_; }
    
protected:
    const IOObj*                ioobj_;
    MistieData                  misties_;
    ZGate                       window_;
    float                       maxshift_;
    Threads::Lock               lock_;
    int                         counter_;
    
    bool        doWork(od_int64 start, od_int64 stop, int threadis);
    bool        doFinish(bool success);
    
    bool        get2DTrc( BufferString line, int trcnr, SeisTrc& trc );
    float       calcAmpScalar( SeisTrc& trcA, SeisTrc& trcB );
    void        calcZandPhase( SeisTrc& trcA, SeisTrc& trcB, float& zdiff, float& phasediff, float& quality );
};

#endif
