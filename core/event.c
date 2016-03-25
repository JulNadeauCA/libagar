/*
 * Copyright (c) 2001-2015 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Implementation of AG_Object events / virtual functions, as well as
 * the generic AG_EventLoop(3) interface.
 */

#include <agar/core/core.h>

#include <string.h>
#include <stdarg.h>

#include <agar/config/have_kqueue.h>
#include <agar/config/have_timerfd.h>
#include <agar/config/have_select.h>
#include <agar/config/ag_debug_core.h>

#if defined(HAVE_KQUEUE)
# ifdef __NetBSD__
#   define _NETBSD_SOURCE
# endif
# include <sys/types.h>
# include <sys/event.h>
# include <unistd.h>
# include <errno.h>
#endif
#if defined(HAVE_TIMERFD)
# include <sys/timerfd.h>
# include <errno.h>
#endif
#if defined(HAVE_SELECT)
# include <sys/types.h>
# include <sys/time.h>
# include <sys/select.h>
# include <unistd.h>
# include <errno.h>
#endif

AG_EventSource *agEventSource = NULL;	/* Event source (thread-local) */
#ifdef AG_THREADS
AG_ThreadKey    agEventSourceKey;
#endif

#ifdef HAVE_KQUEUE
#define EVBUFSIZE 2
typedef struct ag_event_source_kqueue {
	struct ag_event_source _inherit;
	int fd;					/* kqueue() fd */
	struct kevent *changes;			/* Queued changes */
	Uint          nChanges;
	Uint        maxChanges;
	struct kevent events[EVBUFSIZE];	/* Input event buffer */
} AG_EventSourceKQUEUE;
#endif /* HAVE_KQUEUE */

/* #define DEBUG_TIMERS */

#ifdef __NetBSD__
# define AG_EV_SET(kevp,a,b,c,d,e,f) EV_SET((kevp),(uintptr_t)(a),(b),(c),(d),(e),(intptr_t)(f))
#else
# define AG_EV_SET(kevp,a,b,c,d,e,f) EV_SET((kevp),(a),(b),(c),(d),(e),(f))
#endif

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
	ev->fn.fnVoid = NULL;
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

/*
 * Configure an event handler routine for a given event.
 * If a handler routine already exists, replace it.
 */
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
			ev->name[0] = '\0';
		}
		TAILQ_INSERT_TAIL(&ob->events, ev, events);
	} else {
		ev->argc = 1;
		ev->argc0 = 1;
	}
	InitPointerArg(&ev->argv[0], ob);
	ev->fn.fnVoid = fn;
	AG_EVENT_GET_ARGS(ev, fmt);
	ev->argc0 = ev->argc;

	AG_ObjectUnlock(ob);
	return (ev);
}

/*
 * Configure an event handler routine for a given event.
 * If a handler routine already exists, don't replace it.
 */
AG_Event *
AG_AddEvent(void *p, const char *name, AG_EventFn fn, const char *fmt, ...)
{
	AG_Object *ob = p;
	AG_Event *ev, *evOther;

	AG_ObjectLock(ob);

	ev = Malloc(sizeof(AG_Event));
	InitEvent(ev, ob);

	if (name != NULL) {
		TAILQ_FOREACH(evOther, &ob->events, events) {
			if (strcmp(evOther->name, name) == 0)
				break;
		}
		if (evOther != NULL) {
			ev->flags = evOther->flags;
		}
		Strlcpy(ev->name, name, sizeof(ev->name));
	} else {
		ev->name[0] = '\0';
	}

	ev->fn.fnVoid = fn;
	AG_EVENT_GET_ARGS(ev, fmt);
	ev->argc0 = ev->argc;

	TAILQ_INSERT_TAIL(&ob->events, ev, events);
	AG_ObjectUnlock(ob);
	return (ev);
}

/*
 * Anonymous Function Constructors
 */
#undef  AG_SET_TYPED_FN
#define AG_SET_TYPED_FN(memb)				\
	AG_Object *ob = p;				\
	AG_Event *ev;					\
							\
	ev = Malloc(sizeof(AG_Event));			\
	InitEvent(ev, ob);				\
	ev->name[0] = '\0';				\
	ev->fn.memb = fn;				\
	InitPointerArg(&ev->argv[0], ob);		\
	AG_EVENT_GET_ARGS(ev, fmt);			\
							\
	AG_ObjectLock(ob);				\
	TAILQ_INSERT_TAIL(&ob->events, ev, events);	\
	ev->argc0 = ev->argc;				\
	AG_ObjectUnlock(ob);				\
	return (AG_Function *)ev

AG_Function *AG_SetVoidFn(void *p, AG_VoidFn fn, const char *fmt, ...)		{ AG_SET_TYPED_FN(fnVoid);	}
AG_Function *AG_SetIntFn(void *p, AG_IntFn fn, const char *fmt, ...)		{ AG_SET_TYPED_FN(fnInt);	}
AG_Function *AG_SetUint8Fn(void *p, AG_Uint8Fn fn, const char *fmt, ...)	{ AG_SET_TYPED_FN(fnUint8);	}
AG_Function *AG_SetSint8Fn(void *p, AG_Sint8Fn fn, const char *fmt, ...)	{ AG_SET_TYPED_FN(fnSint8);	}
AG_Function *AG_SetUint16Fn(void *p, AG_Uint16Fn fn, const char *fmt, ...)	{ AG_SET_TYPED_FN(fnUint16);	}
AG_Function *AG_SetSint16Fn(void *p, AG_Sint16Fn fn, const char *fmt, ...)	{ AG_SET_TYPED_FN(fnSint16);	}
AG_Function *AG_SetUint32Fn(void *p, AG_Uint32Fn fn, const char *fmt, ...)	{ AG_SET_TYPED_FN(fnUint32);	}
AG_Function *AG_SetSint32Fn(void *p, AG_Sint32Fn fn, const char *fmt, ...)	{ AG_SET_TYPED_FN(fnSint32);	}
#ifdef AG_HAVE_64BIT
AG_Function *AG_SetUint64Fn(void *p, AG_Uint64Fn fn, const char *fmt, ...)	{ AG_SET_TYPED_FN(fnUint64);	}
AG_Function *AG_SetSint64Fn(void *p, AG_Sint64Fn fn, const char *fmt, ...)	{ AG_SET_TYPED_FN(fnSint64);	}
#endif
AG_Function *AG_SetFloatFn(void *p, AG_FloatFn fn, const char *fmt, ...)	{ AG_SET_TYPED_FN(fnFloat);	}
AG_Function *AG_SetDoubleFn(void *p, AG_DoubleFn fn, const char *fmt, ...)	{ AG_SET_TYPED_FN(fnDouble);	}
#ifdef AG_HAVE_LONG_DOUBLE
AG_Function *AG_SetLongDoubleFn(void *p, AG_LongDoubleFn fn, const char *fmt, ...)	{ AG_SET_TYPED_FN(fnLongDouble); }
#endif
AG_Function *AG_SetStringFn(void *p, AG_StringFn fn, const char *fmt, ...)		{ AG_SET_TYPED_FN(fnString);	}
AG_Function *AG_SetPointerFn(void *p, AG_PointerFn fn, const char *fmt, ...)		{ AG_SET_TYPED_FN(fnPointer);	}
AG_Function *AG_SetConstPointerFn(void *p, AG_ConstPointerFn fn, const char *fmt, ...)	{ AG_SET_TYPED_FN(fnConstPointer); }
AG_Function *AG_SetTextFn(void *p, AG_TextFn fn, const char *fmt, ...)			{ AG_SET_TYPED_FN(fnText);	}

