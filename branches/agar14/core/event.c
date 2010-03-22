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

#ifdef DEBUG
	if (agDebugLvl >= 5)
		Debug(ob, "Event <%s> timeout (%u ticks)\n", ev->name,
		    (Uint)ival);
#endif
	ev->flags &= ~(AG_EVENT_SCHEDULED);

	/* Propagate event to children. */
	if (ev->flags & AG_EVENT_PROPAGATE) {
		AG_Object *child;
#ifdef DEBUG
		if (agDebugLvl >= 5)
			Debug(ob, "Propagate <%s> (timeout)\n", ev->name);
#endif
		AG_LockVFS(ob);
		OBJECT_FOREACH_CHILD(child, ob, ag_object) {
			PropagateEvent(ob, child, ev);
		}
		AG_UnlockVFS(ob);
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

	AG_ObjectLock(ob);
	if (name != NULL) {
		TAILQ_FOREACH(ev, &ob->events, events) {
			if (strcmp(ev->name, name) == 0)
				break;
		}
	} else {
		ev = NULL;
	}
	if (ev == NULL) {
		ev = Malloc(sizeof(AG_Event));
		if (name != NULL) {
			Strlcpy(ev->name, name, sizeof(ev->name));
		} else {
			/* XXX use something faster */
			Snprintf(ev->name, sizeof(ev->name), "__%u",
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
	AG_ObjectUnlock(ob);
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

	AG_ObjectLock(ob);

	ev = Malloc(sizeof(AG_Event));
	if (name != NULL) {
		Strlcpy(ev->name, name, sizeof(ev->name));
	} else {
		Snprintf(ev->name, sizeof(ev->name), "__%u", ob->nevents);
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
	AG_ObjectUnlock(ob);
	return (ev);
}

/* Remove the named event handler and cancel any scheduled execution. */
void
AG_UnsetEvent(void *p, const char *name)
{
	AG_Object *ob = p;
	AG_Event *ev;

	AG_ObjectLock(ob);
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
	Free(ev);
out:
	AG_ObjectUnlock(ob);
}

AG_Event *
AG_FindEventHandler(void *p, const char *name)
{
	AG_Object *ob = p;
	AG_Event *ev;
	
	AG_ObjectLock(ob);
	TAILQ_FOREACH(ev, &ob->events, events) {
		if (strcmp(name, ev->name) == 0)
			break;
	}
	AG_ObjectUnlock(ob);
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
#ifdef DEBUG
		if (agDebugLvl >= 5)
			Debug(rcvr, "Propagate <%s> (async)\n", eev->name);
#endif
		AG_LockVFS(rcvr);
		OBJECT_FOREACH_CHILD(chld, rcvr, ag_object) {
			PropagateEvent(rcvr, chld, eev);
		}
		AG_UnlockVFS(rcvr);
	}
#ifdef DEBUG
	if (agDebugLvl >= 5)
		Debug(rcvr, "BEGIN event thread for <%s>\n", eev->name);
#endif
	if (eev->handler != NULL) {
		eev->handler(eev);
	}
#ifdef DEBUG
	if (agDebugLvl >= 5)
		Debug(rcvr, "CLOSE event thread for <%s>\n", eev->name);
#endif
	Free(eev);
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

#ifdef DEBUG
	if (agDebugLvl >= 5)
		Debug(rcvr, "Event <%s> posted from %s\n", evname,
		    sndr?sndr->name:"NULL");
#endif
	AG_ObjectLock(rcvr);
	TAILQ_FOREACH(ev, &rcvr->events, events) {
		if (strcmp(evname, ev->name) != 0)
			continue;
#ifdef THREADS
		if (ev->flags & AG_EVENT_ASYNC) {
			AG_Thread th;
			AG_Event *evNew;

			/* TODO allocate from an per-object pool */
			evNew = Malloc(sizeof(AG_Event));
			memcpy(evNew, ev, sizeof(AG_Event));
			AG_EVENT_GET_ARGS(evNew, fmt);
			evNew->argv[evNew->argc].p = sndr;
			evNew->argt[evNew->argc] = AG_EVARG_POINTER;
			evNew->argn[evNew->argc] = "_sender";
			AG_ThreadCreate(&th, EventThread, evNew);
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
#ifdef DEBUG
				if (agDebugLvl >= 5)
					Debug(rcvr, "Propagate <%s> (post)\n",
					    evname);
#endif
				AG_LockVFS(rcvr);
				OBJECT_FOREACH_CHILD(chld, rcvr, ag_object) {
					PropagateEvent(rcvr, chld, &tmpev);
				}
				AG_UnlockVFS(rcvr);
			}
			if (tmpev.handler != NULL)
				tmpev.handler(&tmpev);
		}
	}
	AG_ObjectUnlock(rcvr);
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

#ifdef DEBUG
	if (agDebugLvl >= 5)
		Debug(rcvr, "Schedule <%s> in %u ticks\n", evname, (Uint)ticks);
#endif
	AG_LockTiming();
	AG_ObjectLock(rcvr);
	TAILQ_FOREACH(ev, &rcvr->events, events) {
		if (strcmp(evname, ev->name) == 0)
			break;
	}
	if (ev == NULL) {
		goto fail;
	}
	if (ev->flags & AG_EVENT_SCHEDULED) {
#ifdef DEBUG
		if (agDebugLvl >= 5)
			Debug(rcvr, "Reschedule <%s>\n", evname);
#endif
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
	AG_ObjectUnlock(rcvr);
	return (0);
fail:
	AG_UnlockTiming();
	AG_ObjectUnlock(rcvr);
	return (-1);
}

int
AG_ReschedEvent(void *p, const char *evname, Uint32 ticks)
{
	AG_Object *ob = p;
	AG_Event *ev;

#ifdef DEBUG
	if (agDebugLvl >= 5)
		Debug(ob, "Reschedule <%s> in %u ticks\n", evname, (Uint)ticks);
#endif
	AG_LockTiming();
	AG_ObjectLock(ob);
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

	AG_ObjectUnlock(ob);
	AG_UnlockTiming();
	return (0);
fail:
	AG_ObjectUnlock(ob);
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
	AG_ObjectLock(ob);
	TAILQ_FOREACH(ev, &ob->events, events) {
		if (strcmp(ev->name, evname) == 0)
			break;
	}
	if (ev == NULL) {
		goto fail;
	}
	if (ev->flags & AG_EVENT_SCHEDULED) {
#ifdef DEBUG
		if (agDebugLvl >= 5)
			Debug(ob, "Cancelled timeout <%s> (cancel)\n", evname);
#endif
		AG_DelTimeout(ob, &ev->timeout);
		rv++;
		ev->flags &= ~(AG_EVENT_SCHEDULED);
	}
	/* XXX concurrent */
	AG_ObjectUnlock(ob);
	AG_UnlockTiming();
	return (rv);
fail:
	AG_ObjectUnlock(ob);
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

#ifdef DEBUG
	if (agDebugLvl >= 5)
		Debug(rcvr, "Event <%s> forwarded from %s\n", event->name,
		    sndr?sndr->name:"NULL");
#endif
	AG_ObjectLock(rcvr);
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
		evNew = Malloc(sizeof(AG_Event));
		memcpy(evNew, ev, sizeof(AG_Event));
		evNew->argv[0].p = rcvr;
		evNew->argv[evNew->argc].p = sndr;
		evNew->argt[evNew->argc] = AG_EVARG_POINTER;
		evNew->argn[evNew->argc] = "_sender";
		AG_ThreadCreate(&th, EventThread, evNew);
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
#ifdef DEBUG
			if (agDebugLvl >= 5)
				Debug(rcvr, "Propagate <%s> (forward)\n",
				    event->name);
#endif
			AG_LockVFS(rcvr);
			OBJECT_FOREACH_CHILD(chld, rcvr, ag_object) {
				PropagateEvent(rcvr, chld, ev);
			}
			AG_UnlockVFS(rcvr);
		}
		/* XXX AG_EVENT_ASYNC.. */
		if (ev->handler != NULL)
			ev->handler(&tmpev);
	}
out:
	AG_ObjectUnlock(rcvr);
}
