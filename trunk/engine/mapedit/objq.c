/*	$csoft: objq.c,v 1.35 2003/01/01 05:18:37 vedge Exp $	*/

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

#include <stdio.h>

#include <engine/engine.h>

#include <engine/map.h>
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/button.h>
#include <engine/widget/tlist.h>
#include <engine/widget/label.h>
#include <engine/widget/text.h>

#include "mapedit.h"
#include "mapview.h"
#include "objq.h"
#include "fileops.h"

enum {
	OBJQ_INSERT_LEFT,
	OBJQ_INSERT_RIGHT,
	OBJQ_INSERT_UP,
	OBJQ_INSERT_DOWN
};

static void	 tilemap_option(int, union evarg *);

static void
tl_objs_poll(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct mapedit *med = argv[1].p;
	struct editobj *eob;

	tlist_clear_items(tl);

	TAILQ_FOREACH(eob, &med->eobjsh, eobjs) {
		struct object *ob = eob->pobj;
		struct art *art = ob->art;
		SDL_Surface *icon = NULL;

		if (ob->art != NULL && ob->art->nsprites > 0) {
			icon = ob->art->sprites[0];
		}
		tlist_insert_item(tl, icon, ob->name, ob);
	}

	tlist_restore_selections(tl);
}

static void
tilemap_option(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	int opt = argv[2].i;

	switch (opt) {
	case MAPEDIT_TOOL_GRID:
		if (mv->flags & MAPVIEW_GRID) {
			mv->flags &= ~(MAPVIEW_GRID);
		} else {
			mv->flags |= MAPVIEW_GRID;
		}
		break;
	case MAPEDIT_TOOL_PROPS:
		if (mv->flags & MAPVIEW_PROPS) {
			mv->flags &= ~(MAPVIEW_PROPS);
		} else {
			mv->flags |= MAPVIEW_PROPS;
		}
		break;
	case MAPEDIT_TOOL_EDIT:
		if (mv->flags & MAPVIEW_EDIT) {
			mv->flags &= ~(MAPVIEW_EDIT);
			window_hide(mv->tmap_win);
		} else {
			mv->flags |= MAPVIEW_EDIT;
			window_show(mv->tmap_win);
		}
		break;
	}
}

static void
objq_insert_tiles(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	struct mapview *mv = argv[2].p;
	int mode = argv[3].i;
	struct tlist_item *it;
	struct map *m = mv->map;

	mv->constr.mode = mode;

	TAILQ_FOREACH(it, &tl->items, items) {
		struct object *pobj = it->p1;
		struct node *node;
		struct noderef *nref = NULL;
		enum noderef_type t;
		SDL_Surface *srcsu;
		Uint32 ind;
		struct art_anim *anim;
 
		if (!it->selected)
			continue;
		if (it->text_len < 2)
			continue;

		ind = (Uint32)atoi(it->text + 1);
		switch (it->text[0]) {
		case 's':
			t = NODEREF_SPRITE;
			srcsu = SPRITE(pobj, ind);
			break;
		case 'a':
			t = NODEREF_ANIM;
			anim = ANIM(pobj, ind);
			srcsu = anim->frames[0];
			break;
		default:
			continue;
		}

		map_adjust(m,
		    mv->constr.x + srcsu->w/TILEW + 1,
		    mv->constr.y + srcsu->h/TILEH + 1);
		node = &m->map[mv->constr.y][mv->constr.x];
		switch (t) {
		case NODEREF_SPRITE:
			nref = node_add_sprite(node, pobj, ind);
			break;
		case NODEREF_ANIM:
			nref = node_add_anim(node, pobj, ind,
			    NODEREF_ANIM_AUTO);
			break;
		default:
			break;
		}
		nref->flags |= mv->constr.nflags;

		switch (mode) {
		case OBJQ_INSERT_LEFT:
			mv->constr.x -= srcsu->w/TILEW;
			if (mv->constr.x < 0)
				mv->constr.x = 0;
			break;
		case OBJQ_INSERT_RIGHT:
			mv->constr.x += srcsu->w/TILEW;
			break;
		case OBJQ_INSERT_UP:
			mv->constr.y -= srcsu->h/TILEH;
			if (mv->constr.y < 0)
				mv->constr.y = 0;
			break;
		case OBJQ_INSERT_DOWN:
			mv->constr.y += srcsu->h/TILEH;
			break;
		}
	}
}

