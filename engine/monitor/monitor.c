/*	$Csoft: monitor.c,v 1.20 2002/12/01 14:27:41 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#ifdef DEBUG

#include <engine/version.h>
#include <engine/map.h>
#include <engine/physics.h>
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/tlist.h>
#include <engine/mapedit/mapview.h>

#include "monitor.h"

const struct object_ops monitor_ops = {
	monitor_destroy,
	NULL,			/* load */
	NULL			/* save */
};

struct monitor monitor;		/* Debug monitor */

static void
toolbar_selected_tool(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	struct tlist_item *it = argv[1].p;
	struct window *(*win_func)() = it->p1;
	struct window *win;

	if ((win = (win_func)()) != NULL) {
		window_show(win);
	}
}

void
monitor_init(struct monitor *mon, char *name)
{
	struct region *reg;
	struct tlist *tl_tools;

	object_init(&mon->obj, "debug-monitor", name, "monitor",
	    OBJECT_ART|OBJECT_CANNOT_MAP, &monitor_ops);

	mon->toolbar = window_new("monitor-toolbar", 0, 0, view->h - 152,
	    222, 152, 222, 152);
	window_set_caption(mon->toolbar, "Debug monitor");

	reg = region_new(mon->toolbar, 0, 0,  0, 100, 100);
	reg->spacing = 1;

	tl_tools = tlist_new(reg, 100, 100, 0);
	tlist_insert_item(tl_tools,
	    SPRITE(mon, MONITOR_OBJECT_BROWSER), "Object browser",
	    object_browser_window);
	tlist_insert_item(tl_tools,
	    SPRITE(mon, MONITOR_LEVEL_BROWSER), "Level browser",
	    level_browser_window);
	tlist_insert_item(tl_tools,
	    SPRITE(mon, MONITOR_WIDGET_BROWSER), "Widget browser",
	    widget_browser_window);
	tlist_insert_item(tl_tools,
	    SPRITE(mon, MONITOR_VIEW_PARAMS), "Viewport parameters",
	    view_params_window);
	tlist_insert_item(tl_tools,
	    SPRITE(mon, MONITOR_MEDIA_BROWSER), "Art browser",
	    art_browser_window);
	event_new(tl_tools, "tlist-changed", toolbar_selected_tool, NULL);
}

void
monitor_destroy(void *ob)
{
	struct monitor *mon = ob;

	view_detach(mon->toolbar);
}

#endif	/* DEBUG */
