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
#include "envvars.h"
#include "errmsg.h"
#include "ranges.h"
#include "position.h"
#include "extproc.h"
#include "msgh.h"
#include "objectset.h"
#include "file.h"
#include "filepath.h"
#include "envvars.h"
#include "procinst.h"
#include "pythonaccess.h"
#include "uistringset.h"
#include "urllib.h"
#ifdef __win__
    #undef snprintf
    #undef strtoull
    #undef strtoll
#endif
#include "nlohmann/json.hpp"


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

using ordered_json = nlohmann::ordered_json;

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
    void		repError(const char*);

    bool		isok_;
    SeisInfo		seisinfo_;
    BufferString 	exfile_;
    BufferString	infile_raw_;
    BufferString	infile_;
    ordered_json	jsonpar_;
    ordered_json	metadata_;
    BufferStringSet	newparamkeys_;
    ObjectSet<ProcInst> idleinsts_;
    Threads::Mutex	idleinstslock_;
    uiRetVal		uirv_;
};

ExtProcImpl::ExtProcImpl(const char* fname, const char* iname)
:  idleinsts_(),isok_(false)
{
    setFile(fname, iname);
}

ExtProcImpl::~ExtProcImpl()
{
// Delete all ProcInst's in idleinsts_
    idleinstslock_.lock();
    while (!idleinsts_.isEmpty()) {
	ProcInst* pi = idleinsts_.pop();
	delete pi;
    }
    idleinstslock_.unLock();
}

void ExtProcImpl::setFile(const char* fname, const char* iname)
{
    exfile_ = fname;
    uirv_.setEmpty();
    isok_ = false;
    if (File::isFile(exfile_)) {
	infile_raw_ = iname;
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
	    repError("ExtProcImpl::setFile - interpreter setting is not a valid executable file");
    } else
	repError("ExtProcImpl::setFile - external attribute is not a valid file");

    isok_ = uirv_.isOK();
}

void ExtProcImpl::updateNewParamKeys()
{
    newparamkeys_.setEmpty();
    const BufferStringSet legacy(LegacyKeys);
    if (jsonpar_.is_object()) {
	for (auto it=jsonpar_.begin(); it!=jsonpar_.end(); it++)
	{
	    const BufferString key(it.key().c_str());
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
	    res.add(envname).add("^&");
	    res.add("set").add(BufferString("PYTHONPATH=", ExtProc::getPythonPath().cat(";"))).add("^&");
	    res.add(pyexe);
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
	    res.add(envname).add("^&");
	    res.add("set").add(BufferString("PYTHONPATH=", ExtProc::getPythonPath().cat(";"))).add("^&");
	    res.add(pyexe);
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
	const char* quote = "\"";
	str.insertAt(0, quote);
	str.add(quote);
    }
}

void ExtProcImpl::startInst( ProcInst* pi )
{
    ordered_json tmppar( jsonpar_ );
    for (auto& x : metadata_.items())
    {
	const std::string key = x.key();
	tmppar[key] = x.value();
    }

    BufferString params(tmppar.dump().c_str());
    BufferStringSet runargs;
    if ( !infile_.isEmpty() && !exfile_.isEmpty() )
	runargs = getInterpreterArgs();
    else if ( exfile_.isEmpty() ) {
	repError("ExtProcImpl::startInst - no external attribute file provided");
	return;
    }

    runargs.add(exfile_);
    runargs.add("-c");
    runargs.add(urllib::urlencode(params.str()).c_str());
    addQuotesIfNeeded(runargs);

    if (!pi->start( runargs, seisinfo_ ))
	repError("ExtProcImpl::startInst - run error");
}

void ExtProcImpl::repError(const char* msg)
{
    ErrMsg(msg);
    uirv_.add(toUiString(msg));
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
	repError("ExtProcImpl::getParam - no external attribute file provided");
	return false;
    }

    runargs.add(exfile_);
    runargs.add("-g");
    addQuotesIfNeeded(runargs);

    if (pi.start( runargs )) {
	params = pi.readAllStdOut();
	const int excode = pi.finish();
	if (excode != 0 ) {
	    BufferString msg("ExtProcImpl::getParam - external attribute exited abnormally - error code : ");
	    msg.add(excode);
	    repError(msg);
	    result = false;
	}
    } else {
	BufferString err("ExtProcImpl::getParam - run error for cmd: ", runargs.cat(" "));
	repError(err.buf());
	result = false;
    }
    if (result) {
	if (!params.isEmpty()) {
	    jsonpar_ = ordered_json::parse(urllib::urldecode(std::string(params.str())));
	    if (jsonpar_.is_null()) {
		repError("ExtProcImpl::getParam - parameter output of external attribute is not valid JSON");
		if (!params.isEmpty())
		    repError(params);
		result = false;
	    }
	} else {
	    repError("ExtProcImpl::getParam - no JSON parameter string returned");
	    result = false;
	}
    }
    uirv_.add(pi.errMsg());
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

bool ExtProc::isOK() const
{
    return pD->isok_ && pD->uirv_.isOK();
}

void ExtProc::setSeisInfo( int ninl, int ncrl, float inlDist, float crlDist,
			   float zFactor, float dipFactor, int nrZ )
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
    (pD->seisinfo_).nrZ = nrZ;
}

