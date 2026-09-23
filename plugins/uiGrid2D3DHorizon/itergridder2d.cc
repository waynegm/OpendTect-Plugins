/*
 *   Iterative Second Derivative gridder class
 *   Copyright (C) 2026 Wayne Mogg
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

#include "itergridder2d.h"

#include "survinfo.h"
#include "uiparent.h"
#include "uimsg.h"
#include "uitaskrunner.h"
#include "threadlock.h"

#include <cerrno>
#include <cmath>
#include <algorithm>
#include <limits>
#include <memory>
#include <vector>

/*
 * Fast interpolation method for surfaces with faults by multi-scale
 * second-derivative optimization (Leger & Clochard, OGST 75, 62, 2020).
 *
 * The cost function is the L2 norm of the surface second derivative,
 * computed by finite differences on the grid (paper eq. 6-9):
 *
 *   2Q = QII + 2*QIJ + QJJ
 *   QII = sum Dii^2, QJJ = sum Djj^2, QIJ = sum Dij^2
 *
 * with the stencils
 *
 *   Dii = f(i+1,j) - 2 f(i,j) + f(i-1,j)     (eq. 1)
 *   Djj = f(i,j+1) - 2 f(i,j) + f(i,j-1)     (eq. 2)
 *   Dij = f(i,j) + f(i+1,j+1) - f(i+1,j) - f(i,j+1)   (eq. 5)
 *
 * Weights: wII = 1/hI^4, wJJ = 1/hJ^4, wIJ = 1/(hI^2*hJ^2).
 * The paper sums raw differences (unit grid); the weights convert them to
 * physical second derivatives so anisotropic survey grids are handled
 * (paper section 2.1: trace distances manage X/Y anisotropy).  The
 * minimized cost is therefore
 *
 *   Q = sum wII*Dii^2 + 2*sum wIJ*Dij^2 + sum wJJ*Djj^2,
 *
 * up to an irrelevant global scale.  Its gradient carries 2.0*wII_,
 * 2.0*wJJ_ and 4.0*wIJ_ coefficients in IterTerms::doWork.
 *
 * summed over triplets/quadruplets containing at least one unknown.  Any
 * term whose cells straddle a fault trace is dropped, so both sides of the
 * fault become disconnected.  Minimization by conjugate gradient (the
 * gradient is computed from the stencils directly, see IterTerms); the heavy
 * passes all run over the OD thread pool (ParallelTask).
 *
 * Multi-scale speedup (paper section 2.5): solve on the coarsest level
 * first (each coarser level doubles the stride and the iteration count),
 * bilinearly initialize each finer level from the coarser solution.  The
 * number of levels is NO = 1 + ceil(log2(NU/NI)/3) where NU is the number of
 * unknowns and NI the number of iterations on the final model.
 */

/*
 * Parallel execution.  The stencil pass (which dominates the run time) is
 * split over row bands: each band accumulates into its own window of the
 * result (the band's unknowns expanded by one stencil halo), so each thread
 * writes private memory and the scratch space stays O(nu) overall.  The
 * windows are merged right after, in the same pass that computes the
 * conjugate-gradient dot products.  The per-iteration vector updates and the
 * relaxation sweeps run in parallel too.
 */
class wmIterativeGridder2D::IterTerms : public ParallelTask
{
    mODTextTranslationClass(wmIterativeGridder2D::IterTerms);
public:
    struct Band
    {
	bool		inited = false;
	int		w0 = 0;
	int		w1 = -1;
	std::vector<double> buf;
    };

    IterTerms( const wmIterativeGridder2D& it, int stride,
	       const Array2DImpl<int>& umap, const std::vector<double>& pv,
	       std::vector<std::unique_ptr<Band>>& bands )
	: iter_(&it)
	, stride_(stride)
	, umap_(&umap)
	, pv_(&pv)
	, bands_(&bands)
	, nrinl_( it.hs_.nrInl() )
	, nrcrl_( it.hs_.nrCrl() )
	, cols_( (nrinl_+stride-1)/stride )
	, rows_( (nrcrl_+stride-1)/stride )
    {
	const double hI = SI().inlDistance();
	const double hJ = SI().crlDistance();
	wII_ = 1.0/(hI*hI*hI*hI);
	wJJ_ = 1.0/(hJ*hJ*hJ*hJ);
	wIJ_ = 1.0/((hI*hI)*(hJ*hJ));
    }

    od_int64	nrIterations() const override	{ return (od_int64)cols_*rows_; }
    uiString	uiMessage() const override	{ return tr("Setting up stencils"); }
    uiString	uiNrDoneText() const override	{ return sPosFinished(); }

protected:

    bool doPrepare( int nrthreads ) override
    {
	nextslot_ = 0;
	const int nb = (int)bands_->size();
	if ( nb < nrthreads )
	    bands_->resize( nrthreads );
	return true;
    }

