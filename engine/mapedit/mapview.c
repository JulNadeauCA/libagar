/*	$Csoft: mapview.c,v 1.45 2003/01/19 12:09:40 vedge Exp $	*/

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
#include <engine/widget/label.h>
#include <engine/widget/tlist.h>

#include "mapedit.h"
#include "mapview.h"

#include "tool/tool.h"

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

static SDL_TimerID zoomin_timer, zoomout_timer;

static void	mapview_scaled(int, union evarg *);
static void	mapview_lostfocus(int, union evarg *);
static void	mapview_scroll(struct mapview *, int);
static void	mapview_stop_zoom(struct mapview *);
static void	mapview_mousemotion(int, union evarg *);
static void	mapview_mousebuttondown(int, union evarg *);
static void	mapview_mousebuttonup(int, union evarg *);
static void	mapview_keyup(int, union evarg *);
static void	mapview_keydown(int, union evarg *);

static __inline__ void	draw_node_props(struct mapview *, struct node *,
			    int, int);

struct mapview *
mapview_new(struct region *reg, struct mapedit *med, struct map *m,
    int flags, int rw, int rh)
{
	struct mapview *mv;

	mv = emalloc(sizeof(struct mapview));
	mapview_init(mv, med, m, flags, rw, rh);
	
	region_attach(reg, mv);

	return (mv);
}

static void
mapview_node_win_close(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	struct mapview *mv = argv[1].p;

	window_hide(win);
	widget_set_int(mv->node_button, "value", 0);
}

static void
mapview_node_poll(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct label *label = argv[1].p;
	struct map *m = argv[2].p;
	struct mapview *mv = argv[3].p;
	struct node *src_node = mv->cur_node;
	struct noderef *nref;
	int i = 0;

	if (src_node == NULL) {
		label_printf(label, "No source node");
		return;
	} else {
		label_printf(label, "Flags: 0x%x", src_node->flags);
	}

	pthread_mutex_lock(&m->lock);
	tlist_clear_items(tl);
	TAILQ_FOREACH(nref, &src_node->nrefs, nrefs) {
		SDL_Surface *icon = NULL;
		struct art_anim *anim;
		char *text;

		switch (nref->type) {
		case NODEREF_SPRITE:
			Asprintf(&text, "%d. s(%s:%d)", i, nref->pobj->name,
			    nref->offs);
			icon = nref->pobj->art->sprites[nref->offs];
			break;
		case NODEREF_ANIM:
			Asprintf(&text, "%d. a(%s:%d)", i, nref->pobj->name,
			    nref->offs);
			anim = nref->pobj->art->anims[nref->offs];
			if (anim->nframes > 0) {
				icon = anim->frames[0];
			}
			break;
		case NODEREF_WARP:
			Asprintf(&text, "%d. w(%s:%d,%d)", i,
			    nref->data.warp.map, nref->data.warp.x,
			    nref->data.warp.y);
			break;
		}
		tlist_insert_item(tl, icon, text, nref);
		free(text);
		i++;
	}
	tlist_restore_selections(tl);
	pthread_mutex_unlock(&m->lock);
}

