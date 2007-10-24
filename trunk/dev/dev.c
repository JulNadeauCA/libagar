/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * Generic dev tool initialization.
 */

#include <config/network.h>
#include <config/threads.h>
#include <config/have_jpeg.h>
#include <config/debug.h>

#include <core/core.h>

#include <gui/window.h>
#include <gui/menu.h>

#include "dev.h"

static const struct dev_tool_ent {
	char *name;
	AG_Window *(*fn)(void);
} devTools[] = {
#if defined(DEBUG)
	{ N_("Performance Graph"),	AG_EventShowPerfGraph },
	{ N_("CPU Information"),	DEV_CPUInfo },
#endif
#if defined(NETWORK) && defined(THREADS) && defined(HAVE_JPEG)
	{ N_("Screenshot Uploader"),	DEV_ScreenshotUploader },
#endif
#if defined(NETWORK) && defined(THREADS)
	{ N_("Debug Server"),		DEV_DebugServer },
#endif
	{ N_("Timer Inspector"),	DEV_TimerInspector },
	{ N_("Unicode Browser"),	DEV_UnicodeBrowser },
	{ N_("Display settings"),	DEV_DisplaySettings },
	{ N_("GUI Debugger"),		DEV_GuiDebugger },
};

static void
SelectTool(AG_Event *event)
{
	const struct dev_tool_ent *ent = AG_PTR(1);
	AG_Window *win;

	if ((win = (ent->fn)()) != NULL)
		AG_WindowShow(win);
}

void
DEV_ToolMenu(AG_MenuItem *mi)
{
	const int devToolCount = sizeof(devTools) / sizeof(devTools[0]);
	int i;

	for (i = 0; i < devToolCount; i++) {
		AG_MenuAction(mi, _(devTools[i].name), NULL,
		    SelectTool, "%p", &devTools[i]);
	}
}

void
DEV_InitSubsystem(Uint flags)
{
	DEV_BrowserInit();
}
