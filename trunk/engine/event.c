/*	$Csoft: event.c,v 1.142 2003/03/12 23:02:14 vedge Exp $	*/

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

#include <config/threads.h>

#include "engine.h"
#include "map.h"
#include "input.h"
#include "config.h"
#include "rootmap.h"
#include "view.h"
#include "world.h"

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
#define DEBUG_UNDERRUNS		0x001
#define DEBUG_VIDEO_UPDATES	0x002
#define DEBUG_VIDEORESIZE_EV	0x004
#define DEBUG_VIDEOEXPOSE_EV	0x008
#define DEBUG_MOUSEMOTION_EV	0x010
#define DEBUG_MOUSEBUTTON_EV	0x020
#define DEBUG_JOY_EV		0x040
#define DEBUG_KEY_EV		0x080
#define DEBUG_QUIT_EV		0x100
#define DEBUG_EVENT_NEW		0x200
#define DEBUG_EVENT_DELIVERY	0x400
#define DEBUG_ASYNC_EVENTS	0x800

int	event_debug =	DEBUG_UNDERRUNS|DEBUG_VIDEOEXPOSE_EV|
			DEBUG_QUIT_EV|DEBUG_ASYNC_EVENTS;
#define	engine_debug event_debug
int	event_count;

static struct window *fps_win;
static struct label *fps_label;
static struct graph *fps_graph;
static struct graph_item *fps_refresh_current, *fps_event_count,
    *fps_event_idletime;
#endif	/* DEBUG */

int	event_idle = 1;			/* Delay at full frame rate */
int	event_idletime = -1;		/* SDL_Delay() granularity */

static void	 event_hotkey(SDL_Event *);
static void	 event_dispatch(SDL_Event *);
static void	*event_post_async(void *);

/* View must be locked. */
static void
event_hotkey(SDL_Event *ev)
{
	pthread_mutex_lock(&world->lock);

	switch (ev->key.keysym.sym) {
#ifdef DEBUG
	case SDLK_r:
		if (ev->key.keysym.mod & KMOD_CTRL) {
			SDL_Event vexp;

			vexp.type = SDL_VIDEOEXPOSE;
			SDL_PushEvent(&vexp);
		}
		break;
	case SDLK_F2:
		object_save(world);
		break;
	case SDLK_F4:
		object_load(world);
		break;
	case SDLK_F6:
		window_show(fps_win);
		break;
	case SDLK_F7:
		if (engine_debug > 0) {
			window_show(monitor.toolbar);
		}
		break;
#endif /* DEBUG */
	case SDLK_F1:
		window_show(config->settings);
		break;
	case SDLK_t:
		if (mapedition) {
			window_show(mapedit.win.toolbar);
		}
		break;
	case SDLK_ESCAPE:
		if ((ev->key.keysym.mod & KMOD_SHIFT) == 0) {
			SDL_Event nev;

			nev.type = SDL_QUIT;
			SDL_PushEvent(&nev);
		}
#ifdef DEBUG
		else {
			dprintf("brutal exit\n");
			SDL_Quit();
			exit(0);
		}
#endif
		break;
	default:
		break;
	}

	pthread_mutex_unlock(&world->lock);
}

#ifdef DEBUG
struct window *
event_show_fps_counter(void)
{
	window_show(fps_win);
	return (fps_win);
}

static __inline__ void
event_update_fps_counter(void)
{
	static int einc = 0;

	label_printf(fps_label, "%dms/%dms",
	    view->refresh.current, view->refresh.delay);
	graph_plot(fps_refresh_current, view->refresh.current);
	graph_plot(fps_event_count, event_count * 30 / 10);
	graph_plot(fps_event_idletime, event_idletime);
	graph_scroll(fps_graph, 1);

	if (++einc == 1) {
		event_count = 0;
		einc = 0;
	}
}

static void
event_init_fps_counter(void)
{
	struct region *reg;

	fps_win = window_new("fps-counter", WINDOW_CENTER, -1, -1,
	    248, 166, 125, 91);
	window_set_caption(fps_win, "Refresh rate");
	reg = region_new(fps_win, REGION_VALIGN, 0, 0, 100, -1);
	{
		fps_label = label_new(reg, 100, -1, "...");
	}

	reg = region_new(fps_win, REGION_VALIGN, 0, -1, 100, 0);
	{
		fps_graph = graph_new(reg, "Refresh rate", GRAPH_LINES,
		    GRAPH_SCROLL|GRAPH_ORIGIN, 200, 100, 100);
		fps_refresh_current = graph_add_item(fps_graph,
		    "refresh-current",
		    SDL_MapRGB(view->v->format, 0, 160, 0));
		fps_event_count = graph_add_item(fps_graph,
		    "event-rate",
		    SDL_MapRGB(view->v->format, 0, 0, 180));
		fps_event_idletime = graph_add_item(fps_graph,
		    "event-idle",
		    SDL_MapRGB(view->v->format, 200, 200, 200));
	}
	
	if (engine_debug > 0) {
		window_show(monitor.toolbar);
	}
}
#endif /* DEBUG */

