#ifndef uimistiecorrmainwin_h
#define uimistiecorrmainwin_h

#include "uimainwin.h"
#include "menuhandler.h"
#include "uitoolbutton.h"
#include "bufstring.h"

#include "uimistiemod.h"

class uiTable;

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
    BufferString    filename_;

    MenuItem        newitem_;
    MenuItem        openitem_;
    MenuItem        saveitem_;
    MenuItem        saveasitem_;
    MenuItem        helpitem_;
    
    void            createToolBar();
    
    void            helpCB(CallBacker*);
    void            newCB(CallBacker*);
    void            openCB(CallBacker*);
    void            saveCB(CallBacker*);
    void            saveasCB(CallBacker*);
    void            newrowCB(CallBacker*);
private:

    uiString    getCaptionStr() const;
};

#endif
