/*
 * Copyright (c) 2001-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <config/have_setlocale.h>
#include <config/localedir.h>
#include <config/version.h>
#include <config/network.h>
#include <config/release.h>

#ifdef THREADS
# include <config/have_pthreads_xopen.h>
# include <config/have_pthread_mutex_recursive.h>
# include <config/have_pthread_mutex_recursive_np.h>
#endif

#include <core/core.h>
#include <core/config.h>
#ifdef NETWORK
# include <core/rcs.h>
#endif

#include <stdio.h>
#ifdef HAVE_SETLOCALE
# include <locale.h>
#endif

#ifdef THREADS
pthread_mutexattr_t agRecursiveMutexAttr;	/* Recursive mutex attributes */
#endif

const char *agProgName = "";
AG_Config *agConfig;
AG_Object *agWorld;
void (*agAtexitFunc)(void) = NULL;
void (*agAtexitFuncEv)(AG_Event *) = NULL;
AG_Mutex agLinkageLock;
AG_Mutex agTimingLock;
int agVerbose = 0;
int agTerminating = 0;

int
AG_InitCore(const char *progname, Uint flags)
{
	if (flags & AG_CORE_VERBOSE)
		agVerbose = 1;

	agProgName = progname;

#ifdef HAVE_SETLOCALE
	setlocale(LC_ALL, "");
#endif
#ifdef ENABLE_NLS
	bindtextdomain("agar", LOCALEDIR);
	textdomain("agar");
#endif

	AG_InitError();
	AG_GetCPUInfo(&agCPU);

#ifdef THREADS
	pthread_mutexattr_init(&agRecursiveMutexAttr);
# if defined(HAVE_PTHREAD_MUTEX_RECURSIVE_NP)
	pthread_mutexattr_settype(&agRecursiveMutexAttr,
	    PTHREAD_MUTEX_RECURSIVE_NP);
# else
	pthread_mutexattr_settype(&agRecursiveMutexAttr,
	    PTHREAD_MUTEX_RECURSIVE);
# endif
	AG_MutexInitRecursive(&agLinkageLock);
	AG_MutexInitRecursive(&agTimingLock);
#endif /* THREADS */

	if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_NOPARACHUTE) != 0) {
		AG_SetError("SDL_Init: %s", SDL_GetError());
		return (-1);
	}

	AG_InitClassTbl();
	AG_RegisterClass(&agConfigClass);

	agConfig = Malloc(sizeof(AG_Config));
	AG_ConfigInit(agConfig);

	agWorld = AG_ObjectNew(NULL, "world", &agObjectClass);
	AG_ObjectRemain(agWorld, AG_OBJECT_REMAIN_DATA);
	
	AG_ObjectLoad(agConfig);
#ifdef NETWORK
	AG_InitNetwork(0);
#endif
	return (0);
}

#ifdef NETWORK
int
AG_InitNetwork(Uint flags)
{
	AG_RcsInit();
	return (0);
}
#endif /* NETWORK */

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

/* Request a graceful shutdown of the application. */
void
AG_Quit(void)
{
	SDL_Event nev;

	nev.type = SDL_QUIT;
	SDL_PushEvent(&nev);
}

/* Immediately clean up and exit the application. */
void
AG_Destroy(void)
{
	if (agAtexitFunc != NULL)
		agAtexitFunc();
	if (agAtexitFuncEv != NULL)
		agAtexitFuncEv(NULL);

#ifdef NETWORK
	AG_RcsDestroy();
#endif

	AG_ObjectDestroy(agWorld);
	AG_ObjectDestroy(agConfig);

	AG_MutexDestroy(&agTimingLock);
	AG_MutexDestroy(&agLinkageLock);

	AG_DestroyError();
	AG_DestroyClassTbl();
	SDL_Quit();
	exit(0);
}

void
AG_GetVersion(AG_AgarVersion *ver)
{
	ver->major = AGAR_MAJOR_VERSION;
	ver->minor = AGAR_MINOR_VERSION;
	ver->patch = AGAR_PATCHLEVEL;
	ver->release = RELEASE;
}