    bool doWork( od_int64 start, od_int64 stop, int ) override
    {
	// A worker may be handed several ranges during one execute(), so each
	// range gets its own band slot.  Bands sit in unique_ptrs so growing
	// the index vector never relocates the buffers other threads use.
	Band* band = nullptr;
	{   Threads::Locker lckr( lock_ );
	    if ( (int)bands_->size() <= nextslot_ )
	    {
		const int os = (int)bands_->size();
		bands_->resize( nextslot_+8 );
		for ( int idx=os; idx<(int)bands_->size(); idx++ )
		    (*bands_)[idx] = std::make_unique<Band>();
	    }
	    if ( !(*bands_)[nextslot_] )
		(*bands_)[nextslot_] = std::make_unique<Band>();
	    band = (*bands_)[nextslot_].get();
	    nextslot_++;
	}

	// Window: the unknowns of this range plus the stencil halo.
	const int ext = cols_+1;
	const od_int64 nitems = (od_int64)cols_*rows_;
	const od_int64 s0 = start-ext < 0 ? 0 : start-ext;
	const od_int64 s1 = stop+ext >= nitems ? nitems-1 : stop+ext;
	int w0 = std::numeric_limits<int>::max();
	int w1 = -1;
	for ( od_int64 id=s0; id<=s1; id++ )
	{
	    const int u = umap_->get( (int)(id%cols_)*stride_,
				      (int)(id/cols_)*stride_ );
	    if ( u>=0 ) { if ( u<w0 ) w0=u; if ( u>w1 ) w1=u; }
	}
	if ( w1>=0 && band->inited && band->w0==w0 && band->w1==w1 )
	    std::fill( band->buf.begin(), band->buf.end(), 0.0 );
	else
	{
	    band->inited = true;
	    band->w0 = w0<0 ? 0 : w0;
	    band->w1 = w1;
	    band->buf.assign( w1>=0 ? (size_t)(w1-w0+1) : 0, 0.0 );
	}
	if ( w1<0 )
	    return true;

	const wmIterativeGridder2D& iter = *iter_;
	const Array2DImpl<int>& umap = *umap_;
	const std::vector<double>& pv = *pv_;
	const int nrinl = nrinl_;
	const int stride = stride_;
	// Fault polygons live in BinID (survey) coordinates, while ix/iy are
	// zero-based grid indices. Convert before testing fault crossings.
	const int inl0 = iter.hs_.start_.inl();
	const int istp = iter.hs_.step_.inl();
	const int crl0 = iter.hs_.start_.crl();
	const int cstp = iter.hs_.step_.crl();
	const auto gc = [&]( int gx, int gy ) -> Coord
	{ return Coord( (double)(inl0+istp*gx), (double)(crl0+cstp*gy) ); };
	const int w0v = band->w0;
	const auto add = [&band,&w0v]( int u, double k )
	{ if ( u>=w0v && u<=band->w1 ) band->buf[u-w0v] += k; };

	for ( od_int64 id=start; id<=stop; id++ )
	{
	    const int ix = (int)(id%cols_)*stride;
	    const int iy = (int)(id/cols_)*stride;
	    const int row = iy*nrinl;
	    if ( !iter.valid(ix,iy) )
		continue;

	    if ( iter.valid(ix-stride,iy) && iter.valid(ix+stride,iy)
	      && !iter.faultBetween(gc(ix-stride,iy),gc(ix,iy))
	      && !iter.faultBetween(gc(ix,iy),gc(ix+stride,iy)) )
	    {
		const int uW = umap.get(ix-stride,iy);
		const int uC = umap.get(ix,iy);
		const int uE = umap.get(ix+stride,iy);
		if ( uW>=0 || uC>=0 || uE>=0 ) {
		    const double D = pv[row+ix+stride] - 2.0*pv[row+ix] + pv[row+ix-stride];
		    const double k = 2.0*wII_*D;
		    if ( uW>=0 ) add(uW,k);
		    if ( uC>=0 ) add(uC,-2.0*k);
		    if ( uE>=0 ) add(uE,k);
		}
	    }

	    if ( iter.valid(ix,iy-stride) && iter.valid(ix,iy+stride)
	      && !iter.faultBetween(gc(ix,iy-stride),gc(ix,iy))
	      && !iter.faultBetween(gc(ix,iy),gc(ix,iy+stride)) )
	    {
		const int uS = umap.get(ix,iy-stride);
		const int uC = umap.get(ix,iy);
		const int uN = umap.get(ix,iy+stride);
		if ( uS>=0 || uC>=0 || uN>=0 ) {
		    const double D = pv[(iy+stride)*nrinl+ix] - 2.0*pv[row+ix] + pv[(iy-stride)*nrinl+ix];
		    const double k = 2.0*wJJ_*D;
		    if ( uS>=0 ) add(uS,k);
		    if ( uC>=0 ) add(uC,-2.0*k);
		    if ( uN>=0 ) add(uN,k);
		}
	    }

	    if ( iter.valid(ix+stride,iy) && iter.valid(ix,iy+stride) && iter.valid(ix+stride,iy+stride)
	      && !iter.faultBetween(gc(ix,iy),gc(ix+stride,iy))
	      && !iter.faultBetween(gc(ix,iy),gc(ix,iy+stride))
	      && !iter.faultBetween(gc(ix+stride,iy),gc(ix+stride,iy+stride))
	      && !iter.faultBetween(gc(ix,iy+stride),gc(ix+stride,iy+stride)) )
	    {
		const int u00 = umap.get(ix,iy);
		const int u10 = umap.get(ix+stride,iy);
		const int u01 = umap.get(ix,iy+stride);
		const int u11 = umap.get(ix+stride,iy+stride);
		if ( u00>=0 || u10>=0 || u01>=0 || u11>=0 ) {
		    const double D = pv[row+ix] + pv[(iy+stride)*nrinl+ix+stride]
				   - pv[row+ix+stride] - pv[(iy+stride)*nrinl+ix];
		    const double k = 4.0*wIJ_*D;
		    if ( u00>=0 ) add(u00,k);
		    if ( u11>=0 ) add(u11,k);
		    if ( u10>=0 ) add(u10,-k);
		    if ( u01>=0 ) add(u01,-k);
		}
	    }
	}
	return true;
    }

private:
    const wmIterativeGridder2D*		iter_;
    const Array2DImpl<int>*		umap_;
    const std::vector<double>*		pv_;
    std::vector<std::unique_ptr<Band>>*	bands_;
    int					stride_;
    int					nrinl_;
    int					nrcrl_;
    int					cols_;
    int					rows_;
    double				wII_;
    double				wJJ_;
    double				wIJ_;
    int					nextslot_ = 0;
    Threads::Lock			lock_;
};


