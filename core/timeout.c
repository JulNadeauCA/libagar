/*
 * Copyright (c) 2004-2009 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Implementation of simple timers. Timers are usually associated with an
 * AG_Object for management purposes.
 */

#include <core/core.h>

struct ag_objectq agTimeoutObjQ = TAILQ_HEAD_INITIALIZER(agTimeoutObjQ);
AG_Object agTimeoutMgr;
#ifdef AG_THREADS
AG_Mutex agTimingLock;
#endif

void
AG_InitTimeouts(void)
{
	AG_MutexInitRecursive(&agTimingLock);
	AG_ObjectInitStatic(&agTimeoutMgr, NULL);
}

void
AG_DestroyTimeouts(void)
{
	AG_ObjectDestroy(&agTimeoutMgr);
	AG_MutexDestroy(&agTimingLock);
}

/* Initialize a timeout structure. */
void
AG_SetTimeout(AG_Timeout *to, Uint32 (*fn)(void *, Uint32, void *), void *arg,
    Uint flags)
{
	to->fn = fn;
	to->arg = arg;
	to->ticks = 0;
	to->flags = flags;
}

/* Schedule (or reschedule) the timeout to occur in dt ticks. */
void
AG_ScheduleTimeout(void *p, AG_Timeout *to, Uint32 dt)
{
	AG_Object *ob = (p != NULL) ? p : &agTimeoutMgr;
	AG_Timeout *toAfter;
	Uint32 t = AG_GetTicks()+dt;
	int listEmpty;

	AG_ObjectLock(ob);
	listEmpty = TAILQ_EMPTY(&ob->timeouts);

	if (to->flags & AG_TIMEOUT_ACTIVE) {			/* Reschedule */
		TAILQ_REMOVE(&ob->timeouts, to, timeouts);
	}
	TAILQ_FOREACH(toAfter, &ob->timeouts, timeouts) {
		if (t < toAfter->ticks) {
			TAILQ_INSERT_BEFORE(toAfter, to, timeouts);
			break;
		}
	}
	if (toAfter == TAILQ_END(&ob->timeouts)) {
		TAILQ_INSERT_TAIL(&ob->timeouts, to, timeouts);
	}

	to->ticks = t;
	to->ival = dt;
	to->flags |= AG_TIMEOUT_ACTIVE;

	if (listEmpty) {
		AG_LockTiming();
		TAILQ_INSERT_TAIL(&agTimeoutObjQ, ob, tobjs);
		AG_UnlockTiming();
	}
	AG_ObjectUnlock(ob);
}

/* Cancel the given timeout if it is scheduled for execution. */
void
AG_DelTimeout(void *p, AG_Timeout *to)
{
	AG_Object *ob = (p != NULL) ? p : &agTimeoutMgr;
	
	AG_LockTimeouts(ob);
	if (to->flags & AG_TIMEOUT_ACTIVE) {
		to->flags &= ~(AG_TIMEOUT_ACTIVE);
		TAILQ_REMOVE(&ob->timeouts, to, timeouts);
		if (TAILQ_EMPTY(&ob->timeouts)) {
			TAILQ_REMOVE(&agTimeoutObjQ, ob, tobjs);
		}
	}
	AG_UnlockTimeouts(ob);
}

/*
 * Block the calling thread until the given timeout executes (and is not
 * immediately rescheduled), or the given delay (given in milliseconds)
 * is exceeded.
 */
int
AG_TimeoutWait(void *p, AG_Timeout *to, Uint32 timeout)
{
	AG_Object *ob = (p != NULL) ? p : &agTimeoutMgr;
	Uint32 elapsed = 0;
	
wait:
	if (timeout > 0 && ++elapsed >= timeout) {
		AG_SetError(_("Timeout after %u ticks"), (Uint)elapsed);
		return (-1);
	}
	AG_Delay(1);
	AG_LockTimeouts(ob);
	if (to->flags & AG_TIMEOUT_ACTIVE) {
		AG_UnlockTimeouts(ob);
		goto wait;
	}
	AG_UnlockTimeouts(ob);
	return (0);
}

void
AG_ProcessTimeouts(Uint32 t)
{
	AG_Timeout *to, *toNext;
	AG_Object *ob, *obNext;
	AG_Timeout *toAfter;
	Uint32 rv;

	AG_LockTiming();
	for (ob = TAILQ_FIRST(&agTimeoutObjQ);
	     ob != TAILQ_END(&agTimeoutObjQ);
	     ob = obNext) {
		obNext = TAILQ_NEXT(ob, tobjs);
		AG_ObjectLock(ob);
rescan:
		for (to = TAILQ_FIRST(&ob->timeouts);
		     to != TAILQ_END(&ob->timeouts);
		     to = toNext) {
			toNext = TAILQ_NEXT(to, timeouts);

			if ((int)(to->ticks - t) > 0) {
				continue;
			}
			rv = to->fn(ob, to->ival, to->arg);
			if (rv > 0) {
				/* Reschedule timer */
				to->ticks = AG_GetTicks()+rv;
				to->ival = rv;
				TAILQ_REMOVE(&ob->timeouts, to, timeouts);
				TAILQ_FOREACH(toAfter, &ob->timeouts, timeouts) {
					if (t < toAfter->ticks) {
						TAILQ_INSERT_BEFORE(toAfter, to,
						    timeouts);
						break;
					}
				}
				if (toAfter == TAILQ_END(&ob->timeouts)) {
					TAILQ_INSERT_TAIL(&ob->timeouts, to,
					    timeouts);
				}
				goto rescan;
			} else {
				/* Stop timer */
				TAILQ_REMOVE(&ob->timeouts, to, timeouts);
				if (TAILQ_EMPTY(&ob->timeouts)) {
					TAILQ_REMOVE(&agTimeoutObjQ, ob, tobjs);
				}
				to->flags &= ~(AG_TIMEOUT_ACTIVE);
			}
		}
		AG_ObjectUnlock(ob);
	}
	AG_UnlockTiming();
	AG_Delay(1);
}
