/*	$Csoft: engine.c,v 1.98 2003/04/12 01:32:36 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003 CubeSoft Communications, Inc.
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
#include <engine/compat/setenv.h>
#include <engine/compat/strlcat.h>

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/input.h>
#include <engine/config.h>
#include <engine/rootmap.h>
#include <engine/view.h>
#include <engine/world.h>

#ifdef DEBUG
#include <engine/monitor/monitor.h>
#endif
#include <engine/mapedit/mapedit.h>

#include <engine/widget/text.h>
#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/textbox.h>
#include <engine/widget/keycodes.h>

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_X11
#include <SDL_syswm.h>
#endif

#ifdef DEBUG
int	engine_debug = 1;		/* Enable debugging */
#endif

#ifdef THREADS
pthread_mutexattr_t	recursive_mutexattr;	/* Recursive mutex attributes */
pthread_key_t		engine_errorkey;	/* Multi-threaded error code */
#else
char *engine_errorkey;				/* Single-threaded error code */
#endif

const struct engine_proginfo *proginfo;	/* Game name, copyright, version */
struct world *world;			/* Describes game elements */
struct config *config;			/* Global configuration settings */

#ifdef HAVE_X11
static int	engine_xerror(Display *, XErrorEvent *);
static int	engine_xioerror(Display *);
static void	engine_xdebug(void);
#endif

int
engine_init(int argc, char *argv[], const struct engine_proginfo *prog,
    int flags)
{
	static int inited = 0;
	const SDL_VideoInfo *vinfo;

	if (inited) {
		error_set("engine already initialized");
		return (-1);
	}

#ifdef THREADS
	pthread_mutexattr_init(&recursive_mutexattr);
	pthread_mutexattr_settype(&recursive_mutexattr,
	    PTHREAD_MUTEX_RECURSIVE);
	
	pthread_key_create(&engine_errorkey, NULL);
#else
	engine_errorkey = NULL;
#endif

	printf("Agar engine v%s\n", ENGINE_VERSION);
	printf("%s %s\n", prog->name, prog->version);
	printf("%s\n\n", prog->copyright);
	proginfo = prog;

	if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_NOPARACHUTE) != 0) {
		error_set("SDL_Init: %s", SDL_GetError());
		return (-1);
	}

	if (flags & ENGINE_INIT_GFX) {
#ifdef HAVE_X11
		setenv("SDL_VIDEO_X11_WMCLASS", prog->name, 1);
#endif
		if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
			error_set("SDL_INIT_VIDEO: %s", SDL_GetError());
			return (-1);
		}
		SDL_WM_SetCaption(prog->name, prog->prog);

		/* Print video device information. */
		vinfo = SDL_GetVideoInfo();
		if (vinfo != NULL) {
			char accel[4096];
			char unaccel[4096];

			printf(
			    "Video device is %dbpp (ckey=0x%x, alpha=0x%04x)\n",
			    vinfo->vfmt->BitsPerPixel, vinfo->vfmt->colorkey,
			    vinfo->vfmt->alpha);
			printf("Video mask is 0x%04x,0x%04x,0x%04x,0x%04x\n",
			    vinfo->vfmt->Rmask, vinfo->vfmt->Gmask,
			    vinfo->vfmt->Bmask, vinfo->vfmt->Amask);
			printf("Window manager is %savailable.\n",
			    vinfo->wm_available ? "" : "un");
			printf("Hardware surfaces are %savailable.\n",
			    vinfo->hw_available ? "" : "un");
		
			accel[0] = '\0';
			unaccel[0] = '\0';

			strlcat(vinfo->blit_hw ? accel : unaccel,
			    "\tHardware blits\n", 4096);
			strlcat(vinfo->blit_hw_CC ? accel : unaccel,
			    "\tHardware->hardware colorkey blits\n", 4096);
			strlcat(vinfo->blit_hw_A ? accel : unaccel,
			    "\tHardware->hardware alpha blits\n", 4096);
		
			strlcat(vinfo->blit_sw ? accel : unaccel,
			    "\tSoftware->hardware blits\n", 4096);
			strlcat(vinfo->blit_sw_CC ? accel : unaccel,
			    "\tSoftware->hardware colorkey blits\n", 4096);
			strlcat(vinfo->blit_sw_A ? accel : unaccel,
			    "\tSoftware->hardware alpha blits\n", 4096);
			strlcat(vinfo->blit_fill ? accel : unaccel,
			    "\tColor fills\n", 4096);

			if (accel[0] != '\0')
				printf("Accelerated operations:\n%s", accel);
			if (unaccel[0] != '\0')
				printf("Unaccelerated operations:\n%s",
				    unaccel);
			printf("\n");
		}
	}
	
	/* Initialize/load engine settings. */
	config = config_new();
	object_load(config, NULL);

	/* Initialize the world structure. */
	world = Malloc(sizeof(struct world));
	world_init(world, prog->prog);

	/* Start up the font engine. */
	if (prop_get_bool(config, "font-engine")) {
		if (text_init() == -1) {
			fatal("text_init: %s", error_get());
		}
	}

	/* Initialize the input devices. */
	if (flags & ENGINE_INIT_INPUT) {
		input_new(INPUT_KEYBOARD, 0);
		input_new(INPUT_MOUSE, 0);

		if (prop_get_bool(config, "input.joysticks") &&
		    SDL_InitSubSystem(SDL_INIT_JOYSTICK) == 0) {
			int i, njoys;

			njoys = SDL_NumJoysticks();
			for (i = 0; i < njoys; i++) {
				input_new(INPUT_JOY, i);
			}
		}
		keycodes_init();
	}

