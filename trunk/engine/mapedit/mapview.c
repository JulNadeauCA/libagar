/*	$Csoft: mapview.c,v 1.144 2004/03/18 21:27:47 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004 CubeSoft Communications, Inc.
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
#include <engine/map.h>
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/button.h>
#include <engine/widget/label.h>
#include <engine/widget/tlist.h>
#include <engine/widget/combo.h>
#include <engine/widget/textbox.h>

#include <engine/mapedit/mediasel.h>

#include "mapedit.h"
#include "mapview.h"

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
	MAPVIEW_ZOOM_INC =	8,	/* Zoom increment (%) */
	MAPVIEW_ZOOM_IVAL =	80,	/* Zoom interval (ms) */
	MAPVIEW_MIN_PROP_ZOOM =	60	/* Minimum zoom to draw props (%) */
};

enum {
	GRID_COLOR,
	CURSOR_COLOR,
	BG1_COLOR,
	BG2_COLOR,
	MSEL_COLOR,
	ESEL_COLOR
};

int	mapview_bg = 1;				/* Background tiles enable */
int	mapview_bg_moving = 1;			/* Background tiles scroll */
int	mapview_bg_squaresize = 16;		/* Background tile size */

static void mapview_lostfocus(int, union evarg *);
static void mapview_mousemotion(int, union evarg *);
static void mapview_mousebuttondown(int, union evarg *);
static void mapview_mousebuttonup(int, union evarg *);
static void mapview_keyup(int, union evarg *);
static void mapview_keydown(int, union evarg *);
static void mapview_begin_selection(struct mapview *);
static void mapview_effect_selection(struct mapview *);

extern struct tool select_tool;
extern void shift_mouse(void *, struct mapview *, Sint16, Sint16);

static __inline__ int
selbounded(struct mapview *mv, int x, int y)
{
	return (!prop_get_bool(&mapedit, "sel-bounded-edition") ||
	    !mv->esel.set ||
	    (x >= mv->esel.x &&
	     y >= mv->esel.y &&
	     x <  mv->esel.x + mv->esel.w &&
	     y <  mv->esel.y + mv->esel.h));
}

static __inline__ int
selecting(struct mapview *mv)
{
	return (mv->curtool == &select_tool ||
	    (SDL_GetModState() & KMOD_CTRL));
}

struct mapview *
mapview_new(void *parent, struct map *m, int flags, struct toolbar *toolbar)
{
	struct mapview *mv;

	mv = Malloc(sizeof(struct mapview), M_OBJECT);
	mapview_init(mv, m, flags, toolbar);
	object_attach(parent, mv);
	return (mv);
}

#ifdef EDITION

static void
mapview_sel_tool(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	struct tool *ntool = argv[2].p;

	if (mv->curtool == ntool) {
		if (ntool->win != NULL) {
			window_hide(ntool->win);
		}
		mv->curtool = NULL;
		return;
	}
	if (mv->curtool != NULL) {
		widget_set_bool(mv->curtool->trigger, "state", 0);
		if (mv->curtool->win != NULL)
			window_hide(mv->curtool->win);
	}
	mv->curtool = ntool;

	if (ntool->win != NULL)
		window_show(ntool->win);
}

void
mapview_reg_tool(struct mapview *mv, const struct tool *tool)
{
	struct tool *ntool;

	ntool = Malloc(sizeof(struct tool), M_MAPEDIT);
	memcpy(ntool, tool, sizeof(struct tool));
	tool_init(ntool, mv);

	if (mv->toolbar != NULL) {
		SDL_Surface *icon = ntool->icon >= 0 ?
		    SPRITE(&mapedit, ntool->icon) : NULL;

		ntool->trigger = toolbar_add_button(mv->toolbar,
		    mv->toolbar->nrows-1, icon, 1, 0, mapview_sel_tool,
		    "%p, %p", mv, ntool);
	}
	TAILQ_INSERT_HEAD(&mv->tools, ntool, tools);
}

