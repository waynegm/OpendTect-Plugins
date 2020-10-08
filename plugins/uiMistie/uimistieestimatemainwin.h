#ifndef uimistieestimatemainwin_h
#define uimistieestimatemainwin_h

#include "uidialog.h"
#include "uigroup.h"
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
    class uiHorInputGrp;
};

class uiMistieEstimateBySeismic : public uiGroup
{ mODTextTranslationClass(uiMistieEstimateBySeismic);
public:
    uiMistieEstimateBySeismic(uiParent*);
    ~uiMistieEstimateBySeismic();

    bool	estimateMisties(MistieData&);

protected:
    uiSeisSel*                  seisselfld_ = nullptr;
    WMLib::uiSeis2DLineSelGrp*  lineselfld_ = nullptr;
    uiCheckBox*                 use3dfld_ = nullptr;
    uiSeisSel*                  data3dfld_ = nullptr;
    uiGenInput*                 trcstepfld_ = nullptr;
    uiGenInput*                 gatefld_;
    uiLabeledSpinBox*           lagfld_;
    uiCheckBox*			onlyzfld_;

    void			use3DCB(CallBacker*);
    void			seisselCB(CallBacker*);
    void		        gatefldchangeCB(CallBacker*);

};

class uiMistieEstimateByHorizon : public uiGroup
{ mODTextTranslationClass(uiMistieEstimateByHorizon);
public:
    uiMistieEstimateByHorizon(uiParent*);
    ~uiMistieEstimateByHorizon();

    bool	estimateMisties(MistieData&);

protected:
    WMLib::uiHorInputGrp*	horinpgrp_;
    uiGenInput*                 trcstepfld_ = nullptr;

    void			use3DCB(CallBacker*);

};


mClass(uiMistie) uiMistieEstimateMainWin : public uiDialog
{ mODTextTranslationClass(uiMistieEstimateMainWin);
public:
    uiMistieEstimateMainWin(uiParent*);
    ~uiMistieEstimateMainWin();
    
    const MistieData& getMisties() const { return misties_; };

protected:
    uiGenInput*			fromseisfld_;
    uiMistieEstimateBySeismic*	seisgrp_;
    uiMistieEstimateByHorizon*	horgrp_;
    MistieData                  misties_;

    void			modeCB(CallBacker*);
    
    bool            acceptOK(CallBacker*);
    
private:

    uiString    getCaptionStr() const;
    void        initCB(CallBacker*);
};

#endif
