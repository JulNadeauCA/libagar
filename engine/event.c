/*	$Csoft: event.c,v 1.1.1.1 2002/01/25 09:50:02 vedge Exp $	*/

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

#include <engine/view.h>
#include <engine/debug.h>
#include <engine/event.h>
#include <engine/object.h>
#include <engine/world.h>
#include <engine/map.h>

extern void quit(void);

static void	 event_dispatch(void *, void *);
static void	 event_hotkey(SDL_Event *);

/* XXX inefficient */
static void
event_dispatch(void *arg, void *p)
{
	struct object *ob = (struct object *)arg;
	SDL_Event *ev = (SDL_Event *)p;

	if (ob->flags & EVENT_HOOK) {
		ob->event_hook(ob, ev);
	}
}

/* Global hotkeys. */
static __inline void
event_hotkey(SDL_Event *ev)
{
	/* Print active object list. */
	switch (ev->key.keysym.sym) {
#ifdef DEBUG
	case SDLK_t:
		world_dump(world);
		break;
#endif /* DEBUG */
	case SDLK_f:
		view_fullscreen(mainview,
		    (mainview->flags & SDL_FULLSCREEN) ? 0 : 1);
		break;
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
}

void
event_loop(void)
{
	SDL_Event ev;

	while (SDL_WaitEvent(&ev)) {
		switch (ev.type) {
#if 0
		case SDL_VIDEOEXPOSE:
			map_draw(curmap);
			continue;
#endif
		case SDL_QUIT:
			quit();
			/* NOTREACHED */
		default:
			if (ev.type == SDL_KEYDOWN) {
				event_hotkey(&ev);
			}
			/* XXX optimize */
			if (pthread_mutex_lock(&world->lock) == 0) {
				g_slist_foreach(world->objs,
				    (GFunc) event_dispatch, &ev);
				pthread_mutex_unlock(&world->lock);
			}
			continue;
		}
	}
}

