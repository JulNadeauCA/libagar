/*	$Csoft: mapview.c,v 1.129 2003/07/09 01:59:42 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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
#include <engine/physics.h>
#include <engine/map.h>
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/button.h>
#include <engine/widget/label.h>
#include <engine/widget/tlist.h>

#include "mapedit.h"
#include "mapview.h"

#include "tool/tool.h"
#include "tool/shift.h"

const struct widget_ops mapview_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		mapview_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	mapview_draw,
	mapview_scale
};

enum {
	MAPVIEW_ZOOM_MIN =	4,	/* Min zoom factor (%) */
	MAPVIEW_ZOOM_MAX =	600,	/* Max zoom factor (%) */
	MAPVIEW_TILE_MAX =	16384,	/* Max tile geometry (pixels) */
	MAPVIEW_ZOOM_INC =	8,	/* Zoom increment (%) */
	MAPVIEW_ZOOM_IVAL =	80,	/* Zoom interval (ms) */
	MAPVIEW_MIN_PROP_ZOOM =	60	/* Minimum zoom to draw props (%) */
};

enum {
	BORDER_COLOR,
	GRID_COLOR,
	CURSOR_COLOR,
	TSETORIG_COLOR,
	SRCNODE_COLOR,
	BG1_COLOR,
	BG2_COLOR,
	MSEL_COLOR,
	ESEL_COLOR
};

int	mapview_bg = 1;
int	mapview_bg_moving = 1;
int	mapview_bg_squaresize = 16;

static void	mapview_lostfocus(int, union evarg *);
static void	mapview_mousemotion(int, union evarg *);
static void	mapview_mousebuttondown(int, union evarg *);
static void	mapview_mousebuttonup(int, union evarg *);
static void	mapview_keyup(int, union evarg *);
static void	mapview_keydown(int, union evarg *);
static void	mapview_begin_selection(struct mapview *);
static void	mapview_effect_selection(struct mapview *);

static __inline__ int
mapview_selbounded(struct mapview *mv, int x, int y)
{
	return (!prop_get_bool(&mapedit, "sel-bounded-edition") ||
	    !mv->esel.set ||
	    (x >= mv->esel.x &&
	     y >= mv->esel.y &&
	     x <  mv->esel.x + mv->esel.w &&
	     y <  mv->esel.y + mv->esel.h));
}

static __inline__ int
mapview_selecting(void)
{
	return (mapedit.curtool == mapedit.tools[MAPEDIT_SELECT] ||
	    (SDL_GetModState() & KMOD_CTRL));
}

struct mapview *
mapview_new(void *parent, struct map *m, int flags)
{
	struct mapview *mv;

	mv = Malloc(sizeof(struct mapview));
	mapview_init(mv, m, flags);
	object_attach(parent, mv);
	return (mv);
}

