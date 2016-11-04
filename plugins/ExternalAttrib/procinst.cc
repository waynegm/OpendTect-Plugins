/*Copyright (C) 2015 Wayne Mogg All rights reserved.

This file may be used either under the terms of:

1. The GNU General Public License version 3 or higher, as published by
the Free Software Foundation, or

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

/*+
________________________________________________________________________

 Author:        Wayne Mogg
 Date:          May 2015
 ________________________________________________________________________

-*/ 
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>

#include "errmsg.h"
#include "msgh.h"
#include "procinst.h"
#include "filepath.h"

#ifdef __win__
#include <windows.h>
#include <io.h>
#else
#include <paths.h>
#include <unistd.h>
#include <sys/wait.h>
#endif

struct TrcInfo
{
	int		nrSamples;
	int		z0;
	int		inl;
	int		crl;
};

struct ProcInstImpl {
public:
	ProcInstImpl();
	~ProcInstImpl() {}
	
	float*			input;
	float*			output;
	int				nrSamples;
	int				nrTraces;
	int				nrOutput;
	int				nrInput;
	TrcInfo			trcInfo;
	
	FILE*			read_fd;
	FILE*			write_fd;
	FILE*			err_fd;
	BufferString	logFile;
#ifdef __win__
	HANDLE			hChildProcess;
	HANDLE			hChildThread;
#else
	pid_t			child_pid;
#endif
};

ProcInstImpl::ProcInstImpl()
: input(NULL), output(NULL), read_fd(NULL), write_fd(NULL)
{
	logFile = FilePath::getTempName();
#ifdef __win__
	hChildProcess = NULL;
	hChildThread = NULL;
#else
	child_pid = -1;
#endif
	nrSamples = 0;
	nrTraces = 0;
	nrOutput = 1;
	nrInput = 1;
}


ProcInst::ProcInst()
:  pD( new ProcInstImpl() )
{
}

ProcInst::~ProcInst()
{
	finish();
	if (pD->input != NULL)
		delete [] pD->input;
	if (pD->output != NULL)
		delete [] pD->output;

	delete pD;
}

BufferString ProcInst::logFileName()
{
	return pD->logFile;
}

void ProcInst::setInput( int inpdx, int trc, int idx, float val )
{
	int pos = inpdx*pD->nrSamples*pD->nrTraces + trc*pD->nrSamples + idx;
	pD->input[pos] = val;
}

float ProcInst::getOutput( int outdx, int idx )
{
	int pos = outdx*pD->nrSamples + idx;
	return pD->output[pos];
}

void ProcInst::resize( int nrsamples )
{
	if (nrsamples != pD->nrSamples) {
		if (pD->input != NULL)
			delete [] pD->input;
		if (pD->output != NULL)
			delete [] pD->output;
		pD->nrSamples = nrsamples;
		int nrtraces = pD->nrTraces;
		int nrout = pD->nrOutput;
		int nrin = pD->nrInput;
		pD->input = new float[nrsamples*nrtraces*nrin];
		pD->output = new float[nrsamples*nrout];
		if (pD->input==NULL || pD->output==NULL)
			ErrMsg( "ProcInst::resize - error allocating array space" );
	}
}

bool ProcInst::start( char *const argv[] )
{
#ifdef __win__
	if (pD->hChildProcess != NULL)
	{
		ErrMsg("ProcInst::start - already in use");
		return false;
	}
	HANDLE g_hChildStd_IN_Rd = NULL;
	HANDLE g_hChildStd_IN_Wr = NULL;
	HANDLE g_hChildStd_OUT_Rd = NULL;
	HANDLE g_hChildStd_OUT_Wr = NULL; 
	HANDLE g_hChildStd_ERR_Rd = NULL;
	HANDLE g_hChildStd_ERR_Wr = NULL; 
	
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL;
	
// Create a pipe for the child process's STDOUT. 
	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0) ||
		!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) {
			ErrMsg("ProcInst::start - opening stdout failed");
			return false;
	}
	
// Create a pipe for the child process's STDIN. 
	if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0) ||
		!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)) {
			ErrMsg("ProcInst::start - opening stdin failed");
			return false;
	}

// Create a pipe for the child process's STERR. 
	if (!CreatePipe(&g_hChildStd_ERR_Rd, &g_hChildStd_ERR_Wr, &saAttr, 0) ||
		!SetHandleInformation(g_hChildStd_ERR_Rd, HANDLE_FLAG_INHERIT, 0)) {
			ErrMsg("ProcInst::start - opening stderr failed");
			return false;
	}
	
