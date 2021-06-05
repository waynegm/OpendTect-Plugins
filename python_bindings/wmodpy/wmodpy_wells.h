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
namespace Well{
    class Data;
    class LoadReqs;
    class Track;
}

class wmWells {
public:
    enum SampleMode { Upscale, Sample };
    enum ZMode { MD, TVD, TVDSS, TWT };
    wmWells( const wmSurvey& thesurvey );

    py::list    getWellNames() const;
    py::dict    getWellInfo() const;
    py::object  getWellInfoDF() const;
    py::object  getWellInfoGDF() const;
    py::list    getWellLogNames(const std::string& wellnm) const;
    py::dict    getWellLogInfo(const std::string& wellnm) const;
    py::object  getWellLogInfoDF(const std::string& wellnm) const;
    py::dict    getMarkers(const std::string& wellnm) const;
    py::object  getMarkersDF(const std::string& wellnm) const;
    py::dict    getTrack(const std::string& wellnm) const;
    py::object  getTrackDF(const std::string& wellnm) const;
    py::dict    getWellLogs(const std::string& wellnm, py::list lognms,
			    float zstep, SampleMode samplemode=Upscale) const;
    py::object  getWellLogsDF(const std::string& wellnm, py::list lognms,
			    float zstep, SampleMode samplemode=Upscale) const;


protected:
    const wmSurvey& survey_;

    const Well::Data* getWD(const std::string& wellnm, Well::LoadReqs lreqs) const;
};
