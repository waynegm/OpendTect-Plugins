#include "nrngridder2d.h"

wmNRNGridder2D::wmNRNGridder2D()
{    
}

bool wmNRNGridder2D::executeGridding(TaskRunner* tr)
{
    localInterp(false);

    for (int idx=0; idx<interpidx_.size(); idx++) {
	BinID gridBid = hs_.atIndex(interpidx_[idx]);
	int ix = hs_.inlIdx(gridBid.inl());
	int iy = hs_.crlIdx(gridBid.crl());
	float gridval = grid_->get(ix,iy);
	if (mIsUdf(gridval))
	    continue;

	grid_->set(ix, iy, (gridval==0.0 ? mUdf(float) : gridval));

    }
    return true;
}

