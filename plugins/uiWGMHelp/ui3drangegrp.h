#ifndef ui3drangegrp_h
#define ui3drangegrp_h

#include "uiwgmhelpmod.h"
#include "uidlggroup.h"
#include "trckeysampling.h"

class uiGenInput;
class Callbacker;

namespace WMLib
{
    
mExpClass(uiWGMHelp) ui3DRangeGrp : public uiDlgGroup
{ mODTextTranslationClass(ui3DRangeGrp);
public:
    ui3DRangeGrp(uiParent*, const uiString&, bool snapToStep=false);
    ~ui3DRangeGrp();
    
    bool                fillPar( IOPar& par ) const;
    TrcKeySampling      getTrcKeySampling() const;
    void                setTrcKeySampling( const TrcKeySampling& );
    
protected:
    uiGenInput*         inlfld_;
    uiGenInput*         crlfld_;
    
    bool                stepSnap_;
    
    void                rangeChg(Callbacker*);
    
};
}; //namespace

#endif
