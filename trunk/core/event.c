/*
 * Copyright (c) 2001-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * Implementation of the generic event system for AG_Object.
 */

#include <config/threads.h>
#include <config/have_opengl.h>

/* #define OPENGL_INVERTED_Y */

#include <core/core.h>
#include <core/config.h>
#include <core/view.h>

#include <gui/window.h>
#include <gui/menu.h>
#ifdef DEBUG
#include <gui/label.h>
#include <gui/fixed_plotter.h>
#endif

#include <string.h>
#include <stdarg.h>

#ifdef DEBUG
#define DEBUG_UNDERRUNS		0x001
#define DEBUG_JOY_EV		0x008
#define DEBUG_KEY_EV		0x010
#define DEBUG_EVENTS		0x040
#define DEBUG_ASYNC		0x080
#define DEBUG_PROPAGATION	0x100
#define DEBUG_SCHED		0x200

int	agEventDebugLvl = DEBUG_UNDERRUNS|DEBUG_ASYNC;
#define	agDebugLvl agEventDebugLvl

int	agEventAvg = 0;		/* Number of events in last frame */
int	agIdleAvg = 0;		/* Measured SDL_Delay() granularity */

AG_Window *agPerfWindow;
static AG_FixedPlotter *agPerfGraph;
static AG_FixedPlotterItem *agPerfFPS, *agPerfEvnts, *agPerfIdle;
#endif	/* DEBUG */

int agIdleThresh = 20;			/* Idling threshold */
int agBgPopupMenu = 0;			/* Background popup menu */

struct ag_global_key {
	SDLKey keysym;
	SDLMod keymod;
	void (*fn)(void);
	void (*fn_ev)(AG_Event *);
	SLIST_ENTRY(ag_global_key) gkeys;
};
static SLIST_HEAD(,ag_global_key) agGlobalKeys =
    SLIST_HEAD_INITIALIZER(&agGlobalKeys);

static void PropagateEvent(AG_Object *, AG_Object *, AG_Event *);
static void *EventThread(void *);

const char *agEvArgTypeNames[] = {
	"pointer",
	"string",
	"char",
	"Uchar",
	"int",
	"Uint",
	"long",
	"Ulong",
	"float",
	"double"
};

#ifdef DEBUG
/*
 * Update the performance counters.
 * XXX remove this once the graph widget implements polling.
 */
static __inline__ void
PerfMonitorUpdate(void)
{
	static int einc = 0;

	AG_FixedPlotterDatum(agPerfFPS, agView->rCur);
	AG_FixedPlotterDatum(agPerfEvnts, agEventAvg * 30 / 10);
	AG_FixedPlotterDatum(agPerfIdle, agIdleAvg);
	AG_FixedPlotterScroll(agPerfGraph, 1);

	if (++einc == 1) {
		agEventAvg = 0;
		einc = 0;
	}
}

AG_Window *
AG_EventShowPerfGraph(void)
{
	AG_WindowShow(agPerfWindow);
	return (agPerfWindow);
}

static void
PerfMonitorInit(void)
{
	AG_Label *lbl;

	agPerfWindow = AG_WindowNewNamed(0, "event-fps-counter");
	AG_WindowSetCaption(agPerfWindow, _("Performance counters"));
	AG_WindowSetPosition(agPerfWindow, AG_WINDOW_LOWER_CENTER, 0);
	lbl = AG_LabelNewPolled(agPerfWindow, AG_LABEL_HFILL,
	    "%dms (nom %dms), %d evnt, %dms idle",
	    &agView->rCur, &agView->rNom, &agEventAvg, &agIdleAvg);
	AG_LabelPrescale(lbl, 1, "000ms (nom 000ms), 00 evnt, 000ms idle");
	agPerfGraph = AG_FixedPlotterNew(agPerfWindow, AG_FIXED_PLOTTER_LINES,
	                                               AG_FIXED_PLOTTER_XAXIS|
						       AG_FIXED_PLOTTER_EXPAND);
	agPerfFPS = AG_FixedPlotterCurve(agPerfGraph, "refresh", 0,160,0, 99);
	agPerfEvnts = AG_FixedPlotterCurve(agPerfGraph, "event", 0,0,180, 99);
	agPerfIdle = AG_FixedPlotterCurve(agPerfGraph, "idle", 180,180,180, 99);
}
#endif /* DEBUG */

