/*	$Csoft: event.c,v 1.181 2004/05/10 11:25:03 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004 CubeSoft Communications, Inc.
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

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/input.h>
#include <engine/config.h>
#include <engine/rootmap.h>
#include <engine/view.h>
#include <engine/timeout.h>

#include <engine/widget/window.h>
#ifdef DEBUG
#include <engine/widget/label.h>
#include <engine/widget/graph.h>
#endif

#include <string.h>
#include <stdarg.h>

#ifdef DEBUG
#define DEBUG_UNDERRUNS		0x001
#define DEBUG_VIDEORESIZE	0x002
#define DEBUG_VIDEOEXPOSE	0x004
#define DEBUG_JOY_EV		0x008
#define DEBUG_KEY_EV		0x010
#define DEBUG_EVENTS		0x040
#define DEBUG_ASYNC		0x080
#define DEBUG_PROPAGATION	0x100
#define DEBUG_SCHED		0x200

int	event_debug = DEBUG_UNDERRUNS|DEBUG_VIDEORESIZE|DEBUG_VIDEOEXPOSE|\
	              DEBUG_ASYNC|DEBUG_PROPAGATION|DEBUG_SCHED;
#define	engine_debug event_debug
int	event_count = 0;

static struct window *fps_win;
static struct label *fps_label;
static struct graph *fps_graph;
static struct graph_item *fps_refresh, *fps_events, *fps_idle;
#endif	/* DEBUG */

int	event_idle = 1;		/* Delay at full frame rate */
int	event_idletime = -1;	/* Approximate SDL_Delay() granularity */
int	event_idlemin = 15;	/* Minimum refresh rate to begin idling */

static void event_propagate(void *, struct event *);
static void event_hotkey(SDL_Event *);
static void event_dispatch(SDL_Event *);
#ifdef THREADS
static void *event_async(void *);
#endif

extern pthread_mutex_t timeout_lock;

static void
event_hotkey(SDL_Event *ev)
{
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
		if (object_save(world) == -1) {
			text_msg(MSG_ERROR, "saving world: %s", error_get());
		}
		break;
	case SDLK_F4:
		if (object_load(world) == -1) {
			text_msg(MSG_ERROR, "loading world: %s", error_get());
		}
		break;
	case SDLK_F6:
		window_show(fps_win);
		break;
#endif /* DEBUG */
	case SDLK_F1:
		window_show(config->settings);
		break;
	case SDLK_F8:
		view_capture(view->v);
		break;
	case SDLK_ESCAPE:
		if ((ev->key.keysym.mod & KMOD_SHIFT) == 0) {
			SDL_Event nev;

			nev.type = SDL_QUIT;
			SDL_PushEvent(&nev);
		}
#ifdef DEBUG
		else {
			dprintf("brutal!\n");
			SDL_Quit();
			exit(0);
		}
#endif
		break;
	default:
		break;
	}
}

#ifdef DEBUG
struct window *
event_fps_window(void)
{
	window_show(fps_win);
	return (fps_win);
}

/* XXX remove this once the graph widget implements polling. */
static __inline__ void
update_fps_counter(void)
{
	static int einc = 0;

	graph_plot(fps_refresh, view->refresh.r);
	graph_plot(fps_events, event_count * 30 / 10);
	graph_plot(fps_idle, event_idletime);
	graph_scroll(fps_graph, 1);

	if (++einc == 1) {
		event_count = 0;
		einc = 0;
	}
}

static void
init_fps_counter(void)
{
	fps_win = window_new("fps-counter");
	window_set_caption(fps_win, _("Refresh rate"));
	window_set_position(fps_win, WINDOW_LOWER_CENTER, 0);
	window_set_closure(fps_win, WINDOW_HIDE);

	fps_label = label_new(fps_win, LABEL_POLLED,
	    "%dms (%dms nom.), %d events, %dms idle",
	    &view->refresh.r, &view->refresh.rnom, &event_count,
	    &event_idletime);

	fps_graph = graph_new(fps_win, "Refresh rate", GRAPH_LINES,
	    GRAPH_ORIGIN, 100);
	/* XXX use polling */

	fps_refresh = graph_add_item(fps_graph, "refresh", 0, 160, 0, 140);
	fps_events = graph_add_item(fps_graph, "event", 0, 0, 180, 140);
	fps_idle = graph_add_item(fps_graph, "idle", 180, 180, 180, 140);
}
#endif /* DEBUG */

