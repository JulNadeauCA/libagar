/*	$Csoft: mapview.c,v 1.51 2003/01/27 02:19:08 vedge Exp $	*/

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
#include "tool/shift.h"

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
mapview_new(struct region *reg, struct map *m, int flags, int rw, int rh)
{
	struct mapview *mv;

	mv = emalloc(sizeof(struct mapview));
	mapview_init(mv, m, flags, rw, rh);
	
	region_attach(reg, mv);

	return (mv);
}

static void
mapview_node_win_close(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	struct mapview *mv = argv[1].p;

	widget_set_int(mv->node.button, "state", 0);
}

static void
mapview_node_poll(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct mapview *mv = argv[1].p;
	struct node *node = mv->cur_node;
	struct tlist_item *it;
	struct noderef *nref;
	size_t nodesz = 0;
	int i = 0;
	char flags[96];
	
	if (node == NULL) {
		label_printf(mv->node.node_flags_lab, "-");
		label_printf(mv->node.node_size_lab, "-");
		return;
	}
	
	flags[0] = '\0';
	if (node->flags & NODE_ORIGIN)		strcat(flags, "origin ");
	if (node->flags & NODE_BLOCK)		strcat(flags, "block ");
	else if (node->flags & NODE_WALK)	strcat(flags, "walk ");
	else if (node->flags & NODE_CLIMB)	strcat(flags, "climb ");
	if (node->flags & NODE_SLIP)		strcat(flags, "slip ");
	if (node->flags & NODE_BIO)		strcat(flags, "bio ");
	else if (node->flags & NODE_REGEN)	strcat(flags, "regen ");
	if (node->flags & NODE_SLOW)		strcat(flags, "slow ");
	else if (node->flags & NODE_HASTE)	strcat(flags, "haste ");
	if (node->flags & NODE_HAS_ANIM)	strcat(flags, "has-anim ");

	label_printf(mv->node.node_flags_lab, "Node flags: %s", flags);

	tlist_clear_items(tl);
	
	pthread_mutex_lock(&mv->map->lock);

	TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
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

		nodesz += sizeof(nref);
	}

	tlist_restore_selections(tl);
	
	label_printf(mv->node.node_size_lab, "Node size: %ld bytes",
	    (long)nodesz);
	label_printf(mv->node.noderef_type_lab, "-");
	label_printf(mv->node.noderef_flags_lab, "-");
	label_printf(mv->node.noderef_center_lab, "-");

	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->selected) {
			struct noderef *nref = it->p1;
			char flags[32];
			char *type = "";

			switch (nref->type) {
			case NODEREF_SPRITE:
				type = "sprite";
				break;
			case NODEREF_ANIM:
				type = "animation";
				break;
			case NODEREF_WARP:
				type = "warp";
				break;
			}
			
			flags[0] = '\0';
			if (nref->flags & NODEREF_SAVEABLE)
				strcat(flags, "saveable ");
			if (nref->flags & NODEREF_BLOCK)
				strcat(flags, "block ");
			
			label_printf(mv->node.noderef_type_lab,
			    "Noderef type: %s", type);
			label_printf(mv->node.noderef_flags_lab,
			    "Noderef flags: %s", flags);
			label_printf(mv->node.noderef_center_lab,
			    "Noderef centering: %d,%d",
			    nref->xcenter, nref->ycenter);
			break;
		}
	}

	pthread_mutex_unlock(&mv->map->lock);
}

static void
mapview_init_node_win(struct mapview *mv)
{
	struct map *m = mv->map;
	struct window *win;
	struct region *reg;

	win = window_generic_new(268, 346, "mapedit-node-%s-%s",
	    OBJECT(mv)->name, OBJECT(m)->name);
	if (win == NULL) {
		fatal("node win exists\n");
	}
	window_set_caption(win, "%s node", OBJECT(m)->name);
	window_set_min_geo(win, 185, 170);
	event_new(win, "window-close", mapview_node_win_close, "%p", mv);
	
	reg = region_new(win, REGION_VALIGN, 0, 0, 100, -1);
	{
		mv->node.node_flags_lab = label_new(reg, 100, -1, " ");
		mv->node.node_size_lab = label_new(reg, 100, -1, " ");
	}
	
	reg = region_new(win, REGION_VALIGN, 0, -1, 100, -1);
	{
		mv->node.noderef_type_lab = label_new(reg, 100, -1, " ");
		mv->node.noderef_flags_lab = label_new(reg, 100, -1, " ");
		mv->node.noderef_center_lab = label_new(reg, 100, -1, " ");
	}

	reg = region_new(win, REGION_VALIGN, 0, -1, 100, 0);
	{
		mv->node.refs_tl = tlist_new(reg, 100, 100,
		    TLIST_POLL|TLIST_MULTI);
		tlist_set_item_height(mv->node.refs_tl, TILEH);
		event_new(mv->node.refs_tl, "tlist-poll",
		    mapview_node_poll, "%p", mv);
	}

	mv->node.win = win;
}