void ExtProc::addMetadata( const char* key, const char* value )
{
    pD->metadata_[key] = value;
}

void ExtProc::addMetadata( const char* key, const BufferStringSet& values )
{
    ordered_json jsarr = ordered_json::array();
    for ( const auto* val : values )
	jsarr.push_back( val->str() );

    pD->metadata_[key] = jsarr;
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
	    pD->repError( "ExtProc::getIdleInst - error allocating new ProcInst" );
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
    return (!pD->jsonpar_.is_null() && pD->jsonpar_.contains("Input"));
}

bool ExtProc::hasInputs() const
{
    return (!pD->jsonpar_.is_null() && pD->jsonpar_.contains("Inputs"));
}

int ExtProc::numInput() const
{
    int nIn = 1;
    if (hasInputs() && pD->jsonpar_["Inputs"].is_array()) {
	ordered_json inarr = pD->jsonpar_["Inputs"];
	nIn = inarr.size();
    }
    return nIn;
}

BufferString ExtProc::inputName(int inum ) const
{
    BufferString name;
    if (hasInputs()) {
	ordered_json inarr = pD->jsonpar_["Inputs"];
	int nIn = inarr.size();
	if (inum >= 0 && inum <nIn)
	    name = inarr[inum].template get<std::string>().c_str();
    } else if (hasInput())
	name = pD->jsonpar_["Input"].template get<std::string>().c_str();

    return name;
}

bool ExtProc::hasOutput() const
{
    return (!pD->jsonpar_.is_null() && pD->jsonpar_.contains("Output"));
}

int ExtProc::numOutput() const
{
    int nOut = 1;
    if (hasOutput()) {
	ordered_json outarr = pD->jsonpar_["Output"];
	nOut = outarr.size();
    }
    return nOut;
}

BufferString ExtProc::outputName( int onum ) const
{
    BufferString name;
    if (hasOutput() && pD->jsonpar_["Output"].is_array()) {
	ordered_json outarr = pD->jsonpar_["Output"];
	int nOut = outarr.size();
	if (onum >= 0 && onum <nOut)
	    name = outarr[onum].template get<std::string>().c_str();
    }
    return name;
}

bool ExtProc::hasZMargin() const
{
    return (!pD->jsonpar_.is_null() && pD->jsonpar_.contains("ZSampMargin"));
}

bool ExtProc::hideZMargin() const
{
    if (hasZMargin()) {
	ordered_json jobj = pD->jsonpar_["ZSampMargin"];
	if (jobj.is_object() && jobj.contains("Hidden"))
	    return jobj["Hidden"];
	else
	    return false;
    } else
	return true;
}

