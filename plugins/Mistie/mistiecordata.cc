#include "mistiecordata.h"

#include "iopar.h"

MistieCorrectionData::MistieCorrectionData()
{}

MistieCorrectionData::~MistieCorrectionData()
{}

int MistieCorrectionData::size() const
{
    return datanames_.size();
}

bool MistieCorrectionData::read( const char* filename )
{
    IOPar misties;
    if (misties.read(filename, 0)) {
        erase();
        int irow = 0;
        for (int idx=0; idx<misties.size(); idx++) {
            BufferString name = misties.getKey(idx);
            float shift, phase, amp;
            if (!name.isEmpty()) {
                misties.get(name, shift, phase, amp);
                datanames_.add(name);
                shifts_ += shift;
                phases_ += phase;
                amps_ += amp;
                irow++;
            }
        }
        return true;
    }
    return false;
}
bool MistieCorrectionData::write( const char* filename ) const
{
    IOPar misties;
    for (int idx=0; idx<size(); idx++)
        misties.set( datanames_.get(idx), shifts_[idx], phases_[idx], amps_[idx]);
    
    return misties.write(filename, 0);
}

void MistieCorrectionData::erase()
{
    datanames_.erase();
    shifts_.erase();
    phases_.erase();
    amps_.erase();
}

BufferString MistieCorrectionData::getDataName( int idx ) const
{
    BufferString result;
    if (idx>=0 && idx<size())
        result = datanames_.get(idx);
    return result;
}

float MistieCorrectionData::getZCor( int idx ) const
{
    float result = mUdf(float);
    if (idx>=0 && idx<size())
        result = shifts_[idx];
    return result;
}

float MistieCorrectionData::getPhaseCor( int idx ) const
{
    float result = mUdf(float);
    if (idx>=0 && idx<size())
        result = phases_[idx];
    return result;
}
float MistieCorrectionData::getAmpCor( int idx ) const
{
    float result = mUdf(float);
    if (idx>=0 && idx<size())
        result = amps_[idx];
    return result;
}

bool MistieCorrectionData::get( const char* name, float& shift, float& phase, float& amp ) const
{
    int idx = datanames_.indexOf(name);
    if (idx>=0) {
        shift = shifts_[idx];
        phase = phases_[idx];
        amp = amps_[idx];
        return true;
    }
    
    return false;
}

void MistieCorrectionData::set( const char* name, float shift, float phase, float amp )
{
    int idx = datanames_.indexOf(name);
    if (idx>=0) {
        shifts_[idx] = shift;
        phases_[idx] = phase;
        amps_[idx] = amp;
    } else {
        datanames_.add(name);
        shifts_ += shift;
        phases_ += phase;
        amps_ += amp;
    }
}
