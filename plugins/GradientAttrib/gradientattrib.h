/*Copyright (C) 2015 Wayne Mogg All rights reserved.

This file may be used either under the terms of:

1. The GNU General Public License version 3 or higher, as published by
the Free Software Foundation, or

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef gradientattrib_h
#define gradientattrib_h

/*+
________________________________________________________________________

 Author:        Wayne Mogg
 Date:          November 2015
 ________________________________________________________________________

-*/

#include "gradientattribmod.h"
#include "attribprovider.h"


/*!\brief Gradient Attribute

Calculate inline, crossline or Z gradient using the operators proposed by Kroon, 2009 

*/


namespace Attrib
{

mClass(GradientAttrib) GradientAttrib : public Provider
{
public:
	static void			initClass();
					GradientAttrib(Desc&);

	static const char*		attribName()	{ return "GradientAttrib"; }

	static const char*		operatorStr()	{ return "operator"; }
	static const char*		outputStr()	{ return "output"; }

	enum OutputType3D		{ Inline, Crossline, Z3D };
	enum OutputType2D		{ Line, Z2D };
	enum OperatorType		{ Kroon_3, Farid_5, Farid_7};
	
	static const float		kroon_3_d[];
	static const float		kroon_3_s[];
	static const float		farid_5_d[];
	static const float		farid_5_s[];
	static const float		farid_7_d[];
	static const float		farid_7_s[];

protected:
					~GradientAttrib();
	static Provider*		createInstance(Desc&);

	bool				allowParallelComputation() const
					{ return true; }

	bool				getInputData(const BinID&,int zintv);
	bool				computeData(const DataHolder&, const BinID& relpos, int z0, int nrsamples, int threadid) const;

	const BinID*			desStepout(int input,int output) const;
	const Interval<int>*		desZSampMargin(int input,int output) const
					{ return &zmargin_; }
				
	bool				getTrcPos();

	BinID				stepout_;
	Interval<int>			zmargin_;
	TypeSet<BinID>			trcpos_;
	int				centertrcidx_;
	int				outtype_;
	int				optype_;
	int				size_;
	
	float				*ikernel_, *xkernel_, *zkernel_;

	int				dataidx_;

	ObjectSet<const DataHolder>	inputdata_;
};

}; // namespace Attrib


#endif


