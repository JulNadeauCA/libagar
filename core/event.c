/*	$Csoft: event.c,v 1.222 2005/10/06 03:11:54 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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
#include <config/have_opengl.h>

/* #define OPENGL_INVERTED_Y */

#include <core/core.h>
#include <core/config.h>
#include <core/view.h>

#include <gui/window.h>
#include <gui/menu.h>
#ifdef DEBUG
#include <gui/label.h>
#include <gui/graph.h>
#endif
#ifdef EDITION
#include <core/objmgr.h>
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
static AG_Graph *agPerfGraph;
static AG_GraphItem *agPerfFPS, *agPerfEvnts, *agPerfIdle;
#endif	/* DEBUG */

int agIdleThresh = 20;				/* Idling threshold */
int agBgPopupMenu = 0;				/* Background popup menu */

struct ag_global_key {
	SDLKey keysym;
	SDLMod keymod;
	void (*fn)(void);
	SLIST_ENTRY(ag_global_key) gkeys;
};
static SLIST_HEAD(,ag_global_key) agGlobalKeys =
    SLIST_HEAD_INITIALIZER(&agGlobalKeys);

static void AG_EventRelay(void *, AG_Event *);
static void AG_EventProcess(SDL_Event *);
static void *AG_EventAsyncHandler(void *);

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
/* XXX remove this once the graph widget implements polling. */
static __inline__ void
update_perf_graph(void)
{
	static int einc = 0;

	AG_GraphPlot(agPerfFPS, agView->rCur);
	AG_GraphPlot(agPerfEvnts, agEventAvg * 30 / 10);
	AG_GraphPlot(agPerfIdle, agIdleAvg);
	AG_GraphScroll(agPerfGraph, 1);

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
init_perf_graph(void)
{
	AG_Label *label;

	agPerfWindow = AG_WindowNewNamed(0, "event-fps-counter");
	AG_WindowSetCaption(agPerfWindow, _("Performance counters"));
	AG_WindowSetPosition(agPerfWindow, AG_WINDOW_LOWER_CENTER, 0);

	label = AG_LabelNew(agPerfWindow, AG_LABEL_POLLED,
	    "%dms (nom %dms), %d evnt, %dms idle",
	    &agView->rCur, &agView->rNom, &agEventAvg,
	    &agIdleAvg);
	AG_LabelPrescale(label, "XXXms (nom XXXms), XX evnt, XXXms idle");

	agPerfGraph = AG_GraphNew(agPerfWindow, AG_GRAPH_LINES,
	    AG_GRAPH_ORIGIN);
	/* TODO use polling */

	agPerfFPS = AG_GraphAddItem(agPerfGraph, "refresh", 0, 160, 0, 99);
	agPerfEvnts = AG_GraphAddItem(agPerfGraph, "event", 0, 0, 180, 99);
	agPerfIdle = AG_GraphAddItem(agPerfGraph, "idle", 180, 180, 180, 99);
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
	AG_Gfx *gfx;
	Uint32 Tr1, Tr2 = 0;

#ifdef DEBUG
	init_perf_graph();
#endif
	Tr1 = SDL_GetTicks();
	for (;;) {
		Tr2 = SDL_GetTicks();
		if (Tr2-Tr1 >= agView->rNom) {
			AG_MutexLock(&agView->lock);
			agView->ndirty = 0;

#if defined(DEBUG) && defined(HAVE_OPENGL)
			if (agView->opengl)
				glClear(GL_COLOR_BUFFER_BIT|
				        GL_DEPTH_BUFFER_BIT);
#endif
			TAILQ_FOREACH(win, &agView->windows, windows) {
				if (!win->visible)
					continue;

				AG_MutexLock(&win->lock);
				AG_WidgetDraw(win);
				AG_MutexUnlock(&win->lock);

				if ((win->flags & AG_WINDOW_NOUPDATERECT) == 0)
					AG_UpdateRectQ(
					    AGWIDGET(win)->x, AGWIDGET(win)->y,
					    AGWIDGET(win)->w, AGWIDGET(win)->h);
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
			AG_MutexUnlock(&agView->lock);

			/* Recalibrate the effective refresh rate. */
			Tr1 = SDL_GetTicks();
			agView->rCur = agView->rNom - (Tr1-Tr2);
#ifdef DEBUG
			if (agPerfWindow->visible)
				update_perf_graph();
#endif
			if (agView->rCur < 1) {
				agView->rCur = 1;
			}
		} else if (SDL_PollEvent(&ev) != 0) {
			AG_EventProcess(&ev);
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
unminimize_window(AG_Event *event)
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

static void
AG_EventProcess(SDL_Event *ev)
{
	extern int agObjMgrExiting;
	extern int agEditMode;
	AG_Window *win;
	int rv;

	AG_MutexLock(&agView->lock);

	switch (ev->type) {
	case SDL_VIDEORESIZE:
		AG_ResizeDisplay(ev->resize.w, ev->resize.h);
		break;
	case SDL_VIDEOEXPOSE:
		AG_ViewVideoExpose();
		break;
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
			AG_MenuInit(me);
			mi = me->sel_item = AG_MenuAddItem(me, NULL);

			TAILQ_FOREACH_REVERSE(win, &agView->windows,
			    windows, ag_windowq) {
				if (strcmp(win->caption, "win-popup")
				    == 0) {
					continue;
				}
				AG_MenuAction(mi, win->caption,
				    OBJ_ICON,
				    unminimize_window, "%p", win);
			}
				
			AG_MouseGetState(&x, &y);
			AG_MenuExpand(me, mi, x+4, y+4);
		}
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
	case SDL_KEYDOWN:
		{
			struct ag_global_key *gk;

			SLIST_FOREACH(gk, &agGlobalKeys, gkeys) {
				if (gk->keysym == ev->key.keysym.sym &&
				    (gk->keymod == 0 ||
				     ev->key.keysym.mod & gk->keymod))
					gk->fn();
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
	case SDL_QUIT:
#ifdef EDITION
		if (!agObjMgrExiting && agEditMode &&
		    AG_ObjectChangedAll(agWorld)) {
			agObjMgrExiting = 1;
			AG_ObjMgrQuitDlg(agWorld);
			break;
		}
#endif
		/* FALLTHROUGH */
	case SDL_USEREVENT:
		AG_MutexUnlock(&agView->lock);
		agObjMgrExiting = 1;
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
event_timeout(void *p, Uint32 ival, void *arg)
{
	AG_Object *ob = p;
	AG_Event *ev = arg;
	va_list ap;
	
	debug(DEBUG_SCHED, "%s: timeout `%s' (ival=%u)\n", ob->name,
	    ev->name, ival);
	ev->flags &= ~(AG_EVENT_SCHEDULED);

	/* Propagate event to children. */
	if (ev->flags & AG_EVENT_PROPAGATE) {
		AG_Object *cobj;
			
		debug(DEBUG_PROPAGATION, "%s: propagate %s (timeout)\n",
		    ob->name, ev->name);
		AG_LockLinkage();
		AGOBJECT_FOREACH_CHILD(cobj, ob, ag_object) {
			AG_EventRelay(cobj, ev);
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
	AG_SetTimeout(&ev->timeout, event_timeout, ev, 0);
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
	AG_SetTimeout(&ev->timeout, event_timeout, ev, 0);
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

/* Forward an event to an object's descendents. */
static void
AG_EventRelay(void *p, AG_Event *ev)
{
	AG_Object *ob = p, *cob;

	AGOBJECT_FOREACH_CHILD(cob, ob, ag_object)
		AG_EventRelay(cob, ev);

	AG_ForwardEvent(ob, ev);
}

#ifdef THREADS
/* Invoke an event handler routine asynchronously. */
static void *
AG_EventAsyncHandler(void *p)
{
	AG_Event *eev = p;
	AG_Object *rcvr = eev->argv[0].p;

	/* Propagate event to children. */
	if (eev->flags & AG_EVENT_PROPAGATE) {
		AG_Object *cobj;

		debug(DEBUG_PROPAGATION, "%s: propagate %s (async)\n",
		    rcvr->name, eev->name);
		AG_LockLinkage();
		AGOBJECT_FOREACH_CHILD(cobj, rcvr, ag_object) {
			AG_EventRelay(cobj, eev);
		}
		AG_UnlockLinkage();
	}

	/* Invoke the event handler routine. */
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
int
AG_PostEvent(void *sp, void *rp, const char *evname, const char *fmt, ...)
{
	AG_Object *sndr = sp;
	AG_Object *rcvr = rp;
	AG_Event *ev;

	debug(DEBUG_EVENTS, "%s: %s -> %s\n", evname,
	    (sndr != NULL) ? sndr->name : "NULL", rcvr->name);

	AG_MutexLock(&rcvr->lock);
	TAILQ_FOREACH(ev, &rcvr->events, events) {
		if (strcmp(evname, ev->name) == 0)
			break;
	}
	if (ev == NULL)
		goto fail;
#ifdef THREADS
	if (ev->flags & AG_EVENT_ASYNC) {
		AG_Thread th;
		AG_Event *nev;

		/* TODO allocate from an per-object pool */
		nev = Malloc(sizeof(AG_Event), M_EVENT);
		memcpy(nev, ev, sizeof(AG_Event));
		AG_EVENT_GET_ARGS(nev, fmt);
		nev->argv[nev->argc].p = sndr;
		nev->argt[nev->argc] = AG_EVARG_POINTER;
		nev->argn[nev->argc] = "_sender";
		AG_ThreadCreate(&th, NULL, AG_EventAsyncHandler, nev);
	} else
#endif /* THREADS */
	{
		AG_Event tmpev;
		va_list ap;

		/* Construct the argument vector. */
		memcpy(&tmpev, ev, sizeof(AG_Event));
		AG_EVENT_GET_ARGS(&tmpev, fmt);
		tmpev.argv[tmpev.argc].p = sndr;
		tmpev.argt[tmpev.argc] = AG_EVARG_POINTER;
		tmpev.argn[tmpev.argc] = "_sender";

		/* Propagate event to children. */
		if (tmpev.flags & AG_EVENT_PROPAGATE) {
			AG_Object *cobj;
			
			debug(DEBUG_PROPAGATION, "%s: propagate %s (post)\n",
			    rcvr->name, evname);
			AG_LockLinkage();
			AGOBJECT_FOREACH_CHILD(cobj, rcvr, ag_object) {
				AG_EventRelay(cobj, &tmpev);
			}
			AG_UnlockLinkage();
		}

		/* Invoke the event handler function. */
		if (tmpev.handler != NULL)
			tmpev.handler(&tmpev);
	}

	AG_MutexUnlock(&rcvr->lock);
	return (1);
fail:
	AG_MutexUnlock(&rcvr->lock);
	return (0);
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
AG_ExecEvent(void *p, const char *evname)
{
	AG_Object *ob = p;
	AG_Event *ev;

	AG_MutexLock(&ob->lock);
	TAILQ_FOREACH(ev, &ob->events, events) {
		if (strcmp(ev->name, evname) == 0)
			break;
	}
	if (ev != NULL && ev->handler != NULL)
		ev->handler(ev);
}

/*
 * Forward an event, without modifying the original event structure, except
 * for the receiver pointer.
 */
/* XXX substitute the sender ptr? */
void
AG_ForwardEvent(void *rp, AG_Event *event)
{
	union evarg nargv[AG_EVENT_ARGS_MAX];
	AG_Object *rcvr = rp;
	AG_Event *ev;

	debug(DEBUG_EVENTS, "%s event to %s\n", event->name, rcvr->name);

	AG_MutexLock(&rcvr->lock);
	memcpy(nargv, event->argv, event->argc*sizeof(union evarg));
	nargv[0].p = rcvr;
	TAILQ_FOREACH(ev, &rcvr->events, events) {
		if (strcmp(event->name, ev->name) == 0)
			break;
	}
	if (ev == NULL)
		goto out;

	/* Propagate event to children. */
	if (ev->flags & AG_EVENT_PROPAGATE) {
		AG_Object *cobj;

		debug(DEBUG_PROPAGATION, "%s: propagate %s (forward)\n",
		    rcvr->name, event->name);
		AG_LockLinkage();
		AGOBJECT_FOREACH_CHILD(cobj, rcvr, ag_object) {
			AG_EventRelay(cobj, ev);
		}
		AG_UnlockLinkage();
	}
	/* XXX AG_EVENT_ASYNC.. */
	if (ev->handler != NULL)
		ev->handler(ev);
out:
	AG_MutexUnlock(&rcvr->lock);
}

