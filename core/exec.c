/*
 * Copyright (c) 2005-2019 Julien Nadeau Carriere <vedge@csoft.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Copyright (c) 2010 Michael J. Wood
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Cross-platform file execution interface.
 */

#include <stdio.h>
#include <errno.h>

#include <agar/core/core.h>
#ifdef AG_ENABLE_EXEC

#if defined(_XBOX)
# include <agar/core/xbox.h>
#elif defined(_WIN32)
# undef SLIST_ENTRY
# include <windows.h>
#endif

#include <agar/config/_mk_have_signal.h>
#ifdef _MK_HAVE_SIGNAL
#include <signal.h>
#endif

AG_ProcessID
AG_Execute(const char *file, char **argv)
{
#ifdef _XBOX 
	AG_LAUNCH_DATA launchData = { LDT_TITLE };
	char xbePath[AG_PATHNAME_MAX];
	char xbeName[AG_FILENAME_MAX];
	char argstr[AG_ARG_MAX];
	char mntDev[AG_PATHNAME_MAX];
	char *p;
	DWORD xbeID;
	int i = 0;

	if(!file) {
		AG_SetErrorS(_("No file provided for execution"));
		return (-1);
	}

	/* Get the destination xbe path */
	if(!argv || !argv[0] || (file && strcmp(file, argv[0]))) {
		p = (char *)file;
	} else {
		p = argv[0];
		i++;
	}

	/* Handle the command-line parameters */
	argstr[0] = '\0';
	if(argv) {
		while(argv[i] != NULL) {
			if( (AG_ARG_MAX - strlen(argstr) < strlen(argv[i]) + 1) ) {
				AG_SetError(_("%s: Arguments exceed AG_ARG_MAX (%d)"),
				    p, AG_ARG_MAX);
				return (-1);
			}
			Strlcat(argstr, argv[i], sizeof(argstr));
			Strlcat(argstr, " ", sizeof(argstr));
			i++;
		}
		if (Strlcpy(launchData.szCmdLine, argstr, sizeof(argstr)) >=
		    sizeof(argstr)) {
			AG_SetErrorS(_("Command argument is too long"));
			return (-1);
		}
	}

	/* Resolve the full xbe path */
	if((strlen(p) >= 7) && (!strncmp(p, "\\Device", 7))) {
		/* The xbe path was passed with the partition mapping */
		Strlcpy(xbePath, p, AG_PATHNAME_MAX);
	} else {
		char drive[3];
		char *dev;

		if(strlen(p) > 3 && isalpha(p[0]) && p[1] == ':' && p[2] == AG_PATHSEPCHAR) {
			/* The xbe path was passed with a drive letter */
			Strlcpy(drive, p, sizeof(drive));
			p = &p[3];
		} else {
			/* Path is relative */
			Strlcpy(drive, "D:", sizeof(drive));
		}
		if((dev = AG_XBOX_GetDeviceFromLogicalDrive(drive)) == NULL) {
			AG_SetErrorS(_("Invalid or unsupported drive letter. "
				       "Please provide a valid drive letter or the full device path"));
			return (-1);
		}
		Strlcpy(xbePath, dev, sizeof(xbePath));
		if(xbePath[strlen(xbePath) - 1] != AG_PATHSEPCHAR && p[0] != AG_PATHSEPCHAR)
			Strlcat(xbePath, AG_PATHSEP, sizeof(xbePath));
		Strlcat(xbePath, p, sizeof(xbePath));
		Free(dev);
	}

	/* Isolate the xbe name */
	p = strrchr(xbePath, '\\') + 1;
	if(!p) {
		AG_SetErrorS(_("No XBE Name included with path"));
		return (-1);
	}
	Strlcpy(xbeName, p, AG_FILENAME_MAX);

	/* mntDev will be the D: path for the new xbe */
	Strlcpy(mntDev, xbePath, p - xbePath);
	mntDev[p - xbePath] = '\0';

	/* Get the xbe ID */
	if((xbeID = AG_XBOX_GetXbeTitleId(xbePath)) == -1) {
		AG_SetErrorS(_("XBE is invalid or corrupted"));
		return (-1);
	}

	/* Complete the launch data */
	Strlcpy(launchData.szLauncherXBE, XeImageFileName->Buffer, XeImageFileName->Length + 1);
	Strlcpy(launchData.szLaunchedXBE, xbePath, sizeof(xbePath));

	/* Get the launcher ID */
	launchData.dwID = AG_XBOX_GetXbeTitleId(launchData.szLauncherXBE);

	launchData.magic = AG_LAUNCH_MAGIC;

	/* If this call succeeds the Agar application will be terminated so any
	   configs need to be saved prior to this call. */
	XWriteTitleInfoAndRebootA(xbeName, mntDev, LDT_TITLE, xbeID, &launchData);

	/* If we are here an error occurred */
	AG_SetError("XWriteTitleInfoAndRebootA failed");
	return (-1);

#elif defined(_WIN32)
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	char argstr[AG_ARG_MAX];
	int  i = 0;

	if(!file) {
		AG_SetErrorS(_("No file provided for execution"));
		return (-1);
	}

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if(file && strncmp(file, argv[0], strlen(file))) {
		Strlcpy(argstr, file, sizeof(argstr));
		Strlcat(argstr, " ", sizeof(argstr));
	} else {
		Strlcpy(argstr, argv[0], sizeof(argstr));
		Strlcat(argstr, " ", sizeof(argstr));
		i++;
	}

	// Add the command-line parameters
	while(argv[i] != NULL) {
		if( (AG_ARG_MAX - strlen(argstr) < strlen(argv[i]) + 1) ) {
			AG_SetError(_("%s: Arguments exceed AG_ARG_MAX (%d)"), 
			    file, AG_ARG_MAX);
			return (-1);
		}
		Strlcat(argstr, argv[i], sizeof(argstr));
		Strlcat(argstr, " ", sizeof(argstr));
		i++;
	}

	if(CreateProcessA(NULL, argstr, NULL, NULL, FALSE, 0, NULL, NULL,
	   &si, &pi) == 0) {
		AG_SetError(_("Failed to execute (%s)"),
		    AG_Strerror(GetLastError()));
		return (-1);
	}
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return (pi.dwProcessId);

#elif defined(HAVE_EXECVP) && !defined(_WIN32)
	AG_ProcessID pid;

	if(!file) {
		AG_SetErrorS(_("No file provided for execution"));
		return (-1);
	}

	if((pid = fork()) == -1) {
		AG_SetError(_("Fork failed (%s)"), AG_Strerror(errno));
		return (-1);
	} else if(pid == 0) {
		execvp(file, argv);

		// If we get here an error occurred
		_exit(EXIT_FAILURE);
	} else {
		return (pid);
	}

#endif
	AG_SetErrorS(_("AG_Execute() is not supported on this platform"));
	return (-1);
}