/*
 * Loop executing timing-related tasks and processing events.
 * Video updates have priority, followed by timers and events.
 * If the effective refresh rate allows it, idle as well.
 */
void
event_loop(void)
{
	extern struct objectq timeout_objq;
	SDL_Event ev;
	struct window *win;
	struct gfx *gfx;
	Uint32 Tr1, Tr2 = 0;

#ifdef DEBUG
	init_fps_counter();
#endif
	Tr1 = SDL_GetTicks();
	for (;;) {
		Tr2 = SDL_GetTicks();
		if (Tr2-Tr1 >= view->refresh.rnom) {
			pthread_mutex_lock(&view->lock);
			view->ndirty = 0;

			if (view->gfx_engine == GFX_ENGINE_TILEBASED) {
				if (view->rootmap->map == NULL) {
					dprintf("NULL map, exiting\n");
					pthread_mutex_unlock(&view->lock);
					break;
				}
				pthread_mutex_lock(&view->rootmap->map->lock);
				rootmap_animate();
				if (view->rootmap->map->redraw != 0) {
					view->rootmap->map->redraw = 0;
					rootmap_draw();
				}
				pthread_mutex_unlock(&view->rootmap->map->lock);
				view_update(0, 0, 0, 0);
			}
			TAILQ_FOREACH(win, &view->windows, windows) {
				pthread_mutex_lock(&win->lock);
				if (win->visible) {
					widget_draw(win);
					view_update(
					    WIDGET(win)->x, WIDGET(win)->y,
					    WIDGET(win)->w, WIDGET(win)->h);
				}
				pthread_mutex_unlock(&win->lock);
			}

			if (view->ndirty > 0) {
#ifdef HAVE_OPENGL
				if (view->opengl) {
					SDL_GL_SwapBuffers();
				} else
#endif
				{
					SDL_UpdateRects(view->v, view->ndirty,
					    view->dirty);
				}
				view->ndirty = 0;
			}
			pthread_mutex_unlock(&view->lock);

			/* Update the shared animation frame numbers. */
			/* XXX use a flag to avoid overhead with 0 anims  */
			pthread_mutex_lock(&gfxq_lock);
			TAILQ_FOREACH(gfx, &gfxq, gfxs) {
				Uint32 i;

				for (i = 0; i < gfx->nanims; i++) {
					struct gfx_anim *anim = gfx->anims[i];
					struct gfx_cached_anim *can;
					struct gfx_animcl *acl =
					    &gfx->canims[i];

					SLIST_FOREACH(can, &acl->anims, anims) {
						if (++can->anim->frame >=
						    can->anim->nframes)
							can->anim->frame = 0;
						
					}
					if (++anim->frame >= anim->nframes)
						anim->frame = 0;
				}
			}
			pthread_mutex_unlock(&gfxq_lock);

			/* Recalibrate the effective refresh rate. */
			Tr1 = SDL_GetTicks();
			view->refresh.r = view->refresh.rnom - (Tr1-Tr2);
#ifdef DEBUG
			if (fps_win->visible)
				update_fps_counter();
#endif
			if (view->refresh.r < 1)
				view->refresh.r = 1;
		} else if (SDL_PollEvent(&ev) != 0) {
			event_dispatch(&ev);
#ifdef DEBUG
			event_count++;
#endif
		} else if (TAILQ_FIRST(&timeout_objq) != NULL) {      /* Safe */
			timeout_process(Tr2);
		} else if (event_idle && event_idlemin < view->refresh.r) {
			if (Tr2-Tr1 < view->refresh.rnom-event_idletime) {
				SDL_Delay(1);
			}
			event_idletime = SDL_GetTicks() - Tr2;
		} else {
			event_idletime = 0;
		}
	}
}

