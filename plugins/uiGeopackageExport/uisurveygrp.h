 
#ifndef uisurveygrp_h
#define uisurveygrp_h

#include "uidlggroup.h"

class uiCheckBox;

class uiSurveyGrp : public uiDlgGroup
{ mODTextTranslationClass(uiSurveyGrp);
public:
    uiSurveyGrp(uiParent*);
    
    bool doExport();
    
protected:
    uiCheckBox* expSurveyfld_;
    
};




#endif