void
mapview_init(struct mapview *mv, struct mapedit *med, struct map *m,
    int flags, int rw, int rh)
{
	struct region *reg;

	widget_init(&mv->wid, "mapview", &mapview_ops, rw, rh);
	mv->wid.flags |= WIDGET_CLIPPING;

	mv->flags = flags;
	mv->med = med;
	mv->map = m;
	mv->mx = m->defx;
	mv->my = m->defy;
	mv->mw = 0;		/* Set on scale */
	mv->mh = 0;
	mv->prop_style = -1; 
	mv->mouse.move = 0;
	mv->mouse.x = 0;
	mv->mouse.y = 0;
	mv->constr.mode = MAPVIEW_CONSTR_VERT;
	mv->constr.x = 0;
	mv->constr.y = 0;
	mv->constr.nflags = NODEREF_SAVEABLE;
	mv->tmap_win = NULL;
	mv->cur_node = NULL;

	if (med == NULL && (mv->flags & (MAPVIEW_TILEMAP | MAPVIEW_EDIT))) {
		fatal("no map editor\n");
	}

	widget_map_color(mv, BORDER_COLOR, "border", 200, 200, 200);
	widget_map_color(mv, GRID_COLOR, "grid", 100, 100, 100);
	widget_map_color(mv, CURSOR_COLOR, "cursor", 100, 100, 100);
	widget_map_color(mv, POSITION_CURSOR_COLOR, "pos-cursor", 0, 100, 150);
	widget_map_color(mv, CONSTR_ORIGIN_COLOR, "constr-orig", 100, 100, 130);
	widget_map_color(mv, SRC_NODE_COLOR, "src-node", 0, 190, 0);
	widget_map_color(mv, BACKGROUND2_COLOR, "background-2", 175, 175, 175);
	widget_map_color(mv, BACKGROUND1_COLOR, "background-1", 114, 114, 114);

	event_new(mv, "widget-scaled", mapview_scaled, NULL);
	event_new(mv, "widget-lostfocus", mapview_lostfocus, NULL);
	event_new(mv, "window-keyup", mapview_keyup, NULL);
	event_new(mv, "window-keydown", mapview_keydown, NULL);
	event_new(mv, "window-mousemotion", mapview_mousemotion, NULL);
	event_new(mv, "window-mousebuttonup", mapview_mousebuttonup, NULL);
	event_new(mv, "window-mousebuttondown", mapview_mousebuttondown, NULL);
	
	/* Create the node edition window. */
	mv->node_win = window_generic_new(268, 346, "mapedit-node-%s",
	    OBJECT(m)->name);
	event_new(mv->node_win, "window-close",
	    mapview_node_win_close, "%p", mv);

	window_set_caption(mv->node_win, "%s node", OBJECT(m)->name);
	reg = region_new(mv->node_win, REGION_VALIGN, 0, 0, 100, 100);
	{
		struct label *lab;
		struct tlist *tl;

		lab = label_new(reg, 100, 50, "...");
	
		tl = tlist_new(reg, 100, 50, TLIST_POLL|TLIST_MULTI);
		tlist_set_item_height(tl, TILEH);
		event_new(tl, "tlist-poll", mapview_node_poll, "%p, %p, %p",
		    lab, m, mv);
		mv->node_tlist = tl;
	}
}

static __inline__ void
draw_node_props(struct mapview *mv, struct node *node, int rx, int ry)
{
	if (mv->prop_style > 0) {
		widget_blit(mv, SPRITE(mv->med, mv->prop_style), rx, ry);
	}

	if (node->flags & NODE_BLOCK) {
		widget_blit(mv, SPRITE(mv->med, MAPEDIT_BLOCK), rx, ry);
		rx += SPRITE(mv->med, MAPEDIT_BLOCK)->w;
	} else if (node->flags & NODE_WALK) {
		widget_blit(mv, SPRITE(mv->med, MAPEDIT_WALK), rx, ry);
		rx += SPRITE(mv->med, MAPEDIT_BLOCK)->w;
	} else if (node->flags & NODE_CLIMB) {
		widget_blit(mv, SPRITE(mv->med, MAPEDIT_CLIMB), rx, ry);
		rx += SPRITE(mv->med, MAPEDIT_BLOCK)->w;
	}
	
	if (node->flags & NODE_BIO) {
		widget_blit(mv, SPRITE(mv->med, MAPEDIT_BIO), rx, ry);
		rx += SPRITE(mv->med, MAPEDIT_BLOCK)->w;
	} else if (node->flags & NODE_REGEN) {
		widget_blit(mv, SPRITE(mv->med, MAPEDIT_REGEN), rx, ry);
		rx += SPRITE(mv->med, MAPEDIT_BLOCK)->w;
	}
	
	if (node->flags & NODE_SLOW) {
		widget_blit(mv, SPRITE(mv->med, MAPEDIT_SLOW), rx, ry);
		rx += SPRITE(mv->med, MAPEDIT_BLOCK)->w;
	} else if (node->flags & NODE_HASTE) {
		widget_blit(mv, SPRITE(mv->med, MAPEDIT_HASTE), rx, ry);
		rx += SPRITE(mv->med, MAPEDIT_BLOCK)->w;
	}

	if (node->flags & NODE_ORIGIN) {
		widget_blit(mv, SPRITE(mv->med, MAPEDIT_ORIGIN), rx, ry);
		rx += SPRITE(mv->med, MAPEDIT_BLOCK)->w;
	}
}

