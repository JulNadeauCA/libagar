/*	$Csoft: objq.c,v 1.73 2003/06/26 02:34:52 vedge Exp $	*/

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

static void
toggle_edition(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;

	if (mv->flags & MAPVIEW_EDIT) {
		mv->flags &= ~(MAPVIEW_EDIT);
		window_hide(mv->constr.win);
	} else {
		mv->flags |= MAPVIEW_EDIT;
		window_show(mv->constr.win);
	}
}

/* Generate graphical noderefs from newly imported graphics. */
static void
import_gfx(int argc, union evarg *argv)
{
	struct tlist *tl = argv[1].p;
	struct mapview *mv = argv[2].p;
	struct mapview_constr *con = &mv->constr;
	int mode = argv[3].i;
	struct tlist_item *it;
	struct map *m = mv->map;
	int sx, sy, dx, dy;

	if (!mv->esel.set) {
		text_msg(MSG_ERROR, _("No selection"));
		return;
	}

	con->x = mv->esel.x;
	con->y = mv->esel.y;

	TAILQ_FOREACH(it, &tl->items, items) {
		struct node *node;
		struct object *pobj = it->p1;
		struct map *submap = it->p1;
		int t, xinc, yinc;
		Uint32 ind;
		unsigned int nw, nh;
 
		if (!it->selected || it->text_len < 2)
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

		nw = con->x + xinc + 1;
		nh = con->y + yinc + 1;

		if (map_resize(m,
		    nw > m->mapw ? nw : m->mapw,
		    nh > m->maph ? nh : m->maph) == -1) {
			text_msg(MSG_ERROR, "%s", error_get());
			continue;
		}

		switch (t) {
		case NODEREF_SPRITE:
			node = &m->map[con->y][con->x];
			if (con->replace) {
				node_init(node);
			}
			dprintf("+sprite: %s:%d\n", pobj->name, ind);
			node_add_sprite(m, node, pobj, ind);
			break;
		case NODEREF_ANIM:
			node = &m->map[con->y][con->x];
			if (con->replace) {
				node_init(node);
			}
			dprintf("+anim: %s:%d\n", pobj->name, ind);
			node_add_anim(m, node, pobj, ind, NODEREF_ANIM_AUTO);
			break;
		case -1:					/* Submap */
			dprintf("+submap %u,%u\n", submap->mapw, submap->maph);
			for (sy = 0, dy = con->y;
			     sy < submap->maph && dy < m->maph;
			     sy++, dy++) {
				for (sx = 0, dx = con->x;
				     sx < submap->mapw && dx < m->mapw;
				     sx++, dx++) {
					struct node *sn = &submap->map[sy][sx];
					struct node *dn = &m->map[dy][dx];

					if (con->replace) {
						node_init(dn);
					}
					node_copy(submap, sn, -1, m, dn, -1);
				}
			}
			break;
		default:
			fatal("bad nref");
		}

		switch (mode) {
		case OBJQ_INSERT_LEFT:
			if ((con->x -= xinc) < 0)
				con->x = 0;
			break;
		case OBJQ_INSERT_RIGHT:
			con->x += xinc;
			break;
		case OBJQ_INSERT_UP:
			if ((con->y -= yinc) < 0)
				con->y = 0;
			break;
		case OBJQ_INSERT_DOWN:
			con->y += yinc;
			break;
		}
	}
}

/* Close a tileset display. */
static void
close_tileset(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	struct mapview *mv = argv[1].p;

	if (mv->constr.win != NULL) {
		window_hide(mv->constr.win);
	}
	window_hide(win);
}

/* Set insert/replace mode. */
static void
tog_replace(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	int state = argv[2].i;

	mv->constr.replace = state;
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

	if ((win = window_new("mapedit-tss-%s", ob->name)) == NULL) {
		return (NULL);
	}
	window_set_caption(win, "%s", ob->name);
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
		bu = button_new(hb, _("Replace"));
		button_set_sticky(bu, 1);
		event_new(bu, "button-pushed", tog_replace, "%p", mv);
	}
	return (win);
}

/* Display a tileset. */
static void
open_tileset(int argc, union evarg *argv)
{
	struct tlist_item *eob_item = argv[1].p;
	struct object *ob = eob_item->p1;
	struct window *win;
	struct box *bo;
	struct mapview *mv;
	struct button *bu;

	if ((win = window_new("mapedit-ts-%s", ob->name)) == NULL) {
		return;
	}
	window_set_caption(win, "%s", ob->name);
	window_set_position(win, WINDOW_MIDDLE_RIGHT, 1);

	mv = Malloc(sizeof(struct mapview));
	mapview_init(mv, ob->gfx->tile_map,
	    MAPVIEW_TILESET|MAPVIEW_PROPS|MAPVIEW_GRID);
	mapview_set_selection(mv, 0, 0, 1, 1);
	mapview_prescale(mv, 1, 4);

	object_load(ob->gfx->tile_map);

	/* Map operation buttons */
	bo = box_new(win, BOX_HORIZ, BOX_HOMOGENOUS|BOX_WFILL);
	WIDGET(bo)->flags |= WIDGET_CLIPPING;
	box_set_spacing(bo, 0);
	{
		bu = button_new(bo, NULL);
		button_set_label(bu, SPRITE(&mapedit, MAPEDIT_TOOL_LOAD_MAP));
		button_set_focusable(bu, 0);
		event_new(bu, "button-pushed", fileops_revert_map, "%p", mv);

		bu = button_new(bo, NULL);
		button_set_label(bu, SPRITE(&mapedit, MAPEDIT_TOOL_SAVE_MAP));
		button_set_focusable(bu, 0);
		event_new(bu, "button-pushed", fileops_save_map, "%p", mv);
		
		bu = button_new(bo, NULL);
		button_set_label(bu, SPRITE(&mapedit, MAPEDIT_TOOL_EDIT));
		button_set_sticky(bu, 1);
		button_set_focusable(bu, 0);
		event_new(bu, "button-pushed", toggle_edition, "%p", mv);
	}
	object_attach(win, mv);
	event_new(win, "window-close", close_tileset, "%p", mv);

	mv->constr.win = gfx_import_window(ob, mv);
	window_show(win);
}

/* Recursive function to find objects linked to gfx. */
static void
find_gfx(struct tlist *tl, struct object *pob)
{
	char label[TLIST_LABEL_MAX];
	struct object *cob;

	TAILQ_FOREACH(cob, &pob->childs, cobjs) {
		if (cob->gfx == NULL)
			continue;
		snprintf(label, sizeof(label), "%s\n%ua/%us/%um\n",
		    cob->name, cob->gfx->nanims, cob->gfx->nsprites,
		    cob->gfx->nsubmaps);
		tlist_insert_item(tl, OBJECT_ICON(cob), label, cob);
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

