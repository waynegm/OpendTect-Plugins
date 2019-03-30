#ifndef uigrid2d3dhorizonmainwin_h
#define uigrid2d3dhorizonmainwin_h

#include "uidialog.h"

class uiGenInput;
class uiTabStack;
class uiInputGrp;
class uiGridGrp;
//class uiwmHorSaveFieldGrp;
class uiSurfaceWrite;

mClass(uiGrid2D3DHorizon) uiGrid2D3DHorizonMainWin : public uiDialog
{ mODTextTranslationClass(uiGrid2D3DHorizonMainWin);
public:
    uiGrid2D3DHorizonMainWin(uiParent*);
    ~uiGrid2D3DHorizonMainWin();

protected:
    uiTabStack*         tabstack_;
    uiGenInput*         scopefld_;
    uiInputGrp*         inputgrp_;
    uiGridGrp*          gridgrp_;
    uiSurfaceWrite*     outfld_;
    
    void                tabSelCB(CallBacker*);
    bool                acceptOK(CallBacker*);

private:

    uiString    getCaptionStr() const;
};

#endif
