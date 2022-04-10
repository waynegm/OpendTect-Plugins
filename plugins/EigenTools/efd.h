
#pragma once
/*
 *   EigenTools Plugin -  Empirical Fourier Decomposition
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
#include "eigentoolsmod.h"
#include "Eigen/Core"
#include <unsupported/Eigen/FFT>
#include "commondefs.h"

mExpClass(EigenTools) EFD
{
public:
    enum PadMode { None, OneSided, Mirror, Symmetric };

    EFD(int maxmodes=5, PadMode padmode=OneSided, bool deramp=true, int tapersz=0);
    ~EFD();

    bool		isEmpty() const		{ return inputfreq_.size()==0; }
    void		setEmpty();

    int			freqsize() const	{ return inputfreq_.size(); }
    int			outsize() const		{ return padmode_==None ? freqsize() : freqsize()/2; }

    void		setInput(const Eigen::ArrayXf& input);
    void                setMaxModes(int maxmodes);
    Eigen::ArrayXf	getMode(int modenum);
    Eigen::ArrayXcf	getModeAnalyticSignal(int modenum);
    Eigen::Array2Xf	getModeHTInstFrequency(int modenum, float dt);
    Eigen::Array2Xf	getModeTKInstFrequency(int modenum, float dt);
    Eigen::ArrayXf	getModeByRank(int ranknum);
    bool		getRamp(float& rampst, float& rampend) const;
    bool		isValidMode(int modenum) const;

    static Eigen::ArrayXf	TKEO(const Eigen::ArrayXf& input);
protected:
    int			maxnrmodes_;
    PadMode		padmode_;
    int			tapersz_ = 0;
    bool		deramp_;
    float		rampst_ = 0.f;
    float		rampend_ = 0.f;
    Eigen::VectorXf	centralfreqs_;
    Eigen::VectorXi	rankidx_;
    Eigen::VectorXi	segbounds_;
    Eigen::VectorXcf	inputfreq_;

    Eigen::FFT<float>	fft_;
    void		computeFreq(const Eigen::ArrayXf& input);
    void		computeFreqSegments();
    void		computeModes();
    Eigen::VectorXcf	computeModeSpectrum(int modenum);
    Eigen::VectorXcf	computeAnalyticSignalSpectrum(int modenum);
    int			pivot() const
			{ const int ns = outsize(); return padmode_==None||padmode_==OneSided ? ns : ns%2==0 ? ns/2 : (ns+1)/2; }

};


