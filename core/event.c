/*
 * Copyright (c) 2001-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

/* Expensive debugging output related to event delivery. */
/* #define DEBUG_EVENTS */

AG_EventSource *_Nullable agEventSource = NULL;	/* Event source (thread-local) */
#ifdef AG_THREADS
AG_ThreadKey agEventSourceKey;
#endif

#ifdef HAVE_KQUEUE

/* Size of kqueue input event buffer (in kevents). */
# ifndef AG_KQ_EVBUFSIZE
# define AG_KQ_EVBUFSIZE 2
# endif

/* Initial size of kqueue changes[] array. */
# ifndef AG_KQ_INIT_MAXCHANGES
# define AG_KQ_INIT_MAXCHANGES AG_MODEL
# endif

typedef struct ag_event_source_kqueue {
	struct ag_event_source _inherit;  /* EventSource -> EventSourceKQUEUE */
	struct kevent *_Nullable changes; /* Queued changes */
	Uint                    nChanges;
	Uint                  maxChanges;
	struct kevent events[AG_KQ_EVBUFSIZE];  /* Input event buffer */
	int fd;                                 /* kqueue() fd */
	Uint32 _pad;
} AG_EventSourceKQUEUE;

static int GrowKqChangelist(AG_EventSourceKQUEUE *_Nonnull, Uint);
#endif /* HAVE_KQUEUE */

/* #define DEBUG_TIMERS */

#ifdef __NetBSD__
# define AG_EV_SET(kevp,a,b,c,d,e,f) EV_SET((kevp),(uintptr_t)(a),(b),(c),(d),(e),(intptr_t)(f))
#else
# define AG_EV_SET(kevp,a,b,c,d,e,f) EV_SET((kevp),(a),(b),(c),(d),(e),(f))
#endif

/* Import inlinables */
#undef AG_INLINE_HEADER
#include <agar/core/inline_event.h>

#ifdef AG_DEBUG
static __inline__ void
InitDebugName(AG_Variable *_Nonnull V, const char *tag)
{
	Strlcpy(V->name, tag, sizeof(V->name));
}
#else
# define InitDebugName(V,tag)
#endif

/* Initialize a pointer argument. */
static __inline__ void
InitPointerArg(AG_Variable *_Nonnull V, void *_Nullable p)
{
#ifdef AG_DEBUG
	memset(V->name, '\0', sizeof(V->name));
#else
	V->name[0] = '\0';
#endif
	V->type = AG_VARIABLE_POINTER;
#ifdef AG_THREADS
	V->mutex = NULL;
#endif
	V->data.p = p;
	V->info.pFlags = 0;
#ifdef AG_DEBUG
	V->vars.tqe_next = NULL;
	V->vars.tqe_prev = NULL;
#endif
}

static __inline__ void
InitEvent(AG_Event *_Nonnull ev, AG_Object *_Nullable ob)
{
#ifdef AG_DEBUG
	memset(ev->name, '\0', sizeof(ev->name));
#else
	ev->name[0] = '\0';
#endif
	ev->fn = NULL;
	ev->argc = 1;
	ev->argc0 = 1;
	InitPointerArg(&ev->argv[0], ob);
	InitDebugName (&ev->argv[0], "self");
#ifdef AG_DEBUG
	ev->events.tqe_next = NULL;
	ev->events.tqe_prev = NULL;
#endif
}

/* Initialize an AG_Event structure. */
void
AG_EventInit(AG_Event *_Nonnull ev)
{
	InitEvent(ev, NULL);
}

/* Initialize an AG_Event structure with the specified arguments. */
void
AG_EventArgs(AG_Event *ev, const char *fmt, ...)
{
	va_list ap;

	InitEvent(ev, NULL);
	if (fmt) {
		va_start(ap, fmt);
		AG_EventGetArgs(ev, fmt, ap);
		va_end(ap);
	}
	ev->argc0 = ev->argc;
}

/* Return a newly allocated AG_Event. */
AG_Event *
AG_EventNew(AG_EventFn fn, void *obj, const char *fmt, ...)
{
	AG_Event *ev;
	va_list ap;

	ev = AG_Malloc(sizeof(AG_Event));
	InitEvent(ev, obj);
	ev->fn = fn;
	if (fmt) {
		va_start(ap, fmt);
		AG_EventGetArgs(ev, fmt, ap);
		va_end(ap);
	}
	return (ev);
}

