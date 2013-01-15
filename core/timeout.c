/*
 * Copyright (c) 2004-2012 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Timer interface.
 */

#include "core.h"

#include <config/have_kqueue.h>
#include <config/have_cocoa.h>
#include <config/have_timerfd.h>

#if defined(HAVE_KQUEUE) && !defined(HAVE_COCOA)
# include <sys/types.h>
# include <sys/event.h>
# include <sys/time.h>
# include <errno.h>
#elif defined(HAVE_TIMERFD)
# include <sys/timerfd.h>
# include <errno.h>
#endif

struct ag_objectq agTimerObjQ = TAILQ_HEAD_INITIALIZER(agTimerObjQ);
Uint              agTimerCount = 0;
AG_Object         agTimerMgr;
AG_Mutex          agTimerLock;

void
AG_InitTimers(void)
{
	AG_MutexInitRecursive(&agTimerLock);
	AG_ObjectInitStatic(&agTimerMgr, NULL);
}

void
AG_DestroyTimers(void)
{
	AG_ObjectDestroy(&agTimerMgr);
	AG_MutexDestroy(&agTimerLock);
}

/* Schedule a timeout to occur in ival ticks */
int
AG_AddTimer(void *p, AG_Timer *to, Uint32 ival, AG_TimerFn fn,
    const char *fmt, ...)
{
	AG_Object *ob = (p != NULL) ? p : &agTimerMgr;
	AG_Timer *toOther;
	int newTimer = 0;
	AG_Event *ev;
	
	AG_LockTimers(ob);

	if (TAILQ_EMPTY(&ob->timers))
		TAILQ_INSERT_TAIL(&agTimerObjQ, ob, tobjs);

#if defined(HAVE_KQUEUE) && !defined(HAVE_COCOA)
	/* Check if the timer is active. Order is not important. */
	if (agKqueue != -1) {
		TAILQ_FOREACH(toOther, &ob->timers, timers) {
			if (toOther == to)
				break;
		}
		if (toOther == NULL) {
			TAILQ_INSERT_TAIL(&ob->timers, to, timers);
			newTimer = 1;
			to->tSched = 0;
		}
	} else
#elif defined(HAVE_TIMERFD)
	/* Check if the timer is active. Order is not important. */
	if (!agSoftTimers) {
		TAILQ_FOREACH(toOther, &ob->timers, timers) {
			if (toOther == to)
				break;
		}
		if (toOther == NULL) {
			TAILQ_INSERT_TAIL(&ob->timers, to, timers);
			newTimer = 1;
			to->tSched = 0;
		}
	} else
#endif
	/* Check if the timer is active. Maintain timing wheel order. */
	{
		to->tSched = AG_GetTicks()+ival;
		newTimer = 1;
rescan:
		TAILQ_FOREACH(toOther, &ob->timers, timers) {
			if (toOther == to) {
				newTimer = 0;
				TAILQ_REMOVE(&ob->timers, to, timers);
				goto rescan;
			}
			if (to->tSched < toOther->tSched) {
				TAILQ_INSERT_BEFORE(toOther, to, timers);
				break;
			}
		}
		if (toOther == TAILQ_END(&ob->timers))
			TAILQ_INSERT_TAIL(&ob->timers, to, timers);
	}

	to->obj = ob;
	to->ival = ival;
	to->fn = fn;
	ev = &to->fnEvent;
	AG_EventInit(ev);
	ev->argv[0].data.p = ob;
	AG_EVENT_GET_ARGS(ev, fmt);
	ev->argc0 = ev->argc;

#if defined(HAVE_KQUEUE) && !defined(HAVE_COCOA)
	/* Create a kqueue timer with a generated unique ID. */
	/* TODO queue the changelist, faster collision test */
	if (agKqueue != -1) {
		AG_Object *obOther;
		struct kevent kev;
	
		if (newTimer) {
# ifdef AG_DEBUG
			if (agTimerCount+1 >= AG_INT_MAX)
				AG_FatalError("agTimerCount");
# endif
			to->id = (int)++agTimerCount;
gen_id:
			TAILQ_FOREACH(obOther, &agTimerObjQ, tobjs) {
				TAILQ_FOREACH(toOther, &obOther->timers, timers) {
					if (toOther == to) { continue; }
					if (toOther->id == to->id) {
						to->id++;
						goto gen_id;
					}
				}
			}
		}
		EV_SET(&kev, to->id, EVFILT_TIMER, EV_ADD|EV_ENABLE, 0,
		    (int)ival, to);
		if (kevent(agKqueue, &kev, 1, NULL, 0, NULL) == -1) {
			AG_SetError("kevent: %s", AG_Strerror(errno));
			goto fail;
		}
	} else
#elif defined(HAVE_TIMERFD)
	/* Create a timerfd. Store the file descriptor as ID. */
	if (!agSoftTimers) {
		struct itimerspec its;

		if ((to->id = timerfd_create(CLOCK_MONOTONIC, 0)) == -1) {
			AG_SetError("timerfd_create: %s", AG_Strerror(errno));
			goto fail;
		}
		its.it_value.tv_sec = ival/1000;
		its.it_value.tv_nsec = (ival % 1000)*1000000L;
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0L;
		if (timerfd_settime(to->id, 0, &its, NULL) == -1) {
			close(to->id);
			AG_SetError("timerfd_settime: %s", AG_Strerror(errno));
			goto fail;
		}
	} else
#endif
	/* Create a soft timer. This method does not require IDs. */
	{
		to->id = 0;
	}

	AG_UnlockTimers(ob);
	return (0);
#if (defined(HAVE_KQUEUE) && !defined(HAVE_COCOA)) || defined(HAVE_TIMERFD)
fail:
	to->obj = NULL;
	TAILQ_REMOVE(&ob->timers, to, timers);
	if (TAILQ_EMPTY(&ob->timers)) { TAILQ_REMOVE(&agTimerObjQ, ob, tobjs); }
	AG_UnlockTimers(ob);
	return (-1);
#endif
}

