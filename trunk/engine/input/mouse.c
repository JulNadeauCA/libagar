/*	$Csoft: mouse.c,v 1.14 2005/09/27 00:25:17 vedge Exp $	*/

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
#include <engine/input.h>
#include <engine/view.h>

#include <engine/map/map.h>

int agMouseDblclickDelay = 250;		/* Mouse double-click delay */
int agMouseSpinDelay = 250;		/* Spinbutton repeat delay */
int agMouseSpinIval = 50;		/* Spinbutton repeat interval */

static int
mouse_match(const void *p, const SDL_Event *ev)
{
	const AG_Mouse *ms = p;

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
	/* TODO */
}

AG_Mouse *
AG_MouseNew(int index)
{
	char name[AG_INPUT_NAME_MAX];
	AG_Mouse *ms;

	ms = Malloc(sizeof(AG_Mouse), M_INPUT);
	ms->index = index;
	snprintf(name, sizeof(name), "mouse%d", index);
	AG_InputSet(ms, AG_INPUT_MOUSE, name, &agMouseOps);
	return (ms);
}

Uint8
AG_MouseGetState(int *x, int *y)
{
	Uint8 rv;

	rv = SDL_GetMouseState(x, y);
#if defined(__APPLE__) && defined(HAVE_OPENGL)
	if (agView->opengl && y != NULL)
		*y = agView->h - *y;
#endif
	return (rv);
}

const AG_InputOps agMouseOps = {
	N_("Mouse"),
	NULL,
	mouse_match,
	mouse_event
};

