/*	$Csoft: mapview.c,v 1.78 2003/03/05 02:16:32 vedge Exp $	*/

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
#include <engine/widget/text.h>

#include "mapedit.h"
#include "mapview.h"

#include "tool/tool.h"
#include "tool/shift.h"
#include "tool/eraser.h"
#include "tool/stamp.h"

static const struct widget_ops mapview_ops = {
	{
		widget_destroy,
		NULL,		/* load */
		NULL		/* save */
	},
	mapview_draw,
	NULL		/* update */
};

enum {
	BORDER_COLOR,
	GRID_COLOR,
	CURSOR_COLOR,
	POSITION_CURSOR_COLOR,
	CONSTR_ORIGIN_COLOR,
	SRC_NODE_COLOR,
	BACKGROUND1_COLOR,
	BACKGROUND2_COLOR
};

static void	mapview_scaled(int, union evarg *);
static void	mapview_lostfocus(int, union evarg *);
static void	mapview_scroll(struct mapview *, int);
static void	mapview_mousemotion(int, union evarg *);
static void	mapview_mousebuttondown(int, union evarg *);
static void	mapview_mousebuttonup(int, union evarg *);
static void	mapview_keyup(int, union evarg *);
static void	mapview_keydown(int, union evarg *);

struct mapview *
mapview_new(struct region *reg, struct map *m, int flags, int rw, int rh)
{
	struct mapview *mv;

	mv = emalloc(sizeof(struct mapview));
	mapview_init(mv, m, flags, rw, rh);
	region_attach(reg, mv);
	return (mv);
}

void
mapview_init(struct mapview *mv, struct map *m, int flags, int rw, int rh)
{
	widget_init(&mv->wid, "mapview", &mapview_ops, rw, rh);
	mv->wid.flags |= WIDGET_CLIPPING;

	mv->flags = flags | MAPVIEW_CENTER;
	mv->mw = 0;		/* Set on scale */
	mv->mh = 0;

	mv->prop_style = 0;
	mv->mouse.scrolling = 0;
	mv->mouse.x = 0;
	mv->mouse.y = 0;
	mv->constr.mode = MAPVIEW_CONSTR_VERT;
	mv->constr.x = 0;
	mv->constr.y = 0;
	mv->constr.nflags = NODEREF_SAVEABLE;
	mv->tmap_win = NULL;
	mv->tmap_insert = 0;
	mv->cur_node = NULL;
	mv->cur_layer = 0;
	mv->nodeed.trigger = NULL;
	mv->layed.trigger = NULL;
	mv->zoom_tm = NULL;
	
	mv->map = m;

	pthread_mutex_lock(&m->lock);

	mv->mx = m->defx;
	mv->my = m->defy;
	mv->cx = -1;
	mv->cy = -1;
	mv->cw = 1;
	mv->ch = 1;

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

	/* The map editor is required for tile maps/edition. */
	if (!mapedition && (mv->flags & (MAPVIEW_TILEMAP|MAPVIEW_EDIT))) {
		fatal("no map editor\n");
	}

	widget_map_color(mv, BORDER_COLOR, "border", 200, 200, 200);
	widget_map_color(mv, GRID_COLOR, "grid", 100, 100, 100);
	widget_map_color(mv, CURSOR_COLOR, "cursor", 100, 100, 100);
	widget_map_color(mv, POSITION_CURSOR_COLOR, "pos-cursor", 0, 100, 150);
	widget_map_color(mv, CONSTR_ORIGIN_COLOR, "constr-orig", 100, 100, 130);
	widget_map_color(mv, SRC_NODE_COLOR, "src-node", 0, 190, 0);
	widget_map_color(mv, BACKGROUND2_COLOR, "background-2", 75, 75, 75);
	widget_map_color(mv, BACKGROUND1_COLOR, "background-1", 14, 14, 14);

	event_new(mv, "widget-scaled", mapview_scaled, NULL);
	event_new(mv, "widget-lostfocus", mapview_lostfocus, NULL);
	event_new(mv, "widget-hidden", mapview_lostfocus, NULL);
	event_new(mv, "window-keyup", mapview_keyup, NULL);
	event_new(mv, "window-keydown", mapview_keydown, NULL);
	event_new(mv, "window-mousemotion", mapview_mousemotion, NULL);
	event_new(mv, "window-mousebuttonup", mapview_mousebuttonup, NULL);
	event_new(mv, "window-mousebuttondown", mapview_mousebuttondown, NULL);

	nodeedit_init(mv);
	layedit_init(mv);
}

