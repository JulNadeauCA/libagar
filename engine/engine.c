/*	$Csoft: engine.c,v 1.161 2005/09/18 04:08:22 vedge Exp $	*/

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

#include <engine/engine.h>
#include <engine/input.h>
#include <engine/config.h>
#include <engine/view.h>
#include <engine/typesw.h>

#ifdef MAP
#include <engine/map/map.h>
#endif

#ifdef EDITION
#include <engine/map/mapedit.h>
#endif

#ifdef NETWORK
#include <engine/rcs.h>
#endif

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/textbox.h>
#include <engine/widget/keycodes.h>
#include <engine/widget/primitive.h>
#include <engine/widget/cursors.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif
#ifdef HAVE_X11
#include <SDL_syswm.h>
#endif

#ifdef THREADS
pthread_mutexattr_t	recursive_mutexattr;	/* Recursive mutex attributes */
#endif

const char *progname = "untitled";
struct config *config;
struct object *world;
pthread_mutex_t linkage_lock;
struct object engine_icons;
void (*engine_atexit_func)(void) = NULL;
extern pthread_mutex_t timeout_lock;

enum gfx_engine gfx_mode = GFX_ENGINE_GUI;
int server_mode = 0;

/* Initialize the Agar engine. */
int
engine_preinit(const char *name)
{
	static int inited = 0;
	
	if (inited) {
		error_set("The engine is already initialized.");
		return (-1);
	}

#ifdef HAVE_SETLOCALE
	setlocale(LC_ALL, "");
#endif
#ifdef ENABLE_NLS
	bindtextdomain("agar", LOCALEDIR);
	textdomain("agar");
#endif
	error_init();

#ifdef THREADS
	pthread_mutexattr_init(&recursive_mutexattr);
	pthread_mutexattr_settype(&recursive_mutexattr,
	    PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&linkage_lock, &recursive_mutexattr);
	pthread_mutex_init(&timeout_lock, &recursive_mutexattr);
#endif
	if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_NOPARACHUTE) != 0) {
		error_set("SDL_Init: %s", SDL_GetError());
		return (-1);
	}
#ifdef HAVE_X11
	setenv("SDL_VIDEO_X11_WMCLASS", name, 1);
#endif
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		error_set("SDL_INIT_VIDEO: %s", SDL_GetError());
		return (-1);
	}
	SDL_WM_SetCaption(name, name);
	progname = name;

	vinfo = SDL_GetVideoInfo();
	if (vinfo != NULL) {
		char accel[2048];
		char unaccel[2048];
		size_t size = 2048;

		vfmt = vinfo->vfmt;

		if (vinfo->wm_available) {
			printf(_("Window manager is available.\n"));
		} else {
			printf(_("Window manager is unavailable.\n"));
		}
			
		if (vinfo->hw_available) {
			printf(_("Hardware surfaces are available.\n"));
		} else {
			printf(_("Hardware surfaces are unavailable.\n"));
		}
		
		accel[0] = '\0';
		unaccel[0] = '\0';

		strlcat(vinfo->blit_hw ? accel : unaccel,
		    _("\tSDL hardware blits\n"), size);
		strlcat(vinfo->blit_hw_CC ? accel : unaccel,
		    _("\tSDL hardware->hardware colorkey blits\n"), size);
		strlcat(vinfo->blit_hw_A ? accel : unaccel,
		    _("\tSDL hardware->hardware alpha blits\n"), size);
		
		strlcat(vinfo->blit_sw ? accel : unaccel,
		    _("\tSDL software->hardware blits\n"), size);
		strlcat(vinfo->blit_sw_CC ? accel : unaccel,
		    _("\tSDL software->hardware colorkey blits\n"), size);
		strlcat(vinfo->blit_sw_A ? accel : unaccel,
		    _("\tSDL software->hardware alpha blits\n"), size);
		strlcat(vinfo->blit_fill ? accel : unaccel,
		    _("\tSDL color fills\n"), size);

		if (accel[0] != '\0')
			printf(_("Accelerated operations:\n%s"), accel);
		if (unaccel[0] != '\0')
			printf(_("Unaccelerated operations:\n%s"), unaccel);

		printf("\n");
	}

	typesw_init();
	colors_init();
#ifdef NETWORK
	rcs_init();
#endif

	config = Malloc(sizeof(struct config), M_OBJECT);
	config_init(config);
	object_load(config);

	world = object_new(NULL, "world");
	object_remain(world, OBJECT_REMAIN_DATA);
	inited++;
	return (0);
}

void
engine_set_gfxmode(enum gfx_engine mode)
{
	gfx_mode  = mode;
}

/* Initialize graphics and input devices. */
int
engine_init(void)
{
	int i, njoys;
	
	if (view_init(gfx_mode) == -1 || text_init() == -1)
		return (-1);
	
	kbd_new(0);
	mouse_new(0);

	if (prop_get_bool(config, "input.joysticks") &&
	    SDL_InitSubSystem(SDL_INIT_JOYSTICK) == 0) {
		njoys = SDL_NumJoysticks();
		for (i = 0; i < njoys; i++)
			joy_new(i);
	}

	primitives_init();
	cursors_init();

	object_init(&engine_icons, "object", "icons", NULL);
	gfx_wire(&engine_icons, "/engine/icons/icons");

	config_window(config);

#if defined(DEBUG) && defined(NETWORK) && defined(THREADS)
	if (server_mode) {
		extern int server_start(void);

		server_start();
	}
#endif
	return (0);
}

/* Register a function to invoke in engine_destroy(). */
void
engine_atexit(void (*func)(void))
{
	engine_atexit_func = func;
}

/*
 * Release all resources allocated by the Agar engine.
 * Only one thread must be running.
 */
void
engine_destroy(void)
{
	if (engine_atexit_func != NULL)
		engine_atexit_func();

#if defined(MAP) && defined(EDITION)
	if (mapedition)
		object_save(&mapedit);
#endif
#ifdef NETWORK
	rcs_destroy();
#endif

	object_destroy(world);
	text_destroy();
	view_destroy();
	input_destroy();
	
	colors_destroy();
	cursors_destroy();

	object_destroy(config);
	Free(config, M_OBJECT);

	pthread_mutex_destroy(&timeout_lock);
#if 0
	pthread_mutex_destroy(&linkage_lock);	/* XXX */
#endif
	error_destroy();
	typesw_destroy();
	SDL_Quit();
	exit(0);
}

void
lock_linkage(void)
{
	pthread_mutex_lock(&linkage_lock);
}

void
unlock_linkage(void)
{
	pthread_mutex_unlock(&linkage_lock);
}