void
mapview_init(struct mapview *mv, struct map *m, int flags, int rw, int rh)
{
	widget_init(&mv->wid, "mapview", &mapview_ops, rw, rh);
	mv->wid.flags |= WIDGET_CLIPPING;

	mv->flags = flags;
	mv->map = m;
	mv->mx = m->defx;
	mv->my = m->defy;
	mv->mw = 0;		/* Set on scale */
	mv->mh = 0;
	mv->prop_style = -1; 
	mv->mouse.scrolling = 0;
	mv->mouse.x = 0;
	mv->mouse.y = 0;
	mv->constr.mode = MAPVIEW_CONSTR_VERT;
	mv->constr.x = 0;
	mv->constr.y = 0;
	mv->constr.nflags = NODEREF_SAVEABLE;
	mv->tmap_win = NULL;
	mv->cur_node = NULL;
	mv->node.button = NULL;
	mv->zoom = m->zoom;
	mv->tilew = m->tilew;
	mv->tileh = m->tileh;
	mv->ssx = m->tilew;
	mv->ssy = m->tileh;

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
	event_new(mv, "window-keyup", mapview_keyup, NULL);
	event_new(mv, "window-keydown", mapview_keydown, NULL);
	event_new(mv, "window-mousemotion", mapview_mousemotion, NULL);
	event_new(mv, "window-mousebuttonup", mapview_mousebuttonup, NULL);
	event_new(mv, "window-mousebuttondown", mapview_mousebuttondown, NULL);

	mapview_init_node_win(mv);
}

