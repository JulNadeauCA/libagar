/*	$Csoft: toolbar.c,v 1.11 2005/09/27 00:25:24 vedge Exp $	*/

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
#include <engine/view.h>

#include "toolbar.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/separator.h>

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
AG_ToolbarNew(void *parent, enum ag_toolbar_type type, int nrows, int flags)
{
	AG_Toolbar *tbar;

	tbar = Malloc(sizeof(AG_Toolbar), M_OBJECT);
	AG_ToolbarInit(tbar, type, nrows, flags);
	AG_ObjectAttach(parent, tbar);
	return (tbar);
}

void
AG_ToolbarInit(AG_Toolbar *tbar, enum ag_toolbar_type type, int nrows, int flags)
{
	int i;
	
	switch (type) {
	case AG_TOOLBAR_HORIZ:
		AG_BoxInit(&tbar->box, AG_BOX_VERT, AG_BOX_WFILL);
		break;
	case AG_TOOLBAR_VERT:
		AG_BoxInit(&tbar->box, AG_BOX_HORIZ, AG_BOX_HFILL);
		break;
	}
	AG_BoxSetPadding(&tbar->box, 1);
	AG_BoxSetSpacing(&tbar->box, 1);
	AG_ObjectSetOps(tbar, &agToolbarOps);

	tbar->type = type;
	tbar->nrows = 0;
	for (i = 0; i < nrows && i < AG_TOOLBAR_MAX_ROWS; i++) {
		int bflags = 0;

		if (flags & AG_TOOLBAR_HOMOGENOUS) {
			bflags = AG_BOX_HOMOGENOUS;
		}
		switch (type) {
		case AG_TOOLBAR_HORIZ:
			tbar->rows[i] = AG_BoxNew(&tbar->box, AG_BOX_HORIZ,
			    AG_BOX_WFILL|bflags);
			break;
		case AG_TOOLBAR_VERT:
			tbar->rows[i] = AG_BoxNew(&tbar->box, AG_BOX_VERT, 
			    AG_BOX_HFILL|bflags);
			break;
		}
		AG_BoxSetPadding(tbar->rows[i], 1);
		AG_BoxSetSpacing(tbar->rows[i], 1);
		tbar->nrows++;
	}
}

AG_Button *
AG_ToolbarAddButton(AG_Toolbar *tbar, int row, SDL_Surface *icon,
    int sticky, int def, void (*handler)(int, union evarg *),
    const char *fmt, ...)
{
	AG_Button *bu;
	AG_Event *ev;
	va_list ap;

#ifdef DEBUG
	if (row < 0 || row > tbar->nrows)
		fatal("no such row %d", row);
#endif
	bu = AG_ButtonNew(tbar->rows[row], NULL);
	AG_ButtonSetSurface(bu, icon);
	AG_ButtonSetFocusable(bu, 0);
	AG_ButtonSetSticky(bu, sticky);
	AG_WidgetSetBool(bu, "state", def);
	
	ev = AG_SetEvent(bu, "button-pushed", handler, NULL);
	if (fmt != NULL) {
		va_start(ap, fmt);
		for (; *fmt != '\0'; fmt++) {
			AG_EVENT_PUSH_ARG(ap, *fmt, ev);
		}
		va_end(ap);
	}
	return (bu);
}

void
AG_ToolbarAddSeparator(AG_Toolbar *tbar, int nrow)
{
	AG_SeparatorNew(tbar->rows[nrow], tbar->type == AG_TOOLBAR_HORIZ ?
	    AG_SEPARATOR_VERT : AG_SEPARATOR_HORIZ);
}

void
AG_ToolbarSelectUnique(AG_Toolbar *tbar, AG_Button *ubu)
{
	AG_WidgetBinding *stateb;
	AG_Button *bu;
	int i;

	for (i = 0; i < tbar->nrows; i++) {
		AGOBJECT_FOREACH_CHILD(bu, tbar->rows[i], ag_button) {
			int *state;

			if (bu == ubu) {
				continue;
			}
			stateb = AG_WidgetGetBinding(bu, "state", &state);
			*state = 0;
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
