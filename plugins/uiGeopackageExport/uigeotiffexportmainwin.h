#ifndef uigeotiffexportmainwin_h
#define uigeotiffexportmainwin_h

#include "uidialog.h"

class uiFileInput;
class uiSurfaceRead;
class uiCheckBox;

mClass(uiGeopackageExport) uiGeotiffExportMainWin : public uiDialog
{ mODTextTranslationClass(uiGeotiffExportMainWin);
public:
    uiGeotiffExportMainWin(uiParent*);
    ~uiGeotiffExportMainWin();

protected:

    uiFileInput*        filefld_;
    uiSurfaceRead*      hor3Dfld_;
    uiCheckBox*         expZvalue_;
    
    bool        acceptOK(CallBacker*);

private:

    uiString    getCaptionStr() const;
};

#endif
