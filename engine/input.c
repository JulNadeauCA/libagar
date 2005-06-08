/*	$Csoft: input.c,v 1.55 2005/04/14 06:19:35 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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
#include <engine/input.h>

#include <string.h>

struct ag_input_devq ag_input_devs;
pthread_mutex_t	     ag_input_lock = PTHREAD_MUTEX_INITIALIZER;

/* Initialize and attach an input device. */
void
AG_InputSet(void *p, enum ag_input_type type, const char *name,
    const AG_InputOps *ops)
{
	AG_Input *in = p;

	strlcpy(in->name, name, sizeof(in->name));
	in->type = type;
	in->ops = ops;

	pthread_mutex_lock(&ag_input_lock);
	SLIST_INSERT_HEAD(&ag_input_devs, in, inputs);
	pthread_mutex_unlock(&ag_input_lock);

	dprintf("registered %s (%s)\n", in->name, in->ops->name);
}

/* Detach and free an input device structure. */
void
AG_InputRemove(void *p)
{
	AG_Input *in = p;

	if (in->ops->in_close != NULL)
		in->ops->in_close(in);

	pthread_mutex_lock(&ag_input_lock);
	SLIST_REMOVE(&ag_input_devs, in, ag_input, inputs);
	pthread_mutex_unlock(&ag_input_lock);

	free(in);
}

/* Destroy all attached input device structures. */
void
AG_InputDestroy(void)
{
	AG_Input *in, *nin;

	pthread_mutex_lock(&ag_input_lock);
	for (in = SLIST_FIRST(&ag_input_devs);
	     in != SLIST_END(&ag_input_devs);
	     in = nin) {
		nin = SLIST_NEXT(in, inputs);

		if (in->ops->in_close != NULL) {
			in->ops->in_close(in);
		}
		free(in);
	}
	SLIST_INIT(&ag_input_devs);
	pthread_mutex_unlock(&ag_input_lock);
}

/* Look for an input device of the given name. */
void *
AG_InputFind(const char *name)
{
	AG_Input *in;

	pthread_mutex_lock(&ag_input_lock);
	SLIST_FOREACH(in, &ag_input_devs, inputs) {
		if (strcmp(in->name, name) == 0)
			break;
	}
	pthread_mutex_unlock(&ag_input_lock);
	return (in);
}

/* Translate an SDL event. */
void
AG_InputEvent(enum ag_input_type type, const SDL_Event *ev)
{
	AG_Input *in;

	pthread_mutex_lock(&ag_input_lock);
	SLIST_FOREACH(in, &ag_input_devs, inputs) {
		if (in->type == type &&
		    in->ops->in_match(in, ev))
			break;
	}
	if (in != NULL) {
		in->ops->in_event(in, ev);
	}
	pthread_mutex_unlock(&ag_input_lock);
}

