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
#include <sys/types.h>
#include "survinfo.h"
#include "errmsg.h"
#include "ranges.h"
#include "position.h"
#include "extproc.h"
#include "json.h"
#include "msgh.h"

#ifdef __win__
#include <windows.h>
#include <io.h>
#else
#include <paths.h>
#include <unistd.h>
#include <sys/wait.h>
#endif

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
	ExtProcImpl(const char* fname, const char* iname);
	~ExtProcImpl();
	
	bool			start( char *const argv[] );
	int				finish();
	
		
	bool			writeSeisInfo();
	bool			writeTrcInfo( int z0, int inl, int crl );
	bool			writeData();
	bool			readData();
	BufferString	readAllStdOut();
	
	bool			getParam();
	void			setFile(const char* fname, const char* iname);
	
	float*			input;
	float*			output;
	int				nrSamples;
	int				nrTraces;
	int				nrOutput;
	SeisInfo		seisInfo;
	TrcInfo			trcInfo;
	FILE*			read_fd;
	FILE*			write_fd;
#ifdef __win__
	HANDLE			hChildProcess;
	HANDLE			hChildThread;
#else
	pid_t			child_pid;
#endif
	BufferString 	exfile;
	BufferString	infile;
	json::Value		jsonpar;
	
	
};

ExtProcImpl::ExtProcImpl(const char* fname, const char* iname)
:  input(NULL), output(NULL), read_fd(NULL), write_fd(NULL)
{
#ifdef __win__
	hChildProcess = NULL;
	hChildThread = NULL;
#else
	child_pid = -1;
#endif
	nrSamples = 0;
	nrTraces = 0;
	nrOutput = 1;
	setFile(fname, iname);
}
	
ExtProcImpl::~ExtProcImpl()
{	
#ifdef __win__
	if (hChildProcess)
		finish();
#else
	if (child_pid != -1)
		finish();
#endif
	if (input != NULL)
		delete [] input;
	if (output != NULL)
		delete [] output;
}

void ExtProcImpl::setFile(const char* fname, const char* iname)
{
	exfile = fname;
	infile = iname;
	getParam();
}

bool ExtProcImpl::start( char *const argv[] )
{
#ifdef __win__

	HANDLE g_hChildStd_IN_Rd = NULL;
	HANDLE g_hChildStd_IN_Wr = NULL;
	HANDLE g_hChildStd_OUT_Rd = NULL;
	HANDLE g_hChildStd_OUT_Wr = NULL; 
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL; 
// Create a pipe for the child process's STDOUT. 
	if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0) ) {
		ErrMsg("ExtProcImpl::start - opening stdout failed");
		return false;
	}
	
// Ensure the read handle to the pipe for STDOUT is not inherited.
	
	if ( ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) ) {
		ErrMsg("ExtProcImpl::start - stdout set handle information failed");
		return false;
	}
	
// Create a pipe for the child process's STDIN. 
	
	if (! CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) {
		ErrMsg("ExtProcImpl::start - opening stdin failed");
		return false;
	}
	
// Ensure the write handle to the pipe for STDIN is not inherited. 
	
	if ( ! SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) ) {
		ErrMsg("ExtProcImpl::start - stdin set handle information failed");
		return false;
	}
// Convert the HANDLES to C FILE*
	int fileInNo, fileOutNo;
	if ((fileInNo = _open_osfhandle((LONG) g_hChildStd_IN_Wr, 0)) == -1 || (fileOutNo = _open_osfhandle((LONG) g_hChildStd_OUT_Rd, 0)) == -1) {
		ErrMsg("ExtProcImpl::start - unable to get file number for child stdin/stdout");
		CloseHandle( g_hChildStd_IN_Rd );
		CloseHandle( g_hChildStd_IN_Wr );
		CloseHandle( g_hChildStd_OUT_Rd );
		CloseHandle( g_hChildStd_OUT_Wr ); 
		return false;
	}
	if (!(read_fd = fdopen(fileOutNo, "r"))) {
		read_fd = NULL;
		write_fd = NULL;
		CloseHandle( g_hChildStd_IN_Rd );
		CloseHandle( g_hChildStd_IN_Wr );
		CloseHandle( g_hChildStd_OUT_Rd );
		CloseHandle( g_hChildStd_OUT_Wr ); 
		ErrMsg("ExtProcImpl::start - open read_fd failed");
		return false;
	}
	if (!(write_fd = fdopen(fileInNo, "w"))) {
		fclose(read_fd);
		read_fd = NULL;
		write_fd = NULL;
		CloseHandle( g_hChildStd_IN_Rd );
		CloseHandle( g_hChildStd_OUT_Rd );
		CloseHandle( g_hChildStd_OUT_Wr ); 
		ErrMsg("ExtProcImpl::start - open write_fd failed");
		return false;
	}
	setbuf( read_fd, NULL );
	setbuf( write_fd, NULL);
	
// Get path to the shell
	char* cmd_path = NULL;
	cmd_path = getenv("ComSpec");
	if (!cmd_path) {
		ErrMsg("ExtProcImpl::start - ComSpec is not defined");
		return false;
	}
// Build the command line
	BufferString cmd = " /C";
	int i = 0;
	while (argv[i]!=0) {
		cmd += " ";
		cmd += argv[i];
		i++;
	}
