/*	$Csoft	    */

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

#include <engine/engine.h>

#include "mapedit.h"
#include "mouse.h"

void
mouse_motion(struct mapedit *med, SDL_Event *ev)
{
	static int ommapx, ommapy;
	struct map *map = med->map;

	ommapx = med->mmapx;
	ommapy = med->mmapy;

	med->mmapx = ev->motion.x / med->map->view->tilew;
	med->mmapy = ev->motion.y / med->map->view->tileh;
	
	if (ommapx < med->mmapx) {
		if (map->view->mapx > 0) {
			scroll(med->map, DIR_LEFT);
		}
	} else if (med->mmapx < ommapx) {
		if (map->view->mapx + map->view->mapw < map->mapw) {
			scroll(med->map, DIR_RIGHT);
		}
	}
	if (ommapy < med->mmapy) {
		if (map->view->mapy > 0) {
			scroll(med->map, DIR_UP);
		}
	} else if (med->mmapy < ommapy) {
		if (map->view->mapy + map->view->maph < map->maph) {
			scroll(med->map, DIR_DOWN);
		}
	}
}

void
mouse_button(struct mapedit *med, SDL_Event *ev)
{
	struct map *map = med->map;
	int mx, my;

	pthread_mutex_lock(&map->lock);
	mx = map->view->mapx + ev->button.x / map->view->tilew;
	my = map->view->mapy + ev->button.y / map->view->tileh;
	if (med->flags & MAPEDIT_TILESTACK)	/* XXX hack */
		mx--;
	if (med->flags & MAPEDIT_OBJLIST)	/* XXX hack */
		my--;
	MAPEDIT_MOVE(med, mx, my);
	pthread_mutex_unlock(&map->lock);
	map->redraw++;

	if (ev->button.button == 3) {
		SDL_Event fev;

		/* Fake the add command. */
		fev.type = SDL_KEYDOWN;
		fev.key.keysym.sym = SDLK_a;
		SDL_PushEvent(&fev);
	}
}

