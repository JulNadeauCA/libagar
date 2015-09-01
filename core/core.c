/*
 * Copyright (c) 2001-2014 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/config/version.h>
#include <agar/config/release.h>
#include <agar/config/enable_nls.h>
#include <agar/config/localedir.h>
#include <agar/config/ag_threads.h>
#include <agar/config/ag_network.h>
#include <agar/config/have_clock_gettime.h>
#include <agar/config/have_nanosleep.h>
#include <agar/config/have_gettimeofday.h>
#include <agar/config/have_select.h>
#include <agar/config/have_db4.h>
#include <agar/config/have_getpwuid.h>
#include <agar/config/have_getuid.h>
#include <agar/config/have_getaddrinfo.h>
#include <agar/config/have_winsock1.h>
#include <agar/config/have_winsock2.h>
#include <agar/config/have_csidl.h>
#ifdef AG_THREADS
# include <agar/config/have_pthreads_xopen.h>
# include <agar/config/have_pthread_mutex_recursive.h>
# include <agar/config/have_pthread_mutex_recursive_np.h>
#endif

#include <agar/core/core.h>
#include <agar/core/config.h>
#include <agar/core/dso.h>

#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
# include <unistd.h>
#endif

#ifdef AG_THREADS
pthread_mutexattr_t agRecursiveMutexAttr;	/* Recursive mutex attributes */
AG_Thread agEventThread;			/* Event-processing thread */
#endif

AG_Config *agConfig;				/* Global Agar config data */
void (*agAtexitFunc)(void) = NULL;		/* User exit function */
void (*agAtexitFuncEv)(AG_Event *) = NULL;	/* User exit handler */
char *agProgName = NULL;			/* Optional application name */

int agVerbose = 0;				/* Verbose console output */
int agSoftTimers = 0;				/* Disable hardware timers */

int
AG_InitCore(const char *progname, Uint flags)
{
	if (flags & AG_VERBOSE)
		agVerbose = 1;
	if (flags & AG_SOFT_TIMERS)
		agSoftTimers = 1;

	if (progname != NULL) {
		if ((agProgName = TryStrdup(progname)) == NULL)
			return (-1);
	} else {
		agProgName = NULL;
	}

#ifdef ENABLE_NLS
	bindtextdomain("agar", LOCALEDIR);
	bind_textdomain_codeset("agar", "UTF-8");
	textdomain("agar");
#endif

	if (AG_InitErrorSubsystem() == -1 ||
	    AG_InitStringSubsystem() == -1 ||
	    AG_InitEventSubsystem(flags) == -1) {
		return (-1);
	}
	AG_GetCPUInfo(&agCPU);

	/* Initialize the thread resources. */
#ifdef AG_THREADS
	agEventThread = AG_ThreadSelf();		/* Main thread */
# ifdef _XBOX
	ptw32_processInitialize();
# endif
	pthread_mutexattr_init(&agRecursiveMutexAttr);
# ifdef HAVE_PTHREAD_MUTEX_RECURSIVE_NP
	pthread_mutexattr_settype(&agRecursiveMutexAttr,
	    PTHREAD_MUTEX_RECURSIVE_NP);
# else
	pthread_mutexattr_settype(&agRecursiveMutexAttr,
	    PTHREAD_MUTEX_RECURSIVE);
# endif
	AG_MutexInitRecursive(&agDSOLock);
#endif /* AG_THREADS */

	/* Register the object classes from ag_core. */
	AG_InitClassTbl();
	AG_RegisterClass(&agConfigClass);
	AG_RegisterClass(&agDbClass);
#ifdef HAVE_DB4
	AG_RegisterClass(&agDbHashClass);
	AG_RegisterClass(&agDbBtreeClass);
#endif

	/* Select the default AG_Time(3) backend. */
#if defined(_WIN32)
	AG_SetTimeOps(&agTimeOps_win32);
#elif defined(HAVE_CLOCK_GETTIME) && defined(HAVE_NANOSLEEP)
	AG_SetTimeOps(&agTimeOps_posix);
#elif defined(HAVE_GETTIMEOFDAY) && defined(HAVE_SELECT)
	AG_SetTimeOps(&agTimeOps_gettimeofday);
#else
	AG_SetTimeOps(&agTimeOps_dummy);
#endif
	
	/* Select the network access routines. */
#ifdef AG_NETWORK
	{
		int rv;
# if defined(HAVE_WINSOCK2)
		rv = AG_InitNetworkSubsystem(&agNetOps_winsock2);
# elif defined(HAVE_WINSOCK1)
		rv = AG_InitNetworkSubsystem(&agNetOps_winsock1);
# elif defined(HAVE_GETADDRINFO)
		rv = AG_InitNetworkSubsystem(&agNetOps_bsd);
# else
		rv = AG_InitNetworkSubsystem(&agNetOps_dummy);
# endif
		if (rv != 0)
			return (-1);
	}
#endif
	
	/* Select the user account interface routines. */
#if defined(_XBOX)
	AG_SetUserOps(&agUserOps_xbox);
#elif defined(_WIN32) && defined(HAVE_CSIDL)
	AG_SetUserOps(&agUserOps_win32);
#elif defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
	AG_SetUserOps(&agUserOps_posix);
#else
	AG_SetUserOps(&agUserOps_dummy);
#endif

	AG_InitTimers();
	AG_DataSourceInitSubsystem();

	if ((agConfig = TryMalloc(sizeof(AG_Config))) == NULL) {
		return (-1);
	}
	if (AG_ConfigInit(agConfig, flags) == -1) {
		return (-1);
	}
	return (0);
}

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

	AG_ObjectDestroy(agConfig);
	AG_DataSourceDestroySubsystem();
	AG_DestroyTimers();
	if (agUserOps != NULL && agUserOps->destroy != NULL) {
		agUserOps->destroy();
		agUserOps = NULL;
	}
#ifdef AG_NETWORK
	AG_DestroyNetworkSubsystem();
#endif
	AG_DestroyClassTbl();
#ifdef AG_THREADS
	pthread_mutexattr_destroy(&agRecursiveMutexAttr);
	AG_MutexDestroy(&agDSOLock);
#endif
	AG_DestroyEventSubsystem();
	AG_DestroyStringSubsystem();
	AG_DestroyErrorSubsystem();
	Free(agProgName); agProgName = NULL;
}

void
AG_GetVersion(AG_AgarVersion *ver)
{
	ver->major = AGAR_MAJOR_VERSION;
	ver->minor = AGAR_MINOR_VERSION;
	ver->patch = AGAR_PATCHLEVEL;
	ver->release = RELEASE;
}
