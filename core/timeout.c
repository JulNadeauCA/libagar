/*
 * Copyright (c) 2004-2020 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>
#ifdef AG_TIMERS

struct ag_objectq agTimerObjQ = TAILQ_HEAD_INITIALIZER(agTimerObjQ);
Uint agTimerCount = 0;
AG_Object agTimerMgr;
#ifdef AG_THREADS
AG_Mutex agTimerLock;
#endif

void
AG_InitTimers(void)
{
	AG_MutexInitRecursive(&agTimerLock);
	AG_ObjectInit(&agTimerMgr, NULL);
	agTimerMgr.flags |= AG_OBJECT_STATIC;
}

void
AG_DestroyTimers(void)
{
	AG_ObjectDestroy(&agTimerMgr);
	AG_MutexDestroy(&agTimerLock);
}

/*
 * Create a new anonymous auto-allocated timer and schedule the
 * execution of a callback routine fn in ival ticks.
 */
AG_Timer *
AG_AddTimerAuto(void *p, Uint32 ival, AG_TimerFn fn, const char *fmt, ...)
{
	AG_Object *ob = (p != NULL) ? OBJECT(p) : &agTimerMgr;
	AG_Timer *to;
	AG_Event *ev;

	if ((to = TryMalloc(sizeof(AG_Timer))) == NULL) {
		return (NULL);
	}
	AG_InitTimer(to, "auto", AG_TIMER_AUTO_FREE);
	AG_LockTimers(ob);
	if (AG_AddTimer(ob, to, ival, fn, NULL) == -1) {
		goto fail;
	}
	ev = &to->fnEvent;
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(ev, fmt, ap);
		va_end(ap);
	}
	ev->argc0 = ev->argc;
	AG_UnlockTimers(ob);
	return (to);
fail:
	AG_UnlockTimers(ob);
	free(to);
	return (NULL);
}

/*
 * Attach a timer to an object (or &agTimerMgr if object=NULL) and schedule
 * the execution of a callback routine fn in ival ticks.
 *
 * The AG_Timer structure should have been initialized by AG_InitTimer().
 * If the referenced timer is already running then AG_AddTimer() will update
 * the interval / callback function & arguments of the existing timer entry.
 */
int
AG_AddTimer(void *p, AG_Timer *to, Uint32 ival, AG_TimerFn fn,
    const char *fmt, ...)
{
	AG_EventSource *src = AG_GetEventSource();
	AG_Object *ob = (p != NULL) ? OBJECT(p) : &agTimerMgr;
	AG_Timer *toOther;
	int newTimer = 0;
	AG_Event *ev;
	
	AG_LockTimers(ob);

	if (src->caps[AG_SINK_TIMER]) {		/* Unordered list */
		if (to->obj == NULL) {
			if (TAILQ_EMPTY(&ob->timers)) {
				TAILQ_INSERT_TAIL(&agTimerObjQ, ob, tobjs);
			}
			TAILQ_INSERT_TAIL(&ob->timers, to, pvt.timers);
			newTimer = 1;
			to->obj = ob;
			to->tSched = 0;
		} else if (to->obj != ob) {
			AG_FatalError("to->obj != ob");
		}
	} else {				/* Ordered timing wheel */
		if (to->obj == NULL) {
			newTimer = 1;
			to->obj = ob;
		} else if (to->obj != ob) {
			AG_FatalError("to->obj != ob");
		}
		if (TAILQ_EMPTY(&ob->timers)) {
			TAILQ_INSERT_TAIL(&agTimerObjQ, ob, tobjs);
		}
		to->tSched = AG_GetTicks()+ival;
reinsert:
		TAILQ_FOREACH(toOther, &ob->timers, pvt.timers) {
			if (toOther == to) {
				newTimer = 0;
				TAILQ_REMOVE(&ob->timers, to, pvt.timers);
				goto reinsert;
			}
			if (to->tSched < toOther->tSched) {
				TAILQ_INSERT_BEFORE(toOther, to, pvt.timers);
				break;
			}
		}
		if (toOther == TAILQ_END(&ob->timers)) {
			TAILQ_INSERT_TAIL(&ob->timers, to, pvt.timers);
		}
		to->ival = ival;
		to->id = 0;				/* Not needed */
	}

	to->fn = fn;
	ev = &to->fnEvent;
	AG_EventInit(ev);
	ev->argv[0].data.p = ob;
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(ev, fmt, ap);
		va_end(ap);
	}
	ev->argc0 = ev->argc;

	if (src->addTimerFn != NULL &&
	    src->addTimerFn(to, ival, newTimer) == -1) {
		goto fail;
	}
	AG_UnlockTimers(ob);
	return (0);