/*
 * Try to ensure a fixed frame rate, and idle as much as possible.
 * TODO provide MD hooks for finer idling.
 */
void
AG_EventLoop_FixedFPS(void)
{
	extern struct ag_objectq agTimeoutObjQ;
	SDL_Event ev;
	AG_Window *win;
	Uint32 Tr1, Tr2 = 0;

#ifdef DEBUG
	PerfMonitorInit();
#endif
	Tr1 = SDL_GetTicks();
	for (;;) {
		Tr2 = SDL_GetTicks();
		if (Tr2-Tr1 >= agView->rNom) {
			AG_MutexLock(&agView->lock);
			agView->ndirty = 0;
#ifdef HAVE_OPENGL
			if (agView->opengl) {
				AG_LockGL();
				glClear(GL_COLOR_BUFFER_BIT|
				        GL_DEPTH_BUFFER_BIT);
			}
#endif
			TAILQ_FOREACH(win, &agView->windows, windows) {
				AG_MutexLock(&win->lock);
				if (!win->visible) {
					AG_MutexUnlock(&win->lock);
					continue;
				}
				AG_WidgetDraw(win);
				AG_MutexUnlock(&win->lock);

				if (win->flags & AG_WINDOW_NOUPDATERECT) {
					continue;
				}
				AG_QueueVideoUpdate(
				    WIDGET(win)->x, WIDGET(win)->y,
				    WIDGET(win)->w, WIDGET(win)->h);
			}
			if (agView->ndirty > 0) {
#ifdef HAVE_OPENGL
				if (agView->opengl) {
					SDL_GL_SwapBuffers();
				} else
#endif
				{
					SDL_UpdateRects(agView->v,
					    agView->ndirty,
					    agView->dirty);
				}
				agView->ndirty = 0;
			}
#ifdef HAVE_OPENGL
			if (agView->opengl)
				AG_UnlockGL();
#endif
			AG_MutexUnlock(&agView->lock);

			/* Recalibrate the effective refresh rate. */
			Tr1 = SDL_GetTicks();
			agView->rCur = agView->rNom - (Tr1-Tr2);
#ifdef DEBUG
			if (agPerfWindow->visible)
				PerfMonitorUpdate();
#endif
			if (agView->rCur < 1) {
				agView->rCur = 1;
			}
		} else if (SDL_PollEvent(&ev) != 0) {
			AG_ProcessEvent(&ev);
#ifdef DEBUG
			agEventAvg++;
#endif
		} else if (TAILQ_FIRST(&agTimeoutObjQ) != NULL) {     /* Safe */
			AG_ProcessTimeout(Tr2);
		} else if (agView->rCur > agIdleThresh) {
			SDL_Delay(agView->rCur - agIdleThresh);
#ifdef DEBUG
			agIdleAvg = SDL_GetTicks() - Tr2;
		} else {
			agIdleAvg = 0;
		}
#else
		}
#endif
	}
}

static void
UnminimizeWindow(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);

	if (!win->visible) {
		AG_WindowShow(win);
		win->flags &= ~(AG_WINDOW_MINIMIZED);
	} else {
		AG_WindowFocus(win);
	}
}

Uint8
AG_MouseGetState(int *x, int *y)
{
	Uint8 rv;

	rv = SDL_GetMouseState(x, y);
#ifdef OPENGL_INVERTED_Y
	if (agView->opengl && y != NULL)
		*y = agView->h - *y;
#endif
	return (rv);
}

