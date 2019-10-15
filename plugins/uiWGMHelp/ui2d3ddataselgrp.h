#ifndef ui2d3ddataselgrp_h
#define ui2d3ddataselgrp_h

#include "uiwgmhelpmod.h"
#include "uidlggroup.h"

class uiCheckBox;
class uiPosSubSel;
class TrcKeyZSampling;

namespace WMLib {
    class uiSeis2DLineSelGrp;

mExpClass(uiWGMHelp) ui2D3DDataSelGrp : public uiDlgGroup
{ mODTextTranslationClass(ui2D3DDataSelGrp);
public:
    ui2D3DDataSelGrp(uiParent*);
    ~ui2D3DDataSelGrp();
    
    int         num2DLinesChosen() const;
    void        get2DGeomIDs( TypeSet<Pos::GeomID>& geomids ) const;
    void        get3Dsel( TrcKeyZSampling& envelope ) const;
    
    
protected:
    uiSeis2DLineSelGrp*	line2dselfld_;
    uiCheckBox*		include3dfld_;
    uiPosSubSel*	subsel3dfld_;
    
    void		initGrp(CallBacker*);
    void		include3DCB(CallBacker*);
    
    
};
};
#endif


