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
#include <map>
#include <string>

class uiAttrSel;
class uiGenInput;
class uiFileInput;
class uiToolButton;
class ExtProc;
class uiExt_StepOutSel;
class ChangeTracker;

/*! \brief External Attribute description editor */

class uiExternalAttribInp : public uiTable
{mODTextTranslationClass(uiExternalAttribInp);
public:
    uiExternalAttribInp(uiParent*);
    ~uiExternalAttribInp();

    void	clearUI();
    void	makeUI(const Attrib::DescSet*, bool is2d);
    void	makeInputUI(const Attrib::DescSet*, bool is2d);
    void	makeOutputUI();
    void	makeZSamplingUI();
    void	makeStepoutUI(bool is2d);
    void	makeLegacyUI();
    void	makeNewUI();

    void	setExtProc(ExtProc* extproc);
    const ExtProc*	getExtProc();
protected:
    friend class		uiExternalAttrib;
    ExtProc*			extproc_ = nullptr;
    ObjectSet<uiAttrSel>	inpflds_;
    uiGenInput*			outputfld_;
    uiGenInput*			zmarginfld_;
    uiExt_StepOutSel*		stepoutfld_;
    uiGenInput*			selectfld_;
    ObjectSet<uiGenInput>	parflds_;
    std::map<std::string,uiGenInput*>	fields_;

    void	doZmarginCheck(CallBacker*);
    void	doStepOutCheck(CallBacker*);

    bool	setParameters(const Attrib::Desc&);
    bool	getParameters(Attrib::Desc&, ChangeTracker&);
    bool	setOutput(const Attrib::Desc&);
    void	addField(const char*, uiGenInput*);

    bool	setNewParams(const BufferString&);
    BufferString	getNewParams(Attrib::Desc&, ChangeTracker&);
};

class uiExternalAttrib : public uiAttrDescEd
{ mODTextTranslationClass(uiExternalAttrib);
public:

    uiExternalAttrib(uiParent*,bool);
    ~uiExternalAttrib();

protected:

    uiFileInput*		exfilefld_;
    uiFileInput*		interpfilefld_;
    uiExternalAttribInp*	uiinp_;
    uiToolButton*		help_;
    uiToolButton*		refinterp_;

    void		makeUI();
    void		exfileChanged(CallBacker*);
    void		exfileRefresh(CallBacker*);
    void		setExFileName( const char* );
    BufferString	getExFileName();
    bool		setParameters(const Attrib::Desc&);
    bool		setInput(const Attrib::Desc&);
    bool		setOutput(const Attrib::Desc&);
    bool		getParameters(Attrib::Desc&);
    bool		getInput(Attrib::Desc&);
    bool		getOutput(Attrib::Desc&);

    void		doHelp( CallBacker* cb );
    void		updateinterpCB( CallBacker* );
    void		initGrp(CallBacker*);

    mDeclReqAttribUIFns
};


#endif