void
AG_ProcessEvent(SDL_Event *ev)
{
	AG_Window *win;
	int rv;

	AG_MutexLock(&agView->lock);

	switch (ev->type) {
	case SDL_MOUSEMOTION:
#ifdef OPENGL_INVERTED_Y
		if (agView->opengl) {
			ev->motion.y = agView->h - ev->motion.y;
			ev->motion.yrel = -ev->motion.yrel;
		}
#endif
		AG_WindowEvent(ev);
		break;
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEBUTTONDOWN:
#ifdef OPENGL_INVERTED_Y
		if (agView->opengl)
			ev->button.y = agView->h - ev->button.y;
#endif
		if (AG_WindowEvent(ev) == 0 &&
		    agBgPopupMenu && ev->type == SDL_MOUSEBUTTONDOWN &&
		    (ev->button.button == SDL_BUTTON_MIDDLE ||
		     ev->button.button == SDL_BUTTON_RIGHT)) {
			AG_Menu *me;
			AG_MenuItem *mi;
			AG_Window *win;
			int x, y;

			me = Malloc(sizeof(AG_Menu), M_OBJECT);
			AG_MenuInit(me, 0);
			mi = me->itemSel = AG_MenuAddItem(me, NULL);

			TAILQ_FOREACH_REVERSE(win, &agView->windows, ag_windowq,
			    windows) {
				if (strcmp(win->caption, "win-popup")
				    == 0) {
					continue;
				}
				AG_MenuAction(mi, win->caption,
				    OBJ_ICON,
				    UnminimizeWindow, "%p", win);
			}
				
			AG_MouseGetState(&x, &y);
			AG_MenuExpand(me, mi, x+4, y+4);
		}
		break;
	case SDL_KEYDOWN:
		{
			struct ag_global_key *gk;

			SLIST_FOREACH(gk, &agGlobalKeys, gkeys) {
				if (gk->keysym == ev->key.keysym.sym &&
				    ((gk->keymod == KMOD_NONE &&
				      ev->key.keysym.mod == KMOD_NONE) ||
				      ev->key.keysym.mod & gk->keymod)) {
					if (gk->fn != NULL) {
						gk->fn();
					} else if (gk->fn_ev != NULL) {
						gk->fn_ev(NULL);
					}
				}
			}
		}
		/* FALLTHROUGH */
	case SDL_KEYUP:
		debug(DEBUG_KEY_EV, "SDL_KEY%s keysym=%d u=%04x state=%s\n",
		    (ev->key.type == SDL_KEYUP) ? "UP" : "DOWN",
		    (int)ev->key.keysym.sym, ev->key.keysym.unicode,
		    (ev->key.state == SDL_PRESSED) ?
		    "PRESSED" : "RELEASED");
		AG_WindowEvent(ev);
		break;
	case SDL_JOYAXISMOTION:
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		debug(DEBUG_JOY_EV, "SDL_JOY%s\n",
		    (ev->type == SDL_JOYAXISMOTION) ? "AXISMOTION" :
		    (ev->type == SDL_JOYBUTTONDOWN) ? "BUTTONDOWN" :
		    (ev->type == SDL_JOYBUTTONUP) ? "BUTTONUP" :
		    "???");
		AG_WindowEvent(ev);
		break;
	case SDL_VIDEORESIZE:
		AG_ResizeDisplay(ev->resize.w, ev->resize.h);
		break;
	case SDL_VIDEOEXPOSE:
		AG_ViewVideoExpose();
		break;
	case SDL_QUIT:
		if (!agTerminating &&
		    AG_FindEventHandler(agWorld, "quit") != NULL) {
			AG_PostEvent(NULL, agWorld, "quit", NULL);
			break;
		}
		/* FALLTHROUGH */
	case SDL_USEREVENT:
		AG_MutexUnlock(&agView->lock);
		agTerminating = 1;
		AG_Destroy();
		/* NOTREACHED */
		break;
	}
out:
	AG_ViewDetachQueued();
	AG_MutexUnlock(&agView->lock);
}

