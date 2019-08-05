#ifndef rsflib_h
#define rsflib_h
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

#include "localattribmod.h" 



namespace sf {
class Triangle;
    
void adjnull(bool adj, bool add, int nx, int ny, float* x, float* y);

void doubint2(int nx, float *xx, bool der);

int first_index(int i, int j, int dim, const int* n, const int* s);

void fold2(int o, int d, int nx, int nb, int np, float *x, const float* tmp);

void smooth2(Triangle& tr, int o, int d, bool derivative, float* x);

void triple2(int o, int d, int nx, int nb, const float* x, float* tmp, bool box, float wt);

class Lop
{
public:
    virtual void doLop(bool, bool, int, int, float*, float*) = 0;
};

class Triangle
{
public:
    Triangle();
    Triangle(int nbox, int ndat, bool box);
    ~Triangle();
    
    void init(int nbox, int ndat, bool box);
    bool isNull();
    void setNull();

    float*  tmp_;
    float   wt_;
    int     np_;
    int     nb_;
    int     nx_;
    bool    box_;
};
        
class Trianglen : public Lop
{
public:
    Trianglen(int ndim, int* nbox, int* ndat);
    ~Trianglen();
    
    void doLop(bool adj, bool add, int nx, int ny, float* x, float* y);
    
protected:
    float*      tmp_;
    Triangle*   tr_;
    int         dim_;
    int         nd_;
    int*        n_;
    int*        s_;
};

class Weight : public Lop
{
public:
    Weight(const float* w1);
    ~Weight();
    
    void doLop(bool adj, bool add, int nx, int ny, float* xx, float* yy);
protected:
    const float* w_;
};

class ConjGrad
{
public:
    ConjGrad(int np1     /* preconditioned size */, 
             int nx1     /* model size */, 
             int nd1     /* data size */, 
             int nr1     /* residual size */, 
             float eps1  /* scaling */,
             float tol1  /* tolerance */, 
             bool hasp01 /* if has initial model */);
    ~ConjGrad();
    
    void doCG(Lop* prec  /* data preconditioning */, 
              Lop* oper  /* linear operator */, 
              Lop* shape /* shaping operator */, 
              float* p          /* preconditioned model */, 
              float* x          /* estimated model */, 
              const float* dat        /* data */, 
              int niter         /* number of iterations */);
protected:
    int np_;
    int nx_;
    int nr_;
    int nd_;
    float*  r_;
    float*  sp_;
    float*  sx_;
    float*  sr_;
    float*  gp_;
    float*  gx_;
    float*  gr_;
    float   eps_;
    float   tol_;
    bool    hasp0_;
};

class Divn
{
public:
        Divn(int ndim, int nd, int* ndat, int* nbox, int niter);
        ~Divn();
        
        void doDiv(const float* num, const float* den, float* rat);
            
protected:
    int         niter_;
    int         n_;
    float*      p_;
    Trianglen   trianglen_;
    ConjGrad    conjgrad_;
    
};

}; //namespace sf

#endif