class wmIterativeGridder2D::IterMerge : public ParallelTask
{
    mODTextTranslationClass(wmIterativeGridder2D::IterMerge);
public:
    /* out receives the summed band contributions for every unknown; if pvec
       is given, the conjugate-gradient dot product p.(A*p) is accumulated
       along the way and exposed through pap(). */
    IterMerge( std::vector<double>& out, const std::vector<double>* pvec,
	       const std::vector<std::unique_ptr<IterTerms::Band>>& bands,
	       od_int64 nu )
	: out_(&out)
	, pvec_(pvec)
	, bands_(&bands)
	, nu_(nu)
    {}

    od_int64	nrIterations() const override	{ return nu_; }
    uiString	uiMessage() const override	{ return tr("Merging stencil results"); }
    uiString	uiNrDoneText() const override	{ return sPosFinished(); }
    double	pap() const			{ return pap_; }

protected:

    bool doPrepare( int nrthreads ) override
    {
	partials_.assign( nrthreads, 0.0 );
	w0s_.clear();
	sbands_.clear();
	for ( const auto& bp : *bands_ )
	{
	    if ( !bp || bp->w1<bp->w0 )
		continue;
	    w0s_.push_back( bp->w0 );
	    sbands_.push_back( bp.get() );
	}
	// Band windows are ordered, so sorting by start gives the merge order.
	std::vector<int> order(w0s_.size());
	for ( size_t idx=0; idx<order.size(); idx++ ) order[idx]=(int)idx;
	std::sort( order.begin(), order.end(),
		   [this]( int a, int b ) { return w0s_[a]<w0s_[b]; } );
	std::vector<int> nw0(w0s_.size());
	std::vector<const IterTerms::Band*> nsb(sbands_.size());
	for ( size_t idx=0; idx<order.size(); idx++ )
	{ nw0[idx]=w0s_[order[idx]]; nsb[idx]=sbands_[order[idx]]; }
	w0s_.swap(nw0); sbands_.swap(nsb);
	return true;
    }

    bool doWork( od_int64 start, od_int64 stop, int tid ) override
    {
	// Bands sorted by increasing w0; the predecessor of u is the last
	// band with w0<=u, and all later bands lie to the right.
	double pap = 0.0;
	std::vector<double>& out = *out_;
	for ( od_int64 u=start; u<=stop; u++ )
	{
	    const int uid = (int)u;
	    int t = (int)( std::upper_bound(w0s_.begin(), w0s_.end(), uid)
								- w0s_.begin() ) - 1;
	    double acc = 0.0;
	    for ( ; t>=0; t-- )
	    {
		const IterTerms::Band& b = *sbands_[t];
		if ( b.w1 < uid )
		    break;
		acc += b.buf[uid-b.w0];
	    }
	    out[uid] = acc;
	    if ( pvec_ )
		pap += (*pvec_)[uid]*acc;
	}
	partials_[tid] += pap;
	return true;
    }

    bool doFinish( bool success ) override
    {
	double pap = 0.0;
	for ( double v : partials_ )
	    pap += v;
	pap_ = pap;
	return success;
    }

private:
    std::vector<double>*				out_;
    const std::vector<double>*				pvec_;
    const std::vector<std::unique_ptr<IterTerms::Band>>*	bands_;
    od_int64						nu_;
    std::vector<double>					partials_;
    std::vector<int>					w0s_;
    std::vector<const IterTerms::Band*>			sbands_;
    double						pap_ = 0.0;
};


class wmIterativeGridder2D::IterUpdate : public ParallelTask
{
    mODTextTranslationClass(wmIterativeGridder2D::IterUpdate);
public:
    IterUpdate( wmIterativeGridder2D& it, const std::vector<int>& xs,
		const std::vector<int>& ys, const std::vector<double>& p,
		const std::vector<double>& ap, std::vector<double>& r, od_int64 nu )
	: iter_(&it)
	, xs_(&xs)
	, ys_(&ys)
	, p_(&p)
	, ap_(&ap)
	, r_(&r)
	, nu_(nu)
    {}

    od_int64	nrIterations() const override	{ return nu_; }
    uiString	uiMessage() const override	{ return tr("Updating the model"); }
    uiString	uiNrDoneText() const override	{ return sPosFinished(); }
    void	setAlpha( double a )		{ alpha_ = a; }
    double	rrnew() const			{ return rrnew_; }
    double	rdotap() const			{ return rdotap_; }
    double	maxdx() const			{ return maxdx_; }

protected:

    bool doPrepare( int nrthreads ) override
    {
	rrnewp_.assign( nrthreads, 0.0 );
	rdotapp_.assign( nrthreads, 0.0 );
	maxdxp_.assign( nrthreads, 0.0 );
	return true;
    }