/* Adjust the refresh rate. */
static __inline__ void
event_adjust_refresh(Uint32 ntick)
{
	view->refresh.current = view->refresh.delay - (SDL_GetTicks() - ntick);
#ifdef DEBUG
	if (fps_win->flags & WINDOW_SHOWN) {
		event_update_fps_counter();
	}
#endif
	if (view->refresh.current < 1) {
		view->refresh.current = 1;
	}
}

void
event_loop(void)
{
	SDL_Event ev;
	Uint32 ltick, t = 0;
	struct window *win;
#ifdef DEBUG
	Uint32 eltick;

	event_init_fps_counter();
#endif

	ltick = SDL_GetTicks();
	for (;;) {
		t = SDL_GetTicks();			/* Rendering starts */

		if ((t - ltick) > view->refresh.delay) {
			pthread_mutex_lock(&view->lock);

			view->ndirty = 0;

			/* Tile-based mode */
			if (view->gfx_engine == GFX_ENGINE_TILEBASED) {
				struct map *m = view->rootmap->map;
				
				if (m == NULL) {
					dprintf("NULL map, exiting\n");
					pthread_mutex_unlock(&view->lock);
					return;
				}
				
				pthread_mutex_lock(&world->lock);
				pthread_mutex_lock(&m->lock);

				/* Draw animated nodes. */
				rootmap_animate();

				/* Draw static nodes. */
				if (m->redraw != 0) {
					m->redraw = 0;
					rootmap_draw();
				}
				pthread_mutex_unlock(&m->lock);
				pthread_mutex_unlock(&world->lock);

				/* Queue the video update. */
				VIEW_UPDATE(view->v->clip_rect);
			}

			/* Update the windows. */
			debug_n(DEBUG_VIDEO_UPDATES, "updating windows: ");
			TAILQ_FOREACH(win, &view->windows, windows) {
				pthread_mutex_lock(&win->lock);
				if (win->flags & WINDOW_SHOWN) {
					debug_n(DEBUG_VIDEO_UPDATES, " %s",
					    OBJECT(win)->name);
					window_draw(win);
				}
				pthread_mutex_unlock(&win->lock);
			}
			pthread_mutex_unlock(&view->lock);
			debug_n(DEBUG_VIDEO_UPDATES, ".\n");

			/* Update the display. */
			if (view->ndirty > 0) {
#ifdef HAVE_OPENGL
				if (view->opengl) {
					SDL_GL_SwapBuffers();
				} else {
					SDL_UpdateRects(view->v, view->ndirty,
					    view->dirty);
				}
#else
				SDL_UpdateRects(view->v, view->ndirty,
				    view->dirty);
#endif
				view->ndirty = 0;
				event_adjust_refresh(t);
			}
			ltick = SDL_GetTicks();		/* Rendering ends */
		} else if (SDL_PollEvent(&ev) != 0) {
			event_dispatch(&ev);
#ifdef DEBUG
			event_count++;
#endif
		} else if (event_idle && view->refresh.current > 15) {
			/*
			 * Sleep if the display is not likely to be redrawn
			 * within the next few milliseconds.
			 *
			 * XXX 
			 */
			if (t - ltick < view->refresh.delay -
			    (event_idletime > 9) ? event_idletime : 20) {
				SDL_Delay(1);
			}
			event_idletime = SDL_GetTicks() - t;
		} else {
			event_idletime = 0;
		}
	}
}

