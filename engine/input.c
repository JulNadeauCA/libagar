/*	$Csoft: input.c,v 1.8 2002/04/10 09:23:36 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include <stdlib.h>

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/physics.h>
#include <engine/input.h>

static struct obvec input_vec = {
	input_destroy,
	input_load,
	input_save
};

static int	keyboard_init(struct input *, int);
static int	joy_init(struct input *, int);
static int	mouse_init(struct input *, int);
static Uint32	input_time(Uint32, void *);

struct input *
input_create(int type, int index)
{
	struct input *input;
	int rv = 0;
	char name[64];
	
	switch (type) {
	case INPUT_KEYBOARD:
		sprintf(name, "keyboard%d", index);
		break;
	case INPUT_JOY:
		sprintf(name, "joy%d:%s", index, SDL_JoystickName(index));
		break;
	case INPUT_MOUSE:
		sprintf(name, "mouse%d", index);
		break;
	}

	input = emalloc(sizeof(struct input));
	object_init(&input->obj, name, OBJ_DEFERGC, &input_vec);
	input->type = type;
	input->index = index;
	input->p = NULL;
	input->pos = NULL;

	switch (type) {
	case INPUT_KEYBOARD:
		rv = keyboard_init(input, index);
		break;
	case INPUT_JOY:
		rv = joy_init(input, index);
		break;
	case INPUT_MOUSE:
		rv = mouse_init(input, index);
		break;
	}

	if (rv != 0) {
		free(input);
		return (NULL);
	}

	return (input);
}

static Uint32
input_time(Uint32 ival, void *p)
{
	struct input *in = (struct input *)p;
	struct map *m;
	Uint32 x, y, moved = 0;

	if (in->pos == NULL) {
		return (ival);
	}

	m = in->pos->map;
	x = in->pos->x;
	y = in->pos->y;
	
	pthread_mutex_lock(&m->lock);

	moved = mapdir_move(&in->pos->dir, &x, &y);
	if (moved != 0) {
		static struct mappos opos;
		struct mappos *npos;
		struct noderef *nref;
		
		opos = *in->pos;
		nref = opos.nref;
	
		object_delpos(nref->pobj);
		npos = object_addpos(nref->pobj, nref->offs, nref->flags,
		    opos.input, m, x, y);
		npos->dir = opos.dir;

		m->redraw++;
		mapdir_postmove(&npos->dir, &x, &y, moved);
	}
	pthread_mutex_unlock(&m->lock);

	return (ival);
}

static void
input_key(struct input *in, SDL_Event *ev)
{
	int set;

	set = (ev->type == SDL_KEYDOWN) ? 1 : 0;

	switch (ev->key.keysym.sym) {
	case SDLK_UP:
		if (in->pos->y > 1) {
			mapdir_set(&in->pos->dir, DIR_UP, set);
		}
		break;
	case SDLK_DOWN:
		if (in->pos->y < in->pos->map->maph - 2) {
			mapdir_set(&in->pos->dir, DIR_DOWN, set);
		}
		break;
	case SDLK_LEFT:
		if (in->pos->x > 1) {
			mapdir_set(&in->pos->dir, DIR_LEFT, set);
		}
		break;
	case SDLK_RIGHT:
		if (in->pos->x < in->pos->map->mapw - 2) {
			mapdir_set(&in->pos->dir, DIR_RIGHT, set);
		}
		break;
	default:
		/* XXX ... */
		break;
	}
}

static void
input_joy(struct input *in, SDL_Event *ev)
{
	static int lastdir = 0;

	switch (ev->jaxis.axis) {
	case 0:	/* X */
		if (ev->jaxis.value < 0) {
			lastdir |= DIR_LEFT;
			lastdir &= ~(DIR_RIGHT);
			mapdir_set(&in->pos->dir, DIR_LEFT, 1);
		} else if (ev->jaxis.value > 0) {
			lastdir |= DIR_RIGHT;
			lastdir &= ~(DIR_LEFT);
			mapdir_set(&in->pos->dir, DIR_RIGHT, 1);
		} else {
			mapdir_set(&in->pos->dir, DIR_ALL, 0);
		}
		break;
	case 1:	/* Y */
		if (ev->jaxis.value < 0) {
			lastdir |= DIR_UP;
			lastdir &= ~(DIR_DOWN);
			mapdir_set(&in->pos->dir, DIR_UP, 1);
		} else if (ev->jaxis.value > 0) {
			lastdir |= DIR_DOWN;
			lastdir &= ~(DIR_UP);
			mapdir_set(&in->pos->dir, DIR_DOWN, 1);
		} else {
			mapdir_set(&in->pos->dir, DIR_ALL, 0);
		}
		break;
	}
}	

static void
input_mouse(struct input *in, SDL_Event *ev)
{
	/* XXX ... */
}

int
input_destroy(void *p)
{
	struct input *in = (struct input *)p;

	switch (in->type) {
	case INPUT_KEYBOARD:
	case INPUT_MOUSE:
		break;
	case INPUT_JOY:
		SDL_JoystickClose(in->p);
		in->p = NULL;
		break;
	}
	return (0);
}

void
input_event(void *p, SDL_Event *ev)
{
	struct input *in = (struct input *)p;

	if (in->pos == NULL) {
		dprintf("%s: not controlling anything\n", in->obj.name);
		return;
	}

	switch (in->type) {
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
}

int
input_load(void *p, int fd)
{
	dprintf("todo\n");
	return (0);
}

int
input_save(void *p, int fd)
{
	dprintf("todo\n");
	return (0);
}

static int
keyboard_init(struct input *in, int index)
{
	/* XXX pref */
	in->timer = SDL_AddTimer(20, input_time, in);

	return (0);
}

static int
joy_init(struct input *in, int index)
{
	if (index < 0) {
		return (-1);
	}

	in->p = (SDL_Joystick *)SDL_JoystickOpen(index);
	if (joy == NULL) {
		warning("joy[%d]: %s\n", index, SDL_GetError());
		return (-1);
	}
	SDL_JoystickEventState(SDL_ENABLE);

	return (0);
}

static int
mouse_init(struct input *in, int index)
{
	/* XXX ... */
	return (0);
}

