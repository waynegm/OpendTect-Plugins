#ifndef ltfattrib_h
#define ltfattrib_h

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
 Date:          April 2014
 ________________________________________________________________________
 
-*/

#include "ltfattribmod.h"
#include "attribprovider.h"

/*!\brief Local Time-Frequency Attribute

	Time-Frequency decomposition using local attributes

*/
    

namespace Attrib
{

mExpClass(LTFAttrib) LTFAttrib : public Provider
{
public:
    static void			initClass();
						LTFAttrib(Desc&);
    static const char*	attribName()			{ return "LTFAttrib"; }
    static const char*	freqStr()				{ return "freq"; }
    static const char*	smoothStr()				{ return "smooth"; }
    static const char*	niterStr()				{ return "niter"; }
    static const char*	marginStr()				{ return "margin"; }
    
protected:

							~LTFAttrib() {}
    static Provider*		createInstance(Desc&);
	static void				updateDesc(Desc&);
	
    bool					allowParallelComputation() const { return false; }

    const Interval<int>*	desZSampMargin(int,int) const { return &dessamp_; }

    bool					getInputData(const BinID&,int zintv);
    bool					computeData(const DataHolder&,const BinID& relpos, int z0,int nrsamples,int threadid) const;

    float		freq_;
	int			smooth_;
    int			niter_;
	int			margin_;

	const DataHolder*	indata_;
	int					indataidx_;
	Interval<int>		dessamp_;
	
};

}; // namespace Attrib

#endif


