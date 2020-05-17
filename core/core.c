/*
 * Copyright (c) 2001-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Agar-Core initialization routines.
 */

#include <agar/config/version.h>
#include <agar/config/release.h>
#include <agar/config/enable_nls.h>
#include <agar/config/localedir.h>
#include <agar/config/ag_threads.h>
#include <agar/config/have_clock_gettime.h>
#include <agar/config/have_nanosleep.h>
#include <agar/config/have_gettimeofday.h>
#include <agar/config/have_select.h>
#include <agar/config/have_db4.h>
#include <agar/config/have_db5.h>
#include <agar/config/have_getpwuid.h>
#include <agar/config/have_getuid.h>
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

#ifdef AG_SERIALIZATION
AG_Config *agConfig = NULL;			/* Global Agar config data */
#endif

void (*agAtexitFunc)(void) = NULL;		/* User exit function */
void (*agAtexitFuncEv)(AG_Event *) = NULL;	/* User exit handler */
char *agProgName = NULL;			/* Optional application name */

#ifdef AG_VERBOSITY
int agVerbose = 0;				/* Verbose console output */
#endif
#ifdef AG_TIMERS
int agSoftTimers = 0;				/* Disable hardware timers */
#endif

const char *agMemoryModelNames[] = {
	N_("Small"),				/* AG_SMALL */
	N_("Medium"),				/* AG_MEDIUM */
	N_("Large")				/* AG_LARGE */
};

/* Initialize the Agar-Core library */
int
AG_InitCore(const char *progname, Uint flags)
{
#ifdef AG_SERIALIZATION
	if (agConfig != NULL) {
		AG_SetError("AG_Core already initialized");
		return (-1);
	}
#endif
#ifdef AG_VERBOSITY
	if (flags & AG_VERBOSE)
		agVerbose = 1;
#endif
#ifdef AG_TIMERS
	if (flags & AG_SOFT_TIMERS)
		agSoftTimers = 1;
#endif
	/* Copy in any specified program name. */
	if (progname != NULL) {
		if ((agProgName = TryStrdup(progname)) == NULL)
			return (-1);
	} else {
		agProgName = NULL;
	}
#ifdef ENABLE_NLS
	/* Bind to the proper translation */
	bindtextdomain("agar", LOCALEDIR);
	bind_textdomain_codeset("agar", "UTF-8");
	textdomain("agar");
#endif
	/* Initialize AG_Error(3), AG_String(3) and AG_Event(3) interfaces. */
	if (AG_InitErrorSubsystem() == -1 ||
	    AG_InitStringSubsystem() == -1) {
		return (-1);
	}
#ifdef AG_EVENT_LOOP
	if (AG_InitEventSubsystem(flags) == -1)
		return (-1);
#endif
	/* Fetch CPU information. */
	AG_GetCPUInfo(&agCPU);

#ifdef AG_THREADS
	/* Initialize threads. */
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
# ifdef AG_ENABLE_DSO
	AG_MutexInitRecursive(&agDSOLock);
# endif
#endif /* AG_THREADS */

	/* Initialize object classes and register Agar-Core classes */
	AG_InitClassTbl();
#ifdef AG_SERIALIZATION
	AG_RegisterClass(&agConfigClass);
	AG_RegisterClass(&agDbClass);
# if defined(HAVE_DB4) || defined(HAVE_DB5)
	AG_RegisterClass(&agDbHashClass);
	AG_RegisterClass(&agDbBtreeClass);
# endif
#endif /* AG_SERIALIZATION */

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

#ifdef AG_USER
	/* Select the user account interface routines. */
# if defined(_XBOX)
	AG_SetUserOps(&agUserOps_xbox);
# elif defined(_WIN32) && defined(HAVE_CSIDL)
	AG_SetUserOps(&agUserOps_win32);
# elif defined(HAVE_GETENV) || (defined(HAVE_GETPWUID) && defined(HAVE_GETUID))
	if (flags & AG_POSIX_USERS) {			/* Prefer posix */
#  if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
		AG_SetUserOps(&agUserOps_posix);
#  else
		AG_SetUserOps(&agUserOps_getenv);
#  endif
	} else {					/* Prefer getenv */
#  if defined(HAVE_GETENV)
		AG_SetUserOps(&agUserOps_getenv);
#  else
		AG_SetUserOps(&agUserOps_posix);
#  endif
	}
# else /* !AG_USER */
	AG_SetUserOps(&agUserOps_dummy);
# endif
#endif /* AG_USER */

#ifdef AG_TIMERS
	/* Initialize the timer system */
	AG_InitTimers();
#endif
#ifdef AG_SERIALIZATION
	/* Initialize the AG_DataSource(3) interface */
	AG_DataSourceInitSubsystem();

	/* Initialize the global AG_Config(3) object. */
	if ((agConfig = TryMalloc(sizeof(AG_Config))) == NULL ||
	    AG_ConfigInit(agConfig, flags) == -1)
		return (-1);
#endif
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
#ifdef AG_SERIALIZATION
	if (agConfig == NULL)
		return;
#endif
	if (agAtexitFunc != NULL) { agAtexitFunc(); }
	if (agAtexitFuncEv != NULL) { agAtexitFuncEv(NULL); }
#ifdef AG_USER
	if (agUserOps != NULL && agUserOps->destroy != NULL) {
		agUserOps->destroy();
		agUserOps = NULL;
	}
#endif
#ifdef AG_SERIALIZATION
	AG_ObjectDestroy(agConfig);
	agConfig = NULL;
	AG_DataSourceDestroySubsystem();
#endif
#ifdef AG_TIMERS
	AG_DestroyTimers();
#endif
	AG_DestroyClassTbl();

#ifdef AG_THREADS
	pthread_mutexattr_destroy(&agRecursiveMutexAttr);
# ifdef AG_ENABLE_DSO
	AG_MutexDestroy(&agDSOLock);
# endif
#endif
#ifdef AG_EVENT_LOOP
	AG_DestroyEventSubsystem();
#endif
	AG_DestroyStringSubsystem();
	AG_DestroyErrorSubsystem();

	Free(agProgName);
	agProgName = NULL;
}

void
AG_GetVersion(AG_AgarVersion *ver)
{
	ver->major = AGAR_MAJOR_VERSION;
	ver->minor = AGAR_MINOR_VERSION;
	ver->patch = AGAR_PATCHLEVEL;
	ver->rev = 0;
	ver->release = RELEASE;
}
