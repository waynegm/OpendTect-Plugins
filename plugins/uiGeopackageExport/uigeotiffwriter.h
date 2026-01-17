#pragma once

#include "arraynd.h"
#include "bufstring.h"
#include "bufstringset.h"
#include "multiid.h"
#include "uistringset.h"

#include "geotiffio.h"
#include "xtiffio.h"

namespace Coords{ class AuthorityCode; }
class BufferStringSet;
class TrcKeySampling;
class uiTaskRunner;

class uiGeotiffWriter
{ mODTextTranslationClass(uiGeotiffWriter);
public:
    uiGeotiffWriter( const char* filename, bool overwrite=true );
    ~uiGeotiffWriter();


    uiRetVal	writeHorizon( uiTaskRunner& taskrunner, const MultiID& hor3Dkey, bool exportZ, const BufferStringSet& attribs );
    uiRetVal	writeZSlices( uiTaskRunner& taskrunner, const TypeSet<MultiID>& seisids, float slicetime );
    bool	isOK()		{ return errmsg_.isOK(); }

protected:
    void	close();
    bool	open();
    void 	addBandMetadata(int bandnr, const char* description);
    void	setMetadataField() const;
    uiRetVal	setTiffTags(const TrcKeySampling&, int nrBands);

    TIFF*				tif_	= nullptr;
    GTIF*				gtif_	= nullptr;
    Coords::AuthorityCode*		srs_	= nullptr;
    BufferString			filename_;
    bool				overwrite_ = true;
    BufferStringSet			bandmetadata_;
    uiRetVal				errmsg_;

};
