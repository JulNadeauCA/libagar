/*	$Csoft: merge.c,v 1.36 2003/05/24 15:53:42 vedge Exp $	*/

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

#include <engine/compat/snprintf.h>

#include <engine/engine.h>
#include <engine/version.h>
#include <engine/view.h>

#include "merge.h"

#include <engine/widget/radio.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/text.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/tlist.h>

#include <libfobj/vector.h>

#include <string.h>

const struct version merge_ver = {
	"agar merge tool",
	4, 0
};

const struct tool_ops merge_ops = {
	{
		NULL,		/* init */
		merge_destroy,
		merge_load,
		merge_save,
		NULL		/* edit */
	},
	merge_window,
	merge_cursor,
	merge_effect,
	NULL			/* mouse */
};

void
merge_init(void *p)
{
	struct merge *merge = p;

	tool_init(&merge->tool, "merge", &merge_ops);
	merge->mode = MERGE_REPLACE;
	merge->inherit_flags = 1;
	merge->random_shift = 0;
	merge->layer = 0;
	TAILQ_INIT(&merge->brushes);
}

#if 0
static void
merge_free_brushes(struct merge *mer)
{
	struct object *ob, *nob;

	for (ob = TAILQ_FIRST(&mer->brushes);
	     ob != TAILQ_END(&mer->brushes);
	     ob = nob) {
		nob = TAILQ_NEXT(ob, cobjs);
		object_destroy(ob);
		free(ob);
	}
	TAILQ_INIT(&mer->brushes);
}
#endif

void
merge_destroy(void *p)
{
	struct merge *mer = p;

	tool_destroy(mer);
}

static void
merge_create_brush(int argc, union evarg *argv)
{
	struct merge *mer = argv[1].p;
	struct textbox *name_tbox = argv[2].p;
	char brush_name[OBJECT_NAME_MAX];
	char m_name[OBJECT_NAME_MAX];
	struct map *m;

	if (textbox_copy_string(name_tbox, brush_name, sizeof(brush_name) - 8)
	    > sizeof(brush_name) - 8) {
		text_msg("Error", "Brush name too big");
		return;
	}
	if (brush_name[0] == '\0') {
		text_msg("Error", "No brush name given");
		return;
	}
	
	snprintf(m_name, sizeof(m_name), "brush(%s)", brush_name);
	if (tlist_item_text(mer->brushes_tl, m_name) != NULL) {
		text_msg("Error", "%s already exists", m_name);
		return;
	}

	m = Malloc(sizeof(struct map));
	map_init(m, m_name);
	if (object_load(m, NULL) == -1) {
		if (map_alloc_nodes(m,
		    prop_get_uint32(&mapedit, "default-brush-width"),
		    prop_get_uint32(&mapedit, "default-brush-height")) == -1) {
			text_msg("Error", "map_alloc_nodes: %s", error_get());
			map_destroy(m);
			free(m);
			return;
		}
	}

	TAILQ_INSERT_HEAD(&mer->brushes, OBJECT(m), cobjs);
	tlist_unselect_all(mer->brushes_tl);
	tlist_select(mer->brushes_tl,
	    tlist_insert_item_head(mer->brushes_tl, NULL, m_name, m));
	textbox_printf(name_tbox, "");
}

static void
merge_edit_brush(int argc, union evarg *argv)
{
	struct merge *mer = argv[1].p;
	struct window *win;
	struct tlist_item *it;
	struct region *reg;

	TAILQ_FOREACH(it, &mer->brushes_tl->items, items) {
		struct map *brush = it->p1;

		if (!it->selected)
			continue;

		win = window_generic_new(169, 242, "mapedit-tool-merge-%s",
		   OBJECT(brush)->name);
		if (win == NULL) 		/* Exists */
			continue;
		window_set_min_geo(win, 181, 189);

		reg = region_new(win, REGION_VALIGN, 0, 0, 100, 100);
		{
			mapview_new(reg, brush,
			    MAPVIEW_EDIT|MAPVIEW_GRID|MAPVIEW_PROPS,
			    100, 100);
		}
		window_show(win);
	}
}

static void
merge_remove_brush(int argc, union evarg *argv)
{
	struct merge *mer = argv[1].p;
	struct tlist_item *it;

	TAILQ_FOREACH(it, &mer->brushes_tl->items, items) {
		if (it->selected) {
			struct object *brush = it->p1;
			struct window *win;
			char wname[OBJECT_NAME_MAX];

			snprintf(wname, sizeof(wname),
			    "win-mapedit-tool-merge-%s", OBJECT(brush)->name);
			if ((win = view_window_exists(wname)) != NULL) {
				window_hide(win);
				view_detach(win);
			}

			TAILQ_REMOVE(&mer->brushes, brush, cobjs);
			tlist_remove_item(mer->brushes_tl, it);
			object_destroy(brush);
			free(brush);
		}
	}
}

