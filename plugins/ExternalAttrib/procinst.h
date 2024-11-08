#pragma once
/*Copyright (C) 2015 Wayne Mogg All rights reserved.

This file may be used either under the terms of:

1. The GNU General Public License version 3 or higher, as published by
the Free Software Foundation, or

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/
/*+
________________________________________________________________________

 Author:        Wayne Mogg
 Date:          May 2015
 ________________________________________________________________________

-*/
#include "bufstring.h"
#include "uistringset.h"

struct SeisInfo
{
	int		nrTraces;
	int		nrInput;
	int		nrOutput;
	int		nrInl;
	int		nrCrl;
	float		zStep;
	float		inlDistance;
	float		crlDistance;
	float		zFactor;
	float		dipFactor;
};

struct ProcInstImpl;

class ProcInst {
public:
	ProcInst();
	~ProcInst();

	void			setInput( int input, int trc, int idx, float val );
	float			getOutput( int output, int idx );
	void			resize( int nrsamples );

	bool			start( const BufferStringSet& runargs);
	bool			start( const BufferStringSet& runargs, SeisInfo& si );
	int				finish();
	BufferString		logFileName();
	BufferString		readAllStdOut();
	bool			compute( int z0, int inl, int crl );

	void			processLog();
	uiRetVal		errMsg();
	bool			isOK();

protected:
	bool			writeSeisInfo(SeisInfo& si);
	bool			writeTrcInfo( int z0, int inl, int crl );
	bool			writeData();
	bool			readData();
	void			repError(const char*);

	ProcInstImpl*		pD;

};
