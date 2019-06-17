#include "mistiecordata.h"

#include "iopar.h"
#include "msgh.h"
#include "mistiedata.h"

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

int MistieCorrectionData::getIndex( const char* dataname ) const
{
    int idx = datanames_.indexOf(dataname);
    return idx>=0 ? idx : -1;
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
    int idx = getIndex(name);
    if (idx>=0) {
        shift = shifts_[idx];
        phase = phases_[idx];
        amp = amps_[idx];
        return true;
    }
    
    return false;
}

void MistieCorrectionData::setDataName( int idx, const char* dataname )
{
    if (idx>=0 && idx <size())
        datanames_.replace(idx, new BufferString(dataname));
}

void MistieCorrectionData::setZCor( int idx, float zcor )
{
    if (idx>=0 && idx <size())
        shifts_[idx] = zcor;
}

void MistieCorrectionData::setZCor( const char* name, float zcor )
{
    int idx = datanames_.indexOf(name);
    if (idx>=0) 
        shifts_[idx] = zcor;
    else {
        datanames_.add(name);
        shifts_ += zcor;
        phases_ += 0.0;
        amps_ += 1.0;
    }
}

void MistieCorrectionData::setPhaseCor( int idx, float phasecor )
{
    if (idx>=0 && idx <size())
        phases_[idx] = phasecor;
}    

void MistieCorrectionData::setAmpCor( int idx, float ampcor )
{
    if (idx>=0 && idx <size())
        amps_[idx] = ampcor;
}

void MistieCorrectionData::setAmpCor( const char* name, float ampcor )
{
    int idx = datanames_.indexOf(name);
    if (idx>=0) 
        amps_[idx] = ampcor;
    else {
        datanames_.add(name);
        shifts_ += 0.0;
        phases_ += 0.0;
        amps_ += 1.0;
    }
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

float MistieCorrectionData::computeZCor( const MistieData& misties, const BufferStringSet& reference, float minQuality, int maxIter, float damping, float delta )
{
    BufferString lineA, lineB;
    float zdiff, errLast, err;
    
    for (int idx=0; idx<misties.size(); idx++) {
        misties.getLines(idx, lineA, lineB);
        setZCor(lineA, 0.0);
        setZCor(lineB, 0.0);
    }
    errLast = 0.0;
    int nrGood = 0;
    for (int idx=0; idx<misties.size(); idx++) {
        if (misties.getQuality(idx)>=minQuality) {
            zdiff = misties.getZMistieWith( *this, idx );
            errLast += zdiff*zdiff;
            nrGood++;
        }
    }
    errLast = sqrt(errLast/nrGood);
    BufferString logmsg("Mistie Z Correction Calculation - Initial RMS mistie: "); logmsg += errLast;
    int iter = 0;
    float chg = 0.0;
    TypeSet<float> est(size(), 0);
    TypeSet<int> estcount(size(), 0);
    do {
        est.setAll(0.0);
        estcount.setAll(0);
        for (int idx=0; idx<misties.size(); idx++) {
            if (misties.getQuality(idx)>=minQuality) {
                misties.getLines(idx, lineA, lineB );
                zdiff = misties.getZMistieWith( *this, idx );
        
                int ilA = getIndex(lineA);
                int ilB = getIndex(lineB);
                est[ilA] += zdiff;
                estcount[ilA]++;
                est[ilB] -= zdiff;
                estcount[ilB]++;
            }
        }
        for (int idx=0; idx<size(); idx++) {
            if (reference.isPresent(getDataName(idx)))
                setZCor(idx, 0.0);
            else
                setZCor(idx, getZCor(idx)+damping*est[idx]/estcount[idx]);
        }
        err=0.0;
        for (int idx=0; idx<misties.size(); idx++) {
            if (misties.getQuality(idx)>=minQuality) {
                zdiff = misties.getZMistieWith( *this, idx );
                err += zdiff*zdiff;
            }
        }
        err = sqrt(err/nrGood);
        chg = abs(errLast-err);
        iter++;
        errLast = err;
    } while (chg > delta && iter < maxIter);

    logmsg += " Final RMS mistie: "; logmsg += err; logmsg += " Iterations: "; logmsg += iter;
    UsrMsg(logmsg);

    return errLast;    
}

float MistieCorrectionData::computeAmpCor( const MistieData& misties, const BufferStringSet& reference, float minQuality, int maxIter, float damping, float delta )
{
    BufferString lineA, lineB;
    float ampdiff, errLast, err;
    
    for (int idx=0; idx<misties.size(); idx++) {
        misties.getLines(idx, lineA, lineB);
        setAmpCor(lineA, 1.0);
        setAmpCor(lineB, 1.0);
    }
    errLast = 0.0;
    int nrGood = 0;
    for (int idx=0; idx<misties.size(); idx++) {
        if (misties.getQuality(idx)>=minQuality) {
            ampdiff = misties.getAmpMistieWith( *this, idx );
            errLast += ampdiff*ampdiff;
            nrGood++;
        }
    }
    errLast = sqrt(errLast/nrGood);
    BufferString logmsg("Mistie Amplitude Correction Calculation - Initial RMS mistie: "); logmsg += errLast;
    int iter = 0;
    float chg = 0.0;
    TypeSet<float> est(size(), 0);
    TypeSet<int> estcount(size(), 0);
    do {
        est.setAll(0.0);
        estcount.setAll(0);
        for (int idx=0; idx<misties.size(); idx++) {
            if (misties.getQuality(idx)>=minQuality) {
                misties.getLines(idx, lineA, lineB );
                ampdiff = log10(misties.getAmpMistieWith( *this, idx ));
                
                int ilA = getIndex(lineA);
                int ilB = getIndex(lineB);
                est[ilA] += ampdiff;
                estcount[ilA]++;
                est[ilB] -= ampdiff;
                estcount[ilB]++;
//                BufferString tmp;
//                tmp += ampdiff; tmp.addSpace(); tmp += lineA; tmp.addSpace(); tmp += est[ilA]; tmp.addSpace(); tmp += lineB; tmp.addSpace(); tmp += est[ilB];
                //ErrMsg(tmp);
            }
        }
        for (int idx=0; idx<size(); idx++) {
//            BufferString tmp("Loop2 ");
//            tmp += est[idx]; tmp.addSpace(); tmp += estcount[idx];
//            ErrMsg(tmp);
            if (reference.isPresent(getDataName(idx)))
                setAmpCor(idx, 1.0);
            else
                setAmpCor(idx, getAmpCor(idx)*pow(10,damping*est[idx]/estcount[idx]));
        }
        err=0.0;
        for (int idx=0; idx<misties.size(); idx++) {
            if (misties.getQuality(idx)>=minQuality) {
                ampdiff = misties.getAmpMistieWith( *this, idx );
                err += ampdiff*ampdiff;
            }
        }
        err = sqrt(err/nrGood);
        chg = abs(errLast-err);
        iter++;
        errLast = err;
    } while (chg > delta && iter < maxIter);
    
    logmsg += " Final RMS mistie: "; logmsg += err; logmsg += " Iterations: "; logmsg += iter;
    UsrMsg(logmsg);
    
    return errLast;    
}
