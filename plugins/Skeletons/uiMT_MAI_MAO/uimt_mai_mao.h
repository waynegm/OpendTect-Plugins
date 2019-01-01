#ifndef uimt_mai_mao_h
#define uimt_mai_mao_h

#include "uimt_mai_maomod.h"
#include "uiattrdesced.h"

class uiAttrSel;
class uiGenInput;
class uiStepOutSel;

mExpClass(uiMT_MAI_MAO) uiMT_MAI_MAO : public uiAttrDescEd
{ mODTextTranslationClass(uiMT_MAI_MAO);
public:

            uiMT_MAI_MAO(uiParent*,bool);

    void    getEvalParams(TypeSet<EvalParam>&) const;

protected:

    uiAttrSel*      inpfld_1_;
    uiAttrSel*      inpfld_2_;
    uiAttrSel*      inpfld_3_;
    
    uiGenInput*     outpfld_;
    uiGenInput*     gatefld_;
    uiStepOutSel*   stepoutfld_;

    bool        setParameters(const Attrib::Desc&);
    bool        setInput(const Attrib::Desc&);
    bool        setOutput(const Attrib::Desc&);
    
    bool        getParameters(Attrib::Desc&);
    bool        getInput(Attrib::Desc&);
    bool        getOutput(Attrib::Desc&);
    
    mDeclReqAttribUIFns
};


#endif