#undef AG_SET_TYPED_FN

/* Delete an event handler by name. */
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
	free(ev);
out:
	AG_ObjectUnlock(ob);
}

/* Look up an AG_Event by name. */
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

#ifdef AG_DEBUG_CORE
	if (agDebugLvl >= 2)
		Debug(ob, "Event <%s> timeout (%u ticks)\n", eventName, (Uint)to->ival);
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
#ifdef AG_DEBUG_CORE
		if (agDebugLvl >= 2)
			Debug(ob, "Propagate <%s> (timeout)\n", ev->name);
#endif
		AG_LockVFS(ob);
		OBJECT_FOREACH_CHILD(child, ob, ag_object) {
			PropagateEvent(ob, child, ev);
		}
		AG_UnlockVFS(ob);
	}

	/* Invoke the event handler routine. */
	if (ev->fn.fnVoid != NULL) {
		ev->fn.fnVoid(ev);
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
#ifdef AG_DEBUG_CORE
		if (agDebugLvl >= 2)
			Debug(rcvr, "Propagate <%s> (async)\n", eev->name);
#endif
		AG_LockVFS(rcvr);
		OBJECT_FOREACH_CHILD(chld, rcvr, ag_object) {
			PropagateEvent(rcvr, chld, eev);
		}
		AG_UnlockVFS(rcvr);
	}
#ifdef AG_DEBUG_CORE
	if (agDebugLvl >= 2)
		Debug(rcvr, "BEGIN event thread for <%s>\n", eev->name);
#endif
	if (eev->fn.fnVoid != NULL) {
		eev->fn.fnVoid(eev);
	}
#ifdef AG_DEBUG_CORE
	if (agDebugLvl >= 2)
		Debug(rcvr, "CLOSE event thread for <%s>\n", eev->name);
#endif
	free(eev);
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
	int propagated = 0;

#ifdef AG_DEBUG_CORE
	if (agDebugLvl >= 2)
		Debug(rcvr, "Event <%s> posted from %s\n", evname, sndr ? sndr->name : "NULL");
#endif
	AG_ObjectLock(rcvr);
	TAILQ_FOREACH(ev, &rcvr->events, events) {
		if (strcmp(evname, ev->name) != 0)
			continue;
#ifdef AG_THREADS
		if (ev->flags & AG_EVENT_ASYNC) {
			AG_Thread th;
			AG_Event *evNew;

			evNew = Malloc(sizeof(AG_Event));
			memcpy(evNew, ev, sizeof(AG_Event));
			AG_EVENT_GET_ARGS(evNew, fmt);
			InitPointerArg(&evNew->argv[evNew->argc], sndr);
			if (evNew->flags & AG_EVENT_PROPAGATE) { propagated = 1; }
			if (propagated) {
				evNew->flags &= ~(AG_EVENT_PROPAGATE);
			}
			AG_ThreadCreate(&th, EventThread, evNew);
		} else
#endif /* AG_THREADS */
		{
			AG_Event tmpev;

			memcpy(&tmpev, ev, sizeof(AG_Event));
			AG_EVENT_GET_ARGS(&tmpev, fmt);
			InitPointerArg(&tmpev.argv[tmpev.argc], sndr);
			if ((tmpev.flags & AG_EVENT_PROPAGATE) && !propagated) {
#ifdef AG_DEBUG_CORE
				if (agDebugLvl >= 2)
					Debug(rcvr, "Propagate <%s>\n", evname);
#endif
				AG_LockVFS(rcvr);
				OBJECT_FOREACH_CHILD(chld, rcvr, ag_object) {
					PropagateEvent(rcvr, chld, &tmpev);
				}
				AG_UnlockVFS(rcvr);
				propagated = 1;
			}
			if (tmpev.fn.fnVoid != NULL)
				tmpev.fn.fnVoid(&tmpev);
		}
	}
	AG_ObjectUnlock(rcvr);
}

/*
 * Variant of AG_PostEvent() which accepts an AG_Event argument instead
 * of looking up the event handler by name.
 */
