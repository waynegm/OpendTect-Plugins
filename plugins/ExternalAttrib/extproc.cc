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
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <paths.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "survinfo.h"
#include "errmsg.h"
#include "ranges.h"
#include "position.h"
#include "extproc.h"
#include "json.h"
#include "msgh.h"

struct SeisInfo
{
	int		nrTraces;
	int		nrInl;
	int		nrCrl;
	float	zStep;
	float	inlDistance;
	float	crlDistance;
};

struct TrcInfo
{
	int		nrSamples;
	int		z0;
	int		inl;
	int		crl;
};

struct ExtProcImpl
{
public:
	ExtProcImpl(const char* fname);
	~ExtProcImpl();
	
	bool			start( char *const argv[] );
	int				finish();
	
		
	bool			writeSeisInfo();
	bool			writeTrcInfo( int z0, int inl, int crl );
	bool			writeData();
	bool			readData();
	BufferString	readAllStdOut();
	
	bool			getParam();
	void			setFile(const char* fname);
	
	float*			input;
	float*			output;
	int				nrSamples;
	int				nrTraces;
	SeisInfo		seisInfo;
	TrcInfo			trcInfo;
	FILE*			read_fd;
	FILE*			write_fd;
	pid_t			child_pid;

	BufferString 	exfile;
	json::Value		jsonpar;
	
	
};

ExtProcImpl::ExtProcImpl(const char* fname)
:  input(NULL), output(NULL), read_fd(NULL), write_fd(NULL), child_pid(-1)
{
	nrSamples = 0;
	nrTraces = 0;
	setFile(fname);
}
	
ExtProcImpl::~ExtProcImpl()
{	
	if (child_pid != -1)
		finish();
	if (input != NULL)
		delete [] input;
	if (output != NULL)
		delete [] output;
}

void ExtProcImpl::setFile(const char* fname)
{
	exfile = fname;
	getParam();
}

bool ExtProcImpl::start( char *const argv[] )
{
	char* envp[3];
	envp[0] = (char*) "IFS= \t\n";
	envp[1] = (char*) "PATH=" _PATH_STDPATH;
	envp[2] = 0;
	
	if (child_pid != -1)
	{
		ErrMsg("ExtProcImpl::start - already in use");
		return false;
	}
	int		stdin_pipe[2], stdout_pipe[2];
	
	if (pipe(stdin_pipe) == -1) {
		return false;
	}
	if (pipe(stdout_pipe) == -1) {
		close(stdin_pipe[1]);
		close(stdin_pipe[0]);
		return false;
	}
	
	if (!(read_fd = fdopen(stdout_pipe[0], "r"))) {
		read_fd = NULL;
		write_fd = NULL;
		close(stdout_pipe[1]);
		close(stdout_pipe[0]);
		close(stdin_pipe[1]);
		close(stdin_pipe[0]);
		ErrMsg("Open read_fd failed");
		return false;
	}
	if (!(write_fd = fdopen(stdin_pipe[1], "w"))) {
		fclose(read_fd);
		read_fd = NULL;
		write_fd = NULL;
		close(stdout_pipe[1]);
		
		close(stdin_pipe[1]);
		close(stdin_pipe[0]);
		return false;
	}
	setbuf( read_fd, NULL );
	setbuf( write_fd, NULL);
	if ((child_pid = fork()) == -1) {
		fclose(write_fd);
		fclose(read_fd);
		read_fd = NULL;
		write_fd = NULL;
		close(stdout_pipe[1]);
		close(stdin_pipe[0]);
		return false;
	}
	
	if (!child_pid) {
//	This is the child process
//	close open files other than stdin, stdout and stderr
		close(stdout_pipe[0]);
		close(stdin_pipe[1]);
		if (stdin_pipe[0] != 0) {
			dup2(stdin_pipe[0], 0);
			close(stdin_pipe[0]);
		}
		if (stdout_pipe[1] != 1) {
			dup2(stdout_pipe[1], 1);
			close(stdout_pipe[1]);
		}
		int fd, fds;
		if ((fds = getdtablesize()) == -1 )
			fds = FOPEN_MAX;
		for (fd = 3; fd<fds; fd++)
			close(fd);
		execve(argv[0], argv, envp);
//	Should never get here
		ErrMsg("ExtProcImpl::start - child process not started");
		return false;
	}
	
	close(stdout_pipe[1]);
	close(stdin_pipe[0]);
	return true;
}

