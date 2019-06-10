#include "mistiedata.h"

#include "od_iostream.h"

MistieData::MistieData()
{}

MistieData::~MistieData()
{
    erase();
}

MistieData& MistieData::operator=(const MistieData& d)
{
    if (&d != this) {
        erase();
        dataA_ = d.dataA_;
        dataB_ = d.dataB_;
        trcA_ = d.trcA_;
        trcB_ = d.trcB_;
        pos_ = d.pos_;
        zdiff_ = d.zdiff_;
        phasediff_ = d.phasediff_;
        ampdiff_ = d.ampdiff_;
        quality_ = d.quality_;
    }
    return *this;
}

int MistieData::size() const
{
    return dataA_.size();
}

bool MistieData::read( const char* filename, bool merge )
{
    if (!merge)
        erase();
    od_istream strm(filename);
    if (!strm.isOK())
        return false;
    
    BufferString buf;
    strm.getLine(buf);
    
    BufferString lineA, lineB;
    int trcA, trcB;
    float x, y, zdiff, phasediff, ampdiff, quality;
    Coord pos;
    
    while (strm.isOK()) {
        strm >> lineA >>trcA >> lineB >> trcB >> x >> y >> zdiff >> phasediff >> ampdiff >> quality;
        if (!strm.isBad()) {
            if (lineA.size())
                add(lineA, trcA, lineB, trcB, Coord(x,y), zdiff, phasediff, ampdiff, quality);
        } else
            return false;
    };
    return true;
}

bool MistieData::write( const char* filename ) const
{
    od_ostream strm(filename);
    if (!strm.isOK())
        return false;
    
    strm << "\"LineA\"\t\"TrcA\"\t\"LineB\"\t\"TrcB\"\t\"X\"\t\"Y\"\t\"Zdiff\"\t\"PhaseDiff\"\t\"AmpDiff\"\t\"Quality\"\n";
    for (int idx=0; idx<size(); idx++)
        strm << dataA_.get(idx) <<'\t'<<trcA_[idx]<<'\t'<<dataB_.get(idx)<<'\t'<<trcB_[idx]<<'\t'
             <<pos_[idx].x<<'\t'<<pos_[idx].y<<'\t'<<zdiff_[idx]<<'\t'<<phasediff_[idx]<<'\t'<<ampdiff_[idx]<<'\t'<<quality_[idx]<<'\n';
             
    if (!strm.isOK())
        return false;
   return true; 
}

void MistieData::erase()
{
    dataA_.erase();
    trcA_.erase();
    dataB_.erase();
    trcB_.erase();
    pos_.erase();
    zdiff_.erase();
    phasediff_.erase();
    ampdiff_.erase();
    quality_.erase();
}

bool MistieData::add( const char* dataA, int trcA, const char* dataB, int trcB, Coord pos, float zdiff, float phasediff, float ampdiff, float quality )
{
    for (int idx=0; idx<size(); idx++) {
        if ((dataA_.get(idx)==dataA && dataB_.get(idx)==dataB && trcA_[idx]==trcA && trcB_[idx]==trcB) 
            || (dataA_.get(idx)==dataB && dataB_.get(idx)==dataA && trcA_[idx]==trcB && trcB_[idx]==trcA))
            return false;
    }
    dataA_.add(dataA);
    trcA_ += trcA;
    dataB_.add(dataB);
    trcB_ += trcB;
    pos_ += pos;
    zdiff_ += zdiff;
    phasediff_ += phasediff;
    ampdiff_ += ampdiff;
    quality_ += quality;
    
    return true;
}

bool MistieData::add( const char* dataA, int trcA, const char* dataB, int trcB, Coord pos )
{
    return add( dataA, trcA, dataB, trcB, pos, mUdf(float), mUdf(float), mUdf(float), mUdf(float));
}

bool MistieData::get( int idx, BufferString& dataA, int& trcA, BufferString& dataB, int& trcB, Coord& pos, float& zdiff, float& phasediff, float& ampdiff, float& quality ) const
{
    if (idx>=0 && idx<size()) {
        dataA = dataA_.get(idx);
        trcA = trcA_[idx];
        dataB = dataB_.get(idx);
        trcB = trcB_[idx];
        pos = pos_[idx];
        zdiff = zdiff_[idx];
        phasediff = phasediff_[idx];
        ampdiff = ampdiff_[idx];
        quality = quality_[idx];
        return true;
    }
    return false;
}

bool MistieData::get( int idx, BufferString& dataA, int& trcA, BufferString& dataB, int& trcB) const
{
    if (idx>=0 && idx<size()) {
        dataA = dataA_.get(idx);
        trcA = trcA_[idx];
        dataB = dataB_.get(idx);
        trcB = trcB_[idx];
        return true;
    }
    return false;
}

bool MistieData::set( int idx, const char* dataA, int trcA, const char* dataB, int trcB, Coord pos, float zdiff, float phasediff, float ampdiff, float quality )
{
    if (idx>=0 && idx<size()) {
        dataA_.get(idx) = dataA;
        trcA_[idx] = trcA;
        dataB_.get(idx) = dataB;
        trcB_[idx] = trcB;
        pos_[idx] = pos;
        zdiff_[idx] = zdiff;
        phasediff_[idx] = phasediff;
        ampdiff_[idx] = ampdiff;
        quality_[idx] = quality;
        return true;
    }
    return false;
}

bool MistieData::set( int idx, float zdiff, float phasediff, float ampdiff, float quality )
{
    if (idx>=0 && idx<size()) {
        zdiff_[idx] = zdiff;
        phasediff_[idx] = phasediff;
        ampdiff_[idx] = ampdiff;
        quality_[idx] = quality;
        return true;
    }
    return false;
}