/*
 * Execute a scheduled event invocation.
 * The object and timeouteq are assumed to be locked.
 */
static Uint32
SchedEventTimeout(void *p, Uint32 ival, void *arg)
{
	AG_Object *ob = p;
	AG_Event *ev = arg;
	va_list ap;
	
	debug(DEBUG_SCHED, "%s: timeout `%s' (ival=%u)\n", ob->name,
	    ev->name, ival);
	ev->flags &= ~(AG_EVENT_SCHEDULED);

	/* Propagate event to children. */
	if (ev->flags & AG_EVENT_PROPAGATE) {
		AG_Object *child;
			
		debug(DEBUG_PROPAGATION, "%s: propagate %s (timeout)\n",
		    ob->name, ev->name);
		AG_LockLinkage();
		OBJECT_FOREACH_CHILD(child, ob, ag_object) {
			PropagateEvent(ob, child, ev);
		}
		AG_UnlockLinkage();
	}

	/* Invoke the event handler routine. */
	if (ev->handler != NULL) {
		ev->handler(ev);
	}
	return (0);
}

void
AG_BindGlobalKey(SDLKey keysym, SDLMod keymod, void (*fn)(void))
{
	struct ag_global_key *gk;

	gk = Malloc(sizeof(struct ag_global_key), M_EVENT);
	gk->keysym = keysym;
	gk->keymod = keymod;
	gk->fn = fn;
	gk->fn_ev = NULL;
	SLIST_INSERT_HEAD(&agGlobalKeys, gk, gkeys);
}

void
AG_BindGlobalKeyEv(SDLKey keysym, SDLMod keymod, void (*fn_ev)(AG_Event *))
{
	struct ag_global_key *gk;

	gk = Malloc(sizeof(struct ag_global_key), M_EVENT);
	gk->keysym = keysym;
	gk->keymod = keymod;
	gk->fn = NULL;
	gk->fn_ev = fn_ev;
	SLIST_INSERT_HEAD(&agGlobalKeys, gk, gkeys);
}

AG_Event *
AG_SetEvent(void *p, const char *name, AG_EventFn fn, const char *fmt,
    ...)
{
	AG_Object *ob = p;
	AG_Event *ev;

	AG_MutexLock(&ob->lock);
	if (name != NULL) {
		TAILQ_FOREACH(ev, &ob->events, events) {
			if (strcmp(ev->name, name) == 0)
				break;
		}
	} else {
		ev = NULL;
	}
	if (ev == NULL) {
		ev = Malloc(sizeof(AG_Event), M_EVENT);
		if (name != NULL) {
			strlcpy(ev->name, name, sizeof(ev->name));
		} else {
			/* XXX use something faster */
			snprintf(ev->name, sizeof(ev->name), "__%u",
			    ob->nevents);
		}
		TAILQ_INSERT_TAIL(&ob->events, ev, events);
		ob->nevents++;
	}
	ev->flags = 0;
	ev->argv[0].p = ob;
	ev->argt[0] = AG_EVARG_POINTER;
	ev->argn[0] = "_self";
	ev->argc = 1;
	ev->handler = fn;
	AG_SetTimeout(&ev->timeout, SchedEventTimeout, ev, 0);
	AG_EVENT_GET_ARGS(ev, fmt);
	ev->argc0 = ev->argc;
	AG_MutexUnlock(&ob->lock);
	return (ev);
}

/*
 * Register an additional event handler function for events of the
 * given type.
 */
