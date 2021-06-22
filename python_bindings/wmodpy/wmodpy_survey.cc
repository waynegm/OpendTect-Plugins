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
#include "wmodpy_survey.h"

#include "ascstream.h"
#include "bufstring.h"
#include "dirlist.h"
#include "file.h"
#include "filepath.h"
#include "genc.h"
#include "ioman.h"
#include "iopar.h"
#include "latlong.h"
#include "moddepmgr.h"
#include "oddirs.h"
#include "odruncontext.h"
#include "oscommand.h"
#include "plugins.h"
#include "ptrman.h"
#include "safefileio.h"
#include "survinfo.h"


namespace py = pybind11;
using namespace pybind11::literals;

std::string wmSurvey::curbasedir_;
std::string wmSurvey::cursurvey_;
std::string wmSurvey::modulepath_;

py::list GetSurveys(const char* basedir) {
	wmSurvey::initModule();
	py::list list;
	if (!IOMan::isValidDataRoot(basedir))
	    return list;
	DirList dl(basedir, File::DirsInDir);
	for (int idx=0; idx<dl.size(); idx++) {
	    const BufferString& dirnm = dl.get(idx);
	    const FilePath fp(basedir, dirnm);
	    const BufferString fpstr = fp.fullPath();
	    if (File::exists(fpstr) && IOMan::isValidSurveyDir(fpstr))
		list.append(std::string(dirnm));
	}
	return list;
}

py::dict GetSurveyInfo(const char* basedir, py::list surveys) {
    wmSurvey::initModule();
    py::dict dict;
    py::list names, types, crs;
    py::list use;
    if (surveys.empty())
        use = GetSurveys(basedir);
    else {
        for (auto survnm : surveys)
            use.append(survnm);
    }

    for (auto survnm : use) {
        wmSurvey survey(basedir, py::cast<std::string>(survnm));
        names.append(std::string(survey.name()));
        types.append(std::string(survey.type()));
        crs.append(survey.epsgCode());
    }
    dict["Name"] = names;
    dict["Type"] = types;
    dict["crs"] = crs;
    return dict;
}

py::object GetSurveyInfoGDF(const char* basedir, py::list surveys) {
	wmSurvey::initModule();
    auto GDF = py::module::import("geopandas").attr("GeoDataFrame");
    auto Polygon = py::module::import("shapely.geometry").attr("Polygon");
    py::dict res;
    py::list names, types, polys;
    py::list use;
    if (surveys.empty())
        use = GetSurveys(basedir);
    else {
        for (auto survnm : surveys)
            use.append(survnm);
    }

    for (auto survnm : use) {
        wmSurvey survey(basedir, py::cast<std::string>(survnm));
        if (!survey.epsgCode().empty()) {
            names.append(std::string(survey.name()));
            types.append(std::string(survey.type()));
            polys.append(Polygon(survey.getSurveyPoints(true)));
        }
    }
    res["Name"] = names;
    res["Type"] = types;
    res["geometry"] = polys;
    return GDF(res, "crs"_a="EPSG:4326");
}

void init_wmodpy_survey(py::module_& m) {
    m.def("get_surveys", &GetSurveys, 
        "Return list of survey names in given survey data root", "data_root"_a);

    m.def("get_survey_info", &GetSurveyInfo,
        "Return a python dictionary with basic information for the surveys in the list",
        "data_root"_a, "surveys"_a=py::list());

    m.def("get_survey_info_df", [](const char* basedir, py::list surveys) {
        auto PDF = py::module::import("pandas").attr("DataFrame");
        return PDF(GetSurveyInfo(basedir, surveys));
    }, "Return a Pandas dataframe with basic information for the surveys in the list",
    "data_root"_a, "surveys"_a=py::list());

    m.def("get_survey_info_gdf", &GetSurveyInfoGDF,
        "Return a GeoPandas GeoDataFrame with basic information for the surveys in the list",
        "data_root"_a, "surveys"_a=py::list());

    py::class_<wmSurvey>(m, "Survey", "Encapsulates an OpendTect survey")
	.def(py::init<const std::string&, const std::string&>(), "data_root"_a, "survey_name"_a)
	.def("name", &wmSurvey::name, "Return the survey name")
    .def("info", &wmSurvey::getSurveyInfo,
	     "Return dict with basic information for the survey")
	.def("info_df", &wmSurvey::getSurveyInfoDF,
	     "Return Pandas dataframe with basic information for the survey - requires Pandas")
	.def("info_gdf", &wmSurvey::getSurveyInfoGDF,
	     "Return GeoPandas geodataframe with basic information for the survey - requires GeoPandas")
	.def("points", &wmSurvey::getSurveyPoints,
        "Return a list of tuples defining the survey extent in the survey CRS or optionally in WGS84", "towgs"_a=false)
    .def("isok", &wmSurvey::isOK, "Return True if the survey is properly setup and accessible")
	.def("has2d", &wmSurvey::has2D, "Return True if the survey contains 2D seismic data")
	.def("has3d", &wmSurvey::has3D, "Return True if the the survey contains 3D seismic data")
	.def("epsg", &wmSurvey::epsgCode, "Return the survey CRS EPSG code");
}

