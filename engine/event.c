/*	$Csoft: event.c,v 1.14 2002/02/17 23:15:36 vedge Exp $	*/

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
#ifdef DEBUG
	case SDLK_F2:
		object_save(world);
		break;
	case SDLK_F4:
		object_load(world);
		break;
#endif
	case SDLK_ESCAPE:
		engine_destroy();
		break;
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
			if (curmapedit != NULL) {	/* XXX */
				mapedit_event(curmapedit, &ev);
				break;
			}
			input_event(mouse, &ev);
			break;
		case SDL_JOYAXISMOTION:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			if (curmapedit != NULL) {	/* XXX */
				mapedit_event(curmapedit, &ev);
				break;
			}
			input_event(joy, &ev);
			break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			if (curmapedit != NULL) {	/* XXX */
				mapedit_event(curmapedit, &ev);
				break;
			}
			input_event(keyboard, &ev);
			break;
		case SDL_QUIT:
			return;
		}
	}
}