// Convert the HANDLES to C FILE*
	int fileInNo, fileOutNo, fileErrNo;
	if ((fileInNo = _open_osfhandle((LONG) g_hChildStd_IN_Wr, 0)) == -1 || 
		(fileOutNo = _open_osfhandle((LONG) g_hChildStd_OUT_Rd, 0)) == -1 ||
		(fileErrNo = _open_osfhandle((LONG) g_hChildStd_ERR_Rd, 0)) == -1 {
			ErrMsg("ProcInst::start - unable to get file number for child stdin/stdout/stderr");
			CloseHandle( g_hChildStd_IN_Rd );
			CloseHandle( g_hChildStd_IN_Wr );
			CloseHandle( g_hChildStd_OUT_Rd );
			CloseHandle( g_hChildStd_OUT_Wr ); 
			CloseHandle( g_hChildStd_ERR_Rd );
			CloseHandle( g_hChildStd_ERR_Wr ); 
			return false;
	}
	if (!(pD->read_fd = fdopen(fileOutNo, "r"))) {
		pD->read_fd = NULL;
		pD->write_fd = NULL;
		pD->err_fd = NULL;
		CloseHandle( g_hChildStd_IN_Rd );
		CloseHandle( g_hChildStd_IN_Wr );
//		CloseHandle( g_hChildStd_OUT_Rd );
		CloseHandle( g_hChildStd_OUT_Wr ); 
		CloseHandle( g_hChildStd_ERR_Rd );
		CloseHandle( g_hChildStd_ERR_Wr ); 
		ErrMsg("ExtProcImpl::start - open read_fd failed");
		return false;
	}
	if (!(pD->write_fd = fdopen(fileInNo, "w"))) {
		fclose(pD->read_fd);
		pD->read_fd = NULL;
		pD->write_fd = NULL;
		pD->err_fd = NULL;
		CloseHandle( g_hChildStd_IN_Rd );
		CloseHandle( g_hChildStd_OUT_Rd );
		CloseHandle( g_hChildStd_OUT_Wr ); 
		CloseHandle( g_hChildStd_ERR_Rd );
		CloseHandle( g_hChildStd_ERR_Wr ); 
		ErrMsg("ProcInst::start - open write_fd failed");
		return false;
	}
	if (!(pD->err_fd = fdopen(fileErrNo, "r"))) {
		fclose(pD->read_fd);
		fclose(pD->write_fd);
		pD->read_fd = NULL;
		pD->write_fd = NULL;
		pD->err_fd = NULL;
		CloseHandle( g_hChildStd_IN_Rd );
		CloseHandle( g_hChildStd_IN_Wr );
		CloseHandle( g_hChildStd_OUT_Rd );
		CloseHandle( g_hChildStd_OUT_Wr ); 
//		CloseHandle( g_hChildStd_ERR_Rd );
		CloseHandle( g_hChildStd_ERR_Wr ); 
		ErrMsg("ExtProcImpl::start - open err_fd failed");
		return false;
	}
	setbuf( pD->read_fd, NULL );
	setbuf( pD->write_fd, NULL);
	setbuf( pD->err_fd, NULL);
	
// Get path to the shell
	char* cmd_path = NULL;
	cmd_path = getenv("ComSpec");
	if (!cmd_path) {
		ErrMsg("ProcInst::start - ComSpec is not defined");
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
	si.hStdError = g_hChildStd_ERR_Wr;
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
		ErrMsg("ProcInst::start - CreateProcess failed");
		CloseHandle( g_hChildStd_IN_Rd );
		CloseHandle( g_hChildStd_IN_Wr );
		CloseHandle( g_hChildStd_OUT_Rd );
		CloseHandle( g_hChildStd_OUT_Wr ); 
		CloseHandle( g_hChildStd_ERR_Rd );
		CloseHandle( g_hChildStd_ERR_Wr ); 
		return false;
	}
	pD->hChildProcess = pi.hProcess;
	pD->hChildThread = pi.hThread;
	CloseHandle(g_hChildStd_OUT_Wr);
	CloseHandle(g_hChildStd_IN_Rd);
	CloseHandle(g_hChildStd_ERR_Wr);
	return true;
#else
	char* envp[3];
	envp[0] = (char*) "IFS= \t\n";
	envp[1] = (char*) "PATH=" _PATH_STDPATH;
	envp[2] = 0;
	
	if (pD->child_pid != -1)
	{
		ErrMsg("ProcInst::start - already in use");
		return false;
	}
	int		stdin_pipe[2], stdout_pipe[2], stderr_pipe[2];
	
	if (pipe(stdin_pipe) == -1) {
		return false;
	}
	if (pipe(stdout_pipe) == -1) {
		close(stdin_pipe[1]);
		close(stdin_pipe[0]);
		return false;
	}
	if (pipe(stderr_pipe) == -1) {
		close(stdin_pipe[1]);
		close(stdin_pipe[0]);
		close(stdout_pipe[1]);
		close(stdout_pipe[0]);
		return false;
	}
	pD->read_fd = fdopen(stdout_pipe[0], "r");
	if (!(pD->read_fd)) {
		pD->read_fd = NULL;
		pD->write_fd = NULL;
		pD->err_fd = NULL;
		close(stdout_pipe[1]);
		close(stdout_pipe[0]);
		close(stdin_pipe[1]);
		close(stdin_pipe[0]);
		close(stderr_pipe[1]);
		close(stderr_pipe[0]);
		ErrMsg("ProcInst::start - open read_fd failed");
		return false;
	}
	pD->write_fd = fdopen(stdin_pipe[1], "w");
	if (!(pD->write_fd )) {
		fclose(pD->read_fd);
		pD->read_fd = NULL;
		pD->write_fd = NULL;
		close(stdout_pipe[1]);
		close(stdin_pipe[1]);
		close(stdin_pipe[0]);
		close(stderr_pipe[1]);
		close(stderr_pipe[0]);
		ErrMsg("ProcInst::start - open write_fd failed");
		return false;
	}
	pD->err_fd = fdopen(stderr_pipe[0], "r");
	if (!(pD->err_fd )) {
		fclose(pD->read_fd);
		fclose(pD->write_fd);
		pD->read_fd = NULL;
		pD->write_fd = NULL;
		pD->err_fd = NULL;
		close(stdout_pipe[1]);
		close(stdin_pipe[0]);
		close(stderr_pipe[1]);
		close(stderr_pipe[0]);
		ErrMsg("ProcInst::start - open err_fd failed");
		return false;
	}
	setbuf( pD->read_fd, NULL );
	setbuf( pD->write_fd, NULL);
	setbuf( pD->err_fd, NULL);
	if ((pD->child_pid = fork()) == -1) {
		fclose(pD->write_fd);
		fclose(pD->read_fd);
		fclose(pD->err_fd);
		pD->read_fd = NULL;
		pD->write_fd = NULL;
		pD->err_fd = NULL;
		close(stdout_pipe[1]);
		close(stdin_pipe[0]);
		close(stderr_pipe[1]);
		return false;
	}
	
	if (!pD->child_pid) {
//	This is the child process
//	close open files other than stdin, stdout and stderr
		close(stdout_pipe[0]);
		close(stdin_pipe[1]);
		close(stderr_pipe[0]);
		if (stdin_pipe[0] != 0) {
			dup2(stdin_pipe[0], 0);
			close(stdin_pipe[0]);
		}
		if (stdout_pipe[1] != 1) {
			dup2(stdout_pipe[1], 1);
			close(stdout_pipe[1]);
		}
		if (stderr_pipe[1] != 1) {
			dup2(stderr_pipe[1], 2);
			close(stderr_pipe[1]);
		}
		int fd, fds;
		if ((fds = getdtablesize()) == -1 )
			fds = FOPEN_MAX;
		for (fd = 3; fd<fds; fd++)
			close(fd);
		execve(argv[0], argv, envp);
//	Should never get here
		ErrMsg("ProcInst::start - child process not started");
		return false;
	}
	
	close(stdout_pipe[1]);
	close(stdin_pipe[0]);
	close(stderr_pipe[1]);
	return true;
#endif	
}


