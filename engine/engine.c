/*	$Csoft: engine.c,v 1.3 2002/02/01 11:57:21 vedge Exp $	*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <glib.h>
#include <SDL.h>

#include <engine/engine.h>
#include <engine/mapedit/mapedit.h>

#ifdef DEBUG
int	engine_debug = 1;
#endif
struct	world *world;
int	mapedit;

static char *mapdesc = NULL, *mapstr = NULL;
static int mapw, maph;

static SDL_Joystick *joy = NULL;

static void printusage(char *);

static void
printusage(char *progname)
{
	fprintf(stderr,
	    "Usage: %s [-vxf] [-w width] [-h height] [-d depth] [-j joy#]\n",
	    progname);
	fprintf(stderr,
	    "Usage: %s [-vxf] [-e mapname] [-D mapdesc] [-W mapw] [-H maph]\n",
	    progname);
}

int
engine_init(int argc, char *argv[], struct gameinfo *gameinfo)
{
	int c, w, h, depth, njoy, flags;
	extern int xcf_debug;

	njoy = 0;
	mapedit = 0;
	w = 640;
	h = 480;
	depth = 32;
	flags = SDL_SWSURFACE;
	curmap = NULL;
	
	while ((c = getopt(argc, argv, "xvfw:h:d:j:e:D:W:H:")) != -1) {
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
			mapedit++;
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
	    SDL_INIT_NOPARACHUTE) != 0) {
		fatal("SDL_Init: %s\n", SDL_GetError());
		return (-1);
	}
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == 0) {
		joy = SDL_JoystickOpen(njoy);
		if (joy != NULL) {
			dprintf("using joystick 0x%x (%s)\n",
			    njoy, SDL_JoystickName(njoy));
			SDL_JoystickEventState(SDL_ENABLE);
		}
	}
	if (flags & SDL_FULLSCREEN) {
		/* Give the new mode some time to take effect. */
		SDL_Delay(1000);
	}
	
	/* Create the main viewport for 32x32 tiles. */
	mainview = view_create(w, h, 32, 32, depth, flags);
	if (mainview == NULL) {
		fatal("view_create\n");
		return (-1);
	}
	
	/* Initialize the world structure. */
	world = world_create("world");
	if (world == NULL) {
		fatal("world_create\n");
		return (-1);
	}

	return (0);
}

int
engine_mapedit(void)
{
	if (mapedit) {
		struct mapedit *medit;

		/* Edit a loaded map, or create a new one. */
		medit = mapedit_create(mapstr, mapdesc);
		if (medit == NULL) {
			fatal("mapedit_create\n");
			return (1);
		}

		/* Map is in a consistent state. Display animations. */
		map_focus(medit->map);
	}
	return (0);
}

void
engine_join(void)
{
	event_loop();
}

void
engine_destroy(void)
{
	object_destroy(world);

	if (joy != NULL) {
		SDL_JoystickClose(joy);
	}

	SDL_Quit();
}

