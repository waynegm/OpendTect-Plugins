#pragma once
#include "mistiemod.h"

#include "arraynd.h"
#include "bufstring.h"
#include "multiid.h"
#include "paralleltask.h"
#include "threadlock.h"

#include "mistiecordata.h"

namespace EM {
    class Horizon2D;
    class Horizon3D;
}

mExpClass(Mistie) MistieApplyToHorizon2D : public ParallelTask
{ mODTextTranslationClass(MistieApplyToHorizon2D)
public:
    MistieApplyToHorizon2D(const MultiID, const MultiID, const TypeSet<Pos::GeomID>&, const BufferString);
    ~MistieApplyToHorizon2D();

    od_int64    nrIterations() const;
    uiString    uiMessage() const;
    uiString    uiNrDoneText() const;

protected:
    BufferString		mistiefile_;
    MultiID			oldhor2did_;
    MultiID			newhor2did_;
    const TypeSet<Pos::GeomID>&	geomids_;
    MistieCorrectionData	corrections_;
    Threads::Lock       	lock_;
    int		                counter_;
    EM::Horizon2D*		inphor_ = nullptr;
    EM::Horizon2D*		outhor_ = nullptr;

    bool        doWork(od_int64 start, od_int64 stop, int threadis);
    bool        doFinish(bool success);
    bool	doPrepare(int);
};

mExpClass(Mistie) MistieApplyToHorizon3D : public ParallelTask
{ mODTextTranslationClass(MistieApplyToHorizon3D)
public:
    MistieApplyToHorizon3D(const MultiID, const MultiID, const BufferString);
    ~MistieApplyToHorizon3D();

    od_int64    nrIterations() const;
    uiString    uiMessage() const;
    uiString    uiNrDoneText() const;

protected:
    BufferString		mistiefile_;
    MultiID			oldhor3did_;
    MultiID			newhor3did_;
    MistieCorrectionData	corrections_;
    Threads::Lock       	lock_;
    int		                counter_;
    EM::Horizon3D*		inphor_ = nullptr;
    EM::Horizon3D*		outhor_ = nullptr;
    Array2D<float>*		arr2d_ = nullptr;
    od_int64			nlocs_;

    bool        doWork(od_int64 start, od_int64 stop, int threadis);
    bool        doFinish(bool success);
    bool	doPrepare(int);
};