    bool doWork( od_int64 start, od_int64 stop, int tid ) override
    {
	Array2DImpl<float>& grid = *iter_->grid_;
	const std::vector<int>& xs = *xs_;
	const std::vector<int>& ys = *ys_;
	const std::vector<double>& p = *p_;
	const std::vector<double>& ap = *ap_;
	std::vector<double>& r = *r_;
	const double alpha = alpha_;
	double rrnew = 0.0;
	double rdotap = 0.0;
	double maxdx = 0.0;
	for ( od_int64 u=start; u<=stop; u++ )
	{
	    const int i = (int)u;
	    const int x = xs[i];
	    const int y = ys[i];
	    const double apv = ap[i];
	    const double ri = r[i] - alpha*apv;
	    r[i] = ri;
	    rrnew += ri*ri;
	    rdotap += ri*apv;
	    const double dx = alpha*p[i];
	    const double adx = dx<0.0 ? -dx : dx;
	    if ( adx>maxdx )
		maxdx = adx;
	    grid.set( x, y, (float)(grid.get(x,y) + dx) );
	}
	rrnewp_[tid] += rrnew;
	rdotapp_[tid] += rdotap;
	// A worker may handle several ranges per execute(): keep the max.
	if ( maxdx>maxdxp_[tid] )
	    maxdxp_[tid] = maxdx;
	return true;
    }

    bool doFinish( bool success ) override
    {
	double rn = 0.0;
	double rd = 0.0;
	double mx = 0.0;
	for ( size_t idx=0; idx<rrnewp_.size(); idx++ )
	{ rn += rrnewp_[idx]; rd += rdotapp_[idx]; }
	for ( size_t idx=0; idx<maxdxp_.size(); idx++ )
	{ if ( maxdxp_[idx]>mx ) mx = maxdxp_[idx]; }
	rrnew_ = rn;
	rdotap_ = rd;
	maxdx_ = mx;
	return success;
    }

private:
    wmIterativeGridder2D*		iter_;
    const std::vector<int>*		xs_;
    const std::vector<int>*		ys_;
    const std::vector<double>*		p_;
    const std::vector<double>*		ap_;
    std::vector<double>*		r_;
    od_int64				nu_;
    std::vector<double>			rrnewp_;
    std::vector<double>			rdotapp_;
    std::vector<double>			maxdxp_;
    double				alpha_ = 0.0;
    double				rrnew_ = 0.0;
    double				rdotap_ = 0.0;
    double				maxdx_ = 0.0;
};


class wmIterativeGridder2D::IterPSet : public ParallelTask
{
    mODTextTranslationClass(wmIterativeGridder2D::IterPSet);
public:
    IterPSet( int nrinl, const std::vector<int>& xs, const std::vector<int>& ys,
	      const std::vector<double>& r, std::vector<double>& p,
	      std::vector<double>& pv, od_int64 nu )
	: nrinl_(nrinl)
	, xs_(&xs)
	, ys_(&ys)
	, r_(&r)
	, p_(&p)
	, pv_(&pv)
	, nu_(nu)
    {}

    od_int64	nrIterations() const override	{ return nu_; }
    uiString	uiMessage() const override	{ return tr("Setting conjugate direction"); }
    uiString	uiNrDoneText() const override	{ return sPosFinished(); }
    void	setBeta( double b )		{ beta_ = b; }

protected:

    bool doPrepare( int ) override		{ return true; }

    bool doWork( od_int64 start, od_int64 stop, int ) override
    {
	const std::vector<int>& xs = *xs_;
	const std::vector<int>& ys = *ys_;
	const std::vector<double>& r = *r_;
	std::vector<double>& p = *p_;
	std::vector<double>& pv = *pv_;
	const double beta = beta_;
	for ( od_int64 u=start; u<=stop; u++ )
	{
	    const int i = (int)u;
	    const double np = r[i] + beta*p[i];
	    p[i] = np;
	    pv[(size_t)ys[i]*nrinl_ + xs[i]] = np;
	}
	return true;
    }

private:
    int					nrinl_;
    const std::vector<int>*			xs_;
    const std::vector<int>*			ys_;
    const std::vector<double>*			r_;
    std::vector<double>*			p_;
    std::vector<double>*			pv_;
    od_int64					nu_;
    double					beta_ = 0.0;
};


class wmIterativeGridder2D::IterSweep : public ParallelTask
{
    mODTextTranslationClass(wmIterativeGridder2D::IterSweep);
public:
    enum Kind { Copy, Reassert, Jacobi, Bilinear };

    IterSweep( const wmIterativeGridder2D& it, int stride, Kind kind,
	       std::vector<float>& snap, int nrinl, int nrcrl )
	: iter_(&it)
	, stride_(stride)
	, kind_(kind)
	, snap_(&snap)
	, nrinl_(nrinl)
	, nrcrl_(nrcrl)
    {}

    od_int64	nrIterations() const override	{ return (od_int64)nrinl_*nrcrl_; }
    uiString	uiMessage() const override	{ return tr("Relaxing the surface"); }
    uiString	uiNrDoneText() const override	{ return sPosFinished(); }

protected:

    bool doPrepare( int ) override		{ return true; }