static void
event_dispatch(SDL_Event *ev)
{
	struct window *win;
	int rv;
	int old_w, old_h;

	pthread_mutex_lock(&view->lock);

	switch (ev->type) {
	case SDL_VIDEORESIZE:
		debug(DEBUG_VIDEORESIZE_EV,
		    "SDL_VIDEORESIZE: w=%d h=%d\n", ev->resize.w, ev->resize.h);

		SDL_SetVideoMode(ev->resize.w, ev->resize.h, 0, view->v->flags);
		if (view->v == NULL) {
			fatal("setting %dx%d mode: %s",
			    ev->resize.w, ev->resize.h, SDL_GetError());
		}

		old_w = view->w;
		old_h = view->h;
		view->w = ev->resize.w;
		view->h = ev->resize.h;

		TAILQ_FOREACH(win, &view->windows, windows) {
			win->rd.x = win->rd.x * ev->resize.w/old_w;
			win->rd.y = win->rd.y * ev->resize.h/old_h;
			win->rd.w = win->rd.w * ev->resize.w/old_w;
			win->rd.h = win->rd.h * ev->resize.h/old_h;
			win->saved_rd.x = win->saved_rd.x * ev->resize.w/old_w;
			win->saved_rd.y = win->saved_rd.y * ev->resize.h/old_h;
			win->saved_rd.w = win->saved_rd.w * ev->resize.w/old_w;
			win->saved_rd.h = win->saved_rd.h * ev->resize.h/old_h;

			window_resize(win);
		}
		break;
	case SDL_VIDEOEXPOSE:
		debug(DEBUG_VIDEOEXPOSE_EV, "SDL_VIDEOEXPOSE\n");

		switch (view->gfx_engine) {
		case GFX_ENGINE_TILEBASED:
			/* Redraw the map only. */
			pthread_mutex_lock(&world->lock);
			view->rootmap->map->redraw++;
			pthread_mutex_unlock(&world->lock);
			break;
		default:
			break;
		}

		TAILQ_FOREACH(win, &view->windows, windows) {
			pthread_mutex_lock(&win->lock);
			if (win->flags & WINDOW_SHOWN) {
				window_draw(win);
			}
			pthread_mutex_unlock(&win->lock);
		}
#ifdef HAVE_OPENGL
		if (view->opengl) {
			SDL_GL_SwapBuffers();
		}
#endif
		break;
	case SDL_MOUSEMOTION:
		debug(DEBUG_MOUSEMOTION_EV, "SDL_MOUSEMOTION x=%d y=%d\n",
		    ev->motion.x, ev->motion.y);
		rv = 0;
		if (!TAILQ_EMPTY(&view->windows)) {
			rv = window_event(ev);
		}
		break;
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEBUTTONDOWN:
		debug(DEBUG_MOUSEBUTTON_EV,
		    "SDL_MOUSEBUTTON%s b=%d x=%d y=%d\n",
		    (ev->button.type == SDL_MOUSEBUTTONUP) ?
		    "UP" : "DOWN", ev->button.button, ev->button.x,
		    ev->button.y);
		rv = 0;
		if (!TAILQ_EMPTY(&view->windows)) {
			rv = window_event(ev);
		}
		break;
	case SDL_JOYAXISMOTION:
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		debug(DEBUG_JOY_EV, "SDL_JOY%s\n",
		    (ev->type == SDL_JOYAXISMOTION) ? "AXISMOTION" :
		    (ev->type == SDL_JOYBUTTONDOWN) ? "BUTTONDOWN" :
		    (ev->type == SDL_JOYBUTTONUP) ? "BUTTONUP" : "???");
		input_event(INPUT_JOY, ev);
		break;
	case SDL_KEYDOWN:
		event_hotkey(ev);
		/* FALLTHROUGH */
	case SDL_KEYUP:
		debug(DEBUG_KEY_EV, "SDL_KEY%s keysym=%d state=%s\n",
		    (ev->key.type == SDL_KEYUP) ? "UP" : "DOWN",
		    (int)ev->key.keysym.sym,
		    (ev->key.state == SDL_PRESSED) ? "PRESSED" : "RELEASED");

		rv = 0;
		if (!TAILQ_EMPTY(&view->windows)) {
			rv = window_event(ev);
		}
		if (rv == 0) {
			input_event(INPUT_KEYBOARD, ev);
		}
		break;
	case SDL_QUIT:
		debug(DEBUG_QUIT_EV, "SDL_QUIT\n");
		if (view->rootmap == NULL) {			/* XXX */
			pthread_mutex_unlock(&view->lock);
		}
		engine_stop();
	}
	
	/* Process the detach queue. */
	if (!TAILQ_EMPTY(&view->detach)) {
		view_detach_queued();
	}
	pthread_mutex_unlock(&view->lock);
}

#define EVENT_INSERT_ARG(eev, ap, member, type) do {		\
	if ((eev)->argc >= EVENT_MAX_ARGS) {			\
		fatal("too many args");				\
	}							\
	(eev)->argv[(eev)->argc++].member = va_arg((ap), type);	\
} while (0)