void
mapview_init(struct mapview *mv, struct map *m, int flags)
{
	widget_init(mv, "mapview", &mapview_ops,
	    WIDGET_FOCUSABLE|WIDGET_CLIPPING|WIDGET_WFILL|WIDGET_HFILL);

	if (gfx_fetch(mv, "/engine/mapedit/mapview/mapview") == -1) {
		fatal("%s", error_get());
	}
	gfx_wire(OBJECT(mv)->gfx);

	mv->flags = (flags | MAPVIEW_CENTER);
	mv->mw = 0;					/* Set on scale */
	mv->mh = 0;
	mv->prew = 4;
	mv->preh = 4;

	mv->prop_style = 0;
	mv->mouse.scrolling = 0;
	mv->mouse.centering = 0;
	mv->mouse.x = 0;
	mv->mouse.y = 0;

	mv->nodeed.trigger = NULL;
	mv->layed.trigger = NULL;

	mv->zoom_inc = MAPVIEW_ZOOM_INC;
	mv->zoom_ival = MAPVIEW_ZOOM_IVAL;
	mv->zoom_tm = NULL;
	mv->map = m;

	pthread_mutex_lock(&m->lock);
	mv->mx = m->origin.x;
	mv->my = m->origin.y;
	mv->cx = -1;
	mv->cy = -1;
	mv->cxrel = 0;
	mv->cyrel = 0;
	mv->msel.set = 0;
	mv->msel.x = 0;
	mv->msel.y = 0;
	mv->msel.xoffs = 0;
	mv->msel.yoffs = 0;
	mv->esel.set = 0;
	mv->esel.x = 0;
	mv->esel.y = 0;
	mv->esel.w = 0;
	mv->esel.h = 0;
	if (mv->flags & MAPVIEW_INDEPENDENT) {
		mv->izoom.zoom = 100;
		mv->izoom.tilew = TILEW;
		mv->izoom.tileh = TILEH;
		mv->izoom.ssx = TILEW;
		mv->izoom.ssy = TILEH;
		mv->zoom = &mv->izoom.zoom;
		mv->tilew = &mv->izoom.tilew;
		mv->tileh = &mv->izoom.tileh;
		mv->ssx = &mv->izoom.ssx;
		mv->ssy = &mv->izoom.ssy;
	} else {
		mv->zoom = &m->zoom;
		mv->tilew = &m->tilew;
		mv->tileh = &m->tileh;
		mv->ssx = &m->ssx;
		mv->ssy = &m->ssy;
	}
	pthread_mutex_unlock(&m->lock);

	/* The map editor is required for tilesets/edition. */
	if (!mapedition && (mv->flags & (MAPVIEW_TILESET|MAPVIEW_EDIT)))
		fatal("no map editor");

	widget_map_color(mv, BORDER_COLOR, "border", 200, 200, 200, 255);
	widget_map_color(mv, GRID_COLOR, "grid", 100, 100, 100, 255);
	widget_map_color(mv, CURSOR_COLOR, "cursor", 100, 100, 100, 255);
	widget_map_color(mv, TSETORIG_COLOR, "tset-orig", 100, 100, 130, 255);
	widget_map_color(mv, SRCNODE_COLOR, "src-node", 0, 190, 0, 255);
	widget_map_color(mv, BG1_COLOR, "background-1", 24, 24, 24, 255);
	widget_map_color(mv, BG2_COLOR, "background-2", 37, 34, 38, 255);
	widget_map_color(mv, MSEL_COLOR, "mouse-sel", 150, 150, 150, 255);
	widget_map_color(mv, ESEL_COLOR, "effective-sel", 180, 180, 180, 255);

	event_new(mv, "widget-lostfocus", mapview_lostfocus, NULL);
	event_new(mv, "widget-hidden", mapview_lostfocus, NULL);
	event_new(mv, "window-keyup", mapview_keyup, NULL);
	event_new(mv, "window-keydown", mapview_keydown, NULL);
	event_new(mv, "window-mousemotion", mapview_mousemotion, NULL);
	event_new(mv, "window-mousebuttonup", mapview_mousebuttonup, NULL);
	event_new(mv, "window-mousebuttondown", mapview_mousebuttondown, NULL);

	if (mapedition) {
		nodeedit_init(mv);
		layedit_init(mv);
	}
}

void
mapview_destroy(void *p)
{
	struct mapview *mv = p;

	if (mapedition) {
		view_detach(mv->nodeed.win);
		view_detach(mv->layed.win);
	}
}

/*
 * Translate widget coordinates to map coordinates.
 * The map must be locked.
 */
void
mapview_map_coords(struct mapview *mv, int *x, int *y)
{
	*x -= *mv->ssx - *mv->tilew;
	*y -= *mv->ssy - *mv->tileh;
	*x /= *mv->tilew;
	*y /= *mv->tileh;
	
	mv->cx = mv->mx + *x;
	mv->cy = mv->my + *y;

	if (mv->cx < 0 || mv->cx >= mv->map->mapw)
		mv->cx = -1;
	if (mv->cy < 0 || mv->cy >= mv->map->maph)
		mv->cy = -1;
}

