/*	$Csoft: world.c,v 1.54 2003/01/01 05:18:34 vedge Exp $	*/

/*
 * Copyright (c) 2003 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include "engine.h"

#include "timer.h"
#include "world.h"

#ifdef DEBUG
#define DEBUG_TIMERS	0x01
#define DEBUG_CALLBACKS	0x02

int	timer_debug = DEBUG_TIMERS;
#define engine_debug timer_debug
#endif

static const struct object_ops timer_ops = {
	timer_destroy,
	NULL,
	NULL
};

struct timer *
timer_new(const char *name, Uint32 ival, Uint32 (*callback)(Uint32, void *),
    void *arg)
{
	struct timer *timer;

	timer = emalloc(sizeof(struct timer));
	timer_init(timer, name, ival, callback, arg);

	world_attach(timer);

	return (timer);
}

void
timer_init(struct timer *timer, const char *name, Uint32 ival,
    Uint32 (*callback)(Uint32, void *), void *arg)
{
	char *tname;

	Asprintf(&tname, "t-%s", name);
	object_init(&timer->obj, "timer", tname, NULL, 0, &timer_ops);
	free(tname);

	timer->ival = ival;
	timer->callback = callback;
	timer->arg = arg;
	pthread_mutex_init(&timer->lock, NULL);
}

/* Start a timer. */
void
timer_start(struct timer *timer)
{
	pthread_mutex_lock(&timer->lock);
	switch (timer->state) {
	case TIMER_RUNNING:
		break;
	case TIMER_STOP:
	case TIMER_STOPPED:
		timer->id = SDL_AddTimer(timer->ival, timer->callback,
		    timer->arg);
		if (timer->id == NULL) {
		 	fatal("SDL_AddTimer: %s\n", SDL_GetError());
		}
		timer->state = TIMER_RUNNING;
		break;
	}
	pthread_mutex_unlock(&timer->lock);
}

/* Schedule for the timer to be stopped. */
void
timer_stop(struct timer *timer)
{
	pthread_mutex_lock(&timer->lock);
	timer->state = TIMER_STOP;
	pthread_mutex_unlock(&timer->lock);
}

void
timer_destroy(void *p)
{
	struct timer *timer = p;

	pthread_mutex_lock(&timer->lock);
	timer_stop(timer);
	pthread_mutex_unlock(&timer->lock);
	pthread_mutex_destroy(&timer->lock);
}

