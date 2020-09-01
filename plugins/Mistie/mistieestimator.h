#ifndef mistieestimator_h
#define mistieestimator_h

#include "multiid.h"
#include "paralleltask.h"
#include "threadlock.h"
#include "ranges.h"


#include "mistiemod.h"
#include "mistiedata.h"

class Line2DInterSectionSet;
class SeisTrc;
class IOObj;

mExpClass(Mistie) MistieEstimatorFromSeismic : public ParallelTask
{ mODTextTranslationClass(MistieEstimatorFromSeismic)
public:
    MistieEstimatorFromSeismic(const IOObj* ioobj, Line2DInterSectionSet& intset, ZGate window, float maxshift, bool allEst=true);
    ~MistieEstimatorFromSeismic();

    od_int64    nrIterations() const;
    uiString    uiMessage() const;
    uiString    uiNrDoneText() const;
    
    const MistieData& getMisties() const { return misties_; }
    
protected:
    const IOObj*                ioobj_;
    MistieData                  misties_;
    ZGate                       window_;
    float                       maxshift_;
    bool			allest_;
    Threads::Lock               lock_;
    int                         counter_;
    
    bool        doWork(od_int64 start, od_int64 stop, int threadis);
    bool        doFinish(bool success);
    
    bool        get2DTrc( BufferString line, int trcnr, SeisTrc& trc );
};

mExpClass(Mistie) MistieEstimatorFromHorizon : public ParallelTask
{ mODTextTranslationClass(MistieEstimatorFromHorizon)
public:
    MistieEstimatorFromHorizon(MultiID hor2Did, Line2DInterSectionSet& intset);
    ~MistieEstimatorFromHorizon();

    od_int64    nrIterations() const;
    uiString    uiMessage() const;
    uiString    uiNrDoneText() const;

    const MistieData& getMisties() const { return misties_; }

protected:
    MultiID	                hor2did_;
    MistieData                  misties_;
    Threads::Lock               lock_;
    int                         counter_;

    bool        doWork(od_int64 start, od_int64 stop, int threadis);
    bool        doFinish(bool success);
};


#endif
