/*	$Csoft: event.c,v 1.65 2002/07/22 05:56:32 vedge Exp $	*/

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

#include "mapedit/mapedit.h"

#include "widget/widget.h"
#include "widget/window.h"
#include "widget/label.h"
#include "widget/text.h"
#include "widget/graph.h"

extern struct gameinfo *gameinfo;	/* script */
extern struct window *game_menu_win;

#ifdef DEBUG

static const struct event_proto {
	char *evname;
	char *fmt;
} protos[] = {
	/* Generic */
	{ "attached", "%p" },				/* object */
	{ "detached", "%p" },				/* object */
	
	/*
	 * Widget type
	 */
	
	/* Change in visibility */
	{ "widget-shown", "%p" },			/* window */
	{ "widget-hidden", "%p" },			/* window */
	
	/* Button pushed */
	{ "button-pushed", NULL },
	/* Checkbox status changed to i */
	{ "checkbox-changed", "%i" },    		/* state */
	/* Text s was entered in textbox */
	{ "textbox-return", "%s" },			/* text */
	/* Character c was inserted in string s */
	{ "textbox-changed", "%s, %c" },		/* string, char */
	/* Radio selection changed */
	{ "radio-changed", "%c, %i" },			/* char, index */

	/*
	 * Window type
	 */

	/* Low-level events */
	{ "window-mousebuttonup", "%i, %i, %i" },	/* button, x, y */
	{ "window-mousebuttondown", "%i, %i, %i" },	/* button, x, y */
	{ "window-mousemotion", "%i, %i, %i, %i" },	/* x, y, xrel, yrel */
	{ "window-mouseout", NULL },
	{ "window-keyup", "%i, %i" },			/* keysym, keymod */
	{ "window-keydown", "%i, %i" },			/* keysym, keymod */
	/* Widget was just scaled */
	{ "widget-scaled", "%i, %i" },			/* width, height */
	/* Change in visibility */
	{ "window-shown", NULL },
	{ "window-hidden", NULL }
};

static struct window *fps_win;

#endif	/* DEBUG */

#define PUSH_EVENT_ARG(eev, ap, member, type) do {			\
	if ((eev)->argc == EVENT_MAXARGS) {				\
		fatal("too many args\n");				\
	}								\
	(eev)->argv[(eev)->argc++].member = va_arg((ap), type);		\
} while (/*CONSTCOND*/ 0)

static void	 event_hotkey(SDL_Event *);
static void	*event_post_async(void *);
#ifdef DEBUG
static void	 event_checkproto(struct object *, char *, const char *);
#endif


