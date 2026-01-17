#pragma once
/*
 *   uiLocalAttrib Plugin
 *   Copyright (C) 2019  Wayne Mogg
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
/*
* This is the UI for the Local Time-Frequency Decomposition attribute
*/
#include "uilocalattribmod.h"
#include "uiattribtestpanel.h"
#include "uiattrdesced.h"

namespace Attrib { class Desc; }
class uiAttrSel;
class uiGenInput;
class uiLabeledSpinBox;
class uiPushButton;

mExpClass(uiLocalAttrib) uiLTFAttrib : public uiAttrDescEd, public TestPanelAdaptor
{ mODTextTranslationClass(uiLTFAttrib);
public:

    uiLTFAttrib(uiParent*,bool);
    ~uiLTFAttrib();

    void		getEvalParams(TypeSet<EvalParam>&) const;
    int			getOutputIdx(float) const;
    float		getOutputValue(int) const;

protected:

    uiAttrSel*			inpfld_;
    uiGenInput*			gatefld_;
    uiLabeledSpinBox*		freqfld_;
    uiLabeledSpinBox*		stepfld_;
//    uiLabeledSpinBox*	smoothfld_;
//	uiLabeledSpinBox*	marginfld_;
    uiLabeledSpinBox*		niterfld_;
    uiAttribTestPanel<uiLTFAttrib>*	testpanel_ = nullptr;
    uiPushButton*			tfpanelbut_;

    void		inputSel(CallBacker*);
    void		stepChg(CallBacker*);


    bool		setParameters(const Attrib::Desc&);
    bool		setInput(const Attrib::Desc&);
    bool		setOutput(const Attrib::Desc&);

    bool		getParameters(Attrib::Desc&);
    bool		getInput(Attrib::Desc&);
    bool		getOutput(Attrib::Desc&);

    void		checkOutValSnapped() const;

    void		showPosDlgCB(CallBacker*);
    void		fillTestParams(Attrib::Desc*) const override;

    friend class uiAttribTestPanel<uiLTFAttrib>;

    			mDeclReqAttribUIFns
};
