#ifndef uimistieanalysismainwin_h
#define uimistieanalysismainwin_h

#include "uidialog.h"
#include "menuhandler.h"
#include "uitoolbutton.h"
#include "bufstring.h"

#include "uimistiemod.h"
#include "mistiedata.h"
#include "mistiecordata.h"

class uiTable;
class uiCheckBox;
class uiCorrViewer;

mClass(uiMistie) uiMistieAnalysisMainWin : public uiDialog
{ mODTextTranslationClass(uiMistieAnalysisMainWin);
public:
    enum Columns {
        lineACol,
        lineBCol,
        trcACol,
        trcBCol,
        xCol,
        yCol,
        zCol,
        phaseCol,
        ampCol,
        qualCol
    };
    uiMistieAnalysisMainWin(uiParent*);
    ~uiMistieAnalysisMainWin();

protected:

    uiToolBar*      tb_;
    uiTable*        table_;
    uiCorrViewer*   corrviewer_;
    BufferString    filename_;
    MistieData      misties_;
    MistieCorrectionData corrs_;
    float	    minquality_;

    MenuItem        newitem_;
    MenuItem        openitem_;
    MenuItem        saveitem_;
    MenuItem        saveasitem_;
    MenuItem        mergeitem_;
    MenuItem	    filteritem_;
    MenuItem        calcitem_;
    MenuItem        xplotitem_;
    MenuItem        helpitem_;
    
    void            createToolBar();
    void            fillTable();
    
    void            helpCB(CallBacker*);
    void            newCB(CallBacker*);
    void            openCB(CallBacker*);
    void            saveCB(CallBacker*);
    void            saveasCB(CallBacker*);
    void            mergeCB(CallBacker*);
    void            filterCB(CallBacker*);
    void            calcCB(CallBacker*);
    void            xplotCB(CallBacker*);
    void            closeCB(CallBacker*);
    
    bool            checkMistieSave();
    
private:

    uiString    getCaptionStr() const;
};

#endif
