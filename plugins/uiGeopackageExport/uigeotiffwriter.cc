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
#include "uitaskrunner.h"
#include "executor.h"

#include "gdal_priv.h"
#include "cpl_conv.h"

uiGeotiffWriter::uiGeotiffWriter( const char* filename )
    : gdalDS_(0)
    , poSRS_(0)
    , filename_(filename)
{
    if (SI().getCoordSystem()->isProjection()) {
        const Coords::ProjectionBasedSystem* const proj = dynamic_cast<const Coords::ProjectionBasedSystem* const>(SI().getCoordSystem().ptr());
        poSRS_= new OGRSpatialReference();
        BufferString projstr(proj->getProjection()->defStr());
        projstr += " +init=epsg:";
        projstr += proj->getProjection()->authCode().id();
        
        if (poSRS_->importFromProj4(projstr) != OGRERR_NONE) {
            BufferString tmp("uiGeotiffWriter::uiGeotiffWriter - setting CRS in output file failed \n");
            tmp += "Survey CRS: ";
            tmp += SI().getCoordSystem()->summary();
            tmp += "\n";
            ErrMsg(tmp);
        }
    } else {
        BufferString crsSummary("uiGeotiffWriter::uiGeotiffWriter - unrecognised CRS: ");
        crsSummary += SI().getCoordSystem()->summary();
        ErrMsg(crsSummary);
    }
}

uiGeotiffWriter::~uiGeotiffWriter()
{
    close();
}

void uiGeotiffWriter::setFileName( const char* name )
{
    filename_  = name;
}

void uiGeotiffWriter::close()
{
    if (gdalDS_ != nullptr)
        GDALClose( gdalDS_ );
}