// Spawn the child process
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	GetStartupInfo(&si);      
	si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.hStdError = g_hChildStd_OUT_Wr;
	si.hStdOutput = g_hChildStd_OUT_Wr;
	si.hStdInput = g_hChildStd_IN_Rd;
	ZeroMemory(&pi, sizeof(pi));
	
	bool res = CreateProcess( 	cmd_path, 
								cmd.getCStr(),
								NULL,
								NULL,
								true,
								0,
								NULL,
								NULL,
								&si,
								&pi );
							  
	if (!res) {
		ErrMsg("ExtProcImpl::start - CreateProcess failed");
		CloseHandle( g_hChildStd_IN_Rd );
		CloseHandle( g_hChildStd_IN_Wr );
		CloseHandle( g_hChildStd_OUT_Rd );
		CloseHandle( g_hChildStd_OUT_Wr ); 
		return false;
	}
	hChildProcess = pi.hProcess;
	hChildThread = pi.hThread;
	CloseHandle(g_hChildStd_OUT_Wr);
	CloseHandle(g_hChildStd_IN_Rd);
	return true;
#else
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
#endif	
}

int ExtProcImpl::finish() {
#ifdef __win__
	DWORD status;
	if (hChildProcess) {
		if (GetExitCodeProcess( hChildProcess, &status)) {
			if (status == STILL_ACTIVE) {
				WaitForInputIdle( hChildProcess, INFINITE );
				TerminateProcess( hChildProcess, 0 );
				WaitForSingleObject( hChildProcess, INFINITE ); 
			}
		} else {
			ErrMsg("ExtProcImpl::finish - GetExitCodeProcess failed");
			return -1;
		}
		CloseHandle( hChildProcess );
		CloseHandle( hChildThread );
		hChildProcess = NULL;
		hChildThread = NULL;
	}
	if (write_fd) {
		fclose(write_fd);
		write_fd = NULL;
	}
	if (read_fd) {
		fclose(read_fd);
		read_fd = NULL;
	}
	return (int) status;
#else
	int   status;
	pid_t pid = -1;
	
	if (child_pid != -1) {
		kill( child_pid, SIGKILL );
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
	if (pid != -1 && WIFEXITED(status)) 
		return WEXITSTATUS(status);
	else 
		return (pid == -1 ? -1 : 0);
#endif	
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
		size_t res = fread((void*) output, nbytes, nrSamples*nrOutput, read_fd);
		if (res != nrSamples*nrOutput) {
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
			size_t count = fread( (void*) buffer, 1, sizeof(buffer-1), read_fd );
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
	char* args[4];
	if (infile.isEmpty()) {
		args[0] = (char*) exfile.str();
		args[1] = (char*) "-g";
		args[2] = 0;
	} else if (exfile.isEmpty()) {
		ErrMsg("ExtProcImpl::getParam - no external attribute file provided");
		return false;
	} else {
		args[0] = (char*) infile.str();
		args[1] = (char*) exfile.str();
		args[2] = (char*) "-g";
		args[3] = 0;
	}
	if (start( args )) {
		params = readAllStdOut();
		if (finish() != 0 ) {
			ErrMsg("ExtProcImpl::getParam - external attribute exited abnormally");
			ErrMsg(params.str());
			return false;
		}
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

ExtProc::ExtProc( const char* fname, const char* iname ) 
: pD(new ExtProcImpl(fname, iname))
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
	pD->nrOutput = numOutput();
	
	BufferString params(json::Serialize(pD->jsonpar).c_str());
#ifdef __win__
	params.replace("\"","\"\""); 
	params.embed('"','"');
#endif
	char* args[5];
	if (pD->infile.isEmpty()) {
		args[0] = (char*) pD->exfile.str();
		args[1] = (char*) "-c";
		args[2] = (char*) params.str();
		args[3] = 0;
	} else if (pD->exfile.isEmpty()) {
		ErrMsg("ExtProc::start - no external attribute file provided");
	} else {
		args[0] = (char*) pD->infile.str();
		args[1] = (char*) pD->exfile.str();
		args[2] = (char*) "-c";
		args[3] = (char*) params.str();
		args[4] = 0;
	}

	if (!pD->start( args ))
		ErrMsg("ExtProc::start - run error");

	pD->writeSeisInfo();
	return;
}

bool ExtProc::isBusy() const
{
#ifdef __win__
	return pD->hChildProcess != NULL;
#else
	return pD->child_pid != -1;
#endif
}

void ExtProc::setInput( int inpdx, int idx, float val )
{
//	int pos = inpdx*(pD->info).nrSamples + idx;
	int pos = idx;
	pD->input[pos] = val;
}

float ExtProc::getOutput( int outdx, int idx )
{
	int pos = outdx*pD->nrSamples + idx;
	return pD->output[pos];
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
		int nrout = pD->nrOutput;
		pD->input = new float[nrsamples*nrtraces];
		pD->output = new float[nrsamples*nrout];
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

BufferString ExtProc::getFile()
{
	return pD->exfile;
}

void ExtProc::setFile( const char* fname, const char* iname)
{
		pD->setFile(fname, iname);
	
}
