#pragma once

#include "uiavopolarattribmod.h"
#include "uiattrdesced.h"

namespace Attrib { class Desc; }
class uiAttrSel;
class uiGenInput;
class uiStepOutSel;

mExpClass(uiAVOPolarAttrib) uiAVOPolarAttrib : public uiAttrDescEd
{ mODTextTranslationClass(uiAVOPolarAttrib);
public:

            uiAVOPolarAttrib(uiParent*,bool);
	    ~uiAVOPolarAttrib();

    void    getEvalParams(TypeSet<EvalParam>&) const;

protected:

    uiAttrSel*      interceptfld_;
    uiAttrSel*      gradientfld_;
    uiGenInput*     outputfld_;

    uiGenInput*     gateBGfld_;
    uiStepOutSel*   stepoutfld_;
    uiGenInput*     gatefld_;

    void        outSel(CallBacker*);

    bool        setParameters(const Attrib::Desc&);
    bool        setInput(const Attrib::Desc&);
    bool        setOutput(const Attrib::Desc&);

    bool        getParameters(Attrib::Desc&);
    bool        getInput(Attrib::Desc&);
    bool        getOutput(Attrib::Desc&);

    mDeclReqAttribUIFns
};