#ifdef __win__
extern "C" { mGlobal(Basic) void SetExecutablePathOverrule(const char*); }
#endif
void wmSurvey::initModule()
{
    if (!modulepath_.empty())
	return;

    py::gil_scoped_acquire acquire;
    py::object wmodpy = py::module::import("wmodpy");
    modulepath_ = wmodpy.attr("__file__").cast<std::string>();

    OD::SetRunContext(OD::BatchProgCtxt);
#ifdef __win__
    SetExecutablePathOverrule(modulepath_.c_str());
#endif
    OS::MachineCommand mc;
    mc.addArg(modulepath_.c_str());
    int argc = mc.args().size();
    char** argv = new char*[argc+1];
    for (int idx=0; idx<argc; idx++) {
        BufferString* arg = new BufferString(mc.args().get(idx));
        argv[idx] = arg->getCStr();
    }
    argv[argc] = 0;
    SetProgramArgs( argc, argv);
    
    OD::ModDeps().ensureLoaded("Well");
    OD::ModDeps().ensureLoaded("EarthModel");
    OD::ModDeps().ensureLoaded("CRS");
}

wmSurvey::wmSurvey(const std::string& basedir, const std::string& surveynm)
    : basedir_(basedir)
    , survey_(surveynm)
{
    activate();
    FilePath fp(basedir.c_str(), surveynm.c_str());
    const BufferString fpstr = fp.fullPath();
    if (File::exists(fpstr))
	si_ = SurveyInfo::read(fpstr);
}

wmSurvey::~wmSurvey()
{
    if (si_) delete si_;
}

std::string wmSurvey::name() const
{
    activate();
    return si_ ? std::string(si_->name()) : std::string();
}

std::string wmSurvey::type() const
{
    std::string res;
    activate();
    if (si_) {
        if (has2D()) res += "2D";
        if (has3D()) res += "3D";
    }
    return res;
}

py::dict wmSurvey::getSurveyInfo() const
{
    py::dict dict;
    activate();
    if (si_) {
        py::list names, types, crs;
        names.append(name());
        types.append(type());
        crs.append(epsgCode());
        dict["Name"] = names;
        dict["Type"] = types;
        dict["crs"] = crs;
    }

    return dict;
}

py::object wmSurvey::getSurveyInfoDF() const
{
    auto PDF = py::module::import("pandas").attr("DataFrame");
    return PDF(getSurveyInfo());
}

py::object wmSurvey::getSurveyInfoGDF() const
{
    auto GDF = py::module::import("geopandas").attr("GeoDataFrame");
    auto Polygon = py::module::import("shapely.geometry").attr("Polygon");
    py::dict info = getSurveyInfo();
    py::list polys;
    polys.append(Polygon(getSurveyPoints(false)));
    info["geometry"] = polys;
    return GDF(info, "crs"_a=epsgCode());
}


py::list wmSurvey::getSurveyPoints(bool towgs) const
{
    py::list points;
    activate();
    if (si_) {
        const StepInterval<int> inlrg = si_->inlRange( false );
        const StepInterval<int> crlrg = si_->crlRange( false );
        Coord coord[4];
        const Coords::CoordSystem* coordsys = si_->getCoordSystem();
        coord[0] = si_->transform(BinID(inlrg.start,crlrg.start));
        coord[1] = si_->transform(BinID(inlrg.start,crlrg.stop));
        coord[2] = si_->transform(BinID(inlrg.stop,crlrg.stop));
        coord[3] = si_->transform(BinID(inlrg.stop,crlrg.start));
        for (int i=0; i<4; i++) {
            if (towgs) {
                const LatLong ll( LatLong::transform(coord[i], true, coordsys) );
                points.append(py::make_tuple(ll.lng_, ll.lat_));
            } else
                points.append(py::make_tuple(coord[i].x, coord[i].y));
        }
    }
    return points;
}

bool wmSurvey::isOK() const
{
    return si_;
}

bool wmSurvey::has2D() const
{
    return si_ ? si_->has2D() : false;
}

bool wmSurvey::has3D() const
{
    return si_ ? si_->has3D() : false;
}

std::string wmSurvey::epsgCode() const
{
    BufferString epsg;
    FilePath fp( basedir_.c_str(), survey_.c_str(), SurveyInfo::sKeySetupFileName() );
    SafeFileIO sfio( fp.fullPath(), false );
    if ( sfio.open(true) )
    {
        ascistream astream( sfio.istrm() );
        if ( astream.isOfFileType("Survey Info") )
        {
            astream.next();
            const IOPar survpar( astream );
            PtrMan<IOPar> coordsystempar = survpar.subselect( sKey::CoordSys() );
            if ( si_ && !coordsystempar )
	            coordsystempar = si_->pars().subselect( sKey::CoordSys() );
            if ( coordsystempar )
            {
                epsg = coordsystempar->find("Projection.ID");
                epsg.replace("`", ":");
            }
        }
        sfio.closeSuccess();
    }
    return std::string(epsg);
}

std::string wmSurvey::surveyPath() const
{
    activate();
    FilePath fp(si_->getDataDirName(), si_->getDirName());
    return std::string(fp.fullPath());
}

extern "C" { mGlobal(Basic) void SetCurBaseDataDirOverrule(const char*); }

void wmSurvey::activate() const {
    initModule();
    if (basedir_==curbasedir_ && survey_==cursurvey_)
        return;

    SetCurBaseDataDirOverrule(basedir_.c_str());
    IOM().setDataSource(basedir_.c_str(), survey_.c_str(), true);
    curbasedir_ = basedir_;
    cursurvey_ = survey_;
//    Well::MGR().cleanup();
}