void
AG_PostEventByPtr(void *sp, void *rp, AG_Event *ev, const char *fmt, ...)
{
	AG_Object *sndr = sp;
	AG_Object *rcvr = rp;
	AG_Object *chld;
	int propagated = 0;

#ifdef AG_DEBUG_CORE
	if (agDebugLvl >= 2)
		Debug(rcvr, "Event %p posted from %s\n", ev, sndr ? sndr->name : "NULL");
#endif
	AG_ObjectLock(rcvr);
#ifdef AG_THREADS
	if (ev->flags & AG_EVENT_ASYNC) {
		AG_Thread th;
		AG_Event *evNew;

		evNew = Malloc(sizeof(AG_Event));
		memcpy(evNew, ev, sizeof(AG_Event));
		AG_EVENT_GET_ARGS(evNew, fmt);
		InitPointerArg(&evNew->argv[evNew->argc], sndr);
		if (evNew->flags & AG_EVENT_PROPAGATE) { propagated = 1; }
		if (propagated) {
			evNew->flags &= ~(AG_EVENT_PROPAGATE);
		}
		AG_ThreadCreate(&th, EventThread, evNew);
	} else
#endif /* AG_THREADS */
	{
		AG_Event evTmp;

		memcpy(&evTmp, ev, sizeof(AG_Event));
		AG_EVENT_GET_ARGS(&evTmp, fmt);
		InitPointerArg(&evTmp.argv[evTmp.argc], sndr);
		if ((evTmp.flags & AG_EVENT_PROPAGATE) && !propagated) {
#ifdef AG_DEBUG_CORE
			if (agDebugLvl >= 2)
				Debug(rcvr, "Propagate event %p (post)\n", ev);
#endif
			AG_LockVFS(rcvr);
			OBJECT_FOREACH_CHILD(chld, rcvr, ag_object) {
				PropagateEvent(rcvr, chld, &evTmp);
			}
			AG_UnlockVFS(rcvr);
			propagated = 1;
		}
		if (evTmp.fn.fnVoid != NULL)
			evTmp.fn.fnVoid(&evTmp);
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

	if ((to = TryMalloc(sizeof(AG_Timer))) == NULL) {
		return (-1);
	}
	AG_InitTimer(to, evname, AG_TIMER_AUTO_FREE);

	AG_LockTiming();
	AG_ObjectLock(rcvr);
	
	if (AG_AddTimer(rcvr, to, ticks,
	    EventTimeout, "%p,%s", sndr, evname) == -1) {
		free(to);
		goto fail;
	}
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

#ifdef AG_DEBUG_CORE
	if (agDebugLvl >= 2)
		Debug(rcvr, "Event <%s> forwarded from %s\n", event->name, sndr ? sndr->name : "NULL");
#endif
	AG_ObjectLock(rcvr);
	TAILQ_FOREACH(ev, &rcvr->events, events) {
		if (strcmp(event->name, ev->name) != 0)
			continue;
#ifdef AG_THREADS
		if (ev->flags & AG_EVENT_ASYNC) {
			AG_Thread th;
			AG_Event *evNew;

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
#ifdef AG_DEBUG_CORE
				if (agDebugLvl >= 2)
					Debug(rcvr, "Propagate <%s> (forward)\n", event->name);
#endif
				AG_LockVFS(rcvr);
				OBJECT_FOREACH_CHILD(chld, rcvr, ag_object) {
					PropagateEvent(rcvr, chld, ev);
				}
				AG_UnlockVFS(rcvr);
			}
			/* XXX AG_EVENT_ASYNC.. */
			if (ev->fn.fnVoid != NULL)
				ev->fn.fnVoid(&tmpev);
		}
	}
	AG_ObjectUnlock(rcvr);
}

#ifdef HAVE_KQUEUE
static __inline__ int
GrowKqChangelist(AG_EventSourceKQUEUE *kq, Uint n)
{
	struct kevent *changesNew;

	if (n <= kq->maxChanges) {
		return (0);
	}
	if ((changesNew = TryRealloc(kq->changes, n*sizeof(struct kevent)))
	    == NULL) {
		return (-1);
	}
	kq->changes = changesNew;
	kq->maxChanges = n;
	return (0);
}
#endif /* HAVE_KQUEUE */

/* Create a new event source. */
static AG_EventSource *
CreateEventSource(void)
{
#ifdef HAVE_KQUEUE
	AG_EventSourceKQUEUE *kq = TryMalloc(sizeof(AG_EventSourceKQUEUE));
	AG_EventSource *src = (AG_EventSource *)kq;
#else
	AG_EventSource *src = TryMalloc(sizeof(AG_EventSource));
#endif
	if (src == NULL) {
		return (NULL);
	}
	src->flags = 0;
	src->addTimerFn = NULL;
	src->delTimerFn = NULL;
	src->breakReq = 0;
	src->returnCode = 0;
	TAILQ_INIT(&src->prologues);
	TAILQ_INIT(&src->epilogues);
	TAILQ_INIT(&src->spinners);
	TAILQ_INIT(&src->sinks);
	memset(src->caps, 0, sizeof(src->caps));

#if defined(HAVE_KQUEUE)
	if ((kq->fd = kqueue()) == -1) {
		AG_SetError("kqueue: %s", AG_Strerror(errno));
		return (NULL);
	}
	kq->changes = NULL;
	kq->nChanges = 0;
	kq->maxChanges = 0;
	memset(kq->events, 0, EVBUFSIZE*sizeof(struct kevent));
	src->sinkFn = AG_EventSinkKQUEUE;
	src->addTimerFn = AG_AddTimerKQUEUE;
	src->delTimerFn = AG_DelTimerKQUEUE;
	src->caps[AG_SINK_TIMER] = 1;		/* Provides timers internally */
	src->caps[AG_SINK_READ] = 1;
	src->caps[AG_SINK_WRITE] = 1;
	src->caps[AG_SINK_FSEVENT] = 1;
	src->caps[AG_SINK_PROCEVENT] = 1;
	GrowKqChangelist(kq, 64);		/* Preallocate */
#elif defined(HAVE_TIMERFD)
	src->sinkFn = AG_EventSinkTIMERFD;
	src->addTimerFn = AG_AddTimerTIMERFD;
	src->delTimerFn = AG_DelTimerTIMERFD;
	src->caps[AG_SINK_TIMER] = 1;		/* Provides timers internally */
	src->caps[AG_SINK_READ] = 1;
	src->caps[AG_SINK_WRITE] = 1;
#elif defined(HAVE_SELECT) && !defined(AG_THREADS)
	src->sinkFn = AG_EventSinkTIMEDSELECT;
	src->caps[AG_SINK_READ] = 1;
	src->caps[AG_SINK_WRITE] = 1;
#elif defined(HAVE_SELECT) && defined(AG_THREADS)
	src->sinkFn = AG_EventSinkSELECT;
	src->caps[AG_SINK_READ] = 1;
	src->caps[AG_SINK_WRITE] = 1;
#else
	src->sinkFn = AG_EventSinkSPINNER;
#endif
	if (agSoftTimers) {			/* Force soft timers */
		src->addTimerFn = NULL;
		src->delTimerFn = NULL;
		src->caps[AG_SINK_TIMER] = 0;
	}
	return (src);
}

