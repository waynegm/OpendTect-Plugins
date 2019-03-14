#ifndef uigrid2d3dhorizonmainwin_h
#define uigrid2d3dhorizonmainwin_h

#include "uidialog.h"

class uiGenInput;
class uiIOObjSel;
class uiSeis2DLineSelGrp;
class uiPosSubSel;

mClass(uiGrid2D3DHorizon) uiGrid2D3DHorizonMainWin : public uiDialog
{ mODTextTranslationClass(uiGrid2D3DHorizonMainWin);
public:
    uiGrid2D3DHorizonMainWin(uiParent*);
    ~uiGrid2D3DHorizonMainWin();

protected:
    uiGenInput*         filltypefld_;
    uiIOObjSel*         polyfld_;
    uiGenInput*         stepfld_;
    uiIOObjSel*         hor2Dfld_;
    uiSeis2DLineSelGrp* lines2Dfld_;
    uiIOObjSel*         hor3Dfld_;
    uiPosSubSel*        subsel3Dfld_;

    void                scopeChgCB(CallBacker*);
    void                hor2DselCB(CallBacker*);
    void                hor3DselCB(CallBacker*);
    bool                acceptOK(CallBacker*);

private:

    uiString    getCaptionStr() const;
};

#endif