void
mapview_toggle_rw(int argc, union evarg *argv)
{
	struct button *bu = argv[0].p;
	struct mapview *mv = argv[1].p;

	if (mv->flags & MAPVIEW_EDIT) {
		mv->flags &= ~(MAPVIEW_EDIT);
	} else {
		if (OBJECT(mv->map)->flags & OBJECT_READONLY) {
			text_msg(MSG_ERROR, _("The `%s' map is read-only."),
			    OBJECT(mv->map)->name);
			widget_set_bool(bu, "state", 0);
		} else {
			mv->flags |= MAPVIEW_EDIT;
		}
	}
}

void
mapview_toggle_nodeedit(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;

	window_toggle_visibility(mv->nodeed.win);
}

void
mapview_toggle_layedit(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	
	window_toggle_visibility(mv->layed.win);
}

void
mapview_toggle_mediasel(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;

	window_toggle_visibility(mv->mediasel.win);
}

void
mapview_selected_layer(int argc, union evarg *argv)
{
	struct combo *com = argv[0].p;
	struct mapview *mv = argv[1].p;
	struct tlist_item *it = argv[2].p;
	struct tlist *tl = com->list;
	int i = 0;

	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->selected) {
			struct map_layer *lay = it->p1;

			mv->map->cur_layer = i;
			textbox_printf(com->tbox, "%d. %s", i, lay->name);
			return;
		}
		i++;
	}
	text_msg(MSG_ERROR, _("No layer is selected."));
}
#endif /* EDITION */

void
mapview_toggle_grid(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;

	if (mv->flags & MAPVIEW_GRID) {
		mv->flags &= ~(MAPVIEW_GRID);
	} else {
		mv->flags |= MAPVIEW_GRID;
	}
}

void
mapview_toggle_props(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;

	if (mv->flags & MAPVIEW_PROPS) {
		mv->flags &= ~(MAPVIEW_PROPS);
	} else {
		mv->flags |= MAPVIEW_PROPS;
	}
}

void
mapview_destroy(void *p)
{
	struct mapview *mv = p;
	struct tool *tool, *ntool;

	for (tool = TAILQ_FIRST(&mv->tools);
	     tool != TAILQ_END(&mv->tools);
	     tool = ntool) {
		ntool = TAILQ_NEXT(tool, tools);
		Free(tool, M_MAPEDIT);
	}

#ifdef EDITION
	if (mapedition) {
		nodeedit_destroy(mv);
		layedit_destroy(mv);
		mediasel_destroy(mv);
	}
#endif
	widget_destroy(mv);
}

static void
mapview_attached(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;
	struct window *win = argv[argc].p;

#ifdef EDITION
	if (mapedition) {
		nodeedit_init(mv, win);
		layedit_init(mv, win);
		mediasel_init(mv, win);
	}
#endif
}

void
mapview_init(struct mapview *mv, struct map *m, int flags,
    struct toolbar *toolbar)
{
	widget_init(mv, "mapview", &mapview_ops,
	    WIDGET_FOCUSABLE|WIDGET_CLIPPING|WIDGET_WFILL|WIDGET_HFILL);
	object_wire_gfx(mv, "/engine/mapedit/mapview/mapview");

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
	mv->nodeed.win = NULL;
	mv->layed.win = NULL;
	mv->mediasel.win = NULL;
	mv->zoom_inc = MAPVIEW_ZOOM_INC;
	mv->zoom_ival = MAPVIEW_ZOOM_IVAL;
	mv->zoom_tm = NULL;
	mv->map = m;
	mv->toolbar = toolbar;
	mv->curtool = NULL;
	TAILQ_INIT(&mv->tools);

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
		mv->izoom.scale = TILESZ;
		mv->izoom.ssx = TILESZ;
		mv->izoom.ssy = TILESZ;
		mv->zoom = &mv->izoom.zoom;
		mv->scale = &mv->izoom.scale;
		mv->ssx = &mv->izoom.ssx;
		mv->ssy = &mv->izoom.ssy;
	} else {
		mv->zoom = &m->zoom;
		mv->scale = &m->scale;
		mv->ssx = &m->ssx;
		mv->ssy = &m->ssy;
	}
	pthread_mutex_unlock(&m->lock);

	widget_map_color(mv, GRID_COLOR, "grid", 100, 100, 100, 255);
	widget_map_color(mv, CURSOR_COLOR, "cursor", 100, 100, 100, 255);
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
	event_new(mv, "attached", mapview_attached, NULL);

	if (mv->toolbar != NULL) {
		toolbar_add_button(toolbar, 0, SPRITE(&mapedit, 6), 1,
		    (flags & MAPVIEW_GRID),
		    mapview_toggle_grid, "%p", mv);
		toolbar_add_button(toolbar, 0, SPRITE(&mapedit, 7), 1,
		    (flags & MAPVIEW_PROPS),
		    mapview_toggle_props, "%p", mv);
		toolbar_add_button(toolbar, 0, SPRITE(&mapedit, 9), 1,
		    (flags & MAPVIEW_EDIT),
		    mapview_toggle_rw, "%p", mv);
#ifdef EDITION
		mv->nodeed.trigger = toolbar_add_button(toolbar, 0,
		    SPRITE(&mapedit, 14), 1, 0,
		    mapview_toggle_nodeedit, "%p", mv);
		mv->layed.trigger = toolbar_add_button(toolbar, 0,
		    SPRITE(&mapedit, 15), 1, 0,
		    mapview_toggle_layedit, "%p", mv);
		mv->mediasel.trigger = toolbar_add_button(toolbar, 0,
		    SPRITE(&mapedit, 21), 1, 0,
		    mapview_toggle_mediasel, "%p", mv);
#endif
	} else {
		mv->nodeed.trigger = NULL;
		mv->layed.trigger = NULL;
		mv->mediasel.trigger = NULL;
	}
}

