/*	$Csoft: event.c,v 1.82 2002/09/13 11:08:29 vedge Exp $	*/

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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

#include "widget/widget.h"
#include "widget/window.h"
#include "widget/label.h"
#include "widget/text.h"
#include "widget/graph.h"

#include "mapedit/mapview.h"

extern struct gameinfo *gameinfo;	/* script */
extern struct window *game_menu_win;

#ifdef DEBUG
int event_debug = 0;
static struct window *fps_win;
#define EVENT_DEBUG if (event_debug) printf
#else
#define EVENT_DEBUG
#endif

static void	 event_hotkey(SDL_Event *);
static void	 event_dispatch(SDL_Event *);
static void	*event_post_async(void *);

static void
event_hotkey(SDL_Event *ev)
{
	pthread_mutex_lock(&world->lock);

	switch (ev->key.keysym.sym) {
#ifdef DEBUG
	case SDLK_r:
		if (ev->key.keysym.mod & KMOD_CTRL) {
			VIEW_REDRAW();
		}
		break;
	case SDLK_F2:
		object_save(world);
		break;
	case SDLK_F4:
		object_load(world);
		break;
	case SDLK_F5:
		pthread_mutex_lock(&view->lock);
		if (view->rootmap != NULL) {
			text_msg("Checking %s\n",
			    OBJECT(view->rootmap->map)->name);
			map_verify(view->rootmap->map);
		}
		pthread_mutex_unlock(&view->lock);
		break;
	case SDLK_F6:
		window_show(fps_win);
		break;
	case SDLK_F7:
		if (engine_debug > 0) {
			window_show(monitor.wins.toolbar);
		}
		break;
#endif
	case SDLK_F1:
		window_show(config->windows.settings);
		break;
	case SDLK_F3:
		window_show(config->windows.algorithm_sw);
		break;
#ifdef DEBUG
	case SDLK_v:
		if (ev->key.keysym.mod & KMOD_CTRL &&
		    view->rootmap != NULL) {
			struct window *win;
			struct region *reg;
			struct mapview *mv;

			win = window_new(NULL, "Map view", WINDOW_TITLEBAR,
			    -1, -1, 320, 200, 320, 200);
			reg = region_new(win, REGION_HALIGN, 0, 0, 100, 100);
			mv = mapview_new(reg, curmapedit, view->rootmap->map,
			    MAPVIEW_CENTER|MAPVIEW_ZOOM|MAPVIEW_SHOW_CURSOR,
			    100, 100);
			window_show(win);
		}
		break;
#endif
	case SDLK_t:
		if (curmapedit != NULL) {
			window_show(curmapedit->toolbar_win);
		}
		break;
	case SDLK_ESCAPE:
		pthread_mutex_unlock(&world->lock);
		engine_stop();
		return;
	default:
		break;
	}

	pthread_mutex_unlock(&world->lock);
}

#ifdef DEBUG
#define UPDATE_FPS(delta) do {				\
	label_printf(fps_label, "%d/100", (delta));	\
	graph_plot(fps_item, (delta));			\
} while (/*CONSTCOND*/0)
#else
#define UPDATE_FPS(delta)
#endif

#define COMPUTE_DELTA(delta, ntick) do {			\
	(delta) = EVENT_MAXFPS - (SDL_GetTicks() - (ntick));	\
	UPDATE_FPS((delta));					\
	if ((delta) < 1) {					\
		dprintf("%d FPS\n", (delta));			\
		(delta) = 1;					\
	}							\
} while (/*CONSTCOND*/0)

void
event_loop(void)
{
	SDL_Event ev;
	Uint32 ltick, ntick;
	int rv, delta;
	struct window *win;
	SDL_Rect wrects[64];
	int nwrects = 0;
#ifdef DEBUG
	struct region *reg;
	struct label *fps_label;
	struct graph *fps_graph;
	struct graph_item *fps_item;

	fps_win = window_new("fps-counter", "Frames/second", WINDOW_CENTER,
	    0, 0, 133, 104, 125, 91);
	reg = region_new(fps_win, REGION_VALIGN,
	    0, 0, 100, 100);
	fps_label = label_new(reg, "...", 100, 20);
	fps_graph = graph_new(reg, "Frames/sec", GRAPH_LINES,
	    GRAPH_SCROLL|GRAPH_ORIGIN, 200,
	    100, 80);
	fps_item = graph_add_item(fps_graph, "fps",
	    SDL_MapRGB(view->v->format, 0, 160, 0));
#endif

	/* Start the media garbage collection process. */
	media_init_gc();

	for (ntick = 0, ltick = SDL_GetTicks(), delta = EVENT_MAXFPS;;) {
		ntick = SDL_GetTicks();

		if ((ntick - ltick) >= delta) {
			pthread_mutex_lock(&view->lock);

			/* Tile-based rendering */
			if (view->gfx_engine == GFX_ENGINE_TILEBASED) {
				struct map *m = view->rootmap->map;

				if (m == NULL) {
					dprintf("NULL map\n");
					pthread_mutex_unlock(&view->lock);
					return;
				}

				pthread_mutex_lock(&world->lock);
				pthread_mutex_lock(&m->lock);

				/* Update animated nodes individually. */
				rootmap_animate();

				/* Update static nodes individually. */
				if (m->redraw != 0) {
					dprintf("tile: redraw\n");
					rootmap_draw();
					COMPUTE_DELTA(delta, ntick);
					m->redraw = 0;
				}
				
				pthread_mutex_unlock(&m->lock);
				pthread_mutex_unlock(&world->lock);
			}

			/* Update the windows. */
			nwrects = 0;
			TAILQ_FOREACH(win, &view->windowsh, windows) {
				pthread_mutex_lock(&win->lock);
				if (win->flags & WINDOW_SHOWN) {
					/* XXX use indirect blit & microtiles */
					window_draw(win);
					wrects[nwrects].x = win->x;
					wrects[nwrects].y = win->y;
					wrects[nwrects].w = win->w;
					wrects[nwrects].h = win->h;
					nwrects++;
				}
				pthread_mutex_unlock(&win->lock);
			}
			pthread_mutex_unlock(&view->lock);

			if (nwrects > 0) {
				SDL_UpdateRects(view->v, nwrects, wrects);
				nwrects = 0;
				if (view->gfx_engine == GFX_ENGINE_GUI) {
					/* Event/motion interpolation. */
					COMPUTE_DELTA(delta, ntick);
				}
#ifdef DEBUG
				/* Update the fps counter. */
				COMPUTE_DELTA(delta, ntick);
#endif
			}

			ltick = SDL_GetTicks();
		} else if (SDL_PollEvent(&ev) != 0) {
			event_dispatch(&ev);
		}
	}
}

