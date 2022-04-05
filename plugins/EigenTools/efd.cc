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
#include <unsupported/Eigen/FFT>
#include "eigentools.h"

#include "commondefs.h"
#include <iostream>

using namespace Eigen;

EFD::EFD(int maxmodes, PadMode padmode, bool deramp, int tapersz)
    : maxnrmodes_(maxmodes)
    , padmode_(padmode)
    , tapersz_(tapersz)
    , deramp_(deramp)
{
}

EFD::~EFD()
{}

bool EFD::isValidMode(int modenum) const
{
    return centralfreqs_.size()>0 && modenum>=0 && modenum<centralfreqs_.size();
}

void EFD::setInput(const ArrayXf& input)
{
    computeFreq(input);
    computeFreqSegments();
}

void EFD::setMaxModes(int maxmodes)
{
    if (maxmodes==maxnrmodes_)
	return;
    maxnrmodes_ = maxmodes;
    if (inputfreq_.size())
	computeFreqSegments();
}

void EFD::setEmpty()
{
    inputfreq_.resize(0);
    centralfreqs_.resize(0);
    segbounds_.resize(0);
    rankidx_.resize(0);
}

ArrayXf EFD::getMode(int modenum)
{
    ArrayXf mode;
    if (isValidMode(modenum)) {
	auto modespec = computeModeSpectrum(modenum);
	const int fsz = inputfreq_.size();
	const int ns = outsize();
	const int piv = pivot();
	VectorXf modevec(fsz);
	fft_.inv(modevec, modespec);
	mode = modevec(seqN(ns-piv,ns)).array();
    }
    return mode;
}

ArrayXcf EFD::getModeAnalyticSignal(int modenum)
{
    ArrayXcf mode_as;
    auto modespec = computeAnalyticSignalSpectrum(modenum);
    if (modespec.size()>0) {
	const int fsz = freqsize();
	const int ns = outsize();
	const int piv = pivot();
	VectorXcf modevec(fsz);
	fft_.inv(modevec, modespec);
	mode_as = modevec(seqN(ns-piv,ns)).array();
    }
    return mode_as;
}

Array2Xf EFD::getModeHTInstFrequency(int modenum, float dt)
{
    auto mode_as = getModeAnalyticSignal(modenum);
    Array2Xf mif = Array2Xf::Zero(2, mode_as.size());
    if (mode_as.size()>0) {
	mif.row(1) = abs(mode_as);
	const int ns = mode_as.size()-2;
	mif.row(0)(seqN(0,ns)) = arg(mode_as(seqN(1,ns))*conj(mode_as(seqN(0,ns))));
	mif(0,ns+1) = arg(mode_as(ns+1)*conj(mode_as(ns+1)));
	mif = (mif<0.f).select(mif+M_PI, mif)/(M_2PI*dt);
    }
    return mif;
}

Array2Xf EFD::getModeTKInstFrequency(int modenum, float dt)
{
    auto mode = getMode(modenum);
    const int ns = mode.size();
    Array2Xf mif = Array2Xf::Zero(2, ns);
    if (ns>0) {
	const int nsm2 = ns-2;
	ArrayXf mode_tk = TKEO(mode);
	ArrayXf yn = mode;
	yn(seq(1,ns-1)) = yn(seq(1,ns-1)) - mode(seq(0,nsm2));
	ArrayXf ynp1 = mode;
	ynp1(seq(0,nsm2)) = mode(seq(1,ns-1)) - ynp1(seq(0,nsm2));
	auto desa_arg = 1.0f - (TKEO(yn)+TKEO(ynp1))/(4.0f*mode_tk);

	mif.row(1) = (mode_tk/(1.0f-desa_arg.square())).sqrt();
	mif.row(0) =  desa_arg.acos()/(M_2PI*dt);
    }
    return mif;
}

ArrayXf EFD::getModeByRank(int ranknum)
{
    ArrayXf mode;
    if (isValidMode(ranknum))
	mode = getMode(rankidx_(ranknum));

    return mode;
}

bool EFD::getRamp(float& rampst, float& rampend) const
{
    if (!deramp_)
	return false;

    rampst = rampst_;
    rampend = rampend_;
    return true;
}

