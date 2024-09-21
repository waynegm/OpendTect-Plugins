#ifndef uidehmainwin_h
#define uidehmainwin_h
/*
 *   Data Extent Horizon generator class
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
#include "uidialog.h"

class uiGenInput;
class uiCheckBox;
class uiSurfaceWrite;
class uiSeis2DLineSelGrp;
namespace WMLib {
    class ui3DRangeGrp;
    class uiSeis2DLineSelGrp;
};
class Callbacker;

mClass(uiWMTools) uidehMainWin : public uiDialog
{ mODTextTranslationClass(uidehMainWin);
public:
    uidehMainWin(uiParent*);
    ~uidehMainWin();

protected:
    uiGenInput*                 zfld_;
    WMLib::uiSeis2DLineSelGrp*  lineselfld_;
    uiCheckBox*                 include3Dfld_;
    WMLib::ui3DRangeGrp*        rangefld_;
    uiSurfaceWrite*             outfld_;

    bool                acceptOK(CallBacker*);
    void		updateRangeCB(CallBacker*);

private:
    uiString    getCaptionStr() const;
};

#endif