static void
event_dispatch(SDL_Event *ev)
{
	int rv;
	struct window *win;

	switch (ev->type) {
	case SDL_VIDEORESIZE:
		EVENT_DEBUG("SDL_VIDEORESIZE: w=%d h=%d\n", ev->resize.w,
		     ev->resize.h);
		view->w = ev->resize.w;
		view->h = ev->resize.h;
		break;
	case SDL_VIDEOEXPOSE:
		EVENT_DEBUG("SDL_VIDEOEXPOSE\n");

		pthread_mutex_lock(&view->lock);
		switch (view->gfx_engine) {
		case GFX_ENGINE_TILEBASED:
			/* Redraw the map only. */
			pthread_mutex_lock(&world->lock);
			view->rootmap->map->redraw++;
			pthread_mutex_unlock(&world->lock);
			break;
		case GFX_ENGINE_GUI:
			/* Redraw the whole screen. */
			SDL_FillRect(view->v, NULL,
			    SDL_MapRGB(view->v->format, 0, 0, 0));
			SDL_UpdateRect(view->v, 0, 0, 0 ,0);
			break;
		}
		TAILQ_FOREACH(win, &view->windowsh, windows) {
			pthread_mutex_lock(&win->lock);
			if (win->flags & WINDOW_SHOWN) {
				window_draw(win);
			}
			pthread_mutex_unlock(&win->lock);
		}
		pthread_mutex_unlock(&view->lock);
		break;
	case SDL_MOUSEMOTION:
		EVENT_DEBUG("SDL_MOUSEMOTION x=%d y=%d\n", ev->motion.x,
		    ev->motion.y);
		rv = 0;
		pthread_mutex_lock(&view->lock);
		if (!TAILQ_EMPTY(&view->windowsh)) {
			rv = window_event_all(ev);
		}
		pthread_mutex_unlock(&view->lock);
		break;
	case SDL_MOUSEBUTTONUP:
		EVENT_DEBUG("SDL_MOUSEBUTTON%s b=%d x=%d y=%d\n",
		    (ev->button.type == SDL_MOUSEBUTTONUP) ?
		    "UP" : "DOWN", ev->button.button, ev->button.x,
		    ev->button.y);
		/* FALLTHROUGH */
	case SDL_MOUSEBUTTONDOWN:
		rv = 0;
		pthread_mutex_lock(&view->lock);
		if (!TAILQ_EMPTY(&view->windowsh)) {
			rv = window_event_all(ev);
		}
		pthread_mutex_unlock(&view->lock);
		break;
	case SDL_JOYAXISMOTION:
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		EVENT_DEBUG("SDL_JOY%s\n",
		    (ev->type == SDL_JOYAXISMOTION) ? "AXISMOTION" :
		    (ev->type == SDL_JOYBUTTONDOWN) ? "BUTTONDOWN" :
		    (ev->type == SDL_JOYBUTTONUP) ? "BUTTONUP" : "???");
		pthread_mutex_lock(&view->lock);
		input_event(joy, ev);
		/* XXX widgets ... */
		pthread_mutex_unlock(&view->lock);
		break;
	case SDL_KEYDOWN:
		event_hotkey(ev);
		/* FALLTHROUGH */
	case SDL_KEYUP:
		EVENT_DEBUG("SDL_KEY%s keysym=%d state=%s\n",
		    (ev->key.type == SDL_KEYUP) ? "UP" : "DOWN",
		    (int)ev->key.keysym.sym,
		    (ev->key.state == SDL_PRESSED) ? "PRESSED" : "RELEASED");

		rv = 0;
		pthread_mutex_lock(&view->lock);
		if (!TAILQ_EMPTY(&view->windowsh)) {
			rv = window_event_all(ev);
		}
		if (rv == 0) {
			input_event(keyboard, ev);
		}
		pthread_mutex_unlock(&view->lock);
		break;
	case SDL_QUIT:
		EVENT_DEBUG("SDL_QUIT\n");
		engine_stop();
		break;
	}
}

