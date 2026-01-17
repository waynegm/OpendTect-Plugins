#pragma once

#include "multiid.h"
#include "paralleltask.h"
#include "ranges.h"
#include "threadlock.h"


#include "mistiemod.h"
#include "mistiedata.h"

class BendPoints;
class IOObj;
class SeisTrc;
class TrcKeySampling;
namespace EM { class Horizon2D; class Horizon3D; }
namespace Survey { class Geometry2D; }
namespace Seis { class RangeSelData; }

mExpClass(Mistie) Line3DOverlapFinder : public ParallelTask
{ mODTextTranslationClass(Line3DOverlapFinder)
public:
    Line3DOverlapFinder(const IOObj* ioobj3D, const ObjectSet<BendPoints>&);
    Line3DOverlapFinder(const EM::Horizon3D* hor3D, const ObjectSet<BendPoints>&);

    od_int64    nrIterations() const;
    uiString    uiMessage() const;
    uiString    uiNrDoneText() const;

    const ManagedObjectSet<Seis::RangeSelData>& selData() const	{ return selranges_; }

protected:
    const ObjectSet<BendPoints>&               bpoints_;
    Geom::PosRectangle<Pos::Ordinate_Type>     bounds3d_;
    ManagedObjectSet<Seis::RangeSelData>       selranges_;
    Threads::Lock lock_;

    void	setBounds(TrcKeySampling);
    bool        doWork(od_int64 start, od_int64 stop, int threadis);
    bool        doFinish(bool success);

};

mExpClass(Mistie) MistieEstimatorFromSeismic2D3D : public ParallelTask
{ mODTextTranslationClass(MistieEstimatorFromSeismic2D3D)
public:
    MistieEstimatorFromSeismic2D3D(const IOObj* ioobj3D, const IOObj* ioobj2D,
				   const ManagedObjectSet<Seis::RangeSelData>& selranges,
				   ZGate window, float maxshift, int trcstep=100,
				   bool allEst=true);
    ~MistieEstimatorFromSeismic2D3D();

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
    bool			allest_;
    Threads::Lock               lock_;
    int                         counter_;

    bool        doWork(od_int64 start, od_int64 stop, int threadis);
    bool        doFinish(bool success);

    bool        get2DTrc( BufferString line, int trcnr, SeisTrc& trc );
    bool        get3DTrc( int inl, int crl, SeisTrc& trc );
};

mExpClass(Mistie) MistieEstimatorFromHorizon2D3D : public ParallelTask
{ mODTextTranslationClass(MistieEstimatorFromHorizon2D3D)
public:
    MistieEstimatorFromHorizon2D3D(MultiID hor3Did, MultiID hor2Did,
				   const ManagedObjectSet<Seis::RangeSelData>& selranges,
				   int trcstep=100);
    ~MistieEstimatorFromHorizon2D3D();

    od_int64    nrIterations() const;
    uiString    uiMessage() const;
    uiString    uiNrDoneText() const;

    const MistieData& getMisties() const { return misties_; }

protected:
    MultiID		hor3did_;
    MultiID		hor2did_;
    EM::Horizon2D*	hor2d_ = nullptr;
    EM::Horizon3D*	hor3d_ = nullptr;
    const ManagedObjectSet<Seis::RangeSelData>& selranges_;
    int			trcstep_;

    MistieData                  misties_;
    ZGate                       window_;
    Threads::Lock               lock_;
    int                         counter_;

    virtual bool	doPrepare(int nrthreads) override;
    virtual bool        doWork(od_int64 start, od_int64 stop, int threadis) override;
    virtual bool        doFinish(bool success) override;

};
