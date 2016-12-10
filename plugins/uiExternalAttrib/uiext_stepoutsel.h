/*Copyright (C) 2016 Wayne Mogg. All rights reserved.

This file may be used either under the terms of:

1. The GNU General Public License version 3 or higher, as published by
the Free Software Foundation, or

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef uiext_stepoutsel_h
#define uiext_stepoutsel_h

/*+
________________________________________________________________________

 Author:        Wayne Mogg
 Date:          December 2016
 ________________________________________________________________________

-*/

#include "uistepoutsel.h"
#include "uispinbox.h"

/*! \brief Derived uiStepOutSel class to access protected fields */

class uiExt_StepOutSel : public uiStepOutSel
{ mODTextTranslationClass(uiExt_StepOutSel);
public:

	uiExt_StepOutSel( uiParent* parobj ,bool single=false ) :uiStepOutSel(parobj, single) {}
	~uiExt_StepOutSel() {} 
    

	void displayFld2( bool show=true) { fld2_->display(show); } 
};


#endif


