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
    void		usePar( const IOPar& par );
    TrcKeySampling      getTrcKeySampling() const;
    void                setTrcKeySampling( const TrcKeySampling& );
    void                displayFields(bool yn=true);
    void                displayFields(bool ranges, bool steps);
    void                setSensitive(bool yn=true);
    void                setSensitive(bool ranges, bool steps);
    void		setSnap(bool yn);
    
protected:
    uiGenInput*         inlfld_;
    uiGenInput*         crlfld_;
    
    bool                stepSnap_;
    
    void                rangeChg(Callbacker*);
    
};
}; //namespace

#endif
