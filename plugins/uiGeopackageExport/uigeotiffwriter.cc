#include "uigeotiffwriter.h"

#include "survinfo.h"
#include "survgeom2d.h"
#include "posinfo2d.h"
#include "coordsystem.h"
#include "crssystem.h"
#include "crsproj.h"
#include "bufstring.h"
#include "string2.h"
#include "uistring.h"
#include "uimsg.h"
#include "errmsg.h"
#include "emhorizon3d.h"
#include "emmanager.h"
#include "emsurfaceiodata.h"
#include "emsurfacetr.h"
#include "emioobjinfo.h"
#include "emsurfaceauxdata.h"
#include "file.h"
#include "ioman.h"
#include "seisdatapack.h"
#include "seisioobjinfo.h"
#include "seisparallelreader.h"
#include "uitaskrunner.h"
#include "executor.h"

#include "geotiffio.h"
#include "xtiffio.h"

uiGeotiffWriter::uiGeotiffWriter( const char* fname, bool overwrite)
    : filename_(fname)
    , overwrite_(overwrite)
{

    if (SI().getCoordSystem()->isProjection())
    {
	mDynamicCastGet(const Coords::ProjectionBasedSystem*, projsys, SI().getCoordSystem().ptr());
	if (!projsys)
	{
	    errmsg_.set(tr("uiGeotiffWriter - getting survey CRS failed."));
	    return;
	}

	const auto* proj = projsys->getProjection();
	if (!proj)
	{
	    errmsg_.set(tr("uiGeotiffWriter - getting survey projection failed."));
	    return;
	}

	srs_ = new Coords::AuthorityCode(proj->authCode());
    }
    else
    {
        errmsg_.set(tr("uiGeotiffWriter - not a projected CRS: %1."));
	errmsg_.add(tr("%1").arg(SI().getCoordSystem()->summary()));
    }
}

uiGeotiffWriter::~uiGeotiffWriter()
{
    close();
    delete(srs_);
}

bool uiGeotiffWriter::open()
{
    if (File::exists(filename_) && !overwrite_)
	return false;

    tif_ = XTIFFOpen(filename_,"w");
    if (!tif_)
    {
	errmsg_.set(tr("uiGeotiffWriter::open - cannot open output file."));
	return false;
    }

    gtif_ = GTIFNew(tif_);
    if (!gtif_)
    {
	errmsg_.set(tr("uiGeotiffWriter::open - cannot create Geotiff directory."));
	return false;
    }

    const TIFFFieldInfo xtiffFieldInfo[] = {
        {TIFFTAG_GDAL_METADATA, -1, -1, TIFF_ASCII, FIELD_CUSTOM, true, false,
         const_cast<char *>("GDALMetadata")},
        {TIFFTAG_GDAL_NODATA, -1, -1, TIFF_ASCII, FIELD_CUSTOM, true, false,
         const_cast<char *>("GDALNoDataValue")},
        {TIFFTAG_RPCCOEFFICIENT, -1, -1, TIFF_DOUBLE, FIELD_CUSTOM, true, true,
         const_cast<char *>("RPCCoefficient")},
        {TIFFTAG_TIFF_RSID, -1, -1, TIFF_ASCII, FIELD_CUSTOM, true, false,
         const_cast<char *>("TIFF_RSID")},
        {TIFFTAG_GEO_METADATA, TIFF_VARIABLE2, TIFF_VARIABLE2, TIFF_BYTE,
         FIELD_CUSTOM, true, true, const_cast<char *>("GEO_METADATA")}
    };
    TIFFMergeFieldInfo(tif_, xtiffFieldInfo,
                       sizeof(xtiffFieldInfo) / sizeof(xtiffFieldInfo[0]));

    return true;
}

void uiGeotiffWriter::close()
{
    if (gtif_)
    {
	GTIFWriteKeys(gtif_);
	GTIFFree(gtif_);
	gtif_ = nullptr;
    }

    if (tif_)
    {
	XTIFFClose(tif_);
	tif_ = nullptr;
    }
}

