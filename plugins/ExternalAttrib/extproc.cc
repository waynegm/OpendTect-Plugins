/*Copyright (C) 2014 Wayne Mogg All rights reserved.

This file may be used either under the terms of:

1. The GNU General Public License version 3 or higher, as published by
the Free Software Foundation, or

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

/*+
________________________________________________________________________

 Author:        Wayne Mogg
 Date:          December 2014
 ________________________________________________________________________

-*/
#include "survinfo.h"
#include "errmsg.h"
#include "ranges.h"
#include "position.h"
#include "extproc.h"
#include "json.h"
#include "msgh.h"
#include "objectset.h"
#include "file.h"
#include "filepath.h"
#include "envvars.h"
#include "procinst.h"
#include "urllib.h"


static const char* LegacyKeys[] =
{
    "Input",
    "Inputs",
    "Output",
    "ZSampMargin",
    "StepOut",
    "Select",
    "Par_0",
    "Par_1",
    "Par_2",
    "Par_3",
    "Par_4",
    "Par_5",
    "Help",
    "Parallel",
    0
};

struct ExtProcImpl
{
public:
    ExtProcImpl(const char* fname, const char* iname);
    ~ExtProcImpl();

    void		startInst( ProcInst* pi );

    bool		getParam();
    void		updateNewParamKeys();
    void		setFile(const char* fname, const char* iname);
    BufferStringSet	getInterpreterArgs() const;
    void		addQuotesIfNeeded(BufferStringSet& args);

    bool		isok_;
    SeisInfo		seisinfo_;
    BufferString 	exfile_;
    BufferString	infile_;
    json::Value		jsonpar_;
    BufferStringSet	newparamkeys_;
    ObjectSet<ProcInst> idleinsts_;
    Threads::Mutex	idleinstslock_;
};

ExtProcImpl::ExtProcImpl(const char* fname, const char* iname)
:  idleinsts_(),isok_(true)
{
    setFile(fname, iname);
}

ExtProcImpl::~ExtProcImpl()
{
// Delete all ProcInst's in idleinsts_
    while (!idleinsts_.isEmpty()) {
	ProcInst* pi = idleinsts_.pop();
	delete pi;
    }
}

void ExtProcImpl::setFile(const char* fname, const char* iname)
{
    exfile_ = fname;
    if (File::isFile(exfile_)) {
	infile_ = BufferString(iname).trimBlanks();
	if (infile_.startsWith("%") && infile_.endsWith("%")) {
	    BufferString tmp = infile_.unEmbed('%','%');
	    infile_ = GetEnvVar(tmp);
	}
	if (File::isExecutable(infile_))
	{
	    if (getParam())
		updateNewParamKeys();
	    else
		newparamkeys_.setEmpty();
	}
	else
	    ErrMsg("ExtProcImpl::setFile - interpreter setting is not a valid executable file");
    } else
	ErrMsg("ExtProcImpl::setFile - external attribute is not a valid file");
}

void ExtProcImpl::updateNewParamKeys()
{
    newparamkeys_.setEmpty();
    const BufferStringSet legacy(LegacyKeys);
    if (jsonpar_.GetType() == json::ObjectVal) {
	const json::Object jsonobj = jsonpar_.ToObject();
	for (auto it=jsonobj.begin(); it!=jsonobj.end(); ++it)
	{
	    const BufferString key(it->first.c_str());
	    if (!legacy.isPresent(key))
		newparamkeys_.add(key);
	}
    }
}

BufferStringSet ExtProcImpl::getInterpreterArgs() const
{
    BufferStringSet res;
    if (infile_.isEmpty())
	return res;

#ifdef __win__
    BufferString pythonstr(sKey::Python());
    pythonstr.toLower();
    if ( infile_.find(pythonstr) && infile_.find( "envs" ) ) {
// Must be an Anaconda environment
	BufferString pyexe = FilePath(infile_).fileName();
	FilePath envpath = FilePath(infile_).pathOnly();
	BufferString envname = envpath.fileName();
	FilePath envspath(envpath.pathOnly());
	if (envspath.fileName() == "envs") {
	    FilePath anaconda_root = FilePath(envspath.pathOnly());
	    FilePath anaconda_scripts = anaconda_root.add("Scripts").add("activate.bat");
	    res.add(anaconda_scripts.fullPath());
	    res.add(envname).add("^&").add(pyexe);
	} else
	    res.add(infile_);
    } else if (infile_.find(pythonstr)) {
	FilePath root(FilePath(infile_).pathOnly());
	if (root.add("envs").exists()) {
// Probably an Anaconda base environment
	    BufferString envname("base");
	    BufferString pyexe = FilePath(infile_).fileName();
	    FilePath anaconda_root = FilePath(infile_).pathOnly();
	    FilePath anaconda_scripts = anaconda_root.add("Scripts").add("activate.bat");
	    res.add(anaconda_scripts.fullPath());
	    res.add(envname).add("^&").add(pyexe);
	}
	else
	    res.add(infile_);
    } else
	res.add(infile_);
#else
    res.add(infile_);
#endif
    return res;
}