/* Initialize a timer structure */
void
AG_InitTimer(AG_Timer *to, const char *name, Uint flags)
{
	if (name == NULL) {
		to->name[0] = '\0';
	} else {
		Strlcpy(to->name, name, sizeof(to->name));
	}
	to->id = -1;
	to->obj = NULL;
	to->flags = flags;
	to->ival = 0;
	to->tSched = 0;
	to->fn = NULL;
}

/*
 * Variant of AG_AddTimer() for an auto-allocated, anonymous timer. The
 * timer structure will be freed upon cancellation.
 *
 * The returned pointer is only safe to access as long as AG_LockTimers()
 * is in effect.
 */
AG_Timer *
AG_AddTimerAuto(void *p, Uint32 ival, AG_TimerFn fn, const char *fmt, ...)
{
	AG_Object *ob = (p != NULL) ? p : &agTimerMgr;
	AG_Timer *to;
	AG_Event *ev;

	if ((to = TryMalloc(sizeof(AG_Timer))) == NULL) {
		return (NULL);
	}
	AG_LockTimers(ob);
	if (AG_AddTimer(ob, to, ival, fn, NULL) == -1) {
		goto fail;
	}
	to->flags |= AG_TIMER_AUTO_FREE;
	ev = &to->fnEvent;
	AG_EVENT_GET_ARGS(ev, fmt);
	ev->argc0 = ev->argc;
	AG_UnlockTimers(ob);
	return (to);
fail:
	AG_UnlockTimers(ob);
	free(to);
	return (NULL);
}

/*
 * Change the interval of a timer. The timer must be running.
 * This is called whenever a timer callback returns a new interval.
 */
int
AG_ResetTimer(void *p, AG_Timer *to, Uint32 ival)
{
	AG_Object *ob = (p != NULL) ? p : &agTimerMgr;
	
	AG_LockTimers(ob);

#if defined(HAVE_KQUEUE) && !defined(HAVE_COCOA)
	if (agKqueue != -1) {
		struct kevent kchg;

		/* TODO: queue changelist */
		EV_SET(&kchg, to->id, EVFILT_TIMER, EV_ADD|EV_ENABLE, 0,
		    (int)ival, to);
		if (kevent(agKqueue, &kchg, 1, NULL, 0, NULL) == -1) {
			AG_SetError("kevent: %s", AG_Strerror(errno));
			goto fail;
		}
	} else
#elif defined(HAVE_TIMERFD)
	if (!agSoftTimers) {
		struct itimerspec its;

		its.it_value.tv_sec = ival/1000;
		its.it_value.tv_nsec = (ival % 1000)*1000000L;
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0L;
		if (timerfd_settime(to->id, 0, &its, NULL) == -1) {
			AG_SetError("timerfd_settime: %s", AG_Strerror(errno));
			goto fail;
		}
	} else
#endif
	{
		AG_Timer *toOther;

		to->tSched = AG_GetTicks()+ival;
		TAILQ_REMOVE(&ob->timers, to, timers);
		TAILQ_FOREACH(toOther, &ob->timers, timers) {
			if (to->tSched < toOther->tSched) {
				TAILQ_INSERT_BEFORE(toOther, to, timers);
				break;
			}
		}
		if (toOther == TAILQ_END(&ob->timers))
			TAILQ_INSERT_TAIL(&ob->timers, to, timers);
	}
	AG_UnlockTimers(ob);
	to->ival = ival;
	return (0);
fail:
	AG_UnlockTimers(ob);
	return (-1);
}

