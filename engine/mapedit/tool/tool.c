/*	$Csoft: tool.c,v 1.3 2002/07/21 10:58:17 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CubeSoft Communications, nor the names of its
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

#include <sys/types.h>

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include <engine/engine.h>
#include <engine/queue.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>

#include <engine/mapedit/mapedit.h>

#include "tool.h"

void
tool_init(struct tool *tool, char *name, struct mapedit *med,
    const void *toolops)
{
	static int toolid = 0;
	char *toolname;

	toolname = object_name(name, toolid++);
	object_init(&tool->obj, "tool", toolname, NULL, 0, toolops);
	free(toolname);

	tool->flags = 0;
	tool->med = med;
	tool->win = (TOOL_OPS(tool)->tool_window != NULL) ? 
	    TOOL_OPS(tool)->tool_window(tool) : NULL;
	tool->type = strdup(name);
}

struct mapview *
tool_mapview(void)
{
	struct window *win;
	struct region *reg;
	struct widget *wid;

	TAILQ_FOREACH_REVERSE(win, &view->windowsh, windows, windowq) {
		if ((win->flags & WINDOW_SHOWN) == 0) {
			continue;
		}
		TAILQ_FOREACH(reg, &win->regionsh, regions) {
			TAILQ_FOREACH(wid, &reg->widgetsh, widgets) {
				if (!WIDGET_FOCUSED(wid)) {
					continue;
				}
				if (strcmp(wid->type, "mapview") == 0) {
					return ((struct mapview *)wid);
				}
			}
		}
	}

	return (NULL);
}

