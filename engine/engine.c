/*	$Csoft: engine.c,v 1.40 2002/05/11 03:24:12 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CubeSoft Communications, nor the names of its
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <engine/engine.h>
#ifdef USE_X11
#include <SDL_syswm.h>
#endif

#include <engine/map.h>
#include <engine/physics.h>
#include <engine/input.h>

#include <engine/mapedit/mapedit.h>

#include <engine/widget/text.h>

#ifdef DEBUG
int	engine_debug = 1;	/* Enable debugging */
#endif

struct	world *world;
struct	gameinfo *gameinfo;
int mapediting;

struct input *keyboard = NULL;
struct input *joy = NULL;
struct input *mouse = NULL;

static char *mapdesc = NULL, *mapstr = NULL;
static int mapw = 64, maph = 64;	/* Default map geometry */
static int tilew = 32, tileh = 32;	/* XXX pref */

static void	printusage(char *);
#ifdef XDEBUG
static int	engine_xerror(Display *, XErrorEvent *);
static int	engine_xioerror(Display *);
static void	engine_xdebug(void);
#endif

static void
printusage(char *progname)
{
	fprintf(stderr,
	    "Usage: %s [-vxf] [-w width] [-h height] [-d depth] [-j joy#]\n",
	    progname);
	fprintf(stderr,
	    "             [-e mapname]\n");
}

int
engine_init(int argc, char *argv[], struct gameinfo *gi, char *path)
{
	int c, w, h, depth, njoy, flags;
	extern int xcf_debug;
	
	curmapedit = NULL;
	gameinfo = gi;

	njoy = 0;
	mapediting = 0;
	w = 640;	/* XXX pref */
	h = 480;
	depth = 32;
	flags = SDL_SWSURFACE;

	/* XXX ridiculous */
	while ((c = getopt(argc, argv, "xvfl:n:w:h:d:j:e:D:W:H:")) != -1) {
		switch (c) {
		case 'x':
			xcf_debug++;
			break;
		case 'v':
			printf("AGAR engine v%s\n", ENGINE_VERSION);
			printf("%s v%d.%d\n", gameinfo->name,
			    gameinfo->ver[0], gameinfo->ver[1]);
			printf("%s\n", gameinfo->copyright);
			exit (255);
		case 'f':
			flags |= SDL_FULLSCREEN;
			break;
		case 'w':
			w = atoi(optarg);
			break;
		case 'h':
			h = atoi(optarg);
			break;
		case 'd':
			depth = atoi(optarg);
			break;
		case 'j':
			njoy = atoi(optarg);
			break;
		case 'e':
			mapediting++;
			mapstr = optarg;
			break;
		case 'D':
			mapdesc = optarg;
			break;
		case 'W':
			mapw = atoi(optarg);
			break;
		case 'H':
			maph = atoi(optarg);
			break;
		default:
			printusage(argv[0]);
			exit(255);
		}
	}

	/* Initialize SDL. */
	if (SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO|SDL_INIT_AUDIO|
	    SDL_INIT_EVENTTHREAD|SDL_INIT_NOPARACHUTE) != 0) {
		fatal("SDL_Init: %s\n", SDL_GetError());
		return (-1);
	}
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0) {
		njoy = -1;
	}

	if (flags & SDL_FULLSCREEN) {
		/* Give the new mode some time to take effect. */
		SDL_Delay(1000);
	}
	
	/*
	 * Create the main viewport. The video mode will be set
	 * as soon as a map is loaded.
	 */
	mainview = view_new(w, h, depth, flags);
	if (mainview == NULL) {
		fatal("could not create main view\n");
		return (-1);
	}

	/* Initialize the world structure. */
	world = emalloc(sizeof(struct world));
	world_init(world, gameinfo->prog);
	
	/* Initialize the font engine. */
	if (text_engine_init() != 0) {
		return (-1);
	}
	
	/* Initialize input devices. */
	keyboard = input_new(INPUT_KEYBOARD, 0);
	joy = input_new(INPUT_JOY, njoy);
	mouse = input_new(INPUT_MOUSE, 0);

#ifdef XDEBUG
	if (engine_debug > 0) {
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
		if (XSynchronize(wm.info.x11.display, True) == False) {
			warning("synchronous X events\n");
		}
		wm.info.x11.unlock_func();
	}

	/* Catch X errors. */
	XSetErrorHandler((XErrorHandler)engine_xerror);
	XSetIOErrorHandler((XIOErrorHandler)engine_xioerror);
}

#endif	/* XDEBUG */

/* Drop into map edition if requested. */
int
engine_editmap(void)
{
	struct mapedit *medit;

	if (!mapediting) {
		return (0);
	}

	medit = emalloc(sizeof(struct mapedit));
	mapedit_init(medit, "mapedit0");

	/* Set the map edition arguments. */
	medit->margs.name = strdup(mapstr);
	medit->margs.desc = (mapdesc != NULL) ? strdup(mapdesc) : "";
	medit->margs.mapw = mapw;
	medit->margs.maph = maph;
	medit->margs.tilew = tilew;
	medit->margs.tileh = tileh;

	/* Start map edition. */
	object_link(medit);

	return (1);
}

void
engine_destroy(void)
{
	/* Unlink all objects and add them to the free list. */
	world_destroy(world);

	/* Force garbage collection. */
	object_start_gc(0, NULL);
	object_destroy_gc();
	
	/* Destroy the font engine. */
	text_engine_destroy();

	/* Shut down the input devices. XXX link */
	input_destroy(keyboard);
	input_destroy(mouse);
	input_destroy(joy);

	SDL_Quit();
	exit(0);
}

void *
emalloc(size_t len)
{
	void *p;

	p = malloc(len);
	if (p == NULL) {
		perror("malloc");
		engine_destroy();
	}
	return (p);
}

void *
erealloc(void *ptr, size_t len)
{
	void *p;

	p = realloc(ptr, len);
	if (p == NULL) {
		perror("realloc");
		engine_destroy();
	}
	return (p);
}

