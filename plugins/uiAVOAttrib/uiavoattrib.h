/*Copyright (C) 2014 Wayne Mogg. All rights reserved.
 * 
 * This file may be used either under the terms of:
 * 
 * 1. The GNU General Public License version 3 or higher, as published by
 * the Free Software Foundation, or
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef uiavoattrib_h
#define uiavoattrib_h

/*+
 * ________________________________________________________________________
 * 
 * Author:        Wayne Mogg
 * Date:          Feb 2014
 * ________________________________________________________________________
 * 
 * -*/

#include "uiavoattribmod.h"
#include "uiattrdesced.h"

namespace Attrib { class Desc; }
class uiAttrSel;
class uiGenInput;
class uiSteeringSel;
class uiStepOutSel;


mExpClass(uiAVOAttrib) uiAVOAttrib : public uiAttrDescEd
{
public:

			uiAVOAttrib(uiParent*,bool);

protected:

    uiAttrSel*		inp_interceptfld_;
	uiAttrSel*		inp_gradientfld_;
	uiGenInput*		outputfld_;
	uiGenInput*		slopefld_;
	uiGenInput*		intercept_devfld_;
	uiGenInput*		gradient_devfld_;
	uiGenInput*		correlationfld_;
	uiGenInput*		class2fld_;
	

    void		actionSel(CallBacker*);

    bool		setParameters(const Attrib::Desc&);
    bool		setInput(const Attrib::Desc&);

    bool		getParameters(Attrib::Desc&);
    bool		getInput(Attrib::Desc&);

    			mDeclReqAttribUIFns
};

#endif