AG_Event *
AG_AddEvent(void *p, const char *name, AG_EventFn fn, const char *fmt, ...)
{
	AG_Object *ob = p;
	AG_Event *ev;

	AG_MutexLock(&ob->lock);

	ev = Malloc(sizeof(AG_Event), M_EVENT);
	if (name != NULL) {
		strlcpy(ev->name, name, sizeof(ev->name));
	} else {
		snprintf(ev->name, sizeof(ev->name), "__%u", ob->nevents);
	}
	ev->flags = 0;
	ev->argv[0].p = ob;
	ev->argt[0] = AG_EVARG_POINTER;
	ev->argn[0] = "_self";
	ev->argc = 1;
	ev->handler = fn;
	AG_SetTimeout(&ev->timeout, SchedEventTimeout, ev, 0);
	AG_EVENT_GET_ARGS(ev, fmt);
	ev->argc0 = ev->argc;

	TAILQ_INSERT_TAIL(&ob->events, ev, events);
	ob->nevents++;
	AG_MutexUnlock(&ob->lock);
	return (ev);
}

/* Remove the named event handler and cancel any scheduled execution. */
void
AG_UnsetEvent(void *p, const char *name)
{
	AG_Object *ob = p;
	AG_Event *ev;

	AG_MutexLock(&ob->lock);
	TAILQ_FOREACH(ev, &ob->events, events) {
		if (strcmp(name, ev->name) == 0)
			break;
	}
	if (ev == NULL) {
		goto out;
	}
	if (ev->flags & AG_EVENT_SCHEDULED) {
		/* XXX concurrent */
		AG_DelTimeout(ob, &ev->timeout);
	}
	TAILQ_REMOVE(&ob->events, ev, events);
	ob->nevents--;
	Free(ev, M_EVENT);
out:
	AG_MutexUnlock(&ob->lock);
}

AG_Event *
AG_FindEventHandler(void *p, const char *name)
{
	AG_Object *ob = p;
	AG_Event *ev;
	
	AG_MutexLock(&ob->lock);
	TAILQ_FOREACH(ev, &ob->events, events) {
		if (strcmp(name, ev->name) == 0)
			break;
	}
	AG_MutexUnlock(&ob->lock);
	return (ev);
}

/* Forward an event to an object's descendents. */
static void
PropagateEvent(AG_Object *sndr, AG_Object *rcvr, AG_Event *ev)
{
	AG_Object *chld;

	OBJECT_FOREACH_CHILD(chld, rcvr, ag_object) {
		PropagateEvent(rcvr, chld, ev);
	}
	AG_ForwardEvent(sndr, rcvr, ev);
}

#ifdef THREADS
/* Invoke an event handler routine asynchronously. */
static void *
EventThread(void *p)
{
	AG_Event *eev = p;
	AG_Object *rcvr = eev->argv[0].p;
	AG_Object *chld;

	if (eev->flags & AG_EVENT_PROPAGATE) {
		debug(DEBUG_PROPAGATION, "%s: propagate %s (async)\n",
		    rcvr->name, eev->name);
		AG_LockLinkage();
		OBJECT_FOREACH_CHILD(chld, rcvr, ag_object) {
			PropagateEvent(rcvr, chld, eev);
		}
		AG_UnlockLinkage();
	}
	debug(DEBUG_ASYNC, "%s: %s begin\n", rcvr->name, eev->name);
	if (eev->handler != NULL) {
		eev->handler(eev);
	}
	debug(DEBUG_ASYNC, "%s: %s end\n", rcvr->name, eev->name);
	Free(eev, M_EVENT);
	return (NULL);
}
#endif /* THREADS */

/*
 * Execute the event handler routine for the given event. The given arguments
 * are appended to the end of the argument vector.
 *
 * Event handler invocations may be nested. However, the event handler table
 * of the object should not be modified while in event context (XXX)
 */
