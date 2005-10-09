/*	$Csoft: engine.c,v 1.167 2005/10/05 02:00:35 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <config/have_x11.h>
#include <config/have_setlocale.h>
#include <config/localedir.h>
#include <config/version.h>
#include <config/network.h>

#include <compat/setenv.h>
#include <compat/strlcat.h>

#include <core/core.h>
#include <core/config.h>
#include <core/view.h>
#include <core/typesw.h>

#include <gui/primitive.h>
#include <gui/cursors.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef MAP
#include <game/map/map.h>
#endif
#ifdef EDITION
#include <game/map/mapedit.h>
#endif
#ifdef NETWORK
#include <core/rcs.h>
#endif
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif
#ifdef HAVE_X11
#include <SDL_syswm.h>
#endif

#ifdef THREADS
pthread_mutexattr_t agRecursiveMutexAttr;	/* Recursive mutex attributes */
#endif

const char *agProgName = "";
AG_Config *agConfig;
AG_Object *agWorld;
AG_Object agIconMgr;
void (*agAtexitFunc)(void) = NULL;
AG_Mutex agLinkageLock;
AG_Mutex agTimingLock;
int agServerMode = 0;

int
AG_InitCore(const char *progname, u_int flags)
{
	agProgName = progname;

#ifdef HAVE_SETLOCALE
	setlocale(LC_ALL, "");
#endif
#ifdef ENABLE_NLS
	bindtextdomain("agar", LOCALEDIR);
	textdomain("agar");
#endif

	AG_InitError();

#ifdef THREADS
	pthread_mutexattr_init(&agRecursiveMutexAttr);
	pthread_mutexattr_settype(&agRecursiveMutexAttr,
	    PTHREAD_MUTEX_RECURSIVE);
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
	AG_ObjectLoad(agConfig);

	printf("\n");
	printf(_("Architecture extensions: "));
	if (SDL_HasRDTSC()) { printf("RDTSC "); }
	if (SDL_HasMMX()) { printf("MMX "); }
	if (SDL_HasMMXExt()) { printf("MMXExt "); }
	if (SDL_Has3DNow()) { printf("3DNow "); }
	if (SDL_Has3DNowExt()) { printf("3DNowExt "); }
	if (SDL_HasSSE()) { printf("SSE "); }
	if (SDL_HasSSE2()) { printf("SSE2 "); }
	if (SDL_HasAltiVec()) { printf("AltiVec "); }
	printf(".\n");

	agWorld = AG_ObjectNew(NULL, "world");
	AG_ObjectRemain(agWorld, AG_OBJECT_REMAIN_DATA);
	return (0);
}

int
AG_InitVideo(int w, int h, int bpp, u_int flags)
{
	extern int agBgPopupMenu;

#ifdef HAVE_X11
	setenv("SDL_VIDEO_X11_WMCLASS", agProgName, 1);
#endif
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		AG_SetError("SDL_INIT_VIDEO: %s", SDL_GetError());
		return (-1);
	}
	SDL_WM_SetCaption(agProgName, agProgName);

	if ((agVideoInfo = SDL_GetVideoInfo()) != NULL) {
		char accel[2048];
		char unaccel[2048];
		size_t size = 2048;

		agVideoFmt = agVideoInfo->vfmt;

		if (agVideoInfo->wm_available) {
			printf(_("Window manager is available.\n"));
		} else {
			printf(_("Window manager is unavailable.\n"));
		}
			
		if (agVideoInfo->hw_available) {
			printf(_("Hardware surfaces are available.\n"));
		} else {
			printf(_("Hardware surfaces are unavailable.\n"));
		}
		
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

	if (flags & AG_VIDEO_BGPOPUPMENU) { agBgPopupMenu = 1; }
	
	AG_ObjectInit(&agIconMgr, "object", "IconMgr", NULL);
	AG_WireGfx(&agIconMgr, "/core-icons");
	return (0);
}

int
AG_InitConfigWin(u_int flags)
{
	AG_ConfigWindow(agConfig, flags);
	return (0);
}

/* Initialize graphics and input devices. */
int
AG_InitInput(u_int flags)
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

Uint8
AG_MouseGetState(int *x, int *y)
{
	Uint8 rv;

	rv = SDL_GetMouseState(x, y);
#if defined(__APPLE__) && defined(HAVE_OPENGL)
	if (agView->opengl && y != NULL)
		*y = agView->h - *y;
#endif
	return (rv);
}

int
AG_InitNetwork(u_int flags)
{
#if defined(DEBUG) && defined(NETWORK) && defined(THREADS)
	if ((flags & AG_INIT_DEBUG_SERVER) && agServerMode) {
		extern int AG_DebugServerStart(void);
		AG_DebugServerStart();
	}
#endif
#if defined(NETWORK)
	if (flags & AG_INIT_RCS)
		AG_RcsInit();
#endif
	return (0);
}

/* Register a function to invoke in AG_Quit(). */
void
AG_AtExitFunc(void (*func)(void))
{
	agAtexitFunc = func;
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

#if defined(MAP) && defined(EDITION)
	if (agEditMode)
		AG_ObjectSave(&agMapEditor);
#endif
#ifdef NETWORK
	AG_RcsDestroy();
#endif

	AG_ObjectDestroy(agWorld);
	AG_TextDestroy();
	AG_ViewDestroy();
	
	AG_ColorsDestroy();
	AG_CursorsDestroy();

	AG_ObjectDestroy(agConfig);
	Free(agConfig, M_OBJECT);

/*	AG_MutexDestroy(&agLinkageLock); */
	AG_MutexDestroy(&agTimingLock);

	AG_DestroyError();
	AG_DestroyTypeSw();
	SDL_Quit();
	exit(0);
}

