/*	$Csoft: tool.c,v 1.29 2003/06/21 06:50:24 vedge Exp $	*/

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

#include "tool.h"

#include <string.h>

/* Handler for window-close events on tool windows. */
static void
tool_window_close(int argc, union evarg *argv)
{
	struct tool *tool = argv[1].p;

	widget_set_int(tool->button, "state", 0);
	mapedit.curtool = NULL;
}

void
tool_init(struct tool *tool, const char *name, const void *ops)
{
	object_init(tool, "tool", name, ops);

	if (gfx_fetch(tool, "/engine/mapedit/tool/tool") == -1)
		fatal("%s", error_get());

	tool->win = (TOOL_OPS(tool)->window != NULL) ? 
	    TOOL_OPS(tool)->window(tool) : NULL;
	if (tool->win != NULL) {
		event_new(tool->win, "window-close", tool_window_close, "%p",
		    tool);
	}
	strlcpy(tool->type, name, sizeof(tool->type));
	tool->button = NULL;
	tool->cursor = NULL;
	tool->icon = NULL;
	SLIST_INIT(&tool->bindings);
}

void
tool_destroy(void *p)
{
	struct tool *tool = p;
	struct tool_binding *binding, *nbinding;

	for (binding = SLIST_FIRST(&tool->bindings);
	     binding != SLIST_END(&tool->bindings);
	     binding = nbinding) {
		nbinding = SLIST_NEXT(binding, bindings);
		free(binding);
	}
}

/* Return the first focused mapview(3) widget found. */
static struct mapview *
find_mapview(struct widget *pwid)
{
	struct widget *cwid;
	
	if (strcmp(pwid->type, "mapview") == 0)
		return ((struct mapview *)pwid);

	OBJECT_FOREACH_CHILD(cwid, pwid, widget) {
		struct mapview *mv;

		if (!widget_holds_focus(cwid))
			continue;
		if ((mv = find_mapview(cwid)) != NULL)
			return (mv);
	}
	return (NULL);
}

/* Return the first visible mapview widget. */
struct mapview *
tool_mapview(void)
{
	struct window *win;
	struct mapview *mv;

	TAILQ_FOREACH_REVERSE(win, &view->windows, windows, windowq) {
		if (!win->visible)
			continue;
		if ((mv = find_mapview(WIDGET(win))) != NULL)
			return (mv);
	}
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

