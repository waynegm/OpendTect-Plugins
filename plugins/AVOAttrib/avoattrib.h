#ifndef avoattrib_h
#define avoattrib_h

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
 Date:          February 2014
 ________________________________________________________________________
 
-*/

#include "avoattribmod.h"
#include "attribprovider.h"

/*!\brief AVO Attribute

	Simple AVO attributes

*/
    

namespace Attrib
{

mExpClass(AVOAttrib) AVOAttrib : public Provider
{
public:
    static void			initClass();
						AVOAttrib(Desc&);
    static const char*	attribName()			{ return "AVOAttrib"; }
    static const char*	outputStr()				{ return "output"; }
    static const char*	slopeStr()				{ return "slope"; }
    static const char*	intercept_devStr()		{ return "intercept_dev"; }
    static const char*	gradient_devStr()		{ return "gradient_dev"; }
    static const char*	correlationStr()		{ return "correlation"; }
    static const char*	class2Str()				{ return "class2width"; }
    

    enum OutputType			{ FluidFactor, LithFactor, PorosityFactor, CrossplotAngle, CrossplotDeviation, AVOClass };
	
protected:

						~AVOAttrib() {}
    static Provider*	createInstance(Desc&);
	static void			updateDesc(Desc&);
	
    bool				allowParallelComputation() const	{ return true; }

    bool		getInputData(const BinID&,int zintv);
    bool		computeData(const DataHolder&,const BinID& relpos, int z0,int nrsamples,int threadid) const;

    int			output_;
    float		xplotang_;
	float		slope_;
    float		intercept_dev_;
	float		gradient_dev_;
	float		correlation_;
	float		class2width_;

	const DataHolder*	interceptdata_;
	const DataHolder*	gradientdata_;
	int			interceptdataidx_;
	int			gradientdataidx_;
	
};

}; // namespace Attrib

#endif