void
mapview_draw(void *p)
{
	struct mapview *mv = p;
	struct map *m = mv->map;
	struct node *node;
	struct noderef *nref;
	struct mapedit *med = mv->med;
	int mx, my, rx, ry, alt = 0;
	SDL_Rect rd;
	Uint32 col;
	int alt1 = 0, alt2 = 0;

	/* Draw a moving gimpish background. */
	if (prop_get_bool(med, "tilemap-bg-moving")) {
		static int softbg = 0;
		int ss;

		ss = prop_get_int(med, "tilemap-bg-square-size");
		if (++softbg > ss-1) {
			softbg = 0;
		}
		rd.w = ss;
		rd.h = ss;
		for (my = -ss + softbg; my < WIDGET(mv)->h; my += ss) {
			rd.y = my;
			for (mx = -ss + softbg; mx < WIDGET(mv)->w; mx += ss) {
				rd.x = mx;
				if (alt1++ == 1) {
					primitives.rect_filled(mv, &rd,
					    WIDGET_COLOR(mv,
					    BACKGROUND1_COLOR));
					alt1 = 0;
				} else {
					primitives.rect_filled(mv, &rd,
					    WIDGET_COLOR(mv,
					    BACKGROUND2_COLOR));
				}
			}
			if (alt2++ == 1) {
				alt2 = 0;
			}
			alt1 = alt2;
		}
	}

	/* XXX soft scroll */

	for (my = mv->my, ry = 0;
	     my-mv->my < mv->mh && my < m->maph;
	     my++, ry += mv->map->tileh) {

		for (mx = mv->mx, rx = 0;
	     	     mx-mv->mx < mv->mw && mx < m->mapw;
		     mx++, rx += mv->map->tilew) {

			node = &m->map[my][mx];
#ifdef DEBUG
			if (node->x != mx || node->y != my) {
				fatal("node at %d,%d should be at %d,%d\n",
				    mx, my, node->x, node->y);
			}
#endif
			node_draw(m, node,
			    WIDGET_ABSX(mv)+rx, WIDGET_ABSY(mv)+ry);

			/* Draw node properties. */
			if (mv->flags & MAPVIEW_PROPS && mv->map->zoom >= 60) {
				draw_node_props(mv, node, rx, ry);
			}

			if (mv->flags & MAPVIEW_GRID) {
				/* XXX overdraw */
				primitives.frame(mv,
				    rx, ry,
				    mv->map->tilew+1, mv->map->tileh+1,
				    WIDGET_COLOR(mv, GRID_COLOR));
			}

			/* Draw the cursor for the current tool. */
			if (mx-mv->mx == mv->mouse.x &&
			    my-mv->my == mv->mouse.y &&
			    mv->mouse.x < mv->mw &&
			    mv->mouse.y < mv->mh) {
				struct tool *curtool = NULL;

				if (med != NULL) {
					curtool = med->curtool;
				}
				if (curtool != NULL &&
				    TOOL_OPS(curtool)->tool_cursor != NULL) {
					TOOL_OPS(curtool)->tool_cursor(curtool,
					    mv, mx, my);
				} else if (mv->map->tilew > 6 &&
				    mv->map->tileh > 6) {
					/* XXX cosmetic */
					primitives.rect_outlined(mv,
					    rx+1, ry+1,
					    mv->map->tilew-1, mv->map->tileh-1,
					    WIDGET_COLOR(mv, CURSOR_COLOR));
					primitives.rect_outlined(mv,
					    rx+2, ry+2,
					    mv->map->tilew-3, mv->map->tileh-3,
					    WIDGET_COLOR(mv, CURSOR_COLOR));
				}
			}

			/* Draw the position cursor. */
			if (mv->cur_node == node) {
				primitives.rect_outlined(mv,
				    rx+3, ry+3,
				    mv->map->tilew-5, mv->map->tileh-5,
				    WIDGET_COLOR(mv, POSITION_CURSOR_COLOR));
			}

			/* Draw the tile map selection cursor. */
			if (node == med->src_node &&
			    mv->map->tilew > 6 && mv->map->tileh > 6) {
				struct noderef *nref;

				primitives.rect_outlined(mv,
				    rx+1, ry+1,
				    mv->map->tilew-1, mv->map->tileh-1,
				    WIDGET_COLOR(mv, SRC_NODE_COLOR));
			}
			/* Draw the tile map construction cursor. */
			if ((mv->flags & MAPVIEW_TILEMAP) &&
			    mv->constr.x == mx && mv->constr.y == my) {
				primitives.frame(mv,
				    rx+2, ry+2,
				    mv->map->tilew-3, mv->map->tileh-3,
				    WIDGET_COLOR(mv, CONSTR_ORIGIN_COLOR));
			}
		}
	}
#if 0
	/* Highlight if the widget is focused. */
	if (WIDGET_FOCUSED(mv) && WINDOW_FOCUSED(WIDGET(mv)->win)) {
		primitives.rect_outlined(mv,
		    0, 0,
		    WIDGET(mv)->w, WIDGET(mv)->h,
		    WIDGET_COLOR(mv, BORDER_COLOR));
	}
#endif
}