static void
event_dispatch(SDL_Event *ev)
{
	struct window *win;

	pthread_mutex_lock(&view->lock);

	switch (ev->type) {
	case SDL_VIDEORESIZE:
		{
			int ow, oh;

			debug(DEBUG_VIDEORESIZE, "SDL_VIDEORESIZE: w=%d h=%d\n",
			    ev->resize.w, ev->resize.h);

			/* XXX set a minimum! */
			SDL_SetVideoMode(ev->resize.w, ev->resize.h, 0,
			    view->v->flags);
			if (view->v == NULL) {
				fatal("resizing to %ux%u: %s", ev->resize.w,
				    ev->resize.h, SDL_GetError());
			}
			ow = view->w;
			oh = view->h;
			view->w = ev->resize.w;
			view->h = ev->resize.h;
			TAILQ_FOREACH(win, &view->windows, windows) {
				pthread_mutex_lock(&win->lock);
				WIDGET(win)->x = WIDGET(win)->x*ev->resize.w/ow;
				WIDGET(win)->y = WIDGET(win)->y*ev->resize.h/oh;
				WIDGET(win)->w = WIDGET(win)->w*ev->resize.w/ow;
				WIDGET(win)->h = WIDGET(win)->h*ev->resize.h/oh;

				WIDGET_OPS(win)->scale(win, WIDGET(win)->w,
				    WIDGET(win)->h);
				widget_update_coords(win, WIDGET(win)->x,
				    WIDGET(win)->y);
				pthread_mutex_unlock(&win->lock);
			}
		}
		break;
	case SDL_VIDEOEXPOSE:
		debug(DEBUG_VIDEOEXPOSE, "SDL_VIDEOEXPOSE\n");
		if (view->gfx_engine == GFX_ENGINE_TILEBASED) {
			rootmap_redraw();
		}
		TAILQ_FOREACH(win, &view->windows, windows) {
			pthread_mutex_lock(&win->lock);
			if (win->visible) {
				widget_draw(win);
			}
			pthread_mutex_unlock(&win->lock);
		}
#ifdef HAVE_OPENGL
		if (view->opengl)
			SDL_GL_SwapBuffers();
#endif
		break;
	case SDL_MOUSEMOTION:
		if (!TAILQ_EMPTY(&view->windows))
			window_event(ev);
		break;
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEBUTTONDOWN:
		if (!TAILQ_EMPTY(&view->windows))
			window_event(ev);
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
		debug(DEBUG_KEY_EV, "SDL_KEY%s keysym=%d u=%04x state=%s\n",
		    (ev->key.type == SDL_KEYUP) ? "UP" : "DOWN",
		    (int)ev->key.keysym.sym, ev->key.keysym.unicode,
		    (ev->key.state == SDL_PRESSED) ? "PRESSED" : "RELEASED");
		{
			int rv = 0;

			if (!TAILQ_EMPTY(&view->windows)) {
				rv = window_event(ev);
			}
			if (rv == 0)
				input_event(INPUT_KEYBOARD, ev);
		}
		break;
	case SDL_QUIT:
		switch (view->gfx_engine) {
		case GFX_ENGINE_TILEBASED:
			/* Stop the event loop synchronously. */
			view->rootmap->map = NULL;
			break;
		default:
			/* Shut down immediately. */
			pthread_mutex_unlock(&view->lock);
			engine_destroy();
			/* NOTREACHED */
		}
		break;
	}

	/* Perform deferred window garbage collection. */
	view_detach_queued();

	pthread_mutex_unlock(&view->lock);
}

/*
 * Execute a scheduled event invocation.
 * The object and timeouteq are assumed to be locked.
 */
static Uint32
event_timeout(void *p, Uint32 ival, void *arg)
{
	struct object *ob = p;
	struct event *ev = arg;
	va_list ap;

	/* Propagate event to children. */
	if (ev->flags & EVENT_PROPAGATE) {
		struct object *cobj;
			
		debug(DEBUG_PROPAGATION, "%s: propagate %s (timeout)\n",
		    ob->name, ev->name);
		lock_linkage();
		OBJECT_FOREACH_CHILD(cobj, ob, object) {
			event_propagate(cobj, ev);
		}
		unlock_linkage();
	}
	ev->flags &= ~(EVENT_SCHEDULED);

	/* Invoke the event handler function. */
	if (ev->handler != NULL) {
		ev->handler(ev->argc, ev->argv);
	}
	return (0);
}