void ExtProcImpl::addQuotesIfNeeded(BufferStringSet& args)
{
    for (int idx = 0; idx < args.size(); idx++) {
	BufferString& str = args.get(idx);
	if (!str.find(' '))
	    continue;
	if ((str.first()=='"' && str.last()=='"'))
	    continue;
	const char* quote = "\"'";
	str.insertAt(0, quote);
	str.add(quote);
    }
}

void ExtProcImpl::startInst( ProcInst* pi )
{
    BufferString params(json::Serialize(jsonpar_).c_str());
    BufferStringSet runargs;
    if ( !infile_.isEmpty() && !exfile_.isEmpty() )
	runargs = getInterpreterArgs();
    else if ( exfile_.isEmpty() ) {
	ErrMsg("ExtProcImpl::startInst - no external attribute file provided");
	return;
    }

    runargs.add(exfile_);
    runargs.add("-c");
    runargs.add(urllib::urlencode(params.str()).c_str());
    addQuotesIfNeeded(runargs);

    if (!pi->start( runargs, seisinfo_ ))
	ErrMsg("ExtProcImpl::startInst - run error");
}

bool ExtProcImpl::getParam()
{
    ProcInst pi;
    bool result = true;
    BufferString params;
    BufferStringSet runargs;
    if (!infile_.isEmpty() && !exfile_.isEmpty())
	runargs = getInterpreterArgs();
    else if (exfile_.isEmpty()) {
	ErrMsg("ExtProcImpl::startInst - no external attribute file provided");
	return false;
    }

    runargs.add(exfile_);
    runargs.add("-g");
    addQuotesIfNeeded(runargs);

    if (pi.start( runargs )) {
	params = pi.readAllStdOut();
	if (pi.finish() != 0 ) {
	    ErrMsg("ExtProcImpl::getParam - external attribute exited abnormally");
	    result = false;
	}
    } else {
	ErrMsg("ExtProcImpl::getParam - run error");
	result = false;
    }
    if (result) {
	jsonpar_ = json::Deserialize(urllib::urldecode(std::string(params.str())));
	if (jsonpar_.GetType() == json::NULLVal) {
	    ErrMsg("ExtProcImpl::getParam - parameter output of external attribute is not valid JSON");
	    if (!params.isEmpty())
		UsrMsg(params);
	    result = false;
	}
    }
    isok_ = result;
    return result;
}

ExtProc::ExtProc( const char* fname, const char* iname )
: pD(new ExtProcImpl(fname, iname))
{
}

ExtProc::~ExtProc()
{
    delete pD;
}

bool ExtProc::isOK()
{
    return pD->isok_;
}

void ExtProc::setSeisInfo( int ninl, int ncrl, float inlDist, float crlDist, float zFactor, float dipFactor )
{
    (pD->seisinfo_).nrTraces = ninl*ncrl;
    (pD->seisinfo_).nrInl = ninl;
    (pD->seisinfo_).nrCrl = ncrl;
    (pD->seisinfo_).zStep = SI().zStep();
    (pD->seisinfo_).inlDistance = inlDist;
    (pD->seisinfo_).crlDistance = crlDist;
    (pD->seisinfo_).zFactor = zFactor;
    (pD->seisinfo_).dipFactor = dipFactor;
    (pD->seisinfo_).nrOutput = numOutput();
    (pD->seisinfo_).nrInput = numInput();
}

void ExtProc::setInput( ProcInst* pi, int inpdx, int trc, int idx, float val )
{
    pi->setInput( inpdx, trc, idx, val );
}

float ExtProc::getOutput( ProcInst* pi, int outdx, int idx )
{
    return pi->getOutput( outdx, idx );
}

ProcInst* ExtProc::getIdleInst( int nrsamples )
{
    ProcInst* pi = NULL;
    pD->idleinstslock_.lock();
    if (!pD->idleinsts_.isEmpty())
	pi = pD->idleinsts_.pop();
    pD->idleinstslock_.unLock();
    if (pi==NULL) {
	pi = new ProcInst();
	if (pi==NULL)
	    ErrMsg( "ExtProc::getIdleInst - error allocating new ProcInst" );
	else {
	    pD->startInst( pi );
	    pi->resize( nrsamples );
	}
    } else
	pi->resize( nrsamples );
    return pi;
}

