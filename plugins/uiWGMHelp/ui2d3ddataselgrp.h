#pragma once

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
    uiSeis2DLineSelGrp*	line2dselfld_ = nullptr;
    uiCheckBox*		include3dfld_ = nullptr;
    uiPosSubSel*	subsel3dfld_ = nullptr;

    void		initGrp(CallBacker*);
    void		include3DCB(CallBacker*);


};
};
