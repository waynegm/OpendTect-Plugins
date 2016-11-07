/*Copyright (C) 2014 Wayne Mogg All rights reserved.

This file may be used either under the terms of:

1. The GNU General Public License version 3 or higher, as published by
the Free Software Foundation, or

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef extproc_h
#define extproc_h

/*+
________________________________________________________________________

 Author:        Wayne Mogg
 Date:          December 2014
 ________________________________________________________________________

-*/ 

#include "bufstring.h"
#include "externalattribmod.h"

struct ExtProcImpl;
class ProcInst;



mExpClass(ExternalAttrib) ExtProc {
public:
	ExtProc( const char* exFile , const char* inFile );
    ~ExtProc();
	
	BufferString	getFile();
	void			setFile( const char* exFile, const char* inFile );
	
	bool			isOK();
	void			setSeisInfo( int niln, int ncrl, float inlDist, float crlDist, float zFactor, float dipFactor );
	ProcInst*		getIdleInst( int nrsamples );
	void			setInstIdle( ProcInst* pi );
	void			setInput( ProcInst* pi, int input, int trc, int idx, float val );
	float			getOutput( ProcInst* pi, int output, int idx );
	bool			compute( ProcInst* pi, int z0, int inl, int crl );
	
	bool				hasInput();
	bool				hasInputs();
	int					numInput();
	BufferString		inputName( int inum = 0 );
	
	bool				hasOutput();
	int					numOutput();
	BufferString		outputName( int onum = 0 );

	bool				hasZMargin();
	bool				hideZMargin();
	bool				zSymmetric();
	Interval<int>		z_minimum();
	Interval<int>		zmargin();
	void				setZMargin(Interval<int> zint);

	bool				hasStepOut();
	bool				hideStepOut();
	BinID				so_minimum();
	BinID				stepout();
	void				setStepOut(BinID s);
    
	bool                hasSelect();
	BufferString		selectName();
    BufferString     	selectOpt( int snum );
	int					numSelect();
    int                 selectValue();
    void                setSelection(int val);
	
	bool				hasParam( int pnum );
	BufferString		paramName( int pnum );
	float				paramValue( int pnum );
	void				setParam( int pnum, float val );
	
	bool				hasHelp();
	BufferString		helpValue();
	
	bool				doParallel();
	
protected:
	ExtProcImpl*	pD;
}; 


#endif
