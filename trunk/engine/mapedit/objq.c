/*	$Csoft: objq.c,v 1.79 2003/07/08 00:34:54 vedge Exp $	*/

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
#include <engine/map.h>
#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/hbox.h>
#include <engine/widget/button.h>
#include <engine/widget/tlist.h>

#include <engine/mapedit/mapedit.h>
#include <engine/mapedit/mapview.h>

enum {
	OBJQ_INSERT_LEFT,
	OBJQ_INSERT_RIGHT,
	OBJQ_INSERT_UP,
	OBJQ_INSERT_DOWN
};

static void	load_tileset(int, union evarg *);
static void	save_tileset(int, union evarg *);
static void	close_tileset(int, union evarg *);

static void
toggle_edition(int argc, union evarg *argv)
{
	struct mapedit_tileset *tset = argv[1].p;

	if (tset->mv->flags & MAPVIEW_EDIT) {
		tset->mv->flags &= ~(MAPVIEW_EDIT);
		window_hide(tset->import_win);
	} else {
		tset->mv->flags |= MAPVIEW_EDIT;
		window_show(tset->import_win);
	}
}

/* Generate graphical noderefs from newly imported graphics. */
static void
import_gfx(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	struct mapview *mv = argv[2].p;
	int mode = argv[3].i;
	struct tlist_item *it;
	struct map *m = mv->map;
	int sx, sy, dx, dy;

	if (!mv->esel.set) {
		text_msg(MSG_ERROR, _("There is no active selection."));
		return;
	}

	TAILQ_FOREACH(it, &tl->items, items) {
		struct node *node;
		struct object *pobj = it->p1;
		struct map *submap = it->p1;
		int t, xinc, yinc;
		Uint32 ind;
		unsigned int nw, nh;
 
		if (!it->selected)
			continue;

		ind = (Uint32)atoi(it->text + 1);
		switch (it->text[0]) {
		case 's':
			{
				SDL_Surface *srcsu = SPRITE(pobj, ind);

				t = NODEREF_SPRITE;
				xinc = srcsu->w / TILEW;
				yinc = srcsu->h / TILEW;
			}
			break;
		case 'a':
			{
				struct object *pobj = it->p1;
				struct gfx_anim *anim = ANIM(pobj, ind);
				SDL_Surface *srcsu = anim->frames[0];

				t = NODEREF_ANIM;
				xinc = srcsu->w / TILEW;
				yinc = srcsu->h / TILEW;
			}
			break;
		case 'm':
			t = -1;
			xinc = submap->mapw;
			yinc = submap->maph;
			break;
		default:
			fatal("bad ref");
		}

		nw = mv->esel.x + xinc + 1;
		nh = mv->esel.y + yinc + 1;

		if (map_resize(m,
		    nw > m->mapw ? nw : m->mapw,
		    nh > m->maph ? nh : m->maph) == -1) {
			text_msg(MSG_ERROR, "%s", error_get());
			continue;
		}

		switch (t) {
		case NODEREF_SPRITE:
			node = &m->map[mv->esel.y][mv->esel.x];
			node_destroy(m, node);
			node_init(node);
			dprintf("+sprite: %s:%d\n", pobj->name, ind);
			node_add_sprite(m, node, pobj, ind);
			break;
		case NODEREF_ANIM:
			node = &m->map[mv->esel.y][mv->esel.x];
			node_destroy(m, node);
			node_init(node);
			dprintf("+anim: %s:%d\n", pobj->name, ind);
			node_add_anim(m, node, pobj, ind, NODEREF_ANIM_AUTO);
			break;
		case -1:					/* Submap */
			dprintf("+submap %u,%u\n", submap->mapw, submap->maph);
			for (sy = 0, dy = mv->esel.y;
			     sy < submap->maph && dy < m->maph;
			     sy++, dy++) {
				for (sx = 0, dx = mv->esel.x;
				     sx < submap->mapw && dx < m->mapw;
				     sx++, dx++) {
					struct node *sn = &submap->map[sy][sx];
					struct node *dn = &m->map[dy][dx];

					node_destroy(m, dn);
					node_init(dn);
					node_copy(submap, sn, -1, m, dn, -1);
				}
			}
			break;
		default:
			fatal("bad nref");
		}

		switch (mode) {
		case OBJQ_INSERT_LEFT:
			if ((mv->esel.x -= xinc) < 0)
				mv->esel.x = 0;
			break;
		case OBJQ_INSERT_RIGHT:
			mv->esel.x += xinc;
			break;
		case OBJQ_INSERT_UP:
			if ((mv->esel.y -= yinc) < 0)
				mv->esel.y = 0;
			break;
		case OBJQ_INSERT_DOWN:
			mv->esel.y += yinc;
			break;
		}
		mv->esel.w = 1;
		mv->esel.h = 1;
	}
}

