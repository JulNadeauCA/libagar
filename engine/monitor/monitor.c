/*	$Csoft: monitor.c,v 1.54 2004/05/15 02:55:21 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004 CubeSoft Communications, Inc.
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

#include <engine/map.h>
#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/tlist.h>
#include <engine/mapedit/mapview.h>

#include "monitor.h"

static void
selected_tool(int argc, union evarg *argv)
{
	struct tlist_item *it = argv[1].p;
	struct window *(*win_func)() = it->p1;		/* XXX unsafe */
	struct window *win;

	if ((win = (win_func)()) != NULL)
		window_show(win);
}

void
monitor_init(void)
{
	const struct tool_ent {
		char		*name;
		struct window	*(*window_func)(void);
	} tool_ents[] = {
		{ N_("Refresh rate"), event_fps_window },
#if defined(THREADS) && defined(HAVE_JPEG)
		{ N_("Screenshot"), screenshot_window },
#endif
		{ N_("Leak detection"), leak_window },
		{ N_("Resident graphics"), gfx_debug_window },
		{ N_("Running timers"), timeouts_window },
		{ N_("Unicode conversion"), uniconv_window },
		{ N_("Viewport"), view_params_window },
		{ N_("Widgets"), widget_debug_window },
	};
	const int ntool_ents = sizeof(tool_ents) / sizeof(tool_ents[0]);
	struct tlist *tl_tools;
	struct window *win;
	int i;

	win = window_new("monitor-toolbar");
	window_set_caption(win, _("Debug monitor"));
	window_set_position(win, WINDOW_LOWER_LEFT, 0);

	tl_tools = tlist_new(win, TLIST_STATIC_ICONS);
	tlist_prescale(tl_tools, "XXXXXXXXXXXXXXXXXXXXXXXXXXX", ntool_ents);
	event_new(tl_tools, "tlist-dblclick", selected_tool, NULL);

	for (i = 0; i < ntool_ents; i++) {
		tlist_insert_item(tl_tools, ICON(OBJ_ICON),
		    _(tool_ents[i].name), tool_ents[i].window_func);
	}

	window_show(win);
}

#endif	/* DEBUG */
