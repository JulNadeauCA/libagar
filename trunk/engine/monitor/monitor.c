/*	$Csoft: monitor.c,v 1.39 2003/06/06 02:44:04 vedge Exp $	*/

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

#include <config/have_jpeg.h>

#include <engine/engine.h>

#ifdef DEBUG

#include <engine/version.h>
#include <engine/map.h>
#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/tlist.h>
#include <engine/mapedit/mapview.h>

#include "monitor.h"

struct monitor monitor;		/* Debug monitor */

static void
select_tool(int argc, union evarg *argv)
{
	struct tlist_item *it = argv[1].p;
	struct window *(*win_func)() = it->p1;		/* XXX unsafe */
	struct window *win;

	if ((win = (win_func)()) != NULL)
		window_show(win);
}

void
monitor_init(struct monitor *mon, const char *name)
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
		{ MONITOR_WIDGET_BROWSER, "Widgets", widget_browser_window },
		{ MONITOR_VIEW_PARAMS, "View params", view_params_window }
	};
	const int ntool_ents = sizeof(tool_ents) / sizeof(tool_ents[0]);
	struct tlist *tl_tools;
	int i;

	object_init(mon, "debug-monitor", name, NULL);
	if (object_load_art(mon, "/engine/monitor/monitor", 0) == -1)
		fatal("monitor: %s", error_get());

	mon->toolbar = window_new("monitor-toolbar");
	window_set_caption(mon->toolbar, "Debug monitor");
	window_set_position(mon->toolbar, WINDOW_LOWER_LEFT, 0);

	tl_tools = tlist_new(mon->toolbar, 0);
	tlist_prescale(tl_tools, "XXXXXXXXXXXXXX", ntool_ents);

	event_new(tl_tools, "tlist-changed", select_tool, NULL);
	for (i = 0; i < ntool_ents; i++) {
		tlist_insert_item(tl_tools,
		    SPRITE(mon, tool_ents[i].ind), tool_ents[i].name,
		    tool_ents[i].window_func);
	}
}

#endif	/* DEBUG */