static void
merge_load_brush_set(int argc, union evarg *argv)
{
	struct merge *mer = argv[1].p;

	if (object_load(mer, NULL) == -1)
		text_msg("Error loading brush set", "%s", error_get());
}

static void
merge_save_brush_set(int argc, union evarg *argv)
{
	struct merge *mer = argv[1].p;
	
	if (object_save(mer, NULL) == -1)
		text_msg("Error saving brush set", "%s", error_get());
}

struct window *
merge_window(void *p)
{
	struct merge *mer = p;
	struct window *win;
	struct region *reg;
	struct textbox *name_tbox;

	win = window_new("mapedit-tool-merge", 0,
	    TOOL_DIALOG_X, TOOL_DIALOG_Y,
	    265, 294,
	    265, 294);
	window_set_caption(win, "Merge");

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, -1);
	{
		static const char *mode_items[] = {
			"Replace",
			NULL
		};
		struct radio *rad;
		struct checkbox *cb;

		rad = radio_new(reg, mode_items);
		widget_bind(rad, "value", WIDGET_INT, NULL, &mer->mode);

		cb = checkbox_new(reg, "Inherit node flags");
		widget_bind(cb, "state", WIDGET_INT, NULL, &mer->inherit_flags);
		
		cb = checkbox_new(reg, "Random shift");
		widget_bind(cb, "state", WIDGET_INT, NULL, &mer->random_shift);
	}
	
	reg = region_new(win, REGION_HALIGN, 0, -1, 100, -1);
	{
		struct button *bu;
		
		name_tbox = textbox_new(reg, "Name: ");
		WIDGET(name_tbox)->rw = 75;
		event_new(name_tbox, "textbox-return",
		    merge_create_brush, "%p, %p", mer, name_tbox);

		bu = button_new(reg, "Create", NULL, BUTTON_NOFOCUS, 25, -1);
		button_set_padding(bu, 6);
		event_new(bu, "button-pushed",
		    merge_create_brush, "%p, %p", mer, name_tbox);
	}
	
	reg = region_new(win, REGION_HALIGN, 0, -1, 100, -1);
	{
		struct button *bu;
		
		bu = button_new(reg, "Load set", NULL, BUTTON_NOFOCUS, 25, -1);
		event_new(bu, "button-pushed", merge_load_brush_set, "%p", mer);
		
		bu = button_new(reg, "Save set", NULL, BUTTON_NOFOCUS, 25, -1);
		event_new(bu, "button-pushed", merge_save_brush_set, "%p", mer);

		bu = button_new(reg, "Edit", NULL, BUTTON_NOFOCUS, 25, -1);
		event_new(bu, "button-pushed", merge_edit_brush, "%p", mer);

		bu = button_new(reg, "Remove", NULL, BUTTON_NOFOCUS, 25, -1);
		event_new(bu, "button-pushed", merge_remove_brush, "%p", mer);
	}
	
	reg = region_new(win, REGION_VALIGN, 0, -1, 100, 0);
	{
		mer->brushes_tl = tlist_new(reg, 100, 0, TLIST_MULTI);
	}
	
	return (win);
}

void
merge_effect(void *p, struct mapview *mv, struct node *dst_node)
{
	struct merge *mer = p;
	struct tlist_item *it;
	struct map *dm = mv->map;

	/* Avoid circular references. XXX ugly */
	if (strncmp(OBJECT(dm)->name, "brush(", 6) == 0)
		return;

	TAILQ_FOREACH(it, &mer->brushes_tl->items, items) {
		struct map *sm;
		int sx, sy, dx, dy;

		if (!it->selected)
			continue;

		sm = it->p1;

		for (sy = 0, dy = mv->cy - sm->maph/2;
		     sy < sm->maph && dy < dm->maph;
		     sy++, dy++) {
			for (sx = 0, dx = mv->cx - sm->mapw/2;
			     sx < sm->mapw && dx < dm->mapw;
			     sx++, dx++) {
				merge_interpolate(mer, sm,
				    &sm->map[sy][sx],
				    &dm->map[dy][dx],
				    dx, dy, mv);
			}
		}
	}
}

static void
merge_copy_node(struct merge *mer, struct node *srcnode, struct node *dstnode,
    int dx, int dy)
{
	struct noderef *nref, *nnref;

	TAILQ_FOREACH(nref, &srcnode->nrefs, nrefs) {
		nnref = node_copy_ref(nref, dstnode);
		nnref->layer = mer->layer;
	}

	if (mer->inherit_flags) {
		dstnode->flags = srcnode->flags;
	}
}