/* Create the graphic import selection window. */
static struct window *
gfx_import_window(struct object *ob, struct mapview *mv)
{
	char label[TLIST_LABEL_MAX];
	struct window *win;
	struct hbox *hb;
	struct tlist *tl;
	Uint32 i;

	win = window_new(NULL);
	window_set_caption(win, _("%s graphics"), ob->name);
	window_set_closure(win, WINDOW_HIDE);

	tl = tlist_new(win, TLIST_MULTI);
	tlist_set_item_height(tl, ttf_font_height(font)*2);

	for (i = 0; i < ob->gfx->nsubmaps; i++) {
		struct map *sm = ob->gfx->submaps[i];

		snprintf(label, sizeof(label), _("m%u\n%ux%u nodes\n"), i,
		    sm->mapw, sm->maph);
		tlist_insert_item(tl, NULL, label, sm);
	}
	for (i = 0; i < ob->gfx->nsprites; i++) {
		SDL_Surface *sp = ob->gfx->sprites[i];
	
		snprintf(label, sizeof(label),
		    "s%u\n%ux%u pixels, %ubpp\n", i, sp->w, sp->h,
		    sp->format->BitsPerPixel);
		tlist_insert_item(tl, ob->gfx->sprites[i], label, ob);
	}
	for (i = 0; i < ob->gfx->nanims; i++) {
		struct gfx_anim *an = ob->gfx->anims[i];

		snprintf(label, sizeof(label), _("a%u\n%u frames\n"), i,
		    an->nframes);
		tlist_insert_item(tl, (an->nframes > 0) ?
		    an->frames[0] : NULL, label, ob);
	}

	hb = hbox_new(win, HBOX_HOMOGENOUS|HBOX_WFILL);
	{
		int i;
		int icons[] = {
			MAPEDIT_TOOL_LEFT,
			MAPEDIT_TOOL_RIGHT,
			MAPEDIT_TOOL_UP,
			MAPEDIT_TOOL_DOWN
		};
		struct button *bu;

		for (i = 0; i < 4; i++) {
			bu = button_new(hb, NULL);
			button_set_label(bu, SPRITE(&mapedit, icons[i]));
			event_new(bu, "button-pushed", import_gfx,
			    "%p, %p, %i", tl, mv, i);
		}
#if 0
		bu = button_new(hb, _("Replace"));
		button_set_sticky(bu, 1);
		widget_bind(bu, "state", WIDGET_INT, NULL, &mv->constr.replace);
#endif
	}
	return (win);
}

/* Load a tileset from disk. */
static void
load_tileset(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;

	if (object_load(mv->map) == -1) {
		text_msg(MSG_ERROR, "%s: %s", OBJECT(mv->map)->name,
		    error_get());
	}
}

/* Save a tileset to disk. */
static void
save_tileset(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;

	if (object_save(mv->map) == -1) {
		text_msg(MSG_ERROR, "%s: %s", OBJECT(mv->map)->name,
		    error_get());
		return;
	}
}

