/*	$Csoft$	*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <glib.h>
#include <SDL.h>

#include <engine/view.h>
#include <engine/debug.h>
#include <engine/event.h>
#include <engine/object.h>
#include <engine/world.h>
#include <engine/map.h>

extern void quit(void);

static void	 event_dispatch(void *, void *);
static void	 event_hotkey(SDL_Event *);

/* XXX inefficient */
static void
event_dispatch(void *arg, void *p)
{
	struct object *ob = (struct object *)arg;
	SDL_Event *ev = (SDL_Event *)p;

	if (ob->flags & EVENT_HOOK) {
		ob->event_hook(ob, ev);
	}
}

/* Global hotkeys. */
static __inline void
event_hotkey(SDL_Event *ev)
{
	/* Print active object list. */
	switch (ev->key.keysym.sym) {
#ifdef DEBUG
	case SDLK_t:
		world_dump(world);
		break;
#endif /* DEBUG */
	case SDLK_f:
		view_fullscreen(mainview,
		    (mainview->flags & SDL_FULLSCREEN) ? 0 : 1);
		break;
	case SDLK_q:
		if (mainview->flags & SDL_FULLSCREEN) {
			mainview->flags &= ~(SDL_FULLSCREEN);
			view_fullscreen(mainview, 0);
		}
		quit();
		break;
	default:
		break;
	}
}

void
event_loop(void)
{
	SDL_Event ev;

	while (SDL_WaitEvent(&ev)) {
		switch (ev.type) {
#if 0
		case SDL_VIDEOEXPOSE:
			map_draw(curmap);
			continue;
#endif
		case SDL_QUIT:
			quit();
			/* NOTREACHED */
		default:
			if (ev.type == SDL_KEYDOWN) {
				event_hotkey(&ev);
			}
			/* XXX optimize */
			if (pthread_mutex_lock(&world->lock) == 0) {
				g_slist_foreach(world->objs,
				    (GFunc) event_dispatch, &ev);
				pthread_mutex_unlock(&world->lock);
			}
			continue;
		}
	}
}

