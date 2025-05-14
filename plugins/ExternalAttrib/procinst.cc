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
#include <fcntl.h>

#include "envvars.h"
#include "errmsg.h"
#include "msgh.h"
#include "procinst.h"
#include "filepath.h"
#include "file.h"
#include "pythonaccess.h"
#include "thread.h"
#include "threadwork.h"
#include "uistringset.h"
#include "extproc.h"

#include <fstream>
#include <string>

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
	int			nrSamples;
	int			nrTraces;
	int			nrOutput;
	int			nrInput;
	TrcInfo			trcInfo;

	FILE*			read_fd;
	FILE*			write_fd;
	BufferString		logFile;
	uiRetVal		uirv;
	PtrMan<Threads::Work>	logmonitor;
	bool			do_logging = true;
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
	logFile = FilePath::getTempFullPath(nullptr, nullptr);
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
	pD->do_logging = false;
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

bool ProcInst::start( const BufferStringSet& runargs)
{
#ifdef __win__
	if (pD->hChildProcess != NULL)
	{
		repError("ProcInst::start - already in use");
		return false;
	}
	HANDLE g_hChildStd_IN_Rd = NULL;
	HANDLE g_hChildStd_IN_Wr = NULL;
	HANDLE g_hChildStd_OUT_Rd = NULL;
	HANDLE g_hChildStd_OUT_Wr = NULL;

	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

// Create a pipe for the child process's STDOUT.
	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0) ||
		!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) {
			repError("ProcInst::start - opening stdout failed");
			return false;
	}

// Create a pipe for the child process's STDIN.
	if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0) ||
		!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)) {
			repError("ProcInst::start - opening stdin failed");
			return false;
	}

// Convert the HANDLES to C FILE*
	int fileInNo = _open_osfhandle((intptr_t)g_hChildStd_IN_Wr, 0);
	int fileOutNo = _open_osfhandle((intptr_t)g_hChildStd_OUT_Rd, 0);

	if (fileInNo == -1 || fileOutNo == -1) {
			repError("ProcInst::start - unable to get file number for child stdin/stdout");
			CloseHandle( g_hChildStd_IN_Rd );
			CloseHandle( g_hChildStd_IN_Wr );
			CloseHandle( g_hChildStd_OUT_Rd );
			CloseHandle( g_hChildStd_OUT_Wr );
			return false;
	}
	pD->read_fd = _fdopen(fileOutNo, "r");
	if (!pD->read_fd) {
		pD->read_fd = NULL;
		pD->write_fd = NULL;
		CloseHandle( g_hChildStd_IN_Rd );
		CloseHandle( g_hChildStd_IN_Wr );
		CloseHandle( g_hChildStd_OUT_Rd );
		CloseHandle( g_hChildStd_OUT_Wr );
		repError("ExtProcImpl::start - open read_fd failed");
		return false;
	}
	pD->write_fd = _fdopen(fileInNo, "w");
	if (!pD->write_fd) {
		fclose(pD->read_fd);
		pD->read_fd = NULL;
		pD->write_fd = NULL;
		CloseHandle( g_hChildStd_IN_Rd );
		CloseHandle( g_hChildStd_OUT_Rd );
		CloseHandle( g_hChildStd_OUT_Wr );
		repError("ProcInst::start - open write_fd failed");
		return false;
	}
	setvbuf( pD->read_fd, NULL, _IONBF, 0 );
	setvbuf( pD->write_fd, NULL, _IONBF, 0 );
//
// Open the error log file.
	HANDLE herr = CreateFile(	logFileName().getCStr(),
					GENERIC_WRITE,
					FILE_SHARE_READ,
					&saAttr,
					CREATE_NEW,
					FILE_ATTRIBUTE_NORMAL,
					NULL );
	if (herr == INVALID_HANDLE_VALUE) {
		repError(BufferString("ProcInst::start - unable to open error log file ", logFileName().getCStr()));
		return false;
	}
//
// Build the command line
	BufferString cmd;
	cmd.add(runargs.cat(" "));
// Spawn the child process
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	GetStartupInfo(&si);
	si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;

	si.wShowWindow = SW_HIDE;
	si.hStdError = herr;
	si.hStdOutput = g_hChildStd_OUT_Wr;
	si.hStdInput = g_hChildStd_IN_Rd;
	ZeroMemory(&pi, sizeof(pi));

	bool res = CreateProcess(	NULL,
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
		CloseHandle( herr );
		return false;
	}
	pD->hChildProcess = pi.hProcess;
	pD->hChildThread = pi.hThread;
	CloseHandle(g_hChildStd_OUT_Wr);
	CloseHandle(g_hChildStd_IN_Rd);
	CloseHandle(herr);
	return true;
