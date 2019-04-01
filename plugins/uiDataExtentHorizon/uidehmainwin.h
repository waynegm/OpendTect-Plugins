#ifndef uidehmainwin_h
#define uidehmainwin_h

#include "uidialog.h"

class uiCheckBox;
class uiSurfaceWrite;
class uiSeis2DLineSelGrp;

mClass(uiDataExtentHorizon) uidehMainWin : public uiDialog
{ mODTextTranslationClass(uidehMainWin);
public:
    uidehMainWin(uiParent*);
    ~uidehMainWin();

protected:
    uiSeis2DLineSelGrp* lineselfld_;
    uiCheckBox*         include3Dfld_;
    uiSurfaceWrite*     outfld_;
    
    bool                acceptOK(CallBacker*);

private:
    uiString    getCaptionStr() const;
};

#endif
