/*	$Csoft: engine.c,v 1.130 2004/04/20 09:16:39 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004 CubeSoft Communications, Inc.
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
#include <config/have_progname.h>
#include <config/have_setlocale.h>
#include <config/localedir.h>
#include <config/version.h>

#include <compat/setenv.h>
#include <compat/strlcat.h>

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/input.h>
#include <engine/config.h>
#include <engine/rootmap.h>
#include <engine/view.h>
#include <engine/typesw.h>

#ifdef EDITION
#include <engine/mapedit/mapedit.h>
#endif

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/textbox.h>
#include <engine/widget/keycodes.h>

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

struct engine_proginfo *proginfo;	/* Game name, copyright, version */
struct config *config;			/* Global configuration settings */
struct object *world;			/* The Old Roots of Evil */
pthread_mutex_t linkage_lock;		/* Protects object linkage */
struct object engine_icons;		/* Global engine icons */

/* Initialize the Agar engine. */
int
engine_init(int argc, char *argv[], struct engine_proginfo *prog, int flags)
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

	/* Initialize the error handling facility. */
	error_init();

#ifdef THREADS
	pthread_mutexattr_init(&recursive_mutexattr);
	pthread_mutexattr_settype(&recursive_mutexattr,
	    PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_init(&linkage_lock, &recursive_mutexattr);
	pthread_mutex_init(&gfxq_lock, &recursive_mutexattr);
#endif

#ifdef HAVE_PROGNAME
	/* Prefer __progname on BSD systems. */
	{
		extern char *__progname;
		prog->progname = __progname;
	}
#endif

	printf("Agar %s\n", VERSION);
	proginfo = prog;

	/* Initialize the SDL library. */
	if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_NOPARACHUTE) != 0) {
		error_set("SDL_Init: %s", SDL_GetError());
		return (-1);
	}
	if (flags & ENGINE_INIT_GFX) {
#ifdef HAVE_X11
		setenv("SDL_VIDEO_X11_WMCLASS", prog->progname, 1);
#endif
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
			error_set("SDL_INIT_VIDEO: %s", SDL_GetError());
			return (-1);
		}
		SDL_WM_SetCaption(prog->name, prog->progname);

		/* Print video device information. */
		vinfo = SDL_GetVideoInfo();
		if (vinfo != NULL) {
			char accel[2048];
			char unaccel[2048];
			size_t size = 2048;

			printf(_("Video device is %dbpp "
			         "(ckey=%d, alpha=%d)\n"),
			    vinfo->vfmt->BitsPerPixel, vinfo->vfmt->colorkey,
			    vinfo->vfmt->alpha);
			printf(_("Video mask is 0x%08x,0x%08x,0x%08x,0x%08x\n"),
			    vinfo->vfmt->Rmask, vinfo->vfmt->Gmask,
			    vinfo->vfmt->Bmask, vinfo->vfmt->Amask);

			if (vinfo->wm_available) {
				printf(_("Window manager is available.\n"));
			} else {
				printf(_("Window manager is unavailable.\n"));
			}
			
			if (vinfo->hw_available) {
				printf(
				    _("Hardware surfaces are available.\n"));
			} else {
				printf(
				    _("Hardware surfaces are unavailable.\n"));
			}
		
			accel[0] = '\0';
			unaccel[0] = '\0';

			strlcat(vinfo->blit_hw ? accel : unaccel,
			    _("\tHardware blits\n"), size);
			strlcat(vinfo->blit_hw_CC ? accel : unaccel,
			    _("\tHardware->hardware colorkey blits\n"), size);
			strlcat(vinfo->blit_hw_A ? accel : unaccel,
			    _("\tHardware->hardware alpha blits\n"), size);
		
			strlcat(vinfo->blit_sw ? accel : unaccel,
			    _("\tSoftware->hardware blits\n"), size);
			strlcat(vinfo->blit_sw_CC ? accel : unaccel,
			    _("\tSoftware->hardware colorkey blits\n"), size);
			strlcat(vinfo->blit_sw_A ? accel : unaccel,
			    _("\tSoftware->hardware alpha blits\n"), size);
			strlcat(vinfo->blit_fill ? accel : unaccel,
			    _("\tColor fills\n"), size);

			if (accel[0] != '\0')
				printf(_("Accelerated operations:\n%s"), accel);
			if (unaccel[0] != '\0')
				printf(_("Unaccelerated operations:\n%s"),
				    unaccel);
			printf("\n");
		}
	}

	/* Initialize the type switch and register the built-in types. */
	typesw_init();

	/* Initialize and load the user engine settings. */
	config = Malloc(sizeof(struct config), M_OBJECT);
	config_init(config);
	object_load(config);

	/* Attach the input devices. */
	if (flags & ENGINE_INIT_INPUT) {
		kbd_new(0);
		mouse_new(0);

		if (prop_get_bool(config, "input.joysticks") &&
		    SDL_InitSubSystem(SDL_INIT_JOYSTICK) == 0) {
			int i, njoys;

			njoys = SDL_NumJoysticks();
			for (i = 0; i < njoys; i++)
				joy_new(i);
		}
	}

	world = object_new(NULL, "world");
	world->save_pfx = NULL;
	inited++;
	return (0);
}

/*
 * Release all resources allocated by the Agar engine.
 * Only one thread must be running.
 */
void
engine_destroy(void)
{
#ifdef EDITION
	if (mapedition)
		object_save(&mapedit);
#endif
	view_destroy();
	object_destroy(world);
	text_destroy();
	input_destroy();

	object_destroy(config);
	Free(config, M_OBJECT);

	pthread_mutex_destroy(&gfxq_lock);
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