AG_ProcessID
AG_WaitOnProcess(AG_ProcessID pid, enum ag_exec_wait_type wait_t)
{
#if defined(_WIN32) && !defined(_XBOX)
	int time = 0;
	int res;
	DWORD status;
	HANDLE psHandle;

	if(wait_t == AG_EXEC_WAIT_INFINITE) {
		time = INFINITE;
	}

	if((psHandle = OpenProcess(SYNCHRONIZE |
	                           PROCESS_QUERY_INFORMATION,
	                           FALSE, pid)) == NULL) {
		AG_SetError(_("Unable to obtain process handle (%s)"), AG_Strerror(GetLastError()));
		return -1;
	}

	res = WaitForSingleObject(psHandle, time);

	if(res) {
		if(res == WAIT_TIMEOUT) {
			return 0;
		} else if(res == WAIT_FAILED) {
			AG_SetError(_("Wait on process failed (%s)"), AG_Strerror(GetLastError()));
			return -1;
		}
	}

	if(GetExitCodeProcess(psHandle, &status) == 0) {
		AG_SetError(_("Failed to obtain process exit code (%s)"), AG_Strerror(GetLastError()));
		return -1;
	} else if(status) {
		AG_SetError(_("Process exited with status (%d)"), (int)status);
		return -1;
	}

	CloseHandle(psHandle);

	return (pid);

#elif defined(HAVE_EXECVP) && !defined(_WIN32)
	int res;
	int status;
	int options = 0;

	if(wait_t == AG_EXEC_WAIT_IMMEDIATE) {
		options = WNOHANG;
	}

	res = waitpid(pid, &status, options);

	if(res == -1) {
		AG_SetError(_("waitpid() failed with error (%s)"), AG_Strerror(errno));
		return (-1);
	} else if(res > 0 && status) {
		if(WIFEXITED(status)) {
			AG_SetError(_("Process exited with status (%d)"), WEXITSTATUS(status));
		} else if(WIFSIGNALED(status)) {
			AG_SetError(_("Process terminated by signal (%d)"), WTERMSIG(status));
		} else {
			AG_SetErrorS(_("Process exited for unknown reason"));
		}
		return (-1);
	}
	return (res);

#endif
	AG_SetErrorS(_("AG_WaitOnProcess() is not supported on this platform"));
	return (-1);
}

int
AG_Kill(AG_ProcessID pid)
{
#if defined(_WIN32) && !defined(_XBOX)
	HANDLE psHandle;
#endif
	if(pid <= 0) {
		AG_SetErrorS(_("Invalid process id"));
		return (-1);
	}

#if defined(_WIN32) && !defined(_XBOX)
	if((psHandle = OpenProcess(SYNCHRONIZE |
	                           PROCESS_TERMINATE |
	                           PROCESS_QUERY_INFORMATION,
	                           FALSE, pid)) == NULL) {
		AG_SetError(_("Unable to obtain process handle (%s)"), AG_Strerror(GetLastError()));
		return -1;
	}

	if(TerminateProcess(psHandle, -1) == 0) {
		AG_SetError(_("Unable to kill process (%s)"), AG_Strerror(GetLastError()));
		return -1;
	}

	CloseHandle(psHandle);

	return (0);
#elif defined(_MK_HAVE_SIGNAL) && !defined(__CC65__)
	if (kill(pid, SIGKILL) == -1) {
		AG_SetError(_("Failed to kill process (%s)"), AG_Strerror(errno));
		return (-1);
	}
	return (0);
#endif
	AG_SetErrorS(_("AG_Kill() is not supported on this platform"));
	return (-1);
}
#endif /* AG_ENABLE_EXEC */