void
mapview_draw_props(struct mapview *mv, struct node *node, int x, int y,
    int mx, int my)
{
	const struct {
		Uint32	flag;
		Uint32	sprite;
	} flags[] = {
		{ NODEREF_WALK,		MAPVIEW_WALK },
		{ NODEREF_CLIMB,	MAPVIEW_CLIMB },
		{ NODEREF_BIO,		MAPVIEW_BIO },
		{ NODEREF_REGEN,	MAPVIEW_REGEN },
		{ NODEREF_SLOW,		MAPVIEW_SLOW },
		{ NODEREF_HASTE,	MAPVIEW_HASTE },
	};
	const struct {
		Uint32	edge;
		Uint32	sprite;
	} edges[] = {
		{ NODEREF_EDGE_E,	MAPVIEW_EDGE_E },
		{ NODEREF_EDGE_N,	MAPVIEW_EDGE_N },
		{ NODEREF_EDGE_S,	MAPVIEW_EDGE_S },
		{ NODEREF_EDGE_W,	MAPVIEW_EDGE_W },
		{ NODEREF_EDGE_NW,	MAPVIEW_EDGE_NW },
		{ NODEREF_EDGE_NE,	MAPVIEW_EDGE_NE },
		{ NODEREF_EDGE_SW,	MAPVIEW_EDGE_SW },
		{ NODEREF_EDGE_SE,	MAPVIEW_EDGE_SE }
	};
	const int nflags = sizeof(flags) / sizeof(flags[0]);
	const int nedges = sizeof(edges) / sizeof(edges[0]);
	struct noderef *r;
	int i;

	if (mv->prop_style > 0)
		widget_blit(mv, SPRITE(&mapedit, mv->prop_style), x, y);

	if (mx == mv->map->origin.x && my == mv->map->origin.y) {
		widget_blit(mv, SPRITE(mv, MAPVIEW_ORIGIN), x, y);
		x += SPRITE(mv,MAPVIEW_ORIGIN)->w;
	}

	TAILQ_FOREACH(r, &node->nrefs, nrefs) {
		if (r->layer != mv->map->cur_layer)
			continue;

		for (i = 0; i < nflags; i++) {
			if ((r->flags & flags[i].flag) == 0) {
				continue;
			}
			widget_blit(mv, SPRITE(mv, flags[i].sprite), x, y);
			x += SPRITE(mv,flags[i].sprite)->w;
		}
		for (i = 0; i < nedges; i++) {
			if (r->r_gfx.edge != edges[i].edge) {
				continue;
			}
			widget_blit(mv, SPRITE(mv, edges[i].sprite), x, y);
			x += SPRITE(mv, edges[i].sprite)->w;
		}
	}
}

static __inline__ void
mapview_draw_tool_cursor(struct mapview *mv)
{
	struct tool *curtool = mapedit.curtool;
	SDL_Rect rd;
	int msx, msy;

	rd.w = mv->map->tilew;
	rd.h = mv->map->tileh;

	if (SDL_GetModState() & KMOD_CTRL) {		/* XXX inefficient? */
		SDL_GetMouseState(&msx, &msy);
		rd.x = msx;
		rd.y = msy;
		SDL_BlitSurface(
		    SPRITE(mapedit.tools[MAPEDIT_SELECT], TOOL_SELECT_CURSOR),
		    NULL, view->v, &rd);
		return;
	}
	
	rd.x = mv->mouse.x*mv->map->tilew - mv->map->tilew + *mv->ssx;
	rd.y = mv->mouse.y*mv->map->tileh - mv->map->tileh + *mv->ssy;

	if (curtool == NULL)
		goto defcurs;

	if (curtool->cursor != NULL) {
		rd.x += WIDGET(mv)->cx;
		rd.y += WIDGET(mv)->cy;
		SDL_BlitSurface(curtool->cursor, NULL, view->v, &rd);
	} else {
		if (TOOL_OPS(curtool)->cursor == NULL ||
		    TOOL_OPS(curtool)->cursor(curtool, mv, &rd) == -1)
			goto defcurs;
	}
	return;
defcurs:
	primitives.rect_outlined(mv,
	    rd.x + 1,
	    rd.y + 1,
	    mv->map->tilew - 1,
	    mv->map->tileh - 1,
	    CURSOR_COLOR);
	primitives.rect_outlined(mv,
	    rd.x + 2,
	    rd.y + 2,
	    mv->map->tilew - 3,
	    mv->map->tileh - 3,
	    CURSOR_COLOR);
}

