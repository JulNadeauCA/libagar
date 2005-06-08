/*	$Csoft: kbd.c,v 1.12 2005/04/14 06:19:37 vedge Exp $	*/

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

#include <engine/map/map.h>

int agKbdUnicode = 1;				/* Unicode translation */
int agKbdDelay = 250;				/* Key repeat delay */
int agKbdRepeat = 35;				/* Key repeat interval */

AG_Keyboard *
AG_KeyboardNew(int index)
{
	char name[AG_INPUT_NAME_MAX];
	AG_Keyboard *kbd;

	kbd = Malloc(sizeof(AG_Keyboard), M_INPUT);
	kbd->index = index;
	snprintf(name, sizeof(name), "kbd%d", index);
	AG_InputSet(kbd, AG_INPUT_KEYBOARD, name, &agKeyboardOps);
	SDL_EnableUNICODE(agKbdUnicode);
	return (kbd);
}

static int
match(const void *p, const SDL_Event *ev)
{
	const AG_Keyboard *kbd = p;

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

static void
proc_event(void *p, const SDL_Event *ev)
{
	/* TODO */
}

const AG_InputOps agKeyboardOps = {
	N_("Keyboard"),
	NULL,
	match,
	proc_event
};

