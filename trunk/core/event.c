/*
 * Copyright (c) 2001-2012 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <string.h>
#include <stdarg.h>

#include <config/have_kqueue.h>
#include <config/ag_objdebug.h>

#ifdef HAVE_KQUEUE
# include <sys/types.h>
# include <sys/event.h>
# include <unistd.h>
# include <errno.h>
#endif

int agKqueue = -1;			/* For kqueue(2) based event loop */

/* Generate a unique event handler name. */
static void
GenEventName(AG_Event *ev, AG_Object *ob, const char *name)
{
	AG_Event *evOther;
	int i = ob->nevents;

	for (;;) {
		ev->name[0] = '_';
		StrlcpyUint(&ev->name[1], i++, sizeof(ev->name));

		TAILQ_FOREACH(evOther, &ob->events, events) {
			if (strcmp(evOther->name, ev->name) == 0)
				break;
		}
		if (evOther == NULL)
			break;
	}
}

/* Initialize a pointer argument. */
static __inline__ void
InitPointerArg(AG_Variable *V, void *p)
{
	V->name[0] = '\0';
	V->type = AG_VARIABLE_POINTER;
	V->fn.fnVoid = NULL;
	V->mutex = NULL;
	V->data.p = p;
}

static __inline__ void
InitEvent(AG_Event *ev, AG_Object *ob)
{
	ev->flags = 0;
	ev->argc = 1;
	ev->argc0 = 1;
	ev->handler = NULL;
	InitPointerArg(&ev->argv[0], ob);
}

/* Initialize an AG_Event structure. */
void
AG_EventInit(AG_Event *ev)
{
	InitEvent(ev, NULL);
}

/* Initialize an AG_Event structure with the specified arguments. */
void
AG_EventArgs(AG_Event *ev, const char *fmt, ...)
{
	InitEvent(ev, NULL);
	AG_EVENT_GET_ARGS(ev, fmt);
	ev->argc0 = ev->argc;
}

/* Set (or change) the event handler function for the named event. */
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
		if (name != NULL) {
			Strlcpy(ev->name, name, sizeof(ev->name));
		} else {
			GenEventName(ev, ob, name);
		}
		TAILQ_INSERT_TAIL(&ob->events, ev, events);
		ob->nevents++;
	} else {
		InitEvent(ev, ob);
	}
	InitPointerArg(&ev->argv[0], ob);
	ev->handler = fn;
	AG_EVENT_GET_ARGS(ev, fmt);
	ev->argc0 = ev->argc;

	AG_ObjectUnlock(ob);
	return (ev);
}

/* Append an event handler function for the named event. */
AG_Event *
AG_AddEvent(void *p, const char *name, AG_EventFn fn, const char *fmt, ...)
{
	AG_Object *ob = p;
	AG_Event *ev;

	AG_ObjectLock(ob);

	ev = Malloc(sizeof(AG_Event));
	InitEvent(ev, ob);
	if (name != NULL) {
		Strlcpy(ev->name, name, sizeof(ev->name));
	} else {
		GenEventName(ev, ob, name);
	}

	ev->handler = fn;
	AG_EVENT_GET_ARGS(ev, fmt);
	ev->argc0 = ev->argc;

	TAILQ_INSERT_TAIL(&ob->events, ev, events);
	ob->nevents++;

	AG_ObjectUnlock(ob);
	return (ev);
}

/* Remove the named event handler. */
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

/* Timeout callback for scheduled events. */
static Uint32
EventTimeout(AG_Timer *to, AG_Event *event)
{
	AG_Object *ob = AG_SELF();
	AG_Object *obSender = AG_PTR(1);
	char *eventName = AG_STRING(2);
	AG_Event *ev;

#ifdef AG_OBJDEBUG
	if (agDebugLvl >= 5)
		Debug(ob, "Event <%s> timeout (%u ticks)\n", eventName,
		    (Uint)to->ival);
#endif
	TAILQ_FOREACH(ev, &ob->events, events) {
		if (strcmp(eventName, ev->name) == 0)
			break;
	}
	if (ev == NULL) {
		return (0);
	}
	InitPointerArg(&ev->argv[ev->argc], obSender);

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
	ev->argc0 = ev->argc;
}

/*
 * Raise the specified event. Configured event handler routines may be
 * called immediately, but they may also get called from a separate
 * thread, or queued for later execution.
 *
 * The argument vector passed to the event handler function contains
 * the AG_SetEvent() arguments, and any arguments specified here are
 * appended to that list.
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
			InitPointerArg(&evNew->argv[evNew->argc], sndr);
			AG_ThreadCreate(&th, EventThread, evNew);
		} else
#endif /* AG_THREADS */
		{
			AG_Event tmpev;

			memcpy(&tmpev, ev, sizeof(AG_Event));
			AG_EVENT_GET_ARGS(&tmpev, fmt);
			InitPointerArg(&tmpev.argv[tmpev.argc], sndr);
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
 * Schedule the execution of the named event in the given number
 * of AG_Time(3) ticks.
 *
 * The argument vector passed to the event handler function contains
 * the AG_SetEvent() arguments, and any arguments specified here are
 * appended to that list.
 */
int
AG_SchedEvent(void *pSndr, void *pRcvr, Uint32 ticks, const char *evname,
    const char *fmt, ...)
{
	AG_Object *sndr = pSndr;
	AG_Object *rcvr = pRcvr;
	AG_Event *ev;
	AG_Timer *to;

	if ((to = TryMalloc(sizeof(AG_Timer))) == NULL)
		return (-1);

	AG_LockTiming();
	AG_ObjectLock(rcvr);
	
	if (AG_AddTimer(rcvr, to, ticks,
	    EventTimeout, "%p,%s", sndr, evname) == -1) {
		free(to);
		goto fail;
	}
	to->flags |= AG_TIMER_AUTO_FREE;
	ev = &to->fnEvent;
	AG_EventInit(ev);
	ev->argv[0].data.p = rcvr;
	AG_EVENT_GET_ARGS(ev, fmt);
	ev->argc0 = ev->argc;

	AG_UnlockTiming();
	AG_ObjectUnlock(rcvr);
	return (0);
fail:
	AG_UnlockTiming();
	AG_ObjectUnlock(rcvr);
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
			InitPointerArg(&evNew->argv[0], rcvr);
			InitPointerArg(&evNew->argv[evNew->argc], sndr);
			AG_ThreadCreate(&th, EventThread, evNew);
		} else
#endif /* AG_THREADS */
		{
			AG_Event tmpev;

			memcpy(&tmpev, event, sizeof(AG_Event));
			InitPointerArg(&tmpev.argv[0], rcvr);
			InitPointerArg(&tmpev.argv[tmpev.argc], sndr);

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

int
AG_InitEventSubsystem(void)
{
#ifdef HAVE_KQUEUE
	if ((agKqueue = kqueue()) == -1) {
		AG_SetError("kqueue: %s", AG_Strerror(errno));
		return (-1);
	}
#endif
	return (0);
}

void
AG_DestroyEventSubsystem(void)
{
#ifdef HAVE_KQUEUE
	close(agKqueue);
	agKqueue = -1;
#endif
}