static __inline__ void
mapview_draw_background(struct mapview *mv)
{
	static int softbg = 0;
	int alt1 = 0, alt2 = 0;
	int x, y;

	if (mapview_bg_moving && ++softbg > mapview_bg_squaresize-1) {
		softbg = 0;
	}
	for (y = -mapview_bg_squaresize + softbg;
	     y < WIDGET(mv)->h;
	     y += mapview_bg_squaresize) {
		for (x = -mapview_bg_squaresize + softbg;
		     x < WIDGET(mv)->w;
		     x += mapview_bg_squaresize) {
			if (alt1++ == 1) {
				primitives.rect_filled(mv,
				    x,
				    y,
				    mapview_bg_squaresize,
				    mapview_bg_squaresize,
				    BG1_COLOR);
				alt1 = 0;
			} else {
				primitives.rect_filled(mv,
				    x,
				    y,
				    mapview_bg_squaresize,
				    mapview_bg_squaresize,
				    BG2_COLOR);
			}
		}
		if (alt2++ == 1) {
			alt2 = 0;
		}
		alt1 = alt2;
	}
}

void
mapview_draw(void *p)
{
	struct mapview *mv = p;
	struct map *m = mv->map;
	struct node *node;
	struct noderef *nref;
	int mx, my, rx, ry;
	Uint16 old_zoom = m->zoom;
	int old_tilew = m->tilew;
	int old_tileh = m->tileh;
	int layer = 0;
	int esel_x = -1, esel_y = -1, esel_w = -1, esel_h = -1;
	int msel_x = -1, msel_y = -1, msel_w = -1, msel_h = -1;

	if (mapview_bg)
		mapview_draw_background(mv);

	pthread_mutex_lock(&m->lock);

	if (mv->flags & MAPVIEW_INDEPENDENT) {
		m->zoom = *mv->zoom;
		m->tilew = *mv->tilew;
		m->tileh = *mv->tileh;
	}

draw_layer:
	if (!m->layers[layer].visible) {
		goto next_layer;
	}
	for (my = mv->my, ry = *mv->ssy - mv->map->tileh;
	     ((my - mv->my) < mv->mh+2) && (my < m->maph);
	     my++, ry += mv->map->tileh) {

		for (mx = mv->mx, rx = *mv->ssx - mv->map->tilew;
	     	     ((mx - mv->mx) < (mv->mw+2)) && (mx < m->mapw);
		     mx++, rx += mv->map->tilew) {
			node = &m->map[my][mx];
			TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
				if (nref->layer == layer) {
					noderef_draw(m, nref,
					    WIDGET(mv)->cx + rx,
					    WIDGET(mv)->cy + ry);
				}
			}

			if (mv->flags & MAPVIEW_PROPS &&
			    mv->map->zoom >= MAPVIEW_MIN_PROP_ZOOM) {
				mapview_draw_props(mv, node, rx, ry, mx, my);
			}
			if (mv->flags & MAPVIEW_GRID && mv->map->zoom >= 8) {
				/* XXX overdraw */
				primitives.rect_outlined(mv,
				    rx,
				    ry,
				    mv->map->tilew + 1,
				    mv->map->tileh + 1,
				    GRID_COLOR);
			}

			if (!mapedition)
				continue;

			/* Indicate the mouse/effective selections. */
			if (mv->msel.set &&
			    mv->msel.x == mx && mv->msel.y == my) {
				msel_x = rx + 1;
				msel_y = ry + 1;
				msel_w = mv->msel.xoffs*mv->map->tilew - 2;
				msel_h = mv->msel.yoffs*mv->map->tileh - 2;
			}
			
			if (mv->esel.set &&
			    mv->esel.x == mx && mv->esel.y == my) {
				esel_x = rx;
				esel_y = ry;
				esel_w = mv->map->tilew*mv->esel.w;
				esel_h = mv->map->tileh*mv->esel.h;
			}
			if (mv->map->tilew < 7 || mv->map->tileh < 7)
				continue;

			/* Indicate the source node selection. XXX selection? */
			if ((mv->flags & MAPVIEW_TILESET)) {
				/* Source node? XXX use selections */
				if (node == mapedit.src_node) {
					primitives.rect_outlined(mv,
					    rx + 1,
					    ry + 1,
					    mv->map->tilew - 1,
					    mv->map->tileh - 1,
					    SRCNODE_COLOR);
				}
			}
		}
	}
