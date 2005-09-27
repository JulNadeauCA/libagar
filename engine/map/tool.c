/*	$Csoft: tool.c,v 1.10 2005/08/22 02:14:59 vedge Exp $	*/

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
AG_MaptoolInit(AG_Maptool *t)
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
AG_MaptoolDestroy(AG_Maptool *tool)
{
	AG_MaptoolKeyBinding *kbinding, *nkbinding;
	AG_MaptoolMouseBinding *mbinding, *nmbinding;
	int i;
	
	if (tool->win != NULL)
		AG_ViewDetach(tool->win);
	
	if (tool->pane != NULL) {
		AG_Window *pwin;
		AG_Widget *wt;

		AGOBJECT_FOREACH_CHILD(wt, tool->pane, ag_widget) {
			AG_ObjectDetach(wt);
			AG_ObjectDestroy(wt);
			Free(wt, M_OBJECT);
		}
		if ((pwin = AG_WidgetParentWindow(tool->pane)) != NULL)
			AG_WINDOW_UPDATE(pwin);
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
close_AG_MaptoolWindow(int argc, union evarg *argv)
{
	AG_Maptool *tool = argv[1].p;

	AG_MapviewSelectTool(tool->mv, NULL, NULL);
}

AG_Window *
AG_MaptoolWindow(void *p, const char *name)
{
	AG_Maptool *tool = p;
	AG_Window *win;

	win = tool->win = AG_WindowNew(0, NULL);
	AG_WindowSetCaption(win, _(tool->ops->desc));
	AG_WindowSetPosition(win, AG_WINDOW_MIDDLE_LEFT, 0);
	AG_SetEvent(win, "window-close", close_AG_MaptoolWindow, "%p", tool);
	return (win);
}

void
AG_MaptoolBindMouseButton(void *p, int button,
    int (*func)(AG_Maptool *, int, int, int, int, void *), void *arg)
{
	AG_Maptool *tool = p;
	AG_MaptoolMouseBinding *mb;
	
	mb = Malloc(sizeof(AG_MaptoolMouseBinding), M_MAPEDIT);
	mb->button = button;
	mb->func = func;
	mb->edit = 0;
	mb->arg = arg;
	SLIST_INSERT_HEAD(&tool->mbindings, mb, mbindings);
}

void
AG_MaptoolBindKey(void *p, SDLMod keymod, SDLKey keysym,
    int (*func)(AG_Maptool *, SDLKey, int, void *), void *arg)
{
	AG_Maptool *tool = p;
	AG_MaptoolKeyBinding *kb;

	kb = Malloc(sizeof(AG_MaptoolKeyBinding), M_MAPEDIT);
	kb->key = keysym;
	kb->mod = keymod;
	kb->func = func;
	kb->edit = 0;
	kb->arg = arg;
	SLIST_INSERT_HEAD(&tool->kbindings, kb, kbindings);
}

void
AG_MaptoolUnbindKey(void *p, SDLMod keymod, SDLKey keysym)
{
	AG_Maptool *tool = p;
	AG_MaptoolKeyBinding *kb;

	SLIST_FOREACH(kb, &tool->kbindings, kbindings) {
		if (kb->mod == keymod &&
		    kb->key == keysym)
			break;
	}
	if (kb != NULL) {
		SLIST_REMOVE(&tool->kbindings, kb, ag_maptool_keybinding,
		    kbindings);
		Free(kb, M_MAPEDIT);
	}
}

void
AG_MaptoolUpdateStatus(void *p)
{
	AG_Maptool *t = p;

	if (t->nstatus > 0 && t->mv->status != NULL) {
		AG_WidgetReplaceSurface(t->mv->status, t->mv->status->surface,
		    AG_TextRender(NULL, -1, AG_COLOR(TEXT_COLOR),
		    t->status[t->nstatus-1]));
	}
}

void
AG_MaptoolPushStatus(void *p, const char *fmt, ...)
{
	AG_Maptool *t = p;
	va_list ap;

	if (t->mv->status == NULL || t->nstatus+1 >= AG_MAPTOOL_STATUS_MAX)
		return;

	va_start(ap, fmt);
	Vasprintf(&t->status[t->nstatus++], fmt, ap);
	va_end(ap);
	AG_MaptoolUpdateStatus(t);
}

void
AG_MaptoolSetStatus(void *p, const char *fmt, ...)
{
	AG_Maptool *t = p;
	va_list ap;

	if (t->mv->status == NULL)
		return;

	va_start(ap, fmt);
	Vasprintf(&t->status[t->nstatus-1], fmt, ap);
	va_end(ap);

	AG_MaptoolUpdateStatus(t);
}

void
AG_MaptoolPopStatus(void *p)
{
	AG_Maptool *t = p;

	if (t->mv->status == NULL || t->nstatus == 1)
		return;

	Free(t->status[--t->nstatus], 0);
	AG_MaptoolUpdateStatus(t);
}

#endif /* MAP */