static void
merge_copy_edge(struct merge *mer, struct map *sm, struct node *dstnode,
    Uint32 nedge)
{
	int sx, sy;

	/* Copy the first edge with the requested orientation. */
	for (sy = 0; sy < sm->maph; sy++) {
		for (sx = 0; sx < sm->mapw; sx++) {
			struct node *srcnode = &sm->map[sy][sx];
			struct noderef *nref, *nnref;

			if (TAILQ_EMPTY(&srcnode->nrefs) ||
			   (srcnode->flags & nedge) == 0)
				continue;

			/* XXX randomize/increment */
			dprintf("edge\n");
			TAILQ_FOREACH(nref, &srcnode->nrefs, nrefs) {
				nnref = node_copy_ref(nref, dstnode);
				nnref->layer = mer->layer;
			}
			if (mer->inherit_flags) {
				dstnode->flags = srcnode->flags;
			}
			dstnode->flags &= ~(NODE_EDGE_ANY);
			dstnode->flags |= nedge;
			return;
		}
	}
}

static void
merge_copy_filling(struct merge *mer, struct map *sm, struct node *dstnode)
{
	int sx, sy;

	/* Copy the first node with the requested orientation (0=filling). */
	for (sy = 0; sy < sm->maph; sy++) {
		for (sx = 0; sx < sm->mapw; sx++) {
			struct node *srcnode = &sm->map[sy][sx];
			struct noderef *nref, *nnref;
			
			if (TAILQ_EMPTY(&srcnode->nrefs) ||
			   (srcnode->flags & NODE_EDGE_ANY))
				continue;

			dprintf("filling\n");

			/* XXX randomize/increment */
			TAILQ_FOREACH(nref, &srcnode->nrefs, nrefs) {
				nnref = node_copy_ref(nref, dstnode);
				nnref->layer = mer->layer;
			}
			if (mer->inherit_flags) {
				dstnode->flags = srcnode->flags;
			}
			return;
		}
	}
}

#define NODE_EDGE_FILLING 0

void
merge_interpolate(struct merge *mer, struct map *sm, struct node *srcnode,
    struct node *dstnode, int dx, int dy, struct mapview *mv)
{
	Uint32 srcedge = srcnode->flags & NODE_EDGE_ANY;
	Uint32 dstedge = dstnode->flags & NODE_EDGE_ANY;
	struct noderef *nref, *nnref;
	int other = 0;

	mer->layer = mv->map->cur_layer;

	if (TAILQ_EMPTY(&srcnode->nrefs))
		return;

	/* Remove other noderefs on this layer. */
	for (nref = TAILQ_FIRST(&dstnode->nrefs);
	     nref != TAILQ_END(&dstnode->nrefs);
	     nref = nnref) {
		nnref = TAILQ_NEXT(nref, nrefs);
		if (nref->layer == mer->layer) {
			TAILQ_REMOVE(&dstnode->nrefs, nref, nrefs);
			noderef_destroy(nref);
			free(nref);

			other++;
		}
	}
	if (!other) {					/* Empty */
		merge_copy_node(mer, srcnode, dstnode, dx, dy);
		return;
	}