#ifdef HAVE_X11
	if (flags & ENGINE_INIT_GFX &&
	    prop_get_bool(config, "view.xsync")) {
		/* Request synchronous X events, and set error handlers. */
		engine_xdebug();
	}
#endif

	inited++;
	return (0);
}

#ifdef HAVE_X11

static int
engine_xerror(Display *dis, XErrorEvent *xerror)
{
	fprintf(stderr, "X error: request 0x%x (minor 0x%x): error %d\n",
	    xerror->request_code, xerror->minor_code,
	    xerror->error_code);
	abort();

	return (-1);
}

static int
engine_xioerror(Display *dis)
{
	fprintf(stderr, "X I/O error\n");
	abort();

	return (-1);
}

static void
engine_xdebug(void)
{
	SDL_SysWMinfo wm;
	const SDL_VideoInfo *vinfo;

	vinfo = SDL_GetVideoInfo();
	if (!vinfo->wm_available) {
		return;
	}

	SDL_VERSION(&wm.version);
	if (SDL_GetWMInfo(&wm) != 1) {
		warning("SDL_GetWMInfo: %s\n", SDL_GetError());
		return;
	}
	if (wm.subsystem == SDL_SYSWM_X11) {
		wm.info.x11.lock_func();
		warning("synchronous X events\n");
		XSynchronize(wm.info.x11.display, True);
		wm.info.x11.unlock_func();
	}

	/* Catch X errors. */
	XSetErrorHandler((XErrorHandler)engine_xerror);
	XSetIOErrorHandler((XIOErrorHandler)engine_xioerror);
}

#endif	/* HAVE_X11 */

void
engine_stop(void)
{
	switch (view->gfx_engine) {
	case GFX_ENGINE_TILEBASED:
		/* Stop the event loop synchronously. */
		pthread_mutex_lock(&view->lock);
		view->rootmap->map = NULL;
		pthread_mutex_unlock(&view->lock);
		break;
	default:
		/* Shut down immediately. */
		engine_destroy();
	}
}

/*
 * Caller must not hold world->lock.
 * Only one thread must be running.
 */
void
engine_destroy(void)
{
	if (mapedition) {
		object_save(&mapedit, NULL);
	}
	world_destroy(world);
	text_destroy();
	input_destroy_all();

	object_destroy(view);
	free(view);
	object_destroy(config);
	free(config);

#ifdef THREADS
	pthread_key_delete(engine_errorkey);
#else
	Free(engine_errorkey);
#endif
	SDL_Quit();
	exit(0);
}

