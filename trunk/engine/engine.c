/*	$Csoft: engine.c,v 1.32 2002/04/24 13:14:44 vedge Exp $	*/

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
#include <engine/map.h>
#include <engine/physics.h>
#include <engine/input.h>

#include <engine/mapedit/mapedit.h>

#include <engine/widget/window.h>
#include <engine/widget/widget.h>
#include <engine/widget/label.h>
#include <engine/widget/button.h>
#include <engine/widget/text.h>

#ifdef DEBUG
int	engine_debug = 1;
#endif
struct	world *world;
struct	gameinfo *gameinfo;

static char *mapdesc = NULL, *mapstr = NULL;
static int mapw = 64, maph = 64;
static int tilew = 32, tileh = 32;
int mapediting;

struct input *keyboard = NULL;
struct input *joy = NULL;
struct input *mouse = NULL;

static void	printusage(char *);
static void	close_button_push(struct button *);

static void
printusage(char *progname)
{
	fprintf(stderr,
	    "Usage: %s [-vxf] [-w width] [-h height] [-d depth] [-j joy#]\n",
	    progname);
	fprintf(stderr,
	    "                 [-e mapname] [-D mapdesc] [-W mapw] [-H maph]\n");
	fprintf(stderr,
	    "                 [-W mapw] [-H maph] [-X tilew] [-Y tileh]\n");
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
	w = 640;
	h = 480;
	depth = 32;
	flags = SDL_SWSURFACE;

	/* XXX ridiculous */
	while ((c = getopt(argc, argv, "xvfl:n:w:h:d:j:e:D:W:H:X:Y:")) != -1) {
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
		case 'X':
			tilew = atoi(optarg);
			break;
		case 'Y':
			tileh = atoi(optarg);
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
	mainview = view_create(w, h, depth, flags);
	if (mainview == NULL) {
		fatal("view_create\n");
		return (-1);
	}

	/* Initialize the world structure. */
	world = (struct world *)emalloc(sizeof(struct world));
	world_init(world, gameinfo->prog);
	
	/* Initialize the font engine. */
	if (text_engine_init() != 0) {
		return (-1);
	}
	
	/* Initialize input devices. */
	keyboard = (struct input *)emalloc(sizeof(struct input));
	joy = (struct input *)emalloc(sizeof(struct input));
	mouse = (struct input *)emalloc(sizeof(struct input));
	input_init(keyboard, INPUT_KEYBOARD, 0);
	input_init(joy, INPUT_JOY, njoy);
	input_init(mouse, INPUT_MOUSE, 0);
	
	return (0);
}

void
engine_editmap(void)
{
	struct mapedit *medit;

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
	event_loop();
	engine_destroy();
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

void
engine_config(void)
{
	struct window *w;
	struct label *fullscreen_label;
	struct button *close_button;

	w = emalloc(sizeof(struct window));
	window_init(w, mainview, "engine-config", "Engine settings", 0,
	    WINDOW_GRADIENT, 64, 64, 512, 256);
	object_link(w);

	fullscreen_label = emalloc(sizeof(struct label));
	label_init(fullscreen_label, w, "fullscreen-label", "Full-screen",
	    0, 10, 10);
	object_link(fullscreen_label);

	close_button = emalloc(sizeof(struct button));
	button_init(close_button, w, "close-button", "Close", 0, 100, 100);
	close_button->push = close_button_push;
	object_link(close_button);
}

static void
close_button_push(struct button *b)
{
	object_unlink(WIDGET(b)->win);
}

