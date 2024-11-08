#prafma once
/*
*   Convex Hull for a set of points
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

#include "polygon.h"
#include "coord.h"

namespace wmLib {
typedef Pos::Ordinate_Type OrdinateType;

class ConvexHull : public ODPolygon<OrdinateType>
{
public:
    ConvexHull(const TypeSet<Coord>& points)
	: ODPolygon<OrdinateType>((TypeSet<Geom::Point2D<OrdinateType>>&) points) { convexHull(); }

protected:

};

};