#else
	BufferString pypath;
	BufferStringSet pypathset = ExtProc::getPythonPath();
	if ( !pypathset.isEmpty() )
	    pypath = BufferString( "PYTHONPATH=", pypathset.cat(":") );

	char* envp[4];
	envp[0] = (char*) "IFS= \t\n";
	envp[1] = (char*) "PATH=" _PATH_STDPATH;
	envp[2] = pypathset.isEmpty() ? 0 : (char*) pypath.buf();
	envp[3] = 0;

	if (pD->child_pid != -1)
	{
		repError("ProcInst::start - already in use");
		return false;
	}
	int	stdin_pipe[2], stdout_pipe[2];

	if (pipe(stdin_pipe) == -1) {
		return false;
	}
	if (pipe(stdout_pipe) == -1) {
		close(stdin_pipe[1]);
		close(stdin_pipe[0]);
		return false;
	}
	pD->read_fd = fdopen(stdout_pipe[0], "r");
	if (!(pD->read_fd)) {
		pD->read_fd = NULL;
		pD->write_fd = NULL;
		close(stdout_pipe[1]);
		close(stdout_pipe[0]);
		close(stdin_pipe[1]);
		close(stdin_pipe[0]);
		repError("ProcInst::start - open read_fd failed");
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
		repError("ProcInst::start - open write_fd failed");
		return false;
	}
	setbuf( pD->read_fd, NULL );
	setbuf( pD->write_fd, NULL);
	if ((pD->child_pid = fork()) == -1) {
		fclose(pD->write_fd);
		fclose(pD->read_fd);
		pD->read_fd = NULL;
		pD->write_fd = NULL;
		close(stdout_pipe[0]);
		close(stdout_pipe[1]);
		close(stdin_pipe[0]);
		close(stdin_pipe[1]);
		return false;
	}

	if (!pD->child_pid) {
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
		int fderr = open(pD->logFile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
		if (fderr != -1 ) {
			dup2(fderr, 2);
			close(fderr);
		}
		int fd, fds;
		if ((fds = getdtablesize()) == -1 )
			fds = FOPEN_MAX;
		for (fd = 3; fd<fds; fd++)
			close(fd);

		char** argv = new char*[runargs.size() + 1];
		for (int idx = 0; idx < runargs.size(); idx++) {
		    BufferString* arg = new BufferString(runargs.get(idx));
		    argv[idx] = arg->getCStr();
		}
		argv[runargs.size()] = 0;
		execve(argv[0], argv, envp);
//	Should never get here
		repError("ProcInst::start - child process not started");
		return false;
	}

	close(stdout_pipe[1]);
	close(stdin_pipe[0]);
	return true;
#endif
}


bool ProcInst::start( const BufferStringSet& runargs, SeisInfo& si )
{
	bool 	result = false;
	pD->uirv.setEmpty();
	pD->logmonitor = new Threads::Work(mCB(this, ProcInst, processLogCB));
	if (pD->logmonitor)
	    Threads::WorkManager::twm().addWork(*pD->logmonitor);

	if (start( runargs )) {
// Check for errors
		result = writeSeisInfo( si );
	} else {
		ErrMsg("ProcInst::start - run error");
	}
	return result;
}

void ProcInst::processLogCB( CallBacker*)
{
    if (pD->logFile.isEmpty())
	return;

    while ( !File::exists(pD->logFile.buf()) )
    {}

    std::ifstream ifs( pD->logFile.buf() );
    if ( ifs.is_open() )
    {
	std::string line;
	std::streamoff p = 0;
	while ( pD && pD->do_logging )
	{
	    ifs.seekg( p );
	    while ( std::getline(ifs, line) )
	    {
		repError(line.c_str());
		if ( ifs.tellg()==-1 )
		    p += line.size();
		else
		    p = ifs.tellg();
	    }
	    ifs.clear();
	}
	ifs.close();
    }
}

void ProcInst::stopLogging(bool remove)
{
    pD->do_logging = false;
    if (!pD->logmonitor)
	return;

    Threads::WorkManager::twm().removeWork(*pD->logmonitor);
    if (!pD->logFile.isEmpty() && remove)
	    File::remove(pD->logFile);
}

int ProcInst::finish() {
	int result = 0;
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
			stopLogging();
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
	result = (int) status;
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
	pD->child_pid=-1;
	if (pid != -1 && WIFEXITED(status))
		result = WEXITSTATUS(status);
	else
		result = (pid == -1 ? -1 : 0);
#endif
	stopLogging();
	return result;
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
	bool result = false;
// 	Send info packet to process stdin
	result |= writeTrcInfo( z0, inl, crl );
// 	Send input array to process stdin
	result |= writeData();
// 	Read output array from process stdout
	result |= readData();
	return result;
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

uiRetVal ProcInst::errMsg()
{
    return pD->uirv;
}

bool ProcInst::isOK()
{
    return pD->uirv.isOK();
}

void ProcInst::repError(const char* msg)
{
    ErrMsg(msg);
    pD->uirv.add(toUiString(msg));
}
