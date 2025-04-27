#pragma once

#include "uidlggroup.h"
#include "uistring.h"
#include "ctxtioobj.h"

class uiCheckBox;
class CtxtIOObj;
class uiIOObjSelGrp;

class uiRandomLinesGrp : public uiDlgGroup
{ mODTextTranslationClass(uiRandomLinesGrp);
public:
    uiRandomLinesGrp(uiParent*);
    ~uiRandomLinesGrp() {}

    bool doLineExport();
    void getLineIds( TypeSet<MultiID>& lineids );
    void reset();

protected:
    CtxtIOObj      ctio_;
    uiIOObjSelGrp* linesfld_;
};
