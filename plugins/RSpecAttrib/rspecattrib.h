#ifndef rspecattrib_h
#define rspecattrib_h

/*Copyright (C) 2014 Wayne Mogg All rights reserved.
 
 This file may be used either under the terms of:
 
 1. The GNU General Public License version 3 or higher, as published by
 the Free Software Foundation, or
 
 This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/
/*+
 ________________________________________________________________________
 
 Author:        Wayne Mogg
 Date:          August 2014
 ________________________________________________________________________
 
-*/

#include "rspecattribmod.h"
#include "attribprovider.h"
#include "odcomplex.h"
#include "arrayndimpl.h"

/*!\brief Recursive spectral decomposition attribute

	Spectral decomposition using a recursive filter approximation to a special case of the short time fourier transform (STFT)

*/
    

namespace Attrib
{

mExpClass(RSpecAttrib) RSpecAttrib : public Provider
{
public:
    static void			initClass();
						RSpecAttrib(Desc&);
    static const char*	attribName()			{ return "RSpecAttrib"; }
//    static const char*	freqStr()				{ return "freq"; }
    static const char*	stepStr()				{ return "step"; }
    static const char*	windowStr()				{ return "window"; }
    
    void				getCompNames(BufferStringSet&) const;
	bool				prepPriorToOutputSetup();
    
protected:

							~RSpecAttrib() {}
    static Provider*		createInstance(Desc&);
	static void				updateDesc(Desc&);
	
    bool					allowParallelComputation() const { return true; }

    const Interval<int>*	desZSampMargin(int,int) const { return &zsampMargin_; }
    
    void 					computeFilterTaps(float freq, Array1DImpl<float_complex>& num, Array1DImpl<float_complex>& den ) const;
	void 					computeFrequency(Array1DImpl<float_complex>& num, Array1DImpl<float_complex>& den,
											 int nrsamples, Array1DImpl<float>& trc, Array1DImpl<float>& spec ) const;
    bool					getInputData(const BinID&,int zintv);
    bool					computeData(const DataHolder&,const BinID& relpos, int z0,int nrsamples,int threadid) const;

	bool					areAllOutputsEnabled() const;
	
	float				window_; // effective time window
	float				step_; // frequency spacing
	int					order_; // recursive filter order

	const DataHolder*	indata_;
	int					indataidx_;
	Interval<int>		zsampMargin_;
	
};

}; // namespace Attrib

#endif


