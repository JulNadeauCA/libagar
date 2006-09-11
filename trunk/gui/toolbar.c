/*	$Csoft: toolbar.c,v 1.12 2005/10/01 14:15:39 vedge Exp $	*/

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
#include <core/view.h>

#include "toolbar.h"

#include <gui/window.h>
#include <gui/primitive.h>
#include <gui/separator.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>

static AG_WidgetOps agToolbarOps = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		AG_BoxDestroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	NULL,
	AG_ToolbarScale
};

AG_Toolbar *
AG_ToolbarNew(void *parent, enum ag_toolbar_type type, int nrows, Uint flags)
{
	AG_Toolbar *bar;

	bar = Malloc(sizeof(AG_Toolbar), M_OBJECT);
	AG_ToolbarInit(bar, type, nrows, flags);
	AG_ObjectAttach(parent, bar);
	return (bar);
}

void
AG_ToolbarInit(AG_Toolbar *bar, enum ag_toolbar_type type, int nrows,
    Uint flags)
{
	int i;
	
	switch (type) {
	case AG_TOOLBAR_HORIZ:
		AG_BoxInit(&bar->box, AG_BOX_VERT, AG_BOX_HFILL);
		break;
	case AG_TOOLBAR_VERT:
		AG_BoxInit(&bar->box, AG_BOX_HORIZ, AG_BOX_VFILL);
		break;
	}
	AG_BoxSetPadding(&bar->box, 1);
	AG_BoxSetSpacing(&bar->box, 1);
	AG_ObjectSetOps(bar, &agToolbarOps);

	bar->flags = flags;
	bar->type = type;
	bar->nrows = 0;
	bar->curRow = 0;
	for (i = 0; i < nrows && i < AG_TOOLBAR_MAX_ROWS; i++) {
		int bflags = 0;

		if (flags & AG_TOOLBAR_HOMOGENOUS) {
			bflags = AG_BOX_HOMOGENOUS;
		}
		switch (type) {
		case AG_TOOLBAR_HORIZ:
			bar->rows[i] = AG_BoxNew(&bar->box, AG_BOX_HORIZ,
			    AG_BOX_HFILL|bflags);
			break;
		case AG_TOOLBAR_VERT:
			bar->rows[i] = AG_BoxNew(&bar->box, AG_BOX_VERT, 
			    AG_BOX_VFILL|bflags);
			break;
		}
		AG_BoxSetPadding(bar->rows[i], 1);
		AG_BoxSetSpacing(bar->rows[i], 1);
		bar->nrows++;
	}
}

static void
StickyUpdate(AG_Event *event)
{
	AG_Button *b = AG_SELF();
	AG_Toolbar *bar = AG_PTR(1);

	AG_ToolbarSelectUnique(bar, b);
}

void
AG_ToolbarRow(AG_Toolbar *bar, int row)
{
#ifdef DEBUG
	if (row < 0 || row >= bar->nrows)
		fatal("no such row %d", row);
#endif
	bar->curRow = row;
}

AG_Button *
AG_ToolbarButtonIcon(AG_Toolbar *bar, SDL_Surface *icon, int def,
    void (*handler)(AG_Event *), const char *fmt, ...)
{
	AG_Button *bu;
	AG_Event *ev;

	bu = AG_ButtonNew(bar->rows[bar->curRow], 0, NULL);
	AG_ButtonSetSurface(bu, icon);
	AG_ButtonSetFocusable(bu, 0);
	AG_ButtonSetSticky(bu, bar->flags & AG_TOOLBAR_STICKY);
	AG_WidgetSetBool(bu, "state", def);
	
	ev = AG_SetEvent(bu, "button-pushed", handler, NULL);
	AG_EVENT_GET_ARGS(ev, fmt);
	
	if (bar->flags & AG_TOOLBAR_STICKY) {
		AG_AddEvent(bu, "button-pushed", StickyUpdate, "%p", bar);
	}
	return (bu);
}

AG_Button *
AG_ToolbarButton(AG_Toolbar *bar, const char *text, int def,
    void (*handler)(AG_Event *), const char *fmt, ...)
{
	AG_Button *bu;
	AG_Event *ev;

	bu = AG_ButtonNew(bar->rows[bar->curRow], 0, text);
	AG_ButtonSetFocusable(bu, 0);
	AG_ButtonSetSticky(bu, bar->flags & AG_TOOLBAR_STICKY);
	AG_WidgetSetBool(bu, "state", def);
	
	ev = AG_SetEvent(bu, "button-pushed", handler, NULL);
	AG_EVENT_GET_ARGS(ev, fmt);

	if (bar->flags & AG_TOOLBAR_STICKY) {
		AG_AddEvent(bu, "button-pushed", StickyUpdate, "%p", bar);
	}
	return (bu);
}

void
AG_ToolbarSeparator(AG_Toolbar *bar)
{
	AG_SeparatorNew(bar->rows[bar->curRow], bar->type == AG_TOOLBAR_HORIZ ?
	    AG_SEPARATOR_VERT : AG_SEPARATOR_HORIZ);
}

void
AG_ToolbarSelectUnique(AG_Toolbar *bar, AG_Button *ubu)
{
	AG_WidgetBinding *stateb;
	AG_Button *bu;
	int i;

	for (i = 0; i < bar->nrows; i++) {
		AGOBJECT_FOREACH_CHILD(bu, bar->rows[i], ag_button) {
			int *state;

			stateb = AG_WidgetGetBinding(bu, "state", &state);
			*state = (bu == ubu);
			AG_WidgetBindingChanged(stateb);
			AG_WidgetUnlockBinding(stateb);
		}
	}
}

void
AG_ToolbarScale(void *p, int w, int h)
{
	AG_Box *bo = p;
	AG_Widget *wid;
	int x = bo->padding;
	int y = bo->padding;

	if (w == -1 && h == -1) {
		int maxw = 0, maxh = 0;
		int dw, dh;

		AGWIDGET(bo)->w = bo->padding*2 - bo->spacing;
		AGWIDGET(bo)->h = bo->padding*2;

		/* Reserve enough space to hold widgets and spacing/padding. */
		AGOBJECT_FOREACH_CHILD(wid, bo, ag_widget) {
			AGWIDGET_OPS(wid)->scale(wid, -1, -1);
			if (wid->w > maxw) maxw = wid->w;
			if (wid->h > maxh) maxh = wid->h;

			if ((dh = maxh + bo->padding*2) > AGWIDGET(bo)->h) {
				AGWIDGET(bo)->h = dh;
			}
			AGWIDGET(bo)->w += wid->w + bo->spacing;
		}
		AGWIDGET(bo)->w -= bo->spacing;
		return;
	}

	AGOBJECT_FOREACH_CHILD(wid, bo, ag_widget) {
		wid->x = x;
		wid->y = y;

		x += wid->w + bo->spacing;
		AGWIDGET_OPS(wid)->scale(wid, wid->w, wid->h);
	}
}