/*
 * Register an event handler function for events of the given type.
 * If another event handler is registered for events of the same type,
 * replace it.
 */
struct event *
event_new(void *p, const char *name, void (*handler)(int, union evarg *),
    const char *fmt, ...)
{
	struct object *ob = p;
	struct event *ev;

	pthread_mutex_lock(&ob->lock);
	TAILQ_FOREACH(ev, &ob->events, events) {
		if (strcmp(ev->name, name) == 0)
			break;
	}
	if (ev == NULL) {
		ev = Malloc(sizeof(struct event), M_EVENT);
		strlcpy(ev->name, name, sizeof(ev->name));
		TAILQ_INSERT_TAIL(&ob->events, ev, events);
	}
	memset(ev->argv, 0, sizeof(union evarg)*EVENT_ARGS_MAX);
	ev->flags = 0;
	ev->argv[0].p = ob;
	ev->argc = 1;
	ev->argc_base = 1;
	ev->handler = handler;
	timeout_set(&ev->timeout, event_timeout, ev);

	if (fmt != NULL) {
		va_list ap;

		for (va_start(ap, fmt); *fmt != '\0'; fmt++) {
			EVENT_PUSH_ARG(ap, *fmt, ev);
			ev->argc_base++;
		}
		va_end(ap);
	}

	pthread_mutex_unlock(&ob->lock);
	return (ev);
}

/* Remove the named event handler and cancel any scheduled execution. */
void
event_remove(void *p, const char *name)
{
	struct object *ob = p;
	struct event *ev;

	pthread_mutex_lock(&ob->lock);
	TAILQ_FOREACH(ev, &ob->events, events) {
		if (strcmp(name, ev->name) == 0)
			break;
	}
	if (ev == NULL) {
		goto out;
	}
	if (ev->flags & EVENT_SCHEDULED) {
		pthread_mutex_lock(&timeout_lock);
		if (timeout_scheduled(ob, &ev->timeout)) {
			timeout_del(ob, &ev->timeout);
		}
		pthread_mutex_unlock(&timeout_lock);
		/* XXX concurrent */
	}
	TAILQ_REMOVE(&ob->events, ev, events);
	Free(ev, M_EVENT);
out:
	pthread_mutex_unlock(&ob->lock);
}

/* Forward an event to an object's descendents. */
static void
event_propagate(void *p, struct event *ev)
{
	struct object *ob = p, *cob;

	OBJECT_FOREACH_CHILD(cob, ob, object)
		event_propagate(cob, ev);

	event_forward(ob, ev->name, ev->argc, ev->argv);
}

#ifdef THREADS
/* Invoke an event handler routine asynchronously. */
static void *
event_async(void *p)
{
	struct event *eev = p;
	struct object *rcvr = eev->argv[0].p;

	/* Propagate event to children. */
	if (eev->flags & EVENT_PROPAGATE) {
		struct object *cobj;

		debug(DEBUG_PROPAGATION, "%s: propagate %s (async)\n",
		    rcvr->name, eev->name);
		lock_linkage();
		OBJECT_FOREACH_CHILD(cobj, rcvr, object) {
			event_propagate(cobj, eev);
		}
		unlock_linkage();
	}

	/* Invoke the event handler function. */
	debug(DEBUG_ASYNC, "%s: %s begin\n", rcvr->name, eev->name);
	if (eev->handler != NULL) {
		eev->handler(eev->argc, eev->argv);
	}
	debug(DEBUG_ASYNC, "%s: %s end\n", rcvr->name, eev->name);

	Free(eev, M_EVENT);
	return (NULL);
}
#endif /* THREADS */

/*
 * Execute the event handler routine for the given event, with the given
 * arguments inserted at the end of the argument vector, argument #0 pointing
 * to the receiver object and argument #argc pointing to the sender.
 *
 * Event handler invocations may be nested. However, the event handler table
 * of the object should not be modified while in event context (XXX)
 */
