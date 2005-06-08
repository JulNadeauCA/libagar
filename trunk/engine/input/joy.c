/*	$Csoft: joy.c,v 1.8 2005/04/14 06:19:37 vedge Exp $	*/

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

AG_Joystick *
AG_JoystickNew(int index)
{
	char name[AG_INPUT_NAME_MAX];
	AG_Joystick *joy;

	joy = Malloc(sizeof(AG_Joystick), M_INPUT);
	joy->index = index;
	snprintf(name, sizeof(name), "joy%d", index);

	if ((joy->joy = SDL_JoystickOpen(index)) == NULL)
		goto fail;

	SDL_JoystickEventState(SDL_ENABLE);
	AG_InputSet(joy, AG_INPUT_JOY, name, &agJoystickOps);
	return (joy);
fail:
	Free(joy, M_INPUT);
	return (NULL);
}

static void
close(void *p)
{
	AG_Joystick *joy = p;

	SDL_JoystickClose(joy->joy);
}

static int
match(const void *p, const SDL_Event *ev)
{
	const AG_Joystick *joy = p;

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
proc_event(void *p, const SDL_Event *ev)
{
	/* TODO */
}

const AG_InputOps agJoystickOps = {
	N_("Joystick"),
	close,
	match,
	proc_event
};

