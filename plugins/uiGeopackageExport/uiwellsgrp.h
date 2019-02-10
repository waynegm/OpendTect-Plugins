#ifndef uiwellsgrp_h
#define uiwellsgrp_h

#include "uidlggroup.h"
#include "ctxtioobj.h"

class uiCheckBox;
class CtxtIOObj;
class uiIOObjSelGrp;

class uiWellsGrp : public uiDlgGroup
{ mODTextTranslationClass(uiWellsGrp);
public:
    uiWellsGrp(uiParent*);
    ~uiWellsGrp() {}
    
    bool doWellExport();
    void getWellIds( TypeSet<MultiID>& wellids );
    void reset();
    
protected:
    CtxtIOObj      ctio_;
    uiIOObjSelGrp* wellsfld_;
};




#endif
