/*
 * Copyright (c) 2004-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>

#include <agar/gui/gui.h>
#include <agar/gui/window.h>
#include <agar/gui/icons.h>
#include <agar/gui/primitive.h>
#include <agar/gui/label.h>

#include <agar/map/map.h>

#include <stdarg.h>

void
MAP_ToolInit(MAP_Tool *t)
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
MAP_ToolDestroy(MAP_Tool *tool)
{
	MAP_ToolKeyBinding *kbinding, *nkbinding;
	MAP_ToolMouseBinding *mbinding, *nmbinding;
	int i;
	
	if (tool->win != NULL)
		AG_ObjectDetach(tool->win);
	
	if (tool->pane != NULL) {
		AG_ObjectFreeChildren(tool->pane);
		AG_WidgetUpdate(tool->pane);
	}
	
	for (i = 0; i < tool->nstatus; i++)
		Free(tool->status[i]);

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
close_MAP_ToolWindow(AG_Event *_Nonnull event)
{
	MAP_Tool *tool = AG_PTR(1);

	MAP_ViewSelectTool(tool->mv, NULL, NULL);
}

AG_Window *
MAP_ToolWindow(void *p, const char *name)
{
	MAP_Tool *tool = p;
	AG_Window *win;

	if ((win = tool->win = AG_WindowNew(0)) == NULL) {
		return (NULL);
	}
	tool->win = win;
	AG_WindowSetCaptionS(win, _(tool->ops->desc));
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);
	AG_SetEvent(win, "window-close", close_MAP_ToolWindow, "%p", tool);
	return (win);
}

void
MAP_ToolBindMouseButton(void *p, int button,
    int (*func)(MAP_Tool *, int, int, int, int, void *), void *arg)
{
	MAP_Tool *tool = p;
	MAP_ToolMouseBinding *mb;
	
	mb = Malloc(sizeof(MAP_ToolMouseBinding));
	mb->button = button;
	mb->func = func;
	mb->edit = 0;
	mb->arg = arg;
	SLIST_INSERT_HEAD(&tool->mbindings, mb, mbindings);
}

void
MAP_ToolBindKey(void *p, AG_KeyMod keymod, AG_KeySym keysym,
    int (*func)(MAP_Tool *, AG_KeySym, int, void *), void *arg)
{
	MAP_Tool *tool = p;
	MAP_ToolKeyBinding *kb;

	kb = Malloc(sizeof(MAP_ToolKeyBinding));
	kb->key = keysym;
	kb->mod = keymod;
	kb->func = func;
	kb->edit = 0;
	kb->arg = arg;
	SLIST_INSERT_HEAD(&tool->kbindings, kb, kbindings);
}

void
MAP_ToolUnbindKey(void *p, AG_KeyMod keymod, AG_KeySym keysym)
{
	MAP_Tool *tool = p;
	MAP_ToolKeyBinding *kb;

	SLIST_FOREACH(kb, &tool->kbindings, kbindings) {
		if (kb->mod == keymod &&
		    kb->key == keysym)
			break;
	}
	if (kb != NULL) {
		SLIST_REMOVE(&tool->kbindings, kb, map_tool_keybinding,
		    kbindings);
		Free(kb);
	}
}

void
MAP_ToolUpdateStatus(void *p)
{
	MAP_Tool *t = p;

	if (t->nstatus > 0 && t->mv->status != NULL) {
		AG_LabelTextS(t->mv->status, t->status[t->nstatus-1]);
	}
}

void
MAP_ToolPushStatus(void *p, const char *fmt, ...)
{
	MAP_Tool *t = p;
	va_list ap;

	if (t->mv->status == NULL || t->nstatus+1 >= AG_MAPTOOL_STATUS_MAX)
		return;

	va_start(ap, fmt);
	Vasprintf(&t->status[t->nstatus++], fmt, ap);
	va_end(ap);
	MAP_ToolUpdateStatus(t);
}

void
MAP_ToolSetStatus(void *p, const char *fmt, ...)
{
	MAP_Tool *t = p;
	va_list ap;

	if (t->mv->status == NULL)
		return;

	va_start(ap, fmt);
	Vasprintf(&t->status[t->nstatus-1], fmt, ap);
	va_end(ap);

	MAP_ToolUpdateStatus(t);
}

void
MAP_ToolPopStatus(void *p)
{
	MAP_Tool *t = p;

	if (t->mv->status == NULL || t->nstatus == 1)
		return;

	Free(t->status[--t->nstatus]);
	MAP_ToolUpdateStatus(t);
}