int ExtProcImpl::finish() {
	int   status;
	pid_t pid;
	
	kill( child_pid, SIGKILL );
	if (child_pid != -1) {
		do {
			pid = waitpid(child_pid, &status, 0);
		} while (pid == -1 && errno == EINTR);
	}
	if (write_fd) {
		fclose(write_fd);
		write_fd = NULL;
	}
	if (read_fd) {
		fclose(read_fd);
		read_fd = NULL;
	}
	child_pid=-1;
	if (pid != -1 && WIFEXITED(status)) return WEXITSTATUS(status);
	else return (pid == -1 ? -1 : 0);
}	
	
bool ExtProcImpl::writeSeisInfo()
{
	if (write_fd) {
		seisInfo.nrTraces = nrTraces;
		size_t nbytes = sizeof(seisInfo);
		size_t res = fwrite((void*) &seisInfo, nbytes, 1, write_fd);
		if (res != 1) {
			ErrMsg("ExtProcImpl::writeSeisInfo - error writing info block to external attribute");
			return false;
		}
		fflush(write_fd);
		return true;
	} else {
		ErrMsg("ExtProcImpl::writeSeisInfo - no stdin");
		return false;
	}
}

bool ExtProcImpl::writeTrcInfo(int z0, int inl, int crl)
{
	if (write_fd) {
		trcInfo.z0 = z0;
		trcInfo.inl = inl;
		trcInfo.crl = crl;
		trcInfo.nrSamples = nrSamples;
		
		size_t nbytes = sizeof(trcInfo);
		size_t res = fwrite((void*) &trcInfo, nbytes, 1, write_fd);
		if (res != 1) {
			ErrMsg("ExtProcImpl::writeTrcInfo - error writing info block to external attribute");
			return false;
		}
		fflush(write_fd);
		return true;
	} else {
		ErrMsg("ExtProcImpl::writeTrcInfo - no stdin");
		return false;
	}
}
	
bool ExtProcImpl::writeData()
{ 
	if (write_fd) {
		size_t nbytes = sizeof(float);
		size_t res = fwrite((void*) input, nbytes, nrSamples*nrTraces, write_fd);
		if (res != nrSamples*nrTraces) {
			ErrMsg("ExtProcImpl::writeData - error writing data to external attribute");
			return false;
		}
		fflush(write_fd);
		return true;
	} else {
		ErrMsg("ExtProcImpl::writeData - no stdin");
		return false;
	}
}

bool ExtProcImpl::readData()
{
	if (read_fd) {
		size_t nbytes = sizeof(float);
		size_t res = fread((void*) output, nbytes, nrSamples, read_fd);
		if (res != nrSamples) {
			ErrMsg("ExtProcImpl::readData - error reading from external attribute");
			return false;
		}
		return true;	
		
	} else {
		ErrMsg("ExtProcImpl::readData - no stdout");	
		
		return false;
	}
}

BufferString ExtProcImpl::readAllStdOut()
{
	BufferString result;
	if (read_fd) {
		char buffer[4096];
		while (1) {
			ssize_t count = fread( (void*) buffer, 1, sizeof(buffer-1), read_fd );
			if (count == -1) {
				if (errno == EINTR) 
					continue;
				else {
					ErrMsg("ExtProcImpl::readAllStdOut- read error");
					return BufferString();
				}
			} else if (count == 0) {
				break;
			} else {
				buffer[count] = 0;
				result += buffer;
			}
		}
	} else
		ErrMsg("ExtProcImpl::readAllStdOut - no stdout");
	
	return result;
}

bool ExtProcImpl::getParam()
{
	BufferString params;
	char* args[3];
	args[0] = (char*) exfile.str();
	args[1] = (char*) "-g";
	args[2] = 0;
	
	if (start( args )) {
		params = readAllStdOut();
		finish();
	} else {
		ErrMsg("ExtProcImpl::getParam - run error"); 
		return false;
	}
	jsonpar = json::Deserialize(params.str());
	if (jsonpar.GetType() == json::NULLVal) {
		ErrMsg("ExtProcImpl::getParam - parameter output of external attribute is not valid JSON");
		ErrMsg(params.str());
		return false;
	}
	return true;
}

