/*	$Csoft: tool.c,v 1.15 2003/01/26 06:15:21 vedge Exp $	*/

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
#include <engine/view.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/text.h>

#include <engine/mapedit/mapedit.h>

#include "tool.h"

static void
tool_window_close(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	struct tool *tool = argv[1].p;

	widget_set_int(tool->button, "state", 0);

	mapedit.curtool = NULL;
}

void
tool_init(struct tool *tool, char *name, const void *ops)
{
	char *toolname;

	Asprintf(&toolname, "tool-%s", name);
	object_init(&tool->obj, "tool", toolname, NULL, 0, ops);
	free(toolname);

	tool->win = (TOOL_OPS(tool)->tool_window != NULL) ? 
	    TOOL_OPS(tool)->tool_window(tool) : NULL;
	if (tool->win != NULL) {
		event_new(tool->win, "window-close",
		    tool_window_close, "%p", tool);
	}
	tool->type = Strdup(name);
	tool->button = NULL;
}

/* Return the first visible mapview widget. */
struct mapview *
tool_mapview(void)
{
	struct window *win;
	struct region *reg;
	struct widget *wid;

	TAILQ_FOREACH_REVERSE(win, &view->windows, windows, windowq) {
		if ((win->flags & (WINDOW_SHOWN|WINDOW_HIDDEN_BODY)) == 0) {
			continue;
		}
		TAILQ_FOREACH(reg, &win->regionsh, regions) {
			TAILQ_FOREACH(wid, &reg->widgets, widgets) {
				if (!WIDGET_FOCUSED(wid)) {
					continue;
				}
				if (strcmp(wid->type, "mapview") == 0) {
					return ((struct mapview *)wid);
				}
			}
		}
	}
	text_msg("Error", "No map is visible");
	return (NULL);
}

