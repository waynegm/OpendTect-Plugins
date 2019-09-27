#include "idwgridder2d.h"

#include "paralleltask.h"
#include "survinfo.h"

typedef std::vector<std::size_t> TIds;

mDefParallelCalc2Pars( IDWGlobalInterpolator, od_static_tr("IDWGlobalInterpolator","IDW global interpolation"),
		       const wmIDWGridder2D*, interp, Threads::Lock, lock )
mDefParallelCalcBody( 
const std::vector<TPoint>& locs_ = interp_->binLocs_;
const std::vector<float>& vals_ = interp_->vals_;
const TrcKeySampling& hs_ = interp_->hs_;
Array2DImpl<float>* grid_ = interp_->grid_;
TypeSet<od_int64> interpidx_ = interp_->interpidx_;
,
BinID gridBid = hs_.atIndex(interpidx_[idx]);
int ix = hs_.inlIdx(gridBid.inl());
int iy = hs_.crlIdx(gridBid.crl());
double x = gridBid.inl();
double y = gridBid.crl();
Coord gridPos(x, y);
double val = 0.0;
double wgtsum = 0.0;
for (int i=0; i<locs_.size(); i++) {
    Coord locPos(locs_[i].first, locs_[i].second);
    if (interp_->faultBetween(gridPos, locPos))
	continue;

    double d = (locPos.x-gridPos.x)*(locPos.x-gridPos.x)+(locPos.y-gridPos.y)*(locPos.y-gridPos.y);
    double wgt = 1.0 / d;
    val += vals_[i] * wgt;
    wgtsum += wgt;
}
if (!mIsZero(wgtsum, mDefEpsD)) {
    val /= wgtsum;
    Threads::Locker lckr( lock_ );
    grid_->set(ix, iy, (float)val);
}    
, )

mDefParallelCalc2Pars( IDWLocalInterpolator, od_static_tr("IDWLocalInterpolator","IDW local interpolation"),
		       const wmIDWGridder2D*, interp, Threads::Lock, lock )
mDefParallelCalcBody( 
const std::vector<TPoint>& locs_ = interp_->binLocs_;
const std::vector<float>& vals_ = interp_->vals_;
const TrcKeySampling& hs_ = interp_->hs_;
Array2DImpl<float>* grid_ = interp_->grid_;
KDTree  kdtree_ = interp_->kdtree_;
TypeSet<od_int64> interpidx_ = interp_->interpidx_;
float xhalfblksize_ = interp_->blocksize_/SI().inlDistance()/2.0;
float yhalfblksize_ = interp_->blocksize_/SI().crlDistance()/2.0;
,
BinID gridBid = hs_.atIndex(interpidx_[idx]);
int ix = hs_.inlIdx(gridBid.inl());
int iy = hs_.crlIdx(gridBid.crl());
double x = gridBid.inl();
double y = gridBid.crl();
Coord gridPos(x, y);
TIds result;
double left = gridPos.x-xhalfblksize_;
double bottom = gridPos.y-yhalfblksize_;
double right = gridPos.x+xhalfblksize_;
double top = gridPos.y+yhalfblksize_;
kdtree_.range( left, bottom, right, top, [&result](const std::size_t id) { result.push_back(id); });
if (result.size() == 0)
    continue;
double val = 0.0;
double wgtsum = 0.0;
for (int i=0; i<result.size(); i++) {
    Coord locPos(locs_[result[i]].first, locs_[result[i]].second);
    if (interp_->faultBetween(gridPos, locPos))
	continue;

    double d = (locPos.x-gridPos.x)*(locPos.x-gridPos.x)+(locPos.y-gridPos.y)*(locPos.y-gridPos.y);
    double wgt = 1.0 / d;
    double vals = vals_[result[i]];
    val += vals * wgt;
    wgtsum += wgt;
}
if (!mIsZero(wgtsum, mDefEpsD)) {
    val /= wgtsum;
    Threads::Locker lckr( lock_ );
    grid_->set(ix, iy, (float)val);
}
, )



wmIDWGridder2D::wmIDWGridder2D()
{}

bool wmIDWGridder2D::executeGridding(TaskRunner* tr)
{
    localInterp();
    if ( !mIsUdf(blocksize_) ) {
	kdtree_.fill(binLocs_);
	Threads::Lock lock;
	IDWLocalInterpolator interp( interpidx_.size(), this, lock );
	if (tr)
	    return TaskRunner::execute( tr, interp );
	else
	    interp.execute();
    } else {
	Threads::Lock lock;
	IDWGlobalInterpolator interp( interpidx_.size(), this, lock );
	if (tr)
	    return TaskRunner::execute( tr, interp );
	else
	    interp.execute();
    }
    return true;
}

