/*
 * Copyright (c) 2004-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * AG_Object for easier management (e.g., automatic cancellation).
 */

#include <core/core.h>

#include "timeout.h"

struct ag_objectq agTimeoutObjQ = TAILQ_HEAD_INITIALIZER(agTimeoutObjQ);

/* Initialize a timeout structure. */
void
AG_SetTimeout(AG_Timeout *to, Uint32 (*fn)(void *, Uint32, void *), void *arg,
    Uint flags)
{
	to->fn = fn;
	to->arg = arg;
	to->ticks = 0;
	to->running = 0;
	to->flags = flags;
}

/* Schedule the timeout to occur in dt ticks. */
/* XXX O(n), use a timing wheel instead */
void
AG_ScheduleTimeout(void *p, AG_Timeout *to, Uint32 dt, int replace)
{
	AG_Object *ob = p;
	AG_Timeout *to2;
	Uint32 t = SDL_GetTicks()+dt;
	int was_empty;

	if (ob == NULL)
		ob = agWorld;

	AG_MutexLock(&ob->lock);
	was_empty = CIRCLEQ_EMPTY(&ob->timeouts);
	if (replace) {
		CIRCLEQ_FOREACH(to2, &ob->timeouts, timeouts) {
			if (to == to2) {
				CIRCLEQ_REMOVE(&ob->timeouts, to, timeouts);
				break;
			}
		}
	}
	CIRCLEQ_FOREACH(to2, &ob->timeouts, timeouts) {
		if (dt < to2->ticks) {
			CIRCLEQ_INSERT_BEFORE(&ob->timeouts, to2, to, timeouts);
			break;
		}
	}
	if (to2 == CIRCLEQ_END(&ob->timeouts)) {
		CIRCLEQ_INSERT_HEAD(&ob->timeouts, to, timeouts);
	}
	to->ticks = t;
	to->ival = dt;
	to->flags = 0;
	if (was_empty) {
		AG_LockTiming();
		TAILQ_INSERT_TAIL(&agTimeoutObjQ, ob, tobjs);
		AG_UnlockTiming();
	}
	AG_MutexUnlock(&ob->lock);
}

/*
 * Return 1 if the given timeout is scheduled.
 * The object and timeout queue must be locked.
 */
int
AG_TimeoutIsScheduled(void *p, AG_Timeout *to)
{
	AG_Object *ob = p;
	AG_Object *tob;
	AG_Timeout *oto;

	if (ob == NULL)
		ob = agWorld;

	TAILQ_FOREACH(tob, &agTimeoutObjQ, tobjs) {
		CIRCLEQ_FOREACH(oto, &tob->timeouts, timeouts) {
			if (oto == to)
				return (1);
		}
	}
	return (0);
}

/* Cancel the given timeout if it is scheduled for execution. */
void
AG_DelTimeout(void *p, AG_Timeout *to)
{
	AG_Object *ob = p;
	AG_Timeout *oto;
	
	if (ob == NULL)
		ob = agWorld;
	
	AG_LockTimeouts(ob);
	CIRCLEQ_FOREACH(oto, &ob->timeouts, timeouts) {
		if (oto == to) {
			CIRCLEQ_REMOVE(&ob->timeouts, to, timeouts);
			if (CIRCLEQ_EMPTY(&ob->timeouts)) {
				TAILQ_REMOVE(&agTimeoutObjQ, ob, tobjs);
			}
			break;
		}
	}
	if (oto == NULL) {
		Debug(ob, "DelTimeout: No such timeout %p\n", to);
	}
	AG_UnlockTimeouts(ob);
}

/*
 * Block the calling thread until the given timeout executes, or the
 * given timeout (given in SDL_Delay() ticks) is exceeded.
 */
int
AG_TimeoutWait(void *p, AG_Timeout *to, Uint32 timeout)
{
	AG_Object *ob = p;
	AG_Timeout *oto;
	Uint32 elapsed = 0;
	
	if (ob == NULL)
		ob = agWorld;
wait:
	if (timeout > 0 && ++elapsed >= timeout) {
		AG_SetError(_("Timeout after %u ticks"), (Uint)elapsed);
		return (-1);
	}
	SDL_Delay(1);
	AG_LockTimeouts(ob);
	CIRCLEQ_FOREACH(oto, &ob->timeouts, timeouts) {
		if (oto == to) {
			AG_UnlockTimeouts(ob);
			goto wait;
		}
	}
	AG_UnlockTimeouts(ob);
	return (0);
}

void
AG_ProcessTimeout(Uint32 t)
{
	AG_Timeout *to;
	AG_Object *ob;
	Uint32 rv;

	AG_LockTiming();
	TAILQ_FOREACH(ob, &agTimeoutObjQ, tobjs) {
		AG_ObjectLock(ob);
		/*
		 * Loop comparing the timestamp of the first element with
		 * the current time for as long as the timestamp is in
		 * the past.
		 */
pop:
		if (!CIRCLEQ_EMPTY(&ob->timeouts)) {
			to = CIRCLEQ_FIRST(&ob->timeouts);
			if ((int)(to->ticks - t) <= 0) {
				CIRCLEQ_REMOVE(&ob->timeouts, to, timeouts);
				if (CIRCLEQ_EMPTY(&ob->timeouts)) {
					TAILQ_REMOVE(&agTimeoutObjQ, ob, tobjs);
				}
				to->running++;
				rv = to->fn(ob, to->ival, to->arg);
				to->running = 0;
				if (rv > 0) {
					to->ival = rv;
					AG_AddTimeout(ob, to, rv);
				}
				goto pop;
			}
		}
		AG_ObjectUnlock(ob);
	}
	AG_UnlockTiming();
	SDL_Delay(1);
}

void
AG_LockTimeouts(void *p)
{
	AG_Object *ob = p;

	if (ob == NULL) {
		ob = agWorld;
	}
	AG_ObjectLock(ob);
	AG_LockTiming();
}

void
AG_UnlockTimeouts(void *p)
{
	AG_Object *ob = p;

	if (ob == NULL) {
		ob = agWorld;
	}
	AG_UnlockTiming();
	AG_ObjectUnlock(ob);
}