/*
 * Translate widget coordinates to map coordinates.
 * The map must be locked.
 */
static __inline__ void
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

__inline__ void
mapview_draw_props(struct mapview *mv, struct node *node,
    int rx, int ry, int mx, int my)
{
	const struct {
		Uint32		flag;
		Uint32		sprite;
	} sprites[] = {
		{ NODE_ORIGIN,	MAPVIEW_ORIGIN },
		{ NODE_WALK,	MAPVIEW_WALK },
		{ NODE_CLIMB,	MAPVIEW_CLIMB },
		{ NODE_BIO,	MAPVIEW_BIO },
		{ NODE_REGEN,	MAPVIEW_REGEN },
		{ NODE_SLOW,	MAPVIEW_SLOW },
		{ NODE_HASTE,	MAPVIEW_HASTE },
		{ NODE_EDGE_E,	MAPVIEW_EDGE_E },
		{ NODE_EDGE_N,	MAPVIEW_EDGE_N },
		{ NODE_EDGE_S,	MAPVIEW_EDGE_S },
		{ NODE_EDGE_W,	MAPVIEW_EDGE_W },
		{ NODE_EDGE_NW,	MAPVIEW_EDGE_NW },
		{ NODE_EDGE_NE,	MAPVIEW_EDGE_NE },
		{ NODE_EDGE_SW,	MAPVIEW_EDGE_SW },
		{ NODE_EDGE_SE,	MAPVIEW_EDGE_SE }
	};
	const int nsprites = sizeof(sprites) / sizeof(sprites[0]);
	int i;
	int x = rx - WIDGET_ABSX(mv);
	int y = ry - WIDGET_ABSY(mv);

	if (mv->prop_style > 0) {
		widget_blit(mv, SPRITE(mv, mv->prop_style), x, y);
	}
	for (i = 0; i < nsprites; i++) {
		if ((node->flags & sprites[i].flag) ||
		    (sprites[i].flag == NODE_ORIGIN &&
		     mx != -1 && mx == mv->map->defx &&
		     my != -1 && my == mv->map->defy)) {
			widget_blit(mv, SPRITE(mv, sprites[i].sprite), x, y);
			x += SPRITE(mv, sprites[i].sprite)->w;
		}
	}
}

static __inline__ void
mapview_draw_tool_cursor(struct mapview *mv)
{
	struct tool *curtool = mapedit.curtool;
	int x = mv->mouse.x;
	int y = mv->mouse.y;

	mapview_map_coords(mv, &x, &y);

	if (mv->cx != -1 && mv->cy != -1) {
		SDL_Rect rd;

		rd.x = mv->mouse.x*mv->map->tilew - mv->map->tilew + *mv->ssx;
		rd.y = mv->mouse.y*mv->map->tileh - mv->map->tileh + *mv->ssy;
		rd.w = mv->map->tilew;
		rd.h = mv->map->tileh;

		if ((curtool == NULL ||
		    TOOL_OPS(curtool)->cursor == NULL ||
		    TOOL_OPS(curtool)->cursor(curtool, mv, &rd) == -1)) {
			primitives.rect_outlined(mv,
			    rd.x+1, rd.y+1,
			    mv->map->tilew-1, mv->map->tileh-1,
			    WIDGET_COLOR(mv, CURSOR_COLOR));
			primitives.rect_outlined(mv,
			    rd.x+2, rd.y+2,
			    mv->map->tilew-3, mv->map->tileh-3,
			    WIDGET_COLOR(mv, CURSOR_COLOR));
		}
	}
}

