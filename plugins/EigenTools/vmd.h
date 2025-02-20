#pragma once
/*
 *   EigenTools Plugin -  Variational Mode Decomposition
 *   Copyright (C) 2021  Wayne Mogg
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
#include "vmdattribmod.h"
#include "eigen3/Eigen/Core"
#include "eigen3/unsupported/Eigen/FFT"


template <typename X>
void mirrorPad(const Eigen::ArrayBase<X>& input, Eigen::ArrayBase<X>& output)
{
    const int ns = input.size();
    const int pivot = ns%2==0 ? ns/2 : (ns+1)/2;
    output << input.head(ns-pivot).reverse(), input, input.tail(pivot).reverse();
}

template <typename X>
void symmetricPad(const Eigen::ArrayBase<X>& input, Eigen::ArrayBase<X>& output)
{
    const int ns = input.size();
    const int pivot = ns%2==0 ? ns/2 : (ns+1)/2;
    X trinput = input.segment(1,ns-2);
    output << trinput.head(ns-pivot).reverse(), input, trinput.tail(pivot).reverse();
}

template <typename X>
void fftshift(const Eigen::ArrayBase<X>& input, Eigen::ArrayBase<X>& output)
{
    const int ns = input.size();
    const int pivot = ns%2==0 ? ns/2 : (ns+1)/2;
    output << input.tail(ns-pivot), input.head(pivot);
}

class VMD
{
public:
    enum	OmegaInit { AllZero, Uniform, Random };
    VMD(int nrmodes=5);
    ~VMD();

    void		setPars(float alpha, float tau, bool makeDC, OmegaInit omega, float tol);
    Eigen::ArrayXXd	computeModes(const Eigen::ArrayXd& input);

protected:
    float	alpha_;
    float	tau_;
    int		nrmodes_;
    bool	makedc_;
    OmegaInit	omega_;
    float	tol_;
    int		niter_;
    Eigen::FFT<double>	fft_;

};


