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

#include "wmodpy_horizons.h"
#include "wmodpy_survey.h"

#include "coord.h"
#include "emhorizonutils.h"
#include "emhorizon3d.h"
#include "emioobjinfo.h"
#include "emmanager.h"
#include "survinfo.h"

namespace py = pybind11;
using namespace pybind11::literals;

void init_wmodpy_horizons(py::module_& m) {
    py::class_<wmHorizons3D>(m, "Horizons3D", "Encapsulates the 3D horizons in an OpendTect survey")
	.def(py::init<const wmSurvey&>())
	.def("names", &wmHorizons3D::getNames,
	     "Return list of all 3D horizon names in the survey")
    .def("get_z", &wmHorizons3D::getZ);

    py::class_<wmHorizons2D>(m, "Horizons2D", "Encapsulates the 2D horizons in an OpendTect survey")
	.def(py::init<const wmSurvey&>())
	.def("names", &wmHorizons2D::getNames,
	     "Return list of all 2D horizon names in the survey");

}


wmHorizons3D::wmHorizons3D( const wmSurvey& thesurvey )
    : survey_(thesurvey)
{ }

py::list wmHorizons3D::getNames() const
{
    py::list horizons;
    survey_.activate();
    ObjectSet<EM::HorizonSelInfo> set;
    EM::HorizonSelInfo::getAll(set, false);
    for (int idx=0; idx<set.size(); idx++)
        horizons.append(std::string(set[idx]->name_));

    return horizons;
}

py::tuple wmHorizons3D::getZ(const std::string& horname) const
{
    py::dict profile;
    py::list transform;
    survey_.activate();
    ObjectSet<EM::HorizonSelInfo> set;
    EM::HorizonSelInfo::getAll(set, false);
    for (int idx=0; idx<set.size(); idx++) {
        if (set[idx]->name_==horname.c_str()) {
            const MultiID hor3dkey = set[idx]->key_;
            EM::IOObjInfo eminfo(hor3dkey);
            if (eminfo.isOK()) {
                TrcKeySampling hs;
                hs.set(eminfo.getInlRange(), eminfo.getCrlRange());
                Coord origin = hs.toCoord(hs.atIndex(0,0));
                Coord delInl = hs.toCoord(hs.atIndex(1,0)) - origin;
                Coord delCrl = hs.toCoord(hs.atIndex(0,1)) - origin;
                origin.x += -0.5*delInl.x - 0.5*delCrl.x;
                origin.y += -0.5*delInl.y - 0.5*delCrl.y;
            
                EM::EMObject* obj = EM::EMM().loadIfNotFullyLoaded(hor3dkey);
                if (!obj)
                    return py::make_tuple(py::none(), py::none());

                obj->ref();
                mDynamicCastGet(EM::Horizon3D*,hor,obj);
                if (!hor)
                    return py::make_tuple(py::none(), py::none());

                py::array_t<float> img({hs.nrInl(), hs.nrCrl()});
                auto r = img.mutable_unchecked<2>();
                const float zfac = SI().zIsTime() ? 1000 : 1;
                for (int inl=0; inl<hs.nrInl(); inl++) {
                    for (int xln = 0; xln<hs.nrCrl(); xln++) {
                        BinID bid = hs.atIndex(inl, xln);
                        TrcKey tk(bid);
                        float z  = hor->getZ(tk);
                        if (!mIsUdf(z))
                            z *= zfac;
                        r(inl,xln) = z;
                    }
                }

                profile["transform"] = py::make_tuple(delInl.x, delCrl.x, origin.x, delInl.y, delCrl.y, origin.y);
                profile["height"] = hs.nrInl();
                profile["width"] = hs.nrCrl();
                profile["nodata"] = mUdf(float);
                profile["dtype"] = "float32";
                profile["crs"] = survey_.epsgCode();
                profile["count"] = 1;
                profile["interleave"] = "band";
                profile["tiled"] = false;
                return py::make_tuple(img, profile);
            }
        }
    }
    return py::make_tuple(py::none(), py::none());
}

wmHorizons2D::wmHorizons2D( const wmSurvey& thesurvey )
    : survey_(thesurvey)
{ }

py::list wmHorizons2D::getNames() const
{
    py::list horizons;
    survey_.activate();
    ObjectSet<EM::HorizonSelInfo> set;
    EM::HorizonSelInfo::getAll(set, true);
    for (int idx=0; idx<set.size(); idx++)
        horizons.append(std::string(set[idx]->name_));
    return horizons;
}