static __inline__ void
mapview_draw_background(struct mapview *mv)
{
	static int softbg = 0;
	int ss, alt1 = 0, alt2 = 0;
	int x, y;
	SDL_Rect rd;

	ss = prop_get_int(&mapedit, "tilemap-bg-square-size");
	if (prop_get_bool(&mapedit, "tilemap-bg-moving") &&
	    ++softbg > ss-1) {
		softbg = 0;
	}
	rd.w = ss;
	rd.h = ss;
	for (y = -ss + softbg; y < WIDGET(mv)->h; y += ss) {
		rd.y = y;
		for (x = -ss + softbg; x < WIDGET(mv)->w; x += ss) {
			rd.x = x;
			if (alt1++ == 1) {
				primitives.rect_filled(mv, &rd,
				    WIDGET_COLOR(mv, BACKGROUND1_COLOR));
				alt1 = 0;
			} else {
				primitives.rect_filled(mv, &rd,
				    WIDGET_COLOR(mv, BACKGROUND2_COLOR));
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
	int mx, my, rx, ry, alt = 0;
	Uint32 col;
	Uint16 old_zoom = m->zoom;
	int old_tilew = m->tilew;
	int old_tileh = m->tileh;
	int layer = 0;

	if (mapedition && prop_get_bool(&mapedit, "tilemap-bg")) {
		mapview_draw_background(mv);
	}

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
	     (my - mv->my) < mv->mh+2 && my < m->maph;
	     my++, ry += mv->map->tileh) {

		for (mx = mv->mx, rx = *mv->ssx - mv->map->tilew;
	     	     (mx - mv->mx) < mv->mw+2 && mx < m->mapw;
		     mx++, rx += mv->map->tilew) {
			node = &m->map[my][mx];

			MAP_CHECK_NODE(node, mx, my);

			TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
				MAP_CHECK_NODEREF(nref);

				if (nref->layer == layer) {
					noderef_draw(m, nref,
					    WIDGET_ABSX(mv) + rx,
					    WIDGET_ABSY(mv) + ry);
				}
			}

			if (mv->flags & MAPVIEW_PROPS && mv->map->zoom >= 60) {
				mapview_draw_props(mv, node,
				    WIDGET_ABSX(mv) + rx,
				    WIDGET_ABSY(mv) + ry,
				    mx, my);
			}
			if (mv->flags & MAPVIEW_GRID && mv->map->zoom >= 8) {
				/* XXX overdraw */
				primitives.frame(mv,
				    rx, ry,
				    mv->map->tilew+1, mv->map->tileh+1,
				    WIDGET_COLOR(mv, GRID_COLOR));
			}

			if (!mapedition ||
			    mv->map->tilew < 6 || mv->map->tileh < 6)
				continue;
			
			/* Draw the position cursor. */
			if (mv->cur_node == node) {
				primitives.rect_outlined(mv,
				    rx+3, ry+3,
				    mv->map->tilew-5, mv->map->tileh-5,
				    WIDGET_COLOR(mv, POSITION_CURSOR_COLOR));
			}
			/* Indicate the source node selection. */
			if (node == mapedit.src_node) {
				primitives.rect_outlined(mv,
				    rx+1, ry+1,
				    mv->cw*mv->map->tilew - 1,
				    mv->ch*mv->map->tileh - 1,
				    WIDGET_COLOR(mv, SRC_NODE_COLOR));
			}
			/* Indicate the construction origin. */
			if ((mv->flags & MAPVIEW_TILEMAP) &&
			    mv->constr.x == mx && mv->constr.y == my) {
				primitives.frame(mv,
				    rx+2, ry+2,
				    mv->map->tilew-3, mv->map->tileh-3,
				    WIDGET_COLOR(mv, CONSTR_ORIGIN_COLOR));
			}
		}
	}
next_layer:
	if (++layer < m->nlayers) {
		goto draw_layer;			/* Draw next layer */
	}

	if (mv->flags & MAPVIEW_EDIT && (mv->flags & MAPVIEW_NO_CURSOR) == 0) {
		mapview_draw_tool_cursor(mv);
	}

#if 0
	if (WIDGET_FOCUSED(mv) && WINDOW_FOCUSED(WIDGET(mv)->win)) {
		primitives.rect_outlined(mv,
		    0, 0,
		    WIDGET(mv)->w, WIDGET(mv)->h,
		    WIDGET_COLOR(mv, BORDER_COLOR));
	}
#endif
	if (mv->flags & MAPVIEW_INDEPENDENT) {
		m->zoom = old_zoom;			/* Restore zoom */
		m->tilew = old_tilew;
		m->tileh = old_tileh;
	}
	pthread_mutex_unlock(&m->lock);
}

void
mapview_zoom(struct mapview *mv, int zoom)
{
	if (mapedition &&
	    (zoom < prop_get_int(&mapedit, "zoom-minimum") ||
	     zoom > prop_get_int(&mapedit, "zoom-maximum"))) {
		return;
	}
#if 0
	/* XXX */
	pthread_mutex_lock(&mv->map->lock);
#endif
	*mv->zoom = zoom;
	*mv->tilew = zoom * TILEW / 100;
	*mv->tileh = zoom * TILEH / 100;
	if (*mv->tilew > 16384)			/* For soft scrolling */
		*mv->tilew = 16384;
	if (*mv->tileh > 16384)
		*mv->tileh = 16384;
	mv->mw = WIDGET(mv)->w/(*mv->tilew) + 1;
	mv->mh = WIDGET(mv)->h/(*mv->tileh) + 1;

	window_set_caption(WIDGET(mv)->win, "%s (%d%%)",
	    OBJECT(mv->map)->name, *mv->zoom);

#if 0
	/* XXX */
	pthread_mutex_unlock(&mv->map->lock);
#endif
}

static Uint32
mapview_zoom_tick(Uint32 ival, void *p)
{
	struct mapview *mv = p;
	int incr;

#if 0
	/* XXX */
	pthread_mutex_lock(&mv->map->lock);
#endif

	if (mv->flags & MAPVIEW_ZOOMING_IN) {
 		incr = mapedition ?
		    prop_get_int(&mapedit, "zoom-increment") : 8;
		mapview_zoom(mv, *mv->zoom + incr);
	} else if (mv->flags & MAPVIEW_ZOOMING_OUT) {
 		incr = mapedition ?
		    prop_get_int(&mapedit, "zoom-increment") : 8;
		mapview_zoom(mv, *mv->zoom - incr);
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
	if (xrel > 0 && (*mv->ssx += xrel) >= *mv->tilew) {
		if (--mv->mx < 0) {
			mv->mx = 0;
			if (mv->mw < mv->map->mapw)
				*mv->ssx = *mv->tilew;
		} else {
			*mv->ssx = 0;
		}
	} else if (xrel < 0 && ((*mv->ssx) += xrel) <= -(*mv->tilew)) {
		if (mv->mw < mv->map->mapw) {
			if (++mv->mx > mv->map->mapw - mv->mw - 1) {
				mv->mx = mv->map->mapw - mv->mw - 1;
				*mv->ssx = -(*mv->tilew);
			} else {
				*mv->ssx = 0;
			}
		}
	}
	if (yrel > 0 && ((*mv->ssy) += yrel) >= *mv->tileh) {
		if (--mv->my < 0) {
			mv->my = 0;
			if (mv->mh < mv->map->maph)
				*mv->ssy = *mv->tileh;
		} else {
			*mv->ssy = 0;
		}
	} else if (yrel < 0 && (*mv->ssy += yrel) <= -(*mv->tileh)) {
		if (mv->mh < mv->map->maph) {
			if (++mv->my > mv->map->maph - mv->mh - 1) {
				mv->my = mv->map->maph - mv->mh - 1;
				*mv->ssy = -(*mv->tileh);
			} else {
				*mv->ssy = 0;
			}
		}
	}
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

	if (mv->mouse.scrolling) {
		mapview_mouse_scroll(mv, xrel, yrel);
	} else if (mapedit.curtool == mapedit.tools[MAPEDIT_SELECT] &&
	    state & SDL_BUTTON(1) &&
	    mv->cx != -1 && mv->cy != -1) {
	    	dprintf("x %d mx %d\n", x, mv->mouse.x);
		mv->cw += x - mv->mouse.x;
		mv->ch += y - mv->mouse.y;
		if (mv->cw < 1)
			mv->cw = 1;
		if (mv->ch < 1)
			mv->ch = 1;
				
		dprintf("cw -> %d", mv->cw);
		dprintf("ch -> %d", mv->ch);
	} else if (mv->flags & MAPVIEW_EDIT) {
		if (state & SDL_BUTTON(2)) {
			shift_mouse(mapedit.tools[MAPEDIT_SHIFT], mv,
			    xrel, yrel);
		} else if (mapedit.curtool != NULL && (state & SDL_BUTTON(1))) {
			struct tool *tool = mapedit.curtool;

			if (TOOL_OPS(tool)->effect != NULL &&
			    mv->cx != -1 && mv->cy != -1 &&
			    (x != mv->mouse.x || y != mv->mouse.y)) {
				TOOL_OPS(tool)->effect(tool, mv,
				    &mv->map->map[mv->cy][mv->cx]);
			} else if (TOOL_OPS(tool)->mouse != NULL) {
				TOOL_OPS(tool)->mouse(tool, mv, xrel, yrel,
				    state);
			}
		}
	} else if (mv->flags & MAPVIEW_TILEMAP) {	/* Not editing */
		if ((state & SDL_BUTTON(1)) &&
		    (mv->cx != -1) && (mv->cy != -1)) {
			struct node *srcnode = &mv->map->map[mv->cy][mv->cx];

			if (!TAILQ_EMPTY(&srcnode->nrefs)) {
				mapedit.src_node = srcnode;
			}
		}
	}
	pthread_mutex_unlock(&mv->map->lock);

	mv->mouse.x = x;
	mv->mouse.y = y;
}

static void
mapview_mousebuttondown(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;
	struct node *srcnode;
	
	WIDGET_FOCUS(mv);

	pthread_mutex_lock(&mv->map->lock);

	mapview_map_coords(mv, &x, &y);

	if (mv->cx < 0 || mv->cy < 0) {
		goto out;
	}
	srcnode = &mv->map->map[mv->cy][mv->cx];

	switch (button) {
	case 1:						/* Select/edit */
		mv->cur_node = srcnode;
		mv->cw = 1;
		mv->ch = 1;
		break;
	case 2:						/* Select/center */
		mv->flags |= MAPVIEW_NO_CURSOR;
		mv->cur_node = srcnode;
		goto out;
	case 3:						/* Scroll */
		mv->mouse.scrolling++;
		goto out;
	}

	if (mv->flags & MAPVIEW_EDIT && mapedit.curtool != NULL &&
	    TOOL_OPS(mapedit.curtool)->effect != NULL) {
		TOOL_OPS(mapedit.curtool)->effect(mapedit.curtool,
		    mv, &mv->map->map[mv->cy][mv->cx]);
	}

	if (mv->flags & MAPVIEW_TILEMAP) {
		if ((SDL_GetModState() & KMOD_CTRL)) {
			mv->constr.x = mv->cx;
			mv->constr.y = mv->cy;
		}

		if (!TAILQ_EMPTY(&srcnode->nrefs)) {
			mapedit.src_node = srcnode;
		}
	} else {
		mv->cur_node = &mv->map->map[mv->cy][mv->cx];
	}
out:
	pthread_mutex_unlock(&mv->map->lock);
}

static void
mapview_mousebuttonup(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;
	int button = argv[1].i;

	switch (button) {
	case 2:
		mv->flags &= ~(MAPVIEW_NO_CURSOR);
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

	/* Zoom keys */
	if (mv->flags & MAPVIEW_ZOOM) {
		Uint32 ival;
		int incr;
		
		ival = mapedition ? prop_get_int(&mapedit, "zoom-speed") : 60;
		incr = mapedition ? prop_get_int(&mapedit, "zoom-increment") :
		    8;

		switch (keysym) {
		case SDLK_EQUALS:
			mapview_zoom(mv, *mv->zoom + incr);
			if (mv->zoom_tm == NULL) {
				mv->zoom_tm = SDL_AddTimer(ival,
				    mapview_zoom_tick, mv);
			}
			mv->flags |= MAPVIEW_ZOOMING_IN;
			break;
		case SDLK_MINUS:
			mapview_zoom(mv, *mv->zoom - incr);
			if (mv->zoom_tm == NULL) {
				mv->zoom_tm = SDL_AddTimer(ival,
				    mapview_zoom_tick, mv);
			}
			mv->flags |= MAPVIEW_ZOOMING_OUT;
			break;
		case SDLK_0:
		case SDLK_1:
			mv->flags &= ~(MAPVIEW_ZOOMING_IN|MAPVIEW_ZOOMING_OUT);
			mapview_zoom(mv, 100);
			break;
		}
	}
	
	/* Edition keys */
	if (mv->flags & MAPVIEW_EDIT && mv->cur_node != NULL) {
		switch (keysym) {
		case SDLK_INSERT:
			if (mapedit.src_node != NULL) {
				stamp_effect(mapedit.tools[MAPEDIT_STAMP],
				    mv, mv->cur_node);
			}
			break;
		case SDLK_DELETE:
			eraser_effect(mapedit.tools[MAPEDIT_ERASER],
			    mv, mv->cur_node);
			break;
		}
	}

	/* Save/load keys */
	if (mv->flags & MAPVIEW_SAVEABLE) {
		switch (keysym) {
		case SDLK_s:
			object_save(mv->map);
			break;
		case SDLK_l:
			if (object_load(mv->map) == -1) {
				text_msg("Error loading map", "%s: %s",
				    OBJECT(mv->map)->name, error_get());
			}
			break;
		}
	}

	/* Grid, props keys */
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

static void
mapview_scaled(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;

	WIDGET(mv)->w = argv[1].i;
	WIDGET(mv)->h = argv[2].i;

	pthread_mutex_lock(&mv->map->lock);

	mv->mw = WIDGET(mv)->w/(*mv->tilew) + 1;
	mv->mh = WIDGET(mv)->h/(*mv->tileh) + 1;

	if (mv->flags & MAPVIEW_CENTER) {
		mapview_center(mv, mv->map->defx, mv->map->defy);
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

