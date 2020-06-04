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

struct ExtProcImpl
{
public:
	ExtProcImpl(const char* fname, const char* iname);
	~ExtProcImpl();
	
	void			startInst( ProcInst* pi );
	
	bool			getParam();
	void			setFile(const char* fname, const char* iname);
	BufferStringSet getInterpreterArgs() const;
	void			addQuotesIfNeeded(BufferStringSet& args);
	
	bool			isOK;
	SeisInfo		seisInfo;
	BufferString 	exfile;
	BufferString	infile;
	json::Value		jsonpar;
	ObjectSet<ProcInst> idleInsts;
	Threads::Mutex	idleInstsLock;
	
};

ExtProcImpl::ExtProcImpl(const char* fname, const char* iname)
:  idleInsts(),isOK(true)
{
	setFile(fname, iname);
}
	
ExtProcImpl::~ExtProcImpl()
{	
// Delete all ProcInst's in idleInsts
	while (!idleInsts.isEmpty()) {
		ProcInst* pi = idleInsts.pop();
		delete pi;
	}
}

void ExtProcImpl::setFile(const char* fname, const char* iname)
{
	exfile = fname;
	if (File::isFile(exfile)) {
		infile = BufferString(iname).trimBlanks();
		if (infile.startsWith("%") && infile.endsWith("%")) {
			BufferString tmp = infile.unEmbed('%','%');
			infile = GetEnvVar(tmp);
		}
		if (File::isExecutable(infile))
			getParam();
		else
			ErrMsg("ExtProcImpl::setFile - interpreter setting is not a valid executable file");
	} else
		ErrMsg("ExtProcImpl::setFile - external attribute is not a valid file");
	return;
}

BufferStringSet ExtProcImpl::getInterpreterArgs() const
{
	BufferStringSet res;
	if (infile.isEmpty())
		return res;

#ifdef __win__
	BufferString pythonstr(sKey::Python()); 
	pythonstr.toLower();
	if ( infile.find(pythonstr) && infile.find( "envs" ) ) {
// Must be an Anaconda environment
		BufferString pyexe = FilePath(infile).fileName();
		FilePath envpath = FilePath(infile).pathOnly();
		BufferString envname = envpath.fileName();
		FilePath envspath(envpath.pathOnly());
		if (envspath.fileName() == "envs") {
			FilePath anaconda_root = FilePath(envspath.pathOnly());
			FilePath anaconda_activate = anaconda_root.add("Scripts").add("activate.bat");
			res.add(anaconda_activate.fullPath()).add(envname).add("^&").add(pyexe);
		} else
			res.add(infile);
	} else if (infile.find(pythonstr)) {
		FilePath root(FilePath(infile).pathOnly());
		if (root.add("envs").exists()) {
// Probably an Anaconda base environment
			BufferString envname("base");
			BufferString pyexe = FilePath(infile).fileName();
			FilePath anaconda_root = FilePath(infile).pathOnly();
			FilePath anaconda_activate = anaconda_root.add("Scripts").add("activate.bat");
			res.add(anaconda_activate.fullPath()).add(envname).add("^&").add(pyexe);
		}
		else
			res.add(infile);
	} else
		res.add(infile);
#else
	res.add(infile);
#endif
	return res;
}

void ExtProcImpl::addQuotesIfNeeded(BufferStringSet& args)
{
	for (int idx = 0; idx < args.size(); idx++) {
		BufferString& str = args.get(idx);
		if (!str.find(' '))
			continue;
		if (str[0] == '"')
			continue;
		const char* quote = "\"";
		str.insertAt(0, quote);
		str.add(quote);
	}
}

void ExtProcImpl::startInst( ProcInst* pi )
{
	BufferString params(json::Serialize(jsonpar).c_str());
#ifdef __win__
	params.replace("\"","\"\""); 
	params.embed('"','"');
#endif
	BufferStringSet runargs;
	if ( !infile.isEmpty() && !exfile.isEmpty() )
		runargs = getInterpreterArgs();
	else if ( exfile.isEmpty() ) {
		ErrMsg("ExtProcImpl::startInst - no external attribute file provided");
		return;
	}

	runargs.add(exfile);
	runargs.add("-c");
	runargs.add(params);
	addQuotesIfNeeded(runargs);

	if (!pi->start( runargs, seisInfo ))
	{
		ErrMsg("ExtProcImpl::startInst - run error");
	}
	return;
}

