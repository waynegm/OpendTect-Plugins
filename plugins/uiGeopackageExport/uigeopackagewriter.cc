#include "uigeopackagewriter.h"

#include "bufstring.h"
#include "bufstringset.h"
#include "coordsystem.h"
#include "crsproj.h"
#include "crssystem.h"
#include "ctxtioobj.h"
#include "emhorizon3d.h"
#include "emhorizon2d.h"
#include "emioobjinfo.h"
#include "emmanager.h"
#include "emsurfaceiodata.h"
#include "emsurfacetr.h"
#include "errmsg.h"
#include "file.h"
#include "ioman.h"
#include "pickset.h"
#include "picksettr.h"
#include "posinfo2d.h"
#include "randomlinegeom.h"
#include "randomlinetr.h"
#include "string2.h"
#include "survinfo.h"
#include "survgeom2d.h"
#include "welldata.h"
#include "wellman.h"
#include "wellmarker.h"
#include "welltrack.h"
#include "uistring.h"
#include "uimsg.h"

#include "gpkgio.h"

uiGeopackageWriter::uiGeopackageWriter( const char* filename, bool append )
: gpkg_(nullptr), srs_(nullptr), append_(append)
{
    if (SI().getCoordSystem()->isProjection())
    {
	mDynamicCastGet(const Coords::ProjectionBasedSystem*, projsys, SI().getCoordSystem().ptr());
	if (!projsys)
	{
	    BufferString tmp("uiGeopackageWriter::uiGeopackageWriter - getting survey CRS failed \n");
            ErrMsg(tmp);
	}
	const auto* proj = projsys->getProjection();
	if (proj && open(filename))
	{
	    srs_ = new Coords::AuthorityCode(proj->authCode());
	    if (!gpkg_ || !gpkg_->addSRS(proj->userName(), srs_->code(), srs_->authority(), srs_->code()))
	    {
		BufferString tmp("uiGeopackageWriter::uiGeopackageWriter - setting CRS in output file failed \n");
		tmp += "Survey CRS: ";
		tmp += SI().getCoordSystem()->summary();
		tmp += "\n";
		ErrMsg(tmp);
		close();
	    }
	}
    }
    else
    {
        BufferString crsSummary("uiGeopackageWriter::uiGeopackageWriter - not a projected CRS: ");
        crsSummary += SI().getCoordSystem()->summary();
        ErrMsg(crsSummary);
    }
}

uiGeopackageWriter::~uiGeopackageWriter()
{
    close();
}

bool uiGeopackageWriter::open( const char* filename )
{
    if (!append_ && File::exists(filename))
	if (!File::remove(filename))
	{
	    BufferString tmp("GeopackageWriter::file already exists and can't be removed. ");
	    ErrMsg(tmp);
	    return false;
	}

    gpkg_ = new GeopackageIO(filename, append_);
    if (!gpkg_->isOK())
    {
	BufferString tmp("GeopackageWriter::open failed with error: ");
	tmp.add(gpkg_->errorMsg());
	ErrMsg(tmp);
	return false;
    }

    return true;
}

void uiGeopackageWriter::close()
{
    delete gpkg_;
    gpkg_ = nullptr;
    delete srs_;
    srs_ = nullptr;
}