void EFD::computeFreq(const ArrayXf& input)
{
    int pinsz = padmode_==None ? input.size() : input.size()*2;
    VectorXf padded_input(pinsz);
    ArrayXf ramp;
    if (deramp_) {
	rampst_ = input(0);
	rampend_ = input(input.size()-1);
	ramp = ArrayXf::LinSpaced(input.size(), rampst_, rampend_);
    }

    if (padmode_==None)
	padded_input << (deramp_ ?  input-ramp : input);
    else if (padmode_==OneSided)
	padded_input << (deramp_ ?  input-ramp : input), ArrayXf::Zero(input.size());
    else if (padmode_==Mirror)
	EigenTools::mirrorPad((deramp_ ?  input-ramp : input), padded_input);
    else if (padmode_==Symmetric)
	EigenTools::symmetricPad((deramp_ ?  input-ramp : input), padded_input);

    if (tapersz_>0)
	EigenTools::cosTaper(tapersz_, padded_input);

    inputfreq_.resize(pinsz);
    fft_.fwd(inputfreq_, padded_input);
}

void EFD::computeFreqSegments()
{
    const int ampsz = inputfreq_.size()/2;
    ArrayXf amp = inputfreq_.head(ampsz).array().abs();
    ArrayXf locmax = ArrayXf::Zero(ampsz);
    const int sz = ampsz-2;
    locmax(seqN(1, sz)) = (amp.head(sz)<amp(seqN(1,sz)) && amp(seqN(1,sz))>amp(seqN(2,sz))).select(amp(seqN(1,sz)), 0);

    VectorXi srtidx = VectorXi::LinSpaced(ampsz, 0, ampsz);
    std::sort(srtidx.begin(), srtidx.end(), [&](int i,int j){return locmax(i)>locmax(j);});
    std::vector<Index> idxmax;
    for (int idx=0; idx<srtidx.size(); idx++) {
	Index eidx = srtidx(idx);
	if (locmax(eidx)==0.0 || idx>=maxnrmodes_)
	    break;
	idxmax.push_back(eidx);
    }
    rankidx_.resize(idxmax.size());
    for (int idx=0; idx<idxmax.size(); idx++)
	rankidx_(idx) = idxmax[idx];

    std::sort(idxmax.begin(), idxmax.end());

    const int M = idxmax.size()+1;
    segbounds_ = VectorXi::Zero(M);
    int index;
    segbounds_(0) = 0;
    segbounds_(M-1) = ampsz;
    for (int idx=1; idx<M-1; idx++) {
	amp(seq(idxmax[idx-1], idxmax[idx])).minCoeff(&index);
	segbounds_(idx) = index + idxmax[idx-1];
    }
    centralfreqs_.resize(idxmax.size());
    const float freqstep = 2.0f*M_PI/inputfreq_.size();
    for (int idx=0; idx<idxmax.size(); idx++ )
	centralfreqs_(idx)= idxmax[idx]*freqstep;
}

// Compute spectrum of the mode
VectorXcf EFD::computeModeSpectrum(int modenum)
{
    VectorXcf spec;
    if (isValidMode(modenum)) {
	const int fsz = inputfreq_.size();
	const Index start = segbounds_(modenum)==0 ? segbounds_(modenum) : segbounds_(modenum)+1;
	const Index stop = segbounds_(modenum+1);
	spec = VectorXcf::Zero(fsz);
	spec(seq(start,stop)) = inputfreq_(seq(start,stop));
	spec(seq(fsz-1-stop,fsz-1-start)) = inputfreq_(seq(fsz-1-stop,fsz-1-start));
    }
    return spec;
}

// Compute spectrum of the mode's analytic signal
VectorXcf EFD::computeAnalyticSignalSpectrum(int modenum)
{
    VectorXcf spec;
    auto modespec = computeModeSpectrum(modenum);
    if (modespec.size()>0) {
	const int fsz = inputfreq_.size();
	spec = VectorXcf::Zero(fsz);
	spec(0) = modespec(0);
	spec(seq(1,fsz/2-1)) = modespec(seq(1,fsz/2-1))*2.f;
	spec(fsz/2) = modespec(fsz/2);
    }
    return spec;
}

ArrayXf EFD::TKEO(const ArrayXf& input)
{
    const int ns = input.size();
    const int nsm2 = ns-2;
    ArrayXf result = input.square();
    result(seq(1,nsm2)) = result(seq(1,nsm2)) - input(seq(0,nsm2-1))*input(seq(2,nsm2+1));
    return result;
}
