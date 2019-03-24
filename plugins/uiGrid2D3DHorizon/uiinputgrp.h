#ifndef uiinputgrp_h
#define uiinputgrp_h

#include "uidlggroup.h"

class uiGenInput;
class uiCheckBox;
class uiLabeledComboBox;
class uiIOObjSel;
class uiSeis2DLineSelGrp;
class uiPosSubSel;
class MultiID;
class TrcKeyZSampling;

class uiInputGrp : public uiDlgGroup
{ mODTextTranslationClass(uiHorizonGrp);
public:
    uiInputGrp(uiParent*, bool has2Dhorizon=false, bool has3Dhorizon=false );
    ~uiInputGrp() {}
    
    void        getHorIds( MultiID& hor2Did, MultiID& hor3Did );
    void        getInputRange( Interval<int>& inlrg, Interval<int>& crlrg );
    int         num2DLinesChosen();
    void        getGeoMids( TypeSet<Pos::GeomID>& geomids );
    void        get3Dsel( TrcKeyZSampling& envelope );
    void        update();
    
protected:
    uiIOObjSel*         hor2Dfld_;
    uiSeis2DLineSelGrp* lines2Dfld_;
    uiIOObjSel*         hor3Dfld_;
    uiPosSubSel*        subsel3Dfld_;
    
    void                hor2DselCB(CallBacker*);
    void                hor3DselCB(CallBacker*);
};




#endif
