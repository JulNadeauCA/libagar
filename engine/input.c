/*	$Csoft: input.c,v 1.51 2004/01/03 04:25:04 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004 CubeSoft Communications, Inc.
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

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/physics.h>
#include <engine/input.h>

#include <string.h>

struct input_devq input_devs;
pthread_mutex_t	  input_lock = PTHREAD_MUTEX_INITIALIZER;

/* Initialize and attach an input device. */
void
input_register(void *p, enum input_type type, const char *name,
    const struct input_driver *drv)
{
	struct input *in = p;

	strlcpy(in->name, name, sizeof(in->name));
	in->type = type;
	in->drv = drv;
	in->pos = NULL;

	pthread_mutex_lock(&input_lock);
	SLIST_INSERT_HEAD(&input_devs, in, inputs);
	pthread_mutex_unlock(&input_lock);

	dprintf("registered %s (%s)\n", in->name, in->drv->name);
}

/* Detach and free an input device structure. */
void
input_deregister(void *p)
{
	struct input *in = p;

	if (in->drv->in_close != NULL)
		in->drv->in_close(in);

	pthread_mutex_lock(&input_lock);
	SLIST_REMOVE(&input_devs, in, input, inputs);
	pthread_mutex_unlock(&input_lock);

	free(in);
}

/* Destroy all attached input device structures. */
void
input_destroy(void)
{
	struct input *in, *nin;

	pthread_mutex_lock(&input_lock);
	for (in = SLIST_FIRST(&input_devs);
	     in != SLIST_END(&input_devs);
	     in = nin) {
		nin = SLIST_NEXT(in, inputs);

		if (in->drv->in_close != NULL) {
			in->drv->in_close(in);
		}
		free(in);
	}
	SLIST_INIT(&input_devs);
	pthread_mutex_unlock(&input_lock);
}

/* Look for an input device of the given name. */
void *
input_find(const char *name)
{
	struct input *in;

	pthread_mutex_lock(&input_lock);
	SLIST_FOREACH(in, &input_devs, inputs) {
		if (strcmp(in->name, name) == 0)
			break;
	}
	pthread_mutex_unlock(&input_lock);
	return (in);
}

/* Translate an SDL event. */
void
input_event(enum input_type type, const SDL_Event *ev)
{
	struct input *in;

	pthread_mutex_lock(&input_lock);
	SLIST_FOREACH(in, &input_devs, inputs) {
		if (in->type == type &&
		    in->drv->in_match(in, ev))
			break;
	}
	if (in != NULL && in->pos != NULL) {
		in->drv->in_event(in, ev);
	}
	pthread_mutex_unlock(&input_lock);
}