    bool doWork( od_int64 start, od_int64 stop, int ) override
    {
	Array2DImpl<float>& grid = *iter_->grid_;
	const Array2DImpl<float>& fixedval = *iter_->fixedval_;
	const Array2DImpl<unsigned char>& fixed = *iter_->fixed_;
	std::vector<float>& snap = *snap_;
	const int nrinl = nrinl_;
	const int nrcrl = nrcrl_;
	const int stride = stride_;
	if ( kind_==Copy )
	{
	    for ( od_int64 id=start; id<=stop; id++ )
		snap[id] = grid.get( (int)(id%nrinl), (int)(id/nrinl) );
	    return true;
	}
	for ( od_int64 id=start; id<=stop; id++ )
	{
	    const int ix = (int)(id%nrinl);
	    const int iy = (int)(id/nrinl);
	    switch ( kind_ )
	    {
	    case Reassert:
		if ( fixed.get(ix,iy) )
		    grid.set(ix,iy, fixedval.get(ix,iy));
		break;
	    case Jacobi:
	    case Bilinear:
		if ( ix%stride || iy%stride || mIsUdf(snap[id])
		  || fixed.get(ix,iy) )
		    break;
		if ( kind_==Jacobi )
		{
		    // Fault polygons use BinID coordinates, not grid indices.
		    const int inl0 = iter_->hs_.start_.inl();
		    const int istp = iter_->hs_.step_.inl();
		    const int crl0 = iter_->hs_.start_.crl();
		    const int cstp = iter_->hs_.step_.crl();
		    const Coord cc((double)(inl0+istp*ix), (double)(crl0+cstp*iy));
		    double sum = 0.0;
		    int n = 0;
		    for ( int dd=0; dd<4; dd++ )
		    {
			int nx=ix; int ny=iy;
			if ( dd==0 ) nx-=stride;
			else if ( dd==1 ) nx+=stride;
			else if ( dd==2 ) ny-=stride;
			else ny+=stride;
			if ( nx<0 || nx>=nrinl || ny<0 || ny>=nrcrl
			  || mIsUdf(snap[(size_t)ny*nrinl+nx]) )
			    continue;
			if ( iter_->faultBetween(cc, Coord((double)(inl0+istp*nx), (double)(crl0+cstp*ny))) )
			    continue;
			sum += snap[(size_t)ny*nrinl+nx];
			n++;
		    }
		    if ( n>0 )
			grid.set(ix,iy, (float)(sum/n));
		}
		else
		{
		    // Reads the coarse lattice from the snapshot, so this is
		    // deterministic and independent of the update order.
		    const int cs = 2*stride;
		    const int bx = (ix/cs)*cs;
		    const int by = (iy/cs)*cs;
		    const int bx2 = bx+cs<nrinl ? bx+cs : bx;
		    const int by2 = by+cs<nrcrl ? by+cs : by;
		    const double fx = double(ix-bx)/cs;
		    const double fy = double(iy-by)/cs;
		    const double wx[2] = { 1.0-fx, fx };
		    const double wy[2] = { 1.0-fy, fy };
		    const int cx[2] = { bx, bx2 };
		    const int cy[2] = { by, by2 };
		    double val = 0.0;
		    double wtot = 0.0;
		    for ( int di=0; di<2; di++ )
		    {
			for ( int dj=0; dj<2; dj++ )
			{
			    const int ccx = cx[di];
			    const int ccy = cy[dj];
			    if ( ccx<0 || ccx>=nrinl || ccy<0 || ccy>=nrcrl
			      || mIsUdf(snap[(size_t)ccy*nrinl+ccx]) )
				continue;
			    const double w = wx[di]*wy[dj];
			    val += w*snap[(size_t)ccy*nrinl+ccx];
			    wtot += w;
			}
		    }
		    if ( wtot>0.0 )
			grid.set(ix,iy, (float)(val/wtot));
		}
		break;
	    default:
		break;
	    }
	}
	return true;
    }

private:
    const wmIterativeGridder2D*		iter_;
    int					stride_;
    Kind				kind_;
    std::vector<float>*			snap_;
    int					nrinl_;
    int					nrcrl_;
};

class wmIterativeGridder2D::RunMultiScale : public SequentialTask
{
public:
    RunMultiScale( wmIterativeGridder2D& gridder, int niter, int nlevels )
    : SequentialTask("Multiscale Gridding")
    , gridder_(&gridder)
    , niter_(niter)
    , nlevels_(nlevels)
    , lvl_(nlevels-1)
    {}

private:
    wmIterativeGridder2D*  gridder_;
    int niter_;
    int nlevels_;
    int lvl_;
    int done_	=	0;

    od_int64 nrDone() const override
    {
	return done_;
    }

    od_int64 totalNr() const override
    {
	return nlevels_;
    }

    int nextStep() override
    {
	const int stride = 1<<lvl_;
	gridder_->reassertData();
	if ( lvl_<nlevels_-1 )
	    gridder_->bilinearInit(stride);
	gridder_->preConj( stride, 10 );
	gridder_->solveLevel( stride, niter_<<lvl_ );
	lvl_--;
	done_++;
	return lvl_<0 ? Finished() : MoreToDo();
    }
};

wmIterativeGridder2D::wmIterativeGridder2D()
    : niter_(100)
{
}

wmIterativeGridder2D::~wmIterativeGridder2D()
{
    deleteAndNullPtr(fixed_);
    deleteAndNullPtr(fixedval_);
    deleteAndNullPtr(cnt_);
}

const char* wmIterativeGridder2D::sKeyNIter()
{ return "NIterations"; }