void uiGeopackageWriter::writeSurvey()
{
    if (!gpkg_ || !srs_)
	return;

    std::vector<std::string> fieldnms({"name"});
    std::vector<std::string> fielddefs({"TEXT NOT NULL"});
    if (!gpkg_->addGeomLayer("polygon", "survey", srs_->code(), fieldnms, fielddefs))
    {
	ErrMsg("uiGeopackageWriter::writeSurvey - creation of feature table failed.");
	ErrMsg(gpkg_->errorMsg());
	return;
    }

    sqlite3_stmt* stmt;
    if (!gpkg_->makeGeomStatement(&stmt, "survey", srs_->code(), fieldnms))
    {
	sqlite3_finalize(stmt);
	ErrMsg("uiGeopackageWriter::writeSurvey - creation of output statement failed.");
	ErrMsg(gpkg_->errorMsg());
	return;
    }

    std::vector<double> points;
    points.reserve(5);
    const TrcKeySampling tk = SI().sampling( false ).hsamp_;
    Coord coord;

    coord = tk.toCoord(tk.corner(0));
    points.push_back(coord.x);
    points.push_back(coord.y);
    coord = tk.toCoord(tk.corner(1));
    points.push_back(coord.x);
    points.push_back(coord.y);
    coord = tk.toCoord(tk.corner(3));
    points.push_back(coord.x);
    points.push_back(coord.y);
    coord = tk.toCoord(tk.corner(2));
    points.push_back(coord.x);
    points.push_back(coord.y);
    coord = tk.toCoord(tk.corner(4));
    points.push_back(coord.x);
    points.push_back(coord.y);

    std::vector<std::vector<double>> poly({points});
    gpkg_->startTransaction();
    if (!gpkg_->addPolygon(stmt, poly, GetSurveyName()))
    {
	ErrMsg("uiGeopackageWriter::writeSurvey - writing feature failed.");
	ErrMsg(gpkg_->errorMsg());
	gpkg_->rollbackTransaction();
    }
    else
	gpkg_->commitTransaction();

    sqlite3_finalize(stmt);
}

void uiGeopackageWriter::write2DLines( TypeSet<Pos::GeomID>& geomids )
{
    if (!gpkg_ || !srs_)
	return;

    std::vector<std::string> fieldnms({"line_name"});
    std::vector<std::string> fielddefs({"TEXT NOT NULL"});
    if (!gpkg_->addGeomLayer("linestring", "lines2d", srs_->code(), fieldnms, fielddefs))
    {
	ErrMsg("uiGeopackageWriter::write2DLines - creation of feature table failed.");
	ErrMsg(gpkg_->errorMsg());
	return;
    }

    sqlite3_stmt* stmt;
    if (!gpkg_->makeGeomStatement(&stmt, "lines2d", srs_->code(), fieldnms))
    {
	sqlite3_finalize(stmt);
	ErrMsg("uiGeopackageWriter::write2DLines - creation of output statement failed.");
	ErrMsg(gpkg_->errorMsg());
	return;
    }

    for ( int idx=0; idx<geomids.size(); idx++ )
    {
	mDynamicCastGet( const Survey::Geometry2D*, geom2d, Survey::GM().getGeometry(geomids[idx]) );
	if ( !geom2d )
	    continue;

	const PosInfo::Line2DData& geom = geom2d->data();
	const TypeSet<PosInfo::Line2DPos>& posns = geom.positions();

	std::vector<double> points;
	points.reserve(posns.size()*2);
	for ( int tdx=0; tdx<posns.size(); tdx++ )
	{
	    const Coord pos = posns[tdx].coord_;
	    points.push_back(pos.x);
	    points.push_back(pos.y);
	}

	gpkg_->startTransaction();
	if (!gpkg_->addLineString(stmt, points, geom.lineName().buf()))
	{
	    ErrMsg("uiGeopackageWriter::write2DLines - writing feature failed.");
	    ErrMsg(gpkg_->errorMsg());
	    gpkg_->rollbackTransaction();
	}
	else
	    gpkg_->commitTransaction();
    }
    sqlite3_finalize(stmt);
}