fail:
	to->obj = NULL;
	TAILQ_REMOVE(&ob->timers, to, pvt.timers);
	if (TAILQ_EMPTY(&ob->timers)) { TAILQ_REMOVE(&agTimerObjQ, ob, tobjs); }
	AG_UnlockTimers(ob);
	return (-1);
}

/* Initialize a timer structure */
void
AG_InitTimer(AG_Timer *to, const char *name, Uint flags)
{
	if (name == NULL) {
		to->name[0] = '\0';
	} else {
#ifdef AG_DEBUG
		if (Strlcpy(to->name, name, sizeof(to->name)) >= sizeof(to->name))
			Verbose("Truncated timer name: \"%s\"\n", to->name);
#else
		Strlcpy(to->name, name, sizeof(to->name));
#endif
	}
	to->id = -1;
	to->obj = NULL;
	to->flags = flags;
	to->ival = 0;
	to->tSched = 0;
	to->fn = NULL;
}

/*
 * Change the interval of a timer. The timer must be running.
 * This is called whenever a timer callback returns a new interval.
 */
int
AG_ResetTimer(void *p, AG_Timer *to, Uint32 ival)
{
	AG_EventSource *src = AG_GetEventSource();
	AG_Object *ob = (p != NULL) ? OBJECT(p) : &agTimerMgr;
	AG_Timer *toOther;
	int rv = 0;
	
	AG_LockTimers(ob);
	if (src->addTimerFn != NULL &&
	    src->addTimerFn(to, ival, 0) == -1) {
		rv = -1;
		goto out;
	}
	if (!src->caps[AG_SINK_TIMER]) {	/* Ordered timing wheel */
		to->tSched = AG_GetTicks()+ival;
		TAILQ_REMOVE(&ob->timers, to, pvt.timers);
		TAILQ_FOREACH(toOther, &ob->timers, pvt.timers) {
			if (to->tSched < toOther->tSched) {
				TAILQ_INSERT_BEFORE(toOther, to, pvt.timers);
				break;
			}
		}
		if (toOther == TAILQ_END(&ob->timers))
			TAILQ_INSERT_TAIL(&ob->timers, to, pvt.timers);
	}
	to->ival = ival;
out:
	AG_UnlockTimers(ob);
	return (rv);
}

/* Cancel the given timeout if it is scheduled for execution. */
void
AG_DelTimer(void *p, AG_Timer *to)
{
	AG_EventSource *src = AG_GetEventSource();
	AG_Object *ob = (p != NULL) ? OBJECT(p) : &agTimerMgr;
	AG_Timer *toOther;

	AG_LockTimers(ob);
	
	TAILQ_FOREACH(toOther, &ob->timers, pvt.timers) {
		if (toOther == to)
			break;
	}
	if (toOther == NULL) 		/* Timer is not active */
		goto out;

	if (src->delTimerFn != NULL) {
		src->delTimerFn(to);
	}
	to->id = -1;
	to->obj = NULL;

	TAILQ_REMOVE(&ob->timers, to, pvt.timers);
	if (TAILQ_EMPTY(&ob->timers))
		TAILQ_REMOVE(&agTimerObjQ, ob, tobjs);

	if (to->flags & AG_TIMER_AUTO_FREE)
		free(to);
out:
	AG_UnlockTimers(ob);
}

/* Cancel all running timers tied to the given object. */
void
AG_DelTimers(void *obj)
{
	AG_Object *ob = obj;
	AG_Timer *to, *toNext;
	
	AG_LockTiming();
	for (to = TAILQ_FIRST(&ob->timers);
	     to != TAILQ_END(&ob->timers);
	     to = toNext) {
		toNext = TAILQ_NEXT(to, pvt.timers);
		AG_DelTimer(ob, to);
	}
	TAILQ_INIT(&ob->timers);
	AG_UnlockTiming();
}

/*
 * Evaluate whether a timer is running.
 * The caller should use AG_LockTimers().
 */
int
AG_TimerIsRunning(void *p, AG_Timer *to)
{
	AG_Object *ob = (p != NULL) ? OBJECT(p) : &agTimerMgr;
	AG_Timer *toRunning;

	TAILQ_FOREACH(toRunning, &ob->timers, pvt.timers) {
		if (toRunning == to)
			break;
	}
	return (toRunning != NULL);
}

/* Invoke a timer callback routine artificially. */
Uint32
AG_ExecTimer(AG_Timer *to)
{
	Uint32 rv;

	AG_LockTiming();
	rv = to->fn(to, &to->fnEvent);
	AG_UnlockTiming();
	return (rv);
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
			toNext = TAILQ_NEXT(to, pvt.timers);

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
#endif /* AG_TIMERS */
