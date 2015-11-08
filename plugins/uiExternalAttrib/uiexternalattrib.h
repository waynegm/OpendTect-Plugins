/*Copyright (C) 2014 Wayne Mogg. All rights reserved.

This file may be used either under the terms of:

1. The GNU General Public License version 3 or higher, as published by
the Free Software Foundation, or

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef uiexternalattrib_h
#define uiexternalattrib_h

/*+
________________________________________________________________________

 Author:        Wayne Mogg
 Date:          December 2014
 ________________________________________________________________________

-*/

#include "uiexternalattribmod.h"
#include "uiattrdesced.h"
#include "json.h"
#include "wmparamsetget.h"

class uiAttrSel;
class uiTable;
class uiGenInput;
class uiFileInput;
class uiStepOutSel;
class uiToolButton;
class ExtProc;

/*! \brief External Attribute description editor */

class uiExternalAttrib : public uiAttrDescEd
{
public:

	uiExternalAttrib(uiParent*,bool);
	~uiExternalAttrib();
    
protected:

	ExtProc*				extproc_;
    uiTable*				table_;
	uiFileInput*			exfilefld_;
	uiFileInput*			interpfilefld_;
	ObjectSet<uiAttrSel>	inpflds_;
	uiGenInput*				zmarginfld_;
	uiStepOutSel*			stepoutfld_;
	uiGenInput*				outputfld_;
	uiGenInput*				selectfld_;
	ObjectSet<uiGenInput>	parflds_;
	uiToolButton*			help_;
	
	void		exfileChanged(CallBacker*);
	void		setExFileName( const char* );
	BufferString	getExFileName();
	bool		setParameters(const Attrib::Desc&);
    bool		setInput(const Attrib::Desc&);
    bool		getParameters(Attrib::Desc&);
    bool		getInput(Attrib::Desc&);

	bool		setOutput(const Attrib::Desc&);
	bool		getOutput(Attrib::Desc&);
	
	void		doHelp( CallBacker* cb );
	void		doZmarginSymmetry( CallBacker* cb );
	
    			mDeclReqAttribUIFns
};


#endif


