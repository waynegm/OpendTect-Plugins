#ifndef mt_mai_mao_h
#define mt_mai_mao_h

#include "attribprovider.h"
#include "arrayndimpl.h"

namespace Attrib {
    
mClass(MT_MAI_MAO) MT_MAI_MAO : public Provider
{ mODTextTranslationClass(MT_MAI_MAO);
public:
    static void     initClass();
                    MT_MAI_MAO(Desc&);

    static const char*  attribName()    { return "MT_MAI_MAO"; }
    static const char*  stepoutStr()    { return "stepout"; }
    static const char*  gateStr()       { return "gate"; }
    
    enum Outputs { Output_1, Output_2, Output_3 };
                        
protected:
                        ~MT_MAI_MAO() {}
    static Provider*    createInstance(Desc&);
    static void         updateDefaults(Desc&);
                    
    bool                allowParallelComputation() const { return false; }
                    
    bool                getInputData(const BinID&,int zintv);
    void                prepPriorToBoundsCalc();
    bool                computeData(const DataHolder&,const BinID& relpos, int z0,int nrsamples,int threadid) const;
    
    void                computeOutput_1( const Array2DImpl<float>& inp1, 
                                         const Array2DImpl<float>& inp2,
                                         const Array2DImpl<float>& inp3,
                                         Array1DImpl<float>& result ) const;

    void                computeOutput_2( const Array2DImpl<float>& inp1, 
                                         const Array2DImpl<float>& inp2,
                                         const Array2DImpl<float>& inp3,
                                         Array1DImpl<float>& result ) const;
                                                                              
    void                computeOutput_3( const Array2DImpl<float>& inp1, 
                                         const Array2DImpl<float>& inp2,
                                         const Array2DImpl<float>& inp3,
                                         Array1DImpl<float>& result ) const;
                                                                                                                   
    const BinID*        desStepout(int,int) const;
    const Interval<int>* desZSampMargin(int,int) const;
    bool                getTrcPos();
    
    Interval<int>   sampgate_;
    Interval<float> gate_;
    BinID           stepout_;
    
    TypeSet<BinID>  trcpos_;
    int             centertrcidx_;
    
    ObjectSet<const DataHolder>   input_1_;
    ObjectSet<const DataHolder>   input_2_;
    ObjectSet<const DataHolder>   input_3_; // Add more/less as required
    int             input_1_idx_;
    int             input_2_idx_;
    int             input_3_idx_;   // Add more/less as required
};

};

#endif

    
