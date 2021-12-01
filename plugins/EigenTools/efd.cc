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

EFD::EFD(int maxmodes, PadMode padmode, int tapersz)
    : maxnrmodes_(maxmodes)
    , padmode_(padmode)
    , tapersz_(tapersz)
{
}

EFD::~EFD()
{}

bool EFD::validMode(int modenum) const
{
    return modenum>=0 && modenum<centralfreqs_.size();
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

bool EFD::getMode(int modenum, ArrayXf& mode)
{
    if (!centralfreqs_.size() || !validMode(modenum))
	return false;

    const Index fsz = inputfreq_.size();
    const Index start = segbounds_(modenum)==0 ? segbounds_(modenum) : segbounds_(modenum)+1;
    const Index stop = segbounds_(modenum+1);
    VectorXcf modefreq = VectorXcf::Zero(inputfreq_.size());
    modefreq(seq(start,stop)) = inputfreq_(seq(start,stop));
    modefreq(seq(fsz-start,fsz-stop)) = inputfreq_(seq(fsz-start,fsz-stop));
    VectorXf modevec(fsz);
    fft_.inv(modevec, modefreq);
    const Index ns = fsz/2;
    const Index pivot = ns%2==0 ? ns/2 : (ns+1)/2;
    mode.resize(ns);
    mode = modevec(seqN(ns-pivot,ns)).array();
    return true;
}

bool EFD::getModeByRank(int ranknum, ArrayXf& mode)
{
    if (!centralfreqs_.size() || !validMode(ranknum))
	return false;

    return getMode(rankidx_(ranknum), mode);
}

void EFD::computeFreq(const ArrayXf& input)
{
    int pinsz = padmode_==None ? input.size() : input.size()*2;
    VectorXf padded_input(pinsz);
    if (padmode_==None)
	padded_input << input;
    else if (padmode_==Mirror)
	EigenTools::mirrorPad(input, padded_input);
    else if (padmode_==Symmetric)
	EigenTools::symmetricPad(input, padded_input);

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

