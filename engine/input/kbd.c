/*	$Csoft: kbd.c,v 1.10 2005/01/05 04:44:04 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004, 2005 CubeSoft Communications, Inc.
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
#include <engine/input.h>

static int	kbd_match(const void *, const SDL_Event *);
static void	kbd_event(void *, const SDL_Event *);

const struct input_driver kbd_driver = {
	N_("Keyboard"),
	NULL,
	kbd_match,
	kbd_event
};

int kbd_unitrans = 1;				/* Unicode translation */
int kbd_delay = 250;				/* Key repeat delay */
int kbd_repeat = 35;				/* Key repeat interval */

struct kbd *
kbd_new(int index)
{
	char name[INPUT_NAME_MAX];
	struct kbd *kbd;

	kbd = Malloc(sizeof(struct kbd), M_INPUT);
	kbd->index = index;
	snprintf(name, sizeof(name), "kbd%d", index);
	input_register(kbd, INPUT_KEYBOARD, name, &kbd_driver);
	SDL_EnableUNICODE(kbd_unitrans);
	return (kbd);
}

int
kbd_match(const void *p, const SDL_Event *ev)
{
	const struct kbd *kbd = p;

	switch (ev->type) {
	case SDL_KEYUP:
	case SDL_KEYDOWN:
		if (ev->key.which == kbd->index) {
			return (1);
		}
		break;
	}
	return (0);
}

void
kbd_event(void *p, const SDL_Event *ev)
{
#if 0
	struct input *in = p;
	int set = (ev->type == SDL_KEYDOWN) ? 1 : 0;

	/* XXX map, etc */
	switch (ev->key.keysym.sym) {
	case SDLK_UP:
		if (in->pos->y > 1) {
			if (set) {
				mapdir_set(&in->pos->dir, DIR_N);
			} else {
				mapdir_unset(&in->pos->dir, DIR_N);
			}
		}
		break;
	case SDLK_DOWN:
		if (in->pos->y < in->pos->map->maph - 2) {
			if (set) {
				mapdir_set(&in->pos->dir, DIR_S);
			} else {
				mapdir_unset(&in->pos->dir, DIR_S);
			}
		}
		break;
	case SDLK_LEFT:
		if (in->pos->x > 1) {
			if (set) {
				mapdir_set(&in->pos->dir, DIR_W);
			} else {
				mapdir_unset(&in->pos->dir, DIR_W);
			}
		}
		break;
	case SDLK_RIGHT:
		if (in->pos->x < in->pos->map->mapw - 2) {
			if (set) {
				mapdir_set(&in->pos->dir, DIR_E);
			} else {
				mapdir_unset(&in->pos->dir, DIR_E);
			}
		}
		break;
	default:
		break;
	}
#endif
}

