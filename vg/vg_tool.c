/*	$Csoft: tool.c,v 1.12 2005/10/04 17:34:51 vedge Exp $	*/

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

#include <core/core.h>

#include <gui/view.h>
#include <gui/window.h>

#include "vg.h"
#include "vg_view.h"

#include <stdarg.h>

void
VG_ToolInit(VG_Tool *t)
{
	t->win = NULL;
	t->pane = NULL;
	t->trigger = NULL;
	SLIST_INIT(&t->kbindings);
	SLIST_INIT(&t->mbindings);

	if (t->ops->init != NULL)
		t->ops->init(t);
}

void
VG_ToolDestroy(VG_Tool *tool)
{
	VG_ToolKeyBinding *kbinding, *nkbinding;
	VG_ToolMouseBinding *mbinding, *nmbinding;
	
	if (tool->win != NULL)
		AG_ViewDetach(tool->win);
	
	if (tool->pane != NULL) {
		AG_Window *pwin;
		AG_Widget *wt;

		OBJECT_FOREACH_CHILD(wt, tool->pane, ag_widget) {
			AG_ObjectDetach(wt);
			AG_ObjectDestroy(wt);
			Free(wt);
		}
		if ((pwin = AG_WidgetParentWindow(tool->pane)) != NULL)
			AG_WindowUpdate(pwin);
	}
	
	for (kbinding = SLIST_FIRST(&tool->kbindings);
	     kbinding != SLIST_END(&tool->kbindings);
	     kbinding = nkbinding) {
		nkbinding = SLIST_NEXT(kbinding, kbindings);
		Free(kbinding);
	}
	for (mbinding = SLIST_FIRST(&tool->mbindings);
	     mbinding != SLIST_END(&tool->mbindings);
	     mbinding = nmbinding) {
		nmbinding = SLIST_NEXT(mbinding, mbindings);
		Free(mbinding);
	}
	if (tool->ops->destroy != NULL)
		tool->ops->destroy(tool);
}

static void
VG_ToolWindowClosed(AG_Event *event)
{
	VG_Tool *tool = AG_PTR(1);

	VG_ViewSelectTool(tool->vgv, NULL, NULL);
}

AG_Window *
VG_ToolWindow(void *p, const char *name)
{
	VG_Tool *tool = p;
	AG_Window *win;

	win = tool->win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _(tool->ops->desc));
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);
	AG_SetEvent(win, "window-close", VG_ToolWindowClosed, "%p", tool);
	return (win);
}

void
VG_ToolBindMouseButton(void *p, int button,
    int (*func)(VG_Tool *, int, int, float, float, void *), void *arg)
{
	VG_Tool *tool = p;
	VG_ToolMouseBinding *mb;
	
	mb = Malloc(sizeof(VG_ToolMouseBinding));
	mb->button = button;
	mb->func = func;
	mb->edit = 0;
	mb->arg = arg;
	SLIST_INSERT_HEAD(&tool->mbindings, mb, mbindings);
}

void
VG_ToolBindKey(void *p, SDLMod keymod, SDLKey keysym,
    int (*func)(VG_Tool *, SDLKey, int, void *), void *arg)
{
	VG_Tool *tool = p;
	VG_ToolKeyBinding *kb;

	kb = Malloc(sizeof(VG_ToolKeyBinding));
	kb->key = keysym;
	kb->mod = keymod;
	kb->func = func;
	kb->edit = 0;
	kb->arg = arg;
	SLIST_INSERT_HEAD(&tool->kbindings, kb, kbindings);
}

void
VG_ToolUnbindKey(void *p, SDLMod keymod, SDLKey keysym)
{
	VG_Tool *tool = p;
	VG_ToolKeyBinding *kb;

	SLIST_FOREACH(kb, &tool->kbindings, kbindings) {
		if (kb->mod == keymod &&
		    kb->key == keysym)
			break;
	}
	if (kb == NULL) {
		return;
	}
	SLIST_REMOVE(&tool->kbindings, kb, vg_tool_keybinding, kbindings);
	Free(kb);
}