#define EVENT_PUSH_ARG(ap, fmt, eev)				\
	switch ((fmt)) {					\
	case 'i':						\
	case 'o':						\
	case 'u':						\
	case 'x':						\
	case 'X':						\
		EVENT_INSERT_ARG((eev), (ap), i, int);		\
		break;						\
	case 'D':						\
	case 'O':						\
	case 'U':						\
		EVENT_INSERT_ARG((eev), (ap), li, long int);	\
		break;						\
	case 'e':						\
	case 'E':						\
	case 'f':						\
	case 'g':						\
	case 'G':						\
		EVENT_INSERT_ARG((eev), (ap), f, double);	\
		break;						\
	case 'c':						\
		EVENT_INSERT_ARG((eev), (ap), c, int);		\
		break;						\
	case 's':						\
		EVENT_INSERT_ARG((eev), (ap), s, char *);	\
		break;						\
	case 'p':						\
		EVENT_INSERT_ARG((eev), (ap), p, void *);	\
		break;						\
	case ' ':						\
	case ',':						\
	case ';':						\
	case '%':						\
		break;						\
	default:						\
		fatal("unknown argument type");			\
	}

/*
 * Register an event handler.
 *
 * Arbitrary arguments are pushed onto an argument stack, whose first
 * element is always a pointer to the parent object.
 */
struct event *
event_new(void *p, char *name, void (*handler)(int, union evarg *),
    const char *fmt, ...)
{
	struct object *ob = p;
	struct event *ev, *eev = NULL;
	int newev = 0;

	debug(DEBUG_EVENT_NEW, "%s: registered `%s' event\n", ob->name, name);

	pthread_mutex_lock(&ob->events_lock);
	TAILQ_FOREACH(ev, &ob->events, events) {
		if (strcmp(ev->name, name) == 0) {
			debug(DEBUG_EVENT_NEW,
			    "replacing %s's existing `%s' handler\n",
			    ob->name, ev->name);
			eev = ev;
			break;
		}
	}
	pthread_mutex_unlock(&ob->events_lock);

	if (eev == NULL) {
		eev = emalloc(sizeof(struct event));
		eev->name = Strdup(name);
		newev = 1;
	}
	eev->flags = 0;
	memset(eev->argv, 0, sizeof(union evarg) * EVENT_MAX_ARGS);
	eev->argv[0].p = ob;
	eev->argc = 1;
	eev->handler = handler;

	if (fmt != NULL) {
		va_list ap;

		for (va_start(ap, fmt); *fmt != '\0'; fmt++) {
			EVENT_PUSH_ARG(ap, *fmt, eev);
		}
		va_end(ap);
	}

	if (newev) {
		pthread_mutex_lock(&ob->events_lock);
		TAILQ_INSERT_TAIL(&ob->events, eev, events);
		pthread_mutex_unlock(&ob->events_lock);
	}
	return (eev);
}

static void *
event_post_async(void *p)
{
#ifdef THREADS
	struct event *eev = p;

	debug(DEBUG_ASYNC_EVENTS, "%p: async event start\n",
	    (void *)pthread_self());
	eev->handler(eev->argc, eev->argv);
	debug(DEBUG_ASYNC_EVENTS, "%p: async event end\n",
	    (void *)pthread_self());

	free(eev);
#else
	fatal("requires THREADS");
#endif
	return (NULL);
}

/*
 * Invoke an event handler. The lock on the object's event list is recursive
 * so the event handler can safely invoke other event handlers.
 *
 * XXX event handlers cannot modify the parent object's event list, such an
 *     operation should be queued like it is done in the window system.
 */
void
event_post(void *obp, char *name, const char *fmt, ...)
{
	struct object *ob = obp;
	struct event *eev, *neev;

	debug(DEBUG_EVENT_DELIVERY, "posting `%s' event to %s\n", name,
	    ob->name);

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
				EVENT_PUSH_ARG(ap, *fmt, neev);
			}
			va_end(ap);
		}

		if (neev->flags & EVENT_ASYNC) {
#ifdef THREADS
			pthread_t async_event_th;

			Pthread_create(&async_event_th, NULL,
			    event_post_async, neev);
#else
			fatal("requires THREADS");
#endif
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
	union evarg nargv[EVENT_MAX_ARGS];
	struct object *ob = child;
	struct event *ev;

	debug(DEBUG_EVENT_DELIVERY, "forwarding `%s' event to %s\n",
	    name, ob->name);

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