bool uiGeotiffWriter::writeHorizon( uiTaskRunner& taskrunner, const MultiID& hor3Dkey, bool exportZ, const BufferStringSet& attribs )
{
    const float zfac = SI().zIsTime() ? 1000 : 1;
    if (!hor3Dkey.isUdf() && poSRS_ && (exportZ || attribs.size()>0)) {
        EM::IOObjInfo eminfo(hor3Dkey);
        if (!eminfo.isOK()) {
            BufferString tmp("uiGeotiffWriter::writeHorizon - cannot read ");
            tmp += eminfo.name();
            ErrMsg( tmp );
            return false;
        }
        TrcKeySampling hs;
        hs.set(eminfo.getInlRange(), eminfo.getCrlRange());
        Coord origin = hs.toCoord(hs.atIndex(0,0));
        Coord delInl = hs.toCoord(hs.atIndex(1,0)) - origin;
        Coord delCrl = hs.toCoord(hs.atIndex(0,1)) - origin;
        double adfGeoTransform[6];
        adfGeoTransform[0] = origin.x - 0.5*delInl.x - 0.5*delCrl.x;
        adfGeoTransform[1] = delInl.x;
        adfGeoTransform[2] = delCrl.x;
        adfGeoTransform[3] = origin.y - 0.5*delInl.y - 0.5*delCrl.y;
        adfGeoTransform[4] = delInl.y;
        adfGeoTransform[5] = delCrl.y;
        
        EM::EMObject* obj = EM::EMM().loadIfNotFullyLoaded(hor3Dkey);
        if (obj==nullptr) {
            ErrMsg("uiGeotiffWriter::writeHorizon - loading 3D horizon failed");
            return false;
        }
        obj->ref();
        mDynamicCastGet(EM::Horizon3D*,hor,obj);
        if (hor==nullptr) {
            ErrMsg("uiGeotiffWriter::writeHorizon - casting 3D horizon failed");
            obj->unRef();
            return false;
        }
        
        GDALAllRegister();
        GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
        if (poDriver == nullptr) {
            ErrMsg("uiGeotiffWriter::writeHorizon - GTiff driver not available");
            return false;
        }

        int nrBands = exportZ ? attribs.size()+1 : attribs.size(); 
        gdalDS_ = poDriver->Create(filename_, hs.nrInl(), hs.nrCrl(), nrBands, GDT_Float32, nullptr);
        if (gdalDS_ == nullptr) {
            ErrMsg("uiGeotiffWriter::writeHorizon - cannot create output file.");
            return false;
        }
        gdalDS_->SetMetadataItem("AREA_OR_POINT", "POINT");
        gdalDS_->SetGeoTransform( adfGeoTransform );
        char* pszSRS_WKT = nullptr;
        poSRS_->exportToWkt( &pszSRS_WKT );
        gdalDS_->SetProjection( pszSRS_WKT );
        CPLFree( pszSRS_WKT );
        
        float* rowBuff = (float*) CPLMalloc(sizeof(float)*hs.nrCrl());
// Export Z
        if (exportZ) {
            GDALRasterBand* poBand = gdalDS_->GetRasterBand(1);
            poBand->SetNoDataValue(mUdf(float));
            poBand->SetColorInterpretation(GCI_Undefined);
            BufferString lbl("Z value ");
            lbl += SI().getZUnitString();
            poBand->SetDescription(lbl);
            for (int ildx=0; ildx<hs.nrInl(); ildx++) {
                for (int icdx=0; icdx<hs.nrCrl(); icdx++) {
                    BinID bid = hs.atIndex(ildx, icdx);
                    TrcKey tk(bid);
                    float z = hor->getZ( tk );
                    if (!mIsUdf(z))
                        z *= zfac;
                    rowBuff[icdx] = z;
                }
                if (poBand->RasterIO(GF_Write, ildx, 0, 1, hs.nrCrl(), rowBuff, 1, hs.nrCrl(), GDT_Float32, 0, 0, NULL) != CE_None) {
                    ErrMsg("uiGeotiffWriter::writeHorizon - error during RasterIO of Z values");
                    obj->unRef();
                    CPLFree( rowBuff );
                    return false;
                }
            }
            poBand->ComputeStatistics(false, NULL, NULL, NULL, NULL, NULL, NULL);
        }
// Export attributes        
        if (attribs.size()>0) {
            ExecutorGroup exgrp( "Reading attribute data" );
            for ( int idx=0; idx<attribs.size(); idx++ )
                exgrp.add( hor->auxdata.auxDataLoader(attribs.get(idx)) );
        
            if ( !TaskRunner::execute( &taskrunner, exgrp ) ) 
                return false;

            int iband = exportZ ? 2 : 1;
            for (int iatt=0; iatt<attribs.size(); iatt++) {
                if (hor->auxdata.hasAuxDataName(attribs.get(iatt))) {
                    int iaux = hor->auxdata.auxDataIndex(attribs.get(iatt));
                    GDALRasterBand* poBand = gdalDS_->GetRasterBand(iband);
                    poBand->SetNoDataValue(mUdf(float));
                    poBand->SetDescription(attribs.get(iatt));
                    poBand->SetColorInterpretation(GCI_Undefined);
                    for (int ildx=0; ildx<hs.nrInl(); ildx++) {
                        for (int icdx=0; icdx<hs.nrCrl(); icdx++) {
                            BinID bid = hs.atIndex(ildx, icdx);
                            TrcKey tk(bid);
                            const float val = hor->auxdata.getAuxDataVal(iaux, tk);
                            rowBuff[icdx] = val;
                        }
                        if (poBand->RasterIO(GF_Write, ildx, 0, 1, hs.nrCrl(), rowBuff, 1, hs.nrCrl(), GDT_Float32, 0, 0, NULL) != CE_None) {
                            BufferString tmp("uiGeotiffWriter::writeHorizon - error during RasterIO of attribute called ");
                            tmp += attribs.get(iatt);
                            ErrMsg(tmp);
                            obj->unRef();
                            CPLFree( rowBuff );
                            return false;
                        }
                    }
                    poBand->ComputeStatistics(false, NULL, NULL, NULL, NULL, NULL, NULL);
                    iband++;
                } else {
                    BufferString tmp("uiGeotiffWriter::writeHorizon - no data for attribute called ");
                    tmp += attribs.get(iatt);
                    ErrMsg(tmp);
                }
            }
        }
        CPLFree( rowBuff );
        obj->unRef();
        return true;
    }
    return false;
}
