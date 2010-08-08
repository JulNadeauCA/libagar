/*
 * Copyright (c) 2001-2010 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>
#include <core/config.h>

#include <string.h>
#include <stdarg.h>

#include <config/ag_objdebug.h>

static void PropagateEvent(AG_Object *, AG_Object *, AG_Event *);

/* Execute a scheduled event invocation. */
static Uint32
SchedEventTimeout(void *p, Uint32 ival, void *arg)
{
	AG_Object *ob = p;
	AG_Event *ev = arg;

#ifdef AG_OBJDEBUG
	if (agDebugLvl >= 5)
		Debug(ob, "Event <%s> timeout (%u ticks)\n", ev->name,
		    (Uint)ival);
#endif
	ev->flags &= ~(AG_EVENT_SCHEDULED);

	/* Propagate event to children. */
	if (ev->flags & AG_EVENT_PROPAGATE) {
		AG_Object *child;
#ifdef AG_OBJDEBUG
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

static __inline__ void
SetEventName(AG_Event *ev, AG_Object *ob, const char *name)
{
	if (name == NULL) {
		if (ob != NULL) {
			ev->name[0] = '_';
			ev->name[1] = '_';
			ev->name[2] = '\0';
			StrlcpyUint(ev->name, ob->nevents, sizeof(ev->name));
		} else {
			Strlcpy(ev->name, "noname", sizeof(ev->name));
		}
	} else {
		Strlcpy(ev->name, name, sizeof(ev->name));
	}
}
	
static __inline__ void
InitPointer(AG_Variable *V, const char *name, AG_Object *ob)
{
	V->type = AG_VARIABLE_POINTER;
	V->fn.fnVoid = NULL;
	V->mutex = NULL;
	V->data.p = ob;
	Strlcpy(V->name, name, sizeof(V->name));
}

static __inline__ void
InitEvent(AG_Event *ev, AG_Object *ob)
{
	ev->flags = 0;
	ev->argc = 1;
	ev->argc0 = 1;
	ev->handler = NULL;

	InitPointer(&ev->argv[0], "_self", ob);
	AG_SetTimeout(&ev->timeout, SchedEventTimeout, ev, 0);
}

void
AG_EventInit(AG_Event *ev)
{
	InitEvent(ev, NULL);
}

/* Construct an Event structure from the given arguments only. */
void
AG_EventArgs(AG_Event *ev, const char *fmt, ...)
{
	InitEvent(ev, NULL);
	AG_EVENT_GET_ARGS(ev, fmt);
}

/* Set or update the handler function for the given event. */
AG_Event *
AG_SetEvent(void *p, const char *name, AG_EventFn fn, const char *fmt, ...)
{
	AG_Object *ob = p;
	AG_Event *ev;

	AG_ObjectLock(ob);

	if (name != NULL) {
		TAILQ_FOREACH(ev, &ob->events, events)
			if (strcmp(ev->name, name) == 0)
				break;
	} else {
		ev = NULL;
	}
	if (ev == NULL) {
		ev = Malloc(sizeof(AG_Event));
		InitEvent(ev, ob);
		SetEventName(ev, ob, name);
		TAILQ_INSERT_TAIL(&ob->events, ev, events);
		ob->nevents++;
	} else {
		InitEvent(ev, ob);
	}
	InitPointer(&ev->argv[0], "_self", ob);
	ev->handler = fn;
	AG_EVENT_GET_ARGS(ev, fmt);
	ev->argc0 = ev->argc;

	AG_ObjectUnlock(ob);
	return (ev);
}

/* Configure an additional handler function for the specified event. */
AG_Event *
AG_AddEvent(void *p, const char *name, AG_EventFn fn, const char *fmt, ...)
{
	AG_Object *ob = p;
	AG_Event *ev;

	AG_ObjectLock(ob);

	ev = Malloc(sizeof(AG_Event));
	InitEvent(ev, ob);
	SetEventName(ev, ob, name);

	ev->handler = fn;
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

/* Return the Event structure for the named event. */
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

#ifdef AG_THREADS
/* Invoke an event handler routine asynchronously. */
static void *
EventThread(void *p)
{
	AG_Event *eev = p;
	AG_Object *rcvr = eev->argv[0].data.p;
	AG_Object *chld;

	if (eev->flags & AG_EVENT_PROPAGATE) {
#ifdef AG_OBJDEBUG
		if (agDebugLvl >= 5)
			Debug(rcvr, "Propagate <%s> (async)\n", eev->name);
#endif
		AG_LockVFS(rcvr);
		OBJECT_FOREACH_CHILD(chld, rcvr, ag_object) {
			PropagateEvent(rcvr, chld, eev);
		}
		AG_UnlockVFS(rcvr);
	}
#ifdef AG_OBJDEBUG
	if (agDebugLvl >= 5)
		Debug(rcvr, "BEGIN event thread for <%s>\n", eev->name);
#endif
	if (eev->handler != NULL) {
		eev->handler(eev);
	}
#ifdef AG_OBJDEBUG
	if (agDebugLvl >= 5)
		Debug(rcvr, "CLOSE event thread for <%s>\n", eev->name);
#endif
	Free(eev);
	return (NULL);
}
#endif /* AG_THREADS */

void
AG_InitEventQ(AG_EventQ *eq)
{
	eq->nEvents = 0;
	eq->events = NULL;
}

void
AG_FreeEventQ(AG_EventQ *eq)
{
	Free(eq->events);
	eq->nEvents = 0;
	eq->events = NULL;
}

/* Add a new entry to an event queue. */
void
AG_QueueEvent(AG_EventQ *eq, const char *evname, const char *fmt, ...)
{
	AG_Event *ev;

	eq->events = Realloc(eq->events, (eq->nEvents+1)*sizeof(AG_Event));
	ev = &eq->events[eq->nEvents++];
	InitEvent(ev, NULL);
	AG_EVENT_GET_ARGS(ev, fmt);
}

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

#ifdef AG_OBJDEBUG
	if (agDebugLvl >= 5)
		Debug(rcvr, "Event <%s> posted from %s\n", evname,
		    sndr?sndr->name:"NULL");
#endif
	AG_ObjectLock(rcvr);
	TAILQ_FOREACH(ev, &rcvr->events, events) {
		if (strcmp(evname, ev->name) != 0)
			continue;
#ifdef AG_THREADS
		if (ev->flags & AG_EVENT_ASYNC) {
			AG_Thread th;
			AG_Event *evNew;

			/* TODO allocate from an per-object pool */
			evNew = Malloc(sizeof(AG_Event));
			memcpy(evNew, ev, sizeof(AG_Event));
			AG_EVENT_GET_ARGS(evNew, fmt);
			InitPointer(&evNew->argv[evNew->argc], "_sender", sndr);
			AG_ThreadCreate(&th, EventThread, evNew);
		} else
#endif /* AG_THREADS */
		{
			AG_Event tmpev;

			memcpy(&tmpev, ev, sizeof(AG_Event));
			AG_EVENT_GET_ARGS(&tmpev, fmt);
			InitPointer(&tmpev.argv[tmpev.argc], "_sender", sndr);
			if (tmpev.flags & AG_EVENT_PROPAGATE) {
#ifdef AG_OBJDEBUG
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

#ifdef AG_OBJDEBUG
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
#ifdef AG_OBJDEBUG
		if (agDebugLvl >= 5)
			Debug(rcvr, "Reschedule <%s>\n", evname);
#endif
		AG_DelTimeout(rcvr, &ev->timeout);
	}
	ev->argc = ev->argc0;
	AG_EVENT_GET_ARGS(ev, fmt);
	InitPointer(&ev->argv[ev->argc], "_sender", sndr);
	ev->flags |= AG_EVENT_SCHEDULED;
	AG_ScheduleTimeout(rcvr, &ev->timeout, ticks);
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

#ifdef AG_OBJDEBUG
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
	AG_ScheduleTimeout(ob, &ev->timeout, ticks);

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
#ifdef AG_OBJDEBUG
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

#ifdef AG_OBJDEBUG
	if (agDebugLvl >= 5)
		Debug(rcvr, "Event <%s> forwarded from %s\n", event->name,
		    sndr?sndr->name:"NULL");
#endif
	AG_ObjectLock(rcvr);
	TAILQ_FOREACH(ev, &rcvr->events, events) {
		if (strcmp(event->name, ev->name) != 0)
			continue;
#ifdef AG_THREADS
		if (ev->flags & AG_EVENT_ASYNC) {
			AG_Thread th;
			AG_Event *evNew;

			/* TODO allocate from an per-object pool */
			evNew = Malloc(sizeof(AG_Event));
			memcpy(evNew, ev, sizeof(AG_Event));
			InitPointer(&evNew->argv[0], "_self", rcvr);
			InitPointer(&evNew->argv[evNew->argc], "_sender", sndr);
			AG_ThreadCreate(&th, EventThread, evNew);
		} else
#endif /* AG_THREADS */
		{
			AG_Event tmpev;

			memcpy(&tmpev, event, sizeof(AG_Event));
			InitPointer(&tmpev.argv[0], "_self", rcvr);
			InitPointer(&tmpev.argv[tmpev.argc], "_sender", sndr);

			if (ev->flags & AG_EVENT_PROPAGATE) {
#ifdef AG_OBJDEBUG
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
	}
	AG_ObjectUnlock(rcvr);
}

