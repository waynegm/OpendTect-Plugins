 
#ifndef uisurveygrp_h
#define uisurveygrp_h

#include "uidlggroup.h"

class uiGenInput;

class uiSurveyGrp : public uiDlgGroup
{ mODTextTranslationClass(uiSurveyGrp);
public:
    uiSurveyGrp(uiParent*);
    
    bool doExport();
    
protected:
    uiGenInput* expSurveyfld_;
    
    bool    expsurvey_;
};




#endif
