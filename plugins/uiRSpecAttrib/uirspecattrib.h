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

#ifndef uirspecattrib_h
#define uirspecattrib_h

/*+
 * ________________________________________________________________________
 * 
 * Author:        Wayne Mogg
 * Date:          August 2014
 * ________________________________________________________________________
 * 
 * -*/

#include "uirspecattribmod.h"
#include "uiattrdesced.h"

namespace Attrib { class Desc; }
class uiAttrSel;
class uiGenInput;
class uiLabeledSpinBox;

mExpClass(uiRSpecAttrib) uiRSpecAttrib : public uiAttrDescEd
{
public:

				uiRSpecAttrib(uiParent*,bool);

	void		getEvalParams(TypeSet<EvalParam>&) const;
	int			getOutputIdx(float) const;
	float		getOutputValue(int) const;
protected:

    uiAttrSel*			inpfld_;
	uiGenInput*			windowfld_;
	uiLabeledSpinBox*	freqfld_;
	uiLabeledSpinBox*	stepfld_;
	
	void		inputSel(CallBacker*);
	void		stepChg(CallBacker*);
	
	
    bool		setParameters(const Attrib::Desc&);
    bool		setInput(const Attrib::Desc&);
	bool		setOutput(const Attrib::Desc&);
	
    bool		getParameters(Attrib::Desc&);
    bool		getInput(Attrib::Desc&);
	bool		getOutput(Attrib::Desc&);

	void		checkOutValSnapped() const;
	
    			mDeclReqAttribUIFns
};

#endif
