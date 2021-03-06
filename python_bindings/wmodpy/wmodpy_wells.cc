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
#include<pybind11/numpy.h>

#include "wmodpy_wells.h"
#include "wmodpy_survey.h"

#include "filepath.h"
#include "iodir.h"
#include "ioobj.h"
#include "ioman.h"
#include "latlong.h"
#include "multiid.h"

#include "welldata.h"
#include "welllog.h"
#include "welllogset.h"
#include "wellman.h"
#include "wellmarker.h"
#include "wellreader.h"
#include "welltrack.h"

namespace py = pybind11;
using namespace pybind11::literals;

void init_wmodpy_wells(py::module_& m) {
    py::class_<wmWells> wells(m, "Wells", "Encapsulates the wells in an OpendTect survey");

    py::enum_<wmWells::SampleMode>(wells, "SampleMode")
        .value("Upscale", wmWells::SampleMode::Upscale)
        .value("Sample", wmWells::SampleMode::Sample)
        .export_values();

	wells.def(py::init<const wmSurvey&>())
	    .def("names", &wmWells::getWellNames,
	         "Return list of all well names in the survey")
	    .def("info", &wmWells::getWellInfo,
	         "Return dict with basic information for all wells in the survey")
	    .def("info_df", &wmWells::getWellInfoDF,
	         "Return Pandas dataframe with basic information for all wells in the survey - requires Pandas")
	    .def("features", &wmWells::getWellFeatures,
             "Return basic information for all wells as a GeoJSON Feature Collection",
             "towgs"_a=false)
	    .def("log_names", &wmWells::getWellLogNames,
	         "Return list of all log names in the specified well")
	    .def("log_info", &wmWells::getWellLogInfo,
	         "Return dict with basic information for all logs in the specified well")
	    .def("log_info_df", &wmWells::getWellLogInfoDF,
	         "Return Pandas dataframe with basic information for all logs in the specified well - requires Pandas")
        .def("log_data", &wmWells::getWellLogs,
             "Return dict with well log data")
        .def("log_data_df", &wmWells::getWellLogsDF,
             "Return Pandas dataframe with well log data - requires Pandas")
	    .def("markers", &wmWells::getMarkers,
	         "Return dict with marker information for the specified well")
	    .def("markers_df", &wmWells::getMarkersDF,
	         "Return Pandas dataframe with marker information for the specified well - requires Pandas")
	    .def("track", &wmWells::getTrack,
	         "Return dict with MD, TVDSS, X and Y for the specified well")
	    .def("track_df", &wmWells::getTrackDF,
	         "Return Pandas dataframe with track information for the specified well - requires Pandas");

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
    py::dict dict;
    py::list names, uwid, oper, state, county, welltype, x, y, rvel, gelev;
    survey_.activate();
    FilePath fp(survey_.surveyPath().c_str(), "WellInfo");
    const IODir dir(fp.fullPath());
    for (int idx=0; idx<dir.size(); idx++) {
	    const IOObj* ioobj = dir.get(idx);
	    if (ioobj && ioobj->group()=="Well") {
	        MultiID wid = ioobj->key();
	        RefMan<Well::Data> wd = new Well::Data;
            if (wd) {
                wd->ref();
                Well::Reader rdr( wid, *wd );
                if (rdr.getInfo()) {
		            names.append(std::string(wd->info().name()));
		            uwid.append(std::string(wd->info().uwid));
		            state.append(std::string(wd->info().state));
		            county.append(std::string(wd->info().county));
		            welltype.append(std::string(wd->info().toString(wd->info().welltype_)));
		            const Coord cd = wd->info().surfacecoord;
		            x.append(cd.x);
		            y.append(cd.y);
		            rvel.append(wd->info().replvel);
		            gelev.append(wd->info().groundelev);
                }
                wd->unRef();
	        }
	    }
    }
    dict["Name"] = names;
    dict["UWID"] = uwid;
    dict["State"] = state;
    dict["County"] = county;
    dict["WellType"] = welltype;
    dict["X"] = x;
    dict["Y"] = y;
    dict["ReplacementVelocity"] = rvel;
    dict["GroundElevation"] = gelev;

    return dict;
}

py::object wmWells::getWellInfoDF() const {
    auto PDF = py::module::import("pandas").attr("DataFrame");
    return PDF( getWellInfo() );
}

py::object wmWells::getWellFeatures(bool towgs) const {
    auto jsondumps = py::module::import("json").attr("dumps");
    survey_.activate();
    py::dict result;
    if (survey_.epsgCode().empty())
	    return jsondumps(result);

    py::dict info = getWellInfo();
    py::list name = info["Name"];
    py::list uwid = info["UWID"];
    py::list state = info["State"];
    py::list county = info["County"];
    py::list type = info["WellType"] ;
    py::list x = info["X"];
    py::list y = info["Y"];
    py::list features;

    const SurveyInfo* si = survey_.si();
    const Coords::CoordSystem* coordsys = si->getCoordSystem();
    for (int idx=0; idx<name.size(); idx++) {
        py::dict feature, props, geom;
        feature["type"] = "Feature";
        props["Name"] = name[idx];
        props["UWID"] = uwid[idx];
        props["State"] = state[idx];
        props["County"] = county[idx];
        props["WellType"] = type[idx];
        feature["properties"] = props;
        py::list point;
        Coord::OrdType xc = py::cast<Coord::OrdType>(x[idx]);
        Coord::OrdType yc = py::cast<Coord::OrdType>(y[idx]);
        if (towgs) {
            const LatLong ll( LatLong::transform(Coord(xc, yc), true, coordsys) );
		    point.append(ll.lng_);
		    point.append(ll.lat_);
        } else {
            point.append(xc);
		    point.append(yc);
        }
        geom["type"] = "Point";
        geom["coordinates"] = point;
        feature["geometry"] = geom;
    	features.append(feature);
    }
    result["type"] = "FeatureCollection";
    result["features"] = features;
    return jsondumps(result);
}