#if AG_MODEL != AG_SMALL
/* Return a newly-allocated duplicate of the given AG_Event. */
AG_Event *
AG_EventDup(const AG_Event *event)
{
	AG_Event *ev;

	ev = Malloc(sizeof(AG_Event));
	AG_EventCopy(ev, event);
	return (ev);
}
#endif /* !AG_SMALL */

/*
 * Copy the name, callback pointer, arguments and inheritable flags
 * from a source AG_Event to a destination AG_Event.
 */
void
AG_EventCopy(AG_Event *dst, const AG_Event *src)
{
	int i;

	memcpy(dst->name, src->name, sizeof(dst->name));
	dst->fn = src->fn;
	dst->argc = src->argc;
	dst->argc0 = src->argc0;
	for (i = 0; i < src->argc; i++) {
		AG_CopyVariable(&dst->argv[i], &src->argv[i]);
	}
	dst->events.tqe_next = NULL;
	dst->events.tqe_prev = NULL;
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
			if (Strlcpy(ev->name, name, sizeof(ev->name)) >= sizeof(ev->name))
				AG_FatalError("Event name too big");
		} else {
			ev->name[0] = '\0';
		}
		TAILQ_INSERT_TAIL(&ob->events, ev, events);
	} else {
		ev->argc = 1;
		ev->argc0 = 1;
	}
	InitPointerArg(&ev->argv[0], ob);
	InitDebugName (&ev->argv[0], "self");
	ev->fn = fn;

	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(ev, fmt, ap);
		va_end(ap);
	}
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
		if (Strlcpy(ev->name, name, sizeof(ev->name)) >= sizeof(ev->name))
			AG_FatalError("Event name too big");
	} else {
		ev->name[0] = '\0';
	}

	ev->fn = fn;

	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(ev, fmt, ap);
		va_end(ap);
	}
	ev->argc0 = ev->argc;

	TAILQ_INSERT_TAIL(&ob->events, ev, events);
	AG_ObjectUnlock(ob);
	return (ev);
}

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

/* Delete an event handler by reference. */
void
AG_UnsetEventByPtr(void *p, AG_Event *ev)
{
	AG_Object *ob = p;

	AG_ObjectLock(ob);
	TAILQ_REMOVE(&ob->events, ev, events);
	AG_ObjectUnlock(ob);

	free(ev);
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

#ifdef AG_TIMERS
/*
 * Timeout callback for scheduled events.
 */
static Uint32
EventTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Object *obj = AG_OBJECT_SELF();
	const char *eventName = AG_STRING(1);
	AG_Event *ev;

# ifdef DEBUG_EVENTS
	Debug(obj, "Event <%s> timeout (%u ticks)\n", eventName,
	    (Uint)to->ival);
# endif
	TAILQ_FOREACH(ev, &obj->events, events) {
		if (strcmp(eventName, ev->name) == 0)
			break;
	}
	if (ev == NULL) {
		return (0);
	}
	/* Invoke the event handler routine. */
	if (ev->fn != NULL) {
		ev->fn(ev);
	}
	return (0);
}
#endif /* AG_TIMERS */

/*
 * Variant of AG_PostEvent() which accepts a resolved AG_Event pointer
 * (as opposed to an event name string).
 */
void
AG_PostEventByPtr(void *pObj, AG_Event *ev, const char *fmt, ...)
{
	AG_Object *obj = pObj;
	va_list ap;

# ifdef DEBUG_EVENTS
	Debug(obj, "Event %p posted\n", ev);
# endif
	AG_ObjectLock(obj);
# if AG_MODEL == AG_SMALL
	{
		AG_Event *evTmp = Malloc(sizeof(AG_Event));

		memcpy(evTmp, ev, sizeof(AG_Event));
		if (fmt) {
			va_start(ap, fmt);
			AG_EventGetArgs(evTmp, fmt, ap);
			va_end(ap);
		}
		if (evTmp->fn != NULL) {
			evTmp->fn(evTmp);
		}
		free(evTmp);
	}
# else /* MEDIUM or LARGE */
	{
		AG_Event evTmp;				/* Fits the stack */

		memcpy(&evTmp, ev, sizeof(AG_Event));
		if (fmt) {
			va_start(ap, fmt);
			AG_EventGetArgs(&evTmp, fmt, ap);
			va_end(ap);
		}
		if (evTmp.fn != NULL)
			evTmp.fn(&evTmp);
	}
# endif /* MEDIUM or LARGE */

	AG_ObjectUnlock(obj);
}

