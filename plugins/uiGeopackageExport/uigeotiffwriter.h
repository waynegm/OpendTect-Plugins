#ifndef uigeotiffwriter_h
#define uigeotiffwriter_h

#include "arraynd.h"
#include "bufstring.h"
#include "bufstringset.h"
#include "multiid.h"
#include "uistringset.h"

#include "geotiffio.h"
#include "xtiffio.h"

namespace Coords{ class AuthorityCode; }
class BufferStringSet;
class uiTaskRunner;

class uiGeotiffWriter
{ mODTextTranslationClass(uiGeotiffWriter);
public:
    uiGeotiffWriter(const MultiID& hor3Dkey, const char* filename, bool overwrite=true );
    ~uiGeotiffWriter();


    uiRetVal	writeHorizon( uiTaskRunner& taskrunner, bool exportZ, const BufferStringSet& attribs );
    bool	isOK()		{ return errmsg_.isOK(); }

protected:
    void	close();
    bool	open();
    void 	addBandMetadata(int bandnr, const char* description);
    void	setMetadataField() const;

    TIFF*				tif_	= nullptr;
    GTIF*				gtif_	= nullptr;
    Coords::AuthorityCode*		srs_;
    MultiID				hor3Did_;
    BufferString			filename_;
    bool				overwrite_ = true;
    BufferStringSet			bandmetadata_;
    uiRetVal				errmsg_;

};

#endif
