#ifndef st_mai_mao_h
#define st_mai_mao_h

#include "attribprovider.h"
#include "arrayndimpl.h"

namespace Attrib {
    
mClass(ST_MAI_MAO) ST_MAI_MAO : public Provider
{ mODTextTranslationClass(ST_MAI_MAO);
public:
    static void     initClass();
                    ST_MAI_MAO(Desc&);

    static const char*  attribName()    { return "ST_MAI_MAO"; }
    static const char*  gateStr()       { return "gate"; }
    
    enum Outputs { Output_1, Output_2, Output_3 };
                        
protected:
                        ~ST_MAI_MAO() {}
    static Provider*    createInstance(Desc&);
    static void         updateDefaults(Desc&);
                    
    bool                allowParallelComputation() const { return false; }
                    
    bool                getInputData(const BinID&,int zintv);
    void                prepPriorToBoundsCalc();
    bool                computeData(const DataHolder&,const BinID& relpos, int z0,int nrsamples,int threadid) const;
    
    void                computeOutput_1( const Array1DImpl<float>& inp1, 
                                         const Array1DImpl<float>& inp2,
                                         const Array1DImpl<float>& inp3,
                                         Array1DImpl<float>& result ) const;

    void                computeOutput_2( const Array1DImpl<float>& inp1, 
                                         const Array1DImpl<float>& inp2,
                                         const Array1DImpl<float>& inp3,
                                         Array1DImpl<float>& result ) const;
                                                                              
    void                computeOutput_3( const Array1DImpl<float>& inp1, 
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

    
