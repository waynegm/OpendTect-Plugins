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
    uiSurveyGrp*        surveygrp_ = nullptr;
    ui2DLinesGrp*       linesgrp_ = nullptr;
    uiRandomLinesGrp*   randomgrp_ = nullptr;
    uiWellsGrp*         wellsgrp_ = nullptr;
    uiPolyLinesGrp*     polygrp_ = nullptr;
    uiHorizonGrp*       horgrp_ = nullptr;

    void            tabSel(CallBacker*);
    bool        acceptOK(CallBacker*);

private:

    uiString    getCaptionStr() const;
};
