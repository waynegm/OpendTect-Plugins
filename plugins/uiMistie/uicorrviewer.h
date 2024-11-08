#pragma once

#include "uidialog.h"
#include "menuhandler.h"
#include "uitoolbutton.h"
#include "bufstring.h"

#include "uimistiemod.h"

class uiTable;
class uiCheckBox;
class MistieCorrectionData;

mClass(uiMistie) uiCorrViewer : public uiDialog
{ mODTextTranslationClass(uiCorrViewer);
public:
    enum Columns {
        lineCol,
        shiftCol,
        phaseCol,
        ampCol
    };
    uiCorrViewer(uiParent* p, const MistieCorrectionData& corrs);
    ~uiCorrViewer();

    void            newCorrData();
    bool            checkSave();

protected:

    uiToolBar*      tb_;
    uiTable*        table_;
    BufferString    filename_;
    const MistieCorrectionData& corrs_;

    MenuItem        saveitem_;
    MenuItem        saveasitem_;

    void            createToolBar();
    void            fillTable();

    void            saveCB(CallBacker*);
    void            saveasCB(CallBacker*);
    void            closeCB(CallBacker*);
private:

    uiString    getCaptionStr() const;
};