next_layer:
	if (++layer < m->nlayers) {
		goto draw_layer;			/* Draw next layer */
	}
	
	/* Indicate the selection. */
	if (esel_x != -1) {
		primitives.rect_outlined(mv,
		    esel_x, esel_y,
		    esel_w, esel_h,
		    ESEL_COLOR);
	}
	if (msel_x != -1) {
		primitives.rect_outlined(mv,
		    msel_x, msel_y,
		    msel_w, msel_h,
		    MSEL_COLOR);
	}

	/* Draw the cursor for the current tool. */
	if ((mv->flags & MAPVIEW_EDIT) &&
	    (mv->flags & MAPVIEW_NO_CURSOR) == 0 &&
	    (mv->cx != -1 && mv->cy != -1)) {
		mapview_draw_tool_cursor(mv);
	}
	if (mv->flags & MAPVIEW_INDEPENDENT) {
		m->zoom = old_zoom;			/* Restore zoom */
		m->tilew = old_tilew;
		m->tileh = old_tileh;
	}
	pthread_mutex_unlock(&m->lock);
}

int
mapview_zoom(struct mapview *mv, int zoom)
{
	if (zoom < MAPVIEW_ZOOM_MIN || zoom > MAPVIEW_ZOOM_MAX) {
		error_set("zoom out of range");
		return (-1);
	}
#if 0
	/* XXX */
	pthread_mutex_lock(&mv->map->lock);
#endif
	*mv->zoom = zoom;
	*mv->tilew = zoom * TILEW / 100;
	*mv->tileh = zoom * TILEH / 100;
	if (*mv->tilew > MAPVIEW_TILE_MAX)
		*mv->tilew = MAPVIEW_TILE_MAX;
	if (*mv->tileh > MAPVIEW_TILE_MAX)
		*mv->tileh = MAPVIEW_TILE_MAX;
	mv->mw = WIDGET(mv)->w/(*mv->tilew) + 1;
	mv->mh = WIDGET(mv)->h/(*mv->tileh) + 1;
#if 0
	window_set_caption(WIDGET(mv)->win, "%s (%d%%)",
	    OBJECT(mv->map)->name, *mv->zoom);
#endif
#if 0
	/* XXX */
	pthread_mutex_unlock(&mv->map->lock);
#endif
	return (0);
}

static Uint32
mapview_zoom_tick(Uint32 ival, void *p)
{
	struct mapview *mv = p;

#if 0
	/* XXX */
	pthread_mutex_lock(&mv->map->lock);
#endif

	if (mv->flags & MAPVIEW_ZOOMING_IN) {
		mapview_zoom(mv, *mv->zoom + mv->zoom_inc);
	} else if (mv->flags & MAPVIEW_ZOOMING_OUT) {
		mapview_zoom(mv, *mv->zoom - mv->zoom_inc);
	}
#if 0
	/* XXX */
	pthread_mutex_unlock(&mv->map->lock);
#endif
	return (ival);
}

static __inline__ void
mapview_mouse_scroll(struct mapview *mv, int xrel, int yrel)
{
	int max;

	if (xrel > 0 && (*mv->ssx += xrel) >= *mv->tilew) {
		if (--mv->mx < 0) {
			mv->mx = 0;
		} else {
			*mv->ssx = 0;
		}
	} else if (xrel < 0 && ((*mv->ssx) += xrel) <= -(*mv->tilew)) {
		if (mv->mw < mv->map->mapw) {
			max = mv->map->mapw - mv->mw - 1;
			if (++mv->mx > max) {
				mv->mx = max;
			} else {
				*mv->ssx = 0;
			}
		}
	}
	if (yrel > 0 && ((*mv->ssy) += yrel) >= *mv->tileh) {
		if (--mv->my < 0) {
			mv->my = 0;
		} else {
			*mv->ssy = 0;
		}
	} else if (yrel < 0 && (*mv->ssy += yrel) <= -(*mv->tileh)) {
		if (mv->mh < mv->map->maph) {
			max = mv->map->maph - mv->mh - 1;
			if (++mv->my > max) {
				mv->my = max;
			} else {
				*mv->ssy = 0;
			}
		}
	}
#if 0
	if (*mv->ssx > *mv->tilew)	*mv->ssx = *mv->tilew;
	if (*mv->ssy > *mv->tileh)	*mv->ssy = *mv->tileh;
#endif
}