	switch (dstedge) {
	case NODE_EDGE_FILLING:
		switch (srcedge) {
		case NODE_EDGE_FILLING:
			merge_copy_node(mer, srcnode, dstnode, dx, dy);
			break;
		default:
			merge_copy_filling(mer, sm, dstnode);
			break;
		}
		break;
	case NODE_EDGE_NW:
		switch (srcedge) {
		case NODE_EDGE_FILLING:
			merge_copy_node(mer, srcnode, dstnode, dx, dy);
			break;
		case NODE_EDGE_NW:				/* Same */
			merge_copy_edge(mer, sm, dstnode, NODE_EDGE_NW);
			break;
		case NODE_EDGE_N:
		case NODE_EDGE_NE:
			merge_copy_edge(mer, sm, dstnode, NODE_EDGE_N);
			break;
		case NODE_EDGE_SW:
		case NODE_EDGE_S:
		case NODE_EDGE_SE:
		case NODE_EDGE_E:
			merge_copy_filling(mer, sm, dstnode);
			break;
		case NODE_EDGE_W:
			merge_copy_edge(mer, sm, dstnode, NODE_EDGE_W);
			break;
		}
		break;
	case NODE_EDGE_N:
		switch (srcedge) {
		case NODE_EDGE_FILLING:
			merge_copy_node(mer, srcnode, dstnode, dx, dy);
			break;
		case NODE_EDGE_N:				/* Same */
		case NODE_EDGE_NW:
		case NODE_EDGE_NE:
			merge_copy_edge(mer, sm, dstnode, NODE_EDGE_N);
			break;
		case NODE_EDGE_S:
		case NODE_EDGE_SW:
		case NODE_EDGE_SE:
		case NODE_EDGE_W:
		case NODE_EDGE_E:
			merge_copy_filling(mer, sm, dstnode);
			break;
		}
		break;
	case NODE_EDGE_NE:
		switch (srcedge) {
		case NODE_EDGE_FILLING:
			merge_copy_node(mer, srcnode, dstnode, dx, dy);
			break;
		case NODE_EDGE_NE:				/* Same */
			merge_copy_node(mer, srcnode, dstnode, dx, dy);
			break;
		case NODE_EDGE_NW:
		case NODE_EDGE_N:
			merge_copy_edge(mer, sm, dstnode, NODE_EDGE_N);
			break;
		case NODE_EDGE_SW:
		case NODE_EDGE_S:
			merge_copy_filling(mer, sm, dstnode);
			break;
		case NODE_EDGE_SE:
		case NODE_EDGE_E:
			merge_copy_edge(mer, sm, dstnode, NODE_EDGE_E);
			break;
		}
		break;
	case NODE_EDGE_S:
		switch (srcedge) {
		case NODE_EDGE_FILLING:
			merge_copy_node(mer, srcnode, dstnode, dx, dy);
			break;
		case NODE_EDGE_S:				/* Same */
		case NODE_EDGE_SW:
		case NODE_EDGE_SE:
			merge_copy_edge(mer, sm, dstnode, NODE_EDGE_S);
			break;
		case NODE_EDGE_N:
		case NODE_EDGE_NW:
		case NODE_EDGE_NE:
		case NODE_EDGE_W:
		case NODE_EDGE_E:
			merge_copy_filling(mer, sm, dstnode);
			break;
		}
		break;
	case NODE_EDGE_SW:
		switch (srcedge) {
		case NODE_EDGE_FILLING:
			merge_copy_node(mer, srcnode, dstnode, dx, dy);
			break;
		case NODE_EDGE_NW:
			merge_copy_edge(mer, sm, dstnode, NODE_EDGE_W);
			break;
		case NODE_EDGE_N:
			/* XXX ... */
			break;
		case NODE_EDGE_NE:
			merge_copy_edge(mer, sm, dstnode, NODE_EDGE_E);
			break;
		case NODE_EDGE_SW:				/* Same */
			merge_copy_edge(mer, sm, dstnode, NODE_EDGE_SW);
			break;
		case NODE_EDGE_S:
		case NODE_EDGE_SE:
			merge_copy_edge(mer, sm, dstnode, NODE_EDGE_S);
			break;
		case NODE_EDGE_W:
			merge_copy_edge(mer, sm, dstnode, NODE_EDGE_W);
			break;
		case NODE_EDGE_E:
			/* XXX */
			break;
		}
		break;
	case NODE_EDGE_SE:
		switch (srcedge) {
		case NODE_EDGE_FILLING:
			merge_copy_node(mer, srcnode, dstnode, dx, dy);
			break;
		case NODE_EDGE_NW:
			merge_copy_edge(mer, sm, dstnode, NODE_EDGE_W);
			break;
		case NODE_EDGE_N:
			/* XXX ... */
		case NODE_EDGE_NE:
			merge_copy_edge(mer, sm, dstnode, NODE_EDGE_E);
			break;
		case NODE_EDGE_SW:
		case NODE_EDGE_S:
			merge_copy_edge(mer, sm, dstnode, NODE_EDGE_S);
			break;
		case NODE_EDGE_SE:				/* Same */
			merge_copy_edge(mer, sm, dstnode, NODE_EDGE_SE);
			break;
		case NODE_EDGE_W:
		case NODE_EDGE_E:
			merge_copy_edge(mer, sm, dstnode, NODE_EDGE_N);
			break;
		}
		break;
	case NODE_EDGE_W:
		switch (srcedge) {
		case NODE_EDGE_FILLING:
			merge_copy_node(mer, srcnode, dstnode, dx, dy);
			break;
		case NODE_EDGE_S:
		case NODE_EDGE_SE:
		case NODE_EDGE_N:
		case NODE_EDGE_NE:
			merge_copy_filling(mer, sm, dstnode);
			break;
		case NODE_EDGE_W:				/* Same */
		case NODE_EDGE_NW:
		case NODE_EDGE_SW:
			merge_copy_edge(mer, sm, dstnode, NODE_EDGE_W);
			break;
		case NODE_EDGE_E:
			merge_copy_filling(mer, sm, dstnode);
			break;
		}
		break;
	case NODE_EDGE_E:
		switch (srcedge) {
		case NODE_EDGE_FILLING:
			merge_copy_node(mer, srcnode, dstnode, dx, dy);
			break;
		case NODE_EDGE_S:
		case NODE_EDGE_SW:
		case NODE_EDGE_N:
		case NODE_EDGE_NW:
			merge_copy_filling(mer, sm, dstnode);
			break;
		case NODE_EDGE_NE:
		case NODE_EDGE_SE:
			merge_copy_edge(mer, sm, dstnode, NODE_EDGE_E);
			break;
		case NODE_EDGE_W:
			merge_copy_filling(mer, sm, dstnode);
			break;
		case NODE_EDGE_E:				/* Same */
			merge_copy_edge(mer, sm, dstnode, NODE_EDGE_E);
			break;
		}
		break;
	}
}