void uiGeopackageWriter::write2DStations( TypeSet<Pos::GeomID>& geomids )
{
    if (!gpkg_ || !srs_)
	return;

    std::vector<std::string> fieldnms({"line_name", "station"});
    std::vector<std::string> fielddefs({"TEXT NOT NULL", "INTEGER"});
    if (!gpkg_->addGeomLayer("point", "stations2d", srs_->code(), fieldnms, fielddefs))
    {
	ErrMsg("uiGeopackageWriter::write2DStations - creation of feature table failed.");
	ErrMsg(gpkg_->errorMsg());
	return;
    }

    sqlite3_stmt* stmt;
    if (!gpkg_->makeGeomStatement(&stmt, "stations2d", srs_->code(), fieldnms))
    {
	sqlite3_finalize(stmt);
	ErrMsg("uiGeopackageWriter::write2DStations - creation of output statement failed.");
	ErrMsg(gpkg_->errorMsg());
	return;
    }

    for ( int idx=0; idx<geomids.size(); idx++ )
    {
	mDynamicCastGet( const Survey::Geometry2D*, geom2d, Survey::GM().getGeometry(geomids[idx]) );
	if ( !geom2d )
	    continue;
	const PosInfo::Line2DData& geom = geom2d->data();
	const TypeSet<PosInfo::Line2DPos>& posns = geom.positions();

	gpkg_->startTransaction();
	for ( int tdx=0; tdx<posns.size(); tdx++ )
	{
	    if ( !gpkg_->addPoint(stmt, posns[tdx].coord_.x, posns[tdx].coord_.y,
			     geom2d->getName(), posns[tdx].nr_))
	    {
		ErrMsg("uiGeopackageWriter::write2DStations - writing feature failed.");
		ErrMsg(gpkg_->errorMsg());
		gpkg_->rollbackTransaction();
		break;
	    }
	}
	gpkg_->commitTransaction();
    }
    sqlite3_finalize(stmt);
}

void uiGeopackageWriter::writeRandomLines( TypeSet<MultiID>& lineids )
{
    std::vector<std::string> fieldnms({"line_name"});
    std::vector<std::string> fielddefs({"TEXT NOT NULL"});
    if (!gpkg_->addGeomLayer("linestring", "random_lines", srs_->code(), fieldnms, fielddefs))
    {
	ErrMsg("uiGeopackageWriter::writeRandomLines - creation of feature table failed.");
	ErrMsg(gpkg_->errorMsg());
	return;
    }

    sqlite3_stmt* stmt;
    if (!gpkg_->makeGeomStatement(&stmt, "random_lines", srs_->code(), fieldnms))
    {
	sqlite3_finalize(stmt);
	ErrMsg("uiGeopackageWriter::writeRandomLines - creation of output statement failed.");
	ErrMsg(gpkg_->errorMsg());
	return;
    }

    for ( int idx=0; idx<lineids.size(); idx++ )
    {
	Geometry::RandomLineSet inprls;
	BufferString msg;
	IOObj* ioobj = IOM().get(lineids[idx]);
	if ( ioobj == nullptr )
	{
	    ErrMsg("uiGeopackageWriter::writeRandomLines - cannot get ioobj" );
	    continue;
	}

	if (!RandomLineSetTranslator::retrieve( inprls, ioobj, msg ))
	{
	    BufferString tmp("uiGeopackageWriter::writeRandomLines - error reading random line - ");
	    tmp += msg;
	    ErrMsg(tmp);
	    return;
	}

	for (int rdx=0; rdx<inprls.size(); rdx++)
	{
	    const Geometry::RandomLine* rdl = inprls.lines()[rdx];
	    if ( rdl == nullptr ) {
		ErrMsg("uiGeopackageWriter::writeRandomLines - cannot get random line geometry" );
		continue;
	    }
	    std::vector<double> points;
	    points.reserve(rdl->nrNodes()*2);
	    for ( int tdx=0; tdx<rdl->nrNodes(); tdx++ )
	    {
		const Coord pos = SI().transform(rdl->nodePosition(tdx));
		points.push_back(pos.x);
		points.push_back(pos.y);
	    }

	    gpkg_->startTransaction();
	    if (!gpkg_->addLineString(stmt, points, rdl->name().str()))
	    {
		ErrMsg("uiGeopackageWriter::writeRandomLines - writing feature failed.");
		ErrMsg(gpkg_->errorMsg());
		gpkg_->rollbackTransaction();
		continue;
	    }
	    else
		gpkg_->commitTransaction();
	}
    }
    sqlite3_finalize(stmt);
}

