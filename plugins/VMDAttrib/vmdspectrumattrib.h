#pragma once
/*
 *   VMDAttrib Plugin -  Variational Mode Spectral Decomposition
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
#include "attribprovider.h"

/*!\brief Variational Mode Decomposition Time-Frequency Attribute

	Time-Frequency decomposition using Varaitional Mode Decomposition

*/


namespace Attrib
{

mExpClass(VMDAttrib) VMDSpectrumAttrib : public Provider
{
public:
    static void		initClass();
			VMDSpectrumAttrib(Desc&);
    static const char*	attribName()		{ return "VMDSpectrumAttrib"; }
    static const char*	stepStr()		{ return "step"; }
    static const char*	modesStr()		{ return "modes"; }

    void		getCompNames(BufferStringSet&) const;
    bool		prepPriorToOutputSetup();

protected:

				~VMDSpectrumAttrib() {}
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