bool ExtProcImpl::getParam()
{
	ProcInst pi;
	bool result = true;
	BufferString params;
	BufferStringSet runargs;
	if (!infile.isEmpty() && !exfile.isEmpty())
		runargs = getInterpreterArgs();
	else if (exfile.isEmpty()) {
		ErrMsg("ExtProcImpl::startInst - no external attribute file provided");
		return false;
	}

	runargs.add(exfile);
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
		jsonpar = json::Deserialize(params.str());
		if (jsonpar.GetType() == json::NULLVal) {
			ErrMsg("ExtProcImpl::getParam - parameter output of external attribute is not valid JSON");
			if (!params.isEmpty())
				UsrMsg(params);
			result = false;
		}
	}
	isOK = result;
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
	return pD->isOK;
}

void ExtProc::setSeisInfo( int ninl, int ncrl, float inlDist, float crlDist, float zFactor, float dipFactor )
{
	(pD->seisInfo).nrTraces = ninl*ncrl;
	(pD->seisInfo).nrInl = ninl;
	(pD->seisInfo).nrCrl = ncrl;
	(pD->seisInfo).zStep = SI().zStep();
	(pD->seisInfo).inlDistance = inlDist;
	(pD->seisInfo).crlDistance = crlDist;
	(pD->seisInfo).zFactor = zFactor;
	(pD->seisInfo).dipFactor = dipFactor;
	(pD->seisInfo).nrOutput = numOutput();
	(pD->seisInfo).nrInput = numInput();
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
	pD->idleInstsLock.lock();
	if (!pD->idleInsts.isEmpty())
		pi = pD->idleInsts.pop();
	pD->idleInstsLock.unLock();
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
	pD->idleInstsLock.lock();
	if (pi!=NULL) {
		pD->idleInsts.push(pi);
	}
	pD->idleInstsLock.unLock();
}

bool ExtProc::compute( ProcInst* pi,  int z0, int inl, int crl)
{
	pD->isOK = pi->compute( z0, inl, crl );
	return pD->isOK;
}

bool ExtProc::hasInput()
{
	return (pD->jsonpar.GetType() != json::NULLVal && pD->jsonpar.HasKey("Input"));
}

bool ExtProc::hasInputs()
{
	return (pD->jsonpar.GetType() != json::NULLVal && pD->jsonpar.HasKey("Inputs"));
}

int ExtProc::numInput()
{
	int nIn = 1;
	if (hasInputs()) {
		json::Array inarr = pD->jsonpar["Inputs"].ToArray();
		nIn = inarr.size();
	}
	return nIn;
}

BufferString ExtProc::inputName(int inum )
{
	BufferString name;
	if (hasInputs()) {
		json::Array inarr = pD->jsonpar["Inputs"].ToArray();
		int nIn = inarr.size();
		if (inum >= 0 && inum <nIn)
			name = inarr[inum].ToString().c_str();
	} else if (hasInput()) {
		name = pD->jsonpar["Input"].ToString().c_str();
	}
	return name;
}

bool ExtProc::hasOutput()
{
	return (pD->jsonpar.GetType() != json::NULLVal && pD->jsonpar.HasKey("Output"));
}

int ExtProc::numOutput()
{
	int nOut = 1;
	if (hasOutput()) {
		json::Array outarr = pD->jsonpar["Output"].ToArray();
		nOut = outarr.size();
	}
	return nOut;
}

BufferString ExtProc::outputName( int onum )
{
	BufferString name;
	if (hasOutput()) {
		json::Array outarr = pD->jsonpar["Output"].ToArray();
		int nOut = outarr.size();
		if (onum >= 0 && onum <nOut)
			name = outarr[onum].ToString().c_str();
	}
	return name;
}

bool ExtProc::hasZMargin()
{
	return (pD->jsonpar.GetType() != json::NULLVal && pD->jsonpar.HasKey("ZSampMargin"));
		 
}

bool ExtProc::hideZMargin()
{
	if (hasZMargin()) {
		json::Object jobj = pD->jsonpar["ZSampMargin"].ToObject();
		if (jobj.HasKey("Hidden"))
			return jobj["Hidden"];
		else
			return false;
	} else
		return true;
}

