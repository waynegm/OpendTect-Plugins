#ifndef uimt_mai_sao_h
#define uimt_mai_sao_h

#include "uimt_mai_saomod.h"
#include "uiattrdesced.h"

class uiAttrSel;
class uiGenInput;
class uiStepOutSel;

mExpClass(uiMT_MAI_SAO) uiMT_MAI_SAO : public uiAttrDescEd
{ mODTextTranslationClass(uiMT_MAI_SAO);
public:

            uiMT_MAI_SAO(uiParent*,bool);

    void    getEvalParams(TypeSet<EvalParam>&) const;

protected:

    uiAttrSel*      inpfld_1_;
    uiAttrSel*      inpfld_2_;
    uiAttrSel*      inpfld_3_;
    
    uiGenInput*     gatefld_;
    uiStepOutSel*   stepoutfld_;

    bool        setParameters(const Attrib::Desc&);
    bool        setInput(const Attrib::Desc&);
    
    bool        getParameters(Attrib::Desc&);
    bool        getInput(Attrib::Desc&);
    
    mDeclReqAttribUIFns
};


#endif
