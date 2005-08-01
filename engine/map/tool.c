/*	$Csoft: tool.c,v 1.8 2005/07/31 03:16:46 vedge Exp $	*/

/*
 * Copyright (c) 2004, 2005 CubeSoft Communications, Inc.
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

#ifdef MAP

#include <engine/view.h>

#include <engine/map/map.h>
#include <engine/map/mapview.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>

#include "tool.h"

#include <stdarg.h>

void
tool_init(struct tool *t)
{
	t->nstatus = 0;
	t->win = NULL;
	t->pane = NULL;
	t->trigger = NULL;
	SLIST_INIT(&t->kbindings);
	SLIST_INIT(&t->mbindings);

	if (t->ops->init != NULL)
		t->ops->init(t);
}

void
tool_destroy(struct tool *tool)
{
	struct tool_kbinding *kbinding, *nkbinding;
	struct tool_mbinding *mbinding, *nmbinding;
	int i;
	
	if (tool->win != NULL)
		view_detach(tool->win);
	
	if (tool->pane != NULL) {
		struct window *pwin;
		struct widget *wt;

		OBJECT_FOREACH_CHILD(wt, tool->pane, widget) {
			object_detach(wt);
			object_destroy(wt);
			Free(wt, M_OBJECT);
		}
		if ((pwin = widget_parent_window(tool->pane)) != NULL)
			WINDOW_UPDATE(pwin);
	}
	
	for (i = 0; i < tool->nstatus; i++)
		Free(tool->status[i], 0);

	for (kbinding = SLIST_FIRST(&tool->kbindings);
	     kbinding != SLIST_END(&tool->kbindings);
	     kbinding = nkbinding) {
		nkbinding = SLIST_NEXT(kbinding, kbindings);
		Free(kbinding, M_MAPEDIT);
	}
	for (mbinding = SLIST_FIRST(&tool->mbindings);
	     mbinding != SLIST_END(&tool->mbindings);
	     mbinding = nmbinding) {
		nmbinding = SLIST_NEXT(mbinding, mbindings);
		Free(mbinding, M_MAPEDIT);
	}
	if (tool->ops->destroy != NULL)
		tool->ops->destroy(tool);
}

static void
close_tool_window(int argc, union evarg *argv)
{
	struct tool *tool = argv[1].p;

	mapview_select_tool(tool->mv, NULL, NULL);
}

struct window *
tool_window(void *p, const char *name)
{
	struct tool *tool = p;
	struct window *win;

	win = tool->win = window_new(0, NULL);
	window_set_caption(win, _(tool->ops->desc));
	window_set_position(win, WINDOW_MIDDLE_LEFT, 0);
	event_new(win, "window-close", close_tool_window, "%p", tool);
	return (win);
}

void
tool_bind_mousebutton(void *p, int button,
    int (*func)(struct tool *, int, int, int, int, void *), void *arg)
{
	struct tool *tool = p;
	struct tool_mbinding *mb;
	
	mb = Malloc(sizeof(struct tool_mbinding), M_MAPEDIT);
	mb->button = button;
	mb->func = func;
	mb->edit = 0;
	mb->arg = arg;
	SLIST_INSERT_HEAD(&tool->mbindings, mb, mbindings);
}

void
tool_bind_key(void *p, SDLMod keymod, SDLKey keysym,
    int (*func)(struct tool *, SDLKey, int, void *), void *arg)
{
	struct tool *tool = p;
	struct tool_kbinding *kb;

	kb = Malloc(sizeof(struct tool_kbinding), M_MAPEDIT);
	kb->key = keysym;
	kb->mod = keymod;
	kb->func = func;
	kb->edit = 0;
	kb->arg = arg;
	SLIST_INSERT_HEAD(&tool->kbindings, kb, kbindings);
}

void
tool_unbind_key(void *p, SDLMod keymod, SDLKey keysym)
{
	struct tool *tool = p;
	struct tool_kbinding *kb;

	SLIST_FOREACH(kb, &tool->kbindings, kbindings) {
		if (kb->mod == keymod &&
		    kb->key == keysym)
			break;
	}
	if (kb != NULL) {
		SLIST_REMOVE(&tool->kbindings, kb, tool_kbinding, kbindings);
		Free(kb, M_MAPEDIT);
	}
}

void
tool_update_status(void *p)
{
	struct tool *t = p;

	if (t->nstatus > 0 && t->mv->status != NULL) {
		widget_replace_surface(t->mv->status, t->mv->status->surface,
		    text_render(NULL, -1, COLOR(TEXT_COLOR),
		    t->status[t->nstatus-1]));
	}
}

void
tool_push_status(void *p, const char *fmt, ...)
{
	struct tool *t = p;
	va_list ap;

	if (t->nstatus+1 >= TOOL_STATUS_MAX)
		return;

	va_start(ap, fmt);
	Vasprintf(&t->status[t->nstatus++], fmt, ap);
	va_end(ap);
	tool_update_status(t);
}

void
tool_set_status(void *p, const char *fmt, ...)
{
	struct tool *t = p;
	va_list ap;

	va_start(ap, fmt);
	Vasprintf(&t->status[t->nstatus-1], fmt, ap);
	va_end(ap);

	tool_update_status(t);
}

void
tool_pop_status(void *p)
{
	struct tool *t = p;

	if (t->nstatus == 1)
		return;

	Free(t->status[--t->nstatus], 0);
	tool_update_status(t);
}

#endif /* MAP */
