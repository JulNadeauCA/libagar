/*	$Csoft: event.c,v 1.4 2002/02/01 05:53:07 vedge Exp $	*/

/*
 * Copyright (c) 2001 CubeSoft Communications, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <glib.h>
#include <SDL.h>

#include <engine/engine.h>
#include <engine/mapedit/mapedit.h>

extern void quit(void);

static int	event_hotkey(SDL_Event *);

static int
event_hotkey(SDL_Event *ev)
{
	/* Print active object list. */
	switch (ev->key.keysym.sym) {
#ifdef DEBUG
	case SDLK_t:
		world_dump(world);
		return (0);
#endif /* DEBUG */
	case SDLK_f:
		view_fullscreen(mainview,
		    (mainview->flags & SDL_FULLSCREEN) ? 0 : 1);
		return (0);
	case SDLK_q:
		if (mainview->flags & SDL_FULLSCREEN) {
			mainview->flags &= ~(SDL_FULLSCREEN);
			view_fullscreen(mainview, 0);
		}
		quit();
		break;
	default:
		break;
	}
	return (-1);
}

void
event_loop(void)
{
	SDL_Event ev;

	while (SDL_WaitEvent(&ev)) {
		switch (ev.type) {
		case SDL_VIDEOEXPOSE:
			curmap->redraw++;
			continue;
		case SDL_QUIT:
			return;
		case SDL_MOUSEMOTION:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		case SDL_JOYAXISMOTION:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			if (event_hotkey(&ev) == 0) {
				continue;
			}
			if (curchar != NULL) {
				curchar->event_hook(curchar, &ev);
			} else if (curmapedit != NULL) {
				curmapedit->event_hook(curmapedit, &ev);
			}
			break;
		}
	}
}

