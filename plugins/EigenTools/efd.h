
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

class EFD
{
public:
    enum PadMode { None, Mirror, Symmetric };

    EFD(int maxmodes=5, PadMode padmode=None, int tapersz=0);
    ~EFD();

    void		setInput(const Eigen::ArrayXf& input);
    void                setMaxModes(int maxmodes);
    void                setEmpty();
    bool		getMode(int modenum, Eigen::ArrayXf& mode);
    bool		getModeByRank(int ranknum, Eigen::ArrayXf& mode);
    bool		validMode(int modenum) const;


protected:
    int			maxnrmodes_;
    PadMode		padmode_;
    int			tapersz_ = 0;
    Eigen::VectorXf	centralfreqs_;
    Eigen::VectorXi	rankidx_;
    Eigen::VectorXi	segbounds_;
    Eigen::VectorXcf	inputfreq_;

    Eigen::FFT<float>	fft_;
    void		computeFreq(const Eigen::ArrayXf& input);
    void		computeFreqSegments();
    void		computeModes();

};


