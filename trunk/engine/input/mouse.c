/*	$Csoft$	*/

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

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/physics.h>
#include <engine/input.h>

static int	mouse_match(const void *, const SDL_Event *);
static void	mouse_event(void *, const SDL_Event *);

const struct input_driver mouse_driver = {
	N_("Mouse"),
	NULL,
	mouse_match,
	mouse_event
};

struct mouse *
mouse_new(int index)
{
	char name[INPUT_NAME_MAX];
	struct mouse *ms;

	ms = Malloc(sizeof(struct mouse));
	ms->index = index;
	snprintf(name, sizeof(name), "mouse%d", index);
	input_register(ms, INPUT_MOUSE, name, &mouse_driver);
	return (ms);
}

static int
mouse_match(const void *p, const SDL_Event *ev)
{
	const struct mouse *ms = p;

	switch (ev->type) {
	case SDL_MOUSEMOTION:
		if (ev->motion.which == ms->index) {
			return (1);
		}
		break;
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEBUTTONDOWN:
		if (ev->button.which == ms->index) {
			return (1);
		}
		break;
	}
	return (0);
}

static void
mouse_event(void *p, const SDL_Event *ev)
{
	struct input *in = p;
	static int lastdir = 0;

	/* XXX map ... */
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

