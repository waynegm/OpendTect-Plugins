#ifndef st_mai_sao_h
#define st_mai_sao_h

#include "attribprovider.h"
#include "arrayndimpl.h"

namespace Attrib {
    
mClass(ST_MAI_SAO) ST_MAI_SAO : public Provider
{ mODTextTranslationClass(ST_MAI_SAO);
public:
    static void     initClass();
                    ST_MAI_SAO(Desc&);

    static const char*  attribName()    { return "ST_MAI_SAO"; }
    static const char*  gateStr()       { return "gate"; }
    
protected:
                        ~ST_MAI_SAO() {}
    static Provider*    createInstance(Desc&);
    static void         updateDefaults(Desc&);
                    
    bool                allowParallelComputation() const { return false; }
                    
    bool                getInputData(const BinID&,int zintv);
    void                prepPriorToBoundsCalc();
    bool                computeData(const DataHolder&,const BinID& relpos, int z0,int nrsamples,int threadid) const;
    
    void                computeOutput( const Array1DImpl<float>& inp1, 
                                       const Array1DImpl<float>& inp2,
                                       const Array1DImpl<float>& inp3,
                                       Array1DImpl<float>& result ) const;

    const Interval<int>*    desZSampMargin(int,int) const;
    
    Interval<int>   sampgate_;
    Interval<float> gate_;
    
    const DataHolder*   input_1_;
    const DataHolder*   input_2_;
    const DataHolder*   input_3_; // Add more/less as required
    int             input_1_idx_;
    int             input_2_idx_;
    int             input_3_idx_;   // Add more/less as required
};

};

#endif

    
