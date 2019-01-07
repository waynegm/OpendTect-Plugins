#ifndef uiavopolar_h
#define uiavopolar_h

#include "uiavopolarmod.h"
#include "uiattrdesced.h"

class uiAttrSel;
class uiGenInput;
class uiStepOutSel;

mExpClass(uiAVOPolar) uiAVOPolar : public uiAttrDescEd
{ mODTextTranslationClass(uiAVOPolar);
public:

            uiAVOPolar(uiParent*,bool);

    void    getEvalParams(TypeSet<EvalParam>&) const;

protected:

    uiAttrSel*      intercept_;
    uiAttrSel*      gradient_;
    
    uiGenInput*     gateBGfld_;
    uiStepOutSel*   stepoutfld_;
    uiGenInput*     parallelfld_;
    uiGenInput*     methodfld_;

    bool        setParameters(const Attrib::Desc&);
    bool        setInput(const Attrib::Desc&);
    
    bool        getParameters(Attrib::Desc&);
    bool        getInput(Attrib::Desc&);
    
    mDeclReqAttribUIFns
};


#endif
