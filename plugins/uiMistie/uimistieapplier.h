#ifndef uimistieapplier_h
#define uimistieapplier_h

#include "uimistiemod.h"
#include "uiattrdesced.h"

class uiAttrSel;
class uiFileInput;
class uiButtonGroup;
class uiCheckBox;

mExpClass(uiMistie) uiMistieApplier : public uiAttrDescEd
{ mODTextTranslationClass(uiMistieApplier);
public:

            uiMistieApplier(uiParent*,bool);

protected:
    uiAttrSel*      inpfld_;
    uiFileInput*    mistiefilefld_;
    uiButtonGroup*  actiongrp_;
    uiCheckBox*     shiftbut_;
    uiCheckBox*     phasebut_;
    uiCheckBox*     ampbut_;

    bool        setParameters(const Attrib::Desc&);
    bool        setInput(const Attrib::Desc&);
    
    bool        getParameters(Attrib::Desc&);
    bool        getInput(Attrib::Desc&);
    
    mDeclReqAttribUIFns
};


#endif
