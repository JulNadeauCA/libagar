/*	$Csoft: input.c,v 1.33 2003/01/12 04:07:16 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003 CubeSoft Communications, Inc.
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

#include "compat/asprintf.h"
#include "engine.h"

#include "map.h"
#include "physics.h"
#include "input.h"
#include "world.h"

static TAILQ_HEAD(, input) inputs;
static pthread_mutex_t inputs_lock = PTHREAD_MUTEX_INITIALIZER;

struct input *
input_new(int type, int index)
{
	char *name;
	struct input *input;
	int rv = 0;
	
	switch (type) {
	case INPUT_KEYBOARD:
		Asprintf(&name, "keyboard%d", index);
		break;
	case INPUT_JOY:
		Asprintf(&name, "joy%d", index);
		break;
	case INPUT_MOUSE:
		Asprintf(&name, "mouse%d", index);
		break;
	}

	input = emalloc(sizeof(struct input));
	object_init(&input->obj, "input-device", name, NULL, OBJECT_SYSTEM,
	    NULL);
	free(name);

	input->type = type;
	input->index = index;
	input->p = NULL;
	input->pos = NULL;
	pthread_mutex_init(&input->lock, NULL);

	switch (type) {
	case INPUT_JOY:
		if (index < 0) {
			error_set("bad index");
			return (NULL);
		}
		input->p = SDL_JoystickOpen(index);
		if (input->p == NULL) {
			error_set("joy[%d]: %s", index, SDL_GetError());
			return (NULL);
		}
		SDL_JoystickEventState(SDL_ENABLE);
		break;
	default:
		break;
	}

	pthread_mutex_lock(&inputs_lock);
	TAILQ_INSERT_HEAD(&inputs, input, inputs);
	pthread_mutex_unlock(&inputs_lock);

	world_attach(input);
	dprintf("registered %s (#%i)\n", OBJECT(input)->name, index);
	return (input);
}

/* Input structure must be locked. */
static void
input_key(struct input *in, SDL_Event *ev)
{
	int set = (ev->type == SDL_KEYDOWN) ? 1 : 0;

	switch (ev->key.keysym.sym) {
	case SDLK_UP:
		if (in->pos->y > 1) {
			if (set) {
				mapdir_set(&in->pos->dir, DIR_UP);
			} else {
				mapdir_unset(&in->pos->dir, DIR_UP);
			}
		}
		break;
	case SDLK_DOWN:
		if (in->pos->y < in->pos->map->maph - 2) {
			if (set) {
				mapdir_set(&in->pos->dir, DIR_DOWN);
			} else {
				mapdir_unset(&in->pos->dir, DIR_DOWN);
			}
		}
		break;
	case SDLK_LEFT:
		if (in->pos->x > 1) {
			if (set) {
				mapdir_set(&in->pos->dir, DIR_LEFT);
			} else {
				mapdir_unset(&in->pos->dir, DIR_LEFT);
			}
		}
		break;
	case SDLK_RIGHT:
		if (in->pos->x < in->pos->map->mapw - 2) {
			if (set) {
				mapdir_set(&in->pos->dir, DIR_RIGHT);
			} else {
				mapdir_unset(&in->pos->dir, DIR_RIGHT);
			}
		}
		break;
	default:
		/* XXX ... */
		break;
	}
}

/* Input structure must be locked. */
static void
input_joy(struct input *in, SDL_Event *ev)
{
	static int lastdir = 0;

	switch (ev->jaxis.axis) {
	case 0:	/* X */
		if (ev->jaxis.value < 0) {
			lastdir |= DIR_LEFT;
			lastdir &= ~(DIR_RIGHT);
			mapdir_set(&in->pos->dir, DIR_LEFT);
		} else if (ev->jaxis.value > 0) {
			lastdir |= DIR_RIGHT;
			lastdir &= ~(DIR_LEFT);
			mapdir_set(&in->pos->dir, DIR_RIGHT);
		} else {
			mapdir_unset(&in->pos->dir, DIR_ALL);
		}
		break;
	case 1:	/* Y */
		if (ev->jaxis.value < 0) {
			lastdir |= DIR_UP;
			lastdir &= ~(DIR_DOWN);
			mapdir_set(&in->pos->dir, DIR_UP);
		} else if (ev->jaxis.value > 0) {
			lastdir |= DIR_DOWN;
			lastdir &= ~(DIR_UP);
			mapdir_set(&in->pos->dir, DIR_DOWN);
		} else {
			mapdir_unset(&in->pos->dir, DIR_ALL);
		}
		break;
	}
}	

