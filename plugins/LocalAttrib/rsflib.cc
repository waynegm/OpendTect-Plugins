/*
 *   LocalAttrib Plugin
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

#include <cstddef>
#include "rsflib.h"

#ifdef NO_BLAS
void cblas_saxpy(int n, float a, const float *x, int sx, float *y, int sy)
{
    int i, ix, iy;
    
    for (i=0; i < n; i++) {
        ix = i*sx;
        iy = i*sy;
        y[iy] += a * x[ix];
    }    
}

void cblas_sswap(int n, float *x, int sx, float* y, int sy)
{
    int i, ix, iy;
    float t;
    
    for (i=0; i < n; i++) {
        ix = i*sx;
        iy = i*sy;
        t = x[ix];
        x[ix] = y[iy];
        y[iy] = t;
    }  
}

double cblas_dsdot(int n, const float *x, int sx, const float *y, int sy)
{
    int i, ix, iy;
    double dot;
    
    dot = 0.;
    
    for (i=0; i < n; i++) {
        ix = i*sx;
        iy = i*sy;
        dot += (double) x[ix] * y[iy];
    }
    
    return dot;    
}
#else
#ifdef HAVE_MKL
#include <mkl.h>
#else
#include <cblas.h>
#endif
#endif


namespace sf {
    
void adjnull(bool adj, bool add, int nx, int ny, float* x, float* y)
{
    if (add)
        return;
    if (adj)
        for (int i=0; i<nx; i++)
            x[i] = 0.0;
    else
        for (int i=0; i<ny; i++)
            y[i] = 0.0;
}

void doubint2(int nx, float *xx, bool der)
{
    /* integrate forward */
    float t=0.;
    for (int i=0; i < nx; i++) {
        t += xx[i];
        xx[i] = t;
    }
    
    if (der) return;
    
    /* integrate backward */
    t = 0.;
    for (int i=nx-1; i >= 0; i--) {
        t += xx[i];
        xx[i] = t;
    }
}

int first_index(int i, int j, int dim, const int* n, const int* s)
{
    int n123 = 1;
    int i0 = 0;
    for (int k=0; k < dim; k++) {
        if (k == i) continue;
        int ii = (j/n123)%n[k]; /* to cartesian */
        n123 *= n[k];	
        i0 += ii*s[k];      /* back to line */
    }
    
    return i0;
}

void fold2 (int o, int d, int nx, int nb, int np, float *x, const float* tmp)
{
    /* copy middle */
    for (int i=0; i < nx; i++) 
        x[o+i*d] = tmp[i+nb];
    
    /* reflections from the right side */
    for (int j=nb+nx; j < np; j += nx) {
        for (int i=0; i < nx && i < np-j; i++)
            x[o+(nx-1-i)*d] += tmp[j+i];
        j += nx;
        for (int i=0; i < nx && i < np-j; i++)
            x[o+i*d] += tmp[j+i];
    }
    
    /* reflections from the left side */
    for (int j=nb; j >= 0; j -= nx) {
        for (int i=0; i < nx && i < j; i++)
            x[o+i*d] += tmp[j-1-i];
        j -= nx;
        for (int i=0; i < nx && i < j; i++)
            x[o+(nx-1-i)*d] += tmp[j-1-i];
    }    
}

void smooth2(Triangle& tr, int o, int d, bool der, float* x)
{
    triple2(o,d,tr.nx_,tr.nb_,x,tr.tmp_, tr.box_, tr.wt_);
    doubint2(tr.np_,tr.tmp_,(bool) (tr.box_ || der));
    fold2(o,d,tr.nx_,tr.nb_,tr.np_,x,tr.tmp_);
}

void triple2(int o, int d, int nx, int nb, const float* x, float* tmp, bool box, float wt)
{
    for (int i=0; i < nx + 2*nb; i++) {
        tmp[i] = 0;
    }
    
    if (box) {
        cblas_saxpy(nx,  +wt,x+o,d,tmp+1   ,1);
        cblas_saxpy(nx,  -wt,x+o,d,tmp+2*nb,1);
    } else {
        cblas_saxpy(nx,  -wt,x+o,d,tmp     ,1);
        cblas_saxpy(nx,2.*wt,x+o,d,tmp+nb  ,1);
        cblas_saxpy(nx,  -wt,x+o,d,tmp+2*nb,1);
    }
}

