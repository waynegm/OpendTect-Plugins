/*
 *   Multistage MBA gridder class
 *   Copyright (C) 2019  Wayne Mogg
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "msmbagridder2d.h"
#include "nanoflann_extra.h"
#include "mba.hpp"

#include "paralleltask.h"
#include "survinfo.h"

typedef std::vector<std::pair<size_t,Pos::Ordinate_Type>> RadiusResultSet;
typedef mba::point<2> MBAPoint2D;
typedef std::vector<MBAPoint2D> MBAPointVector;
typedef mba::MBA<2> MBA2D;
typedef mba::index<2> MBAIndex2D;

MBAPoint2D makePoint(double x, double y)
{
    MBAPoint2D p = {{ x, y }};
    return p;
}

MBAIndex2D makeIndex( int x, int y )
{
    MBAIndex2D p = {{x, y}};
    return p;
}

mDefParallelCalc3Pars( MSMBAInterpolator, od_static_tr("MSMBAInterpolator","Multistage MBA interpolation"),
		       const wmMSMBAGridder2D*, interp, CoordKDTree&, index, Threads::Lock, lock )
mDefParallelCalcBody(
const TypeSet<Coord>& locs_ = interp_->binLocs_;
const TypeSet<float>& vals_ = interp_->vals_;
const TrcKeySampling& hs_ = interp_->hs_;
Array2DImpl<float>* grid_ = interp_->grid_;
TypeSet<od_int64> interpidx_ = interp_->interpidx_;
RadiusResultSet result;
Pos::Ordinate_Type srsq = interp_->searchradius_/(SI().inlDistance()+SI().crlDistance())*2.0;
srsq *= srsq;
nanoflann::SearchParams params;
Pos::Ordinate_Type pt[2];
MBAPointVector mbaLocs;
std::vector<float> mbaVals;
,
Task::Control state = getState();
if (state==Task::Stop)
    break;
else if (state==Task::Pause) {
    while (getState()==Task::Pause)
	Threads::sleep(1);
}

BinID gridBid = hs_.atIndex(interpidx_[idx]);
int ix = hs_.inlIdx(gridBid.inl());
int iy = hs_.crlIdx(gridBid.crl());
double x = gridBid.inl();
double y = gridBid.crl();
Coord gridPos(x, y);
pt[0] = x;
pt[1] = y;

int nrpoints = index_.radiusSearch(&pt[0], srsq, result, params);
if (nrpoints <7) {
    Threads::Locker lckr( lock_ );
    grid_->set(ix, iy, mUdf(float));
    continue;
}

mbaLocs.clear();
mbaVals.clear();
TrcKeySampling lhs;
for (int i=0; i<nrpoints; i++) {
    Coord locPos(locs_[result[i].first]);
    if (interp_->faultBetween(gridPos, locPos))
	continue;

    mbaLocs.push_back(makePoint(locPos.x, locPos.y));
    mbaVals.push_back(vals_[result[i].first]);
    lhs.include(TrcKey(BinID(mNINT32(locPos.x), mNINT32(locPos.y))));
}

nrpoints = mbaLocs.size();

if (nrpoints <7) {
    grid_->set(ix, iy, mUdf(float));
    continue;
}

MBAPoint2D lo = makePoint((double)lhs.start_.inl(), (double)lhs.start_.crl());
MBAPoint2D hi = makePoint((double)lhs.stop_.inl(), (double)lhs.stop_.crl());

MBAIndex2D grid = makeIndex(2, 2);
MBA2D mbainterp(lo, hi, grid, mbaLocs, mbaVals );

double val = mbainterp(makePoint(x, y));

Threads::Locker lckr( lock_ );
val += grid_->get(ix,iy);
grid_->set(ix, iy, (float)val);
, )


wmMSMBAGridder2D::wmMSMBAGridder2D()
{}

bool wmMSMBAGridder2D::executeGridding(TaskRunner* tr)
{
    localInterp();
    calcResidual();

    const CoordTypeSetAdaptor coords( binLocs_ );
    CoordKDTree nfindex( 2, coords );
    Threads::Lock lock;
    nfindex.buildIndex();
    MSMBAInterpolator interp( interpidx_.size(), this, nfindex, lock );
    if (tr)
	return TaskRunner::execute( tr, interp );
    else
	interp.execute();

    return true;
}

void wmMSMBAGridder2D::calcResidual()
{
    std::vector<mba::point<2>> binLocs(binLocs_.size());
    std::vector<float> vals(binLocs_.size());
    for (int idx=0; idx<binLocs_.size(); idx++) {
	binLocs[idx] = mba::point<2>{{binLocs_[idx].x, binLocs_[idx].y}};
	vals[idx] = vals_[idx];
    }

    mba::point<2> lo = {{ (double)hs_.start_.inl(), (double)hs_.start_.crl() }};
    mba::point<2> hi = {{ (double)hs_.stop_.inl(), (double)hs_.stop_.crl() }};

    mba::index<2> grid = {{ 2, 2 }};

    mba::MBA<2> interp(lo, hi, grid, binLocs, vals );

    for (od_int64 idx=0; idx<binLocs_.size(); idx++) {
	vals_[idx] -= interp(mba::point<2>{{binLocs_[idx].x, binLocs_[idx].y}});
    }

    for (od_int64 idx=0; idx<interpidx_.size(); idx++) {
	BinID gridBid = hs_.atIndex(interpidx_[idx]);
	int ix = hs_.inlIdx(gridBid.inl());
	int iy = hs_.crlIdx(gridBid.crl());
	grid_->set(ix, iy, interp(mba::point<2>{{(double)gridBid.inl(), (double)gridBid.crl()}}));
    }
}
