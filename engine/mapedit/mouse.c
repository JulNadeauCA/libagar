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

/*
 * Map editor mouse motion handler.
 * Must be called on a locked map.
 */
void
mouse_motion(struct mapedit *med, SDL_Event *ev)
{
	static Uint32 ommapx, ommapy, omtmapy;
	struct map *m = med->map;
	Uint32 mx, my;

	ommapx = med->mmapx;
	ommapy = med->mmapy;
	omtmapy = med->mtmapy;

	med->mmapx = (ev->motion.x / m->tilew);
	med->mmapy = (ev->motion.y / m->tileh);
	med->mtmapy = med->mmapy;
	mx = (m->view->mapx + med->mmapx) - 1;
	my = (m->view->mapy + med->mmapy) - 1;

	if (med->x == mx && med->y == my) {
		/* Nothing to do. */
		return;
	}

	/* Tile stack. No functionality. */
	if (med->mmapx == 0) {
		return;
	}
	/* Object list. Allow selection/scrolling. */
	if (med->mmapy == 0) {
		if (SDL_GetMouseState(NULL, NULL) &
		   (SDL_BUTTON_MMASK|SDL_BUTTON_RMASK)) {
			/* XXX */
		}
		return;
	}

	/* Tile list. Allow selection/scrolling. */
	if (med->mmapx > m->view->mapw) {
		static Uint8 ms;

		ms = SDL_GetMouseState(NULL, NULL);
		
		if (ms & (SDL_BUTTON_MMASK|SDL_BUTTON_RMASK)) {
			/* Scroll */
			if (med->mtmapy > omtmapy &&	/* Down */
			    --med->curoffs < 0) {
				med->curoffs = med->curobj->nrefs - 1;
			}
			if (med->mtmapy < omtmapy &&	/* Up */
			    ++med->curoffs > med->curobj->nrefs - 1) {
				med->curoffs = 0;
			}
			mapedit_tilelist(med);
		} else if (ms & (SDL_BUTTON_LMASK)) {
			/* Select */
			med->curoffs -= ((med->tilelist.h / m->tileh) / 2);
			med->curoffs -= med->mtmapy;
			mapedit_tilelist(med);
		}
		return;
	}
	/* Don't exceed view boundaries. */
	if (med->mmapy > m->view->maph) {
		return;
	}

	/* XXX prefs */
	if (SDL_GetMouseState(NULL, NULL) &
	   (SDL_BUTTON_MMASK|SDL_BUTTON_RMASK)) {
	   	if (med->cursor_dir.current == 0) {
			mapedit_move(med, mx, my);
			mapedit_sticky(med);
			m->redraw++;
		}
	}
	if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_RMASK) {
		mapedit_push(med, &m->map[mx][my], med->curoffs, med->curflags);
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

/*
 * Map editor mouse button handler.
 * Must be called on a locked map.
 */
void
mouse_button(struct mapedit *med, SDL_Event *ev)
{
	struct map *m = med->map;
	Uint32 mx, my;
	
	if (med->cursor_dir.current != 0) {
		return;
	}
	
	mx = (m->view->mapx + ev->button.x / m->tilew) - 1;
	my = (m->view->mapy + ev->button.y / m->tileh) - 1;

	if ((mx > 1 && my > 1) &&
	    (mx < m->mapw && my < m->maph)) {
		dprintf("move to %dx%d\n", mx, my);
		mapedit_move(med, mx, my);
		mapedit_sticky(med);
		if (ev->button.button == 3) {
			mapedit_push(med, &m->map[mx][my], med->curoffs,
			    med->curflags);
		}
		m->redraw++;
	}
}

