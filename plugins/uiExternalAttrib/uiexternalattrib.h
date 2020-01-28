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
#include "uitable.h"
#include "json.h"
#include "wmparamsetget.h"

class uiAttrSel;
class uiGenInput;
class uiFileInput;
class uiToolButton;
class ExtProc;
class uiExt_StepOutSel;

/*! \brief External Attribute description editor */

class uiExternalAttribInp : public uiTable
{mODTextTranslationClass(uiExternalAttribInp);
public:
    uiExternalAttribInp(uiParent*,const uiTable::Setup&);
    ~uiExternalAttribInp();


protected:
    ObjectSet<uiAttrSel>		inpflds_;
    uiGenInput*				outputfld_;
};

class uiExternalAttrib : public uiAttrDescEd
{ mODTextTranslationClass(uiExternalAttrib);
public:

    uiExternalAttrib(uiParent*,bool);
    ~uiExternalAttrib();
    
protected:

    ExtProc*			extproc_;
    uiFileInput*		exfilefld_;
    uiFileInput*		interpfilefld_;
    uiExternalAttribInp*	uiinp_;
    ObjectSet<uiAttrSel>	inpflds_;
    uiGenInput*			zmarginfld_;
    uiExt_StepOutSel*		stepoutfld_;
    uiGenInput*			outputfld_;
    uiGenInput*			selectfld_;
    ObjectSet<uiGenInput>	parflds_;
    uiToolButton*		help_;
    uiToolButton*		refinterp_;
	
    void		makeUI();
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
    void		doZmarginCheck( CallBacker* cb );
    void		doStepOutCheck( CallBacker* cb );
    void		updateinterpCB( CallBacker* );
	
    mDeclReqAttribUIFns
};


#endif


