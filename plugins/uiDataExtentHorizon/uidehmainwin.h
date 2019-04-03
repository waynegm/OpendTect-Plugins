#ifndef uidehmainwin_h
#define uidehmainwin_h

#include "uidialog.h"

class uiGenInput;
class uiCheckBox;
class uiSurfaceWrite;
class uiSeis2DLineSelGrp;
namespace WMLib { 
    class ui3DRangeGrp;
    class uiSeis2DLineSelGrp;
};
class Callbacker;

mClass(uiDataExtentHorizon) uidehMainWin : public uiDialog
{ mODTextTranslationClass(uidehMainWin);
public:
    uidehMainWin(uiParent*);
    ~uidehMainWin();

protected:
    uiGenInput*                 zfld_;
    WMLib::uiSeis2DLineSelGrp*  lineselfld_;
    uiCheckBox*                 include3Dfld_;
    WMLib::ui3DRangeGrp*        rangefld_;
    uiSurfaceWrite*             outfld_;
    
    bool                acceptOK(CallBacker*);
    void                lineselCB(Callbacker*);
    void                include3DCB(Callbacker*);

private:
    uiString    getCaptionStr() const;
};

#endif