/*
 * Translate widget coordinates to map coordinates.
 * The map must be locked.
 */
void
mapview_map_coords(struct mapview *mv, int *x, int *y)
{
	*x -= *mv->ssx - *mv->scale;
	*y -= *mv->ssy - *mv->scale;
	*x /= *mv->scale;
	*y /= *mv->scale;
	
	mv->cx = mv->mx + *x;
	mv->cy = mv->my + *y;

	if (mv->cx < 0 || mv->cx >= mv->map->mapw)
		mv->cx = -1;
	if (mv->cy < 0 || mv->cy >= mv->map->maph)
		mv->cy = -1;
}

/* Draw the node property icons. */
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
		{ NODEREF_REGEN,	MAPVIEW_REGEN }
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

/* Draw the cursor for the selected map edition tool. */
static void
draw_curtool_cursor(struct mapview *mv)
{
	SDL_Rect rd;
	int msx, msy;

	rd.w = mv->map->scale;
	rd.h = mv->map->scale;

	if (SDL_GetModState() & KMOD_CTRL) {		/* XXX inefficient */
		SDL_GetMouseState(&msx, &msy);
		rd.x = msx;
		rd.y = msy;
		SDL_BlitSurface(
		    SPRITE(&mapedit, MAPEDIT_SELECT_CURSOR),
		    NULL, view->v, &rd);
		return;
	}
	
	rd.x = mv->mouse.x*mv->map->scale - mv->map->scale + *mv->ssx;
	rd.y = mv->mouse.y*mv->map->scale - mv->map->scale + *mv->ssy;

	if (mv->curtool == NULL)
		goto defcurs;

	if (mv->curtool->cursor_su != NULL) {
		rd.x += WIDGET(mv)->cx;
		rd.y += WIDGET(mv)->cy;
		SDL_BlitSurface(mv->curtool->cursor_su, NULL, view->v, &rd);
	} else {
		if (mv->curtool->cursor == NULL ||
		    mv->curtool->cursor(mv->curtool, mv, &rd) == -1)
			goto defcurs;
	}
	return;
defcurs:
	primitives.rect_outlined(mv,
	    rd.x + 1,
	    rd.y + 1,
	    mv->map->scale - 1,
	    mv->map->scale - 1,
	    CURSOR_COLOR);
	primitives.rect_outlined(mv,
	    rd.x + 2,
	    rd.y + 2,
	    mv->map->scale - 3,
	    mv->map->scale - 3,
	    CURSOR_COLOR);
}