int
event_post(void *sp, void *rp, const char *evname, const char *fmt, ...)
{
	struct object *sndr = sp;
	struct object *rcvr = rp;
	struct event *ev;

	debug(DEBUG_EVENTS, "%s: %s -> %s\n", evname,
	    (sndr != NULL) ? sndr->name : "NULL", rcvr->name);

	pthread_mutex_lock(&rcvr->lock);
	TAILQ_FOREACH(ev, &rcvr->events, events) {
		if (strcmp(evname, ev->name) == 0)
			break;
	}
	if (ev == NULL)
		goto fail;

#ifdef THREADS
	if (ev->flags & EVENT_ASYNC) {
		pthread_t th;
		va_list ap;
		struct event *nev;

		/* Construct the argument vector. */
		/* TODO allocate from a pool */
		nev = Malloc(sizeof(struct event), M_EVENT);
		memcpy(nev, ev, sizeof(struct event));
		if (fmt != NULL) {
			va_start(ap, fmt);
			for (; *fmt != '\0'; fmt++) {
				EVENT_PUSH_ARG(ap, *fmt, nev);
			}
			va_end(ap);
		}
		nev->argv[nev->argc].p = sndr;

		/* Create the event handler function thread. */
		Pthread_create(&th, NULL, event_async, nev);
	} else
#endif /* THREADS */
	{
		struct event tmpev;
		va_list ap;

		/* Construct the argument vector. */
		memcpy(&tmpev, ev, sizeof(struct event));
		if (fmt != NULL) {
			va_start(ap, fmt);
			for (; *fmt != '\0'; fmt++) {
				EVENT_PUSH_ARG(ap, *fmt, &tmpev);
			}
			va_end(ap);
		}
		tmpev.argv[tmpev.argc].p = sndr;

		/* Propagate event to children. */
		if (tmpev.flags & EVENT_PROPAGATE) {
			struct object *cobj;
			
			debug(DEBUG_PROPAGATION, "%s: propagate %s (post)\n",
			    rcvr->name, evname);
			lock_linkage();
			OBJECT_FOREACH_CHILD(cobj, rcvr, object) {
				event_propagate(cobj, &tmpev);
			}
			unlock_linkage();
		}

		/* Invoke the event handler function. */
		if (tmpev.handler != NULL)
			tmpev.handler(tmpev.argc, tmpev.argv);
	}

	pthread_mutex_unlock(&rcvr->lock);
	return (1);
fail:
	pthread_mutex_unlock(&rcvr->lock);
	return (0);
}
 
/*
 * Schedule the execution of the given event in the given number of
 * ticks, with the given arguments inserted at the end of the
 * argument vector. Argument #0 always points to the receiver object,
 * and argument #argc points to the sender.
 *
 * Since the timeout resides in the event handler structure,
 * multiple executions of the same event handler are not possible.
 */
int
event_schedule(void *sp, void *rp, Uint32 ticks, const char *evname,
    const char *fmt, ...)
{
	struct object *sndr = sp;
	struct object *rcvr = rp;
	struct event *ev;
	va_list ap;
	
	debug(DEBUG_SCHED, "%s: sched `%s' (in %u ticks)\n", rcvr->name,
	    evname, ticks);

	pthread_mutex_lock(&timeout_lock);
	pthread_mutex_lock(&rcvr->lock);
	TAILQ_FOREACH(ev, &rcvr->events, events) {
		if (strcmp(evname, ev->name) == 0)
			break;
	}
	if (ev == NULL) {
		goto fail;
	}
	if (ev->flags & EVENT_SCHEDULED) {
		dprintf("%s: resched `%s'\n", rcvr->name, evname);
		timeout_del(rcvr, &ev->timeout);
	}

	/* Construct the argument vector. */
	ev->argc = ev->argc_base;
	if (fmt != NULL) {
		va_start(ap, fmt);
		for (; *fmt != '\0'; fmt++) {
			EVENT_PUSH_ARG(ap, *fmt, ev);
		}
		va_end(ap);
	}
	ev->argv[ev->argc].p = sndr;
	ev->flags |= EVENT_SCHEDULED;

	timeout_add(rcvr, &ev->timeout, ticks);

	pthread_mutex_unlock(&timeout_lock);
	pthread_mutex_unlock(&rcvr->lock);
	return (1);
fail:
	pthread_mutex_unlock(&timeout_lock);
	pthread_mutex_unlock(&rcvr->lock);
	return (0);
}

