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
 * Date:          January 2021
 * ________________________________________________________________________
 *
 */
#include "pybind11/pybind11.h"
namespace py = pybind11;

#include "wellman.h"

class wmSurvey;
namespace Well{ class Data; class LoadReqs; }

class wmWells {
public:
    wmWells( const wmSurvey& thesurvey );

    py::list getWellNames() const;
    py::dict getWellInfo() const;
    py::dict getWellLogInfo(const std::string& wellnm) const;
    py::dict getMarkers(const std::string& wellnm) const;


protected:
    const wmSurvey& survey_;

    const Well::Data* getWD(const std::string& wellnm, Well::LoadReqs lreqs) const;
};