static void
DestroyEventSource(void *pEventSource)
{
	AG_EventSource *src = pEventSource;
	AG_EventSink *es, *esNext;
	
	if (agEventSource == NULL)
		return;

#ifdef HAVE_KQUEUE
	{
		AG_EventSourceKQUEUE *kq = pEventSource;

		if (kq->fd != -1) {
			close(kq->fd);
		}
		Free(kq->changes);
	}
#endif
	for (es = TAILQ_FIRST(&src->prologues); es != TAILQ_END(&src->prologues); es = esNext) {
		esNext = TAILQ_NEXT(es, sinks);
		free(es);
	}
	for (es = TAILQ_FIRST(&src->epilogues); es != TAILQ_END(&src->epilogues); es = esNext) {
		esNext = TAILQ_NEXT(es, sinks);
		free(es);
	}
	for (es = TAILQ_FIRST(&src->spinners); es != TAILQ_END(&src->spinners); es = esNext) {
		esNext = TAILQ_NEXT(es, sinks);
		free(es);
	}
	for (es = TAILQ_FIRST(&src->sinks); es != TAILQ_END(&src->sinks); es = esNext) {
		esNext = TAILQ_NEXT(es, sinks);
		free(es);
	}
	free(src);
}

/* Return the calling thread's effective event source. */
AG_EventSource *
AG_GetEventSource(void)
{
	AG_EventSource *src;

#ifdef AG_THREADS
	if ((src = AG_ThreadKeyGet(agEventSourceKey)) != NULL && src != NULL)
		return (src);
#else
	if (agEventSource != NULL)
		return (agEventSource);
#endif
	if ((src = CreateEventSource()) == NULL)
		AG_FatalError(NULL);
#ifdef AG_THREADS
	AG_ThreadKeySet(agEventSourceKey, src);
#else
	agEventSource = src;
#endif
	return (src);
}

int
AG_InitEventSubsystem(Uint flags)
{
	/* Initialize the main thread's event source. */
	agEventSource = NULL;
#ifdef AG_THREADS
	if (AG_ThreadKeyTryCreate(&agEventSourceKey, DestroyEventSource) == -1)
		return (-1);
#endif
	if ((agEventSource = AG_GetEventSource()) == NULL) {
		return (-1);
	}
	return (0);
}

void
AG_DestroyEventSubsystem(void)
{
	if (agEventSource != NULL) {
		DestroyEventSource(agEventSource);
		agEventSource = NULL;
	}
}

#ifdef HAVE_KQUEUE
/*
 * Routines for translating between AG_EventSink and kqueue types.
 */
static __inline__ enum ag_event_sink_type
GetSinkType(int filter)
{
	switch (filter) {
	case EVFILT_TIMER:	return (AG_SINK_TIMER);
	case EVFILT_READ:	return (AG_SINK_READ);
	case EVFILT_WRITE:	return (AG_SINK_WRITE);
	case EVFILT_VNODE:	return (AG_SINK_FSEVENT);
	case EVFILT_PROC:	return (AG_SINK_PROCEVENT);
	default:		return (AG_SINK_NONE);
	}
}
static Uint
GetKqFilterFlags(Uint flags)
{
	Uint fflags = 0;
	if (flags & AG_FSEVENT_DELETE) { fflags |= NOTE_DELETE; }
	if (flags & AG_FSEVENT_WRITE)  { fflags |= NOTE_WRITE;  }
	if (flags & AG_FSEVENT_EXTEND) { fflags |= NOTE_EXTEND; }
	if (flags & AG_FSEVENT_ATTRIB) { fflags |= NOTE_ATTRIB; }
	if (flags & AG_FSEVENT_LINK)   { fflags |= NOTE_LINK;   }
	if (flags & AG_FSEVENT_RENAME) { fflags |= NOTE_RENAME; }
	if (flags & AG_FSEVENT_REVOKE) { fflags |= NOTE_REVOKE; }
	if (flags & AG_PROCEVENT_EXIT) { fflags |= NOTE_EXIT; }
	if (flags & AG_PROCEVENT_FORK) { fflags |= NOTE_FORK; }
	if (flags & AG_PROCEVENT_EXEC) { fflags |= NOTE_EXEC; }
	return (fflags);
}
static Uint
GetSinkFlags(Uint fflags)
{
	Uint flags = 0;
	if (fflags & NOTE_DELETE) { fflags |= AG_FSEVENT_DELETE; }
	if (fflags & NOTE_WRITE)  { fflags |= AG_FSEVENT_WRITE;  }
	if (fflags & NOTE_EXTEND) { fflags |= AG_FSEVENT_EXTEND; }
	if (fflags & NOTE_ATTRIB) { fflags |= AG_FSEVENT_ATTRIB; }
	if (fflags & NOTE_LINK)   { fflags |= AG_FSEVENT_LINK;   }
	if (fflags & NOTE_RENAME) { fflags |= AG_FSEVENT_RENAME; }
	if (fflags & NOTE_REVOKE) { fflags |= AG_FSEVENT_REVOKE; }
	if (fflags & NOTE_EXIT) { fflags |= AG_PROCEVENT_EXIT; }
	if (fflags & NOTE_FORK) { fflags |= AG_PROCEVENT_FORK; }
	if (fflags & NOTE_EXEC) { fflags |= AG_PROCEVENT_EXEC; }
	return (flags);
}
#endif /* HAVE_KQUEUE */

/*
 * Add/remove an event processing prologue. The function will be invoked
 * only once at the beginning of AG_EventLoop().
 */
