/*	$Csoft: event.c,v 1.16 2002/03/03 06:22:42 vedge Exp $	*/

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
#include <engine/input.h>
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
	struct map *m = world->curmap;
	static Uint32 ltick, ntick;
	static Sint32 delta;

	if (m == NULL) {
		fatal("no map\n");
		return;
	}

	ltick = SDL_GetTicks();

	for (ntick = 0, delta = m->fps;;) {
		ntick = SDL_GetTicks();
		if ((ntick - ltick) >= delta) {
			if (pthread_mutex_lock(&m->lock) == 0) {
				map_animate(m);
				if (m->redraw) {
					m->redraw = 0;
					map_draw(m);
					delta = m->fps -
					    (SDL_GetTicks() - ntick);
					if (delta < 1) {
						dprintf("overrun (delta=%d)\n",
						    delta);
						delta = 1;
					}
				}
				pthread_mutex_unlock(&m->lock);
			} else {
				perror("map");
			}
			ltick = SDL_GetTicks();
		} else if (SDL_PollEvent(&ev)) {
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
				break;
			case SDL_JOYAXISMOTION:
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				if (curmapedit != NULL) {	/* XXX */
					mapedit_event(curmapedit, &ev);
					break;
				} else {
					input_event(joy, &ev);
				}
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				if (curmapedit != NULL) {	/* XXX */
					mapedit_event(curmapedit, &ev);
					break;
				} else {
					input_event(keyboard, &ev);
				}
				break;
			case SDL_QUIT:
				return;
			}
		}
	}
}

