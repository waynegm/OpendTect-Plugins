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

    BufferString	getFile() const;
    BufferString	getInterpStr() const;
    void		setFile( const char* exFile, const char* inFile );

    bool		isOK() const;
    void		setSeisInfo( int niln, int ncrl, float inlDist, float crlDist,
				     float zFactor, float dipFactor );
    void		addMetadata( const char* key, const char* value );
    void		addMetadata( const char* key, const BufferStringSet& values );
    ProcInst*		getIdleInst( int nrsamples );
    void		setInstIdle( ProcInst* pi );
    void		setInput( ProcInst* pi, int input, int trc, int idx, float val );
    float		getOutput( ProcInst* pi, int output, int idx );
    bool		compute( ProcInst* pi, int z0, int inl, int crl );

    BufferStringSet	getInputNames() const;
    BufferStringSet	getOutputNames() const;
    bool		hasInput() const;
    bool		hasInputs() const;
    int			numInput() const;
    BufferString	inputName( int inum = 0 ) const;

    bool		hasOutput() const;
    int			numOutput() const;
    BufferString	outputName( int onum = 0 ) const;

    bool		hasZMargin() const;
    bool		hideZMargin() const;
    bool		zSymmetric() const;
    Interval<int>	z_minimum() const;
    Interval<int>	zmargin() const;
    void		setZMargin(Interval<int> zint);

    bool		hasStepOut() const;
    bool		hideStepOut() const;
    bool		so_same();
    BinID		so_minimum();
    BinID		stepout() const;
    void		setStepOut(BinID s);

    bool               	hasSelect() const;
    BufferString	selectName();
    BufferString     	selectOpt( int snum );
    int			numSelect();
    int                 selectValue();
    void                setSelection(int val);

    bool		hasParam( int pnum ) const;
    BufferString	paramName( int pnum );
    float		paramValue( int pnum );
    void		setParam( int pnum, float val );

    bool		hasHelp() const;
    BufferString	helpValue() const;

    bool		doParallel();

    bool		hasNewParams() const;
    BufferStringSet	paramKeys() const;
    BufferString	getParamType(const char*) const;
    float		getParamFValue(const char* key, const char* subkey="Value") const;
    void		setParamFValue(const char*, float);
    BufferString	getParamStrValue(const char* key, const char* subkey="Value") const;
    void		setParamStrValue(const char*, const char*);
    BufferStringSet	getParamStrLstValue(const char* key, const char* subkey="Value") const;
    BufferString	getParamsEncodedStr();
    bool		setParamsEncodedStr(const BufferString&);

    uiRetVal		errMsg() const;

protected:
	ExtProcImpl*	pD;
};


#endif