/* Display a tileset. */
static void
open_tileset(int argc, union evarg *argv)
{
	struct tlist_item *eob_item = argv[1].p;
	struct object *ob = eob_item->p1;
	struct mapedit_tileset *tset;
	struct box *bo;
	struct button *bu;

	TAILQ_FOREACH(tset, &mapedit.tilesets, tilesets) {
		if (tset->obj == ob)
			break;
	}
	if (tset != NULL) {
		window_show(tset->win);
		return;
	}
	
	object_add_dep(mapedit.pseudo, ob);

	tset = Malloc(sizeof(struct mapedit_tileset));
	tset->obj = ob;
	tset->map = ob->gfx->tile_map;
	tset->win = window_new(NULL);
	window_set_caption(tset->win, "%s", ob->name);
	window_set_position(tset->win, WINDOW_MIDDLE_RIGHT, 1);
	
	tset->mv = Malloc(sizeof(struct mapview));
	mapview_init(tset->mv, tset->map, MAPVIEW_TILESET|MAPVIEW_PROPS|
	                                  MAPVIEW_GRID);
	window_attach(tset->win, tset->mv->nodeed.win);
	window_attach(tset->win, tset->mv->layed.win);

	mapview_set_selection(tset->mv, 0, 0, 1, 1);
	mapview_prescale(tset->mv, 1, 8);

	if (object_load(tset->map) == -1)
		dprintf("loading tileset: %s\n", error_get());

	/* Create the map operation buttons. */
	bo = box_new(tset->win, BOX_HORIZ, BOX_HOMOGENOUS|BOX_WFILL);
	WIDGET(bo)->flags |= WIDGET_CLIPPING;
	box_set_spacing(bo, 0);
	{
		bu = button_new(bo, NULL);
		button_set_label(bu, SPRITE(&mapedit, MAPEDIT_TOOL_LOAD_MAP));
		button_set_focusable(bu, 0);
		event_new(bu, "button-pushed", load_tileset, "%p", tset->mv);

		bu = button_new(bo, NULL);
		button_set_label(bu, SPRITE(&mapedit, MAPEDIT_TOOL_SAVE_MAP));
		button_set_focusable(bu, 0);
		event_new(bu, "button-pushed", save_tileset, "%p", tset->mv);
		
		bu = button_new(bo, NULL);
		button_set_label(bu, SPRITE(&mapedit, MAPEDIT_TOOL_EDIT));
		button_set_sticky(bu, 1);
		button_set_focusable(bu, 0);
		event_new(bu, "button-pushed", toggle_edition, "%p", tset);
	}
	object_attach(tset->win, tset->mv);
	event_new(tset->win, "window-close", close_tileset, "%p", tset);

	/* Create the graphic import window. */
	tset->import_win = gfx_import_window(ob, tset->mv);
	TAILQ_INSERT_TAIL(&mapedit.tilesets, tset, tilesets);
	window_attach(tset->win, tset->import_win);
	window_show(tset->win);
}

/* Close a tileset display. */
static void
close_tileset(int argc, union evarg *argv)
{
	struct mapedit_tileset *tset = argv[1].p;

	object_del_dep(mapedit.pseudo, tset->obj);

	TAILQ_REMOVE(&mapedit.tilesets, tset, tilesets);
	free(tset);
	
	view_detach(tset->win);
}

/* Recursive function to find objects linked to gfx. */
static void
find_gfx(struct tlist *tl, struct object *pob)
{
	char label[TLIST_LABEL_MAX];
	struct object *cob;

	TAILQ_FOREACH(cob, &pob->childs, cobjs) {
		if (cob->gfx != NULL) {
			snprintf(label, sizeof(label), "%s\n%ua/%us/%um\n",
			    cob->name, cob->gfx->nanims, cob->gfx->nsprites,
			    cob->gfx->nsubmaps);
			tlist_insert_item(tl, OBJECT_ICON(cob), label, cob);
		}
		find_gfx(tl, cob);
	}
}

static void
poll_tilesets(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;

	tlist_clear_items(tl);
	
	lock_linkage();
	find_gfx(tl, world);
	unlock_linkage();

	tlist_restore_selections(tl);
}

/* Create the tilesets window . */
struct window *
tilesets_window(void)
{
	struct window *win;
	struct tlist *tl;

	if ((win = window_new("mapedit-tilesets")) == NULL) {
		return (NULL);
	}
	window_set_caption(win, _("Tilesets"));
	window_set_position(win, WINDOW_UPPER_RIGHT, 0);
	window_set_closure(win, WINDOW_HIDE);

	tl = tlist_new(win, TLIST_POLL);
	tlist_set_item_height(tl, ttf_font_height(font)*2);
	event_new(tl, "tlist-poll", poll_tilesets, NULL);
	event_new(tl, "tlist-changed", open_tileset, NULL);
	return (win);
}