static void
event_hotkey(SDL_Event *ev)
{
	pthread_mutex_lock(&world->lock);

	switch (ev->key.keysym.sym) {
#ifdef DEBUG
	case SDLK_r:
		VIEW_REDRAW();
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
#endif
	case SDLK_F1:
		window_show(config->settings_win);
		break;
	case SDLK_v:
		if (ev->key.keysym.mod & KMOD_CTRL) {
			text_msg("AGAR engine v%s\n%s v%d.%d\n%s\n",
			    ENGINE_VERSION, gameinfo->name, gameinfo->ver[0],
			    gameinfo->ver[1], gameinfo->copyright);
		}
		break;
	case SDLK_t:	/* XXX move */
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
	label_printf(fps_label, "%d FPS", (delta));	\
	graph_plot(fps_graph_item, (delta));		\
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

void *
event_loop(void *arg)
{
	SDL_Event ev;
	Uint32 ltick, ntick;
	struct window *win;
	int rv, delta;
#ifdef DEBUG
	struct region *reg;
	struct label *fps_label;
	struct graph *fps_graph;
	struct graph_item *fps_graph_item;
	
	fps_win = window_new("Frames/second", WINDOW_SOLID,
	    view->w-143, view->h-114,
	    133, 104,
	    125, 91);
	reg = region_new(fps_win, REGION_VALIGN,
	    0, 0, 100, 100);
	fps_label = label_new(reg, "...", 0);
	WIDGET(fps_label)->rh = 20;
	fps_graph = graph_new(reg, "Frames/sec", GRAPH_LINES,
	    GRAPH_SCROLL|GRAPH_ORIGIN, 200,
	    100, 80);
	fps_graph_item = graph_add_item(fps_graph, "fps",
	    SDL_MapRGB(view->v->format, 0, 160, 0));
#endif

	/* Start the garbage collection process. */
	object_init_gc();

	for (ntick = 0, ltick = SDL_GetTicks(), delta = EVENT_MAXFPS;;) {
		ntick = SDL_GetTicks();
		if ((ntick - ltick) >= delta) {
			pthread_mutex_lock(&view->lock);

			if (view->gfx_engine == GFX_ENGINE_TILEBASED) {
				struct map *m = view->rootmap->map;

				if (m == NULL) {
					dprintf("NULL map\n");
					pthread_mutex_unlock(&view->lock);
					engine_destroy();
					return (NULL);
				}

				pthread_mutex_lock(&m->lock);
				rootmap_animate(m);
				if (m->redraw != 0) {
					rootmap_draw(m);
					COMPUTE_DELTA(delta, ntick);
					m->redraw = 0;
				}
				pthread_mutex_unlock(&m->lock);
			}

			/* Update the windows. */
			TAILQ_FOREACH(win, &view->windowsh, windows) {
				pthread_mutex_lock(&win->lock);
				if (win->flags & WINDOW_SHOWN) {
					window_animate(win);
				    	/* XXX ignore redraw flag,
					   that will require a fancy
					   microtile algorithm */
					window_draw(win);
				}
				pthread_mutex_unlock(&win->lock);
			}
			pthread_mutex_unlock(&view->lock);

			if (view->gfx_engine == GFX_ENGINE_GUI) {
				/* XXX unclear */
				SDL_UpdateRect(view->v, 0, 0, 0, 0);
				COMPUTE_DELTA(delta, ntick);
			}
			
			ltick = SDL_GetTicks();
		} else if (SDL_PollEvent(&ev)) {
			switch (ev.type) {
			case SDL_VIDEOEXPOSE:
				dprintf("expose\n");
				pthread_mutex_lock(&view->lock);
				switch (view->gfx_engine) {
				case GFX_ENGINE_TILEBASED:
					/* Redraw the map only. */
					pthread_mutex_lock(
					    &view->rootmap->map->lock);
					view->rootmap->map->redraw++;
					pthread_mutex_unlock(
					    &view->rootmap->map->lock);
					break;
				case GFX_ENGINE_GUI:
					/* Redraw the whole screen. */
					SDL_FillRect(view->v, NULL,
					    SDL_MapRGB(view->v->format,
					    0, 0, 0));
					SDL_UpdateRect(view->v, 0, 0, 0 ,0);
					break;
				}
				TAILQ_FOREACH(win, &view->windowsh,
				    windows) {
					pthread_mutex_lock(&win->lock);
					if (win->flags & WINDOW_SHOWN) {
						window_draw(win);
					}
					pthread_mutex_unlock(&win->lock);
				}
				pthread_mutex_unlock(&view->lock);
				break;
			case SDL_MOUSEMOTION:
				rv = 0;
				pthread_mutex_lock(&view->lock);
				if (!TAILQ_EMPTY(&view->windowsh)) {
					rv = window_event_all(&ev);
				}
				pthread_mutex_unlock(&view->lock);
				break;
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
				rv = 0;
				pthread_mutex_lock(&view->lock);
				if (!TAILQ_EMPTY(&view->windowsh)) {
					rv = window_event_all(&ev);
				}
				pthread_mutex_unlock(&view->lock);
				break;
			case SDL_JOYAXISMOTION:
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				pthread_mutex_lock(&view->lock);
				input_event(joy, &ev);
				/* XXX widgets ... */
				pthread_mutex_unlock(&view->lock);
				break;
			case SDL_KEYDOWN:
				event_hotkey(&ev);
				/* FALLTHROUGH */
			case SDL_KEYUP:
				rv = 0;
				pthread_mutex_lock(&view->lock);
				if (!TAILQ_EMPTY(&view->windowsh)) {
					rv = window_event_all(&ev);
				}
				if (rv == 0) {
					input_event(keyboard, &ev);
				}
				pthread_mutex_unlock(&view->lock);
				break;
			case SDL_QUIT:
				engine_stop();
				break;
			}
		}
	}

	engine_destroy();
	return (NULL);
}

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

#ifdef DEBUG
	if (engine_debug)
		event_checkproto(ob, name, NULL);
#endif

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

#ifdef DEBUG
	if (engine_debug)
		event_checkproto(ob, name, fmt);
#endif

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
			/* XXX safe, but sick */
			pthread_mutex_unlock(&ob->events_lock);
			neev->handler(neev->argc, neev->argv);
			free(neev);
			pthread_mutex_lock(&ob->events_lock);
		}
		break;
	}
	pthread_mutex_unlock(&ob->events_lock);
}

#ifdef DEBUG
static void
event_checkproto(struct object *ob, char *evname, const char *fmt)
{
	const struct event_proto *eproto = NULL;
	int i;

	for (i = 0; i < sizeof(protos) / sizeof(struct event_proto); i++) {
		eproto = &protos[i];

		if (strcmp(eproto->evname, evname) == 0) {
			if (fmt != NULL && strcmp(eproto->fmt, fmt) != 0) {
				fatal("'%s' events use format: '%s'\n", evname,
				    eproto->fmt);
			}
			break;
		}
	}
	if (eproto == NULL) {
		fatal("unknown event type `%s'\n", evname);
	}
}
#endif

