/*	$Csoft: monitor.c,v 1.31 2003/03/20 01:17:06 vedge Exp $	*/

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

#ifdef DEBUG

#include <config/have_jpeg.h>

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
	struct window *(*win_func)() = it->p1;		/* XXX unsafe */
	struct window *win;

	if ((win = (win_func)()) != NULL) {
		window_show(win);
	}
}

void
monitor_init(struct monitor *mon, char *name)
{
	const struct tool_ent {
		int		 ind;
		char		*name;
		struct window	*(*window_func)(void);
	} tool_ents[] = {
		{ MONITOR_FPS_COUNTER, "FPS counter", event_show_fps_counter },
#if defined(DEBUG) && defined(THREADS) && defined(HAVE_JPEG)
		{ MONITOR_SCREENSHOT, "Screenshot", screenshot_window },
#endif
		{ MONITOR_OBJECT_BROWSER, "Objects", object_browser_window },
		{ MONITOR_LEVEL_BROWSER, "Levels", level_browser_window },
		{ MONITOR_WIDGET_BROWSER, "Widgets", widget_browser_window },
		{ MONITOR_VIEW_PARAMS, "View params", view_params_window },
		{ MONITOR_MEDIA_BROWSER, "Graphics", art_browser_window },
	};
	const int ntool_ents = sizeof(tool_ents) / sizeof(tool_ents[0]);
	struct region *reg;

	object_init(&mon->obj, "debug-monitor", name, "monitor",
	    OBJECT_STATIC|OBJECT_ART|OBJECT_CANNOT_MAP, &monitor_ops);

	mon->toolbar = window_new("monitor-toolbar", 0,
	    0, view->h - 124,
	    177, 124,
	    177, 124);
	window_set_caption(mon->toolbar, "Debug monitor");

	reg = region_new(mon->toolbar, 0, 0,  0, 100, 100);
	{
		struct tlist *tl_tools;
		int i;

		tl_tools = tlist_new(reg, 100, 100, 0);
		event_new(tl_tools, "tlist-changed",
		    toolbar_selected_tool, NULL);
		for (i = 0; i < ntool_ents; i++) {
			tlist_insert_item(tl_tools,
			    SPRITE(mon, tool_ents[i].ind), tool_ents[i].name,
			    tool_ents[i].window_func);
		}
	}
}

void
monitor_destroy(void *ob)
{
	struct monitor *mon = ob;

	view_detach(mon->toolbar);
}

#endif	/* DEBUG */
