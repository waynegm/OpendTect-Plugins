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

void init_wmodpy_survey(py::module_&);
void init_wmodpy_wells(py::module_&);

PYBIND11_MODULE(wmodpy, m) {
    m.doc() = "pybind11 wmodpy plugin for OpendTect";

    init_wmodpy_survey(m);
    init_wmodpy_wells(m);
}

