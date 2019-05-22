#ifndef uimistiecorrmainwin_h
#define uimistiecorrmainwin_h

#include "uimainwin.h"
#include "menuhandler.h"
#include "uitoolbutton.h"
#include "bufstring.h"

#include "uimistiemod.h"

class uiTable;
class uiCheckBox;

mClass(uiMistie) uiMistieCorrMainWin : public uiMainWin
{ mODTextTranslationClass(uiMistieCorrMainWin);
public:
    enum Columns {
        lineCol,
        shiftCol,
        phaseCol,
        ampCol
    };
    uiMistieCorrMainWin(uiParent*);
    ~uiMistieCorrMainWin();

protected:

    uiToolBar*      tb_;
    uiTable*        table_;
    uiCheckBox*     locknames_;
    BufferString    filename_;

    MenuItem        newitem_;
    MenuItem        openitem_;
    MenuItem        saveitem_;
    MenuItem        saveasitem_;
    MenuItem        mergeitem_;
    MenuItem        helpitem_;
    
    void            createToolBar();
    
    void            helpCB(CallBacker*);
    void            newCB(CallBacker*);
    void            openCB(CallBacker*);
    void            saveCB(CallBacker*);
    void            saveasCB(CallBacker*);
    void            mergeCB(CallBacker*);
    void            newrowCB(CallBacker*);
    void            locknamesCB(CallBacker*);
private:

    uiString    getCaptionStr() const;
};

#endif