AG_EventSink *
AG_AddEventPrologue(AG_EventSinkFn fn, const char *fnArgs, ...)
{
	AG_EventSource *src = AG_GetEventSource();
	AG_EventSink *es;

	if ((es = TryMalloc(sizeof(AG_EventSink))) == NULL) {
		return (NULL);
	}
	es->type = AG_SINK_PROLOGUE;
	es->fn = fn;
	InitEvent(&es->fnArgs, NULL);
	AG_EVENT_GET_ARGS(&es->fnArgs, fnArgs);
	es->fnArgs.argc0 = es->fnArgs.argc;
	TAILQ_INSERT_TAIL(&src->prologues, es, sinks);
	return (es);
}
void
AG_DelEventPrologue(AG_EventSink *es)
{
	AG_EventSource *src = AG_GetEventSource();

#ifdef AG_DEBUG
	if (es->type != AG_SINK_PROLOGUE)
		AG_FatalError("AG_DelEventPrologue");
#endif
	TAILQ_REMOVE(&src->prologues, es, sinks);
	free(es);
}

/*
 * Add/remove an event sink epilogue. The function will be invoked
 * after all incoming / queued events have been processed.
 */
AG_EventSink *
AG_AddEventEpilogue(AG_EventSinkFn fn, const char *fnArgs, ...)
{
	AG_EventSource *src = AG_GetEventSource();
	AG_EventSink *es;

	if ((es = TryMalloc(sizeof(AG_EventSink))) == NULL) {
		return (NULL);
	}
	es->type = AG_SINK_EPILOGUE;
	es->fn = fn;
	InitEvent(&es->fnArgs, NULL);
	AG_EVENT_GET_ARGS(&es->fnArgs, fnArgs);
	es->fnArgs.argc0 = es->fnArgs.argc;
	TAILQ_INSERT_TAIL(&src->epilogues, es, sinks);
	return (es);
}
void
AG_DelEventEpilogue(AG_EventSink *es)
{
	AG_EventSource *src = AG_GetEventSource();

#ifdef AG_DEBUG
	if (es->type != AG_SINK_EPILOGUE)
		AG_FatalError("AG_DelEventEpilogue");
#endif
	TAILQ_REMOVE(&src->epilogues, es, sinks);
	free(es);
}

/*
 * Add/remove a spinning event sink. If at least one spinning sink exists,
 * AG_EventLoop() will invoke it repeatedly and force non-blocking operation
 * of subsequent polling methods.
 */
AG_EventSink *
AG_AddEventSpinner(AG_EventSinkFn fn, const char *fnArgs, ...)
{
	AG_EventSource *src = AG_GetEventSource();
	AG_EventSink *es;

	if ((es = TryMalloc(sizeof(AG_EventSink))) == NULL) {
		return (NULL);
	}
	es->type = AG_SINK_SPINNER;
	es->fn = fn;
	InitEvent(&es->fnArgs, NULL);
	AG_EVENT_GET_ARGS(&es->fnArgs, fnArgs);
	es->fnArgs.argc0 = es->fnArgs.argc;
	TAILQ_INSERT_TAIL(&src->spinners, es, sinks);
	return (es);
}
void
AG_DelEventSpinner(AG_EventSink *es)
{
	AG_EventSource *src = AG_GetEventSource();

#ifdef AG_DEBUG
	if (es->type != AG_SINK_SPINNER)
		AG_FatalError("AG_DelEventSpinner");
#endif
	TAILQ_REMOVE(&src->spinners, es, sinks);
	free(es);
}

/*
 * Add/remove a low-level event sink. The function will be called
 * whenever the specified event occurs.
 */
