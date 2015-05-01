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

struct ExtProcImpl;

class ExtProc {
public:
	ExtProc( const char* exFile , const char* inFile );
    ~ExtProc();
	
	BufferString	getFile();
	void			setFile( const char* exFile, const char* inFile );
	
	void 			start( int niln, int ncrl, float inlDist, float crlDist, float zFactor, float dipFactor );
	void			compute( int z0, int inl, int crl );
	
	void			resize( int nrsamples );
	void			setInput( int input, int idx, float val );
	float			getOutput( int output, int idx );
	
	bool			isBusy() const;
	
	bool				hasInput();
	BufferString		inputName();
	
	bool				hasOutput();
	int					numOutput();
	BufferString		outputName( int onum );

	bool				hasZMargin();
	bool				hideZMargin();
	bool				zSymmetric();
	Interval<int>		zmargin();
	void				setZMargin(Interval<int> zint);

	bool				hasStepOut();
	bool				hideStepOut();
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
	
protected:
	ExtProcImpl*	pD;
}; 


#endif
