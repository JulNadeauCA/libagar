/*	$Csoft: nodeedit.c,v 1.1 2003/03/02 04:08:54 vedge Exp $	*/

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

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/button.h>
#include <engine/widget/label.h>
#include <engine/widget/tlist.h>
#include <engine/widget/text.h>

#include "mapedit.h"
#include "mapview.h"

static void
nodeedit_close_win(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	struct mapview *mv = argv[1].p;

	widget_set_int(mv->nodeed.trigger, "state", 0);
}

static void
nodeedit_poll(int argc, union evarg *argv)
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
		label_printf(mv->nodeed.node_flags_lab, "-");
		label_printf(mv->nodeed.node_size_lab, "-");
		return;
	}
	
	flags[0] = '\0';
	if (node->flags & NODE_ORIGIN)		strcat(flags, "origin ");
	else if (node->flags & NODE_WALK)	strcat(flags, "walk ");
	else if (node->flags & NODE_CLIMB)	strcat(flags, "climb ");
	if (node->flags & NODE_SLIP)		strcat(flags, "slip ");
	if (node->flags & NODE_BIO)		strcat(flags, "bio ");
	else if (node->flags & NODE_REGEN)	strcat(flags, "regen ");
	if (node->flags & NODE_SLOW)		strcat(flags, "slow ");
	else if (node->flags & NODE_HASTE)	strcat(flags, "haste ");
	if (node->flags & NODE_HAS_ANIM)	strcat(flags, "has-anim ");
	
	if (node->flags & (NODE_EDGE_NW))	strcat(flags, "NW-edge ");
	else if (node->flags & (NODE_EDGE_NE))	strcat(flags, "NE-edge ");
	else if (node->flags & (NODE_EDGE_SW))	strcat(flags, "SW-edge ");
	else if (node->flags & (NODE_EDGE_SE))	strcat(flags, "SE-edge ");
	else if (node->flags & NODE_EDGE_N)	strcat(flags, "N-edge ");
	else if (node->flags & NODE_EDGE_S)	strcat(flags, "S-edge ");

	label_printf(mv->nodeed.node_flags_lab, "Node flags: %s", flags);

	tlist_clear_items(tl);

	TAILQ_FOREACH_REVERSE(nref, &node->nrefs, nrefs, noderefq) {
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
	
	label_printf(mv->nodeed.node_size_lab, "Node size: %lu bytes",
	    (unsigned long)nodesz);
	label_printf(mv->nodeed.noderef_type_lab, "-");
	label_printf(mv->nodeed.noderef_flags_lab, "-");
	label_printf(mv->nodeed.noderef_center_lab, "-");

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
			
			label_printf(mv->nodeed.noderef_type_lab,
			    "Noderef type: %s", type);
			label_printf(mv->nodeed.noderef_flags_lab,
			    "Noderef flags: %s", flags);
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
	struct node *node = mv->cur_node;
	struct noderef *nref;

	if (node == NULL) {
		text_msg("Error", "No node is selected");
		return;
	}

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
	struct region *reg;

	win = window_generic_new(268, 346, "mapedit-node-%s-%s",
	    OBJECT(mv)->name, OBJECT(m)->name);
	if (win == NULL) {
		return;						/* Exists */
	}
	window_set_caption(win, "%s node", OBJECT(m)->name);
	window_set_min_geo(win, 175, 160);
	event_new(win, "window-close", nodeedit_close_win, "%p", mv);
	
	reg = region_new(win, REGION_VALIGN, 0, 0, 100, -1);
	{
		mv->nodeed.node_flags_lab = label_new(reg, 100, -1, " ");
		mv->nodeed.node_size_lab = label_new(reg, 100, -1, " ");
	}
	
	reg = region_new(win, REGION_VALIGN, 0, -1, 100, -1);
	{
		mv->nodeed.noderef_type_lab = label_new(reg, 100, -1, " ");
		mv->nodeed.noderef_flags_lab = label_new(reg, 100, -1, " ");
		mv->nodeed.noderef_center_lab = label_new(reg, 100, -1, " ");
	}

	reg = region_new(win, REGION_HALIGN, 0, -1, 100, -1);
	{
		struct button *bu;

		bu = button_new(reg, "Remove", NULL, 0, 25, -1);
		event_new(bu, "button-pushed",
		    mapview_node_op, "%p, %i", mv, MAPVIEW_NODE_REMOVE);

		bu = button_new(reg, "Dup", NULL, 0, 25, -1);
		event_new(bu, "button-pushed",
		    mapview_node_op, "%p, %i", mv, MAPVIEW_NODE_DUP);
		
		bu = button_new(reg, "Up", NULL, 0, 25, -1);
		event_new(bu, "button-pushed",
		    mapview_node_op, "%p, %i", mv, MAPVIEW_NODE_MOVE_UP);

		bu = button_new(reg, "Down", NULL, 0, 25, -1);
		event_new(bu, "button-pushed",
		    mapview_node_op, "%p, %i", mv, MAPVIEW_NODE_MOVE_DOWN);
	}
	
	reg = region_new(win, REGION_VALIGN, 0, -1, 100, 0);
	{
		mv->nodeed.refs_tl = tlist_new(reg, 100, 100,
		    TLIST_POLL|TLIST_MULTI);
		tlist_set_item_height(mv->nodeed.refs_tl, TILEH);
		event_new(mv->nodeed.refs_tl, "tlist-poll",
		    nodeedit_poll, "%p", mv);
	}

	mv->nodeed.win = win;
}
