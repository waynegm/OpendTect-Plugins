#pragma once
/*
 *   Convex hull generator class
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
#include "polygon.h"

namespace WMLib {
    class ui2D3DDataSelGrp;
    class uiHorInputGrp;
};
namespace Pick {
    class Set;
};
class Callbacker;
class uiColorInput;
class uiGenInput;
class uiCheckBox;


mClass(uiWMTools) uiConvexHull : public uiDialog
{ mODTextTranslationClass(uiConvexHull);
public:
    uiConvexHull(uiParent*);
    ~uiConvexHull();

    bool	addToDisplay() const;
    RefMan<Pick::Set>	getPolygonPickSet() const;
    float	getZ() const { return z_; }

protected:

    WMLib::ui2D3DDataSelGrp*	navinputgrp_;
    WMLib::uiHorInputGrp*	horinputfld_ = nullptr;
    uiGenInput*			namefld_;
    uiGenInput*			zfld_;
    uiColorInput*		colorfld_;
    uiCheckBox*                 usenavfld_;
    uiCheckBox*                 usehorfld_;

    BufferString			polyname_;
    ODPolygon<Pos::Ordinate_Type>	poly_;
    Pos::Ordinate_Type			z_;

    bool                acceptOK(CallBacker*);
    void		usenavCB(CallBacker*);
    void		usehorCB(CallBacker*);
    void 		dataselCB(CallBacker*);
    void 		initGrp(CallBacker*);

    void		fillPolyFromNav();
    void		fillPolyFromHorizon();

private:
    uiString    getCaptionStr() const;
};