void ExtProc::setInstIdle( ProcInst* pi )
{
    pD->idleinstslock_.lock();
    if (pi!=NULL) {
	pD->idleinsts_.push(pi);
    }
    pD->idleinstslock_.unLock();
}

bool ExtProc::compute( ProcInst* pi,  int z0, int inl, int crl)
{
    pD->isok_ = pi->compute( z0, inl, crl );
    return pD->isok_;
}

BufferStringSet ExtProc::getInputNames() const
{
    if (!hasInput() && !hasInputs())
	return BufferStringSet();

    const int numin = numInput();
    BufferStringSet res;
    for (int idx=0; idx<numin; idx++)
	res.add( inputName(idx));

    return res;
}

BufferStringSet ExtProc::getOutputNames() const
{
    if (!hasOutput())
	return BufferStringSet();

    const int numout = numOutput();
    BufferStringSet res;
    for (int idx=0; idx<numout; idx++)
	res.add( outputName(idx));

    return res;

}

bool ExtProc::hasInput() const
{
    return (pD->jsonpar_.GetType() != json::NULLVal && pD->jsonpar_.HasKey("Input"));
}

bool ExtProc::hasInputs() const
{
    return (pD->jsonpar_.GetType() != json::NULLVal && pD->jsonpar_.HasKey("Inputs"));
}

int ExtProc::numInput() const
{
    int nIn = 1;
    if (hasInputs()) {
	json::Array inarr = pD->jsonpar_["Inputs"].ToArray();
	nIn = inarr.size();
    }
    return nIn;
}

BufferString ExtProc::inputName(int inum ) const
{
    BufferString name;
    if (hasInputs()) {
	json::Array inarr = pD->jsonpar_["Inputs"].ToArray();
	int nIn = inarr.size();
	if (inum >= 0 && inum <nIn)
	    name = inarr[inum].ToString().c_str();
    } else if (hasInput())
	name = pD->jsonpar_["Input"].ToString().c_str();

    return name;
}

bool ExtProc::hasOutput() const
{
    return (pD->jsonpar_.GetType() != json::NULLVal && pD->jsonpar_.HasKey("Output"));
}

int ExtProc::numOutput() const
{
    int nOut = 1;
    if (hasOutput()) {
	json::Array outarr = pD->jsonpar_["Output"].ToArray();
	nOut = outarr.size();
    }
    return nOut;
}

BufferString ExtProc::outputName( int onum ) const
{
    BufferString name;
    if (hasOutput()) {
	json::Array outarr = pD->jsonpar_["Output"].ToArray();
	int nOut = outarr.size();
	if (onum >= 0 && onum <nOut)
	    name = outarr[onum].ToString().c_str();
    }
    return name;
}

bool ExtProc::hasZMargin() const
{
    return (pD->jsonpar_.GetType() != json::NULLVal && pD->jsonpar_.HasKey("ZSampMargin"));
}

bool ExtProc::hideZMargin() const
{
    if (hasZMargin()) {
	json::Object jobj = pD->jsonpar_["ZSampMargin"].ToObject();
	if (jobj.HasKey("Hidden"))
	    return jobj["Hidden"];
	else
	    return false;
    } else
	return true;
}

bool ExtProc::zSymmetric() const
{
    if (hasZMargin()) {
	json::Object jobj = pD->jsonpar_["ZSampMargin"].ToObject();
	if (jobj.HasKey("Symmetric"))
	    return jobj["Symmetric"];
	else
	    return false;
    } else
	return false;
}

Interval<int> ExtProc::z_minimum() const
{
    Interval<int> res;
    if (hasZMargin()) {
	json::Object jobj = pD->jsonpar_["ZSampMargin"].ToObject();
	if (jobj.HasKey("Minimum")) {
	    json::Array zmarr = jobj["Minimum"].ToArray();
	    int begin = zmarr[0];
	    int end = zmarr[1];
	    res = Interval<int>(begin, end);
	}
    }
    return res;
}

Interval<int> ExtProc::zmargin() const
{
    Interval<int> res;
    if (hasZMargin()) {
	json::Object jobj = pD->jsonpar_["ZSampMargin"].ToObject();
	if (jobj.HasKey("Value")) {
	    json::Array zmarr = jobj["Value"].ToArray();
	    int begin = zmarr[0];
	    int end = zmarr[1];
	    res = Interval<int>(begin, end);
	}
    }
    return res;
}