py::list wmWells::getWellLogNames(const std::string& wellnm) const {
    py::list names;
    survey_.activate();
    Well::LoadReqs lreq = Well::LoadReqs(Well::LogInfos);
    auto* wd = getWD(wellnm, lreq);
    if (wd) {
	const Well::LogSet& ls = wd->logs();
	for (int il=0; il<ls.size(); il++) {
	    const Well::Log& log = ls.getLog(il);
	    names.append(std::string(log.name()));
	}
    }

    return names;
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

py::object wmWells::getWellLogInfoDF(const std::string& wellnm) const {
    auto PDF = py::module::import("pandas").attr("DataFrame");
    return PDF( getWellLogInfo(wellnm) );
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
    dict["MD"] = zs;

    return dict;
}

py::object wmWells::getMarkersDF(const std::string& wellnm) const {
    auto PDF = py::module::import("pandas").attr("DataFrame");
    return PDF( getMarkers(wellnm) );
}

py::dict wmWells::getTrack(const std::string& wellnm) const {
    py::dict dict;
    survey_.activate();
    Well::LoadReqs lreq = Well::LoadReqs(Well::Trck);
    auto* wd = getWD(wellnm, lreq);
    if (wd) {
	    const Well::Track& lt = wd->track();
        const int sz = lt.size();
        py::array_t<float> md = py::array_t<float>(sz);
        py::array_t<float> tvdss = py::array_t<float>(sz);
        py::array_t<float> x = py::array_t<float>(sz);
        py::array_t<float> y = py::array_t<float>(sz);
        auto md_data = md.mutable_data();
        auto tvdss_data = tvdss.mutable_data();
        auto x_data = x.mutable_data();
        auto y_data = y.mutable_data();

        for (int i=0; i<sz; i++) {
            md_data[i] = lt.dah(i);
            const Coord3 pt = lt.pos(i);
            tvdss_data[i] = pt.z;
            x_data[i] = pt.x;
            y_data[i] = pt.y;
        }
        dict["md"] = md;
        dict["tvdss"] = tvdss;
        dict["x"] = x;
        dict["y"] = y;
    }
    return dict;
}

py::object wmWells::getTrackDF(const std::string& wellnm) const {
    auto PDF = py::module::import("pandas").attr("DataFrame");
    return PDF( getTrack(wellnm) );
}

py::dict wmWells::getWellLogs(const std::string& wellnm, py::list lognames, float zstep, SampleMode sampmode) const
{
    py::dict dict;
    survey_.activate();
    Well::LoadReqs lreq = Well::LoadReqs(Well::LogInfos);
    auto* wd = getWD(wellnm, lreq);
    StepInterval<float> dahrg;
    dahrg.setUdf();
    dahrg.step = zstep;
    for (auto logname : lognames) {
        auto* log = wd->logs().getLog(py::cast<std::string>(logname).c_str());
        if (log)
            dahrg.include(log->dahRange());
    }

    bool first = true;
    for (auto logname : lognames) {
        auto* log = wd->getLog(py::cast<std::string>(logname).c_str());
        if (log) {
            Well::Log* outlog;
            if (sampmode==Upscale)
                outlog = log->upScaleLog(dahrg);
            else
                outlog = log->sampleLog(dahrg);

            py::array_t<float> outarr = py::array_t<float>(outlog->size());
            auto outarr_data = outarr.mutable_data();

            for (int idx=0; idx<outlog->size();idx++)
                outarr_data[idx] = mIsUdf(outlog->value(idx)) ? nanf("") : outlog->value(idx);

            if (first) {
                py::array_t<float> depths = py::array_t<float>(outlog->size());
                auto depths_data = depths.mutable_data();
                for (int idx=0; idx<outlog->size(); idx++)
                    depths_data[idx] = outlog->dah(idx);

                dict["MD"] = depths;
                first = false;
            }
	        dict[logname] = outarr;
            delete outlog;
        }
    }
    return dict;
}

py::object wmWells::getWellLogsDF(const std::string& wellnm, py::list lognames, float zstep, SampleMode sampmode) const {
    auto PDF = py::module::import("pandas").attr("DataFrame");
    return PDF( getWellLogs(wellnm, lognames, zstep, sampmode) );
}

const Well::Data* wmWells::getWD(const std::string& wellnm, Well::LoadReqs lreqs) const {
    IOObj* ioobj = Well::findIOObj(wellnm.c_str(), nullptr);
    if (ioobj) {
	const Well::Data* wd = Well::MGR().get(ioobj->key(), lreqs);
	return wd;
    }
    return nullptr;
}