static void
mapview_mousemotion(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;
	int x = argv[1].i;
	int y = argv[2].i;
	int xrel = argv[3].i;
	int yrel = argv[4].i;
	Uint8 state;
	
	state = SDL_GetMouseState(NULL, NULL);

	pthread_mutex_lock(&mv->map->lock);

	mapview_map_coords(mv, &x, &y);
	mv->cxrel = x - mv->mouse.x;
	mv->cyrel = y - mv->mouse.y;

	if (mv->mouse.scrolling) {				/* Scrolling */
		mapview_mouse_scroll(mv, xrel, yrel);
	} else if (mv->msel.set) {				/* Selection */
		mv->msel.xoffs += x - mv->mouse.x;
		mv->msel.yoffs += y - mv->mouse.y;
	} else if (mv->flags & MAPVIEW_EDIT) {			/* Edition */
		if (state & SDL_BUTTON(2)) {
			/* Always invoke the 'shift' tool. */
			shift_mouse(mapedit.tools[MAPEDIT_SHIFT], mv,
			    xrel, yrel);
		} else if (mapedit.curtool != NULL && (state & SDL_BUTTON(1))) {
			struct tool *tool = mapedit.curtool;

			/* Invoke 'effect' tool operations. */
			if (TOOL_OPS(tool)->effect != NULL &&
			    mv->cx != -1 && mv->cy != -1 &&
			    (x != mv->mouse.x || y != mv->mouse.y) &&
			    (mapview_selbounded(mv, mv->cx, mv->cy))) {
				TOOL_OPS(tool)->effect(tool, mv, mv->map,
				    &mv->map->map[mv->cy][mv->cx]);
			}
			/* Invoke 'mouse' tool operations. */
			if (TOOL_OPS(tool)->mouse != NULL) {
				TOOL_OPS(tool)->mouse(tool, mv, xrel, yrel,
				    state);
			}
		}
	} else if (mv->flags & MAPVIEW_TILESET) {		/* Source */
		if ((state & SDL_BUTTON(1)) &&
		    (mv->cx != -1) && (mv->cy != -1)) {
			struct node *srcnode = &mv->map->map[mv->cy][mv->cx];

			if (!TAILQ_EMPTY(&srcnode->nrefs)) {
				mapedit.src_node = srcnode;
			}
		}
	}
	pthread_mutex_unlock(&mv->map->lock);

	if (!mv->mouse.centering) {
		mv->mouse.x = x;
		mv->mouse.y = y;
	}
}

static void
mapview_mousebuttondown(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;
	struct node *curnode;
	
	widget_focus(mv);

	pthread_mutex_lock(&mv->map->lock);
	mapview_map_coords(mv, &x, &y);
	if (mv->cx < 0 || mv->cy < 0)
		goto out;
	curnode = &mv->map->map[mv->cy][mv->cx];

	switch (button) {
	case 1:						/* Select/edit */
		if (mapview_selecting()) {
			mapview_begin_selection(mv);
			goto out;
		}
		break;
	case 2:						/* Adjust centering */
		mv->flags |= MAPVIEW_NO_CURSOR;
		mv->mouse.centering++;
		goto out;
	case 3:						/* Scroll */
		mv->mouse.scrolling++;
		goto out;
	}

	if (mv->flags & MAPVIEW_EDIT) {
		if (mapedit.curtool != NULL &&
		    TOOL_OPS(mapedit.curtool)->effect != NULL &&
		    mapview_selbounded(mv, mv->cx, mv->cy)) {
			TOOL_OPS(mapedit.curtool)->effect(mapedit.curtool,
			    mv, mv->map, curnode);
		}
	}
	if (mv->flags & MAPVIEW_TILESET) {
		if (!TAILQ_EMPTY(&curnode->nrefs))
			mapedit.src_node = curnode;
	}
out:
	pthread_mutex_unlock(&mv->map->lock);
}

/* Begin a mouse selection. */
static void
mapview_begin_selection(struct mapview *mv)
{
	mv->msel.set = 1;
	mv->msel.x = mv->cx;
	mv->msel.y = mv->cy;
	mv->msel.xoffs = 1;
	mv->msel.yoffs = 1;
}

