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
#include <config/have_pthreads_xopen.h>
#include <config/have_pthread_mutex_recursive.h>
#include <config/have_pthread_mutex_recursive_np.h>
#include <config/have_sdl_cpuinfo.h>
#include <config/have_x11.h>

#include <core/core.h>
#include <core/config.h>
#include <core/view.h>
#include <core/typesw.h>

#include <gui/primitive.h>
#include <gui/cursors.h>
#include <gui/colors.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef NETWORK
#include <core/rcs.h>
#endif
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif

/*
 * Force synchronous X11 events. Reduces performance, but useful for
 * debugging things like accesses to illegal video regions.
 */
/* #define SYNC_X11_EVENTS */

#if defined(HAVE_X11) && defined(SYNC_X11_EVENTS)
#include <SDL_syswm.h>
#include <X11/Xlib.h>
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
#if defined(HAVE_PTHREAD_MUTEX_RECURSIVE_NP)
	pthread_mutexattr_settype(&agRecursiveMutexAttr,
	    PTHREAD_MUTEX_RECURSIVE_NP);
#elif defined(HAVE_PTHREAD_MUTEX_RECURSIVE) || defined(HAVE_PTHREADS_XOPEN)
	pthread_mutexattr_settype(&agRecursiveMutexAttr,
	    PTHREAD_MUTEX_RECURSIVE);
#else
#error "THREADS options requires recursive mutexes"
#endif
	AG_MutexInitRecursive(&agLinkageLock);
	AG_MutexInitRecursive(&agTimingLock);
#endif
	if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_NOPARACHUTE) != 0) {
		AG_SetError("SDL_Init: %s", SDL_GetError());
		return (-1);
	}

	AG_InitTypeSw();
	
	agConfig = Malloc(sizeof(AG_Config), M_OBJECT);
	AG_ConfigInit(agConfig);

	agWorld = AG_ObjectNew(NULL, "world", &agObjectOps);
	AG_ObjectRemain(agWorld, AG_OBJECT_REMAIN_DATA);
	
	AG_ObjectLoad(agConfig);

	if (agVerbose) {
		printf("\n");
	}
#ifdef NETWORK
	AG_InitNetwork(0);
#endif
	return (0);
}

#if defined(HAVE_X11) && defined(SYNC_X11_EVENTS)
static int
AG_X11_ErrorHandler(Display *disp, XErrorEvent *event)
{
	printf("Caught X11 error!\n");
	abort();
}
#endif

int
AG_InitVideo(int w, int h, int bpp, Uint flags)
{
	char path[MAXPATHLEN];
	extern int agBgPopupMenu;
	char *s;

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		AG_SetError("SDL_INIT_VIDEO: %s", SDL_GetError());
		return (-1);
	}
	SDL_WM_SetCaption(agProgName, agProgName);

	agVideoInfo = SDL_GetVideoInfo();
#ifdef DEBUG
	if (agVerbose) {
		char accel[2048];
		char unaccel[2048];
		size_t size = 2048;

		agVideoFmt = agVideoInfo->vfmt;

		printf(_("Window manager is %s.\n"),
		    agVideoInfo->wm_available ?
		    _("available") : _("unavailable"));
		printf(_("Hardware surfaces are %s.\n"),
		    agVideoInfo->hw_available ?
		    _("available") : _("unavailable"));
		
		accel[0] = '\0';
		unaccel[0] = '\0';

		strlcat(agVideoInfo->blit_hw ? accel : unaccel,
		    _("\tSDL hardware blits\n"), size);
		strlcat(agVideoInfo->blit_hw_CC ? accel : unaccel,
		    _("\tSDL hardware->hardware colorkey blits\n"), size);
		strlcat(agVideoInfo->blit_hw_A ? accel : unaccel,
		    _("\tSDL hardware->hardware alpha blits\n"), size);
		strlcat(agVideoInfo->blit_sw ? accel : unaccel,
		    _("\tSDL software->hardware blits\n"), size);
		strlcat(agVideoInfo->blit_sw_CC ? accel : unaccel,
		    _("\tSDL software->hardware colorkey blits\n"), size);
		strlcat(agVideoInfo->blit_sw_A ? accel : unaccel,
		    _("\tSDL software->hardware alpha blits\n"), size);
		strlcat(agVideoInfo->blit_fill ? accel : unaccel,
		    _("\tSDL color fills\n"), size);

		if (accel[0] != '\0')
			printf(_("Accelerated operations:\n%s"), accel);
		if (unaccel[0] != '\0')
			printf(_("Unaccelerated operations:\n%s"), unaccel);

		printf("\n");
	}
