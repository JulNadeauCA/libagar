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
#include "command.h"
#include "mouse.h"

void
mouse_motion(struct mapedit *med, SDL_Event *ev)
{
	static Uint32 ommapx, ommapy;
	struct map *m = med->map;
	Uint32 mx, my;

	ommapx = med->mmapx;
	ommapy = med->mmapy;

	med->mmapx = (ev->motion.x / m->tilew);
	med->mmapy = (ev->motion.y / m->tileh);
	mx = (m->view->mapx + med->mmapx) - 1;
	my = (m->view->mapy + med->mmapy) - 1;

	if (med->x == mx && med->y == my) {
		/* Nothing to do. */
		return;
	}

	/* XXX prefs */
	if (SDL_GetMouseState(NULL, NULL) &
	   (SDL_BUTTON_MMASK|SDL_BUTTON_RMASK)) {
	   	if (med->cursor_dir.current == 0) {
			pthread_mutex_lock(&m->lock);
			mapedit_move(med, mx, my);
			mapedit_sticky(med);
			pthread_mutex_unlock(&m->lock);
			m->redraw++;
		}
	}
	if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_RMASK) {
		pthread_mutex_lock(&m->lock);
		mapedit_push(med, &m->map[mx][my], med->curoffs, med->curflags);
		pthread_mutex_unlock(&m->lock);
		m->redraw++;
	}
	if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK) {
		if (ommapx < med->mmapx) {
			if (m->view->mapx > 0) {
				scroll(m, DIR_LEFT);
			}
		} else if (med->mmapx < ommapx) {
			if (m->view->mapx + m->view->mapw < m->mapw) {
				scroll(m, DIR_RIGHT);
			}
		}
		if (ommapy < med->mmapy) {
			if (m->view->mapy > 0) {
				scroll(m, DIR_UP);
			}
		} else if (med->mmapy < ommapy) {
			if (m->view->mapy + m->view->maph < m->maph) {
				scroll(m, DIR_DOWN);
			}
		}
	}
}

void
mouse_button(struct mapedit *med, SDL_Event *ev)
{
	struct map *m = med->map;
	Uint32 mx, my;

	pthread_mutex_lock(&m->lock);
	mx = (m->view->mapx + ev->button.x / m->tilew) - 1;
	my = (m->view->mapy + ev->button.y / m->tileh) - 1;
	if (med->cursor_dir.current == 0) {
		mapedit_move(med, mx, my);
	}

	if (ev->button.button == 3) {
		mapedit_push(med, &m->map[mx][my], med->curoffs, med->curflags);
	}
	
	pthread_mutex_unlock(&m->lock);
	m->redraw++;
}

