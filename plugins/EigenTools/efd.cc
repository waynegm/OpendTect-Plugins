/*
 *   EigenTools Plugin - Empirical Fourier Decomposition
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
#include "efd.h"

#include <Eigen/Core>
#include "eigentools.h"

EFD::EFD(int maxmodes)
    , maxnrmodes_(maxmodes)
{
}

EFD::~EFD()
{}

bool EFD::validMode(int modenum)
{
    return modenum>=0 && modenum<modes_.cols();
}

void EFD::setInput(const Eigen::ArrayXd& input)
{
    modes_.resize(input.size(), maxnrmodes_);
    computeFreq(input);
    computeSegments();
}

bool EFD::getMode(int modenum, Eigen::ArrayXd& mode) const
{
    if (!validMode(modenum) || !modes_.rows())
	return false;

    mode.resize(modes_.rows());
    mode = modes_.col(modenum);
}

void EFD::computeFreq(const Eigen::ArrayXd& input)
{
    Eigen::ArrayXd padded_input(input.size()*2);
    mirrorPad(input, padded_input);
    inputfreq_.resize(padded_input.size());
    fft_.fwd(inputfreq_, padded_input);
}

void EFD::computeSegments()
{
    const int ampsz = inputfreq_.size()/2;
    Eigen::ArrayXd amp = inputfreq_.head(ampsz).abs();
    Eigen::ArrayXd locmax = Eigen::ArrayXd::Zero(ampsz);
    const int sz = ampsz-2;
    locmax(seqN(1, sz)) = (amp.head(sz)<amp(seqN(1,sz)) && amp(seqN(1,sz))>amp(seqN(2,sz))).select(amp(seqN(1,sz)), 0);
    locmax(0) = amp(0);
    locmax(ampsz-1) = amp(ampsz-1);

    Eigen::ArrayXi srtidx = Eigen::ArrayXi::LinSpaced(ampsz, 0, ampsz);
    std::sort(srtidx.begin(), srtidx.end(), [&](int i,int j){return locmax(i)>locmax(j);});
    std::vector<Eigen::Index> idxmax;
    for (int idx=0; idx<srtidx.size(); idx++) {
	Eigen::Index eidx = srtidx(idx);
	if (locmax(eidx)==0.0 || idx>=maxnrmodes_)
	    break;
	idxmax.push_back(eidx)
    }
    std::sort(idxmax.begin(), idxmax.end());

    const int M = idxmax.size()+1;
    seqbounds_ = Eigen::ArrayXi::Zero(M);
    int index;
    seqbounds_(0) = 0;
    seqbounds_(M-1) = ampsz-1;
    for (int idx=1; idx<M-1; idx++) {
	amp(seq(idxmax(idx-1), idxmax(idx))).minCoeff(index);
	seqbounds_(idx) = index;
    }
    centralfreqs_ = idxmax*mPI/ampsz;
}

void EFD::computeModes(const Eigen::ArrayXd& input)
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