const char* wmIterativeGridder2D::sKeyTol()
{ return "ConvergenceTolerance"; }

bool wmIterativeGridder2D::usePar(const IOPar& par)
{
    if (!wmGridder2D::usePar(par))
	return false;
    par.get(sKeyNIter(), niter_);
    par.get(sKeyTol(), dxtol_);
    return true;
}

bool wmIterativeGridder2D::valid( int ix, int iy ) const
{
    return ix>=0 && ix<hs_.nrInl() && iy>=0 && iy<hs_.nrCrl()
	&& !mIsUdf(grid_->get(ix,iy));
}

bool wmIterativeGridder2D::allocSolveArrays()
{
    deleteAndNullPtr(fixed_);
    deleteAndNullPtr(fixedval_);
    deleteAndNullPtr(cnt_);
    const int nrinl = hs_.nrInl();
    const int nrcrl = hs_.nrCrl();
    fixed_ = new Array2DImpl<unsigned char>(nrinl, nrcrl);
    fixedval_ = new Array2DImpl<float>(nrinl, nrcrl);
    cnt_ = new Array2DImpl<int>(nrinl, nrcrl);
    if ( !fixed_ || !fixedval_ || !cnt_
	 || !fixed_->isOK() || !fixedval_->isOK() || !cnt_->isOK() )
	return false;
    fixed_->setAll(0);
    fixedval_->setAll(0.0f);
    cnt_->setAll(0);
    return true;
}

od_int64 wmIterativeGridder2D::prepareSolve()
{
    const int nrinl = hs_.nrInl();
    const int nrcrl = hs_.nrCrl();
    for (int idx=0; idx<binLocs_.size(); idx++) {
	const Coord& pos = binLocs_[idx];
	const BinID bidSnap = hs_.getNearest( BinID(mNINT32(pos.x_), mNINT32(pos.y_)) );
	const int ix = hs_.inlIdx( bidSnap.inl() );
	const int iy = hs_.crlIdx( bidSnap.crl() );
	if ( ix<0 || ix>=nrinl || iy<0 || iy>=nrcrl )
	    continue;
	if ( mIsUdf( grid_->get(ix,iy) ) )
	    continue;

	const int c = cnt_->get(ix,iy);
	const float nval = c==0 ? vals_[idx] : ( grid_->get(ix,iy)*c + vals_[idx] )/(c+1);
	grid_->set(ix,iy,nval);
	fixed_->set(ix,iy,1);
	fixedval_->set(ix,iy,nval);
	cnt_->set(ix,iy,c+1);
    }

    od_int64 nu = 0;
    for (int iy=0; iy<nrcrl; iy++) {
	for (int ix=0; ix<nrinl; ix++) {
	    if ( !mIsUdf(grid_->get(ix,iy)) && fixed_->get(ix,iy)==0 )
		nu++;
	}
    }
    return nu;
}

int wmIterativeGridder2D::computeLevels( od_int64 nu ) const
{
    int nlevels = 1;
    if ( nu > niter_ ) {
	const double l2 = std::log2( double(nu)/double(niter_) ) / 3.0;
	nlevels = 1 + (int)std::ceil(l2);
    }
    const int minsz = std::min( hs_.nrInl(), hs_.nrCrl() );
    int maxpow = 0;
    while ( (1<<(maxpow+1)) <= minsz )
	maxpow++;
    if ( nlevels > maxpow+1 )
	nlevels = maxpow+1;
    return nlevels<1 ? 1 : nlevels;
}

bool wmIterativeGridder2D::executeGridding( uiParent* p )
{
    if ( !grid_ || hs_.nrInl()<=0 || hs_.nrCrl()<=0 )
	return false;

    if ( !allocSolveArrays() ) {
	reportMemError( "wmIterativeGridder2D::executeGridding",
			"cannot allocate memory for interpolation masks",
			(od_int64)(hs_.nrInl()*hs_.nrCrl())
			    *(sizeof(unsigned char)+sizeof(float)+sizeof(int)) );
	return false;
    }

    // Grid-align input data before fixing constraints.
    // localInterp spreads off-node input points to surrounding grid nodes using
    // distance-weighted interpolation (respecting faults), then rebuilds
    // binLocs_/vals_ from the result.  This ensures prepareSolve sees all input
    // data, not just the subset that happens to snap exactly to a grid node.
    if ( !localInterp(p, true) )
	return false;

    const od_int64 nu = prepareSolve();
    if ( nu==0 )
	return true;

    const int nlevels = computeLevels(nu);
    auto run_multiscale = RunMultiScale( *this, niter_, nlevels );
    uiTaskRunner uitr(p);

    if ( !uitr.execute( run_multiscale ) )
	return false;
    fixedmask_ = fixed_;
    const bool smoothok = smoothGrid(p);
    fixedmask_ = nullptr;
    return smoothok;
}

void wmIterativeGridder2D::reassertData()
{
    const int nrinl = hs_.nrInl();
    const int nrcrl = hs_.nrCrl();
    std::vector<float> snap( (size_t)nrinl*nrcrl );
    IterSweep sweep( *this, 1, IterSweep::Reassert, snap, nrinl, nrcrl );
    sweep.execute();
}

void wmIterativeGridder2D::bilinearInit( int stride )
{
    const int nrinl = hs_.nrInl();
    const int nrcrl = hs_.nrCrl();
    std::vector<float> snap( (size_t)nrinl*nrcrl, mUdf(float) );
    IterSweep copy( *this, stride, IterSweep::Copy, snap, nrinl, nrcrl );
    copy.execute();
    IterSweep fill( *this, stride, IterSweep::Bilinear, snap, nrinl, nrcrl );
    fill.execute();
}

