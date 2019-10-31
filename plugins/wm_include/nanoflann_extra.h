#pragma once
 /*
  *   Nanoflann helper classes
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
#include "nanoflann.h"

#include "coord.h"
#include "typeset.h"
//
// TypeSet to nanoflann kd-tree adaptor class
//

class CoordTypeSetAdaptor
{
public:
    CoordTypeSetAdaptor( const TypeSet<Coord>& obj ) : obj_(obj) {}

    inline size_t kdtree_get_point_count() const { return obj_.size(); }

    inline Pos::Ordinate_Type kdtree_get_pt(const size_t idx, const size_t dim) const
    {
	return dim==0 ? obj_[idx].x : obj_[idx].y;
    }

    template <class BBOX>
    bool kdtree_get_bbox(BBOX& ) const { return false; }


    const TypeSet<Coord>& obj_;
};

typedef nanoflann::KDTreeSingleIndexAdaptor<
    nanoflann::L2_Simple_Adaptor<Pos::Ordinate_Type, CoordTypeSetAdaptor >,
    CoordTypeSetAdaptor,
    2>		CoordKDTree;
