/*	$Csoft: event.c,v 1.44 2002/05/31 10:48:34 vedge Exp $	*/

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

#include "widget/window.h"
#include "widget/widget.h"
#include "widget/text.h"

extern struct gameinfo *gameinfo;	/* script */
extern struct window *game_menu_win;

#define PUSH_EVENT_ARG(eev, ap, member, type) do {			\
	if ((eev)->argc == EVENT_MAXARGS) {				\
		fatal("too many args\n");				\
	}								\
	(eev)->argv[(eev)->argc++].member = va_arg((ap), type);		\
} while (/*CONSTCOND*/ 0)

static void	 event_hotkey(SDL_Event *);
static void	*event_post_async(void *);
static void	 event_pusharg(va_list, char, struct event *);

static void
event_hotkey(SDL_Event *ev)
{
	pthread_mutex_lock(&world->lock);

	switch (ev->key.keysym.sym) {
#ifdef DEBUG
	case SDLK_m:
		if (ev->key.keysym.mod & KMOD_CTRL) {
			view_dumpmask(mainview);
		}
		break;
	case SDLK_r:
		if (ev->key.keysym.mod & KMOD_CTRL) {
			world->curmap->redraw++;
		}
		break;
	case SDLK_F2:
		object_save(world);
		break;
	case SDLK_F4:
		object_load(world);
		break;
	case SDLK_F5:
		map_verify(world->curmap);
		break;
#endif
	case SDLK_F1:
		pthread_mutex_lock(&mainview->lock);
		pthread_mutex_lock(&config->settings_win->lock);
		window_show(config->settings_win);
		pthread_mutex_unlock(&config->settings_win->lock);
		pthread_mutex_unlock(&mainview->lock);
		break;
	case SDLK_v:
		if (ev->key.keysym.mod & KMOD_CTRL) {
			text_msg(10, TEXT_SLEEP,
			    "AGAR engine v%s\n%s v%d.%d\n%s\n",
			    ENGINE_VERSION, gameinfo->name, gameinfo->ver[0],
			    gameinfo->ver[1], gameinfo->copyright);
		}
		break;
	case SDLK_ESCAPE:
		window_show(game_menu_win);
		break;
	default:
		break;
	}
	pthread_mutex_unlock(&world->lock);
}

void *
event_loop(void *arg)
{
	Uint32 ltick, ntick;
	Sint32 delta;
	SDL_Event ev;
	struct map *m = NULL;
	int rv;
	
	/* Start the garbage collection process. */
	object_init_gc();

	/* XXX pref: max fps */
	for (ntick = 0, ltick = SDL_GetTicks(), delta = 100;;) {
		ntick = SDL_GetTicks();
		if ((ntick - ltick) >= delta) {
			/* XXX inefficient locking */
			pthread_mutex_lock(&world->lock);
			if (world->curmap == NULL) {
				dprintf("map vanished\n");
				pthread_mutex_unlock(&world->lock);
				engine_destroy();
				return (NULL);
			}
			m = world->curmap;
			pthread_mutex_lock(&m->lock);	
			map_animate(m);
			if (m->redraw != 0) {
				m->redraw = 0;
				map_draw(m);
				delta = m->fps - (SDL_GetTicks() - ntick);
				if (delta < 1) {
					dprintf("overrun (delta=%d)\n", delta);
					delta = 1;
				}
				text_drawall();		/* XXX window */
			}
			pthread_mutex_unlock(&m->lock);
			pthread_mutex_unlock(&world->lock);

			/* XXX inefficient, should share locks */
			pthread_mutex_lock(&mainview->lock);
			if (!TAILQ_EMPTY(&mainview->windowsh)) {
				window_draw_all();
			}
			pthread_mutex_unlock(&mainview->lock);

			ltick = SDL_GetTicks();
		} else if (SDL_PollEvent(&ev)) {
			switch (ev.type) {
			case SDL_VIDEOEXPOSE:
				view_redraw(mainview);
				break;
			case SDL_MOUSEMOTION:
				if (curmapedit != NULL) {	/* XXX */
					mapedit_event(curmapedit, &ev);
				}
				break;
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
				rv = 0;
				pthread_mutex_lock(&mainview->lock);
				if (!TAILQ_EMPTY(&mainview->windowsh)) {
					rv = window_event_all(mainview, &ev);
				}
				pthread_mutex_unlock(&mainview->lock);
				if (rv == 0) {
					if (curmapedit != NULL) { /* XXX */
						mapedit_event(curmapedit, &ev);
					}
				}
				break;
			case SDL_JOYAXISMOTION:
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				if (curmapedit != NULL) {	/* XXX */
					mapedit_event(curmapedit, &ev);
				} else {
					input_event(joy, &ev);
				}
				break;
			case SDL_KEYDOWN:
				event_hotkey(&ev);
				/* FALLTHROUGH */
			case SDL_KEYUP:
				rv = 0;
				pthread_mutex_lock(&mainview->lock);
				if (!TAILQ_EMPTY(&mainview->windowsh)) {
					rv = window_event_all(mainview, &ev);
				}
				pthread_mutex_unlock(&mainview->lock);
				if (rv == 0) {
					if (curmapedit != NULL) { /* XXX */
						mapedit_event(curmapedit, &ev);
					} else {
						input_event(keyboard, &ev);
					}
				}
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

static void
event_pusharg(va_list ap, char fmt, struct event *eev)
{
	switch (fmt) {
	case 'd':
	case 'i':
	case 'o':
	case 'u':
	case 'x':
	case 'X':
		PUSH_EVENT_ARG(eev, ap, i, int);
		break;
	case 'D':
	case 'O':
	case 'U':
		PUSH_EVENT_ARG(eev, ap, li, long int);
		break;
	case 'e':
	case 'E':
	case 'f':
	case 'g':
	case 'G':
		PUSH_EVENT_ARG(eev, ap, f, double);
		break;
	case 'c':
		PUSH_EVENT_ARG(eev, ap, c, char);
		break;
	case 's':
		PUSH_EVENT_ARG(eev, ap, s, char *);
		break;
	case 'p':
		PUSH_EVENT_ARG(eev, ap, p, void *);
		break;
	case ' ':
	case ',':
	case ';':
	case '%':
		break;
	default:
		fatal("unknown argument type\n");
	}
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
			event_pusharg(ap, *fmt, eev);
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
event_post(void *obp, const char *name, const char *fmt, ...)
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
				event_pusharg(ap, *fmt, neev);
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
	}
	pthread_mutex_unlock(&ob->events_lock);
}