void wmIterativeGridder2D::preConj( int stride, int nsweep )
{
    if ( nsweep<1 )
	return;
    const int nrinl = hs_.nrInl();
    const int nrcrl = hs_.nrCrl();
    std::vector<float> snap( (size_t)nrinl*nrcrl, mUdf(float) );
    for ( int sw=0; sw<nsweep; sw++ ) {
	IterSweep copy( *this, stride, IterSweep::Copy, snap, nrinl, nrcrl );
	copy.execute();
	IterSweep relax( *this, stride, IterSweep::Jacobi, snap, nrinl, nrcrl );
	relax.execute();
    }
}

/*
 * Solve one level.  Unknowns are the valid, un-fixed cells on the stride
 * lattice, numbered row-major (index = (iy/stride)*cols + ix/stride) in the
 * same order the stencil task iterates them.  pv is a full-grid copy of the
 * current direction (zero outside the unknowns); the stencil task writes
 * band-local fragments which the merge task sums into ap (and, in the same
 * pass, computes the p.(A*p) dot product used to size the step).
 */
void wmIterativeGridder2D::solveLevel( int stride, int niter )
{
    if ( niter<1 )
	return;
    const int nrinl = hs_.nrInl();
    const int nrcrl = hs_.nrCrl();
    const size_t total = (size_t)nrinl*nrcrl;

    Array2DImpl<int> umap(nrinl, nrcrl);
    if ( !umap.isOK() )
	return;
    umap.setAll(-1);

    // Downscale data constraints to this coarse level (paper section 2.5).
    // Fine-grid fixed nodes are snapped to the nearest coarse lattice point;
    // multiple fine nodes mapping to the same coarse node are averaged.
    // Build a temporary per-coarse-node average into lvl_fixedval/lvl_cnt,
    // then write the averaged values into grid_ BEFORE building umap/xs/ys
    // so those nodes are excluded from the unknown set just like fine fixed nodes.
    // The fine-resolution fixed_/fixedval_ arrays are left untouched for reassertData().
    Array2DImpl<float>   lvl_fixedval(nrinl, nrcrl);
    Array2DImpl<int>     lvl_cnt(nrinl, nrcrl);
    bool                 have_lvl_fixed = false;

    if ( stride > 1 ) {
	if ( !lvl_fixedval.isOK() || !lvl_cnt.isOK() )
	    return;
	lvl_fixedval.setAll(mUdf(float));
	lvl_cnt.setAll(0);

	for (int fy=0; fy<nrcrl; fy++) {
	    for (int fx=0; fx<nrinl; fx++) {
		if ( !fixed_->get(fx,fy) )
		    continue;
		const float fv = fixedval_->get(fx, fy);
		if ( mIsUdf(fv) )
		    continue;
		// Snap to nearest coarse lattice point, clamped to grid
		int cx = (int)std::round((double)fx/stride)*stride;
		int cy = (int)std::round((double)fy/stride)*stride;
		if ( cx >= nrinl ) cx -= stride;
		if ( cy >= nrcrl ) cy -= stride;
		if ( cx < 0 || cy < 0 )
		    continue;
		const int c = lvl_cnt.get(cx, cy);
		const float prev = mIsUdf(lvl_fixedval.get(cx,cy)) ? 0.0f : lvl_fixedval.get(cx,cy);
		lvl_fixedval.set(cx, cy, (prev*c + fv) / (c+1));
		lvl_cnt.set(cx, cy, c+1);
		have_lvl_fixed = true;
	    }
	}
	// Write averaged coarse fixed values into grid_ so umap building sees them
	if ( have_lvl_fixed ) {
	    for (int cy=0; cy<nrcrl; cy+=stride) {
		for (int cx=0; cx<nrinl; cx+=stride) {
		    if ( lvl_cnt.get(cx,cy) > 0 && !mIsUdf(grid_->get(cx,cy)) )
			grid_->set(cx, cy, lvl_fixedval.get(cx,cy));
		}
	    }
	}
    }

    std::vector<int> xs;
    std::vector<int> ys;
    xs.reserve(total/4);
    ys.reserve(total/4);
    int nu = 0;
    for (int iy=0; iy<nrcrl; iy+=stride) {
	for (int ix=0; ix<nrinl; ix+=stride) {
	    // Exclude both fine-resolution fixed nodes and coarse-level aggregated nodes
	    if ( !valid(ix,iy) )
		continue;
	    if ( fixed_->get(ix,iy) )
		continue;
	    if ( stride > 1 && have_lvl_fixed && lvl_cnt.get(ix,iy) > 0 )
		continue;
	    umap.set(ix,iy,nu);
	    xs.push_back(ix);
	    ys.push_back(iy);
	    nu++;
	}
    }
    if ( nu==0 )
	return;
    const od_int64 nunk = nu;

    std::vector<double> grad(nunk,0.0);
    std::vector<double> r(nunk,0.0);
    std::vector<double> p(nunk,0.0);
    std::vector<double> ap(nunk,0.0);
    std::vector<double> pv(total,0.0);

    for (int iy=0; iy<nrcrl; iy++) {
	const size_t row = (size_t)iy*nrinl;
	for (int ix=0; ix<nrinl; ix++) {
	    const float v = grid_->get(ix,iy);
	    pv[row+ix] = mIsUdf(v) ? 0.0 : v;
	}
    }

    std::vector<std::unique_ptr<IterTerms::Band>> bands;
    IterTerms termstask( *this, stride, umap, pv, bands );
    IterMerge gradmerge( grad, nullptr, bands, nunk );
    termstask.execute();
    gradmerge.execute();

    double rr = 0.0;
    for (int i=0; i<nunk; i++) {
	r[i] = -grad[i];
	rr += r[i]*r[i];
    }
    if ( rr<=1e-30 )
	return;
    const double rr0 = rr;
    for (int i=0; i<nunk; i++)
	p[i] = r[i];

    // From here on pv holds the direction: zero everywhere except p at the
    // unknowns (kept in sync by the pset pass).
    std::fill(pv.begin(), pv.end(), 0.0);

    IterMerge apmerge( ap, &p, bands, nunk );
    IterUpdate updatetask( *this, xs, ys, p, ap, r, nunk );
    IterPSet psettask( nrinl, xs, ys, r, p, pv, nunk );

    psettask.setBeta(0.0);
    psettask.execute();

    for (int it=0; it<niter; it++) {
	termstask.execute();
	apmerge.execute();

	double pap = apmerge.pap();
	if ( pap<=1e-30 ) {
	    psettask.setBeta(0.0);
	    psettask.execute();
	    termstask.execute();
	    apmerge.execute();
	    pap = apmerge.pap();
	    if ( pap<=1e-30 )
		break;
	}

	const double alpha = rr/pap;
	updatetask.setAlpha(alpha);
	updatetask.execute();
	if ( updatetask.rrnew() <= 1e-30*rr0 )
	    break;
	// No visible change anymore: further iterations only burn time.
	// dxtol_<=0 disables this test (exact convergence, e.g. self-test).
	if ( dxtol_>0.0 && updatetask.maxdx()<=dxtol_ )
	    break;
	double beta = -alpha*updatetask.rdotap()/rr;
	if ( beta<0.0 )
	    beta = 0.0;
	rr = updatetask.rrnew();
	psettask.setBeta(beta);
	psettask.execute();
    }
}

