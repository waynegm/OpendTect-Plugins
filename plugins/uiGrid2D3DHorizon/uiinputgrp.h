#ifndef uiinputgrp_h
#define uiinputgrp_h

#include "uidlggroup.h"

class uiGenInput;
class uiCheckBox;
class uiLabeledComboBox;
class uiIOObjSel;
class uiPosSubSel;
class MultiID;
class TrcKeyZSampling;
namespace WMLib {
    class uiSeis2DLineSelGrp;
};

class uiInputGrp : public uiDlgGroup
{ mODTextTranslationClass(uiHorizonGrp);
public:
    uiInputGrp(uiParent*, bool has2Dhorizon=false, bool has3Dhorizon=false );
    ~uiInputGrp() {}
    
    void        getHorIds( MultiID& hor2Did, MultiID& hor3Did ) const;
    void        getInputRange( Interval<int>& inlrg, Interval<int>& crlrg );
    int         num2DLinesChosen();
    void        getGeoMids( TypeSet<Pos::GeomID>& geomids ) const;
    void        get3Dsel( TrcKeyZSampling& envelope ) const;
    void        update();
    bool        fillPar(IOPar&) const;
    
protected:
    uiIOObjSel*                 hor2Dfld_;
    WMLib::uiSeis2DLineSelGrp*  lines2Dfld_;
    uiCheckBox*                 exp3D_;
    uiIOObjSel*                 hor3Dfld_;
    uiPosSubSel*                subsel3Dfld_;
    
    void                hor2DselCB(CallBacker*);
    void                hor3DselCB(CallBacker*);
    void                exp3DselCB(CallBacker*);
};




#endif