/* Effect a mouse selection. */
static void
mapview_effect_selection(struct mapview *mv)
{
	int excess;

	mv->esel.x = mv->msel.x;
	mv->esel.y = mv->msel.y;
	mv->esel.w = mv->msel.xoffs;
	mv->esel.h = mv->msel.yoffs;

	if (mv->msel.xoffs < 0) {
		mv->esel.x += mv->msel.xoffs;
		mv->esel.w = -mv->msel.xoffs;
	}
	if (mv->msel.yoffs < 0) {
		mv->esel.y += mv->msel.yoffs;
		mv->esel.h = -mv->msel.yoffs;
	}

	if ((excess = (mv->esel.x + mv->esel.w) - mv->map->mapw) > 0) {
		if (excess < mv->esel.w)
			mv->esel.w -= excess;
	}
	if ((excess = (mv->esel.y + mv->esel.h) - mv->map->maph) > 0) {
		if (excess < mv->esel.h)
			mv->esel.h -= excess;
	}

	if (mv->esel.x < 0) {
		mv->esel.w += mv->esel.x;
		mv->esel.x = 0;
	}
	if (mv->esel.y < 0) {
		mv->esel.h += mv->esel.y;
		mv->esel.y = 0;
	}

	mv->esel.set = 1;
}

static void
mapview_mousebuttonup(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;

	switch (argv[1].i) {
	case 1:
		if (mapview_selecting() &&
		    mv->msel.set &&
		    (mv->msel.xoffs == 0 || mv->msel.yoffs == 0)) {
			mv->esel.set = 0;
			mv->msel.set = 0;
		} else {
			if (mv->msel.set) {
				mapview_effect_selection(mv);
				mv->msel.set = 0;
			}
		}
		break;
	case 2:
		mv->flags &= ~(MAPVIEW_NO_CURSOR);
		mv->mouse.centering = 0;
		break;
	case 3:
		mv->mouse.scrolling = 0;
		break;
	}
}

static void
mapview_keyup(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;
	int keysym = argv[1].i;

	switch (keysym) {
	case SDLK_EQUALS:
		mv->flags &= ~MAPVIEW_ZOOMING_IN;
		break;
	case SDLK_MINUS:
		mv->flags &= ~MAPVIEW_ZOOMING_OUT;
		break;
	}
}

static void
mapview_keydown(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;
	int keysym = argv[1].i;
	int keymod = argv[2].i;

	pthread_mutex_lock(&mv->map->lock);

	switch (keysym) {
	case SDLK_EQUALS:
		if (mv->zoom_tm == NULL) {
			mapview_zoom(mv, *mv->zoom + mv->zoom_inc);
			mv->zoom_tm = SDL_AddTimer(mv->zoom_ival,
			    mapview_zoom_tick, mv);
		}
		mv->flags &= ~(MAPVIEW_ZOOMING_OUT);
		mv->flags |= MAPVIEW_ZOOMING_IN;
		break;
	case SDLK_MINUS:
		if (mv->zoom_tm == NULL) {
			mapview_zoom(mv, *mv->zoom - mv->zoom_inc);
			mv->zoom_tm = SDL_AddTimer(mv->zoom_ival,
			    mapview_zoom_tick, mv);
		}
		mv->flags &= ~(MAPVIEW_ZOOMING_OUT);
		mv->flags |= MAPVIEW_ZOOMING_OUT;
		break;
	case SDLK_0:
	case SDLK_1:
		mv->flags &= ~(MAPVIEW_ZOOMING_IN|MAPVIEW_ZOOMING_OUT);
		mapview_zoom(mv, 100);
		break;
	case SDLK_2:
		mapview_zoom(mv, 20);
		break;
	case SDLK_3:
		mapview_zoom(mv, 30);
		break;
	case SDLK_4:
		mapview_zoom(mv, 40);
		break;
	case SDLK_5:
		mapview_zoom(mv, 50);
		break;
	case SDLK_6:
		mapview_zoom(mv, 120);
		break;
	case SDLK_7:
		mapview_zoom(mv, 130);
		break;
	case SDLK_8:
		mapview_zoom(mv, 140);
		break;
	case SDLK_9:
		mapview_zoom(mv, 150);
		break;
	case SDLK_o:
		mapview_center(mv,
		    mv->map->origin.x, mv->map->origin.y);
		break;
	}

	if (mapedition) {
		struct tool_binding *binding;
		int i;

		for (i = 0; i < MAPEDIT_NTOOLS; i++) {
			struct tool *tool = mapedit.tools[i];

			SLIST_FOREACH(binding, &tool->bindings, bindings) {
				if (binding->key == keysym &&
				    (binding->mod == KMOD_NONE ||
				     keymod & binding->mod)) {
					if (binding->edit &&
					   (mv->flags & MAPVIEW_EDIT) == 0) {
						continue;
					}
					binding->func(tool, mv);
				}
			}
		}
	}

	switch (keysym) {
	case SDLK_g:
		if (mv->flags & MAPVIEW_GRID) {
			mv->flags &= ~(MAPVIEW_GRID);
		} else {
			mv->flags |= MAPVIEW_GRID;
		}
		break;
	case SDLK_p:
		if (keymod & KMOD_SHIFT) {
			if (++mv->prop_style == MAPVIEW_FRAMES_END) {
				mv->prop_style = 0;
			}
		} else {
			if (mv->flags & MAPVIEW_PROPS) {
				mv->flags &= ~(MAPVIEW_PROPS);
			} else {
				mv->flags |= MAPVIEW_PROPS;
			}
		}
		break;
	}

	pthread_mutex_unlock(&mv->map->lock);
}