void ExtProc::setZMargin( Interval<int> g )
{
    json::Array garr;
    garr.insert(0, g.start);
    garr.insert(1, g.stop);
    json::Object jobj;
    jobj["Value"] = garr;
    pD->jsonpar_["ZSampMargin"] = jobj;
}

bool ExtProc::hasStepOut() const
{
    return (pD->jsonpar_.GetType() != json::NULLVal && pD->jsonpar_.HasKey("StepOut"));
}

bool ExtProc::hideStepOut() const
{
    if (hasStepOut()) {
	json::Object jobj = pD->jsonpar_["StepOut"].ToObject();
	if (jobj.HasKey("Hidden"))
	    return jobj["Hidden"];
	else
	    return false;
    } else
	return true;
}

bool ExtProc::so_same()
{
    if (hasStepOut()) {
	json::Object jobj = pD->jsonpar_["StepOut"].ToObject();
	if (jobj.HasKey("Same"))
	    return jobj["Same"];
	else
	    return false;
    } else
	return false;
}

BinID ExtProc::so_minimum()
{
    BinID res;
    if (hasStepOut()) {
	json::Object jobj = pD->jsonpar_["StepOut"].ToObject();
	if (jobj.HasKey("Minimum")) {
	    json::Array sarr = jobj["Minimum"].ToArray();
	    int inl = sarr[0];
	    int xln = sarr[1];
	    res = BinID(inl, xln);
	}
    }
    return res;
}

BinID ExtProc::stepout() const
{
    BinID res;
    if (hasStepOut()) {
	json::Object jobj = pD->jsonpar_["StepOut"].ToObject();
	if (jobj.HasKey("Value")) {
	    json::Array sarr = jobj["Value"].ToArray();
	    int inl = sarr[0];
	    int xln = sarr[1];
	    res = BinID(inl, xln);
	}
    }
    return res;
}

void ExtProc::setStepOut(BinID s)
{
    json::Array sarr;
    sarr.insert(0, s.inl());
    sarr.insert(1, s.crl());
    json::Object jobj;
    jobj["Value"] = sarr;
    pD->jsonpar_["StepOut"] = jobj;
}

bool ExtProc::hasSelect() const
{
    return (pD->jsonpar_.GetType() != json::NULLVal && pD->jsonpar_.HasKey("Select"));
}

int ExtProc::numSelect()
{
    int nsel = 0;
    if (hasSelect()) {
	json::Object jobj = pD->jsonpar_["Select"].ToObject();
	if (jobj.HasKey("Values")) {
	    json::Array selarr = jobj["Values"].ToArray();
	    nsel = selarr.size();
	}
    }
    return nsel;
}

BufferString ExtProc::selectName()
{
    BufferString name;
    if (hasSelect()) {
	json::Object jobj = pD->jsonpar_["Select"].ToObject();
	if (jobj.HasKey("Name"))
	    name = jobj["Name"].ToString().c_str();
    }
    return name;
}

BufferString ExtProc::selectOpt( int snum )
{
    BufferString name;
    if (hasSelect()) {
	json::Object jobj = pD->jsonpar_["Select"].ToObject();
	if (jobj.HasKey("Values")) {
	    json::Array selarr = jobj["Values"].ToArray();
	    int nsel = selarr.size();
	    if (snum>=0 && snum<nsel)
		name = selarr[snum].ToString().c_str();
	}
    }
    return name;
}

int ExtProc::selectValue()
{
    int val = 0;
    if (hasSelect()) {
        json::Object jobj = pD->jsonpar_["Select"].ToObject();
        if (jobj.HasKey("Selection"))
            val = jobj["Selection"];
    }
    return val;
}

void ExtProc::setSelection( int sel )
{
    json::Object jobj;
    jobj["Selection"] = sel;
    pD->jsonpar_["Select"] = jobj;
}

bool ExtProc::hasParam( int pnum ) const
{
    BufferString tmp("Par_");
    tmp += pnum;
    return (pD->jsonpar_.GetType() != json::NULLVal && pD->jsonpar_.HasKey(tmp.str()));
}

BufferString ExtProc::paramName( int pnum )
{
    BufferString name;
    if (hasParam(pnum)) {
	BufferString tmp("Par_");
	tmp += pnum;
	json::Object jobj = pD->jsonpar_[tmp.str()].ToObject();
	if (jobj.HasKey("Name"))
	    name = jobj["Name"].ToString().c_str();
    }
    return name;
}