void uiGeotiffWriter::addBandMetadata(int band, const char* description)
{
    BufferString desc("<Item name=""DESCRIPTION"" sample=""");
    desc.add(band).add(""" role=""description"">").add(description).add("</Item>");
    bandmetadata_.add(desc);
}

void uiGeotiffWriter::setMetadataField() const
{
    if ( bandmetadata_.isEmpty())
	return;

    BufferStringSet metadata;
    metadata.add("<GDALMetadata>");
    metadata.add(bandmetadata_, true);
    metadata.add("</GDALMetadata>");
    TIFFSetField(tif_, TIFFTAG_GDAL_METADATA, metadata.cat().buf());
}

uiRetVal uiGeotiffWriter::setTiffTags(const TrcKeySampling& hs, int nrbands)
{
    TIFFSetField(tif_, TIFFTAG_IMAGEWIDTH, hs.nrCrl());
    TIFFSetField(tif_, TIFFTAG_IMAGELENGTH, hs.nrInl());
    TIFFSetField(tif_, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    TIFFSetField(tif_, TIFFTAG_PLANARCONFIG, PLANARCONFIG_SEPARATE);
    TIFFSetField(tif_, TIFFTAG_PHOTOMETRIC,   PHOTOMETRIC_MINISBLACK);
    TIFFSetField(tif_, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
    TIFFSetField(tif_, TIFFTAG_SAMPLESPERPIXEL, nrbands);
    TIFFSetField(tif_,TIFFTAG_BITSPERSAMPLE, sizeof(float)*8);
    TIFFSetField(tif_, TIFFTAG_GDAL_NODATA, toString(mUdf(float)));
    TIFFSetField(tif_, TIFFTAG_ROWSPERSTRIP,  1L);
    if (nrbands>1)
    {
	PtrMan<uint16_t> exsamp = new uint16_t[nrbands-1];
	for (int idx=0; idx<nrbands-1; idx++)
	    exsamp[idx] = EXTRASAMPLE_UNSPECIFIED;

	TIFFSetField(tif_, TIFFTAG_EXTRASAMPLES, nrbands-1, exsamp.ptr());
    }

    GTIFKeySet(gtif_, GTModelTypeGeoKey, TYPE_SHORT, 1, ModelProjected);
    GTIFKeySet(gtif_, GTRasterTypeGeoKey, TYPE_SHORT, 1, RasterPixelIsPoint);
    BufferString crs( srs_->code() );
    if ( crs.isNumber(true) )
	GTIFKeySet(gtif_, ProjectedCRSGeoKey, TYPE_SHORT, 1, crs.toInt() );
    else
	return uiRetVal(tr("uiGeotiffWriter::writeHorizon - unrecognised CRS: %1.").arg(srs_->code()));


    Coord origin = hs.toCoord(hs.atIndex(0,0));
    Coord delInl = hs.toCoord(hs.atIndex(1,0)) - origin;
    Coord delCrl = hs.toCoord(hs.atIndex(0,1)) - origin;
    double geotransform[16] = {};
    geotransform[0] = delCrl.x;
    geotransform[1] = delInl.x;
    geotransform[3] = origin.x - 0.5*delInl.x - 0.5*delCrl.x;
    geotransform[4] = delCrl.y;
    geotransform[5] = delInl.y;
    geotransform[7] = origin.y - 0.5*delInl.y - 0.5*delCrl.y;
    geotransform[15] = 1.0;
    TIFFSetField(tif_, TIFFTAG_GEOTRANSMATRIX, 16, geotransform);
        // double adfGeoTransform[6];
        // adfGeoTransform[0] = origin.x - 0.5*delInl.x - 0.5*delCrl.x;
        // adfGeoTransform[1] = delInl.x;
        // adfGeoTransform[2] = delCrl.x;
        // adfGeoTransform[3] = origin.y - 0.5*delInl.y - 0.5*delCrl.y;
        // adfGeoTransform[4] = delInl.y;
        // adfGeoTransform[5] = delCrl.y;
    return uiRetVal::OK();
}

uiRetVal uiGeotiffWriter::writeHorizon( uiTaskRunner& taskrunner, const MultiID& hor3Dkey,
					bool exportZ, const BufferStringSet& attribs )
{
    if (!srs_)
	return uiRetVal(tr("uiGeotiffWriter::writeHorizon - srs is undefined."));

    if (hor3Dkey.isUdf())
	return uiRetVal(tr("uiGeotiffWriter::writeHorizon - undefined horizon id."));

    if (!exportZ && attribs.isEmpty())
	return uiRetVal(tr("uiGeotiffWriter::writeHorizon - nothing to export."));

    if (!isOK() || !open())
	return errmsg_;

    const float zfac = SI().zIsTime() ? 1000 : 1;
    EM::IOObjInfo eminfo(hor3Dkey);
    if (!eminfo.isOK())
	return uiRetVal(tr("uiGeotiffWriter::writeHorizon - cannot read %1.").arg(eminfo.name()));

    TrcKeySampling hs;
    hs.set(eminfo.getInlRange(), eminfo.getCrlRange());
    int nrBands = exportZ ? attribs.size()+1 : attribs.size();
    bandmetadata_.setEmpty();

    uiRetVal uirv = setTiffTags(hs, nrBands);
    if ( !uirv.isOK() )
    {
	close();
	return uirv;
    }

    ConstPtrMan<IOObj> ioobj = IOM().get( hor3Dkey );
    if ( !ioobj )
	return uiRetVal(tr("uiGeotiffWriter::writeHorizon - undefined horizon ioobj."));

    EM::SurfaceIOData sd;
    uiString errmsg;
    if ( !EM::EMM().getSurfaceData(ioobj->key(), sd, errmsg) )
	return uiRetVal(tr("uiGeoTiffWriter::writeHorizon - "), errmsg);

    EM::SurfaceIODataSelection sels( sd );
    sels.selvalues.erase();
    RefMan<EM::EMObject> emobj = EM::EMM().createTempObject( ioobj->group() );
    if ( !emobj )
	return uiRetVal(tr("uiGeotiffWriter::writeHorizon - cannot create temporary horizon."));

    emobj->setMultiID( ioobj->key() );
    mDynamicCastGet(EM::Horizon3D*,hor,emobj.ptr())
    PtrMan<Executor> loader = hor->geometry().loader( &sels );
    if ( !loader )
	return uiRetVal(tr("uiGeotiffWriter::writeHorizon - getting 3D horizon loader failed"));

    if ( !TaskRunner::execute(&taskrunner,*loader) )
	 return uiRetVal(tr("uiGeotiffWriter::writeHorizon - loading 3D horizon failed"));

    float* rowBuff = (float*) _TIFFmalloc(TIFFScanlineSize(tif_));
    BufferString description;
    int bandnr = 0;
// Export Z
    if (exportZ)
    {
	description = BufferString("Z value ", SI().getZUnitString());
	addBandMetadata(bandnr, description);
	for (int ildx=0; ildx<hs.nrInl(); ildx++)
	{
	    for (int icdx=0; icdx<hs.nrCrl(); icdx++)
	    {
		const auto trckey = hs.trcKeyAt( ildx, icdx );
		float z = hor->getZ( trckey );
		if (!mIsUdf(z))
		    z *= zfac;
		rowBuff[icdx] = z;
	    }
	    if (!TIFFWriteScanline(tif_, rowBuff, ildx, bandnr))
	    {
		_TIFFfree(rowBuff);
		return uiRetVal(tr("uiGeotiffWriter::writeHorizon - writing horizon z data failed"));
	    }
        }
        bandnr++;
    }
// Export attributes
    if (attribs.size()>0)
    {
	for (int iatt=0; iatt<attribs.size(); iatt++)
	{
	    PtrMan<Executor> auxloader = hor->auxdata.auxDataLoader(attribs.get(iatt).buf());
	    if ( !loader || !TaskRunner::execute( &taskrunner, *auxloader ) )
		    return uiRetVal(tr("uiGeotiffWriter::writeHorizon - loading 3D horizon attributes failed"));

	    if (hor->auxdata.hasAuxDataName(attribs.get(iatt)))
	    {

		int iaux = hor->auxdata.auxDataIndex(attribs.get(iatt));
		description = attribs.get(iatt);
		addBandMetadata(bandnr, description);
		for (int ildx=0; ildx<hs.nrInl(); ildx++)
		{
		    for (int icdx=0; icdx<hs.nrCrl(); icdx++)
		    {
			const auto trckey = hs.trcKeyAt( ildx, icdx );
			float z = hor->auxdata.getAuxDataVal( iaux, trckey );
			rowBuff[icdx] = z;
		    }
		    if (!TIFFWriteScanline(tif_, rowBuff, ildx, bandnr))
		    {
			_TIFFfree(rowBuff);
			return uiRetVal(tr("uiGeotiffWriter::writeHorizon - writing attribute data failed"));
		    }
		}
	    } else
	    {
		_TIFFfree(rowBuff);
		return uiRetVal(tr("uiGeotiffWriter::writeHorizon - no data for attribute: %1").arg(attribs.get(iatt)));
	    }
	    bandnr++;
	    hor->auxdata.removeAll();
	}
    }
    setMetadataField();
    close();
    _TIFFfree( rowBuff );

    return uiRetVal::OK();
}

uiRetVal uiGeotiffWriter::writeZSlices( uiTaskRunner& taskrunner, const TypeSet<MultiID>& seisids, float slicetime )
{
    if (!srs_)
	return uiRetVal(tr("uiGeotiffWriter::writeZSlices - srs is undefined."));

    if (seisids.isEmpty())
	return uiRetVal(tr("uiGeotiffWriter::writeZSlices - nothing to export."));

    TrcKeySampling hs;
    bool first = true;
    int nrBands = 0;
    for ( const auto& seisid : seisids )
    {
	PtrMan<IOObj> ioobj = IOM().get(seisid);
	if ( !ioobj )
	    return uiRetVal(tr("uiGeotiffWriter::writeZSlices - cannot find data set."));

	const SeisIOObjInfo seisinfo(ioobj);
	TrcKeyZSampling tkzs;
	seisinfo.getRanges(tkzs);
	nrBands += seisinfo.nrComponents();
	if (first)
	{
	    hs = tkzs.hsamp_;
	    continue;
	}

	if (tkzs.hsamp_.lineRange().isCompatible(hs.lineRange()) && tkzs.hsamp_.trcRange().isCompatible(hs.trcRange()))
	    hs.include(tkzs.hsamp_);
	else
	    return uiRetVal(tr("uiGeotiffWriter::writeZSlices - data sets have inconsistent horizontal sampling."));
    }

    if (!isOK() || !open())
	return errmsg_;

    const float zfac = SI().showZ2UserFactor();
    slicetime /= zfac;
    bandmetadata_.setEmpty();
    uiRetVal uirv = setTiffTags(hs, nrBands);
    if ( !uirv.isOK() )
    {
	close();
	return uirv;
    }

    float* rowBuff = (float*) _TIFFmalloc(TIFFScanlineSize(tif_));
    BufferString description;
    int bandnr = 0;
    for ( const auto& seisid : seisids )
    {
	PtrMan<IOObj> ioobj = IOM().get(seisid);
	const SeisIOObjInfo seisinfo(ioobj);
	TrcKeyZSampling tkzforload;
	seisinfo.getRanges(tkzforload);
	tkzforload.hsamp_ = hs;
	tkzforload.zsamp_ = StepInterval<float>(slicetime, slicetime, tkzforload.zsamp_.step);
	Seis::SequentialReader rdr( *ioobj, &tkzforload );
	if ( !rdr.execute() )
	    return uiRetVal(tr("uiGeotiffWriter::writeZSlices - reading seismic volume failed."));

	ConstRefMan<RegularSeisDataPack> dp = rdr.getDataPack();
	if ( !dp )
	    continue;

	const TrcKeyZSampling& actualtkz = dp->sampling();
	const int nrcomp = dp->nrComponents();

	for ( int cidx=0; cidx<nrcomp; cidx++ )
	{
	    description = dp->getComponentName(cidx);
	    addBandMetadata(bandnr, description);
	    const Array3DImpl<float>& arr = dp->data(cidx);
	    for (int ildx=0; ildx<hs.nrInl(); ildx++)
	    {
		for (int icdx=0; icdx<hs.nrCrl(); icdx++)
		{
		    const int dpidx = dp->getGlobalIdx(hs.trcKeyAt( ildx, icdx ));
		    const int lidx = actualtkz.hsamp_.lineIdxFromGlobal(dpidx);
		    const int tidx = actualtkz.hsamp_.trcIdxFromGlobal(dpidx);
		    if ( dpidx!=-1 )
			rowBuff[icdx] = arr.get(lidx, tidx, 0);
		    else
			rowBuff[icdx] = mUdf(float);
		}
		if (!TIFFWriteScanline(tif_, rowBuff, ildx, bandnr))
		{
		    _TIFFfree(rowBuff);
		    return uiRetVal(tr("uiGeotiffWriter::writeZSlices - writing z slices failed"));
		}
	    }
	    bandnr++;
	}
    }
    setMetadataField();
    close();
    _TIFFfree( rowBuff );

    return uiRetVal::OK();
}
