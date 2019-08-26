#ifndef ltfattrib_h
#define ltfattrib_h
/*
 *   LocalAttrib Plugin
 *   Copyright (C) 2019  Wayne Mogg
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

#include "localattribmod.h"
#include "attribprovider.h"

/*!\brief Local Time-Frequency Attribute

	Time-Frequency decomposition using local attributes

*/
    

namespace Attrib
{

mExpClass(LocalAttrib) LTFAttrib : public Provider
{
public:
    static void			initClass();
						LTFAttrib(Desc&);
    static const char*	attribName()			{ return "LTFAttrib"; }
    static const char*	stepStr()				{ return "step"; }
    static const char*	gateStr()				{ return "gate"; }
//    static const char*	freqStr()				{ return "freq"; }
//    static const char*	smoothStr()				{ return "smooth"; }
    static const char*	niterStr()				{ return "niter"; }
//    static const char*	marginStr()				{ return "margin"; }

void				getCompNames(BufferStringSet&) const;
bool				prepPriorToOutputSetup();

protected:

							~LTFAttrib() {}
    static Provider*		createInstance(Desc&);
	static void				updateDesc(Desc&);
    static void				updateDefaults(Desc&);
    
    bool					allowParallelComputation() const { return false; }

    const Interval<int>*	desZSampMargin(int,int) const { return &zsampMargin_; }

    bool					getInputData(const BinID&,int zintv);
    bool					computeData(const DataHolder&,const BinID& relpos, int z0,int nrsamples,int threadid) const;

    bool					areAllOutputsEnabled() const;
    
    Interval<float>		gate_;
    float				window_; // effective time window
    float				step_; // frequency spacing
    
    float		freq_;
	int			smooth_;
    int			niter_;
	int			margin_;

	const DataHolder*	indata_;
	int					indataidx_;
    Interval<int>		zsampMargin_;
	
};

}; // namespace Attrib

#endif