float ExtProc::paramValue( int pnum )
{
    float val = mUdf(float);
    if (hasParam(pnum)) {
	BufferString tmp("Par_");
	tmp += pnum;
	json::Object jobj = pD->jsonpar_[tmp.str()].ToObject();
	if (jobj.HasKey("Value"))
	    val = jobj["Value"];
    }
    return val;
}

void ExtProc::setParam( int pnum, float val )
{
    BufferString tmp("Par_");
    tmp += pnum;
    json::Object jobj;
    jobj["Value"] = val;
    pD->jsonpar_[tmp.str()] = jobj;
}

bool ExtProc::hasHelp() const
{
    return (pD->jsonpar_.GetType() != json::NULLVal && pD->jsonpar_.HasKey("Help"));
}

BufferString ExtProc::helpValue() const
{
    BufferString res;
    if (hasHelp()) {
	res = pD->jsonpar_["Help"].ToString().c_str();
    }
    return res;
}

bool ExtProc::doParallel()
{
    if (pD->jsonpar_.GetType() != json::NULLVal && pD->jsonpar_.HasKey("Parallel"))
	return pD->jsonpar_["Parallel"];
    else
	return true;
}

BufferString ExtProc::getFile() const
{
    return pD->exfile_;
}

void ExtProc::setFile( const char* fname, const char* iname)
{
    pD->setFile(fname, iname);

}

bool ExtProc::hasNewParams() const
{
    return !pD->newparamkeys_.isEmpty();
}

BufferStringSet ExtProc::paramKeys() const
{
    return pD->newparamkeys_;
}

BufferString ExtProc::getParamType( const char* key) const
{
    BufferString keytype;
    if (pD->jsonpar_.GetType() != json::NULLVal && pD->jsonpar_.HasKey(key)
					    && pD->jsonpar_[key].HasKey("Type"))
	keytype = pD->jsonpar_[key]["Type"].ToString().c_str();

	return keytype;
}

float ExtProc::getParamFValue(const char* key, const char* subkey) const
{
    float val = mUdf(float);
    if (pD->jsonpar_.GetType() != json::NULLVal && pD->jsonpar_.HasKey(key)
					    && pD->jsonpar_[key].HasKey(subkey))
	val = pD->jsonpar_[key][subkey];

    return val;
}

void ExtProc::setParamFValue(const char* key, float val)
{
    if (pD->jsonpar_.GetType() != json::NULLVal && pD->jsonpar_.HasKey(key))
	pD->jsonpar_[key]["Value"] = val;
}

BufferString ExtProc::getParamStrValue(const char* key, const char* subkey) const
{
    BufferString val;
    if (pD->jsonpar_.GetType() != json::NULLVal && pD->jsonpar_.HasKey(key)
					    && pD->jsonpar_[key].HasKey(subkey))
	val = pD->jsonpar_[key][subkey].ToString().c_str();

    return val;
}

void ExtProc::setParamStrValue(const char* key, const char* val)
{
    if (pD->jsonpar_.GetType() != json::NULLVal && pD->jsonpar_.HasKey(key))
	pD->jsonpar_[key]["Value"] = val;
}

BufferStringSet ExtProc::getParamStrLstValue(const char* key, const char* subkey) const
{
    BufferStringSet res;
    if (pD->jsonpar_.GetType() != json::NULLVal && pD->jsonpar_.HasKey(key)
	&& pD->jsonpar_[key].HasKey(subkey) && pD->jsonpar_[key][subkey].GetType()==json::ArrayVal) {
	json::Array jarr = pD->jsonpar_[key][subkey].ToArray();
	for (auto& val : jarr)
	    res.add(val.ToString().c_str());
    }
    return res;
}

BufferString ExtProc::getParamsEncodedStr()
{
    json::Object jobj;
    for (auto* key : pD->newparamkeys_)
	jobj[key->str()] = pD->jsonpar_[key->str()];

    return BufferString(urllib::urlencode(json::Serialize(jobj)).c_str());
}

bool ExtProc::setParamsEncodedStr(const BufferString& encodedstr)
{
    json::Value jsonval = json::Deserialize(urllib::urldecode(std::string(encodedstr)));
    if (jsonval.GetType()!=json::NULLVal) {
	const json::Object jobj = jsonval.ToObject();
	for (auto it=jobj.begin(); it!=jobj.end(); ++it) {
	    const std::string key = it->first;
        if (it->second.HasKey("Value") && pD->jsonpar_.HasKey(key) && pD->jsonpar_[key].HasKey("Value"))
        {
            pD->jsonpar_[key]["Value"] = it->second["Value"];
        }
	}
	return true;
    } else
	return false;
}

