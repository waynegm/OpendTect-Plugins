#pragma once
/*
 *   Local Thin-plate Spline gridder class
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
#include "wmgridder2d.h"
class uiParent;



class wmLTPSGridder2D : public wmGridder2D
{
public:
    friend class LTPSInterpolator;
    wmLTPSGridder2D();
    ~wmLTPSGridder2D() {}

    bool	executeGridding(uiParent*);
    double	basis( double r ) const;

protected:
    void	calcResidual();

};

inline double wmLTPSGridder2D::basis( double rsq ) const
{
    double R2 = rsq+0.0001;
    return R2>1.0 ? 0.0 : R2*(log(R2)-1)+1;
}
