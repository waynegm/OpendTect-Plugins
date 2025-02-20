/*
 *   EigenTools Plugin - Variational Mode Decomposition
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
#include "vmd.h"

#include "eigen3/Eigen/Core"


VMD::VMD(int nrmodes)
    : alpha_(2000.0)
    , tau_(0.0)
    , nrmodes_(nrmodes)
    , makedc_(false)
    , tol_(1e-6)
    , niter_(500)
{
}

VMD::~VMD()
{}

void VMD::setPars(float alpha, float tau, bool makeDC, OmegaInit omega, float tol, int niter)
{
    alpha_ = alpha;
    tau_ = tau;
    makedc_ = makeDC;
    omega_ = omega;
    tol_ = tol;
    niter_ = niter;
}

Eigen::ArrayXXd VMD::computeModes(const Eigen::ArrayXd& input)
{
    double fs = 1.0/input.size();
    Eigen::ArrayXd padded_input(input.size()*2);
    mirrorPad(input, padded_input);
    int T = padded_input.size();
    Eigen::ArrayXd t = Eigen::ArrayXd::LinSpaced(T, 1.0, T)/T;
    Eigen::ArrayXd freqs = t - 0.5 - (1/T);

    Eigen::ArrayXcd freqvec(T);
    fft_.fwd(freqvec, padded_input);
    Eigen::ArrayXcd f_hat(T);
    fftshift(freqvec, f_hat);
    Eigen::ArrayXcd f_hat_plus = f_hat;
    f_hat_plus.head(T/2) = 0;

    auto Alpha = alpha_ * Eigen::ArrayXd::Ones(nrmodes_);
    auto omega_plus = Eigen::ArrayXXd::Zero(nrmodes_);

    if (omega==Uniform)
	    omega_plus.row(0) = 0.5/nrmodes_* Eigen::ArrayXd::LinSpaced(nrmodes_,0,nrmodes_-1);
    else if (omega==Random)
	omega_plus.row(0) = (Eigen::ArrayXd::Random(nrmodes_)+1.0)/4.0;
    else
	omega_plus.row(0) = 0.0;

    if (makeDC_)
	omega_plus(0,0) = 0.0;

    auto lambda_hat = Eigen::ArrayXXcd::Zero(freqs.size());
    double udiff = tol_ + ;
    int n = 0;
    auto sum_uk = Eigen::ArrayXd::Zero(freqs.size()); //Check size and initialisation
    auto u_hat_plus = Eigen::ArrayXXcd::Zero(freqs.size, nrmodes_);

    while (uDiff>tol && n<niter_-1) {
	int k = 0;
	sum_uk += u_hat_plus.col(nrmodes_-1) - u_hat_plus.col(0);
	u_hat_plus.col(k) = (f_hat_plus - sum_uk - lambda_hat/2.0)/(1.0+Alpha(k)*(freqs-omega_plus(k))**2);
	if (!makeDC_)
	    omega_plus =

	for (int imode=0; imode<nrmodes_, imode++) {
	    sum_uk += u_hat_plus.col
	}
    }

}