#ifdef WMITERSELFTEST

#include <cstdio>

bool wmIterativeGridder2D::selfTest()
{
    auto setup = [](wmIterativeGridder2D& mg, int n)
    {
	mg.hs_.start_ = BinID(0,0);
	mg.hs_.stop_ = BinID(n-1,n-1);
	mg.hs_.step_ = BinID(1,1);
	mg.grid_ = new Array2DImpl<float>( mg.hs_.nrInl(), mg.hs_.nrCrl() );
	if ( !mg.grid_ || !mg.grid_->isOK() )
	    return false;
	mg.grid_->setAll(0.0f);
	return true;
    };
    auto check = [](wmIterativeGridder2D& mg, int n, bool withfault)
    {
	for (int j=0; j<n; j++) {
	    for (int i=0; i<n; i++) {
		const float v = mg.grid_->get(i,j);
		if ( mIsUdf(v) )
		    return false;
		// The fault at x=4.5 severs the edge between cell columns 4
		// and 5, so column 4 belongs to the left half-plane.
		const double want = withfault ? ( i<5 ? j : 10.0+j ) : 2.0*i+3.0*j+5.0;
		const double tol = withfault ? 0.5 : 1e-3;
		if ( std::abs(v-want)>tol )
		    return false;
	    }
	}
	return true;
    };
    auto run = [](wmIterativeGridder2D& mg) -> bool
    {
	if ( !mg.allocSolveArrays() )
	    return false;
	const od_int64 nu = mg.prepareSolve();
	if ( nu<=0 )
	    return false;
	// niter must exceed the unknown count for CG to converge to the
	// exact minimizer (its convergence is N-step on this quadratic).
	mg.setNIter(200);
	mg.setTol(0.0);
	const int nlevels = mg.computeLevels(nu);
	RunMultiScale rms(mg, mg.niter_, nlevels);
	return rms.execute();
    };

    // exact-reproduction case: data sample a plane, whose second derivative
    // is identically zero, so the minimizer must return the plane exactly.
    wmIterativeGridder2D mg;
    if ( !setup(mg,9) || !run(mg) || !check(mg,9,false) )
    {
	ErrMsg("wmIterativeGridder2D::selfTest - plane interpolation failed");
	return false;
    }

    // fault case: two half-planes offset by 10, separated by a vertical fault
    // at x=4.5.  The fault must be honored (no smoothing across it).
    wmIterativeGridder2D mf;
    if ( !setup(mf,10) )
    {
	ErrMsg("wmIterativeGridder2D::selfTest - fault setup failed");
	return false;
    }
    ODPolygon<Pos::Ordinate_Type>* fault = new ODPolygon<Pos::Ordinate_Type>;
    fault->add( Coord(4.5,-1) );
    fault->add( Coord(4.5,10) );
    mf.faultpoly_ += fault;
    for (int j=0; j<10; j++) {
	mf.setPoint( Coord(2,j), (float)j );
	mf.setPoint( Coord(7,j), (float)(10+j) );
    }
    if ( !run(mf) || !check(mf,10,true) )
    {
	ErrMsg("wmIterativeGridder2D::selfTest - fault interpolation failed");
	return false;
    }
    return true;
}

#endif // WMITERSELFTEST