static void
mapview_scroll(struct mapview *mv, int dir)
{
	if (mv->flags & MAPVIEW_TILEMAP) {
		switch (dir) {
		case DIR_LEFT:
		case DIR_RIGHT:
			if (!prop_get_bool(mv->med, "tilemap-scroll-x"))
				return;
			break;
		case DIR_UP:
		case DIR_DOWN:
			if (!prop_get_bool(mv->med, "tilemap-scroll-y"))
				return;
			break;
		}
	}

	/* XXX soft scroll */
	switch (dir) {
	case DIR_LEFT:
#if 1
		if (--mv->mx <= 0) {
			mv->mx = 0;
		}
#else
		mv->mx--;
#endif
		break;
	case DIR_RIGHT:
		if (mv->mx + 1 < mv->map->mapw - 1) {
			mv->mx++;
		}
		break;
	case DIR_UP:
#if 1
		if (--mv->my <= 0) {
			mv->my = 0;
		}
#else
		mv->my--;
#endif
		break;
	case DIR_DOWN:
		if (mv->my + 1 < (mv->mh - mv->map->maph)) {
			mv->my++;
		}
		break;
	}
	dprintf("coords = %d,%d\n", mv->mx, mv->my);
}

static void
mapview_caption(struct mapview *mv)
{
	struct window *win = WIDGET(mv)->win;

	if (mv->flags & MAPVIEW_TILEMAP) {
		window_set_caption(win, "%s", OBJECT(mv->map)->name);
	} else {
		if ((mv->flags & MAPVIEW_EDIT) == 0) {
			window_set_caption(win, "@ %s (%d%%)",
			    OBJECT(mv->map)->name, mv->map->zoom);
		} else {
			window_set_caption(win, "%s (%d%%)",
			    OBJECT(mv->map)->name, mv->map->zoom);
		}
	}
}

void
mapview_zoom(struct mapview *mv, int zoom)
{
	if (zoom < prop_get_int(mv->med, "zoom-minimum") ||
	    zoom > prop_get_int(mv->med, "zoom-maximum")) {
		return;
	}

	map_set_zoom(mv->map, zoom);

	mv->mw = WIDGET(mv)->w/mv->map->tilew + 1;
	mv->mh = WIDGET(mv)->h/mv->map->tileh + 1;

	mapview_caption(mv);
}

static Uint32
mapview_zoomin(Uint32 ival, void *p)
{
	struct mapview *mv = p;
	
	mapview_zoom(mv, mv->map->zoom +
	    prop_get_int(mv->med, "zoom-increment"));
	return (ival);
}

static Uint32
mapview_zoomout(Uint32 ival, void *p)
{
	struct mapview *mv = p;

	mapview_zoom(mv, mv->map->zoom -
	    prop_get_int(mv->med, "zoom-increment"));
	return (ival);
}

static void
mapview_mousemotion(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;
	struct mapedit *med = mv->med;
	int x = argv[1].i / mv->map->tilew;
	int y = argv[2].i / mv->map->tileh;

	/* Scroll */
	if (mv->mouse.move) {
		mapview_scroll(mv,
		    (mv->mouse.x < x) ? DIR_LEFT :
		    (mv->mouse.x > x) ? DIR_RIGHT :
		    (mv->mouse.y < y) ? DIR_UP :
		    (mv->mouse.y > y) ? DIR_DOWN : 0);
	} else if (mv->flags & MAPVIEW_EDIT && med->curtool != NULL &&
	    TOOL_OPS(med->curtool)->tool_effect != NULL) {
		if ((x != mv->mouse.x || y != mv->mouse.y) &&
		    (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK) &&
		    (x >= 0 && y >= 0 && x < mv->mw && y < mv->mh) &&
		    (mv->mx+x < mv->map->mapw && mv->my+y < mv->map->maph)) {
			TOOL_OPS(med->curtool)->tool_effect(med->curtool,
			    mv, mv->mx+x, mv->my+y);
		}
	}
	if (mv->flags & MAPVIEW_TILEMAP) {
		if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK &&
		    (x >= 0 && y >= 0 && x < mv->mw && y < mv->mh) &&
		    (mv->mx+x < mv->map->mapw) &&
		    (mv->my+y < mv->map->maph)) {
			med->src_node = &mv->map->map[mv->my+y][mv->mx+x];
		}
	}
	
	mv->mouse.x = x;
	mv->mouse.y = y;
}

