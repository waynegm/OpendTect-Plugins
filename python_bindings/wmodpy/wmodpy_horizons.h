#pragma once
/*Copyright (C) 2021 Wayne Mogg All rights reserved.
 *
 * This file may be used either under the terms of:
 *
 * 1. The GNU General Public License version 3 or higher, as published by
 * the Free Software Foundation, or
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 * ________________________________________________________________________
 *
 * Author:        Wayne Mogg
 * Date:          March 2021
 * ________________________________________________________________________
 *
 */
#include "pybind11/pybind11.h"
namespace py = pybind11;


class wmSurvey;

class wmHorizons3D {
public:
    wmHorizons3D( const wmSurvey& thesurvey );

    py::list    getNames() const;
    py::tuple   getZ(const std::string& name) const;

protected:
    const wmSurvey& survey_;

};

class wmHorizons2D {
public:
    wmHorizons2D( const wmSurvey& thesurvey );

    py::list getNames() const;

protected:
    const wmSurvey& survey_;

};
