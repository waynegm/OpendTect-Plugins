#pragma once

#include "uidlggroup.h"

class uiGenInput;

class uiSurveyGrp : public uiDlgGroup
{ mODTextTranslationClass(uiSurveyGrp);
public:
    uiSurveyGrp(uiParent*);

    bool doExport();

protected:
    uiGenInput* expSurveyfld_;

};
