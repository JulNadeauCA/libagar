/*	$Csoft: merge.c,v 1.2 2003/02/08 00:33:54 vedge Exp $	*/

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

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/radio.h>
#include <engine/widget/text.h>

#include <engine/mapedit/mapedit.h>
#include <engine/mapedit/mapview.h>

#include "tool.h"
#include "merge.h"

static const struct tool_ops merge_ops = {
	{
		merge_destroy,	/* destroy */
		NULL,		/* load */
		NULL		/* save */
	},
	merge_window,
	NULL,			/* cursor */
	merge_effect
};

void
merge_init(void *p)
{
	struct merge *merge = p;

	tool_init(&merge->tool, "merge", &merge_ops);
	merge->mode = MERGE_REPLACE;

	map_init(&merge->brush, MAP_2D, "brush", NULL);
	if (object_load(&merge->brush) == -1) {
		map_alloc_nodes(&merge->brush, 4, 4);
	}
}

void
merge_destroy(void *p)
{
	struct merge *m = p;

	object_save(&m->brush);
	map_destroy(&m->brush);
}

struct window *
merge_window(void *p)
{
	struct merge *mer = p;
	struct window *win;
	struct region *reg;

	win = window_new("mapedit-tool-merge", 0,
	    TOOL_DIALOG_X, TOOL_DIALOG_Y,
	    175, 222,
	    151, 151);
	window_set_caption(win, "Merge");
		
	reg = region_new(win, REGION_VALIGN, 0, 0, 100, -1);
	{
		static const char *mode_items[] = {
			"Replace",
			"Insert highest",
			NULL
		};
		struct radio *rad;

		rad = radio_new(reg, mode_items);
		widget_bind(rad, "value", WIDGET_INT, NULL, &mer->mode);
	}

	reg = region_new(win, REGION_VALIGN, 0, -1, 100, 0);
	{
		struct mapview *mv;

		mv = mapview_new(reg, &mer->brush,
		    MAPVIEW_EDIT|MAPVIEW_PROPS|MAPVIEW_ZOOM|MAPVIEW_GRID,
		    100, 100);
	}
	return (win);
}

void
merge_effect(void *p, struct mapview *mv, struct node *dst_node)
{
	struct merge *mer = p;
	struct map *sm = &mer->brush;
	struct map *dm = mv->map;
	Uint32 sx, sy, dx, dy;
	struct noderef *nref;

	for (sy = 0, dy = mv->cy - sm->maph/2;
	     sy < sm->maph && dy < dm->maph;
	     sy++, dy++) {
		for (sx = 0, dx = mv->cx - sm->mapw/2;
		     sx < sm->mapw && dx < dm->mapw;
		     sx++, dx++) {
			struct node *srcnode = &sm->map[sy][sx];
			struct node *dstnode = &dm->map[dy][dx];

			switch (mer->mode) {
			case MERGE_REPLACE:
				node_destroy(dstnode);
				node_init(dstnode, dx, dy);
				TAILQ_FOREACH(nref, &srcnode->nrefs, nrefs)
					node_copy_ref(nref, dstnode);
				break;
			case MERGE_INSERT_HIGHEST:
				TAILQ_FOREACH(nref, &srcnode->nrefs, nrefs)
					node_copy_ref(nref, dstnode);
				break;
			case MERGE_INSERT_EMPTY:
				if (!TAILQ_EMPTY(&dstnode->nrefs))
					continue;
				TAILQ_FOREACH(nref, &srcnode->nrefs, nrefs)
					node_copy_ref(nref, dstnode);
				break;
			}
		}
	}
}