/*
 * Post an event (by name) to an object. If fmt is given, append the
 * given arguments (specified in the same format as AG_SetEvent(3)),
 * to the end of the argument vector.
 */
void
AG_PostEvent(void *pObj, const char *evname, const char *fmt, ...)
{
	AG_Object *obj = pObj;
	AG_Event *ev;
	va_list ap;

#ifdef AG_DEBUG
	if (obj == NULL) { AG_FatalError("NULL object"); }
#endif
#ifdef DEBUG_EVENTS
	Debug(obj, "PostEvent <%s>\n", evname);
#endif
	AG_ObjectLock(obj);
	TAILQ_FOREACH(ev, &obj->events, events) {
		if (strcmp(evname, ev->name) != 0)
			continue;
#if AG_MODEL == AG_SMALL
		{
			AG_Event *evTmp = Malloc(sizeof(AG_Event));

			memcpy(evTmp, ev, sizeof(AG_Event));
			if (fmt) {
				va_start(ap, fmt);
				AG_EventGetArgs(evTmp, fmt, ap);
				va_end(ap);
			}
			if (evTmp->fn != NULL) {
				evTmp->fn(evTmp);
			}
			free(evTmp);
		}
#else /* MEDIUM or LARGE */
		{
			AG_Event evTmp;			/* Fits the stack */

			memcpy(&evTmp, ev, sizeof(AG_Event));
			if (fmt) {
				va_start(ap, fmt);
				AG_EventGetArgs(&evTmp, fmt, ap);
				va_end(ap);
			}
			if (evTmp.fn != NULL)
				evTmp.fn(&evTmp);
		}
#endif /* MEDIUM or LARGE */
	}
	AG_ObjectUnlock(obj);
}

/*
 * Parse a format string of AG_Event-style arguments into the argument
 * vector of ev. The parser macros are defined in <core/event.h>.
 */
void
AG_EventGetArgs(AG_Event *ev, const char *fmt, va_list ap)
{
	if (fmt) {
		const char *c = (const char *)fmt;

		while (*c != '\0')
			AG_EVENT_PUSH_ARG(ap, ev);
	}
}

/*
 * Forward an event to an object. The original arguments are copied
 * as is, except for Pointer 0 (SELF) which becomes pObj.
 */
void
AG_ForwardEvent(void *pObj, const AG_Event *event)
{
	AG_Object *obj = pObj;
	AG_Event *ev;

#ifdef DEBUG_EVENTS
	Debug(obj, "Event <%s> forwarded\n", event->name);
#endif
	AG_ObjectLock(obj);
	TAILQ_FOREACH(ev, &obj->events, events) {
		if (strcmp(event->name, ev->name) != 0)
			continue;
#if AG_MODEL == AG_SMALL
		{
			AG_Event *evTmp = Malloc(sizeof(AG_Event));

			memcpy(evTmp, event, sizeof(AG_Event));
			InitPointerArg(&evTmp->argv[0], obj);
			InitDebugName (&evTmp->argv[0], "self");
			if (ev->fn != NULL) {
				ev->fn(evTmp);
			}
			free(evTmp);
		}
#else /* MEDIUM or LARGE */
		{
			AG_Event evTmp;			/* Fits the stack */

			memcpy(&evTmp, event, sizeof(AG_Event));
			InitPointerArg(&evTmp.argv[0], obj);
			InitDebugName (&evTmp.argv[0], "self");

			if (ev->fn != NULL)
				ev->fn(&evTmp);
		}
#endif /* MEDIUM or LARGE */
	}
	AG_ObjectUnlock(obj);
}

#ifdef AG_TIMERS
/*
 * Schedule the execution of the named event in the given number
 * of AG_Time(3) ticks.
 *
 * The argument vector passed to the event handler function contains
 * the AG_SetEvent() arguments, and any arguments specified here are
 * appended to that list.
 */
