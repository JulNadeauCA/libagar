/*	$Csoft: monitor.c,v 1.60 2005/01/27 05:46:24 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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
#include <config/have_jpeg.h>
#include <config/have_libqnet.h>

#ifdef DEBUG

#include <engine/map.h>
#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/tableview.h>
#include <engine/mapedit/mapview.h>

#include "monitor.h"

static const struct tool_ent {
	char		    *name;
	struct window	*(*window_func)(void);
} tool_ents[] = {
	{ N_("Refresh rate"), event_fps_window },
#if defined(THREADS) && defined(HAVE_JPEG)
	{ N_("Screenshot"), screenshot_window },
#endif
#if defined(THREADS) && defined(HAVE_LIBQNET)
	{ N_("Server"), server_window },
#endif
	{ N_("Leak detection"), leak_window },
	{ N_("Resident graphics"), gfx_debug_window },
	{ N_("Running timers"), timeouts_window },
	{ N_("Unicode conversion"), uniconv_window },
	{ N_("Viewport"), view_params_window },
	{ N_("Widgets"), widget_debug_window },
};

static void
selected_tool(int argc, union evarg *argv)
{
	struct tableview_row *row = argv[1].p;
	struct window *(*win_func)(void) =
	    tool_ents[tableview_row_getID(row)].window_func;
	struct window *win;

	if ((win = (win_func)()) != NULL)
		window_show(win);
}

void
monitor_init(void)
{
	const int ntool_ents = sizeof(tool_ents) / sizeof(tool_ents[0]);
	struct tableview *tv;
	struct window *win;
	int i;

	if ((win = window_new(WINDOW_NO_RESIZE, "monitor-toolbar")) == NULL) {
			return;
	}
	window_set_caption(win, _("Debug monitor"));
	window_set_position(win, WINDOW_LOWER_LEFT, 0);

	tv = tableview_new(win, TABLEVIEW_NOHEADER, NULL, NULL);
	tableview_prescale(tv, "ZZZZZZZZZZZZZZZZZZZZZZZZ", ntool_ents);
	tableview_col_add(tv, TABLEVIEW_COL_FILL, 0, NULL, NULL);
	
	event_new(tv, "tableview-dblclick", selected_tool, NULL);

	for (i = 0; i < ntool_ents; i++)
	    tableview_row_add(tv, 0, NULL, i, 0, tool_ents[i].name);

	window_show(win);
}

#endif	/* DEBUG */
