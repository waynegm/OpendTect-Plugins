#include "idwgridder2d.h"

#include "paralleltask.h"
#include "survinfo.h"

#include "nanoflann_extra.h"


mDefParallelCalc2Pars( IDWGlobalInterpolator, od_static_tr("IDWGlobalInterpolator","IDW global interpolation"),
		       const wmIDWGridder2D*, interp, Threads::Lock, lock )
mDefParallelCalcBody( 
const TypeSet<Coord>& locs_ = interp_->binLocs_;
const TypeSet<float>& vals_ = interp_->vals_;
const TrcKeySampling& hs_ = interp_->hs_;
Array2DImpl<float>* grid_ = interp_->grid_;
TypeSet<od_int64> interpidx_ = interp_->interpidx_;
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
double val = 0.0;
double wgtsum = 0.0;
for (int i=0; i<locs_.size(); i++) {
    Coord locPos(locs_[i]);
    if (interp_->faultBetween(gridPos, locPos))
	continue;

    double d = locPos.sqHorDistTo(gridPos);
    double wgt = 1.0 / (d+0.001);
    val += vals_[i] * wgt;
    wgtsum += wgt;
}

float fval = mIsZero(wgtsum, mDefEpsD) ? mUdf(float) : val/wgtsum;
Threads::Locker lckr( lock_ );
grid_->set(ix, iy, fval);
, )

mDefParallelCalc3Pars( IDWKNNInterpolator, od_static_tr("IDWKNNInterpolator","IDW KNN interpolation "),
		       const wmIDWGridder2D*, interp, CoordKDTree&, index, Threads::Lock, lock )
mDefParallelCalcBody(
const TypeSet<Coord>& locs_ = interp_->binLocs_;
const TypeSet<float>& vals_ = interp_->vals_;
const TrcKeySampling& hs_ = interp_->hs_;
Array2DImpl<float>* grid_ = interp_->grid_;
TypeSet<od_int64> interpidx_ = interp_->interpidx_;
std::vector<size_t> resindex(interp_->maxpoints_);
std::vector<Pos::Ordinate_Type> distsq(interp_->maxpoints_);
Pos::Ordinate_Type pt[2];
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
double val = 0.0;
double wgtsum = 0.0;

int nrpoints = index_.knnSearch(&pt[0], interp_->maxpoints_, &resindex[0], &distsq[0]);

for (int i=0; i<nrpoints; i++) {
    Coord locPos(locs_[resindex[i]]);
    if (interp_->faultBetween(gridPos, locPos))
	continue;

    double d = distsq[i];
    double wgt = 1.0 / (d+0.001);
    val += vals_[resindex[i]] * wgt;
    wgtsum += wgt;
}

float fval = mIsZero(wgtsum, mDefEpsD) ? mUdf(float) : val/wgtsum;
Threads::Locker lckr( lock_ );
grid_->set(ix, iy, fval);
, )

typedef std::vector<std::pair<size_t,Pos::Ordinate_Type>> RadiusResultSet;

mDefParallelCalc3Pars( IDWLocalInterpolator, od_static_tr("IDWLocalInterpolator","IDW local interpolation"),
		       const wmIDWGridder2D*, interp, CoordKDTree&, index, Threads::Lock, lock )
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
TypeSet<Coord> usePos;
TypeSet<float> useVal;
TypeSet<Pos::Ordinate_Type> useDSQ;
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
if (nrpoints == 0) {
    Threads::Locker lckr( lock_ );
    grid_->set(ix, iy, mUdf(float));
    continue;
}

usePos.erase();
useVal.erase();
useDSQ.erase();
for (int i=0; i<nrpoints; i++) {
    Coord locPos(locs_[result[i].first]);
    if (interp_->faultBetween(gridPos, locPos))
	continue;

    usePos += locPos;
    useVal += vals_[result[i].first];
    useDSQ += result[i].second;
    if (!mIsUdf(interp_->maxpoints_) && usePos.size()==interp_->maxpoints_)
	break;
}
double val = 0.0;
double wgtsum = 0.0;
for (int i=0; i<usePos.size(); i++) {
    Coord locPos(usePos[i]);

    double d = useDSQ[i];
    double wgt = 1.0 / (d+0.001);
    val += useVal[i] * wgt;
    wgtsum += wgt;
}

float fval = mIsZero(wgtsum, mDefEpsD) ? mUdf(float) : val/wgtsum;
Threads::Locker lckr( lock_ );
grid_->set(ix, iy, fval);
, )



wmIDWGridder2D::wmIDWGridder2D()
{}

bool wmIDWGridder2D::executeGridding(TaskRunner* tr)
{
    localInterp();
    const CoordTypeSetAdaptor coords( binLocs_ );
    CoordKDTree index( 2, coords );
    Threads::Lock lock;

    if ( mIsUdf(maxpoints_) && mIsUdf(searchradius_) ) {
	IDWGlobalInterpolator interp( interpidx_.size(), this, lock );
	if (tr)
	    return TaskRunner::execute( tr, interp );
	else
	    interp.execute();
    } else if ( !mIsUdf(maxpoints_) && mIsUdf(searchradius_) ) {
	index.buildIndex();
	IDWKNNInterpolator interp( interpidx_.size(), this, index, lock );
	if (tr)
	    return TaskRunner::execute( tr, interp );
	else
	    interp.execute();
    } else {
	index.buildIndex();
	IDWLocalInterpolator interp( interpidx_.size(), this, index, lock );
	if (tr)
	    return TaskRunner::execute( tr, interp );
	else
	    interp.execute();
    }

    return true;
}

