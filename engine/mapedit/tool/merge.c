/*	$Csoft: merge.c,v 1.12 2003/02/13 11:30:14 vedge Exp $	*/

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
#include <engine/version.h>
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/radio.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/text.h>
#include <engine/widget/textbox.h>
#include <engine/widget/button.h>
#include <engine/widget/tlist.h>

#include <engine/mapedit/mapedit.h>
#include <engine/mapedit/mapview.h>

#include <libfobj/fobj.h>
#include <libfobj/vector.h>

#include "tool.h"
#include "merge.h"

static const struct version merge_ver = {
	"agar merge tool",
	3, 0
};

static const struct tool_ops merge_ops = {
	{
		merge_destroy,	/* destroy */
		merge_load,	/* load */
		merge_save	/* save */
	},
	merge_window,
	merge_cursor,
	merge_effect
};

void
merge_init(void *p)
{
	struct merge *merge = p;

	tool_init(&merge->tool, "merge", &merge_ops);
	merge->mode = MERGE_REPLACE;
	merge->inherit_flags = 1;
	merge->random_shift = 0;

	SLIST_INIT(&merge->brushes);
}

void
merge_destroy(void *p)
{
	struct merge *mer = p;
	struct object *ob;
	
	SLIST_FOREACH(ob, &mer->brushes, wobjs) {
		object_destroy(ob);
	}
}

static void
merge_create_brush(int argc, union evarg *argv)
{
	struct merge *mer = argv[1].p;
	struct textbox *name_tbox = argv[2].p;
	char *brush_name, *m_name;
	struct map *m;
	struct tlist_item *it;

	brush_name = textbox_string(name_tbox);
	if (strcmp(brush_name, "") == 0) {
		text_msg("Error", "No brush name given");
		return;
	}
	
	Asprintf(&m_name, "brush(%s)", brush_name);
	if (tlist_item_text(mer->brushes_tl, m_name) != NULL) {
		text_msg("Error", "%s already exists", m_name);
		return;
	}

	m = emalloc(sizeof(struct map));
	map_init(m, MAP_2D, m_name, NULL);

	if (object_load(m) == -1) {
		map_alloc_nodes(m,
		    prop_get_uint32(&mapedit, "default-brush-width"),
		    prop_get_uint32(&mapedit, "default-brush-height"));
	}

	SLIST_INSERT_HEAD(&mer->brushes, OBJECT(m), wobjs);

	tlist_unselect_all(mer->brushes_tl);
	tlist_insert_item_selected(mer->brushes_tl, NULL, m_name, m);
	textbox_printf(name_tbox, "");
	
	free(m_name);
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

		win = window_generic_new(200, 150, "mapedit-tool-merge-%s",
		   OBJECT(brush)->name);
		if (win == NULL) 		/* Exists */
			continue;
		window_set_min_geo(win, 67, 88);

		reg = region_new(win, REGION_VALIGN, 0, 0, 100, 100);
		{
			mapview_new(reg, brush,
			    MAPVIEW_EDIT|MAPVIEW_ZOOM|MAPVIEW_GRID|
			    MAPVIEW_PROPS,
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
			char *wname;

			Asprintf(&wname, "win-mapedit-tool-merge-%s",
			    OBJECT(brush)->name);
			if ((win = view_window_exists(wname)) != NULL) {
				window_hide(win);
				view_detach(win);
			}
			free(wname);
			
			tlist_remove_item(it);
			SLIST_REMOVE(&mer->brushes, brush, object, wobjs);
			object_destroy(brush);
		}
	}
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
	    213, 323,
	    151, 285);
	window_set_caption(win, "Merge");

	reg = region_new(win, REGION_VALIGN, 0, 0, 100, -1);
	{
		static const char *mode_items[] = {
			"Fill",
			"Replace",
			"Insert highest",
			"Insert empty",
			"Erase",
			NULL
		};
		struct radio *rad;
		struct checkbox *cb;

		rad = radio_new(reg, mode_items);
		widget_bind(rad, "value", WIDGET_INT, NULL, &mer->mode);

		cb = checkbox_new(reg, -1, "Inherit node flags");
		widget_bind(cb, "state", WIDGET_INT, NULL, &mer->inherit_flags);
		
		cb = checkbox_new(reg, -1, "Random shift");
		widget_bind(cb, "state", WIDGET_INT, NULL, &mer->random_shift);
	}
	
	reg = region_new(win, REGION_HALIGN, 0, -1, 100, -1);
	{
		struct button *bu;
		
		name_tbox = textbox_new(reg, "Name: ", 0, 75, -1);
		event_new(name_tbox, "textbox-return",
		    merge_create_brush, "%p, %p", mer, name_tbox);

		bu = button_new(reg, "Create", NULL, BUTTON_NOFOCUS, 25, -1);
		event_new(bu, "button-pushed",
		    merge_create_brush, "%p, %p", mer, name_tbox);
	}
	
	reg = region_new(win, REGION_HALIGN, 0, -1, 100, -1);
	{
		struct button *bu;

		bu = button_new(reg, "Edit", NULL, BUTTON_NOFOCUS, 50, -1);
		event_new(bu, "button-pushed", merge_edit_brush, "%p", mer);

		bu = button_new(reg, "Remove", NULL, BUTTON_NOFOCUS, 50, -1);
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
	if (strncmp(OBJECT(dm)->name, "brush(", 6) == 0) {
		text_msg("Error", "Circular reference");
		return;
	}

	TAILQ_FOREACH(it, &mer->brushes_tl->items, items) {
		struct map *sm;
		Uint32 sx, sy, dx, dy;

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
				    dx, dy);
			}
		}
	}
}