void uiGeopackageWriter::writeWells( TypeSet<MultiID>& wellids )
{
    if (!gpkg_ || !srs_)
	return;

    std::vector<std::string> fieldnms({"name", "uwid", "status"});
    std::vector<std::string> fielddefs({"TEXT NOT NULL", "TEXT", "INTEGER"});
    if (!gpkg_->addGeomLayer("point", "wells", srs_->code(), fieldnms, fielddefs))
    {
	ErrMsg("uiGeopackageWriter::writeWells - creation of feature table failed.");
	ErrMsg(gpkg_->errorMsg());
	return;
    }

    sqlite3_stmt* stmt;
    if (!gpkg_->makeGeomStatement(&stmt, "wells", srs_->code(), fieldnms))
    {
	sqlite3_finalize(stmt);
	ErrMsg("uiGeopackageWriter::writeWells - creation of output statement failed.");
	ErrMsg(gpkg_->errorMsg());
	return;
    }
    for ( int idx=0; idx<wellids.size(); idx++ )
    {
	ConstRefMan<Well::Data> wd = Well::MGR().get(wellids[idx]);
	if ( !wd )
	{
	    ErrMsg("uiGeopackageWriter::writeWells - unable to read well data.");
	    continue;
	}
	const Well::Info& wdinfo = wd->info();
	gpkg_->startTransaction();
	if ( !gpkg_->addPoint(stmt, wdinfo.surfacecoord_.x, wdinfo.surfacecoord_.y,
			      wdinfo.name().str(), wdinfo.uwid_.str(), OD::WellTypeDef().indexOf(wdinfo.welltype_)))
	{
	    ErrMsg("uiGeopackageWriter::writeWells - writing well data error.");
	    ErrMsg(gpkg_->errorMsg());
	    gpkg_->rollbackTransaction();
	    continue;
	}
	else
	    gpkg_->commitTransaction();
    }
    sqlite3_finalize(stmt);
}

void uiGeopackageWriter::writeWellTracks( TypeSet<MultiID>& wellids )
{
    if (!gpkg_ || !srs_)
	return;

    std::vector<std::string> fieldnms({"name"});
    std::vector<std::string> fielddefs({"TEXT NOT NULL"});
    if (!gpkg_->addGeomLayer("linestring", "well_tracks", srs_->code(), fieldnms, fielddefs))
    {
	ErrMsg("uiGeopackageWriter::writeWellTracks - creation of feature table failed.");
	ErrMsg(gpkg_->errorMsg());
	return;
    }

    sqlite3_stmt* stmt;
    if (!gpkg_->makeGeomStatement(&stmt, "well_tracks", srs_->code(), fieldnms))
    {
	sqlite3_finalize(stmt);
	ErrMsg("uiGeopackageWriter::writeWellTracks - creation of output statement failed.");
	ErrMsg(gpkg_->errorMsg());
	return;
    }
    for ( int idx=0; idx<wellids.size(); idx++ )
    {
	ConstRefMan<Well::Data> wd = Well::MGR().get(wellids[idx]);
	if (!wd)
	{
	    ErrMsg("uiGeopackageWriter::writeWellTracks - unable to read well data");
	    continue;
	}
	const Well::Info& wdinfo = wd->info();
	const Well::Track& wtrack = wd->track();
	const TypeSet<Coord3>& wtrackpos = wtrack.getAllPos();
	std::vector<double> points;
	points.reserve(wtrackpos.size()*2);
	for (int tdx=0; tdx<wtrackpos.size(); tdx++)
	{
	    Coord3 pos = wtrackpos[tdx];
	    points.push_back(pos.x);
	    points.push_back(pos.y);
	}
	gpkg_->startTransaction();
	if (!gpkg_->addLineString(stmt, points, wdinfo.name().str()))
	{
	    ErrMsg("uiGeopackageWriter::writeWellTracks - writing feature failed.");
	    ErrMsg(gpkg_->errorMsg());
	    gpkg_->rollbackTransaction();
	    continue;
	}
	else
	    gpkg_->commitTransaction();
    }
    sqlite3_finalize(stmt);
}

