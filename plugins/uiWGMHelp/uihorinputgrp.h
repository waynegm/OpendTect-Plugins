#ifndef uihorinputgrp_h
#define uihorinputgrp_h

#include "uiwgmhelpmod.h"
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


mExpClass(uiWGMHelp) uiHorInputGrp : public uiDlgGroup
{ mODTextTranslationClass(uiHorInputGrp);
public:
    uiHorInputGrp(uiParent*, bool has2Dhorizon=false, bool has3Dhorizon=false, bool show3Dsubsel=true );
    ~uiHorInputGrp();

    void        getHorIds( MultiID& hor2Did, MultiID& hor3Did ) const;
    void        getInputRange( Interval<int>& inlrg, Interval<int>& crlrg );
    int         num2DLinesChosen();
    void        getGeoMids( TypeSet<Pos::GeomID>& geomids ) const;
    void        get3Dsel( TrcKeyZSampling& envelope ) const;
    bool        fillPar(IOPar&) const;
    void        usePar(const IOPar&);

    static const char*	sKey2DLineID() 		{ return "2DLineID"; }
    static const char*	sKey2DHorizonID()	{ return "2DHorizonID"; }
    static const char* 	sKey2DLineIDNr()	{ return "2DLineIDNr"; }
    static const char*	sKey3DHorizonID()	{ return "3DHorizonID"; }

    static bool	has2Dhorizons();
    static bool has3Dhorizons();

    uiCheckBox*                 exp3D_ = nullptr;

protected:
    uiIOObjSel*                 hor2Dfld_;
    uiSeis2DLineSelGrp* 	lines2Dfld_;
    uiIOObjSel*                 hor3Dfld_;
    uiPosSubSel*                subsel3Dfld_;

    void		initGrp(CallBacker*);
    void                hor2DselCB(CallBacker*);
    void                hor3DselCB(CallBacker*);
    void                exp3DselCB(CallBacker*);
};

};



#endif