static void
merge_copy_node(struct merge *mer, struct node *srcnode, struct node *dstnode,
    Uint32 dx, Uint32 dy)
{
	struct noderef *nref;

	TAILQ_FOREACH(nref, &srcnode->nrefs, nrefs)
		node_copy_ref(nref, dstnode);
	if (mer->inherit_flags)
		dstnode->flags = srcnode->flags;
}

static void
merge_copy_edge(struct merge *mer, struct map *sm, struct node *dstnode,
    Uint32 nedge)
{
	Uint32 sx, sy;

	/* Copy the first edge with the requested orientation. */
	for (sy = 0; sy < sm->maph; sy++) {
		for (sx = 0; sx < sm->mapw; sx++) {
			struct node *srcnode = &sm->map[sy][sx];
			struct noderef *nref;

			if (TAILQ_EMPTY(&srcnode->nrefs) ||
			   (srcnode->flags & nedge) == 0)
				continue;

			/* XXX randomize/increment */
			dprintf("edge\n");
			TAILQ_FOREACH(nref, &srcnode->nrefs, nrefs)
				node_copy_ref(nref, dstnode);
			dstnode->flags = srcnode->flags;
			dstnode->flags &= ~(NODE_EDGE_ANY);
			dstnode->flags |= nedge;
			return;
		}
	}
}

static void
merge_copy_filling(struct merge *mer, struct map *sm, struct node *dstnode)
{
	Uint32 sx, sy;

	/* Copy the first node with the requested orientation (0=filling). */
	for (sy = 0; sy < sm->maph; sy++) {
		for (sx = 0; sx < sm->mapw; sx++) {
			struct node *srcnode = &sm->map[sy][sx];
			struct noderef *nref;
			
			if (TAILQ_EMPTY(&srcnode->nrefs) ||
			   (srcnode->flags & NODE_EDGE_ANY))
				continue;

			dprintf("filling\n");

			/* XXX randomize/increment */
			TAILQ_FOREACH(nref, &srcnode->nrefs, nrefs)
				node_copy_ref(nref, dstnode);
			dstnode->flags = srcnode->flags;
			return;
		}
	}
}