void uiGeopackageWriter::writeWellMarkers( TypeSet<MultiID>& wellids, bool inFeet )
{
    if (!gpkg_ || !srs_)
	return;

    std::vector<std::string> fieldnms({"name", "marker", inFeet?"md(ft)":"md(m)", inFeet?"tvdss(ft)":"tvdss(m)"});
    std::vector<std::string> fielddefs({"TEXT NOT NULL", "TEXT NOT NULL", "REAL", "REAL"});
    if (!gpkg_->addGeomLayer("point", "well_markers", srs_->code(), fieldnms, fielddefs))
    {
	ErrMsg("uiGeopackageWriter::writeWellMarkers - creation of feature table failed.");
	ErrMsg(gpkg_->errorMsg());
	return;
    }

    sqlite3_stmt* stmt;
    if (!gpkg_->makeGeomStatement(&stmt, "well_markers", srs_->code(), fieldnms))
    {
	sqlite3_finalize(stmt);
	ErrMsg("uiGeopackageWriter::writeWellMarkers - creation of output statement failed.");
	ErrMsg(gpkg_->errorMsg());
	return;
    }
    for ( int idx=0; idx<wellids.size(); idx++ )
    {
	ConstRefMan<Well::Data> wd = Well::MGR().get(wellids[idx]);
	if (!wd)
	{
	    ErrMsg("uiGeopackageWriter::writeWellMarkers - unable to read well data");
	    continue;
	}
	const Well::Info& wdinfo = wd->info();
	const Well::Track& wtrack = wd->track();
	const Well::MarkerSet& wmarkers = wd->markers();
	for (int tdx=0; tdx<wmarkers.size(); tdx++)
	{
	    float md = wmarkers[tdx]->dah();
	    Coord3 pos = wtrack.getPos(md);
	    float tvdss = pos.z;

	    gpkg_->startTransaction();
	    if ( !gpkg_->addPoint(stmt, pos.x, pos.y, wdinfo.name().str(), wmarkers[tdx]->name().str(), md, tvdss))
	    {
		ErrMsg("uiGeopackageWriter::writeWellMarkers - writing well data error.");
		ErrMsg(gpkg_->errorMsg());
		gpkg_->rollbackTransaction();
		continue;
	    }
	    else
		gpkg_->commitTransaction();
	}
    }
    sqlite3_finalize(stmt);
}

