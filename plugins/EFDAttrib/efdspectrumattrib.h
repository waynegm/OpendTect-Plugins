#pragma once
/*
 *   EFDAttrib Plugin -  Empirical Fourier Spectral Decomposition
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

#include "efdattribmod.h"
#include "attribprovider.h"

/*!\brief Empirical Fourier Decomposition Time-Frequency Attribute

	Time-Frequency decomposition using Empirical Fourier Decomposition

*/


namespace Attrib
{

mExpClass(EFDAttrib) EFDSpectrumAttrib : public Provider
{
public:
    static void		initClass();
			EFDSpectrumAttrib(Desc&);
    static const char*	attribName()		{ return "EFDSpectrumAttrib"; }
    static const char*	stepStr()		{ return "step"; }
    static const char*	modesStr()		{ return "modes"; }

    void		getCompNames(BufferStringSet&) const;
    bool		prepPriorToOutputSetup();

protected:

				~EFDSpectrumAttrib() {}
    static Provider*		createInstance(Desc&);
    static void			updateDesc(Desc&);
    static void			updateDefaults(Desc&);

    bool			allowParallelComputation() const { return false; }

    const Interval<int>*	desZSampMargin(int,int) const { return &zsampMargin_; }

    bool			getInputData(const BinID&,int zintv);
    bool			computeData(const DataHolder&,const BinID& relpos, int z0,int nrsamples,int threadid) const;

    bool			areAllOutputsEnabled() const;

    int				nmodes_;
    float			step_; // frequency spacing

    const DataHolder*		indata_;
    int				indataidx_;
    Interval<int>		zsampMargin_;
};

}; // namespace Attrib