/* XXX very, very inelegant */
static void
draw_background(struct mapview *mv)
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
				    x, y,
				    mapview_bg_squaresize,
				    mapview_bg_squaresize,
				    BG1_COLOR);
				alt1 = 0;
			} else {
				primitives.rect_filled(mv,
				    x, y,
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
	int old_scale = m->scale;
	int layer = 0;
	int esel_x = -1, esel_y = -1, esel_w = -1, esel_h = -1;
	int msel_x = -1, msel_y = -1, msel_w = -1, msel_h = -1;

	if (mapview_bg)
		draw_background(mv);

	pthread_mutex_lock(&m->lock);

	/* Deal with 0x0 maps. */
	if (mv->map->map == NULL)
		goto out;

	if (mv->flags & MAPVIEW_INDEPENDENT) {
		m->zoom = *mv->zoom;
		m->scale = *mv->scale;
	}

draw_layer:
	if (!m->layers[layer].visible) {
		goto next_layer;
	}
	for (my = mv->my, ry = *mv->ssy - mv->map->scale;
	     ((my - mv->my) < mv->mh+2) && (my < m->maph);
	     my++, ry += mv->map->scale) {

		for (mx = mv->mx, rx = *mv->ssx - mv->map->scale;
	     	     ((mx - mv->mx) < (mv->mw+2)) && (mx < m->mapw);
		     mx++, rx += mv->map->scale) {
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
				    rx, ry,
				    mv->map->scale + 1,
				    mv->map->scale + 1,
				    GRID_COLOR);
			}
#ifdef EDITION
			if (!mapedition)
				continue;

			/* Indicate the mouse/effective selections. */
			if (mv->msel.set &&
			    mv->msel.x == mx && mv->msel.y == my) {
				msel_x = rx + 1;
				msel_y = ry + 1;
				msel_w = mv->msel.xoffs*mv->map->scale - 2;
				msel_h = mv->msel.yoffs*mv->map->scale - 2;
			}
			
			if (mv->esel.set &&
			    mv->esel.x == mx && mv->esel.y == my) {
				esel_x = rx;
				esel_y = ry;
				esel_w = mv->map->scale*mv->esel.w;
				esel_h = mv->map->scale*mv->esel.h;
			}
			if (mv->map->scale < MAP_MIN_SCALE)
				continue;
#endif /* EDITION */
		}
	}
next_layer:
	if (++layer < m->nlayers)
		goto draw_layer;			/* Draw next layer */

#ifdef EDITION
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
		draw_curtool_cursor(mv);
	}
#endif /* EDITION */

	if (mv->flags & MAPVIEW_INDEPENDENT) {
		m->zoom = old_zoom;			/* Restore zoom */
		m->scale = old_scale;
	}
out:
	pthread_mutex_unlock(&m->lock);
}

