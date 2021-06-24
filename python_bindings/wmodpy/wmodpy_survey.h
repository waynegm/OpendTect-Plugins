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

class SurveyInfo;

class wmSurvey {
public:
    wmSurvey(const std::string& basedir, const std::string& surveynm);
    ~wmSurvey();

    std::string name() const;
    std::string type() const;
    py::dict    getSurveyInfo() const;
    py::object  getSurveyInfoDF() const;
    py::dict	getSurveyFeature(bool towgs=false) const;
    py::list    getSurveyPoints(bool towgs=false) const;
    bool        isOK() const;
    bool        has2D() const;
    bool        has3D() const;
    std::string epsgCode() const;
    std::string surveyPath() const;
    void        activate() const;
    SurveyInfo* si() const { return si_; }

    static void initModule();

protected:
    SurveyInfo*	si_=nullptr;
    std::string	basedir_, survey_;
    static std::string curbasedir_, cursurvey_, modulepath_;

};