bool ProcInst::start( char *const argv[], SeisInfo& si )
{
	if (start( argv )) {
// Check for errors
		return writeSeisInfo( si );
	} else {
		ErrMsg("ProcInst::start - run error");
		return false;
	}
}

int ProcInst::finish() {
#ifdef __win__
	DWORD status=0;
	if (pD->hChildProcess) {
		if (GetExitCodeProcess( pD->hChildProcess, &status)) {
			if (status == STILL_ACTIVE) {
				WaitForInputIdle( pD->hChildProcess, INFINITE );
				TerminateProcess( pD->hChildProcess, 0 );
				WaitForSingleObject( pD->hChildProcess, INFINITE ); 
			}
		} else {
			ErrMsg("ProcInst::finish - GetExitCodeProcess failed");
			return -1;
		}
		CloseHandle( pD->hChildProcess );
		CloseHandle( pD->hChildThread );
		pD->hChildProcess = NULL;
		pD->hChildThread = NULL;
	}
	if (pD->write_fd) {
		fclose(pD->write_fd);
		pD->write_fd = NULL;
	}
	if (pD->read_fd) {
		fclose(pD->read_fd);
		pD->read_fd = NULL;
	}
	if (pD->err_fd) {
		fclose(pD->err_fd);
		pD->err_fd = NULL;
	}
	return (int) status;
#else
	int   status;
	pid_t pid = -1;
	
	if (pD->child_pid != -1) {
		kill( pD->child_pid, SIGKILL );
		do {
			pid = waitpid(pD->child_pid, &status, 0);
		} while (pid == -1 && errno == EINTR);
	}

	if (pD->write_fd) {
		fclose(pD->write_fd);
		pD->write_fd = NULL;
	}
	if (pD->read_fd) {
		fclose(pD->read_fd);
		pD->read_fd = NULL;
	}
	if (pD->err_fd) {
		fclose(pD->err_fd);
		pD->err_fd = NULL;
	}
	pD->child_pid=-1;
	if (pid != -1 && WIFEXITED(status)) 
		return WEXITSTATUS(status);
	else 
		return (pid == -1 ? -1 : 0);
#endif	
}	
	

