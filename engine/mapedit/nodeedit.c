/*	$Csoft: nodeedit.c,v 1.19 2003/07/28 15:29:58 vedge Exp $	*/

/*
 * Copyright (c) 2003 CubeSoft Communications, Inc.
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

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/button.h>
#include <engine/widget/label.h>
#include <engine/widget/tlist.h>
#include <engine/widget/primitive.h>

#include "mapedit.h"
#include "mapview.h"

static void
nodeedit_close_win(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	struct mapview *mv = argv[1].p;

	widget_set_int(mv->nodeed.trigger, "state", 0);
	window_hide(win);
}

static void
nodeedit_poll_refs(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct mapview *mv = argv[1].p;
	struct node *node;
	struct noderef *r;
	Uint32 i = 0;
	int sx, sy;
	
	if (!mapview_get_selection(mv, &sx, &sy, NULL, NULL))
		return;

	node = &mv->map->map[sy][sx];
	
	tlist_clear_items(tl);
	TAILQ_FOREACH_REVERSE(r, &node->nrefs, nrefs, noderefq) {
		SDL_Surface *icon = NULL;
		struct gfx_anim *anim;
		char label[TLIST_LABEL_MAX];
		struct transform *tr;
		int trfound = 0;

		switch (r->type) {
		case NODEREF_SPRITE:
			snprintf(label, sizeof(label),
			    "%u. [%u] s(%s:%u)\n",
			    i, r->layer, r->r_sprite.obj->name,
			    r->r_sprite.offs);
			icon = r->r_sprite.obj->gfx->sprites[r->r_sprite.offs];
			break;
		case NODEREF_ANIM:
			snprintf(label, sizeof(label),
			    "%u. [%u] a(%s:%u)\n",
			    i, r->layer, r->r_anim.obj->name, r->r_anim.offs);
			anim = r->r_anim.obj->gfx->anims[r->r_anim.offs];
			if (anim->nframes > 0) {
				icon = anim->frames[0];
			}
			break;
		case NODEREF_WARP:
			snprintf(label, sizeof(label),
			    "%u. [%u]. w(%s:%d,%d)\n",
			    i, r->layer, r->r_warp.map, r->r_warp.x,
			    r->r_warp.y);
			break;
		}

		TAILQ_FOREACH(tr, &r->transforms, transforms) {
			extern const struct transform_ent transforms[];
			extern const int ntransforms;
			int tri;

			for (tri = 0; tri < ntransforms; tri++) {
				if (transforms[tri].type == tr->type) {
					strlcat(label, "+", sizeof(label));
					strlcat(label, transforms[tri].name,
					    sizeof(label));
					break;
				}
			}
			trfound++;
		}
		if (trfound)
			strlcat(label, "\n", sizeof(label));

		tlist_insert_item(tl, icon, label, r);
		i++;
	}
	tlist_restore_selections(tl);
}

enum {
	MAPVIEW_NODE_REMOVE,
	MAPVIEW_NODE_DUP,
	MAPVIEW_NODE_MOVE_UP,
	MAPVIEW_NODE_MOVE_DOWN
};

static void
mapview_node_op(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;
	int op = argv[2].i;
	struct tlist *tl = mv->nodeed.refs_tl;
	struct tlist_item *it;
	struct node *node;
	struct noderef *r;
	int sx, sy;

	if (!mapview_get_selection(mv, &sx, &sy, NULL, NULL)) {
		text_msg(MSG_ERROR, _("No node is selected."));
		return;
	}
	node = &mv->map->map[sy][sx];

	switch (op) {
	case MAPVIEW_NODE_REMOVE:
		TAILQ_FOREACH(it, &tl->items, items) {
			if (it->selected) {
				r = it->p1;
				node_remove_ref(mv->map, node, r);
				it->selected = 0;
			}
		}
		break;
	case MAPVIEW_NODE_DUP:
		TAILQ_FOREACH(it, &tl->items, items) {
			if (it->selected) {
				r = it->p1;
				node_copy_ref(r, mv->map, node,
				    mv->map->cur_layer);
			}
		}
		break;
	case MAPVIEW_NODE_MOVE_UP:
		TAILQ_FOREACH(it, &tl->items, items) {
			if (it->selected) {
				r = it->p1;
				if (TAILQ_NEXT(r, nrefs) == NULL) {
					break;
				}
				node_moveup_ref(node, r);
			}
		}
		break;
	case MAPVIEW_NODE_MOVE_DOWN:
		TAILQ_FOREACH_REVERSE(it, &tl->items, items, tlist_itemq) {
			if (it->selected) {
				r = it->p1;
				if (TAILQ_PREV(r, noderefq, nrefs) == NULL) {
					break;
				}
				node_movedown_ref(node, r);
			}
		}
		break;
	}
}

void
nodeedit_init(struct mapview *mv)
{
	struct map *m = mv->map;
	struct window *win;
	struct box *bo;
	struct tlist *tl;

	win = window_new(NULL);
	window_set_caption(win, _("%s node"), OBJECT(m)->name);
	event_new(win, "window-close", nodeedit_close_win, "%p", mv);
	
	bo = box_new(win, BOX_HORIZ, BOX_HOMOGENOUS|BOX_WFILL);
	{
		struct button *bu;

		bu = button_new(bo, _("Remove"));
		event_new(bu, "button-pushed", mapview_node_op, "%p, %i", mv,
		    MAPVIEW_NODE_REMOVE);
		bu = button_new(bo, _("Dup"));
		event_new(bu, "button-pushed", mapview_node_op, "%p, %i", mv,
		    MAPVIEW_NODE_DUP);
		bu = button_new(bo, _("Up"));
		event_new(bu, "button-pushed", mapview_node_op, "%p, %i", mv,
		    MAPVIEW_NODE_MOVE_UP);
		bu = button_new(bo, _("Down"));
		event_new(bu, "button-pushed", mapview_node_op, "%p, %i", mv,
		    MAPVIEW_NODE_MOVE_DOWN);
	}
	
	tl = tlist_new(win, TLIST_POLL|TLIST_MULTI);
	tlist_set_item_height(tl, TILEH);
	event_new(tl, "tlist-poll", nodeedit_poll_refs, "%p", mv);

	mv->nodeed.refs_tl = tl;
	mv->nodeed.win = win;
}