Triangle::Triangle()
    : tmp_(NULL)
{
}
Triangle::Triangle(int nbox, int ndat, bool box)
{
    init(nbox, ndat, box);
}

Triangle::~Triangle()
{
    delete[] tmp_;
}

void Triangle::init(int nbox, int ndat, bool box)
{
    nx_ = ndat;
    nb_ = nbox;
    box_ = box;
    np_ = ndat + 2*nbox;
    wt_ = box_ ? 1.0/(2*nbox-1) : 1.0/(nbox*nbox);
    tmp_ = new float[np_];
}

bool Triangle::isNull()
{ return tmp_ == NULL; }

void Triangle::setNull()
{
    delete[] tmp_;
    tmp_ = NULL;
}

Trianglen::Trianglen(int ndim, int* nbox, int* ndat)
    : dim_(ndim)
    , nd_(1)
    , n_(new int[ndim])
    , s_(new int[ndim])
    , tr_(new Triangle[ndim])
{
    for (int i=0; i<dim_; i++) {
        if (nbox[i] > 1)
            tr_[i].init(nbox[i], ndat[i], false);
        else
            tr_[i].setNull();
        s_[i] = nd_;
        n_[i] = ndat[i];
        nd_ *= ndat[i];
    }

    tmp_ = new float[nd_];
}

Trianglen::~Trianglen()
{
    delete[] tmp_;
    delete[] tr_;
    delete[] n_;
    delete[] s_;
}

void Trianglen::doLop(bool adj, bool add, int nx, int ny, float* x, float* y)
{
    if (nx != ny || nx != nd_) 
        return;
        
    adjnull (adj, add, nx, ny, x, y);
    
    for (int i=0; i<nd_; i++) 
        tmp_[i] = adj ? y[i] : x[i];
    
    for (int i=0; i<dim_; i++) {
        if (!tr_[i].isNull()) {
            for (int j=0; j<nd_/n_[i]; j++) {
                int i0 = first_index(i, j, dim_, n_, s_);
                smooth2(tr_[i], i0, s_[i], false, tmp_);
            }
        }
    }
    
    if (adj) {
        for (int i=0; i<nd_; i++)
            x[i] += tmp_[i];
    } else {
        for (int i=0; i<nd_; i++)
            y[i] += tmp_[i];
    }        
}

Weight::Weight(const float* w)
    : w_(w)
{
}

Weight::~Weight()
{}

void Weight::doLop(bool adj, bool add, int nx, int ny, float* xx, float* yy)
{
    if (ny!=nx) 
        return;
    
    adjnull (adj, add, nx, ny, xx, yy);
    
    if (adj) {
        for (int i=0; i < nx; i++) {
            xx[i] += yy[i] * w_[i];
        }
    } else {
        for (int i=0; i < nx; i++) {
            yy[i] += xx[i] * w_[i];
        }
    }
}

ConjGrad::ConjGrad(int np1, int nx1, int nd1, int nr1, float eps1, float tol1, bool hasp01)
    : np_(np1)
    , nx_(nx1)
    , nr_(nr1)
    , nd_(nd1)
    , eps_(eps1*eps1)
    , tol_(tol1)
    , hasp0_(hasp01)
{
    r_ = new float[nr_];
    sp_ = new float[np_];
    gp_ = new float[np_];
    sx_ = new float[nx_];
    gx_ = new float[nx_];
    sr_ = new float[nr_];
    gr_ = new float[nr_];
}

ConjGrad::~ConjGrad()
{
    delete[] r_;
    delete[] sp_;
    delete[] gp_;
    delete[] sx_;
    delete[] gx_;
    delete[] sr_;
    delete[] gr_;
}

