#ifndef avopolarattrib_h
#define avopolarattrib_h

#include "attribprovider.h"

#include "Eigen/Core"

namespace Attrib {
    
mClass(AVOPolarAttrib) AVOPolarAttrib : public Provider
{ mODTextTranslationClass(AVOPolarAttrib);
public:
    static void     initClass();
                    AVOPolarAttrib(Desc&);

    static const char*  attribName()    { return "AVOPolarAttrib"; }
    static const char*  stepoutStr()    { return "stepout"; }
    static const char*  gateBGStr()     { return "gateBG"; }
    static const char*  gateStr()       { return "gate"; }
    
    enum Output { BGAngle, EventAngle, Difference, Strength, Product, Quality };
    
protected:
                        ~AVOPolarAttrib() {}
    static Provider*    createInstance(Desc&);
    static void         updateDefaults(Desc&);
                    
    bool                allowParallelComputation() const { return true; }
                    
    bool                getInputData(const BinID&,int zintv);
    void                prepPriorToBoundsCalc();
    bool                computeData(const DataHolder&,const BinID& relpos, int z0,int nrsamples,int threadid) const;

    void                computeBackgroundAngle(  const Eigen::ArrayXXd& intercept, const Eigen::ArrayXXd& gradient, Eigen::ArrayXd& result ) const;
    void                computeEventAngle(  const Eigen::ArrayXXd& intercept, const Eigen::ArrayXXd& gradient, Eigen::ArrayXd& angle, Eigen::ArrayXd& quality ) const;
    void                computeStrength(  const Eigen::ArrayXXd& intercept, const Eigen::ArrayXXd& gradient, Eigen::ArrayXd& result ) const;
    
    
    const BinID*            desStepout(int,int) const;
    const Interval<int>*    desZSampMargin(int,int) const;
    bool                    getTrcPos();
    
    Interval<int>   sampgateBG_;
    Interval<float> gateBG_;
    BinID           stepout_;

    Interval<int>   sampgate_;
    Interval<float> gate_;
    
    TypeSet<BinID>  trcpos_;
    int             centertrcidx_;
    
    ObjectSet<const DataHolder>   intercept_;
    ObjectSet<const DataHolder>   gradient_;
    int             intercept_idx_;
    int             gradient_idx_;
};

};

#endif

    
