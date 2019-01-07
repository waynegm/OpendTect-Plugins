#ifndef uist_mai_sao_h
#define uist_mai_sao_h

#include "uist_mai_saomod.h"
#include "uiattrdesced.h"

class uiAttrSel;
class uiGenInput;

mExpClass(uiST_MAI_SAO) uiST_MAI_SAO : public uiAttrDescEd
{ mODTextTranslationClass(uiST_MAI_SAO);
public:

            uiST_MAI_SAO(uiParent*,bool);

    void    getEvalParams(TypeSet<EvalParam>&) const;

protected:

    uiAttrSel*      inpfld_1_;
    uiAttrSel*      inpfld_2_;
    uiAttrSel*      inpfld_3_;
    
    uiGenInput*     gatefld_;

    bool        setParameters(const Attrib::Desc&);
    bool        setInput(const Attrib::Desc&);
    
    bool        getParameters(Attrib::Desc&);
    bool        getInput(Attrib::Desc&);
    
    mDeclReqAttribUIFns
};


#endif
