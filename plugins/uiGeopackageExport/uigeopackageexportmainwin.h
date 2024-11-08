#pragma once
#include "uidialog.h"

class uiFileInput;
class uiCheckBox;
class uiTabStack;
class uiSurveyGrp;
class ui2DLinesGrp;
class uiRandomLinesGrp;
class uiWellsGrp;
class uiPolyLinesGrp;
class uiHorizonGrp;

mClass(uiGeopackageExport) uiGeopackageExportMainWin : public uiDialog
{ mODTextTranslationClass(uiGeopackageExportMainWin);
public:
    uiGeopackageExportMainWin(uiParent*);
    ~uiGeopackageExportMainWin();

    bool            checkCRSdefined();
protected:

    uiFileInput*        filefld_;
    uiTabStack*         tabstack_;
    uiCheckBox*         appendfld_;
    uiSurveyGrp*        surveygrp_;
    ui2DLinesGrp*       linesgrp_;
    uiRandomLinesGrp*   randomgrp_;
    uiWellsGrp*         wellsgrp_;
    uiPolyLinesGrp*     polygrp_;
    uiHorizonGrp*       horgrp_;

    void            tabSel(CallBacker*);
    bool        acceptOK(CallBacker*);

private:

    uiString    getCaptionStr() const;
};
