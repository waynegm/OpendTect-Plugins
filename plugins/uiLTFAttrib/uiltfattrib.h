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

#ifndef uiltfattrib_h
#define uiltfattrib_h

/*+
 * ________________________________________________________________________
 * 
 * Author:        Wayne Mogg
 * Date:          Apr 2014
 * ________________________________________________________________________
 * 
 * -*/

#include "uiltfattribmod.h"
#include "uiattrdesced.h"

namespace Attrib { class Desc; }
class uiAttrSel;
class uiGenInput;
class uiLabeledSpinBox;

mExpClass(uiLTFAttrib) uiLTFAttrib : public uiAttrDescEd
{
public:

			uiLTFAttrib(uiParent*,bool);

protected:

    uiAttrSel*			inpfld_;
	uiLabeledSpinBox*	freqfld_;
	uiLabeledSpinBox*	smoothfld_;
	uiLabeledSpinBox*	marginfld_;
	uiLabeledSpinBox*	niterfld_;
	
	void		inputSel(CallBacker*);
	
	
    bool		setParameters(const Attrib::Desc&);
    bool		setInput(const Attrib::Desc&);

    bool		getParameters(Attrib::Desc&);
    bool		getInput(Attrib::Desc&);

    			mDeclReqAttribUIFns
};

#endif
