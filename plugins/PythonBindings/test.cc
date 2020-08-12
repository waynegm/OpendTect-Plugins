#include "pybind11/pybind11.h"

#include "survinfo.h"

PYBIND11_MODULE(test, m) {
    m.doc() = "pybind11 test plugin";

    py::class_<SurveyInfo>(m, "SurveyInfo")
	.def
