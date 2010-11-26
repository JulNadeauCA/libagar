/*
 * Copyright (c) 2001-2010 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Core initialization routines.
 */

#include <config/version.h>
#include <config/release.h>
#include <config/enable_nls.h>
#include <config/localedir.h>
#include <config/ag_network.h>
#include <config/ag_threads.h>
#include <config/have_gettimeofday.h>
#include <config/have_select.h>
#include <config/have_cygwin.h>
#include <config/have_clock_gettime.h>

#ifdef AG_THREADS
#include <config/have_pthreads_xopen.h>
#include <config/have_pthread_mutex_recursive.h>
#include <config/have_pthread_mutex_recursive_np.h>
#endif

#include "core.h"
#include "config.h"
#include "dso.h"
#ifdef AG_NETWORK
#include "rcs.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef AG_THREADS
pthread_mutexattr_t agRecursiveMutexAttr;	/* Recursive mutex attributes */
#endif

AG_Config *agConfig;				/* Global Agar config data */
void (*agAtexitFunc)(void) = NULL;		/* User exit function */
void (*agAtexitFuncEv)(AG_Event *) = NULL;	/* User exit handler */
char *agProgName = NULL;			/* User program name */

int agVerbose = 0;		/* Verbose console output */
int agTerminating = 0;		/* Application is exiting */

int
AG_InitCore(const char *progname, Uint flags)
{
	if (flags & AG_VERBOSE)
		agVerbose = 1;

	if ((agProgName = TryStrdup(progname)) == NULL)
		return (-1);

#ifdef ENABLE_NLS
	bindtextdomain("agar", LOCALEDIR);
	bind_textdomain_codeset("agar", "UTF-8");
	textdomain("agar");
#endif

	AG_InitError();
	AG_GetCPUInfo(&agCPU);

#ifdef AG_THREADS
#ifdef _XBOX
	ptw32_processInitialize();
#endif
	pthread_mutexattr_init(&agRecursiveMutexAttr);
# if defined(HAVE_PTHREAD_MUTEX_RECURSIVE_NP)
	pthread_mutexattr_settype(&agRecursiveMutexAttr,
	    PTHREAD_MUTEX_RECURSIVE_NP);
# else
	pthread_mutexattr_settype(&agRecursiveMutexAttr,
	    PTHREAD_MUTEX_RECURSIVE);
# endif
	AG_MutexInitRecursive(&agDSOLock);
#endif /* AG_THREADS */

	AG_InitClassTbl();
	AG_RegisterClass(&agConfigClass);
	AG_RegisterClass(&agDbObjectClass);
	AG_RegisterClass(&agDbClass);

#if defined(HAVE_GETTIMEOFDAY) && !defined(HAVE_CYGWIN)
# if defined(AG_THREADS) && defined(HAVE_CLOCK_GETTIME)
	AG_SetTimeOps(&agTimeOps_condwait);
# else
#  if defined(HAVE_SELECT)
	AG_SetTimeOps(&agTimeOps_gettimeofday);
#  endif
# endif
#elif defined(_WIN32)
	AG_SetTimeOps(&agTimeOps_win32);
#else
	AG_SetTimeOps(&agTimeOps_dummy);
#endif

	AG_InitTimeouts();
	AG_DataSourceInitSubsystem();

	if ((agConfig = TryMalloc(sizeof(AG_Config))) == NULL) {
		return (-1);
	}
	if (AG_ConfigInit(agConfig, flags) == -1) {
		return (-1);
	}
	if (!(flags & AG_NO_CFG_AUTOLOAD)) {
		(void)AG_ConfigLoad();
	}
#ifdef AG_NETWORK
	AG_InitNetwork(0);
#endif
	return (0);
}

#ifdef AG_NETWORK
int
AG_InitNetwork(Uint flags)
{
	AG_RcsInit();
	return (0);
}
#endif /* AG_NETWORK */

/* Register a function to invoke in AG_Quit(). */
void
AG_AtExitFunc(void (*func)(void))
{
	agAtexitFunc = func;
}

void
AG_AtExitFuncEv(void (*func)(AG_Event *))
{
	agAtexitFuncEv = func;
}

/* Immediately terminate the application. */
void
AG_Quit(void)
{
	AG_Destroy();
	exit(0);
}

/* Clean up the resources allocated by Agar-Core. */
void
AG_Destroy(void)
{
	if (agAtexitFunc != NULL) { agAtexitFunc(); }
	if (agAtexitFuncEv != NULL) { agAtexitFuncEv(NULL); }

#ifdef AG_NETWORK
	AG_RcsDestroy();
#endif
	AG_ObjectDestroy(agConfig);
	AG_DataSourceDestroySubsystem();

	AG_DestroyTimeouts();
	AG_DestroyError();
	AG_DestroyClassTbl();
	Free(agProgName);
}

void
AG_GetVersion(AG_AgarVersion *ver)
{
	ver->major = AGAR_MAJOR_VERSION;
	ver->minor = AGAR_MINOR_VERSION;
	ver->patch = AGAR_PATCHLEVEL;
	ver->release = RELEASE;
}