void ConjGrad::doCG(Lop* prec, Lop* oper, Lop* shape, float* p, float* x, const float* dat, int niter) 
{
    float* d = NULL;
    
    if (NULL != prec) {
        d = new float[nd_]; 
        for (int i=0; i < nd_; i++) {
            d[i] = - dat[i];
        }
        prec->doLop(false, false, nd_, nr_, d, r_);
    } else {
        for (int i=0; i < nr_; i++) {
            r_[i] = - dat[i];
        }
    }
    
    if (hasp0_) { /* initial p */
        shape->doLop(false, false, np_, nx_, p, x);
        if (NULL != prec) {
            oper->doLop(false, false, nx_, nd_, x, d);
            prec->doLop(false, true, nd_, nr_, d, r_);
        } else {
            oper->doLop(false, true, nx_, nr_, x, r_);
        }
    } else {
        for (int i=0; i < np_; i++) {
            p[i] = 0.;
        }
        for (int i=0; i < nx_; i++) {
            x[i] = 0.;
        }
    } 
    
    double dg = 0.0;
    double g0 = 0.0;
    double gnp = 0.0;
    double alpha = 0.0;
    double r0 = cblas_dsdot(nr_, r_, 1, r_, 1);
    if (r0 == 0.)
        return;
    
    for (int iter=0; iter < niter; iter++) {
        for (int i=0; i < np_; i++) {
            gp_[i] = eps_*p[i];
        }
        for (int i=0; i < nx_; i++) {
            gx_[i] = -eps_*x[i];
        }
        
        if (NULL != prec) {
            prec->doLop(true, false, nd_, nr_, d, r_);
            oper->doLop(true, true, nx_, nd_, gx_, d);
        } else {
            oper->doLop(true, true, nx_, nr_, gx_, r_);
        }
        
        shape->doLop(true, true, np_, nx_, gp_, gx_);
        shape->doLop(false, false, np_, nx_, gp_, gx_);
        
        if (NULL != prec) {
            oper->doLop(false, false, nx_, nd_, gx_, d);
            prec->doLop(false, false, nd_, nr_, d, gr_);
        } else {
            oper->doLop(false, false, nx_, nr_, gx_, gr_);
        }
        
        double gn = cblas_dsdot(np_, gp_, 1, gp_, 1);
        
        if (iter==0) {
            g0 = gn;
            
            for (int i=0; i < np_; i++) {
                sp_[i] = gp_[i];
            }
            for (int i=0; i < nx_; i++) {
                sx_[i] = gx_[i];
            }
            for (int i=0; i < nr_; i++) {
                sr_[i] = gr_[i];
            }
        } else {
            alpha = gn / gnp;
            dg = gn / g0;
            
            if (alpha < tol_ || dg < tol_) {
                    break;
            }
            
            cblas_saxpy(np_, alpha, sp_, 1, gp_, 1);
            cblas_sswap(np_, sp_, 1, gp_, 1);
            
            cblas_saxpy(nx_, alpha, sx_, 1, gx_, 1);
            cblas_sswap(nx_, sx_, 1, gx_, 1);
            
            cblas_saxpy(nr_, alpha, sr_, 1, gr_, 1);
            cblas_sswap(nr_, sr_, 1, gr_, 1);
        }
        
        double beta = cblas_dsdot(nr_, sr_, 1, sr_, 1) + eps_*(cblas_dsdot(np_, sp_, 1, sp_, 1) - cblas_dsdot(nx_, sx_, 1, sx_, 1));
        
        alpha = - gn / beta;
        
        cblas_saxpy(np_, alpha, sp_, 1, p, 1);
        cblas_saxpy(nx_, alpha, sx_, 1, x, 1);
        cblas_saxpy(nr_, alpha, sr_, 1, r_, 1);
        
        gnp = gn;
    }
    
    if (NULL != prec) delete[] d;
}

Divn::Divn(int ndim, int nd, int* ndat, int* nbox, int niter)
    : niter_(niter)
    , n_(nd)
    , trianglen_(ndim, nbox, ndat)
    , conjgrad_(nd, nd, nd, nd, 1., 1.e-6, false)
    , p_(new float[nd])
{
}

Divn::~Divn()
{
    delete[] p_;
}

void Divn::doDiv(const float* num, const float* den, float* rat)
{
    Weight w(den);
    conjgrad_.doCG(NULL, &w, &trianglen_, p_, rat, num, niter_);
}
    
}; //namespace sf
