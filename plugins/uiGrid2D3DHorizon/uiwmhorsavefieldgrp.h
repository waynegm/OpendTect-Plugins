#ifndef uiwmhorsavefieldgrp_h
#define uiwmhorsavefieldgrp_h

#include "uihorsavefieldgrp.h"

class uiwmHorSaveFieldGrp : public uiHorSaveFieldGrp
{mODTextTranslationClass(uiwmHorSaveFieldGrp);
public:
    uiwmHorSaveFieldGrp(uiParent*);
    
    bool createNewHorizon();
    bool setHorRange(const Interval<int>&, const Interval<int>&);
    
};

#endif

