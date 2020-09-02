#pragma once
/*
 *   Apply Mistie Corrections to Horizon Interpretation
 *   Copyright (C) 2020  Wayne Mogg
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
#include "uidialog.h"

class uiSurfaceWrite;
class uiFileInput;
namespace WMLib { 
    class uiHorInputGrp;
};
class Callbacker;

mClass(uiMistie) uiMistieCorrHorDlg : public uiDialog
{ mODTextTranslationClass(uiMistieCorrHorDlg);
public:
    uiMistieCorrHorDlg(uiParent*);
    ~uiMistieCorrHorDlg();

protected:
    WMLib::uiHorInputGrp*	horinpgrp_;
    uiFileInput*   		mistiefilefld_;
    uiSurfaceWrite*		outfld_;

    bool                acceptOK(CallBacker*);

private:
    uiString    getCaptionStr() const;
};