static __inline__ void
draw_node_props(struct mapview *mv, struct node *node, int rx, int ry)
{
	if (mv->prop_style > 0) {
		widget_blit(mv, SPRITE(mv, mv->prop_style), rx, ry);
	}

	if (node->flags & NODE_BLOCK) {
		widget_blit(mv, SPRITE(mv, MAPVIEW_BLOCK), rx, ry);
		rx += SPRITE(mv, MAPVIEW_BLOCK)->w;
	} else if (node->flags & NODE_WALK) {
		widget_blit(mv, SPRITE(mv, MAPVIEW_WALK), rx, ry);
		rx += SPRITE(mv, MAPVIEW_BLOCK)->w;
	} else if (node->flags & NODE_CLIMB) {
		widget_blit(mv, SPRITE(mv, MAPVIEW_CLIMB), rx, ry);
		rx += SPRITE(mv, MAPVIEW_BLOCK)->w;
	}
	
	if (node->flags & NODE_BIO) {
		widget_blit(mv, SPRITE(mv, MAPVIEW_BIO), rx, ry);
		rx += SPRITE(mv, MAPVIEW_BLOCK)->w;
	} else if (node->flags & NODE_REGEN) {
		widget_blit(mv, SPRITE(mv, MAPVIEW_REGEN), rx, ry);
		rx += SPRITE(mv, MAPVIEW_BLOCK)->w;
	}
	
	if (node->flags & NODE_SLOW) {
		widget_blit(mv, SPRITE(mv, MAPVIEW_SLOW), rx, ry);
		rx += SPRITE(mv, MAPVIEW_BLOCK)->w;
	} else if (node->flags & NODE_HASTE) {
		widget_blit(mv, SPRITE(mv, MAPVIEW_HASTE), rx, ry);
		rx += SPRITE(mv, MAPVIEW_BLOCK)->w;
	}

	if (node->flags & NODE_ORIGIN) {
		widget_blit(mv, SPRITE(mv, MAPVIEW_ORIGIN), rx, ry);
		rx += SPRITE(mv, MAPVIEW_BLOCK)->w;
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
	SDL_Rect rd;
	Uint32 col;
	int alt1 = 0, alt2 = 0;
	Uint16 old_zoom;
	int old_tilew, old_tileh;

	/* Draw a moving gimpish background. */
	if (mapedition && prop_get_bool(&mapedit, "tilemap-bg-moving")) {
		static int softbg = 0;
		int ss;

		ss = prop_get_int(&mapedit, "tilemap-bg-square-size");
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

	pthread_mutex_lock(&m->lock);

	old_zoom = m->zoom;					/* Save zoom */
	old_tilew = m->tilew;
	old_tileh = m->tileh;

	m->zoom = mv->zoom;
	m->tilew = mv->tilew;
	m->tileh = mv->tileh;

	for (my = mv->my, ry = mv->ssy - mv->map->tileh;
	     (my - mv->my) <= mv->mh && my < m->maph;
	     my++, ry += mv->map->tileh) {

		for (mx = mv->mx, rx = mv->ssx - mv->map->tilew;
	     	     (mx - mv->mx) <= mv->mw && mx < m->mapw;
		     mx++, rx += mv->map->tilew) {
			node = &m->map[my][mx];
#ifdef DEBUG
			if (map_nodesigs &&
			    (strcmp(NODE_MAGIC, node->magic) != 0 ||
			    node->x != mx || node->y != my)) {
				fatal("bad node\n");
			}
#endif

			TAILQ_FOREACH(nref, &node->nrefs, nrefs) {
#ifdef DEBUG
				if (map_nodesigs &&
				    (strcmp(NODEREF_MAGIC, node->magic) != 0 ||
				    node->x != mx || node->y != my)) {
					fatal("bad noderef\n");
				}
#endif
				noderef_draw(m, nref,
				    WIDGET_ABSX(mv)+rx, WIDGET_ABSY(mv)+ry);
			}

			/* Indicate the node properties. */
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

			if (!mapedition)
				continue;

			/* Draw the cursor for the current tool. */
			if (mx - mv->mx == mv->mouse.x &&
			    my - mv->my == mv->mouse.y &&
			    mv->mouse.x < mv->mw &&
			    mv->mouse.y < mv->mh) {
				struct tool *curtool = NULL;

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
			/* Indicate the source node selection. */
			if (node == mapedit.src_node &&
			    mv->map->tilew > 6 && mv->map->tileh > 6) {
				primitives.rect_outlined(mv,
				    rx+1, ry+1,
				    mv->map->tilew-1, mv->map->tileh-1,
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
#if 0
	/* Highlight if the widget is focused. */
	if (WIDGET_FOCUSED(mv) && WINDOW_FOCUSED(WIDGET(mv)->win)) {
		primitives.rect_outlined(mv,
		    0, 0,
		    WIDGET(mv)->w, WIDGET(mv)->h,
		    WIDGET_COLOR(mv, BORDER_COLOR));
	}
#endif
	m->zoom = old_zoom;				/* Restore zoom */
	m->tilew = old_tilew;
	m->tileh = old_tileh;
	pthread_mutex_unlock(&m->lock);
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
	if (mapedition &&
	    (zoom < prop_get_int(&mapedit, "zoom-minimum") ||
	     zoom > prop_get_int(&mapedit, "zoom-maximum"))) {
		return;
	}

	mv->zoom = zoom;
	mv->tilew = zoom * TILEW / 100;
	mv->tileh = zoom * TILEH / 100;

	if (mv->tilew > 32767)			/* For soft scrolling */
		mv->tilew = 32767;
	if (mv->tileh > 32767)
		mv->tileh = 32767;
	
	mv->mw = WIDGET(mv)->w/mv->tilew + 1;
	mv->mh = WIDGET(mv)->h/mv->tileh + 1;

	mapview_caption(mv);
}

static Uint32
mapview_zoomin(Uint32 ival, void *p)
{
	struct mapview *mv = p;

	mapview_zoom(mv, mv->zoom +
	    mapedition ? prop_get_int(&mapedit, "zoom-increment") : 16);
	return (ival);
}

static Uint32
mapview_zoomout(Uint32 ival, void *p)
{
	struct mapview *mv = p;

	mapview_zoom(mv, mv->zoom -
	    mapedition ? prop_get_int(&mapedit, "zoom-increment") : 16);
	return (ival);
}

/* Translate widget coordinates to map coordinates. */
static __inline__ void
mapview_map_coords(struct mapview *mv, int *x, int *y)
{
	*x -= mv->ssx - mv->tilew;
	*y -= mv->ssy - mv->tileh;

	*x /= mv->tilew;
	*y /= mv->tileh;
}

static void
mapview_mousemotion(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;
	int x = argv[1].i;
	int y = argv[2].i;
	int xrel = argv[3].i;
	int yrel = argv[4].i;
	Uint8 mouse;

	mapview_map_coords(mv, &x, &y);

	mouse = SDL_GetMouseState(NULL, NULL);

	if (mv->mouse.scrolling) {
		/* Soft-scrolling/centering */
		if (xrel > 0 && (mv->ssx += xrel) >= mv->tilew) {
			if (--mv->mx < 0) {
				mv->mx = 0;
				if (mv->mw < mv->map->mapw)
					mv->ssx = mv->tilew;
			} else {
				mv->ssx = 0;
			}
		} else if (xrel < 0 && (mv->ssx += xrel) <= -mv->tilew) {
			if (mv->mw < mv->map->mapw) {
				if (++mv->mx > mv->map->mapw - mv->mw - 1) {
					mv->mx = mv->map->mapw - mv->mw - 1;
					mv->ssx = -mv->tilew;
				} else {
					mv->ssx = 0;
				}
			}
		}
		if (yrel > 0 && (mv->ssy += yrel) >= mv->tileh) {
			if (--mv->my < 0) {
				mv->my = 0;
				if (mv->mh < mv->map->maph)
					mv->ssy = mv->tileh;
			} else {
				mv->ssy = 0;
			}
		} else if (yrel < 0 && (mv->ssy += yrel) <= -mv->tileh) {
			if (mv->mh < mv->map->maph) {
				if (++mv->my > mv->map->maph - mv->mh - 1) {
					mv->my = mv->map->maph - mv->mh - 1;
					mv->ssy = -mv->tileh;
				} else {
					mv->ssy = 0;
				}
			}
		}
	} else if (mv->flags & MAPVIEW_EDIT) {
		if ((SDL_GetModState() & KMOD_SHIFT) ||
		    (mouse & SDL_BUTTON(2))) {
			shift_mouse(mapedit.tools.shift, mv, xrel, yrel);
		} else if (mapedit.curtool != NULL &&
		    TOOL_OPS(mapedit.curtool)->tool_effect != NULL) {
			if ((x != mv->mouse.x || y != mv->mouse.y) &&
			    (mouse & SDL_BUTTON(1)) &&
			    (x >= 0 && y >= 0 &&
			     x < mv->mw && y < mv->mh) &&
			    (mv->mx+x < mv->map->mapw &&
			     mv->my+y < mv->map->maph)) {
				TOOL_OPS(mapedit.curtool)->tool_effect(
				    mapedit.curtool, mv, mv->mx+x, mv->my+y);
			}
		}
	}
	if (mv->flags & MAPVIEW_TILEMAP) {
		if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK) &&
		    (x >= 0 && y >= 0 && x < mv->mw && y < mv->mh) &&
		    (mv->mx+x < mv->map->mapw) &&
		    (mv->my+y < mv->map->maph)) {
			struct node *srcnode= &mv->map->map[mv->my+y][mv->mx+x];
			
			if (!TAILQ_EMPTY(&srcnode->nrefs)) {
				mapedit.src_node = srcnode;
			}
		}
	}

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
	struct node *srcnode = NULL;

	mapview_map_coords(mv, &x, &y);

	WIDGET_FOCUS(mv);

	if (x >= 0 && mv->mx+x <= mv->map->mapw &&
	    y >= 0 && mv->my+y <= mv->map->maph) {
		srcnode = &mv->map->map[mv->my+y][mv->mx+x];
	} else {
		return;
	}

	switch (button) {
	case 2:
		mv->cur_node = srcnode;
		return;
	case 3:
		mv->mouse.scrolling++;
		return;
	}

	if ((x >= 0 && y >= 0 && x < mv->mw && y < mv->mh) &&
	    (mv->mx+x < mv->map->mapw) && (mv->my+y < mv->map->maph)) {
		if (mv->flags & MAPVIEW_EDIT &&
		    mapedit.curtool != NULL &&
		    TOOL_OPS(mapedit.curtool)->tool_effect != NULL) {
			TOOL_OPS(mapedit.curtool)->tool_effect(mapedit.curtool,
			    mv, mv->mx+x, mv->my+y);
		}

		if (mv->flags & MAPVIEW_TILEMAP) {
			if ((SDL_GetModState() & KMOD_CTRL)) {
				mv->constr.x = mv->mx+x;
				mv->constr.y = mv->my+y;
			}

			if (!TAILQ_EMPTY(&srcnode->nrefs)) {
				mapedit.src_node = srcnode;
			}
		} else {
			mv->cur_node = &mv->map->map[mv->my+y][mv->mx+x];
		}
	}
}

static void
mapview_mousebuttonup(int argc, union evarg *argv)
{
	struct mapview *mv = argv[0].p;
	int button = argv[1].i;

	switch (button) {
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
			    prop_get_int(&mapedit, "zoom-speed"),
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
		case SDLK_1:
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

	mapview_zoom(mv, mv->zoom);

	if (mv->flags & MAPVIEW_CENTER) {
		mapview_center(mv, mv->map->defx, mv->map->defy);
		mv->flags &= ~(MAPVIEW_CENTER);
	}
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

	mv->mx = x - mv->mw/2;
	mv->my = y - mv->mh/2;

	if (mv->mx < 0)
		mv->mx = 0;
	if (mv->my < 0)
		mv->my = 0;
	if (mv->mx >= m->mapw - mv->mw)
		mv->mx = m->mapw - mv->mw;
	if (mv->my >= m->maph - mv->mh)
		mv->my = m->maph - mv->mh;
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