bool ExtProc::zSymmetric() const
{
    if (hasZMargin()) {
	ordered_json jobj = pD->jsonpar_["ZSampMargin"];
	if (jobj.is_object() && jobj.contains("Symmetric"))
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
	ordered_json jobj = pD->jsonpar_["ZSampMargin"];
	if (jobj.is_object() && jobj.contains("Minimum") && jobj["Minimum"].is_array()) {
	    ordered_json zmarr = jobj["Minimum"];
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
	ordered_json jobj = pD->jsonpar_["ZSampMargin"];
	if (jobj.is_object() && jobj.contains("Value") && jobj["Value"].is_array()) {
	    ordered_json zmarr = jobj["Value"];
	    int begin = zmarr[0];
	    int end = zmarr[1];
	    res = Interval<int>(begin, end);
	}
    }
    return res;
}

void ExtProc::setZMargin( Interval<int> g )
{
    ordered_json garr = ordered_json::array({g.start, g.stop});
    ordered_json jobj = ordered_json::object({{"Value", garr}});
    pD->jsonpar_["ZSampMargin"] = jobj;
}

bool ExtProc::hasStepOut() const
{
    return (!pD->jsonpar_.is_null() && pD->jsonpar_.contains("StepOut"));
}

bool ExtProc::hideStepOut() const
{
    if (hasStepOut()) {
	ordered_json jobj = pD->jsonpar_["StepOut"];
	if (jobj.is_object() && jobj.contains("Hidden"))
	    return jobj["Hidden"];
	else
	    return false;
    } else
	return true;
}

bool ExtProc::so_same()
{
    if (hasStepOut()) {
	ordered_json jobj = pD->jsonpar_["StepOut"];
	if (jobj.is_object() && jobj.contains("Same"))
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
	ordered_json jobj = pD->jsonpar_["StepOut"];
	if (jobj.is_object() && jobj.contains("Minimum") && jobj["Minimum"].is_array()) {
	    ordered_json sarr = jobj["Minimum"];
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
	ordered_json jobj = pD->jsonpar_["StepOut"];
	if (jobj.is_object() && jobj.contains("Value") && jobj["Value"].is_array()) {
	    ordered_json sarr = jobj["Value"];
	    int inl = sarr[0];
	    int xln = sarr[1];
	    res = BinID(inl, xln);
	}
    }
    return res;
}

void ExtProc::setStepOut(BinID s)
{
    ordered_json sarr = ordered_json::array({s.inl(), s.crl()});
    ordered_json jobj = ordered_json::object({{"Value", sarr}});
    pD->jsonpar_["StepOut"] = jobj;
}

bool ExtProc::hasSelect() const
{
    return (!pD->jsonpar_.is_null() && pD->jsonpar_.contains("Select"));
}

int ExtProc::numSelect()
{
    int nsel = 0;
    if (hasSelect()) {
	ordered_json jobj = pD->jsonpar_["Select"];
	if (jobj.is_object() && jobj.contains("Values") && jobj["Values"].is_array()) {
	    ordered_json selarr = jobj["Values"];
	    nsel = selarr.size();
	}
    }
    return nsel;
}

BufferString ExtProc::selectName()
{
    BufferString name;
    if (hasSelect()) {
	ordered_json jobj = pD->jsonpar_["Select"];
	if (jobj.is_object() && jobj.contains("Name"))
	    name = jobj["Name"].template get<std::string>().c_str();
    }
    return name;
}

BufferString ExtProc::selectOpt( int snum )
{
    BufferString name;
    if (hasSelect()) {
	ordered_json jobj = pD->jsonpar_["Select"];
	if (jobj.is_object() && jobj.contains("Values") && jobj["Values"].is_array()) {
	    ordered_json selarr = jobj["Values"];
	    int nsel = selarr.size();
	    if (snum>=0 && snum<nsel)
		name = selarr[snum].template get<std::string>().c_str();
	}
    }
    return name;
}

int ExtProc::selectValue()
{
    int val = 0;
    if (hasSelect()) {
        ordered_json jobj = pD->jsonpar_["Select"];
        if (jobj.contains("Selection"))
            val = jobj["Selection"];
    }
    return val;
}

void ExtProc::setSelection( int sel )
{
    ordered_json jobj = ordered_json::object({{"Selection", sel}});
    pD->jsonpar_["Select"] = jobj;
}

bool ExtProc::hasParam( int pnum ) const
{
    BufferString tmp("Par_");
    tmp += pnum;
    return (!pD->jsonpar_.is_null() && pD->jsonpar_.contains(tmp.str()));
}

BufferString ExtProc::paramName( int pnum )
{
    BufferString name;
    if (hasParam(pnum)) {
	BufferString tmp("Par_");
	tmp += pnum;
	ordered_json jobj = pD->jsonpar_[tmp.str()];
	if (jobj.is_object() && jobj.contains("Name"))
	    name = jobj["Name"].template get<std::string>().c_str();
    }
    return name;
}

float ExtProc::paramValue( int pnum )
{
    float val = mUdf(float);
    if (hasParam(pnum)) {
	BufferString tmp("Par_");
	tmp += pnum;
	ordered_json jobj = pD->jsonpar_[tmp.str()];
	if (jobj.is_object() && jobj.contains("Value"))
	    val = jobj["Value"];
    }
    return val;
}

void ExtProc::setParam( int pnum, float val )
{
    BufferString tmp("Par_");
    tmp += pnum;
    ordered_json jobj = ordered_json::object({{"Value", val}});
    pD->jsonpar_[tmp.str()] = jobj;
}

bool ExtProc::hasHelp() const
{
    return (!pD->jsonpar_.is_null() && pD->jsonpar_.contains("Help"));
}

BufferString ExtProc::helpValue() const
{
    BufferString res;
    if (hasHelp()) {
	res = pD->jsonpar_["Help"].template get<std::string>().c_str();
    }
    return res;
}

bool ExtProc::doParallel()
{
    if (!pD->jsonpar_.is_null() && pD->jsonpar_.contains("Parallel"))
	return pD->jsonpar_["Parallel"];
    else
	return true;
}

int ExtProc::minTaskSize() const
{
    if (!pD->jsonpar_.is_null() && pD->jsonpar_.contains("MinSamplesPerThread"))
	return pD->jsonpar_["MinSamplesPerThread"];
    else
	return 40;
}

BufferString ExtProc::getFile() const
{
    return pD->exfile_;
}

BufferString ExtProc::getInterpStr() const
{
    return pD->infile_raw_;
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
    if (!pD->jsonpar_.is_null() && pD->jsonpar_.contains(key)
					    && pD->jsonpar_[key].contains("Type"))
	keytype = pD->jsonpar_[key]["Type"].template get<std::string>().c_str();

	return keytype;
}

float ExtProc::getParamFValue(const char* key, const char* subkey) const
{
    float val = mUdf(float);
    if (!pD->jsonpar_.is_null() && pD->jsonpar_.contains(key)
					    && pD->jsonpar_[key].contains(subkey))
	val = pD->jsonpar_[key][subkey];

    return val;
}

void ExtProc::setParamFValue(const char* key, float val)
{
    if (!pD->jsonpar_.is_null() && pD->jsonpar_.contains(key))
	pD->jsonpar_[key]["Value"] = val;
}

BufferString ExtProc::getParamStrValue(const char* key, const char* subkey) const
{
    BufferString val;
    if (!pD->jsonpar_.is_null() && pD->jsonpar_.contains(key)
					    && pD->jsonpar_[key].contains(subkey))
	val = pD->jsonpar_[key][subkey].template get<std::string>().c_str();

    return val;
}

void ExtProc::setParamStrValue(const char* key, const char* val)
{
    if (!pD->jsonpar_.is_null() && pD->jsonpar_.contains(key))
	pD->jsonpar_[key]["Value"] = val;
}

BufferStringSet ExtProc::getParamStrLstValue(const char* key, const char* subkey) const
{
    BufferStringSet res;
    if (!pD->jsonpar_.is_null() && pD->jsonpar_.contains(key)
	&& pD->jsonpar_[key].contains(subkey) && pD->jsonpar_[key][subkey].is_array()) {
	ordered_json jarr = pD->jsonpar_[key][subkey];
	for (auto& val : jarr)
	    res.add(val.template get<std::string>().c_str());
    }
    return res;
}

BufferString ExtProc::getParamsEncodedStr()
{
    ordered_json jobj = ordered_json::object();
    for (auto* key : pD->newparamkeys_)
	jobj[key->str()] = pD->jsonpar_[key->str()];

    const std::string str = jobj.dump();
    return BufferString(urllib::urlencode(str).c_str());
}

bool ExtProc::setParamsEncodedStr(const BufferString& encodedstr)
{
    ordered_json jsonval = ordered_json::parse(urllib::urldecode(std::string(encodedstr)));
    if (!jsonval.is_null() && jsonval.is_object()) {
	for (auto&  x : jsonval.items()) {
	    const std::string key = x.key();
	    if (x.value().contains("Value") && pD->jsonpar_.contains(key) && pD->jsonpar_[key].contains("Value"))
	    {
		pD->jsonpar_[key]["Value"] = x.value()["Value"];
	    }
	}
	return true;
    } else
	return false;
}

uiRetVal ExtProc::errMsg() const
{
    return pD->uirv_;
}

BufferStringSet ExtProc::getPythonPath()
{
    BufferStringSet pypathset = OD::PythA().getBasePythonPath();
    BufferString dnm = GetEnvVar("OD_APPL_PLUGIN_DIR");
    if (!dnm.isEmpty())
    {
	FilePath fp(dnm, "bin", "python");
	if (fp.exists())
	    pypathset.insertAt(new BufferString(fp.fullPath()), 0);
    }

    dnm = GetEnvVar("OD_USER_PLUGIN_DIR");
    if (!dnm.isEmpty())
    {
	FilePath fp(dnm, "bin", "python");
	if (fp.exists())
	    pypathset.insertAt(new BufferString(fp.fullPath()), 0);
    }

    return pypathset;
}
