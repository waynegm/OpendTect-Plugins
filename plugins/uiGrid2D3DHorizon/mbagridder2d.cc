#include "mbagridder2d.h"

wmMBAGridder2D::wmMBAGridder2D()
: maxlevels_(5)
{    
}

void wmMBAGridder2D::setPoint( const Coord& binLoc, const float val )
{
    binLocs_.push_back(mba::point<2>{binLoc.x, binLoc.y});
    includeInRange(binLoc);
    vals_.push_back(val);
}

bool wmMBAGridder2D::prepareForGridding()
{
    if (!wmGridder2D::prepareForGridding())
	return false;
    
    return true;
}

bool wmMBAGridder2D::executeGridding(TaskRunner* tr)
{
    mba::point<2> lo = { hs_.start_.inl(), hs_.start_.crl() };
    mba::point<2> hi = { hs_.stop_.inl(), hs_.stop_.crl() };
    
    int xstart = mNINT32((hs_.nrInl()-1)/pow(2,maxlevels_-1)+1);
    int ystart = mNINT32((hs_.nrCrl()-1)/pow(2,maxlevels_-1)+1);
    mba::index<2> grid = { xstart, ystart };
    
    mba::MBA<2> interp(lo, hi, grid, binLocs_, vals_, maxlevels_);
    
    const Array2DInfo& arrinfo = grid_->info();
    int pos[2];
    for (od_int64 idx=0; idx<hs_.totalNr(); idx++) {
	BinID gridBid = hs_.atIndex(idx);
	int ix = hs_.inlIdx(gridBid.inl());
	int iy = hs_.crlIdx(gridBid.crl());
	double x = gridBid.inl();
	double y = gridBid.crl();
	Coord gridPos(x, y);
	if (isUncropped(gridPos) && !inFaultHeave(gridPos))
	    grid_->set(ix, iy, interp(mba::point<2>{x, y}));
	else
	    grid_->set(ix, iy, mUdf(float));
    }
    return true;
}

