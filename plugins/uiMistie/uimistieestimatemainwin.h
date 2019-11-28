#ifndef uimistieestimatemainwin_h
#define uimistieestimatemainwin_h

#include "uidialog.h"
#include "bufstring.h"

#include "uimistiemod.h"
#include "mistiedata.h"

class uiFileInput;
class uiGenInput;
class uiCheckBox;
class uiLabeledSpinBox;
class uiSeisSel;
namespace WMLib {
    class uiSeis2DLineSelGrp;
};



mClass(uiMistie) uiMistieEstimateMainWin : public uiDialog
{ mODTextTranslationClass(uiMistieEstimateMainWin);
public:
    uiMistieEstimateMainWin(uiParent*);
    ~uiMistieEstimateMainWin();
    
    const MistieData& getMisties() const { return misties_; };

protected:
    uiSeisSel*                  seisselfld_;
    WMLib::uiSeis2DLineSelGrp*  lineselfld_;
    uiCheckBox*                 use3dfld_ = nullptr;
    uiSeisSel*                  data3dfld_ = nullptr;
    uiGenInput*                 trcstepfld_ = nullptr;
    uiGenInput*                 gatefld_;
    uiLabeledSpinBox*           lagfld_;
    
    MistieData                  misties_;
    
    bool            acceptOK(CallBacker*);
    void            use3DCB(CallBacker*);
    
private:

    uiString    getCaptionStr() const;
    void        gatefldchangeCB(CallBacker*);
    void        seisselCB(CallBacker*);
};

#endif
