/*Copyright (C) 2014 Wayne Mogg All rights reserved.

This file may be used either under the terms of:

1. The GNU General Public License version 3 or higher, as published by
the Free Software Foundation, or

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef externalattrib_h
#define externalattrib_h

/*+
________________________________________________________________________

 Author:        Wayne Mogg
 Date:          December 2014
 ________________________________________________________________________

-*/
#include "externalattribmod.h"
#include "attribprovider.h"


/*!\brief External Attribute

Compute attribute using external process 

*/
class ExtProc;

namespace Attrib
{

mClass(ExternalAttrib) ExternalAttrib : public Provider
{
public:
	static void				initClass();
							ExternalAttrib(Desc&);

	static const char*		attribName()	{ return "ExternalAttrib"; }
	void					getCompNames(BufferStringSet&) const;
	
	static const char*		interpFileStr()	{ return "interpfile"; }
	static const char*		exFileStr()		{ return "exfile"; }
	static const char*		zmarginStr() 	{ return "zmargin"; }
	static const char*		stepoutStr()	{ return "stepout"; }
	static const char*		par0Str()		{ return "par0"; }
	static const char*		par1Str()		{ return "par1"; }
	static const char*		par2Str()		{ return "par2"; }
	static const char*		par3Str()		{ return "par3"; }
	static const char*		par4Str()		{ return "par4"; }
	
protected:
							~ExternalAttrib();
	static Provider*		createInstance(Desc&);
	static void				updateDesc(Desc&);
    static ExtProc*         dProc_;
	
	bool					allowParallelComputation() const
							{ return false; }
	const Interval<int>*	desZSampMargin(int,int) const { return &zmargin_; }
	const BinID*			desStepout(int input,int output) const { return input?0:&stepout_; }
	bool					getTrcPos();
	
	bool					getInputData(const BinID&,int zintv);
	bool					computeData(const DataHolder&, const BinID& relpos, int z0, int nrsamples, int threadid) const;

				
	BufferString			interpfile_;
	BufferString			exfile_;
	Interval<int>			zmargin_;
	BinID					stepout_;
	float					par0_, par1_, par2_, par3_, par4_;
	TypeSet<BinID>			trcpos_;
	
	int						indataidx_;

	ObjectSet<const DataHolder>		indata_;
	
	ExtProc*				proc_;
};

}; // namespace Attrib


#endif


