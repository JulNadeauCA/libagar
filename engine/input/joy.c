/*	$Csoft: joy.c,v 1.3 2004/01/03 04:25:07 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004 CubeSoft Communications, Inc.
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

static void	joy_close(void *);
static int	joy_match(const void *, const SDL_Event *);
static void	joy_event(void *, const SDL_Event *);

const struct input_driver joy_driver = {
	N_("Joystick"),
	joy_close,
	joy_match,
	joy_event
};

struct joy *
joy_new(int index)
{
	char name[INPUT_NAME_MAX];
	struct joy *joy;

	joy = Malloc(sizeof(struct joy));
	joy->index = index;
	snprintf(name, sizeof(name), "joy%d", index);

	if ((joy->joy = SDL_JoystickOpen(index)) == NULL)
		goto fail;

	SDL_JoystickEventState(SDL_ENABLE);

	input_register(joy, INPUT_JOY, name, &joy_driver);
	return (joy);
fail:
	free(joy);
	return (NULL);
}

void
joy_close(void *p)
{
	struct joy *joy = p;

	SDL_JoystickClose(joy->joy);
}

static int
joy_match(const void *p, const SDL_Event *ev)
{
	const struct joy *joy = p;

	switch (ev->type) {
	case SDL_JOYAXISMOTION:
		if (ev->jaxis.which == joy->index) {
			return (1);
		}
		break;
	case SDL_JOYBALLMOTION:
		if (ev->jball.which == joy->index) {
			return (1);
		}
		break;
	case SDL_JOYHATMOTION:
		if (ev->jhat.which == joy->index) {
			return (1);
		}
		break;
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		if (ev->jbutton.which == joy->index) {
			return (1);
		}
		break;
	}
	return (0);
}

static void
joy_event(void *p, const SDL_Event *ev)
{
	struct input *in = p;
	static int lastdir = 0;

	/* XXX map ... */
	switch (ev->jaxis.axis) {
	case 0:	/* X */
		if (ev->jaxis.value < 0) {
			lastdir |= DIR_W;
			lastdir &= ~(DIR_E);
			mapdir_set(&in->pos->dir, DIR_W);
		} else if (ev->jaxis.value > 0) {
			lastdir |= DIR_E;
			lastdir &= ~(DIR_W);
			mapdir_set(&in->pos->dir, DIR_E);
		} else {
			mapdir_unset(&in->pos->dir, DIR_ALL);
		}
		break;
	case 1:	/* Y */
		if (ev->jaxis.value < 0) {
			lastdir |= DIR_N;
			lastdir &= ~(DIR_S);
			mapdir_set(&in->pos->dir, DIR_N);
		} else if (ev->jaxis.value > 0) {
			lastdir |= DIR_S;
			lastdir &= ~(DIR_N);
			mapdir_set(&in->pos->dir, DIR_S);
		} else {
			mapdir_unset(&in->pos->dir, DIR_ALL);
		}
		break;
	}
}

