#pragma once
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
#include "wmgridder2d.h"

#include <vector>

class uiParent;

/*
 * Multi-scale L2-minimization of the surface second derivative (Leger &
 * Clochard, OGST 75, 62, 2020). Data points are honored exactly, faults are
 * handled by removing any second-derivative term whose triplet/quadruplet
 * crosses a fault trace.
 */
class wmIterativeGridder2D : public wmGridder2D
{
public:
    wmIterativeGridder2D();
    ~wmIterativeGridder2D();

    bool		executeGridding(uiParent*);
    bool		usePar(const IOPar&);
    void		setNIter(int n) { niter_ = n; }
    void		setTol(double t) { dxtol_ = t; }

    static const char*	sKeyNIter();
    static const char*	sKeyTol();

#ifdef WMITERSELFTEST
    static bool		selfTest();
#endif

protected:
    void		solveLevel(int stride, int niter);
    void		preConj(int stride, int nsweep);
    void		bilinearInit(int stride);
    void		reassertData();
    bool		allocSolveArrays();
    od_int64		prepareSolve();
    int			computeLevels(od_int64 nu) const;
    bool		valid(int ix, int iy) const;

    int					niter_;
    double				dxtol_ = 1e-3;
    Array2DImpl<float>*			fixedval_ = nullptr;
    Array2DImpl<unsigned char>*		fixed_ = nullptr;
    Array2DImpl<int>*			cnt_ = nullptr;

    class IterTerms;
    class IterMerge;
    class IterUpdate;
    class IterPSet;
    class IterSweep;
    class RunMultiScale;
};
