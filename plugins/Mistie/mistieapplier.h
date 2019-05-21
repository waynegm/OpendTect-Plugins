#ifndef mistieapplier_h
#define mistieapplier_h

#include "mistiemod.h"
#include "attribprovider.h"
#include "arrayndimpl.h"

#include "mistiecordata.h"

namespace Attrib {
    
mClass(Mistie) MistieApplier : public Provider
{ mODTextTranslationClass(MistieApplier);
public:
    static void     initClass();
                    MistieApplier(Desc&);

    static const char*  attribName()    { return "MistieApplier"; }
    static const char*  shiftStr()     { return "ApplyShift"; }
    static const char*  phaseStr()     { return "ApplyPhase"; }
    static const char*  ampStr()     { return "ApplyAmp"; }
    static const char*  mistieFileStr() { return "mistie_database"; }
    
protected:
                        ~MistieApplier() {}
    static Provider*    createInstance(Desc&);
                    
    bool                allowParallelComputation() const { return true; }
                    
    bool                getInputData(const BinID&,int zintv);
    void                prepareForComputeData();
    bool                computeData(const DataHolder&,const BinID& relpos, int z0,int nrsamples,int threadid) const;
    const Interval<int>* desZSampMargin(int input,int output) const { return &zmargin_; }
    
    
    BufferString        mistiefile_;
    MistieCorrectionData corrections_;
    bool                applyshift_;
    bool                applyphase_;
    bool                applyamp_;
    const DataHolder*   data_;
    int                 dataidx_;
    Interval<int>       zmargin_;
    
    
    float               shift_;
    float               phase_;
    float               amp_;
};

};

#endif

    
