/*Copyright (C) 2014 Wayne Mogg All rights reserved.

This file may be used either under the terms of:

1. The GNU General Public License version 3 or higher, as published by
the Free Software Foundation, or

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef mlvfilterattrib_h
#define mlvfilterattrib_h

/*+
________________________________________________________________________

 Author:        Wayne Mogg
 Date:          January 2014
 ________________________________________________________________________

-*/

#include "attribprovider.h"
#include "statcalc.h"
#include "point3D.h"


/*!\brief MLVFilter Attribute

Structure preserving smoothing 

*/


namespace Attrib
{
typedef wmGeom::Point3D<int> PlanePoint;
typedef TypeSet<PlanePoint> PlanePointSet;
typedef Geom::Point2D<int> SamplePoint;
typedef TypeSet<SamplePoint> SamplePointSet;

mClass(MLVFilterAttrib) MLVFilter : public Provider
{
public:
	static void				initClass();
							MLVFilter(Desc&);

	static const char*		attribName()	{ return "MLVFilter"; }

	static const char*		sizeStr()	{ return "size"; }
	static const char*		outputStr()	{ return "output"; }
	static const char*      sdevsStr() { return "sdevs"; }

	enum OutputType			{ Average, Median, TrimmedMean, Element };

protected:
							~MLVFilter() {}
	static Provider*		createInstance(Desc&);

	bool					allowParallelComputation() const
							{ return true; }

	bool					getInputData(const BinID&,int zintv);
	bool					computeData(const DataHolder&, const BinID& relpos, int z0, int nrsamples, int threadid) const;

	int						findLeastVarianceElement( int idx, int z0, float* result , wmStats::StatCalc<float>& elemStats ) const;
	const BinID*			desStepout(int input,int output) const;
	const Interval<int>*	desZSampMargin(int input,int output) const
							{ return &dessampgate_; }
				
	bool					getTrcPos();
	bool					getElements();

	BinID					stepout_;
	Interval<int>			dessampgate_;
	TypeSet<BinID>			trcpos_;
	TypeSet<SamplePointSet>	elements_;
	int						centertrcidx_;
	int						outtype_;
	int						size_;
    float                   sdevs_;

	int						dataidx_;

	ObjectSet<const DataHolder>	inputdata_;
};

} // namespace Attrib


#endif


