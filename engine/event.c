/*	$Csoft: event.c,v 1.12 2002/02/14 05:26:00 vedge Exp $	*/

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

#include <engine/engine.h>
#include <engine/mapedit/mapedit.h>

static void	event_hotkey(SDL_Event *);

static void
event_hotkey(SDL_Event *ev)
{
	switch (ev->key.keysym.sym) {
#ifdef DEBUG
	case SDLK_w:
		if (ev->key.keysym.mod & KMOD_CTRL) {
			world_dump(world);
		}
		break;
#endif /* DEBUG */
	case SDLK_f:
		if (ev->key.keysym.mod & KMOD_CTRL) {
			view_fullscreen(mainview,
			    (mainview->flags & SDL_FULLSCREEN) ? 0 : 1);
		}
		break;
	case SDLK_ESCAPE:
		engine_destroy();
		/*NOTREACHED*/
	default:
		break;
	}
}

void
event_loop(void)
{
	SDL_Event ev;

	while (SDL_WaitEvent(&ev)) {
		if (ev.type == SDL_KEYDOWN) {
			event_hotkey(&ev);
		}
		switch (ev.type) {
		case SDL_VIDEOEXPOSE:
			world->curmap->redraw++;
			continue;
		case SDL_MOUSEMOTION:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		case SDL_JOYAXISMOTION:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			if (curchar != NULL) {
				if (curchar->obj.vec->event != NULL) {
					curchar->obj.vec->event(curchar, &ev);
				}
			} else if (curmapedit != NULL) {
				if (curmapedit->obj.vec->event != NULL) {
					curmapedit->obj.vec->event(curmapedit,
					    &ev);
				}
			}
			break;
		case SDL_QUIT:
			return;
		}
	}
}

