#pragma once
/*
 *   uiEFDModesAttrib Plugin -  Empirical Fourier Decomposition Modes UI
 *   Copyright (C) 2021  Wayne Mogg
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "uiefdattribmod.h"
#include "uiattribtestpanel.h"
#include "uiattrdesced.h"

namespace Attrib { class Desc; }
class uiAttrSel;
class uiGenInput;
class uiLabeledSpinBox;
class uiPushButton;

mExpClass(uiEFDAttrib) uiEFDModesAttrib : public uiAttrDescEd, public TestPanelAdaptor
{ mODTextTranslationClass(uiEFDModesAttrib);
public:

    uiEFDModesAttrib(uiParent*,bool);
    ~uiEFDModesAttrib();

    void		getEvalParams(TypeSet<EvalParam>&) const;

protected:

    uiAttrSel*				inpfld_;
    uiGenInput*				nrmodesfld_;
    uiLabeledSpinBox*			outmodefld_;
    uiAttribTestPanel<uiEFDModesAttrib>* testpanel_ = nullptr;
    uiPushButton*			modepanelbut_;

    void		nrmodesChgCB(CallBacker*);


    bool		setParameters(const Attrib::Desc&);
    bool		setInput(const Attrib::Desc&);
    bool		setOutput(const Attrib::Desc&);

    bool		getParameters(Attrib::Desc&);
    bool		getInput(Attrib::Desc&);
    bool		getOutput(Attrib::Desc&);

    void		showPosDlgCB(CallBacker*);
    void		fillTestParams(Attrib::Desc*) const override;

    friend class uiAttribTestPanel<uiEFDModesAttrib>;

    			mDeclReqAttribUIFns
};