#endif

	if (flags & (AG_VIDEO_OPENGL|AG_VIDEO_OPENGL_OR_SDL)) {
#ifdef HAVE_OPENGL
		AG_SetBool(agConfig, "view.opengl", 1);
#else
		if ((flags & AG_VIDEO_OPENGL_OR_SDL) == 0)
			fatal("Agar OpenGL support is not compiled in");
#endif
	}
	if (AG_ViewInit(w, h, bpp, flags) == -1 || AG_TextInit() == -1) {
		return (-1);
	}
	AG_ColorsInit();
	AG_InitPrimitives();
	AG_CursorsInit();

	strlcpy(path, AG_String(agConfig, "save-path"), sizeof(path));
	strlcat(path, AG_PATHSEP, sizeof(path));
	strlcat(path, "gui-colors.acs", sizeof(path));
	(void)AG_ColorsLoad(path);

	/* Fill the background. */
#ifdef HAVE_OPENGL
	if (agView->opengl) {
		Uint8 r, g, b;

		SDL_GetRGB(AG_COLOR(BG_COLOR), agVideoFmt, &r, &g, &b);
		AG_LockGL();
		glClearColor(r/255.0, g/255.0, b/255.0, 1.0);
		AG_UnlockGL();
	} else
#endif
	{
		SDL_FillRect(agView->v, NULL, AG_COLOR(BG_COLOR));
		SDL_UpdateRect(agView->v, 0, 0, agView->w, agView->h);
	}

	if (flags & AG_VIDEO_BGPOPUPMENU) { agBgPopupMenu = 1; }

#if defined(HAVE_X11) && defined(SYNC_X11_EVENTS)
	{	
		SDL_SysWMinfo wminfo;
		if (SDL_GetWMInfo(&wminfo) &&
		    wminfo.subsystem == SDL_SYSWM_X11) {
			dprintf("Enabling synchronous X11 events\n");
			XSynchronize(wminfo.info.x11.display, True);
			XSetErrorHandler(AG_X11_ErrorHandler);
		}
	}
#endif

	AG_IconMgrInit(&agIconMgr, "core-icons");
	if (AG_IconMgrLoadFromDenXCF(&agIconMgr, "core-icons") == -1) {
		fatal("Unable to load icons: %s", AG_GetError());
	}
	return (0);
}

int
AG_InitConfigWin(Uint flags)
{
	AG_ConfigWindow(agConfig, flags);
	return (0);
}

/* Initialize graphics and input devices. */
int
AG_InitInput(Uint flags)
{
	extern int agKbdUnicode;			/* config.c */
	int i, n, njoys;

	SDL_EnableUNICODE(agKbdUnicode || (flags & AG_FORCE_UNICODE));

	if (AG_Bool(agConfig, "input.joysticks") ||
	    (flags & AG_FORCE_JOYSTICK)) {
		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == 0) {
			n = SDL_NumJoysticks();
			for (i = 0, njoys = 0; i < n; i++) {
				if (SDL_JoystickOpen(i) != NULL)
					njoys++;
			}
			if (njoys > 0)
				SDL_JoystickEventState(SDL_ENABLE);
		}
	}
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

	if (agView != NULL) {
		AG_TextDestroy();
		AG_ViewDestroy();
		AG_ColorsDestroy();
		AG_CursorsDestroy();
		AG_ObjectDestroy(&agIconMgr);
	}

	AG_ObjectDestroy(agConfig);
	Free(agConfig, M_OBJECT);

/*	AG_MutexDestroy(&agLinkageLock); */
	AG_MutexDestroy(&agTimingLock);

	AG_DestroyError();
	AG_DestroyTypeSw();
	SDL_Quit();
	exit(0);
}

