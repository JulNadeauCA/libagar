/*	$Csoft: tool.c,v 1.26 2003/05/07 13:01:58 vedge Exp $	*/

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

#include <engine/compat/snprintf.h>

#include <engine/engine.h>
#include <engine/view.h>

#include "tool.h"

#include <string.h>

static void
tool_window_close(int argc, union evarg *argv)
{
	struct tool *tool = argv[1].p;

	widget_set_int(tool->button, "state", 0);

	mapedit.curtool = NULL;
}

void
tool_init(struct tool *tool, char *name, const void *ops)
{
	char toolname[OBJECT_NAME_MAX];

	snprintf(toolname, sizeof(toolname), "tool-%s", name);
	object_init(&tool->obj, "tool", toolname, ops);
	object_load_art(tool, "tool", 0);

	tool->win = (TOOL_OPS(tool)->window != NULL) ? 
	    TOOL_OPS(tool)->window(tool) : NULL;
	if (tool->win != NULL) {
		event_new(tool->win, "window-close",
		    tool_window_close, "%p", tool);
	}
	tool->type = Strdup(name);
	tool->button = NULL;
	tool->cursor = NULL;
	SLIST_INIT(&tool->bindings);
}

void
tool_destroy(void *p)
{
	struct tool *tool = p;
	struct tool_binding *binding, *nbinding;

	free(tool->type);
	
	for (binding = SLIST_FIRST(&tool->bindings);
	     binding != SLIST_END(&tool->bindings);
	     binding = nbinding) {
		nbinding = SLIST_NEXT(binding, bindings);
		free(binding);
	}
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
				if (!WIDGET_FOCUSED(wid))
					continue;
				if (strcmp(wid->type, "mapview") == 0)
					return ((struct mapview *)wid);
			}
		}
	}
	error_set("no map is visible");
	return (NULL);
}

void
tool_bind_key(void *p, SDLMod keymod, SDLKey keysym,
    void (*func)(void *, struct mapview *), int edit)
{
	struct tool *tool = p;
	struct tool_binding *binding;

	binding = Malloc(sizeof(struct tool_binding));
	binding->key = keysym;
	binding->mod = keymod;
	binding->func = func;
	binding->edit = edit;
	SLIST_INSERT_HEAD(&tool->bindings, binding, bindings);
}