void
mapview_scale(void *p, int rw, int rh)
{
	struct mapview *mv = p;

	if (rw == -1 && rh == -1) {
		WIDGET(mv)->w = mv->prew * TILEW;
		WIDGET(mv)->h = mv->preh * TILEH;
	}

	pthread_mutex_lock(&mv->map->lock);
	mv->mw = WIDGET(mv)->w/(*mv->tilew) + 1;
	mv->mh = WIDGET(mv)->h/(*mv->tileh) + 1;
	if (mv->flags & MAPVIEW_CENTER) {
		mapview_center(mv, mv->map->origin.x, mv->map->origin.y);
		mv->flags &= ~(MAPVIEW_CENTER);
	}
	mapview_zoom(mv, *mv->zoom);
	pthread_mutex_unlock(&mv->map->lock);
}

static void
mapview_lostfocus(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;

	mv->flags &= ~(MAPVIEW_ZOOMING_IN|MAPVIEW_ZOOMING_OUT);
	if (mv->zoom_tm != NULL) {
		SDL_RemoveTimer(mv->zoom_tm);
		mv->zoom_tm = NULL;
	}
}

void
mapview_center(struct mapview *mv, int x, int y)
{
	*mv->ssx = *mv->tilew;
	*mv->ssy = *mv->tileh;
	mv->mx = x - mv->mw/2;
	mv->my = y - mv->mh/2;

	if (mv->mx < 0)
		mv->mx = 0;
	if (mv->my < 0)
		mv->my = 0;

#if 0
	/* XXX */
	pthread_mutex_lock(&mv->map->lock);
#endif

	if (mv->mx >= mv->map->mapw - mv->mw)
		mv->mx = mv->map->mapw - mv->mw;
	if (mv->my >= mv->map->maph - mv->mh)
		mv->my = mv->map->maph - mv->mh;

#if 0
	/* XXX */
	pthread_mutex_unlock(&mv->map->lock);
#endif
}

void
mapview_set_selection(struct mapview *mv, int x, int y, int w, int h)
{
	mv->msel.set = 0;
	mv->esel.set = 1;
	mv->esel.x = x;
	mv->esel.y = y;
	mv->esel.w = w;
	mv->esel.h = h;
}

int
mapview_get_selection(struct mapview *mv, int *x, int *y, int *w, int *h)
{
	if (mv->esel.set) {
		if (x != NULL)
			*x = mv->esel.x;
		if (y != NULL)
			*y = mv->esel.y;
		if (w != NULL)
			*w = mv->esel.w;
		if (h != NULL)
			*h = mv->esel.h;
		return (1);
	} else {
		return (0);
	}
}

void	
mapview_prescale(struct mapview *mv, int w, int h)
{
	mv->prew = w;
	mv->preh = h;
}
