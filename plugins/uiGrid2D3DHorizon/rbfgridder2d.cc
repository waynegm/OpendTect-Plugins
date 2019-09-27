#include "rbfgridder2d.h"
#include "Eigen/Core"
#include "mathtoolbox/rbf-interpolation.hpp"

#include "bufstring.h"

wmRBFGridder2D::wmRBFGridder2D()
{}

bool wmRBFGridder2D::executeGridding(TaskRunner* tr)
{
    localInterp();
/*
    const int nrpoint = binLocs_.size();
    BufferString tmp("In executeGridding: ");
    tmp += nrpoint;
    ErrMsg(tmp);
    
    Eigen::MatrixXd X(2, nrpoint);
    Eigen::VectorXd y(nrpoint);
    for (int i = 0; i<nrpoint; i++) {
	X.col(i) = Eigen::Vector2d(binLocs_[i].first, binLocs_[i].second);
	y(i) = vals_[i];
    }
    
    
    mathtoolbox::RbfInterpolation rbf_interp;
    rbf_interp.SetData(X, y);
    tmp = "After assign data";
    ErrMsg(tmp);
    rbf_interp.ComputeWeights(true);
    
    tmp = "After compute weights";
    ErrMsg(tmp);
    
    
    for (od_int64 idx=0; idx<hs_.totalNr(); idx++) {
	BinID gridBid = hs_.atIndex(idx);
	int ix = hs_.inlIdx(gridBid.inl());
	int iy = hs_.crlIdx(gridBid.crl());
	double xg = gridBid.inl();
	double yg = gridBid.crl();
	Coord gridPos(xg, yg);
	if (isUncropped(gridPos) && !inFaultHeave(gridPos))
	    grid_->set(ix, iy, rbf_interp.GetValue(Eigen::Vector2d(xg, yg)));
	else
	    grid_->set(ix, iy, mUdf(float));
    }
*/    
    BufferString tmp;
    tmp = "After setting grid";
    ErrMsg(tmp);
    return true;
}

bool wmRBFGridder2D::usePar(const IOPar& par)
{
    par.get(sKeyTension(), tension_);
    return wmGridder2D::usePar(par);
}