int
mapview_zoom(struct mapview *mv, int zoom)
{
	if (zoom < MAPVIEW_ZOOM_MIN || zoom > MAPVIEW_ZOOM_MAX) {
		error_set(_("The zoom is out of range."));
		return (-1);
	}
#if 0
	/* XXX */
	pthread_mutex_lock(&mv->map->lock);
#endif
	*mv->zoom = zoom;
	*mv->scale = zoom*TILESZ/100;
	if (*mv->scale > MAP_MAX_SCALE) {
		*mv->scale = MAP_MAX_SCALE;
	}
	mv->mw = WIDGET(mv)->w/(*mv->scale) + 1;
	mv->mh = WIDGET(mv)->h/(*mv->scale) + 1;
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

	if (xrel > 0 && (*mv->ssx += xrel) >= *mv->scale) {
		if (--mv->mx < 0) {
			mv->mx = 0;
		} else {
			*mv->ssx = 0;
		}
	} else if (xrel < 0 && ((*mv->ssx) += xrel) <= -(*mv->scale)) {
		if (mv->mw < mv->map->mapw) {
			max = mv->map->mapw - mv->mw - 1;
			if (++mv->mx > max) {
				mv->mx = max;
			} else {
				*mv->ssx = 0;
			}
		}
	}
	if (yrel > 0 && ((*mv->ssy) += yrel) >= *mv->scale) {
		if (--mv->my < 0) {
			mv->my = 0;
		} else {
			*mv->ssy = 0;
		}
	} else if (yrel < 0 && (*mv->ssy += yrel) <= -(*mv->scale)) {
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
	if (*mv->ssx > *mv->scale)	*mv->ssx = *mv->scale;
	if (*mv->ssy > *mv->scale)	*mv->ssy = *mv->scale;
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
	int state = argv[5].i;

	pthread_mutex_lock(&mv->map->lock);

	mapview_map_coords(mv, &x, &y);
	mv->cxrel = x - mv->mouse.x;
	mv->cyrel = y - mv->mouse.y;

	/* XXX configurable */
	if (mv->mouse.scrolling) {				/* Scrolling */
		mapview_mouse_scroll(mv, xrel, yrel);
	} else if (mv->msel.set) {				/* Selection */
		mv->msel.xoffs += x - mv->mouse.x;
		mv->msel.yoffs += y - mv->mouse.y;
	} else if (mv->flags & MAPVIEW_EDIT) {			/* Edition */
		if (state & SDL_BUTTON(2)) {			/* Shift */
			shift_mouse(NULL, mv, xrel, yrel);
		} else if (mv->curtool != NULL && (state & SDL_BUTTON(1))) {
			if (TOOL(mv->curtool)->effect != NULL &&
			    mv->cx != -1 && mv->cy != -1 &&
			    (x != mv->mouse.x || y != mv->mouse.y) &&
			    (selbounded(mv, mv->cx, mv->cy))) {
				TOOL(mv->curtool)->effect(mv->curtool, mv,
				    mv->map, &mv->map->map[mv->cy][mv->cx]);
			}
			if (TOOL(mv->curtool)->mouse != NULL)
				TOOL(mv->curtool)->mouse(mv->curtool, mv,
				    xrel, yrel, state);
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

	/* Translate to map coordinates. */
	pthread_mutex_lock(&mv->map->lock);
	mapview_map_coords(mv, &x, &y);
	if (mv->cx < 0 || mv->cy < 0) {
		goto out;
	}
	curnode = &mv->map->map[mv->cy][mv->cx];

	/* XXX configurable */
	switch (button) {
	case 1:						/* Select/edit */
		if (selecting(mv)) {
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
		if (mv->curtool != NULL &&
		    mv->curtool->effect != NULL &&
		    selbounded(mv, mv->cx, mv->cy)) {
			mv->curtool->effect(mv->curtool, mv, mv->map, curnode);
		}
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

	/* XXX configurable */
	switch (argv[1].i) {
	case 1:
		if (selecting(mv) &&
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
	struct tool *tool;

	pthread_mutex_lock(&mv->map->lock);

	/* XXX configurable */
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
		mapview_center(mv, mv->map->origin.x, mv->map->origin.y);
		break;
	}


	TAILQ_FOREACH(tool, &mv->tools, tools) {
		struct tool_kbinding *kbinding;

		SLIST_FOREACH(kbinding, &tool->kbindings, kbindings) {
			if (kbinding->key == keysym &&
			    (kbinding->mod == KMOD_NONE ||
			     keymod & kbinding->mod)) {
				if (kbinding->edit &&
				   (((mv->flags & MAPVIEW_EDIT) == 0) ||
				    ((OBJECT(mv->map)->flags &
				     OBJECT_READONLY)))) {
					continue;
				}
				kbinding->func(mv);
			}
		}
	}

	/* XXX configurable */
	switch (keysym) {
	case SDLK_g:
		if (mv->flags & MAPVIEW_GRID) {
			mv->flags &= ~(MAPVIEW_GRID);
		} else {
			mv->flags |= MAPVIEW_GRID;
		}
		break;
	case SDLK_p:
		/* XXX replace by config settings */
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
		WIDGET(mv)->w = mv->prew*TILESZ;
		WIDGET(mv)->h = mv->preh*TILESZ;
		return;
	}

	pthread_mutex_lock(&mv->map->lock);
	mv->mw = WIDGET(mv)->w/(*mv->scale) + 1;
	mv->mh = WIDGET(mv)->h/(*mv->scale) + 1;
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
	*mv->ssx = *mv->scale;
	*mv->ssy = *mv->scale;
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

/* Set the coordinates and geometry of the selection rectangle. */
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

/* Fetch the coordinates and geometry of the selection rectangle. */
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