void
AG_PostEvent(void *sp, void *rp, const char *evname, const char *fmt, ...)
{
	AG_Object *sndr = sp;
	AG_Object *rcvr = rp;
	AG_Event *ev;
	AG_Object *chld;

	debug(DEBUG_EVENTS, "%s: %s -> %s\n", evname,
	    (sndr != NULL) ? sndr->name : "NULL", rcvr->name);

	AG_MutexLock(&rcvr->lock);
	TAILQ_FOREACH(ev, &rcvr->events, events) {
		if (strcmp(evname, ev->name) != 0)
			continue;
#ifdef THREADS
		if (ev->flags & AG_EVENT_ASYNC) {
			AG_Thread th;
			AG_Event *evNew;

			/* TODO allocate from an per-object pool */
			evNew = Malloc(sizeof(AG_Event), M_EVENT);
			memcpy(evNew, ev, sizeof(AG_Event));
			AG_EVENT_GET_ARGS(evNew, fmt);
			evNew->argv[evNew->argc].p = sndr;
			evNew->argt[evNew->argc] = AG_EVARG_POINTER;
			evNew->argn[evNew->argc] = "_sender";
			AG_ThreadCreate(&th, NULL, EventThread, evNew);
		} else
#endif /* THREADS */
		{
			AG_Event tmpev;
			va_list ap;

			memcpy(&tmpev, ev, sizeof(AG_Event));
			AG_EVENT_GET_ARGS(&tmpev, fmt);
			tmpev.argv[tmpev.argc].p = sndr;
			tmpev.argt[tmpev.argc] = AG_EVARG_POINTER;
			tmpev.argn[tmpev.argc] = "_sender";

			if (tmpev.flags & AG_EVENT_PROPAGATE) {
				debug(DEBUG_PROPAGATION,
				    "%s: propagate %s (post)\n",
				    rcvr->name, evname);
				AG_LockLinkage();
				OBJECT_FOREACH_CHILD(chld, rcvr, ag_object) {
					PropagateEvent(rcvr, chld, &tmpev);
				}
				AG_UnlockLinkage();
			}
			if (tmpev.handler != NULL)
				tmpev.handler(&tmpev);
		}
	}
	AG_MutexUnlock(&rcvr->lock);
}

/*
 * Schedule the execution of the given event in the given number of
 * ticks. The given arguments are appended to the argument vector.
 */
int
AG_SchedEvent(void *sp, void *rp, Uint32 ticks, const char *evname,
    const char *fmt, ...)
{
	AG_Object *sndr = sp;
	AG_Object *rcvr = rp;
	AG_Event *ev;
	
	debug(DEBUG_SCHED, "%s: sched `%s' (in %u ticks)\n", rcvr->name,
	    evname, ticks);

	AG_LockTiming();
	AG_MutexLock(&rcvr->lock);
	TAILQ_FOREACH(ev, &rcvr->events, events) {
		if (strcmp(evname, ev->name) == 0)
			break;
	}
	if (ev == NULL) {
		goto fail;
	}
	if (ev->flags & AG_EVENT_SCHEDULED) {
		debug(DEBUG_SCHED, "%s: resched `%s'\n", rcvr->name, evname);
		AG_DelTimeout(rcvr, &ev->timeout);
	}
	ev->argc = ev->argc0;
	AG_EVENT_GET_ARGS(ev, fmt);
	ev->argv[ev->argc].p = sndr;
	ev->argt[ev->argc] = AG_EVARG_POINTER;
	ev->argn[ev->argc] = "_sender";
	ev->flags |= AG_EVENT_SCHEDULED;
	AG_AddTimeout(rcvr, &ev->timeout, ticks);
	AG_UnlockTiming();
	AG_MutexUnlock(&rcvr->lock);
	return (0);
fail:
	AG_UnlockTiming();
	AG_MutexUnlock(&rcvr->lock);
	return (-1);
}

int
AG_ReschedEvent(void *p, const char *evname, Uint32 ticks)
{
	AG_Object *ob = p;
	AG_Event *ev;

	debug(DEBUG_SCHED, "%s: resched `%s' (%u ticks)\n", ob->name, evname,
	    ticks);

	AG_LockTiming();
	AG_MutexLock(&ob->lock);

	TAILQ_FOREACH(ev, &ob->events, events) {
		if (strcmp(evname, ev->name) == 0)
			break;
	}
	if (ev == NULL) {
		goto fail;
	}
	if (ev->flags & AG_EVENT_SCHEDULED) {
		AG_DelTimeout(ob, &ev->timeout);
	}
	ev->flags |= AG_EVENT_SCHEDULED;
	AG_AddTimeout(ob, &ev->timeout, ticks);

	AG_MutexUnlock(&ob->lock);
	AG_UnlockTiming();
	return (0);
fail:
	AG_MutexUnlock(&ob->lock);
	AG_UnlockTiming();
	return (-1);
}