static void
input_mouse(struct input *in, SDL_Event *ev)
{
	/* XXX ... */
}

void
input_destroy_all(void)
{
	struct input *in, *nin;

	pthread_mutex_lock(&inputs_lock);
	printf("freed input devices:");
	for (in = TAILQ_FIRST(&inputs);
	     in != TAILQ_END(&inputs);
	     in = nin) {
		nin = TAILQ_NEXT(in, inputs);

		printf(" %s", OBJECT(in)->name);
		fflush(stdout);
		object_destroy(in);
	}
	printf("\n");
	TAILQ_INIT(&inputs);
	pthread_mutex_unlock(&inputs_lock);
}

void
input_destroy(void *p)
{
	struct input *in = p;

	switch (in->type) {
	case INPUT_JOY:
		SDL_JoystickClose(in->p);
		break;
	default:
		break;
	}
	pthread_mutex_destroy(&in->lock);
	free(in);
}

/* Look for an input device of the given name. */
struct input *
input_find(char *name)
{
	struct input *in;

	pthread_mutex_lock(&inputs_lock);
	TAILQ_FOREACH(in, &inputs, inputs) {
		if (strcmp(OBJECT(in)->name, name) == 0) {
			pthread_mutex_unlock(&inputs_lock);
			return (in);
		}
	}
	pthread_mutex_unlock(&inputs_lock);
	return (NULL);
}

/*
 * Map an SDL input device event to an input device object.
 * Input device list must be locked.
 */
struct input *
input_find_ev(enum input_type type, SDL_Event *ev)
{
	struct input *in;

	TAILQ_FOREACH(in, &inputs, inputs) {
		if (in->type != type) {
			continue;
		}
		switch (type) {
		case INPUT_KEYBOARD:
			switch (ev->type) {
			case SDL_KEYUP:
			case SDL_KEYDOWN:
				if (ev->key.which == in->index) {
					return (in);
				}
			}
			break;
		case INPUT_MOUSE:
			switch (ev->type) {
			case SDL_MOUSEMOTION:
				if (ev->motion.which == in->index) {
					return (in);
				}
				break;
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
				if (ev->button.which == in->index) {
					return (in);
				}
				break;
			}
			break;
		case INPUT_JOY:
			switch (ev->type) {
			case SDL_JOYAXISMOTION:
				if (ev->jaxis.which == in->index) {
					return (in);
				}
				break;
			case SDL_JOYBALLMOTION:
				if (ev->jball.which == in->index) {
					return (in);
				}
				break;
			case SDL_JOYHATMOTION:
				if (ev->jhat.which == in->index) {
					return (in);
				}
				break;
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				if (ev->jbutton.which == in->index) {
					return (in);
				}
				break;
			}
			break;
		}
	}

	return (NULL);
}

/* Translate an SDL event. */
void
input_event(enum input_type type, SDL_Event *ev)
{
	struct input *in;
	
	pthread_mutex_lock(&inputs_lock);

	/* See which input device should handle this event. */
	in = input_find_ev(type, ev);
	if (in == NULL) {
		pthread_mutex_unlock(&inputs_lock);
		dprintf("unrecognized event %d\n", ev->type);
		return;
	}

	pthread_mutex_lock(&in->lock);
	if (in->pos == NULL) {
#if 0
		dprintf("%s: unbound input event\n", OBJECT(in)->name);
#endif
		goto done;
	}
	switch (type) {
	case INPUT_KEYBOARD:
		input_key(in, ev);
		break;
	case INPUT_JOY:
		input_joy(in, ev);
		break;
	case INPUT_MOUSE:
		input_mouse(in, ev);
		break;
	}
done:
	pthread_mutex_unlock(&in->lock);
	pthread_mutex_unlock(&inputs_lock);
}

