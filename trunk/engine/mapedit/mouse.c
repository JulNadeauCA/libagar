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

static void	mouse_tlsel(struct mapedit *, Uint32);
static void	mouse_objlsel(struct mapedit *, Uint32);

/*
 * Map editor mouse motion handler.
 * Must be called on a locked map.
 */
void
mouse_motion(struct mapedit *med, SDL_Event *ev)
{
	static Uint32 ommapx, ommapy;
	static Uint32 omtmapx, omtmapy;
	struct map *m = med->map;
	Uint32 mx, my;
	Uint8 ms;

	ommapx = med->mmapx;
	ommapy = med->mmapy;
	omtmapx = med->mtmapx;
	omtmapy = med->mtmapy;

	med->mmapx = (ev->motion.x / m->tilew);
	med->mmapy = (ev->motion.y / m->tileh);
	med->mtmapx = med->mmapx;
	med->mtmapy = med->mmapy;
	mx = (m->view->mapx + med->mmapx) - 1;
	my = (m->view->mapy + med->mmapy) - 1;

	if (med->x == mx && med->y == my) {
		/* Nothing to do. */
		return;
	}

	ms = SDL_GetMouseState(NULL, NULL);

	/*
	 * Tile stack. No functionality.
	 */
	if (med->mmapx == 0) {
		return;
	}

	/*
	 * Object list. Allow selection/scrolling.
	 */
	if (med->mmapy == 0) {
		if (ms & (SDL_BUTTON_LMASK|SDL_BUTTON_MMASK)) {
			/* Scroll */
			if (med->mtmapx > omtmapx &&	/* Right */
			    --med->objlist_offs < 0) {
				med->objlist_offs = med->neobjs - 1;
			}
			if (med->mtmapx < omtmapx &&	/* Left */
			    ++med->objlist_offs > med->neobjs - 1) {
				med->objlist_offs = 0;
			}
			mapedit_objlist(med);
		} else if (ms & (SDL_BUTTON_RMASK)) {
			/* Select */
			mouse_objlsel(med, med->mtmapx);
		}
		return;
	}

	/*
	 * Tile list. Allow selection/scrolling.
	 */
	if (med->mmapx > m->view->mapw && med->mtmapy < m->view->maph + 1) {
		if (ms & (SDL_BUTTON_LMASK|SDL_BUTTON_MMASK)) {
			/* Scroll */
			if (med->mtmapy > omtmapy &&	/* Down */
			    --med->tilelist_offs < 0) {
				med->tilelist_offs = med->curobj->nrefs - 1;
			}
			if (med->mtmapy < omtmapy &&	/* Up */
			    ++med->tilelist_offs > med->curobj->nrefs - 1) {
				med->tilelist_offs = 0;
			}
			mapedit_tilelist(med);
		} else if (ms & (SDL_BUTTON_RMASK)) {
			/* Select */
			mouse_tlsel(med, med->mtmapy);
		}
		return;
	}

	/*
	 * Map view. Node operations.
	 */
	if (med->mmapy < m->view->maph && med->mmapy < m->view->mapy) {
		if (ms & SDL_BUTTON_MMASK) {
			/* Move */
			mapedit_move(med, mx, my);
			med->map->redraw++;
		} else if (ms & SDL_BUTTON_RMASK) {
			/* Add/move */
			mapedit_push(med, &m->map[mx][my], med->curoffs,
			    med->curflags);
			mapedit_move(med, mx, my);
			med->map->redraw++;
		} else if (ms & SDL_BUTTON_LMASK) {
			/* Move cursor */
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
}

/*
 * Map editor mouse button handler.
 * Must be called on a locked map.
 */
void
mouse_button(struct mapedit *med, SDL_Event *ev)
{
	struct map *m = med->map;
	Uint32 mx, my, vx, vy;
	
	if (med->cursor_dir.current != 0) {
		return;
	}

	vx = (ev->button.x / m->tilew);
	vy = (ev->button.y / m->tileh);
	mx = (m->view->mapx + vx) - 1;
	my = (m->view->mapy + vy) - 1;
	
	if (med->mmapy == 0) {
		/* Object list. Allow selection. */
		switch (ev->button.button) {
		case 2:
		case 3:
			mouse_objlsel(med, vx);
			mapedit_objlist(med);
			mapedit_tilelist(med);
			break;
		}
	} else if (vx > m->view->mapw && vy <= m->view->maph) {
		/* Tile list. Allow selection. */
		switch (ev->button.button) {
		case 2:
		case 3:
			mouse_tlsel(med, vy);
			mapedit_tilelist(med);
			break;
		}
	} else if ((mx > 1 && my > 1) && (mx < m->mapw && my < m->maph)) {
		/* Map view. Node operations. */
	    	switch (ev->button.button) {
		case 2:
			mapedit_move(med, mx, my);
			mapedit_sticky(med);
			m->redraw++;
			break;
		case 3:
			mapedit_move(med, mx, my);
			mapedit_push(med, &m->map[mx][my], med->curoffs,
			    med->curflags);
			m->redraw++;
			break;
		}
	}
}

/* Select a node on the tile list. */
static void
mouse_tlsel(struct mapedit *med, Uint32 vy)
{
	med->curoffs = med->tilelist_offs + (vy - 1);
	if (med->curoffs < 0) {
		/* Wrap */
		med->curoffs += med->tilelist_offs + med->curobj->nrefs;
	} else if (med->curoffs >= med->curobj->nrefs) {
		med->curoffs -= med->curobj->nrefs;
	}
}


/* Select a node on the obj list. */
static void
mouse_objlsel(struct mapedit *med, Uint32 vx)
{
	struct editobj *eob;
	Uint32 curoffs;

	curoffs = med->objlist_offs + (vx - 1);
	if (curoffs < 0) {
		/* Wrap */
		curoffs += med->tilelist_offs + med->neobjs;
	} else if (curoffs > med->neobjs) {
		curoffs -= med->neobjs;
	}
	while (curoffs > med->neobjs - 1) {
		curoffs -= med->neobjs;
	}

	TAILQ_INDEX(eob, &med->eobjsh, eobjs, curoffs);
	med->curobj = eob;
	if (med->curoffs > med->curobj->nrefs) {
		med->curoffs = 0;
	}
}

