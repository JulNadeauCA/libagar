/*	$Csoft: event.c,v 1.38 2002/05/19 14:32:22 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc.
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
#include <engine/map.h>
#include <engine/physics.h>
#include <engine/input.h>

#include <engine/mapedit/mapedit.h>

#include <engine/widget/window.h>
#include <engine/widget/widget.h>
#include <engine/widget/text.h>

extern struct gameinfo *gameinfo;

static void	event_hotkey(SDL_Event *);

static void
event_hotkey(SDL_Event *ev)
{
	struct object *ob;

	pthread_mutex_lock(&world->lock);

	switch (ev->key.keysym.sym) {
#ifdef DEBUG
	case SDLK_m:
		view_dumpmask(mainview);
		break;
	case SDLK_w:
		SLIST_FOREACH(ob, &world->wobjsh, wobjs) {
			object_dump(ob);
		}
		break;
	case SDLK_r:
		world->curmap->redraw++;
		break;
	case SDLK_F2:
		object_save(world);
		break;
	case SDLK_F4:
		object_load(world);
		break;
	case SDLK_F5:
		map_verify(world->curmap);
		break;
	case SDLK_t:
		text_msg(5, TEXT_SLEEP, "%d ticks\n", SDL_GetTicks());
		break;
#endif
	case SDLK_F1:
		engine_config();
		break;
	case SDLK_f:
		if (ev->key.keysym.mod & KMOD_CTRL) {
			view_fullscreen(world->curmap->view,
			    (world->curmap->view->flags & SDL_FULLSCREEN) ?
			     0 : 1);
		}
		break;
	case SDLK_v:
		text_msg(10, TEXT_SLEEP, "AGAR engine v%s\n%s v%d.%d\n%s\n",
		    ENGINE_VERSION, gameinfo->name, gameinfo->ver[0],
		    gameinfo->ver[1], gameinfo->copyright);
		break;
	case SDLK_ESCAPE:
		pthread_mutex_unlock(&world->lock);
		engine_destroy();
		break;
	default:
		break;
	}

	pthread_mutex_unlock(&world->lock);
}

void *
event_loop(void *arg)
{
	Uint32 ltick, ntick;
	Sint32 delta;
	SDL_Event ev;
	struct map *m = NULL;
	struct window *win;
	
	/* Start the garbage collection process. */
	object_init_gc();

	/* XXX pref: max fps */
	for (ntick = 0, ltick = SDL_GetTicks(), delta = 100;;) {
		ntick = SDL_GetTicks();
		if ((ntick - ltick) >= delta) {
			/* XXX inefficient locking */
			pthread_mutex_lock(&world->lock);
			m = world->curmap;
			pthread_mutex_lock(&m->lock);
			map_animate(m);
			if (m->redraw != 0) {
				m->redraw = 0;
				map_draw(m);
				delta = m->fps - (SDL_GetTicks() - ntick);
				if (delta < 1) {
					dprintf("overrun (delta=%d)\n", delta);
					delta = 1;
				}
				text_drawall();		/* XXX window */
			}
			pthread_mutex_unlock(&m->lock);
			pthread_mutex_unlock(&world->lock);

			/* XXX inefficient, should share locks */
			pthread_mutex_lock(&mainview->lock);
			if (!TAILQ_EMPTY(&mainview->windowsh)) {
				window_draw_all();
			}
			pthread_mutex_unlock(&mainview->lock);

			ltick = SDL_GetTicks();
		} else if (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_KEYDOWN) {
				event_hotkey(&ev);
			}
			switch (ev.type) {
			case SDL_VIDEOEXPOSE:
				view_redraw(mainview);
				break;
			case SDL_MOUSEMOTION:
				if (curmapedit != NULL) {	/* XXX */
					mapedit_event(curmapedit, &ev);
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				if (curmapedit != NULL) {	/* XXX */
					mapedit_event(curmapedit, &ev);
				}
				pthread_mutex_lock(&mainview->lock);
				TAILQ_FOREACH(win, &mainview->windowsh,
				    windows) {
				    	struct region *reg;

					pthread_mutex_lock(&win->lock);
					SLIST_FOREACH(reg, &win->regionsh,
					    regions) {
						struct widget *wid;

						TAILQ_FOREACH(wid,
						    &reg->widgetsh, widgets) {
							if (wid->flags &
							    WIDGET_HIDE) {
								continue;
							}
							window_widget_event(
							    reg, wid, &ev);
						}
					}
					pthread_mutex_unlock(&win->lock);
				}
				pthread_mutex_unlock(&mainview->lock);
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
				return (NULL);
			}
		}
	}

	engine_destroy();
	return (NULL);
}