ExtProc::ExtProc( const char* fname ) 
: pD(new ExtProcImpl(fname))
{ 
}

ExtProc::~ExtProc()
{ 
	delete pD;
}

	
void ExtProc::start(int ninl, int ncrl)
{
	if (isBusy()) {
		ErrMsg("ExtProc::start - error process is already busy");
		return;
	}
	(pD->seisInfo).nrInl = ninl;
	(pD->seisInfo).nrCrl = ncrl;
	(pD->seisInfo).zStep = SI().zStep();
	(pD->seisInfo).inlDistance = (float) SI().inlDistance();
	(pD->seisInfo).crlDistance = (float) SI().crlDistance();
	pD->nrTraces = ninl*ncrl;
	(pD->seisInfo).nrTraces = ninl*ncrl;
	
	BufferString params(json::Serialize(pD->jsonpar).c_str());
	char* args[4];
	args[0] = (char*) pD->exfile.str();
	args[1] = (char*) "-c";
	args[2] = (char*) params.str();
	args[3] = 0;

	if (!pD->start( args ))
		ErrMsg("ExtProc::start - run error");

	pD->writeSeisInfo();
	return;
}

bool ExtProc::isBusy() const
{
	return pD->child_pid != -1;
}

void ExtProc::setInput( int inpdx, int idx, float val )
{
//	int pos = inpdx*(pD->info).nrSamples + idx;
	int pos = idx;
	pD->input[pos] = val;
}

float ExtProc::getOutput( int outdx, int idx )
{
	return pD->output[idx];
}

void ExtProc::resize( int nrsamples )
{
	if (nrsamples != pD->nrSamples) {
		if (pD->input != NULL)
			delete [] pD->input;
		if (pD->output != NULL)
			delete [] pD->output;
		pD->nrSamples = nrsamples;
		int nrtraces = pD->nrTraces;
		pD->input = new float[nrsamples*nrtraces];
		pD->output = new float[nrsamples];
		if (pD->input==NULL || pD->output==NULL)
			ErrMsg( "ExtProc::resize - error allocating array space" );
	}
}

void ExtProc::compute( int z0, int inl, int crl)
{
	// Send info packet to process stdin
	pD->writeTrcInfo( z0, inl, crl );
// Send input array to process stdin
	pD->writeData();
// Read output array from process stdout 
	pD->readData();
}

bool ExtProc::hasInput()
{
	return (pD->jsonpar.GetType() != json::NULLVal && pD->jsonpar.HasKey("Input"));
	
}

BufferString ExtProc::inputName()
{
	BufferString res;
	if (hasInput()) {
		res = pD->jsonpar["Input"].ToString().c_str();
	}
	return res;
}

bool ExtProc::hasZMargin()
{
	return (pD->jsonpar.GetType() != json::NULLVal && pD->jsonpar.HasKey("ZSampMargin"));
		 
}
	
Interval<int> ExtProc::zmargin()
{
	Interval<int> res;
	if (hasZMargin()) {
		json::Array zmarr = pD->jsonpar["ZSampMargin"].ToArray();
		int begin = zmarr[0];
		int end = zmarr[1];
		res = Interval<int>(begin, end);
	}
	return res;
}

void ExtProc::setZMargin( Interval<int> g )
{
	json::Array garr;
	garr.insert(0, g.start);
	garr.insert(1, g.stop);
	pD->jsonpar["ZSampMargin"] = garr;
}

bool ExtProc::hasStepOut()
{
	return (pD->jsonpar.GetType() != json::NULLVal && pD->jsonpar.HasKey("StepOut"));
}

BinID ExtProc::stepout()
{
	BinID res;
	if (hasStepOut()) {
		json::Array sarr = pD->jsonpar["StepOut"].ToArray();
		int inl = sarr[0];
		int xln = sarr[1];
		res = BinID(inl, xln);
	}
	return res;
}

void ExtProc::setStepOut(BinID s)
{
	json::Array sarr;
	sarr.insert(0, s.inl());
	sarr.insert(1, s.crl());
	pD->jsonpar["StepOut"] = sarr;
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

BufferString ExtProc::getFile()
{
	return pD->exfile;
}

void ExtProc::setFile( const char* fname )
{
		pD->setFile(fname);
	
}