void uiGeopackageWriter::writePolyLines( TypeSet<MultiID>& lineids )
{
    if (!gpkg_ || !srs_ || !lineids.size())
	return;

    std::vector<std::string> fieldnms({"name"});
    std::vector<std::string> fielddefs({"TEXT NOT NULL"});
    if (!gpkg_->addGeomLayer("linestring", "polylines", srs_->code(), fieldnms, fielddefs) ||
	!gpkg_->addGeomLayer("polygon", "polygons", srs_->code(), fieldnms, fielddefs))
    {
	ErrMsg("uiGeopackageWriter::writePolyLines - creation of feature table failed.");
	ErrMsg(gpkg_->errorMsg());
	return;
    }

    sqlite3_stmt* pl_stmt;
    sqlite3_stmt* pg_stmt;
    if (!gpkg_->makeGeomStatement(&pl_stmt, "polylines", srs_->code(), fieldnms) ||
	!gpkg_->makeGeomStatement(&pg_stmt, "polygons", srs_->code(), fieldnms))
    {
	sqlite3_finalize(pl_stmt);
	sqlite3_finalize(pg_stmt);
	ErrMsg("uiGeopackageWriter::writePolyLines - creation of output statement failed.");
	ErrMsg(gpkg_->errorMsg());
	return;
    }

    for ( int idx=0; idx<lineids.size(); idx++ )
    {
	RefMan<Pick::Set> ps = new Pick::Set;
	BufferString msg;
	IOObj* ioobj = IOM().get(lineids[idx]);
	if ( !ioobj ) {
	    ErrMsg("uiGeopackageWriter::writePolyLines - cannot get ioobj" );
	    continue;
	}

	if (!PickSetTranslator::retrieve(*ps, ioobj, true, msg))
	{
	    BufferString tmp("uiGeopackageWriter::writePolyLines - error reading polyline - ");
	    tmp += msg;
	    ErrMsg(tmp);
	    return;
	}

	const bool ispolygon = ps->isPolygon() && ps->disp_.connect_==Pick::Set::Disp::Close;
	std::vector<double> points;
	points.reserve(ispolygon ? ps->size()*2+2 : ps->size()*2);
	for (int rdx=0; rdx<ps->size(); rdx++)
	{
	    const Coord3 pos = ps->get(rdx).pos();
	    points.push_back(pos.x);
	    points.push_back(pos.y);
	}
	if (ispolygon)
	{
	    points.push_back(ps->get(0).pos().x);
	    points.push_back(ps->get(0).pos().y);
	    std::vector<std::vector<double>> poly({points});
	    gpkg_->startTransaction();
	    if (!gpkg_->addPolygon(pg_stmt, poly, ps->name().str()))
	    {
		ErrMsg("uiGeopackageWriter::writePolyLines - writing feature failed.");
		ErrMsg(gpkg_->errorMsg());
		gpkg_->rollbackTransaction();
		continue;
	    }
	    else
		gpkg_->commitTransaction();
	}
	else
	{
	    gpkg_->startTransaction();
	    if (!gpkg_->addLineString(pl_stmt, points, ps->name().str()))
	    {
		ErrMsg("uiGeopackageWriter::writePolyLines - writing feature failed.");
		ErrMsg(gpkg_->errorMsg());
		gpkg_->rollbackTransaction();
		continue;
	    }
	    else
		gpkg_->commitTransaction();
	}
    }
    sqlite3_finalize(pl_stmt);
    sqlite3_finalize(pg_stmt);
}