int
event_resched(void *p, const char *evname, Uint32 ticks)
{
	struct object *ob = p;
	struct event *ev;

	debug(DEBUG_SCHED, "%s: resched `%s' (%u ticks)\n", ob->name, evname,
	    ticks);

	pthread_mutex_lock(&timeout_lock);
	pthread_mutex_lock(&ob->lock);

	TAILQ_FOREACH(ev, &ob->events, events) {
		if (strcmp(evname, ev->name) == 0)
			break;
	}
	if (ev == NULL) {
		goto fail;
	}
	if (ev->flags & EVENT_SCHEDULED) {
		timeout_del(ob, &ev->timeout);
	}
	ev->flags |= EVENT_SCHEDULED;
	timeout_add(ob, &ev->timeout, ticks);

	pthread_mutex_unlock(&ob->lock);
	pthread_mutex_unlock(&timeout_lock);
	return (0);
fail:
	pthread_mutex_unlock(&ob->lock);
	pthread_mutex_unlock(&timeout_lock);
	return (-1);
}

/* Cancel any future execution of the given event. */
int
event_cancel(void *p, const char *evname)
{
	struct object *ob = p;
	struct event *ev;
	int rv = 0;

	pthread_mutex_lock(&timeout_lock);
	pthread_mutex_lock(&ob->lock);
	TAILQ_FOREACH(ev, &ob->events, events) {
		if (strcmp(ev->name, evname) == 0)
			break;
	}
	if (ev == NULL) {
		goto fail;
	}
	if (ev->flags & EVENT_SCHEDULED) {
		dprintf("%s: cancelled timeout %s (cancel)\n", ob->name,
		    evname);
		timeout_del(ob, &ev->timeout);
		rv++;
		ev->flags &= ~(EVENT_SCHEDULED);
	}
	pthread_mutex_unlock(&ob->lock);
	pthread_mutex_unlock(&timeout_lock);
	return (rv);
fail:
	pthread_mutex_unlock(&ob->lock);
	pthread_mutex_unlock(&timeout_lock);
	return (-1);
}

/* Immediately execute the given event handler. */
void
event_execute(void *p, const char *evname)
{
	struct object *ob = p;
	struct event *ev;

	pthread_mutex_lock(&ob->lock);
	TAILQ_FOREACH(ev, &ob->events, events) {
		if (strcmp(ev->name, evname) == 0)
			break;
	}
	if (ev != NULL &&
	    ev->handler != NULL)
		ev->handler(ev->argc, ev->argv);
}

/*
 * Forward an event, without modifying the original event structure, except
 * for the receiver pointer.
 */
/* XXX substitute the sender ptr? */
void
event_forward(void *rp, const char *evname, int argc, union evarg *argv)
{
	union evarg nargv[EVENT_ARGS_MAX];
	struct object *rcvr = rp;
	struct event *ev;

	debug(DEBUG_EVENTS, "%s event to %s\n", evname, rcvr->name);

	pthread_mutex_lock(&rcvr->lock);
	memcpy(nargv, argv, argc*sizeof(union evarg));
	nargv[0].p = rcvr;
	TAILQ_FOREACH(ev, &rcvr->events, events) {
		if (strcmp(evname, ev->name) == 0)
			break;
	}
	if (ev == NULL)
		goto out;

	/* Propagate event to children. */
	if (ev->flags & EVENT_PROPAGATE) {
		struct object *cobj;

		debug(DEBUG_PROPAGATION, "%s: propagate %s (forward)\n",
		    rcvr->name, evname);
		lock_linkage();
		OBJECT_FOREACH_CHILD(cobj, rcvr, object) {
			event_propagate(cobj, ev);
		}
		unlock_linkage();
	}
	/* XXX EVENT_ASYNC.. */
	if (ev->handler != NULL)
		ev->handler(argc, nargv);
out:
	pthread_mutex_unlock(&rcvr->lock);
}

