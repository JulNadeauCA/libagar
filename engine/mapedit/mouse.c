/*	$Csoft	    */

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

/* XXX use generic input, remapping */

#include <engine/engine.h>
#include <engine/map.h>
#include <engine/physics.h>

#include "mapedit.h"
#include "command.h"
#include "mouse.h"

static void	mouse_tlsel(struct mapedit *, Uint32);
static void	mouse_objlsel(struct mapedit *, Uint32);

/*
 * Map editor mouse motion handler.
 * Map editor and map must be locked, called in event context.
 */
void
mouse_motion(struct mapedit *med, SDL_Event *ev)
{
	static int ommapx, ommapy;
	static int omtmapx, omtmapy;
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
	mx = (m->view->mapx + med->mmapx) - m->view->mapxoffs;
	my = (m->view->mapy + med->mmapy) - m->view->mapyoffs;

	if (med->x == mx && med->y == my) {
		/* Nothing to do. */
		return;
	}

	ms = SDL_GetMouseState(NULL, NULL);

	/* Tile stack. No functionality. */
	if (med->mmapx == 0) {
		return;
	}
	
	/* Object list. Allow selection/scrolling. */
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
			med->redraw++;
		}
		return;
	}
	
	/* Tile list. Allow selection/scrolling. */
	if (med->mmapx == m->view->mapw - 1 &&	/* XXX */
	    med->mtmapy <= m->view->maph) {
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
			med->redraw++;
		}
		return;
	}

	/* Map view. Node operations. */
	if (med->mmapy + m->view->mapyoffs < m->view->maph &&
	    med->mmapx + m->view->mapxoffs < m->view->mapw + 2 &&
	    med->mmapx >= m->view->mapxoffs &&
	    med->mmapy >= m->view->mapyoffs) {
		if (ms & SDL_BUTTON_MMASK) {
			/* Move */
			mapedit_move(med, mx, my);
			mapedit_sticky(med);
			med->redraw++;
		} else if (ms & SDL_BUTTON_RMASK) {
			/* Add/move */
			mapedit_push(med, &m->map[my][mx], med->curoffs,
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
 * Map editor and map must be locked, called in event context.
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
	mx = (m->view->mapx + vx) - m->view->mapxoffs;
	my = (m->view->mapy + vy) - m->view->mapyoffs;

	/* XXX redundant */
	if (vy == 0) {
		/* Object list. Allow selection. */
		switch (ev->button.button) {
		case 2:
		case 3:
			mouse_objlsel(med, vx);
			med->redraw++;
			break;
		}
	} else if (vx >= m->view->mapw - 1 && vy <= m->view->maph) {  /* XXX? */
		/* Tile list. Allow selection. */
		switch (ev->button.button) {
		case 2:
		case 3:
			mouse_tlsel(med, vy);
			med->redraw++;
			break;
		}
	} else if (m->view->mapxoffs+vx <= m->view->mapw+1 &&
	    m->view->mapyoffs+vy < m->view->maph &&
	    vx >= m->view->mapxoffs &&
	    vy >= m->view->mapyoffs) {
		/* Map view. Node operations. */
	    	switch (ev->button.button) {
		case 2:
			mapedit_move(med, mx, my);
			mapedit_sticky(med);
			m->redraw++;
			break;
		case 3:
			mapedit_move(med, mx, my);
			mapedit_push(med, &m->map[my][mx], med->curoffs,
			    med->curflags);
			m->redraw++;
			break;
		}
	}
}

/*
 * Select a node on the tile list.
 * Map editor and map must be locked.
 */
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
	while (med->curoffs > med->curobj->nrefs - 1) {
		med->curoffs -= med->curobj->nrefs;
	}
}

/*
 * Select a node on the obj list.
 * Map editor and map must be locked.
 */
static void
mouse_objlsel(struct mapedit *med, Uint32 vx)
{
	struct editobj *eob;
	int curoffs;

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
	med->curoffs = 0;
	med->tilelist_offs = 0;
}

