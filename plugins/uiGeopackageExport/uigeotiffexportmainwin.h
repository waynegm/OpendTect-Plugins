#ifndef uigeotiffexportmainwin_h
#define uigeotiffexportmainwin_h

#include "uidialog.h"

class uiFileInput;
class uiSurfaceRead;
class uiCheckBox;
class uiGenInput;
class uiIOObjSelGrp;

mClass(uiGeopackageExport) uiGeotiffExportMainWin : public uiDialog
{ mODTextTranslationClass(uiGeotiffExportMainWin);
public:
    uiGeotiffExportMainWin(uiParent*);
    ~uiGeotiffExportMainWin();

protected:

    uiFileInput*        filefld_;
    uiSurfaceRead*      hor3Dfld_;
    uiCheckBox*         expZvalue_;
    uiGenInput*		expTypefld_;
    uiIOObjSelGrp*	inpfld_;
    uiGenInput*		expZfld_;

    bool        acceptOK(CallBacker*);
    void	initCB(CallBacker*);
    void	updateuiCB(CallBacker*);
    void	inputselCB(CallBacker*);

private:

    uiString    getCaptionStr() const;
};

#endif