/* Cancel the given timeout if it is scheduled for execution. */
void
AG_DelTimer(void *p, AG_Timer *to)
{
	AG_Object *ob = (p != NULL) ? p : &agTimerMgr;
	AG_Timer *toOther;

	AG_LockTimers(ob);
	
	TAILQ_FOREACH(toOther, &ob->timers, timers) {
		if (toOther == to)
			break;
	}
	if (toOther == NULL) 		/* Timer is not active */
		goto out;

#if defined(HAVE_KQUEUE) && !defined(HAVE_COCOA)
	/* Remove our kqueue event filter. TODO queue the changelist */
	if (agKqueue != -1) {
		struct kevent kev;
	
		EV_SET(&kev, to->id, EVFILT_TIMER, EV_DELETE, 0, 0, 0);
		if (kevent(agKqueue, &kev, 1, NULL, 0, NULL) == -1) {
			AG_FatalError("kevent: %s", AG_Strerror(errno));
		}
		to->id = -1;
		agTimerCount--;
	} else
#elif defined(HAVE_TIMERFD)
	if (!agSoftTimers) {
# ifdef AG_DEBUG
		if (to->id == -1) { AG_FatalError("timerfd inconsistency"); }
# endif
		close(to->id);
		to->id = -1;
	} else
#endif
	{
		to->id = 0;
	}

	to->obj = NULL;
	TAILQ_REMOVE(&ob->timers, to, timers);
	if (TAILQ_EMPTY(&ob->timers)) { TAILQ_REMOVE(&agTimerObjQ, ob, tobjs); }
	if (to->flags & AG_TIMER_AUTO_FREE)
		free(to);
out:
	AG_UnlockTimers(ob);
}

/*
 * Evaluate whether a timer is running.
 * The caller should use AG_LockTimers().
 */
int
AG_TimerIsRunning(void *p, AG_Timer *to)
{
	AG_Object *ob = (p != NULL) ? p : &agTimerMgr;
	AG_Timer *toRunning;

	TAILQ_FOREACH(toRunning, &ob->timers, timers) {
		if (toRunning == to)
			break;
	}
	return (toRunning != NULL);
}

/*
 * Block the calling thread until the given timer expires (and is not
 * immediately restarted), or the given delay (in ticks) is exceeded.
 * 
 * XXX TODO use a condition variable instead of a delay loop.
 */
int
AG_TimerWait(void *p, AG_Timer *to, Uint32 timeout)
{
	AG_Object *ob = (p != NULL) ? p : &agTimerMgr;
	Uint32 elapsed = 0;

	for (;;) {
		if (timeout > 0 && ++elapsed >= timeout) {
			return (-1);
		}
		if (!AG_TimerIsRunning(ob, to)) {
			break;
		}
		AG_Delay(1);
	}
	return (0);
}

/*
 * Execute the callback routines of expired timers using AG_GetTicks()
 * as a time source. This is used on platforms where system timers are not
 * available and delay loops are the only option.
 *
 * Applications calling this routine explicitely must pass AG_SOFT_TIMERS to
 * AG_InitCore().
 */
void
AG_ProcessTimeouts(Uint32 t)
{
	AG_Timer *to, *toNext;
	AG_Object *ob, *obNext;
	Uint32 rv;

	AG_LockTiming();

	for (ob = TAILQ_FIRST(&agTimerObjQ);
	     ob != TAILQ_END(&agTimerObjQ);
	     ob = obNext) {
		obNext = TAILQ_NEXT(ob, tobjs);
		AG_ObjectLock(ob);
rescan:
		for (to = TAILQ_FIRST(&ob->timers);
		     to != TAILQ_END(&ob->timers);
		     to = toNext) {
			toNext = TAILQ_NEXT(to, timers);

			if ((int)(to->tSched - t) > 0) {
				continue;
			}
			rv = to->fn(to, &to->fnEvent);
			if (rv > 0) {				/* Restart */
				(void)AG_ResetTimer(ob, to, rv);
				goto rescan;
			} else {				/* Cancel */
				AG_DelTimer(ob, to);
			}
		}
		AG_ObjectUnlock(ob);
	}
	AG_UnlockTiming();
}

#ifdef AG_LEGACY
static Uint32
LegacyTimerCallback(AG_Timer *to, AG_Event *event)
{
	return to->fnLegacy(to->obj, to->ival, to->argLegacy);
}
void
AG_SetTimeout(AG_Timeout *to, Uint32 (*fn)(void *, Uint32, void *), void *arg,
    Uint flags)
{
	to->fnLegacy = fn;
	to->argLegacy = arg;
}
void
AG_ScheduleTimeout(void *p, AG_Timeout *to, Uint32 ival)
{
	AG_Object *ob = (p != NULL) ? p : &agTimerMgr;

	if (AG_AddTimer(ob, to, ival, LegacyTimerCallback, NULL) == -1)
		AG_FatalError("ScheduleTimeout: %s", AG_GetError());
}
#endif /* AG_LEGACY */
