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

#include <core/core.h>
#include <core/config.h>

#include <string.h>
#include <stdarg.h>

#ifdef DEBUG
#define DEBUG_EVENTS		0x01
#define DEBUG_ASYNC		0x02
#define DEBUG_PROPAGATION	0x04
#define DEBUG_SCHED		0x08
#define	agDebugLvl		agEventDebugLvl
int	agEventDebugLvl = 0;
#endif

static void PropagateEvent(AG_Object *, AG_Object *, AG_Event *);

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

/*
 * Execute a scheduled event invocation.
 * The object and timeouteq are assumed to be locked.
 */
static Uint32
SchedEventTimeout(void *p, Uint32 ival, void *arg)
{
	AG_Object *ob = p;
	AG_Event *ev = arg;
	
	debug(DEBUG_SCHED, "%s: timeout `%s' (ival=%u)\n", ob->name,
	    ev->name, (Uint)ival);
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

AG_Event *
AG_SetEvent(void *p, const char *name, AG_EventFn fn, const char *fmt, ...)
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
	    evname, (Uint)ticks);

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
	    (Uint)ticks);

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