static void
tl_objs_selected(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct mapedit *med = argv[1].p;
	struct tlist_item *eob_item = argv[2].p;
	struct object *ob = eob_item->p1;
	struct window *win;
	struct region *reg;
	struct mapview *mv;
	struct button *bu;
	static int cury = 140;
	char *wname;

	win = window_generic_new(202, 365, "mapedit-tmap-%s", ob->name);
	if (win == NULL) {
		return;		/* Exists */
	}
	win->rd.x = view->w - 88;
	win->rd.y = cury;
	win->minw = 56;
	win->minh = 117;
	if ((cury += 32) + 264 > view->h) {
		cury = 140;
	}
	window_set_caption(win, "%s", ob->name);

	mv = emalloc(sizeof(struct mapview));
	mapview_init(mv, med, ob->art->tiles.map,
	    MAPVIEW_CENTER|MAPVIEW_ZOOM|MAPVIEW_TILEMAP|MAPVIEW_GRID|
	    MAPVIEW_PROPS,
	    100, 100);

	if (object_load(ob->art->tiles.map) == -1) {
		dprintf("loading tile map: %s\n", error_get());
	}

	/* Map operations */
	reg = region_new(win, REGION_HALIGN, 0, 0, 100, 10);
	reg->spacing = 1;
	{
		bu = button_new(reg, NULL,		/* Load map */
		    SPRITE(med, MAPEDIT_TOOL_LOAD_MAP), 0, 16, 100);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS|WIDGET_UNFOCUSED_BUTTONUP;
		event_new(bu, "button-pushed", fileops_revert_map, "%p", mv);

		bu = button_new(reg, NULL,		/* Save map */
		    SPRITE(med, MAPEDIT_TOOL_SAVE_MAP), 0, 16, 100);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS|WIDGET_UNFOCUSED_BUTTONUP;
		event_new(bu, "button-pushed", fileops_save_map, "%p", mv);
		
		bu = button_new(reg, NULL,		/* Clear map */
		    SPRITE(med, MAPEDIT_TOOL_CLEAR_MAP), 0, 16, 100);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS|WIDGET_UNFOCUSED_BUTTONUP;
		event_new(bu, "button-pushed", fileops_clear_map, "%p", mv);
		
		bu = button_new(reg, NULL,		/* Toggle grid */
		    SPRITE(med, MAPEDIT_TOOL_GRID),
		    BUTTON_STICKY|BUTTON_PRESSED, 16, 100);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
		event_new(bu, "button-pushed",
		    tilemap_option, "%p, %i", mv, MAPEDIT_TOOL_GRID);

		bu = button_new(reg, NULL,		/* Toggle props */
		    SPRITE(med, MAPEDIT_TOOL_PROPS),
		    BUTTON_STICKY|BUTTON_PRESSED, 16, 100);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
		event_new(bu, "button-pushed",
		    tilemap_option, "%p, %i", mv, MAPEDIT_TOOL_PROPS);
	
		bu = button_new(reg, NULL,		/* Toggle edition */
		    SPRITE(med, MAPEDIT_TOOL_EDIT),
		    BUTTON_STICKY, 16, 100);
		WIDGET(bu)->flags |= WIDGET_NO_FOCUS;
		event_new(bu, "button-pushed",
		    tilemap_option, "%p, %i", mv, MAPEDIT_TOOL_EDIT);
	}
	/* Map view */
	reg = region_new(win, REGION_HALIGN, 0, 10, 100, 90);
	reg->spacing = 1;
	{
		region_attach(reg, mv);
	}
	window_show(win);

	/* Tile selection window */
	win = window_generic_new(152, 287, "mapedit-tmap-%s-tiles", ob->name);
	if (win == NULL) {
		return;					/* Exists */
	}
	event_new(win, "window-close", window_generic_hide, "%p", win);
	mv->tmap_win = win;
	window_set_caption(win, "%s", ob->name);
	{
		struct button *button;
		struct label *lab;
		struct tlist *tl;
		int i;
	
		reg = region_new(win, 0, 0, 0, 100, 85);
		{
			char s[8];

			tl = tlist_new(reg, 100, 100, TLIST_MULTI);
			tlist_set_item_height(tl, TILEH);

			for (i = 0; i < ob->art->nsprites; i++) {
				snprintf(s, 8, "s%d", i);
				tlist_insert_item(tl, ob->art->sprites[i],
				    s, ob);
			}
			
			for (i = 0; i < ob->art->nanims; i++) {
				struct art_anim *an = ob->art->anims[i];
			
				snprintf(s, 8, "a%d", i);
				tlist_insert_item(tl, (an->nframes > 0) ?
				    an->frames[0] : NULL, s, ob);
			}
		}
		
		reg = region_new(win, REGION_HALIGN, 0, 85, 100, 15);
		{
			int i;
			int icons[] = {
				MAPEDIT_TOOL_LEFT,
				MAPEDIT_TOOL_RIGHT,
				MAPEDIT_TOOL_UP,
				MAPEDIT_TOOL_DOWN
			};
		
			lab = label_polled_new(reg, 26, 100, NULL,
			    "%i,%i", &mv->constr.x, &mv->constr.y);
	
			for (i = 0; i < 4; i++) {
				button = button_new(reg, NULL,
				    SPRITE(med, icons[i]), 0, 16, 90);
				event_new(button, "button-pushed",
				    objq_insert_tiles, "%p, %p, %i",
				    tl, mv, i);
			}
		}
	}
}

struct window *
objq_window(struct mapedit *med)
{
	struct window *win;
	struct region *reg;

	win = window_generic_new(215, 140, "mapedit-objq");
	if (win == NULL)
		return (NULL);		/* Exists */
	event_new(win, "window-close", window_generic_hide, "%p", win);
	win->rd.x = view->w - 88;
	win->rd.y = 0;
	window_set_caption(win, "Objects");

	reg = region_new(win, 0, 0, 0, 100, 100);
	{
		struct tlist *tl;

		tl = tlist_new(reg, 100, 100, TLIST_POLL);
		event_new(tl, "tlist-poll", tl_objs_poll, "%p", med);
		event_new(tl, "tlist-changed", tl_objs_selected, "%p", med);
	}
	return (win);
}