static void
merge_mix_edges(struct merge *mer, struct map *sm, struct node *srcnode,
    struct node *dstnode)
{
	struct noderef *nref;

	dprintf("mix\n");

	TAILQ_FOREACH(nref, &srcnode->nrefs, nrefs) {
		node_copy_ref(nref, dstnode);
	}
	dstnode->flags |= srcnode->flags & NODE_EDGE_ANY;
}

#define NODE_EDGE_FILLING 0

void
merge_interpolate(struct merge *mer, struct map *sm, struct node *srcnode,
    struct node *dstnode, Uint32 dx, Uint32 dy)
{
	Uint32 srcedge = srcnode->flags & NODE_EDGE_ANY;
	Uint32 dstedge = dstnode->flags & NODE_EDGE_ANY;
	
	if (TAILQ_EMPTY(&srcnode->nrefs))
		return;
	if (TAILQ_EMPTY(&dstnode->nrefs)) {
		merge_copy_node(mer, srcnode, dstnode, dx, dy);
		return;
	}

	node_destroy(dstnode);
	node_init(dstnode, dx, dy);

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
			merge_mix_edges(mer, sm, srcnode, dstnode);
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
merge_load(void *p, int fd)
{
	struct merge *mer = p;
	Uint32 i, nbrushes;
	
	if (version_read(fd, &merge_ver) != 0) {
		return (-1);
	}
	
	mer->mode = (Uint32)read_uint32(fd);
	mer->inherit_flags = (Uint32)read_uint32(fd);
	mer->random_shift = (Uint32)read_uint32(fd);

	nbrushes = read_uint32(fd);
	for (i = 0; i < nbrushes; i++) {
		struct map *nbrush;
		char *m_name;

		m_name = read_string(fd, NULL);

		nbrush = emalloc(sizeof(struct map));
		map_init(nbrush, MAP_2D, m_name, NULL);
		map_load(nbrush, fd);

		free(m_name);
	
		SLIST_INSERT_HEAD(&mer->brushes, OBJECT(nbrush), wobjs);
		tlist_insert_item(mer->brushes_tl, NULL, m_name, nbrush);
	}
	return (0);
}

int
merge_save(void *p, int fd)
{
	struct merge *mer = p;
	struct object *ob;
	Uint32 nbrushes = 0;
	off_t count_offs;

	version_write(fd, &merge_ver);

	write_uint32(fd, (Uint32)mer->mode);
	write_uint32(fd, (Uint32)mer->inherit_flags);
	write_uint32(fd, (Uint32)mer->random_shift);

	count_offs = lseek(fd, 0, SEEK_CUR);
	write_uint32(fd, 0);				/* Skip count */
	SLIST_FOREACH(ob, &mer->brushes, wobjs) {
		struct brush *brush = (struct brush *)ob;
	
		write_string(fd, OBJECT(brush)->name);
		map_save(brush, fd);
		nbrushes++;
	}
	pwrite_uint32(fd, nbrushes, count_offs);	/* Write count */
	return (0);
}

int
merge_cursor(void *p, struct mapview *mv, SDL_Rect *rd)
{
	struct merge *mer = p;
	SDL_Surface *srcsu;
	struct noderef *nref;
	struct map *dm = mv->map;
	struct map *sm;
	Uint32 sx, sy, dx, dy;
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
		for (sy = 0, dy = rd->y - (sm->maph*mv->map->tileh)/2;
		     sy < sm->maph;
		     sy++, dy += mv->map->tileh) {
			for (sx = 0, dx = rd->x - (sm->mapw*mv->map->tilew)/2;
			     sx < sm->mapw;
			     sx++, dx += mv->map->tilew) {
				struct node *srcnode = &sm->map[sy][sx];

				TAILQ_FOREACH(nref, &srcnode->nrefs, nrefs) {
					noderef_draw(mv->map, nref, dx, dy);
					mapview_draw_props(mv, srcnode, dx, dy);
					rv = 0;
				}
			}
		}
	}
	return (rv);
}
