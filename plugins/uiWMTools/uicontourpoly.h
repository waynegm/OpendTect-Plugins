#ifndef uicontourpoly_h
#define uicontourpoly_h
/*
 *   Constant Z polyline generator class
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
#include "commontypes.h"

namespace Pick {
    class Set;
};
class Callbacker;
class uiColorInput;
class uiGenInput;


mClass(uiWMTools) uiContourPoly : public uiDialog
{ mODTextTranslationClass(uiContourPoly);
public:
    uiContourPoly(uiParent*);
    ~uiContourPoly();

    RefMan<Pick::Set>	getPolygonPickSet() const;
    float	getZ() const { return z_; }

protected:

    uiGenInput*			namefld_;
    uiGenInput*			zfld_;
    uiColorInput*		colorfld_;

    BufferString		polyname_;
    Pos::Ordinate_Type		z_;

    bool                acceptOK(CallBacker*);

private:
    uiString    getCaptionStr() const;
};

#endif