#define PUSH_EVENT_ARG(eev, ap, member, type) do {			\
	if ((eev)->argc == EVENT_MAXARGS) {				\
		fatal("too many args\n");				\
	}								\
	(eev)->argv[(eev)->argc++].member = va_arg((ap), type);		\
} while (/*CONSTCOND*/ 0)

#define EVENT_PUSHARG(ap, fmt, eev)				\
	switch ((fmt)) {					\
	case 'i':						\
	case 'o':						\
	case 'u':						\
	case 'x':						\
	case 'X':						\
		PUSH_EVENT_ARG((eev), (ap), i, int);		\
		break;						\
	case 'D':						\
	case 'O':						\
	case 'U':						\
		PUSH_EVENT_ARG((eev), (ap), li, long int);	\
		break;						\
	case 'e':						\
	case 'E':						\
	case 'f':						\
	case 'g':						\
	case 'G':						\
		PUSH_EVENT_ARG((eev), (ap), f, double);		\
		break;						\
	case 'c':						\
		PUSH_EVENT_ARG((eev), (ap), c, char);		\
		break;						\
	case 's':						\
		PUSH_EVENT_ARG((eev), (ap), s, char *);		\
		break;						\
	case 'p':						\
		PUSH_EVENT_ARG((eev), (ap), p, void *);		\
		break;						\
	case ' ':						\
	case ',':						\
	case ';':						\
	case '%':						\
		break;						\
	default:						\
		fatal("unknown argument type\n");		\
	}


/*
 * Register an event handler.
 *
 * Arbitrary arguments are pushed onto an argument stack that the event
 * handler reads. event_post() may push more args onto this stack.
 * The first element is always a pointer to the object.
 */
void
event_new(void *p, char *name, int flags, void (*handler)(int, union evarg *),
    const char *fmt, ...)
{
	struct object *ob = p;
	struct event *eev;

	eev = emalloc(sizeof(struct event));
	eev->name = name;
	eev->flags = flags;
	memset(eev->argv, 0, sizeof(union evarg) * EVENT_MAXARGS);
	eev->argv[0].p = ob;
	eev->argc = 1;
	eev->handler = handler;
	
	if (fmt != NULL) {
		va_list ap;

		for (va_start(ap, fmt); *fmt != '\0'; fmt++) {
			EVENT_PUSHARG(ap, *fmt, eev);
		}
		va_end(ap);
	}

	pthread_mutex_lock(&ob->events_lock);
	TAILQ_INSERT_TAIL(&ob->events, eev, events);
	pthread_mutex_unlock(&ob->events_lock);
}

static void *
event_post_async(void *p)
{
	struct event *eev = p;

	dprintf("%p: async event start\n", (void *)pthread_self());
	eev->handler(eev->argc, eev->argv);
	dprintf("%p: async event end\n", (void *)pthread_self());

	free(eev);
	return (NULL);
}

/*
 * Call an object's event handler, synchronously or asynchronously.
 * This may be called recursively, as long as the object's event
 * handler list is not modified.
 */
void
event_post(void *obp, char *name, const char *fmt, ...)
{
	struct object *ob = obp;
	struct event *eev, *neev;

	pthread_mutex_lock(&ob->events_lock);
	TAILQ_FOREACH(eev, &ob->events, events) {
		if (strcmp(name, eev->name) != 0) {
			continue;
		}
		neev = emalloc(sizeof(struct event));
		memcpy(neev, eev, sizeof(struct event));
		if (fmt != NULL) {
			va_list ap;

			for (va_start(ap, fmt); *fmt != '\0'; fmt++) {
				EVENT_PUSHARG(ap, *fmt, neev);
			}
			va_end(ap);
		}

		if (neev->flags & EVENT_ASYNC) {
			pthread_t async_event_th;

			pthread_create(&async_event_th, NULL,
			    event_post_async, neev);
		} else {
			neev->handler(neev->argc, neev->argv);
			free(neev);
		}
		break;
	}
	pthread_mutex_unlock(&ob->events_lock);
}

/* Forward an event, without modifying the original event structure. */
void
event_forward(void *child, char *name, int argc, union evarg *argv)
{
	union evarg nargv[EVENT_MAXARGS];
	struct object *ob = child;
	struct event *ev;

	pthread_mutex_lock(&ob->events_lock);

	/* Replace argument zero. */
	memcpy(nargv, argv, argc * sizeof(union evarg));
	nargv[0].p = child;

	TAILQ_FOREACH(ev, &ob->events, events) {
		if (strcmp(name, ev->name) != 0) {
			continue;
		}
		ev->handler(argc, nargv);
	}
	pthread_mutex_unlock(&ob->events_lock);
}