int
AG_SchedEvent(void *pObj, Uint32 ticks, const char *evname, const char *fmt, ...)
{
	AG_Object *obj = pObj;
	AG_Event *ev;
	AG_Timer *to;
	va_list ap;

	if ((to = TryMalloc(sizeof(AG_Timer))) == NULL) {
		return (-1);
	}
	AG_InitTimer(to, evname, AG_TIMER_AUTO_FREE);

	AG_LockTiming();
	AG_ObjectLock(obj);
	
	if (AG_AddTimer(obj, to, ticks, EventTimeout, "%s", evname) == -1) {
		free(to);
		goto fail;
	}
	ev = &to->fnEvent;
	AG_EventInit(ev);
	ev->argv[0].data.p = obj;
	if (fmt) {
		va_start(ap, fmt);
		AG_EventGetArgs(ev, fmt, ap);
		va_end(ap);
	}
	ev->argc0 = ev->argc;

	AG_UnlockTiming();
	AG_ObjectUnlock(obj);
	return (0);
fail:
	AG_UnlockTiming();
	AG_ObjectUnlock(obj);
	return (-1);
}
#endif /* AG_TIMERS */

#ifdef AG_EVENT_LOOP
/*
 * Create a new event source.
 */
static AG_EventSource *_Nullable
CreateEventSource(void)
{
# ifdef HAVE_KQUEUE
	AG_EventSourceKQUEUE *kq = TryMalloc(sizeof(AG_EventSourceKQUEUE));
	AG_EventSource *src = (AG_EventSource *)kq;
# else
	AG_EventSource *src = TryMalloc(sizeof(AG_EventSource));
# endif
	if (src == NULL) {
		return (NULL);
	}
	src->flags = 0;
	src->breakReq = 0;
# ifdef AG_TIMERS
	src->addTimerFn = NULL;
	src->delTimerFn = NULL;
# endif
	TAILQ_INIT(&src->prologues);
	TAILQ_INIT(&src->epilogues);
	TAILQ_INIT(&src->spinners);
	TAILQ_INIT(&src->sinks);
	src->returnCode = 0;
	memset(src->caps, 0, sizeof(src->caps));

# if defined(HAVE_KQUEUE)
	if ((kq->fd = kqueue()) == -1) {
		AG_SetError("kqueue: %s", AG_Strerror(errno));
		return (NULL);
	}
	kq->changes = NULL;
	kq->nChanges = 0;
	kq->maxChanges = 0;
	memset(kq->events, 0, AG_KQ_EVBUFSIZE*sizeof(struct kevent));
	src->sinkFn = AG_EventSinkKQUEUE;
#  ifdef AG_TIMERS
	src->addTimerFn = AG_AddTimerKQUEUE;
	src->delTimerFn = AG_DelTimerKQUEUE;
#  endif
	src->caps[AG_SINK_TIMER] = 1;		/* Provides timers internally */
	src->caps[AG_SINK_READ] = 1;
	src->caps[AG_SINK_WRITE] = 1;
	src->caps[AG_SINK_FSEVENT] = 1;
	src->caps[AG_SINK_PROCEVENT] = 1;
	if (GrowKqChangelist(kq, AG_KQ_INIT_MAXCHANGES) == -1) {
		AG_FatalError("GrowKqChangelist");
	}
# elif defined(HAVE_TIMERFD)
	src->sinkFn = AG_EventSinkTIMERFD;
#  ifdef AG_TIMERS
	src->addTimerFn = AG_AddTimerTIMERFD;
	src->delTimerFn = AG_DelTimerTIMERFD;
#  endif
	src->caps[AG_SINK_TIMER] = 1;		/* Provides timers internally */
	src->caps[AG_SINK_READ] = 1;
	src->caps[AG_SINK_WRITE] = 1;
# elif defined(HAVE_SELECT) && !defined(AG_THREADS)
	src->sinkFn = AG_EventSinkTIMEDSELECT;
	src->caps[AG_SINK_READ] = 1;
	src->caps[AG_SINK_WRITE] = 1;
# elif defined(HAVE_SELECT) && defined(AG_THREADS)
	src->sinkFn = AG_EventSinkSELECT;
	src->caps[AG_SINK_READ] = 1;
	src->caps[AG_SINK_WRITE] = 1;
# else
	src->sinkFn = AG_EventSinkSPINNER;
# endif
# ifdef AG_TIMERS
	if (agSoftTimers) {			/* Force soft timers */
		src->addTimerFn = NULL;
		src->delTimerFn = NULL;
		src->caps[AG_SINK_TIMER] = 0;
	}
# else
	src->caps[AG_SINK_TIMER] = 0;
# endif
	return (src);
}

