/*	$Csoft: nodeedit.c,v 1.11 2003/05/20 12:05:13 vedge Exp $	*/

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
#include <engine/widget/vbox.h>
#include <engine/widget/hbox.h>
#include <engine/widget/button.h>
#include <engine/widget/label.h>
#include <engine/widget/tlist.h>
#include <engine/widget/primitive.h>

#include "mapedit.h"
#include "mapview.h"

static void
nodeedit_close_win(int argc, union evarg *argv)
{
	struct mapview *mv = argv[1].p;

	widget_set_int(mv->nodeed.trigger, "state", 0);
}

static void
nodeedit_poll(int argc, union evarg *argv)
{
	static char flags[96];
	struct tlist *tl = argv[0].p;
	struct mapview *mv = argv[1].p;
	struct node *node;
	struct tlist_item *it;
	struct noderef *nref;
	Uint32 i = 0;
	int sx, sy;
	
	if (!mapview_get_selection(mv, &sx, &sy, NULL, NULL)) {
		label_printf(mv->nodeed.node_flags_lab, "");
		return;
	}
	node = &mv->map->map[sy][sx];
	
	flags[0] = '\0';
	if (node->flags & NODE_WALK) {				/* Movement */
		strlcat(flags, "walk ", sizeof(flags));
	} else if (node->flags & NODE_CLIMB) {
		strlcat(flags, "climb ", sizeof(flags));
	} else {
		strlcat(flags, "block ", sizeof(flags));
	}
	if (node->flags & NODE_SLIP)
		strlcat(flags, "slip ", sizeof(flags));

	if (node->flags & NODE_BIO) {				/* Health mod */
		strlcat(flags, "bio ", sizeof(flags));
	} else if (node->flags & NODE_REGEN) {
		strlcat(flags, "regen ", sizeof(flags));
	}
	if (node->flags & NODE_SLOW) {				/* Speed mod */
		strlcat(flags, "slow ", sizeof(flags));
	} else if (node->flags & NODE_HASTE) {
		strlcat(flags, "haste ", sizeof(flags));
	}

	if (node->flags & (NODE_EDGE_NW)) {			/* Edges */
		strlcat(flags, "NW-edge ", sizeof(flags));
	} else if (node->flags & (NODE_EDGE_NE)) {
		strlcat(flags, "NE-edge ", sizeof(flags));
	} else if (node->flags & (NODE_EDGE_SW)) {
		strlcat(flags, "SW-edge ", sizeof(flags));
	} else if (node->flags & (NODE_EDGE_SE)) {
		strlcat(flags, "SE-edge ", sizeof(flags));
	} else if (node->flags & NODE_EDGE_N) {
		strlcat(flags, "N-edge ", sizeof(flags));
	} else if (node->flags & NODE_EDGE_S) {
		strlcat(flags, "S-edge ", sizeof(flags));
	}

	label_printf(mv->nodeed.node_flags_lab, "Node flags: [ %s]", flags);

	tlist_clear_items(tl);

	TAILQ_FOREACH_REVERSE(nref, &node->nrefs, nrefs, noderefq) {
		SDL_Surface *icon = NULL;
		struct art_anim *anim;
		char label[TLIST_LABEL_MAX];

		switch (nref->type) {
		case NODEREF_SPRITE:
			snprintf(label, sizeof(label), "%u. [%u] s(%s:%u)",
			    i, nref->layer, nref->pobj->name, nref->offs);
			icon = nref->pobj->art->sprites[nref->offs];
			break;
		case NODEREF_ANIM:
			snprintf(label, sizeof(label), "%u. [%u] a(%s:%u)",
			    i, nref->layer, nref->pobj->name, nref->offs);
			anim = nref->pobj->art->anims[nref->offs];
			if (anim->nframes > 0) {
				icon = anim->frames[0];
			}
			break;
		case NODEREF_WARP:
			snprintf(label, sizeof(label), "%u. [%u]. w(%s:%d,%d)",
			    i, nref->layer, nref->data.warp.map,
			    nref->data.warp.x, nref->data.warp.y);
			break;
		}
		tlist_insert_item(tl, icon, label, nref);
		i++;
	}

	tlist_restore_selections(tl);
	
	label_printf(mv->nodeed.noderef_type_lab, "");
	label_printf(mv->nodeed.noderef_flags_lab, "");
	label_printf(mv->nodeed.noderef_center_lab, "");

	TAILQ_FOREACH(it, &tl->items, items) {
		if (it->selected) {
			struct noderef *nref = it->p1;
			char nrflags[32];
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
			
			nrflags[0] = '\0';
			if (nref->flags & NODEREF_SAVEABLE)
				strlcat(nrflags, "saveable ", sizeof(nrflags));
			if (nref->flags & NODEREF_BLOCK)
				strlcat(nrflags, "block ", sizeof(nrflags));

			label_printf(mv->nodeed.noderef_type_lab,
			    "Noderef type: %s", type);
			label_printf(mv->nodeed.noderef_flags_lab,
			    "Noderef flags: [ %s]", nrflags);
			label_printf(mv->nodeed.noderef_center_lab,
			    "Noderef centering: %d,%d",
			    nref->xcenter, nref->ycenter);
			break;
		}
	}
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
	struct noderef *nref;
	int sx, sy;

	if (!mapview_get_selection(mv, &sx, &sy, NULL, NULL)) {
		text_msg("Error", "No node is selected");
		return;
	}
	node = &mv->map->map[sy][sx];

	switch (op) {
	case MAPVIEW_NODE_REMOVE:
		TAILQ_FOREACH(it, &tl->items, items) {
			if (it->selected) {
				nref = it->p1;
				node_remove_ref(node, nref);
				it->selected = 0;
			}
		}
		break;
	case MAPVIEW_NODE_DUP:
		TAILQ_FOREACH(it, &tl->items, items) {
			if (it->selected) {
				nref = it->p1;
				node_copy_ref(nref, node);
			}
		}
		break;
	case MAPVIEW_NODE_MOVE_UP:
		TAILQ_FOREACH(it, &tl->items, items) {
			if (it->selected) {
				nref = it->p1;
				if (TAILQ_NEXT(nref, nrefs) == NULL)
					break;
				node_moveup_ref(node, nref);
			}
		}
		break;
	case MAPVIEW_NODE_MOVE_DOWN:
		TAILQ_FOREACH_REVERSE(it, &tl->items, items, tlist_itemq) {
			if (it->selected) {
				nref = it->p1;
				if (TAILQ_PREV(nref, noderefq, nrefs) == NULL)
					break;
				node_movedown_ref(node, nref);
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
	struct vbox *vb;
	struct hbox *hb;
	struct tlist *tl;

	if ((win = window_new("mapedit-node-%s-%s", OBJECT(mv)->name,
	    OBJECT(m)->name)) == NULL) {
		return;
	}
	window_set_caption(win, "%s node", OBJECT(m)->name);
	event_new(win, "window-close", nodeedit_close_win, "%p", mv);
	
	vb = vbox_new(win, VBOX_WFILL);
	WIDGET(vb)->flags |= WIDGET_CLIPPING;
	{
		mv->nodeed.node_flags_lab = label_new(vb, "");
		mv->nodeed.noderef_type_lab = label_new(vb, "");
		mv->nodeed.noderef_flags_lab = label_new(vb, "");
		mv->nodeed.noderef_center_lab = label_new(vb, "");
	}

	hb = hbox_new(win, HBOX_HOMOGENOUS|HBOX_WFILL);
	{
		struct button *bu;

		bu = button_new(hb, "Remove");
		event_new(bu, "button-pushed", mapview_node_op, "%p, %i", mv,
		    MAPVIEW_NODE_REMOVE);
		bu = button_new(hb, "Dup");
		event_new(bu, "button-pushed", mapview_node_op, "%p, %i", mv,
		    MAPVIEW_NODE_DUP);
		bu = button_new(hb, "Move up");
		event_new(bu, "button-pushed", mapview_node_op, "%p, %i", mv,
		    MAPVIEW_NODE_MOVE_UP);
		bu = button_new(hb, "Move down");
		event_new(bu, "button-pushed", mapview_node_op, "%p, %i", mv,
		    MAPVIEW_NODE_MOVE_DOWN);
	}
	
	tl = tlist_new(win, TLIST_POLL|TLIST_MULTI);
	tlist_set_item_height(tl, TILEH);
	event_new(mv->nodeed.refs_tl, "tlist-poll", nodeedit_poll, "%p", mv);

	mv->nodeed.refs_tl = tl;
	mv->nodeed.win = win;
}
