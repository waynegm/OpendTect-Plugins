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
#include "wmodpy_wells.h"
#include "wmodpy_survey.h"

#include "filepath.h"
#include "iodir.h"
#include "ioobj.h"
#include "ioman.h"
#include "multiid.h"

#include "welldata.h"
#include "welllog.h"
#include "welllogset.h"
#include "wellman.h"
#include "wellmarker.h"
#include "wellreader.h"

namespace py = pybind11;

void init_wmodpy_wells(py::module_& m) {
    py::class_<wmWells>(m, "Wells", "Encapsulates the wells in an OpendTect survey")
	.def(py::init<const wmSurvey&>())
	.def("get_well_names", &wmWells::getWellNames, "Return list of all well names in the survey")
	.def("get_well_info", &wmWells::getWellInfo, "Return dict with basic information for all wells in the survey")
	.def("get_well_log_info", &wmWells::getWellLogInfo, "Return dict with basic information for all logs in the specified well")
	.def("get_markers", &wmWells::getMarkers, "Return dict with marker information for the specified well");

}


wmWells::wmWells( const wmSurvey& thesurvey )
    : survey_(thesurvey)
{ }

py::list wmWells::getWellNames() const
{
    py::list list;
    FilePath fp(survey_.surveyPath().c_str(), "WellInfo");
    const IODir dir(fp.fullPath());
    for (int idx=0; idx<dir.size(); idx++) {
	const IOObj* ioobj = dir.get(idx);
	if (ioobj->group()=="Well")
	    list.append(std::string(ioobj->name()));
    }
    return list;
}

py::dict wmWells::getWellInfo() const {
    auto Point = py::module::import("shapely.geometry").attr("Point");
    py::dict dict;
    py::list names, uwid, oper, state, county, welltype, geometry, rvel, gelev;
    survey_.activate();
    FilePath fp(survey_.surveyPath().c_str(), "WellInfo");
    const IODir dir(fp.fullPath());
    Well::Man& mgr = Well::MGR();
    for (int idx=0; idx<dir.size(); idx++) {
	const IOObj* ioobj = dir.get(idx);
	if (ioobj && ioobj->group()=="Well") {
	    const MultiID wid = ioobj->key();
	    const Well::Data* wd = mgr.get(wid, Well::LoadReqs(Well::Inf));
	    if (wd) {
		names.append(std::string(wd->info().name()));
		uwid.append(std::string(wd->info().uwid));
		state.append(std::string(wd->info().state));
		county.append(std::string(wd->info().county));
		welltype.append(std::string(wd->info().toString(wd->info().welltype_)));
		const Coord cd = wd->info().surfacecoord;
		py::object p = Point(cd.x, cd.y);
		geometry.append(p);
		rvel.append(wd->info().replvel);
		gelev.append(wd->info().groundelev);
	    }
	}
    }
    dict["Name"] = names;
    dict["UWID"] = uwid;
    dict["State"] = state;
    dict["County"] = county;
    dict["WellType"] = welltype;
    dict["Geometry"] = geometry;
    dict["ReplacementVelocity"] = rvel;
    dict["GroundElevation"] = gelev;

    return dict;
}

py::dict wmWells::getWellLogInfo(const std::string& wellnm) const {
    py::dict dict;
    py::list names, mnemonic, uom, dahrange, valrange;
    survey_.activate();
    Well::LoadReqs lreq = Well::LoadReqs(Well::LogInfos);
    auto* wd = getWD(wellnm, lreq);
    if (wd) {
	const Well::LogSet& ls = wd->logs();
	for (int il=0; il<ls.size(); il++) {
	    const Well::Log& log = ls.getLog(il);
	    names.append(std::string(log.name()));
	    mnemonic.append(std::string(log.mnemLabel()));
	    uom.append(std::string(log.unitMeasLabel()));
	    py::tuple dahrg(2);
	    dahrg[0] = log.dahRange().start;
	    dahrg[1] = log.dahRange().stop;
	    dahrange.append(dahrg);
	    py::tuple valrg(2);
	    valrg[0] = log.valueRange().start;
	    valrg[1] = log.valueRange().stop;
	    valrange.append(valrg);
	}
    }
    dict["Name"] = names;
    dict["Mnem"] = mnemonic;
    dict["Uom"] = uom;
    dict["DahRange"] = dahrange;
    dict["ValueRange"] = valrange;

    return dict;
}

py::dict wmWells::getMarkers(const std::string& wellnm) const {
    py::dict dict;
    py::list names, colors, zs;
    survey_.activate();
    Well::LoadReqs lreq = Well::LoadReqs(Well::Mrkrs);
    auto* wd = getWD(wellnm, lreq);
    if (wd) {
	const Well::MarkerSet& ms = wd->markers();
	for (int im=0; im<ms.size(); im++) {
	    names.append(std::string(ms[im]->name()));
	    colors.append(std::string(ms[im]->color().getStdStr()));
	    zs.append(ms[im]->dah());
	}
    }
    dict["Name"] = names;
    dict["Color"] = colors;
    dict["Dah"] = zs;

    return dict;
}

const Well::Data* wmWells::getWD(const std::string& wellnm, Well::LoadReqs lreqs) const {
    IOObj* ioobj = Well::findIOObj(wellnm.c_str(), nullptr);
    if (ioobj) {
	const Well::Data* wd = Well::MGR().get(ioobj->key(), lreqs);
	return wd;
    }
    return nullptr;
}