/* Free all resources allocated by an event source. */
static void
DestroyEventSource(void *_Nullable pEventSource)
{
	AG_EventSource *src = pEventSource;
	AG_EventSink *es, *esNext;
	
	if (agEventSource == NULL)
		return;

# ifdef HAVE_KQUEUE
	{
		AG_EventSourceKQUEUE *kq = pEventSource;

		if (kq->fd != -1) {
			close(kq->fd);
		}
		Free(kq->changes);
	}
# endif
	for (es = TAILQ_FIRST(&src->prologues);
	     es != TAILQ_END(&src->prologues);
	     es = esNext) {
		esNext = TAILQ_NEXT(es, sinks);
		free(es);
	}
	for (es = TAILQ_FIRST(&src->epilogues);
	     es != TAILQ_END(&src->epilogues);
	     es = esNext) {
		esNext = TAILQ_NEXT(es, sinks);
		free(es);
	}
	for (es = TAILQ_FIRST(&src->spinners);
	     es != TAILQ_END(&src->spinners);
	     es = esNext) {
		esNext = TAILQ_NEXT(es, sinks);
		free(es);
	}
	for (es = TAILQ_FIRST(&src->sinks);
	     es != TAILQ_END(&src->sinks);
	     es = esNext) {
		esNext = TAILQ_NEXT(es, sinks);
		free(es);
	}
	free(src);
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

/* Return the calling thread's effective event source. */
AG_EventSource *
AG_GetEventSource(void)
{
	AG_EventSource *src;

# ifdef AG_THREADS
	if ((src = AG_ThreadKeyGet(agEventSourceKey)) != NULL && src != NULL)
		return (src);
# else
	if (agEventSource != NULL)
		return (agEventSource);
# endif
	if ((src = CreateEventSource()) == NULL)
		AG_FatalError(NULL);
# ifdef AG_THREADS
	AG_ThreadKeySet(agEventSourceKey, src);
# else
	agEventSource = src;
# endif
	return (src);
}

# ifdef HAVE_KQUEUE
/*
 * Routines for translating between AG_EventSink and kqueue types.
 */
static __inline__ enum ag_event_sink_type _Const_Attribute
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
static Uint _Const_Attribute
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
static Uint _Const_Attribute
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
# endif /* HAVE_KQUEUE */

/*
 * Add/remove an event processing prologue. The function will be invoked
 * only once at the beginning of AG_EventLoop().
 */
AG_EventSink *
AG_AddEventPrologue(AG_EventSinkFn fn, const char *fnArgs, ...)
{
	AG_EventSource *src = AG_GetEventSource();
	AG_EventSink *es;
	va_list ap;

	if ((es = TryMalloc(sizeof(AG_EventSink))) == NULL) {
		return (NULL);
	}
	es->type = AG_SINK_PROLOGUE;
	es->fn = fn;
	InitEvent(&es->fnArgs, NULL);
	if (fnArgs) {
		va_start(ap, fnArgs);
		AG_EventGetArgs(&es->fnArgs, fnArgs, ap);
		va_end(ap);
	}
	es->fnArgs.argc0 = es->fnArgs.argc;
	TAILQ_INSERT_TAIL(&src->prologues, es, sinks);
	return (es);
}
void
AG_DelEventPrologue(AG_EventSink *es)
{
	AG_EventSource *src = AG_GetEventSource();

# ifdef AG_DEBUG
	if (es->type != AG_SINK_PROLOGUE)
		AG_FatalError("AG_DelEventPrologue");
# endif
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
	va_list ap;

	if ((es = TryMalloc(sizeof(AG_EventSink))) == NULL) {
		return (NULL);
	}
	es->type = AG_SINK_EPILOGUE;
	es->fn = fn;
	InitEvent(&es->fnArgs, NULL);
	if (fnArgs) {
		va_start(ap, fnArgs);
		AG_EventGetArgs(&es->fnArgs, fnArgs, ap);
		va_end(ap);
	}
	es->fnArgs.argc0 = es->fnArgs.argc;
	TAILQ_INSERT_TAIL(&src->epilogues, es, sinks);
	return (es);
}
void
AG_DelEventEpilogue(AG_EventSink *es)
{
	AG_EventSource *src = AG_GetEventSource();

# ifdef AG_DEBUG
	if (es->type != AG_SINK_EPILOGUE)
		AG_FatalError("AG_DelEventEpilogue");
# endif
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
	va_list ap;

	if ((es = TryMalloc(sizeof(AG_EventSink))) == NULL) {
		return (NULL);
	}
	es->type = AG_SINK_SPINNER;
	es->fn = fn;
	InitEvent(&es->fnArgs, NULL);
	if (fnArgs) {
		va_start(ap, fnArgs);
		AG_EventGetArgs(&es->fnArgs, fnArgs, ap);
		va_end(ap);
	}
	es->fnArgs.argc0 = es->fnArgs.argc;
	TAILQ_INSERT_TAIL(&src->spinners, es, sinks);
	return (es);
}
void
AG_DelEventSpinner(AG_EventSink *es)
{
	AG_EventSource *src = AG_GetEventSource();

# ifdef AG_DEBUG
	if (es->type != AG_SINK_SPINNER)
		AG_FatalError("AG_DelEventSpinner");
# endif
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
# ifdef HAVE_KQUEUE
	AG_EventSourceKQUEUE *kq = (AG_EventSourceKQUEUE *)src;
	struct kevent *kev;
# endif
	va_list ap;

	if (type >= AG_SINK_LAST || !src->caps[type]) {
		AG_SetErrorV("E1", "No such event type");
		return (NULL);
	}
	if ((es = TryMalloc(sizeof(AG_EventSink))) == NULL) {
		return (NULL);
	}
	es->type = type;
	es->ident = ident;
	es->flags = flags;

# ifdef HAVE_KQUEUE
	if (kq->nChanges+1 > kq->maxChanges &&
	    GrowKqChangelist(kq, kq->nChanges+1) == -1) {
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
# endif /* HAVE_KQUEUE */

	es->fn = fn;
	InitEvent(&es->fnArgs, NULL);
	if (fnArgs) {
		va_start(ap, fnArgs);
		AG_EventGetArgs(&es->fnArgs, fnArgs, ap);
		va_end(ap);
	}
	es->fnArgs.argc0 = es->fnArgs.argc;
	TAILQ_INSERT_TAIL(&src->sinks, es, sinks);
	return (es);
}

# ifdef HAVE_KQUEUE
static int
GrowKqChangelist(AG_EventSourceKQUEUE *_Nonnull kq, Uint n)
{
	struct kevent *changesNew;

	if ((changesNew = TryRealloc(kq->changes, n*sizeof(struct kevent)))
	    == NULL) {
		return (-1);
	}
	kq->changes = changesNew;
	kq->maxChanges = n;
	return (0);
}
# endif /* HAVE_KQUEUE */

void
AG_DelEventSink(AG_EventSink *es)
{
	AG_EventSource *src = AG_GetEventSource();
# ifdef HAVE_KQUEUE
	AG_EventSourceKQUEUE *kq = (AG_EventSourceKQUEUE *)src;
	struct kevent *kev;

	if (kq->nChanges+1 > kq->maxChanges &&
	    GrowKqChangelist(kq, kq->nChanges+1) == -1) {
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
# endif /* HAVE_KQUEUE */

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

# ifdef HAVE_KQUEUE
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
#  ifdef DEBUG_TIMERS
	for (i = 0; i < kq->nChanges; i++) {
		struct kevent *chg = &kq->changes[i];
		Verbose("changes[%d]: f=%d i=%u f=0x%x ff=0x%x u=%p\n",
		    i, (int)chg->filter,
		    (Uint)chg->ident, chg->flags, chg->fflags,
		    chg->udata);
	}
#  endif
	rv = kevent(kq->fd, kq->changes, kq->nChanges, kq->events,
	            AG_KQ_EVBUFSIZE, pTimeo);
	if (rv < 0) {
		if (errno == EINTR) {
			goto restart;
		}
		AG_SetError("kevent(): %s", AG_Strerror(errno));
		return (-1);
	}
	kq->nChanges = 0;

#  ifdef AG_TIMERS
	/* 1. Process timer expirations. */
	AG_LockTiming();
	for (i = 0; i < rv; i++) {
		struct kevent *kev = &kq->events[i];
		const enum ag_event_sink_type esType = GetSinkType(kev->filter);
		Uint32 rvt;
		AG_Timer *to;
		AG_Object *ob;

		if (kev->flags & EV_ERROR) {
#  ifdef DEBUG_TIMERS
			Verbose("kevent (%ld,%d): %s\n", kev->ident, kev->filter,
			    AG_Strerror((int)kev->data));
#  endif
			continue;
		}
		if (esType != AG_SINK_TIMER ||
		    (to = (AG_Timer *)kev->udata) == NULL) {
			continue;
		}
		rvt = to->fn(to, &to->fnEvent);
		if (rvt > 0) {				/* Restart timer */
			struct kevent *kev;
#  ifdef DEBUG_TIMERS
			Verbose("TIMER[%d] resetting t=+%u\n", to->id,
			    (Uint)rvt);
#  endif
			if (kq->nChanges+1 > kq->maxChanges &&
			    GrowKqChangelist(kq, kq->nChanges+1) == -1) {
				AG_UnlockTiming();
				return (-1);
			}
			kev = &kq->changes[kq->nChanges++];
			AG_EV_SET(kev, to->id, EVFILT_TIMER,
			    EV_ADD | EV_ENABLE | EV_ONESHOT, 0, (int)rvt, to);
			to->ival = rvt;
		} else {				/* Expire */
#  ifdef DEBUG_TIMERS
			Verbose("TIMER[%d] expired\n", to->id);
#  endif
			if ((ob = to->obj) == NULL) {
				continue;
			}
			TAILQ_REMOVE(&ob->timers, to, pvt.timers);
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
#  endif /* AG_TIMERS */

	/* 2. Process I/O and other events. */
	for (i = 0; i < rv; i++) {
		struct kevent *kev = &kq->events[i];
		const enum ag_event_sink_type esType = GetSinkType(kev->filter);
		AG_EventSink *es;

		switch (esType) {
		case AG_SINK_READ:
		case AG_SINK_WRITE:
			es = (AG_EventSink *)kev->udata;
			if (es) {
				es->fn(es, &es->fnArgs);
			}
			break;
		case AG_SINK_FSEVENT:
		case AG_SINK_PROCEVENT:
			es = (AG_EventSink *)kev->udata;
			if (es) {
				es->flagsMatched = GetSinkFlags(kev->fflags);
				es->fn(es, &es->fnArgs);
			}
			break;
		default:
			break;
		}
	}
	return (0);
}

#  ifdef AG_TIMERS
/*
 * Add/remove a kqueue(2) based timer.
 */
/*
 * XXX TODO only increment agTimerCount after timer has been actually
 * created (and make GenerateTimerID() pure).
 */
static int
GenerateTimerID(AG_Timer *_Nonnull to)
{
	AG_Object *obOther;
	AG_Timer *toOther;
	int id;

gen_id:
#   ifdef AG_DEBUG
	if (agTimerCount+1 >= (AG_INT_MAX-1))
		AG_FatalError("agTimerCount");
#   endif
	id = (int)++agTimerCount;			/* XXX */
	TAILQ_FOREACH(obOther, &agTimerObjQ, tobjs) {
		TAILQ_FOREACH(toOther, &obOther->timers, pvt.timers) {
			if (toOther == to) { continue; }
			if (toOther->id == id) {
				id++;
				goto gen_id;
			}
		}
	}
	return (id);
}

/* Create a new kernel-based timer with kqueue(2). */
int
AG_AddTimerKQUEUE(AG_Timer *to, Uint32 ival, int newTimer)
{
	AG_EventSourceKQUEUE *kq = (AG_EventSourceKQUEUE *)agEventSource;
	
	if (newTimer) {
		to->id = GenerateTimerID(to);
	}
	if (newTimer || to->ival != ival) {
		struct kevent *kev;
#   ifdef DEBUG_TIMERS
		Verbose("kevent: creating timer ID=%d ival=%d\n", to->id, (int)ival);
#   endif
		if (kq->nChanges+1 > kq->maxChanges &&
		    GrowKqChangelist(kq, kq->nChanges+1) == -1) {
			return (-1);
		}
		kev = &kq->changes[kq->nChanges++];
		AG_EV_SET(kev, to->id, EVFILT_TIMER,
		    EV_ADD | EV_ENABLE | EV_ONESHOT, 0, (int)ival, to);
		to->ival = ival;
	}
	return (0);
}

/* Delete an active kqueue(2) timer. */
void
AG_DelTimerKQUEUE(AG_Timer *to)
{
	AG_EventSourceKQUEUE *kq = (AG_EventSourceKQUEUE *)agEventSource;
	struct kevent *kev;

	if (kq->nChanges+1 > kq->maxChanges &&
	    GrowKqChangelist(kq, kq->nChanges+1) == -1) {
		AG_FatalError(NULL);
	}
	kev = &kq->changes[kq->nChanges++];
	AG_EV_SET(kev, to->id, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
	agTimerCount--;
}
#  endif /* AG_TIMERS */
# endif /* HAVE_KQUEUE */

# ifdef HAVE_TIMERFD
/*
 * Standard event sink using select(2) and fd-based timers,
 * usually available on Linux.
 */
int
AG_EventSinkTIMERFD(void)
{
	fd_set rdFds, wrFds;
	int nFds, rv;
	AG_EventSink *es, *esNext;
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
		default:
			break;
		}
	}
#  ifdef AG_TIMERS
	TAILQ_FOREACH(ob, &agTimerObjQ, tobjs) {
		TAILQ_FOREACH(to, &ob->timers, pvt.timers) {
			FD_SET(to->id, &rdFds);
			if (to->id > nFds) { nFds = to->id; }
		}
	}
#  endif
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

#  ifdef AG_TIMERS
	/* 1. Process timer expirations. */
	AG_LockTiming();
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

			toNext = TAILQ_NEXT(to, pvt.timers);
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
	AG_UnlockTiming();
#  endif /* AG_TIMERS */
	
	/* 2. Process I/O events. */
	for (es = TAILQ_FIRST(&agEventSource->sinks);
	     es != TAILQ_END(&agEventSource->sinks);
	     es = esNext) {
		esNext = TAILQ_NEXT(es, sinks);
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
		default:
			break;
		}
	}
	return (0);
}

#  ifdef AG_TIMERS
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
#  ifdef AG_DEBUG
	if (to->id == -1)
		AG_FatalError("timerfd inconsistency");
#  endif
	close(to->id);
}
#  endif /* AG_TIMERS */
# endif /* HAVE_TIMERFD */

# if defined(HAVE_SELECT) && !defined(AG_THREADS)
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
#  ifdef AG_TIMERS
	AG_Object *ob, *obNext;
	AG_Timer *to, *toNext;
	struct timeval timeo, *pTimeo;
	Uint32 t, tSoonest;
#  endif

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
#  ifdef AG_TIMERS
	if (!TAILQ_EMPTY(&agEventSource->spinners)) {
		timeo.tv_sec = 0;
		timeo.tv_usec = 0;
	} else {
		AG_LockTiming();
		t = AG_GetTicks();
		tSoonest = 0xfffffffe;
		TAILQ_FOREACH(ob, &agTimerObjQ, tobjs) {
			TAILQ_FOREACH(to, &ob->timers, pvt.timers) {
				if ((to->tSched - t) < tSoonest)
					tSoonest = (to->tSched - t);
			}
		}
		timeo.tv_sec = tSoonest/1000;
		timeo.tv_usec = (tSoonest % 1000)*1000;
		AG_UnlockTiming();
	}
#  else /* !AG_TIMERS */
	timeo.tv_sec = 0;
	timeo.tv_usec = 0;
#  endif /* AG_TIMERS */
	rv = select(nFds+1, &rdFds, &wrFds, NULL, &timeo);
	if (rv == -1) {
		if (errno == EINTR) {
			goto restart;
		}
		AG_SetError("select: %s", AG_Strerror(errno));
		return (-1);
	}
#  ifdef AG_TIMERS
	AG_LockTiming();
	/* 1. Process timer expirations. */
	AG_ProcessTimeouts(t);
#  endif
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
#  ifdef AG_TIMERS
	AG_UnlockTiming();
#  endif
	return (0);
}
# endif /* HAVE_SELECT and !AG_THREADS */

# if defined(HAVE_SELECT) && defined(AG_THREADS)
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
#  ifdef AG_TIMERS
	AG_LockTiming();
	/* 1. Process timer expirations. */
	AG_ProcessTimeouts(AG_GetTicks());
#  endif
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
			default:
				break;
			}
		}
	}
#  ifdef AG_TIMERS
	AG_UnlockTiming();
#  endif
	if (TAILQ_EMPTY(&agEventSource->spinners)) {
		AG_Delay(1);
	}
	return (0);
}
# endif /* HAVE_SELECT and AG_THREADS */

/*
 * Fallback "spinning" event sink using a delay loop. This is inefficient,
 * but is the only option on some platforms.
 */
int
AG_EventSinkSPINNER(void)
{
# ifdef AG_TIMERS
	AG_ProcessTimeouts(AG_GetTicks());
# endif
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
	AG_EventSink *es, *esNext;
	
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
		for (es = TAILQ_FIRST(&src->epilogues);
		     es != TAILQ_END(&src->epilogues);
		     es = esNext) {
			esNext = TAILQ_NEXT(es, sinks);
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
#endif /* AG_EVENT_LOOP */