int
merge_load(void *p, struct netbuf *buf)
{
	struct merge *mer = p;
	Uint32 i, nbrushes;
	
	if (version_read(buf, &merge_ver, NULL) != 0)
		return (-1);

	mer->mode = (Uint32)read_uint32(buf);
	mer->inherit_flags = (Uint32)read_uint32(buf);
	mer->random_shift = (Uint32)read_uint32(buf);

	tlist_clear_items(mer->brushes_tl);
	TAILQ_INIT(&mer->brushes);

	nbrushes = read_uint32(buf);
	for (i = 0; i < nbrushes; i++) {
		struct map *nbrush;
		char m_name[OBJECT_NAME_MAX];

		if (copy_string(m_name, buf, sizeof(m_name)) >=
		    sizeof(m_name)) {
			text_msg("String overflow", "Brush name is too big");
			continue;
		}
		dprintf("loading brush: %s\n", m_name);

		nbrush = Malloc(sizeof(struct map));
		map_init(nbrush, m_name);
		map_load(nbrush, buf);

		TAILQ_INSERT_HEAD(&mer->brushes, OBJECT(nbrush), cobjs);
		tlist_insert_item_head(mer->brushes_tl, NULL, m_name, nbrush);
	}
	return (0);
}

int
merge_save(void *p, struct netbuf *buf)
{
	struct merge *mer = p;
	struct object *ob;
	Uint32 nbrushes = 0;
	off_t count_offs;

	version_write(buf, &merge_ver);

	write_uint32(buf, (Uint32)mer->mode);
	write_uint32(buf, (Uint32)mer->inherit_flags);
	write_uint32(buf, (Uint32)mer->random_shift);

	count_offs = buf->offs;				/* Skip count */
	write_uint32(buf, 0);	
	TAILQ_FOREACH(ob, &mer->brushes, cobjs) {
		struct brush *brush = (struct brush *)ob;

		write_string(buf, ob->name);
		map_save(brush, buf);
		nbrushes++;
	}
	pwrite_uint32(buf, nbrushes, count_offs);	/* Write count */
	return (0);
}

int
merge_cursor(void *p, struct mapview *mv, SDL_Rect *rd)
{
	struct merge *mer = p;
	struct noderef *nref;
	struct map *sm;
	int sx, sy, dx, dy;
	struct tlist_item *it;
	int rv = -1;
	
	/* XXX ugly */
	if (strncmp(OBJECT(mv->map)->name, "brush(", 6) == 0) {
		return (-1);
	}

	TAILQ_FOREACH(it, &mer->brushes_tl->items, items) {
		if (!it->selected)
			continue;
		sm = it->p1;
		for (sy = 0, dy = WIDGET_ABSY(mv) +
		     rd->y - (sm->maph*mv->map->tileh)/2;
		     sy < sm->maph;
		     sy++, dy += mv->map->tileh) {
			for (sx = 0, dx = WIDGET_ABSX(mv) +
			     rd->x - (sm->mapw*mv->map->tilew)/2;
			     sx < sm->mapw;
			     sx++, dx += mv->map->tilew) {
				struct node *srcnode = &sm->map[sy][sx];

				TAILQ_FOREACH(nref, &srcnode->nrefs, nrefs) {
					noderef_draw(mv->map, nref, dx, dy);
					if (mv->flags & MAPVIEW_PROPS) {
						mapview_draw_props(mv,
						    srcnode, dx, dy, -1, -1);
					}
					rv = 0;
				}
			}
		}
	}
	return (rv);
}