AG_EventSink *
AG_AddEventSink(enum ag_event_sink_type type, int ident, Uint flags,
    AG_EventSinkFn fn, const char *fnArgs, ...)
{
	AG_EventSource *src = AG_GetEventSource();
	AG_EventSink *es;
#ifdef HAVE_KQUEUE
	AG_EventSourceKQUEUE *kq = (AG_EventSourceKQUEUE *)src;
	struct kevent *kev;
#endif
	if (type >= AG_SINK_LAST || !src->caps[type]) {
		AG_SetError("Unsupported event type: %u", (Uint)type);
		return (NULL);
	}
	if ((es = TryMalloc(sizeof(AG_EventSink))) == NULL) {
		return (NULL);
	}
	es->type = type;
	es->ident = ident;
	es->flags = flags;

#ifdef HAVE_KQUEUE
	if (GrowKqChangelist(kq, kq->nChanges+1) == -1) {
		free(es);
		return (NULL);
	}
	kev = &kq->changes[kq->nChanges++];
	switch (type) {
	case AG_SINK_READ:
		AG_EV_SET(kev, ident, EVFILT_READ, EV_ADD|EV_ENABLE, 0, 0, es);
		break;
	case AG_SINK_WRITE:
		AG_EV_SET(kev, ident, EVFILT_WRITE, EV_ADD|EV_ENABLE, 0, 0, es);
		break;
	case AG_SINK_FSEVENT:
		AG_EV_SET(kev, ident, EVFILT_VNODE, EV_ADD|EV_ENABLE,
		    GetKqFilterFlags(flags), 0, es);
		break;
	case AG_SINK_PROCEVENT:
		AG_EV_SET(kev, ident, EVFILT_PROC, EV_ADD|EV_ENABLE,
		    GetKqFilterFlags(flags), 0, es);
		break;
	default:
		kq->nChanges--;
		break;
	}
#endif /* HAVE_KQUEUE */

	es->fn = fn;
	InitEvent(&es->fnArgs, NULL);
	AG_EVENT_GET_ARGS(&es->fnArgs, fnArgs);
	es->fnArgs.argc0 = es->fnArgs.argc;
	TAILQ_INSERT_TAIL(&src->sinks, es, sinks);
	return (es);
}
void
AG_DelEventSink(AG_EventSink *es)
{
	AG_EventSource *src = AG_GetEventSource();
#ifdef HAVE_KQUEUE
	AG_EventSourceKQUEUE *kq = (AG_EventSourceKQUEUE *)src;
	struct kevent *kev;

	if (GrowKqChangelist(kq, kq->nChanges+1) == -1) {
		AG_FatalError(NULL);
	}
	kev = &kq->changes[kq->nChanges++];

	switch (es->type) {
	case AG_SINK_READ:
		AG_EV_SET(kev, es->ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
		break;
	case AG_SINK_WRITE:
		AG_EV_SET(kev, es->ident, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
		break;
	case AG_SINK_FSEVENT:
		AG_EV_SET(kev, es->ident, EVFILT_VNODE, EV_DELETE,
		    GetKqFilterFlags(es->flags), 0, NULL);
		break;
	case AG_SINK_PROCEVENT:
		AG_EV_SET(kev, es->ident, EVFILT_PROC, EV_DELETE,
		    GetKqFilterFlags(es->flags), 0, NULL);
		break;
	default:
		kq->nChanges--;
		break;
	}
#endif /* HAVE_KQUEUE */

	TAILQ_REMOVE(&src->sinks, es, sinks);
	free(es);
}
void
AG_DelEventSinksByIdent(enum ag_event_sink_type type, int ident, Uint flags)
{
	AG_EventSource *src = AG_GetEventSource();
	AG_EventSink *es, *esNext;

	for (es = TAILQ_FIRST(&src->sinks); es != TAILQ_END(&src->sinks); es = esNext) {
		esNext = TAILQ_NEXT(es, sinks);
		if (es->type == type &&
		    es->ident == ident &&
		    es->flags == flags)
			AG_DelEventSink(es);
	}
}

#ifdef HAVE_KQUEUE
/*
 * Standard event sink using kqueue(2), commonly found on modern BSD
 * derived operating systems. 
 */
int
AG_EventSinkKQUEUE(void)
{
	AG_EventSourceKQUEUE *kq = (AG_EventSourceKQUEUE *)agEventSource;
	int rv, i;
	struct timespec timeo, *pTimeo;

restart:
	if (!TAILQ_EMPTY(&agEventSource->spinners)) {
		timeo.tv_sec = 0;
		timeo.tv_nsec = 0L;
		pTimeo = &timeo;
	} else {
		pTimeo = NULL;
	}
#ifdef DEBUG_TIMERS
	for (i = 0; i < kq->nChanges; i++) {
		struct kevent *chg = &kq->changes[i];
		Verbose("changes[%d]: f=%d i=%u f=0x%x ff=0x%x u=%p\n",
		    i, (int)chg->filter,
		    (Uint)chg->ident, chg->flags, chg->fflags,
		    chg->udata);
	}
#endif
	rv = kevent(kq->fd, kq->changes, kq->nChanges, kq->events, EVBUFSIZE,
	    pTimeo);
	if (rv < 0) {
		if (errno == EINTR) {
			goto restart;
		}
		AG_SetError("kevent(): %s", AG_Strerror(errno));
		return (-1);
	}
	kq->nChanges = 0;

	/* 1. Process timer expirations. */
	AG_LockTiming();
	for (i = 0; i < rv; i++) {
		struct kevent *kev = &kq->events[i];
		enum ag_event_sink_type esType = GetSinkType(kev->filter);
		Uint32 rvt;
		AG_Timer *to;
		AG_Object *ob;

		if (kev->flags & EV_ERROR) {
			Verbose("kevent (%ld,%d): %s\n", kev->ident, kev->filter,
			    AG_Strerror((int)kev->data));
			continue;
		}
		if (esType != AG_SINK_TIMER ||
		    (to = (AG_Timer *)kev->udata) == NULL) {
			continue;
		}
		rvt = to->fn(to, &to->fnEvent);
		if (rvt > 0) {				/* Restart timer */
			struct kevent *kev;
#ifdef DEBUG_TIMERS
			Verbose("TIMER[%d] resetting t=+%u\n", to->id, (Uint)rvt);
#endif
			if (GrowKqChangelist(kq, kq->nChanges+1) == -1) {
				AG_UnlockTiming();
				return (-1);
			}
			kev = &kq->changes[kq->nChanges++];
			AG_EV_SET(kev, to->id, EVFILT_TIMER,
			    EV_ADD|EV_ENABLE|EV_ONESHOT, 0, (int)rvt, to);
			to->ival = rvt;
		} else {				/* Expire */
#ifdef DEBUG_TIMERS
			Verbose("TIMER[%d] expired\n", to->id);
#endif
			if ((ob = to->obj) == NULL) {
				continue;
			}
			TAILQ_REMOVE(&ob->timers, to, timers);
			if (TAILQ_EMPTY(&ob->timers)) {
				TAILQ_REMOVE(&agTimerObjQ, ob, tobjs);
			}
			if (to->flags & AG_TIMER_AUTO_FREE) {
				free(to);
			} else {
				to->ival = 0;
				to->id = -1;
				to->obj = NULL;
			}
			agTimerCount--;
		}
	}
	AG_UnlockTiming();

	/* 2. Process I/O and other events. */
	for (i = 0; i < rv; i++) {
		struct kevent *kev = &kq->events[i];
		enum ag_event_sink_type esType = GetSinkType(kev->filter);
		AG_EventSink *es;

		switch (esType) {
		case AG_SINK_READ:
		case AG_SINK_WRITE:
			es = (AG_EventSink *)kev->udata;
			es->fn(es, &es->fnArgs);
			break;
		case AG_SINK_FSEVENT:
		case AG_SINK_PROCEVENT:
			es = (AG_EventSink *)kev->udata;
			es->flagsMatched = GetSinkFlags(kev->fflags);
			es->fn(es, &es->fnArgs);
			break;
		default:
			break;
		}
	}
	return (0);
}

/*
 * Add/remove a kqueue(2) based timer.
 */
static int
GenerateTimerID(AG_Timer *to)
{
	AG_Object *obOther;
	AG_Timer *toOther;
	int id;

gen_id:
#ifdef AG_DEBUG
	if (agTimerCount+1 >= (AG_INT_MAX-1))
		AG_FatalError("agTimerCount");
#endif
	id = (int)++agTimerCount;			/* XXX */
	TAILQ_FOREACH(obOther, &agTimerObjQ, tobjs) {
		TAILQ_FOREACH(toOther, &obOther->timers, timers) {
			if (toOther == to) { continue; }
			if (toOther->id == id) {
				id++;
				goto gen_id;
			}
		}
	}
	return (id);
}
int
AG_AddTimerKQUEUE(AG_Timer *to, Uint32 ival, int newTimer)
{
	AG_EventSourceKQUEUE *kq = (AG_EventSourceKQUEUE *)agEventSource;
	
	/* Create a kernel-based timer with kqueue. */
	if (newTimer) {
		to->id = GenerateTimerID(to);
	}
	if (newTimer || to->ival != ival) {
		struct kevent *kev;
#ifdef DEBUG_TIMERS
		Verbose("kevent: creating timer ID=%d ival=%d\n", to->id, (int)ival);
#endif
		if (GrowKqChangelist(kq, kq->nChanges+1) == -1) {
			return (-1);
		}
		kev = &kq->changes[kq->nChanges++];
		AG_EV_SET(kev, to->id, EVFILT_TIMER,
		    EV_ADD|EV_ENABLE|EV_ONESHOT, 0, (int)ival, to);
		to->ival = ival;
	}
	return (0);
}
void
AG_DelTimerKQUEUE(AG_Timer *to)
{
	AG_EventSourceKQUEUE *kq = (AG_EventSourceKQUEUE *)agEventSource;
	struct kevent *kev;

	if (GrowKqChangelist(kq, kq->nChanges+1) == -1) {
		AG_FatalError(NULL);
	}
	kev = &kq->changes[kq->nChanges++];
	AG_EV_SET(kev, to->id, EVFILT_TIMER, EV_DELETE,
	    0, 0, NULL);
	agTimerCount--;
}

#endif /* HAVE_KQUEUE */

#ifdef HAVE_TIMERFD
/*
 * Standard event sink using select(2) and fd-based timers,
 * usually available on Linux.
 */
int
AG_EventSinkTIMERFD(void)
{
	fd_set rdFds, wrFds;
	int nFds, rv;
	AG_EventSink *es;
	AG_Object *ob, *obNext;
	AG_Timer *to, *toNext;
	struct timeval timeo, *pTimeo;

restart:
	nFds = 0;
	FD_ZERO(&rdFds);
	FD_ZERO(&wrFds);
	TAILQ_FOREACH(es, &agEventSource->sinks, sinks) {
		switch (es->type) {
		case AG_SINK_READ:
			FD_SET(es->ident, &rdFds);
			if (es->ident > nFds) { nFds = es->ident; }
			break;
		case AG_SINK_WRITE:
			FD_SET(es->ident, &wrFds);
			if (es->ident > nFds) { nFds = es->ident; }
			break;
		}
	}
	TAILQ_FOREACH(ob, &agTimerObjQ, tobjs) {
		TAILQ_FOREACH(to, &ob->timers, timers) {
			FD_SET(to->id, &rdFds);
			if (to->id > nFds) { nFds = to->id; }
		}
	}
	if (!TAILQ_EMPTY(&agEventSource->spinners)) {
		timeo.tv_sec = 0;
		timeo.tv_usec = 0;
		pTimeo = &timeo;
	} else {
		pTimeo = NULL;
	}
	rv = select(nFds+1, &rdFds, &wrFds, NULL, pTimeo);
	if (rv == -1) {
		if (errno == EINTR) {
			goto restart;
		}
		AG_SetError("select: %s", AG_Strerror(errno));
		return (-1);
	}
	
	AG_LockTiming();

	/* 1. Process timer expirations. */
	for (ob = TAILQ_FIRST(&agTimerObjQ);
	     ob != TAILQ_END(&agTimerObjQ);
	     ob = obNext) {
		obNext = TAILQ_NEXT(ob, tobjs);
		AG_ObjectLock(ob);
		for (to = TAILQ_FIRST(&ob->timers);
		     to != TAILQ_END(&ob->timers);
		     to = toNext) {
			struct itimerspec its;
			Uint32 rvt;

			toNext = TAILQ_NEXT(to, timers);
			if (!FD_ISSET(to->id, &rdFds)) {
				continue;
			}
			rvt = to->fn(to, &to->fnEvent);
			if (rvt > 0) {
				its.it_value.tv_sec = rvt/1000;
				its.it_value.tv_nsec = (rvt % 1000)*1000000L;
				its.it_interval.tv_sec = 0;
				its.it_interval.tv_nsec = 0L;
				if (timerfd_settime(to->id, 0, &its, NULL) == -1) {
					Verbose("timerfd_settime: %s\n", AG_Strerror(errno));
					FD_CLR(to->id, &rdFds);
					AG_DelTimer(ob, to);
				}
			} else {
				FD_CLR(to->id, &rdFds);
				AG_DelTimer(ob, to);
			}
		}
		AG_ObjectUnlock(ob);
	}
	
	/* 2. Process I/O events. */
	TAILQ_FOREACH(es, &agEventSource->sinks, sinks) {
		switch (es->type) {
		case AG_SINK_READ:
			if (FD_ISSET(es->ident, &rdFds)) {
				es->fn(es, &es->fnArgs);
			}
			break;
		case AG_SINK_WRITE:
			if (FD_ISSET(es->ident, &wrFds)) {
				es->fn(es, &es->fnArgs);
			}
			break;
		}
	}

	AG_UnlockTiming();
	return (0);
}

/*
 * Add/remove a fd-based timer.
 */
int
AG_AddTimerTIMERFD(AG_Timer *to, Uint32 ival, int newTimer)
{
	struct itimerspec its;

	if (newTimer) {
		/* Create a timerfd. Store the file descriptor as ID. */
		if ((to->id = timerfd_create(CLOCK_MONOTONIC, 0)) == -1) {
			AG_SetError("timerfd_create: %s", AG_Strerror(errno));
			return (-1);
		}
	}
	its.it_value.tv_sec = ival/1000;
	its.it_value.tv_nsec = (ival % 1000)*1000000L;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0L;
	if (timerfd_settime(to->id, 0, &its, NULL) == -1) {
		close(to->id);
		AG_SetError("timerfd_settime: %s", AG_Strerror(errno));
		return (-1);
	}
	to->ival = ival;
	return (0);
}
void
AG_DelTimerTIMERFD(AG_Timer *to)
{
#ifdef AG_DEBUG
	if (to->id == -1) { AG_FatalError("timerfd inconsistency"); }
#endif
	close(to->id);
}
#endif /* HAVE_TIMERFD */

#if defined(HAVE_SELECT) && !defined(AG_THREADS)
/*
 * Standard event sink using select(2) with timers implemented using the
 * select() timeout. Only available in single-threaded builds, since timers
 * cannot be added, restarted or removed from different threads with this
 * method.
 */
int
AG_EventSinkTIMEDSELECT(void)
{
	fd_set rdFds, wrFds;
	int i, nFds, rv;
	AG_EventSink *es;
	AG_Object *ob, *obNext;
	AG_Timer *to, *toNext;
	struct timeval timeo, *pTimeo;
	Uint32 t, tSoonest;

restart:
	nFds = 0;
	FD_ZERO(&rdFds);
	FD_ZERO(&wrFds);
	TAILQ_FOREACH(es, &agEventSource->sinks, sinks) {
		switch (es->type) {
		case AG_SINK_READ:
			FD_SET(es->ident, &rdFds);
			if (es->ident > nFds) { nFds = es->ident; }
			break;
		case AG_SINK_WRITE:
			FD_SET(es->ident, &wrFds);
			if (es->ident > nFds) { nFds = es->ident; }
			break;
		}
	}

	if (!TAILQ_EMPTY(&agEventSource->spinners)) {
		timeo.tv_sec = 0;
		timeo.tv_usec = 0;
	} else {
		AG_LockTiming();
		t = AG_GetTicks();
		tSoonest = 0xfffffffe;
		TAILQ_FOREACH(ob, &agTimerObjQ, tobjs) {
			TAILQ_FOREACH(to, &ob->timers, timers) {
				if ((to->tSched - t) < tSoonest)
					tSoonest = (to->tSched - t);
			}
		}
		timeo.tv_sec = tSoonest/1000;
		timeo.tv_usec = (tSoonest % 1000)*1000;
		AG_UnlockTiming();
	}
	rv = select(nFds+1, &rdFds, &wrFds, NULL, &timeo);
	if (rv == -1) {
		if (errno == EINTR) {
			goto restart;
		}
		AG_SetError("select: %s", AG_Strerror(errno));
		return (-1);
	}
	
	AG_LockTiming();
	/* 1. Process timer expirations. */
	AG_ProcessTimeouts(t);
	if (rv > 0) {
		/* 2. Process I/O events */
		TAILQ_FOREACH(es, &agEventSource->sinks, sinks) {
			switch (es->type) {
			case AG_SINK_READ:
				if (FD_ISSET(es->ident, &rdFds)) {
					es->fn(es, &es->fnArgs);
				}
				break;
			case AG_SINK_WRITE:
				if (FD_ISSET(es->ident, &wrFds)) {
					es->fn(es, &es->fnArgs);
				}
				break;
			}
		}
	}
	AG_UnlockTiming();
	return (0);
}
#endif /* HAVE_SELECT and !AG_THREADS */

#if defined(HAVE_SELECT) && defined(AG_THREADS)
/*
 * Standard event sink using non-blocking select(2) with timers implemented
 * with a delay loop. This is inefficient, but on some platforms, it is the
 * only thread-safe option.
 */
int
AG_EventSinkSELECT(void)
{
	fd_set rdFds, wrFds;
	int nFds, rv;
	AG_EventSink *es;
	struct timeval timeo;

	nFds = 0;
	FD_ZERO(&rdFds);
	FD_ZERO(&wrFds);
	
	TAILQ_FOREACH(es, &agEventSource->sinks, sinks) {
		switch (es->type) {
		case AG_SINK_READ:
			FD_SET(es->ident, &rdFds);
			if (es->ident > nFds) { nFds = es->ident; }
			break;
		case AG_SINK_WRITE:
			FD_SET(es->ident, &wrFds);
			if (es->ident > nFds) { nFds = es->ident; }
			break;
		default:
			break;
		}
	}

restart:
	timeo.tv_sec = 0;
	timeo.tv_usec = 0;
	rv = select(nFds+1, &rdFds, &wrFds, NULL, &timeo);
	if (rv == -1) {
		if (errno == EINTR) {
			goto restart;
		}
		AG_SetError("select: %s", AG_Strerror(errno));
		return (-1);
	}
	
	AG_LockTiming();
	/* 1. Process timer expirations. */
	AG_ProcessTimeouts(AG_GetTicks());
	if (rv > 0) {
		/* 2. Process I/O events. */
		TAILQ_FOREACH(es, &agEventSource->sinks, sinks) {
			switch (es->type) {
			case AG_SINK_READ:
				if (FD_ISSET(es->ident, &rdFds)) {
					es->fn(es, &es->fnArgs);
				}
				break;
			case AG_SINK_WRITE:
				if (FD_ISSET(es->ident, &wrFds)) {
					es->fn(es, &es->fnArgs);
				}
				break;
			}
		}
	}
	AG_UnlockTiming();
	
	if (TAILQ_EMPTY(&agEventSource->spinners)) {
		AG_Delay(1);
	}
	return (0);
}
#endif /* HAVE_SELECT and AG_THREADS */

/*
 * Fallback "spinning" event sink using a delay loop. This is inefficient,
 * but is the only option on some platforms.
 */
int
AG_EventSinkSPINNER(void)
{
	AG_ProcessTimeouts(AG_GetTicks());
	AG_Delay(1);
	return (0);
}

/*
 * Standard event loop routine. We loop over the event source and invoke
 * the related event sinks. AG_EventLoop() may be used outside of the
 * main thread (in which case, a new event source will be created).
 */
int
AG_EventLoop(void)
{
	AG_EventSource *src = AG_GetEventSource();
	AG_EventSink *es;
	
	TAILQ_FOREACH(es, &src->prologues, sinks) {
		es->fn(es, &es->fnArgs);
	}
	for (;;) {
		TAILQ_FOREACH(es, &src->spinners, sinks) {
			es->fn(es, &es->fnArgs);
		}
		if (src->sinkFn() == -1) {
			return (1);
		}
		TAILQ_FOREACH(es, &src->epilogues, sinks) {
			es->fn(es, &es->fnArgs);
		}
		if (src->breakReq)
			break;
	}
	return (src->returnCode);
}

/* Request that we break out of AG_EventLoop(). */
void
AG_Terminate(int retCode)
{
	AG_EventSource *src = AG_GetEventSource();

	src->breakReq = 1;
	src->returnCode = retCode;
}
void
AG_TerminateEv(AG_Event *ev)
{
	AG_EventSource *src = AG_GetEventSource();

	if (ev->argc > 1 &&
	    ev->argv[1].type == AG_VARIABLE_INT) {
		src->returnCode = ev->argv[1].data.i;
	} else {
		src->returnCode = 0;
	}
	src->breakReq = 1;
}
