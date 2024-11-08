#pragma once
/*Copyright (C) 2014 Wayne Mogg. All rights reserved.

This file may be used either under the terms of:

1. The GNU General Public License version 3 or higher, as published by
the Free Software Foundation, or

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/
/*+
________________________________________________________________________

 Author:        Wayne Mogg
 Date:          January 2014
 ________________________________________________________________________

-*/

#include "uimlvfilterattribmod.h"
#include "uiattrdesced.h"

class uiAttrSel;
class uiGenInput;
class uiLabeledSpinBox;




/*! \brief Structure Preserving Smoothing Attribute description editor */

class uiMLVFilterAttrib : public uiAttrDescEd
{ mODTextTranslationClass(uiMLVFilterAttrib);
public:

			uiMLVFilterAttrib(uiParent*,bool);
			~uiMLVFilterAttrib();

    void		getEvalParams(TypeSet<EvalParam>&) const;

protected:

    uiAttrSel*		inpfld_;
    uiLabeledSpinBox*	sizefld_;
    uiGenInput*		outfld_;
    uiGenInput*     sdevsfld_;

    bool		setParameters(const Attrib::Desc&);
    bool		setInput(const Attrib::Desc&);
    bool		getParameters(Attrib::Desc&);
    bool		getInput(Attrib::Desc&);

    void        outChgCB(CallBacker*);
    void        sdevsChgCB(CallBacker*);

    			mDeclReqAttribUIFns
};