bool ExtProc::zSymmetric()
{
	if (hasZMargin()) {
		json::Object jobj = pD->jsonpar["ZSampMargin"].ToObject();
		if (jobj.HasKey("Symmetric"))
			return jobj["Symmetric"];
		else
			return false;
	} else
		return false;
}

Interval<int> ExtProc::z_minimum()
{
	Interval<int> res;
	if (hasZMargin()) {
		json::Object jobj = pD->jsonpar["ZSampMargin"].ToObject();
		if (jobj.HasKey("Minimum")) {
			json::Array zmarr = jobj["Minimum"].ToArray();
			int begin = zmarr[0];
			int end = zmarr[1];
			res = Interval<int>(begin, end);
		}
	}
	return res;
}

Interval<int> ExtProc::zmargin()
{
	Interval<int> res;
	if (hasZMargin()) {
		json::Object jobj = pD->jsonpar["ZSampMargin"].ToObject();
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
	pD->jsonpar["ZSampMargin"] = jobj;
}

bool ExtProc::hasStepOut()
{
	return (pD->jsonpar.GetType() != json::NULLVal && pD->jsonpar.HasKey("StepOut"));
}

bool ExtProc::hideStepOut()
{
	if (hasStepOut()) {
		json::Object jobj = pD->jsonpar["StepOut"].ToObject();
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
		json::Object jobj = pD->jsonpar["StepOut"].ToObject();
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
		json::Object jobj = pD->jsonpar["StepOut"].ToObject();
		if (jobj.HasKey("Minimum")) {
			json::Array sarr = jobj["Minimum"].ToArray();
			int inl = sarr[0];
			int xln = sarr[1];
			res = BinID(inl, xln);
		}
	}
	return res;
}

BinID ExtProc::stepout()
{
	BinID res;
	if (hasStepOut()) {
		json::Object jobj = pD->jsonpar["StepOut"].ToObject();
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
	pD->jsonpar["StepOut"] = jobj;
}

bool ExtProc::hasSelect()
{
	return (pD->jsonpar.GetType() != json::NULLVal && pD->jsonpar.HasKey("Select"));
}

int ExtProc::numSelect()
{
	int nsel = 0;
	if (hasSelect()) {
		json::Object jobj = pD->jsonpar["Select"].ToObject();
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
		json::Object jobj = pD->jsonpar["Select"].ToObject();
		if (jobj.HasKey("Name")) 
			name = jobj["Name"].ToString().c_str();
	}
	return name;
}

BufferString ExtProc::selectOpt( int snum )
{
	BufferString name;
	if (hasSelect()) {
		json::Object jobj = pD->jsonpar["Select"].ToObject();
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
        json::Object jobj = pD->jsonpar["Select"].ToObject();
        if (jobj.HasKey("Selection"))
            val = jobj["Selection"];
    }
    return val;
}

void ExtProc::setSelection( int sel )
{
    json::Object jobj;
    jobj["Selection"] = sel;
    pD->jsonpar["Select"] = jobj;    
}

bool ExtProc::hasParam( int pnum )
{
	BufferString tmp("Par_");
	tmp += pnum;
	return (pD->jsonpar.GetType() != json::NULLVal && pD->jsonpar.HasKey(tmp.str()));
}

BufferString ExtProc::paramName( int pnum )
{
	BufferString name;
	if (hasParam(pnum)) {
		BufferString tmp("Par_");
		tmp += pnum;
		json::Object jobj = pD->jsonpar[tmp.str()].ToObject();
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
		json::Object jobj = pD->jsonpar[tmp.str()].ToObject();
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
	pD->jsonpar[tmp.str()] = jobj;
}

bool ExtProc::hasHelp()
{
	return (pD->jsonpar.GetType() != json::NULLVal && pD->jsonpar.HasKey("Help"));
}

BufferString ExtProc::helpValue()
{
	BufferString res;
	if (hasHelp()) {
		res = pD->jsonpar["Help"].ToString().c_str();
	}
	return res;
}

bool ExtProc::doParallel()
{
	if (pD->jsonpar.GetType() != json::NULLVal && pD->jsonpar.HasKey("Parallel")) 
			return pD->jsonpar["Parallel"];
	else
			return true;
}
	
BufferString ExtProc::getFile()
{
	return pD->exfile;
}

void ExtProc::setFile( const char* fname, const char* iname)
{
		pD->setFile(fname, iname);
	
}