void uiGeopackageWriter::writeHorizon( const char* layerName,
				       const MultiID& hor2Dkey, const char* attrib2D, const TypeSet<Pos::GeomID>& geomids,
				       const MultiID& hor3Dkey, const char* attrib3D, const TrcKeyZSampling& cs  )
{
    if (!gpkg_ || !srs_)
	return;

    BufferString attrib("zvals");
    if (!hor2Dkey.isUdf() && attrib2D)
	attrib = attrib2D;

    if (!hor3Dkey.isUdf() && attrib3D)
    {
	if (!attrib2D)
	    attrib = attrib3D;
	else if (!caseInsensitiveEqual(attrib2D, attrib3D))
	{
	    attrib += "_";
	    attrib += attrib3D;
	}
    }

    std::vector<std::string> fieldnms({attrib.getCStr()});
    std::vector<std::string> fielddefs({"REAL"});
    if (!gpkg_->addGeomLayer("point", layerName, srs_->code(), fieldnms, fielddefs))
    {
	ErrMsg("uiGeopackageWriter::writeHorizon - creation of feature table failed.");
	ErrMsg(gpkg_->errorMsg());
	return;
    }

    sqlite3_stmt* stmt;
    if (!gpkg_->makeGeomStatement(&stmt, layerName, srs_->code(), fieldnms))
    {
	sqlite3_finalize(stmt);
	ErrMsg("uiGeopackageWriter::writeHorizon - creation of output statement failed.");
	ErrMsg(gpkg_->errorMsg());
	return;
    }

    const float zfac = attrib=="zvals" && SI().zIsTime() ? 1000 : 1;

    if (!hor2Dkey.isUdf() && geomids.size()>0)
    {
	EM::EMObject* obj = EM::EMM().loadIfNotFullyLoaded(hor2Dkey);
	if (!obj)
	{
	    sqlite3_finalize(stmt);
	    ErrMsg("uiGeopackageWriter::writeHorizon - loading 2D horizon failed");
	    return;
	}
	obj->ref();
	mDynamicCastGet(EM::Horizon2D*,hor,obj);
	if (!hor)
	{
	    sqlite3_finalize(stmt);
	    ErrMsg("uiGeopackageWriter::writeHorizon - casting 2D horizon failed");
	    obj->unRef();
	    return;
	}

	for (int idx=0; idx<geomids.size(); idx++)
	{
	    const StepInterval<int> trcrg = hor->geometry().colRange( geomids[idx] );
	    mDynamicCastGet(const Survey::Geometry2D*,survgeom2d,Survey::GM().getGeometry(geomids[idx]))
	    if (!survgeom2d || trcrg.isUdf() || !trcrg.step)
		continue;

	    TrcKey tk( geomids[idx], -1 );
	    Coord pos;
	    float spnr = mUdf(float);
	    gpkg_->startTransaction();
	    for ( int trcnr=trcrg.start; trcnr<=trcrg.stop; trcnr+=trcrg.step )
	    {
		tk.setTrcNr( trcnr );
		const float z = hor->getZ( tk );
		if (mIsUdf(z))
		    continue;
		const float scaledZ = z*zfac;
		survgeom2d->getPosByTrcNr( trcnr, pos, spnr );
		if ( !gpkg_->addPoint(stmt, pos.x, pos.y, scaledZ))
		{
		    ErrMsg("uiGeopackageWriter::writeHorizon - creating point for 2D horizon failed.");
		    ErrMsg(gpkg_->errorMsg());
		    gpkg_->rollbackTransaction();
		    sqlite3_finalize(stmt);
		    obj->unRef();
		    break;
		}
	    }
	    gpkg_->commitTransaction();
	}
	obj->unRef();
    }

    if (!hor3Dkey.isUdf())
    {
	EM::EMObject* obj = EM::EMM().loadIfNotFullyLoaded(hor3Dkey);
	if (!obj)
	{
	    sqlite3_finalize(stmt);
	    ErrMsg("uiGeopackageWriter::writeHorizon - loading 3D horizon failed");
	    return;
	}
	obj->ref();
	mDynamicCastGet(EM::Horizon3D*,hor,obj);
	if (!hor)
	{
	    sqlite3_finalize(stmt);
	    ErrMsg("uiGeopackageWriter::writeHorizon - casting 3D horizon failed");
	    obj->unRef();
	    return;
	}

	gpkg_->startTransaction();
	TrcKeySampling expSel = cs.hsamp_;
	for (int iln=expSel.start_.inl(); iln<=expSel.stop_.inl(); iln+=expSel.step_.inl())
	{
	    for (int xln=expSel.start_.crl(); xln<=expSel.stop_.crl(); xln+=expSel.step_.crl())
	    {
		BinID bid(iln,xln);
		TrcKey tk(bid);
		const float z = hor->getZ( tk );
		if (mIsUdf(z))
		    continue;

		const float scaledZ = z*zfac;
		const Coord pos = SI().transform(bid);
		if ( !gpkg_->addPoint(stmt, pos.x, pos.y, scaledZ))
		{
		    ErrMsg("uiGeopackageWriter::writeHorizon - creating point for 3D horizon failed.");
		    ErrMsg(gpkg_->errorMsg());
		    gpkg_->rollbackTransaction();
		    sqlite3_finalize(stmt);
		    obj->unRef();
		    break;
		}
	    }
	}
	gpkg_->commitTransaction();
	obj->unRef();
    }
    sqlite3_finalize(stmt);
}