static void
mapview_mousebuttondown(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;
	struct mapedit *med = mv->med;
	int button = argv[1].i;
	int x = argv[2].i / mv->map->tilew;
	int y = argv[3].i / mv->map->tileh;
	
	WIDGET_FOCUS(mv);
	if (button > 1) {
		mv->mouse.move++;
	}

	if ((x >= 0 && y >= 0 && x < mv->mw && y < mv->mh) &&
	    (mv->mx+x < mv->map->mapw) && (mv->my+y < mv->map->maph)) {
		if (mv->flags & MAPVIEW_EDIT &&
		    med->curtool != NULL &&
		    TOOL_OPS(med->curtool)->tool_effect != NULL) {
			TOOL_OPS(med->curtool)->tool_effect(med->curtool,
			    mv, mv->mx+x, mv->my+y);
		}
		
		if (button == 1) {
			mv->cur_node = &mv->map->map[mv->my+y][mv->mx+x];
			if (mv->flags & MAPVIEW_TILEMAP) {
				if ((SDL_GetModState() & KMOD_CTRL)) {
					mv->constr.x = mv->mx+x;
					mv->constr.y = mv->my+y;
				}
				med->src_node = mv->cur_node;
			}
		}
	}
}

static void
mapview_mousebuttonup(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;
	int button = argv[1].i;

	if (button > 1) {
		mv->mouse.move = 0;
	}
}

static void
mapview_keyup(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;
	int keysym = argv[1].i;

	if (mv->flags & MAPVIEW_ZOOM) {
		switch (keysym) {
		case SDLK_EQUALS:
		case SDLK_MINUS:
			mapview_stop_zoom(mv);
			break;
		}
	}
}

static void
mapview_keydown(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;
	int keysym = argv[1].i;

	if (mv->flags & MAPVIEW_ZOOM) {
		switch (keysym) {
		case SDLK_EQUALS:
			zoomin_timer = SDL_AddTimer(
			    prop_get_int(mv->med, "zoom-speed"),
			    mapview_zoomin, mv);
			if (zoomin_timer == NULL) {
				warning("SDL_AddTimer: %s\n", SDL_GetError());
			}
			break;
		case SDLK_MINUS:
			zoomout_timer = SDL_AddTimer(60, mapview_zoomout, mv);
			if (zoomout_timer == NULL) {
				warning("SDL_AddTimer: %s\n",
				    SDL_GetError());
			}
			break;
		case SDLK_0:
			mapview_zoom(mv, 100);
			mapview_stop_zoom(mv);
			break;
		}
	}
}

static void
mapview_scaled(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;

	WIDGET(mv)->w = argv[1].i;
	WIDGET(mv)->h = argv[2].i;

	mapview_zoom(mv, mv->map->zoom);
}

static void
mapview_lostfocus(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;

	mapview_stop_zoom(mv);
}

void
mapview_center(struct mapview *mv, int x, int y)
{
	struct map *m = mv->map;
	int nx, ny;

	nx = x - mv->mw/2;
	ny = y - mv->mh/2;
	
	if (nx < 0)
		nx = 0;
	if (ny < 0)
		ny = 0;
	if (nx >= m->mapw-mv->mw)
		nx = m->mapw - mv->mw;
	if (ny >= m->maph-mv->mh)
		ny = m->maph - mv->mh;

	mv->mx = nx;
	mv->my = ny;
}

static void
mapview_stop_zoom(struct mapview *mv)
{
	if (zoomin_timer != NULL) {
		SDL_RemoveTimer(zoomin_timer);
		zoomin_timer = NULL;
	}
	if (zoomout_timer != NULL) {
		SDL_RemoveTimer(zoomout_timer);
		zoomout_timer = NULL;
	}
}

