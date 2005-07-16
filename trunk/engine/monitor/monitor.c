/*	$Csoft: monitor.c,v 1.68 2005/06/11 11:10:38 vedge Exp $	*/

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

#include <engine/map/map.h>
#include <engine/map/mapview.h>

#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/tableview.h>
#include <engine/widget/menu.h>

#include "monitor.h"

static const struct tool_ent {
	char	      *name;
	struct window *(*fn)(void);
} tool_ents[] = {
	{ N_("Refresh rate"), event_fps_window },
#if defined(THREADS) && defined(HAVE_JPEG)
	{ N_("Upload screenshot"), screenshot_window },
#endif
#if defined(THREADS) && defined(HAVE_LIBQNET)
	{ N_("Server mode"), server_window },
#endif
	{ N_("Leak detection"), leak_window },
	{ N_("Running timers"), timeouts_window },
	{ N_("Unicode browser"), uniconv_window },
	{ N_("Display settings"), view_params_window },
	{ N_("Widget browser"), widget_debug_window },
};

static void
selected_tool(int argc, union evarg *argv)
{
	const struct tool_ent *ent = argv[1].p;
	struct window *win;

	if ((win = (ent->fn)()) != NULL)
		window_show(win);
}

void
monitor_menu(struct AGMenuItem *mi)
{
	const int ntool_ents = sizeof(tool_ents) / sizeof(tool_ents[0]);
	int i;

	for (i = 0; i < ntool_ents; i++) {
		menu_action(mi, _(tool_ents[i].name), -1,
		    selected_tool, "%p", &tool_ents[i]);
	}
}

#endif	/* DEBUG */