BufferString ProcInst::readAllStdOut()
{
	BufferString result;
	if (pD->read_fd) {
		char buffer[4096];
		while (1) {
			size_t count = fread( (void*) buffer, 1, sizeof(buffer-1), pD->read_fd );
			if (count == -1) {
				if (errno == EINTR) 
					continue;
				else {
					ErrMsg("ProcInst::readAllStdOut- read error");
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
		ErrMsg("ProcInst::readAllStdOut - no stdout");
	
	return result;
}

bool ProcInst::compute( int z0, int inl, int crl )
{
// Send info packet to process stdin
	writeTrcInfo( z0, inl, crl );
// Send input array to process stdin
	writeData();
// Read output array from process stdout 
	 return readData();
}

bool ProcInst::writeSeisInfo( SeisInfo& si )
{
	pD->nrTraces = si.nrTraces;
	pD->nrInput = si.nrInput;
	pD->nrOutput = si.nrOutput;
	if (pD->write_fd) {
		size_t nbytes = sizeof(si);
		size_t res = fwrite((void*) &si, nbytes, 1, pD->write_fd);
// Check for errors
		if (res != 1) {
			ErrMsg("ProcInst::writeSeisInfo - error writing info block to external attribute");
			return false;
		}
		fflush(pD->write_fd);
		return true;
	} else {
		ErrMsg("ProcInst::writeSeisInfo - no stdin");
		return false;
	}
}

bool ProcInst::writeTrcInfo(int z0, int inl, int crl)
{
	if (pD->write_fd) {
		TrcInfo& ti = pD->trcInfo;
		ti.z0 = z0;
		ti.inl = inl;
		ti.crl = crl;
		ti.nrSamples = pD->nrSamples;
		
		size_t nbytes = sizeof(ti);
		size_t res = fwrite((void*) &ti, nbytes, 1, pD->write_fd);
		if (res != 1) {
			ErrMsg("ProcInst::writeTrcInfo - error writing info block to external attribute");
			return false;
		}
		fflush(pD->write_fd);
		return true;
	} else {
		ErrMsg("ProcInst::writeTrcInfo - no stdin");
		return false;
	}
}
	
bool ProcInst::writeData()
{
	if (pD->write_fd) {
		size_t nbytes = sizeof(float);
		size_t nsize = pD->nrSamples * pD->nrTraces * pD->nrInput;
		size_t res = fwrite((void*) pD->input, nbytes, nsize, pD->write_fd);
		if (res != nsize) {
			ErrMsg("ProcInst::writeData - error writing data to external attribute");
			return false;
		}
		fflush(pD->write_fd);
		return true;
	} else {
		ErrMsg("ProcInst::writeData - no stdin");
		return false;
	}
}

bool ProcInst::readData()
{
	if (pD->read_fd) {
		size_t nbytes = sizeof(float);
		size_t nsize = pD->nrSamples * pD->nrOutput;
		size_t res = fread((void*) pD->output, nbytes, nsize, pD->read_fd);
		if (res != nsize) {
			ErrMsg("ProcInst::readData - error reading from external attribute");
			return false;
		}
		return true;	
		
	} else {
		ErrMsg("ProcInst::readData - no stdout");	
		
		return false;
	}
}
