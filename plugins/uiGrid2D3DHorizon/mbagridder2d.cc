#include "mbagridder2d.h"

wmMBAGridder2D::wmMBAGridder2D()
: maxlevels_(10)
{    
}

bool wmMBAGridder2D::prepareForGridding()
{
    if (!wmGridder2D::prepareForGridding())
	return false;
    
    return true;
}

bool wmMBAGridder2D::executeGridding(TaskRunner* tr)
{
    std::vector<mba::point<2>> binLocs(binLocs_.size());
    std::vector<float> vals(binLocs_.size());
    for (int idx=0; idx<binLocs_.size(); idx++) {
	binLocs[idx] = mba::point<2>{binLocs_[idx].x, binLocs_[idx].y};
	vals[idx] = vals_[idx];
    }

    mba::point<2> lo = { hs_.start_.inl(), hs_.start_.crl() };
    mba::point<2> hi = { hs_.stop_.inl(), hs_.stop_.crl() };
    
    mba::index<2> grid = { 2, 2 };
    
    mba::MBA<2> interp(lo, hi, grid, binLocs, vals, maxlevels_);
    
    for (od_int64 idx=0; idx<interpidx_.size(); idx++) {
	BinID gridBid = hs_.atIndex(interpidx_[idx]);
	int ix = hs_.inlIdx(gridBid.inl());
	int iy = hs_.crlIdx(gridBid.crl());
	grid_->set(ix, iy, interp(mba::point<2>{gridBid.inl(), gridBid.crl()}));
    }
    return true;
}

