#ifndef uipolylinesgrp_h
#define uipolylinesgrp_h

#include "uidlggroup.h"
#include "ctxtioobj.h"

class uiCheckBox;
class CtxtIOObj;
class uiIOObjSelGrp;

class uiPolyLinesGrp : public uiDlgGroup
{ mODTextTranslationClass(uiPolyLinesGrp);
public:
    uiPolyLinesGrp(uiParent*);
    ~uiPolyLinesGrp() {}
    
    bool doLineExport();
    void getLineIds( TypeSet<MultiID>& lineids );
    void reset();
    
protected:
    CtxtIOObj      ctio_;
    uiIOObjSelGrp* linesfld_;
};




#endif
