/*	$Csoft: engine.c,v 1.66 2002/09/06 01:23:39 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include "mcconfig.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef USE_X11
#include <SDL_syswm.h>
#endif

#include "engine.h"
#include "map.h"
#include "physics.h"
#include "input.h"
#include "config.h"
#include "rootmap.h"

#include "mapedit/mapedit.h"

#ifdef DEBUG
#include "monitor/monitor.h"
#endif

#include "widget/text.h"
#include "widget/widget.h"
#include "widget/window.h"
#include "widget/textbox.h"
#include "widget/keycodes.h"

#ifdef DEBUG
int	engine_debug = 1;	/* Enable debugging */
#endif

pthread_key_t engine_errorkey;	/* Used by AGAR_{Get,Set}Error() */

const struct gameinfo *gameinfo;	/* Game name, copyright, version */
struct world *world;			/* Describes game elements */
struct config *config;			/* Global configuration settings */
int mapediting;				/* Map edition mode */

/* Input devices */
struct input *keyboard = NULL;
struct input *joy = NULL;
struct input *mouse = NULL;

static void	printusage(char *, int);
#ifdef XDEBUG
static int	engine_xerror(Display *, XErrorEvent *);
static int	engine_xioerror(Display *);
static void	engine_xdebug(void);
#endif

static void
printusage(char *progname, int flags)
{
	fprintf(stderr, "Usage: %s [-efv] [-w width] [-h height] [-j joy#]\n",
	    progname);
}

int
engine_init(int argc, char *argv[], const struct gameinfo *gi,
    char *path, int flags)
{
	int c, njoy = -1, fullscreen = 0;
	int w = -1, h = -1;

	pthread_key_create(&engine_errorkey, NULL);

	mapediting = 0;
	gameinfo = gi;
	
	printf("AGAR engine v%s\n", ENGINE_VERSION);
	printf("%s %s\n", gameinfo->name, gameinfo->version);
	printf("%s\n", gameinfo->copyright);
	
	while ((c = getopt(argc, argv, "vfegl:n:j:w:h:")) != -1) {
		switch (c) {
		case 'v':
			exit (0);
		case 'f':
			fullscreen++;
			break;
		case 'j':
			njoy = atoi(optarg);
			break;
		case 'e':
			mapediting++;
			break;
		case 'w':
			w = atoi(optarg);
			if (w < 1 || w > 65536) {
				fprintf(stderr, "invalid width argument\n");
				exit(1);
			}
			break;
		case 'h':
			h = atoi(optarg);
			if (w < 1 || w > 65536) {
				fprintf(stderr, "invalid height argument\n");
				exit(1);
			}
			break;
		default:
			printusage(argv[0], flags);
			exit(255);
		}
	}

	/* Initialize SDL. */
	if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO|SDL_INIT_AUDIO|
	    SDL_INIT_NOPARACHUTE) != 0) {
		fatal("SDL_Init: %s\n", SDL_GetError());
	}
	if (njoy != -1) {	/* XXX default */
		if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0) {
			njoy = -1;
		}
	}
	
	/* Initialize/load engine settings. */
	config = config_new();
	object_load(config);

	/* Initialize the world structure. */
	world = emalloc(sizeof(struct world));
	world_init(world, gameinfo->prog);

	/* Initialize the font engine. */
	if (text_engine_init() != 0) {
		fatal("cannot initialize font engine\n");
	}

	/* Overrides */
	if (fullscreen)
		prop_set_uint32(config, "flags",
		    prop_uint32(config, "flags") | CONFIG_FULLSCREEN);
	if (w > 0)
		prop_set_uint32(config, "view.w", w);
	if (h > 0)
		prop_set_uint32(config, "view.h", h);

	/* Initialize input devices. */
	keyboard = input_new(INPUT_KEYBOARD, 0);
	if (njoy != -1) {	/* XXX default */
		joy = input_new(INPUT_JOY, njoy);
	}
	mouse = input_new(INPUT_MOUSE, 0);

	/*
	 * Set the video mode.
	 * Initialize the masks and rectangles in game mode.
	 */
	if (mapediting || (flags & ENGINE_INIT_GUI)) {
		view_init(GFX_ENGINE_GUI);
	} else {
		view_init(GFX_ENGINE_TILEBASED);
	}

#ifdef XDEBUG
	if (engine_debug > 0) {
		/* Request synchronous X events, and set error handlers. */
		engine_xdebug();
	}
#endif
	return (0);
}

#ifdef XDEBUG

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

#endif	/* XDEBUG */

/*
 * Create static windows, drop into map edition if requested.
 * This is done after static objects are created, and before the event
 * loop is started.
 */
int
engine_start(void)
{
	struct mapedit *medit;
	struct map *m;

	/* Create the configuration settings window. */
	config_init_wins(config);

#ifdef DEBUG
	if (engine_debug > 0) {
		monitor_init(&monitor, "debug-monitor");
	}
#endif

	switch (view->gfx_engine) {
	case GFX_ENGINE_TILEBASED:
		/* Start with a default map. */
		m = emalloc(sizeof(struct map));
		map_init(m, "base", NULL, 0);
		pthread_mutex_lock(&world->lock);
		world_attach(world, m);
		pthread_mutex_unlock(&world->lock);
		if (object_load(m) != 0) {
			fatal("cannot load base.m\n");
		}
		rootmap_focus(m);
		return (0);
	case GFX_ENGINE_GUI:
		break;
	}

	if (mapediting) {
		medit = emalloc(sizeof(struct mapedit));
		mapedit_init(medit, "mapedit0");

		/* Start map edition. */
		pthread_mutex_lock(&world->lock);
		world_attach(world, medit);
		pthread_mutex_unlock(&world->lock);
	}

	return (1);
}

/* Caller must not hold world->lock. */
void
engine_stop(void)
{
	if (view->rootmap != NULL) {
		pthread_mutex_lock(&view->lock);
		view->rootmap->map = NULL;
		pthread_mutex_unlock(&view->lock);
	} else {
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
	/* Unlink game objects. */
	world_destroy(world);
	
	/* Force garbage collection of media. */
	media_start_gc(0, NULL);
	media_destroy_gc();

	/* Destroy the font engine. */
	text_engine_destroy();

	/* Free glyph cache. */
	keycodes_freeglyphs();

	/* Shut down the input devices. XXX link */
	input_destroy(keyboard);
	input_destroy(mouse);
	if (joy != NULL) {	/* XXX default */
		input_destroy(joy);
	}

	/* Destroy the views. */
	object_destroy(view);

	/* Free the config structure. */
	object_destroy(config);

	pthread_key_delete(engine_errorkey);

	SDL_Quit();
	exit(0);
}

