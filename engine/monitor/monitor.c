/*	$Csoft: monitor.c,v 1.72 2005/09/18 04:08:22 vedge Exp $	*/

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
#include <config/network.h>

#ifdef DEBUG

#include <engine/map/map.h>
#include <engine/map/mapview.h>

#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/tableview.h>
#include <engine/widget/menu.h>

#include "monitor.h"

static AG_Window *
show_fps(void)
{
	extern AG_Window *agFPSWindow;

	AG_WindowShow(agFPSWindow);
	return (agFPSWindow);
}

static const struct ag_maptool_ent {
	char	      *name;
	AG_Window *(*fn)(void);
} tool_ents[] = {
	{ N_("Refresh rate"), show_fps },
#if defined(THREADS) && defined(HAVE_JPEG)
	{ N_("Upload screenshot"), AG_DebugScreenshot },
#endif
#if defined(THREADS) && defined(NETWORK)
	{ N_("Debug server"), AG_DebugServerWindow },
#endif
	{ N_("Leak detection"), AG_DebugLeakDetector },
	{ N_("Running timers"), AG_DebugTimeoutList },
	{ N_("Unicode browser"), AG_DebugUnicodeBrowser },
	{ N_("Display settings"), AG_DebugViewSettings },
	{ N_("Widget browser"), AG_DebugWidgetBrowser },
};

static void
selected_tool(int argc, union evarg *argv)
{
	const struct ag_maptool_ent *ent = argv[1].p;
	AG_Window *win;

	if ((win = (ent->fn)()) != NULL)
		AG_WindowShow(win);
}

void
AG_MonitorMenu(AG_MenuItem *mi)
{
	extern int agObjMgrHexDiff;
	const int ntool_ents = sizeof(tool_ents) / sizeof(tool_ents[0]);
	int i;

	for (i = 0; i < ntool_ents; i++) {
		AG_MenuAction(mi, _(tool_ents[i].name), -1,
		    selected_tool, "%p", &tool_ents[i]);
	}
	AG_MenuSeparator(mi);
	AG_MenuIntBool(mi, _("Datafile hex diffs"), OBJ_ICON,
	    &agObjMgrHexDiff, 0);
}

#endif	/* DEBUG */
