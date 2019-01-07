#ifndef avopolar_h
#define avopolar_h

#include "attribprovider.h"
#include "arrayndimpl.h"

namespace Attrib {
    
mClass(AVOPolar) AVOPolar : public Provider
{ mODTextTranslationClass(AVOPolar);
public:
    static void     initClass();
                    AVOPolar(Desc&);

    static const char*  attribName()    { return "AVOPolar"; }
    static const char*  stepoutStr()    { return "stepout"; }
    static const char*  gateBGStr()     { return "gateBG"; }
    static const char*  methodStr()     { return "method"; }
    static const char*  parallelStr()   { return "parallel";}
    
    enum Method { Normal, Eigen, ArrayFire };
    static const char* MethodNames[];
    
protected:
                        ~AVOPolar() {}
    static Provider*    createInstance(Desc&);
    static void         updateDefaults(Desc&);
                    
    bool                allowParallelComputation() const { return doparallel_; }
                    
    bool                getInputData(const BinID&,int zintv);
    void                prepPriorToBoundsCalc();
    bool                computeData(const DataHolder&,const BinID& relpos, int z0,int nrsamples,int threadid) const;

    bool                computeDataNormal(const DataHolder&,const BinID& relpos, int z0,int nrsamples,int threadid) const;
    bool                computeDataEigen(const DataHolder&,const BinID& relpos, int z0,int nrsamples,int threadid) const;
    bool                computeDataArrayFire(const DataHolder&,const BinID& relpos, int z0,int nrsamples,int threadid) const;
    
    
    void                computeBackgroundAngle(  const Array2DImpl<float>& intercept, 
                                                 const Array2DImpl<float>& gradient,
                                                 Array1DImpl<float>& result ) const;
                                                                                                                   
    const BinID*        desStepout(int,int) const;
    const Interval<int>* desZSampMargin(int,int) const;
    bool                getTrcPos();
    
    Interval<int>   sampgateBG_;
    Interval<float> gateBG_;
    BinID           stepout_;

    int             method_;
    bool            doparallel_;
    
    TypeSet<BinID>  trcpos_;
    int             centertrcidx_;
    
    ObjectSet<const DataHolder>   intercept_;
    ObjectSet<const DataHolder>   gradient_;
    int             intercept_idx_;
    int             gradient_idx_;
};

};

#endif

    