/* Cancel any future execution of the given event. */
int
AG_CancelEvent(void *p, const char *evname)
{
	AG_Object *ob = p;
	AG_Event *ev;
	int rv = 0;

	AG_LockTiming();
	AG_MutexLock(&ob->lock);
	TAILQ_FOREACH(ev, &ob->events, events) {
		if (strcmp(ev->name, evname) == 0)
			break;
	}
	if (ev == NULL) {
		goto fail;
	}
	if (ev->flags & AG_EVENT_SCHEDULED) {
		debug(DEBUG_SCHED, "%s: cancelled timeout %s (cancel)\n",
		    ob->name, evname);
		AG_DelTimeout(ob, &ev->timeout);
		rv++;
		ev->flags &= ~(AG_EVENT_SCHEDULED);
	}
	/* XXX concurrent */
	AG_MutexUnlock(&ob->lock);
	AG_UnlockTiming();
	return (rv);
fail:
	AG_MutexUnlock(&ob->lock);
	AG_UnlockTiming();
	return (-1);
}

/* Immediately execute the given event handler. */
void
AG_ExecEventFn(void *obj, AG_Event *ev)
{
	if (ev->handler != NULL)
		AG_PostEvent(NULL, obj, ev->name, NULL);
}

/*
 * Forward an event, without modifying the original event structure, except
 * for the sender and receiver pointers.
 */
void
AG_ForwardEvent(void *pSndr, void *pRcvr, AG_Event *event)
{
	AG_Object *sndr = pSndr;
	AG_Object *rcvr = pRcvr;
	AG_Object *chld;
	AG_Event *ev;
	va_list ap;

	debug(DEBUG_EVENTS, "%s event to %s\n", event->name, rcvr->name);
	AG_MutexLock(&rcvr->lock);
	TAILQ_FOREACH(ev, &rcvr->events, events) {
		if (strcmp(event->name, ev->name) == 0)
			break;
	}
	if (ev == NULL)
		goto out;
#ifdef THREADS
	if (ev->flags & AG_EVENT_ASYNC) {
		AG_Thread th;
		AG_Event *evNew;

		/* TODO allocate from an per-object pool */
		evNew = Malloc(sizeof(AG_Event), M_EVENT);
		memcpy(evNew, ev, sizeof(AG_Event));
		evNew->argv[0].p = rcvr;
		evNew->argv[evNew->argc].p = sndr;
		evNew->argt[evNew->argc] = AG_EVARG_POINTER;
		evNew->argn[evNew->argc] = "_sender";
		AG_ThreadCreate(&th, NULL, EventThread, evNew);
	} else
#endif /* THREADS */
	{
		AG_Event tmpev;

		memcpy(&tmpev, event, sizeof(AG_Event));
		tmpev.argv[0].p = rcvr;
		tmpev.argv[tmpev.argc].p = sndr;
		tmpev.argt[tmpev.argc] = AG_EVARG_POINTER;
		tmpev.argn[tmpev.argc] = "_sender";

		if (ev->flags & AG_EVENT_PROPAGATE) {
			debug(DEBUG_PROPAGATION, "%s: propagate %s (forward)\n",
			    rcvr->name, event->name);
			AG_LockLinkage();
			OBJECT_FOREACH_CHILD(chld, rcvr, ag_object) {
				PropagateEvent(rcvr, chld, ev);
			}
			AG_UnlockLinkage();
		}
		/* XXX AG_EVENT_ASYNC.. */
		if (ev->handler != NULL)
			ev->handler(&tmpev);
	}
out:
	AG_MutexUnlock(&rcvr->lock);
}

