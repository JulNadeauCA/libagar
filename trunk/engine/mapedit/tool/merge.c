/*	$Csoft: merge.c,v 1.22 2003/01/26 06:15:21 vedge Exp $	*/

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
	map_alloc_nodes(&merge->brush, 4, 4);
}

void
merge_destroy(void *p)
{
	struct merge *m = p;

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
	    456, 301, 456, 301);
	window_set_caption(win, "Merge");
		
	reg = region_new(win, REGION_VALIGN, 0, 0, 100, 50);
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

	reg = region_new(win, REGION_VALIGN, 0, 50, 100, 50);
	{
		struct mapview *mv;

//		mv = mapview_new(reg, &mer->brush,
//		    MAPVIEW_EDIT|MAPVIEW_PROPS|MAPVIEW_ZOOM,
//		    100, 50);
//		win->focus = WIDGET(mv);
	}
	return (win);
}

void
merge_effect(void *p, struct mapview *mv, struct node *node)
{
	struct merge *mer = p;
}

