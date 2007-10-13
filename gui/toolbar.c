/*
 * Copyright (c) 2004-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "toolbar.h"

#include "window.h"
#include "primitive.h"
#include "separator.h"

AG_Toolbar *
AG_ToolbarNew(void *parent, enum ag_toolbar_type type, int nRows, Uint flags)
{
	AG_Toolbar *bar;

	bar = Malloc(sizeof(AG_Toolbar), M_OBJECT);
	AG_ToolbarInit(bar, type, nRows, flags);
	AG_ObjectAttach(parent, bar);
	return (bar);
}

void
AG_ToolbarInit(AG_Toolbar *bar, enum ag_toolbar_type type, int nRows,
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
	WIDGET(bar)->flags |= AG_WIDGET_IGNORE_PADDING;

	bar->flags = flags;
	bar->type = type;
	bar->nRows = 0;
	bar->nButtons = 0;
	bar->curRow = 0;
	for (i = 0; i < nRows && i < AG_TOOLBAR_MAX_ROWS; i++) {
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
		bar->nRows++;
	}
}

static void
StickyUpdate(AG_Event *event)
{
	AG_Button *selBtn = AG_SELF();
	AG_Toolbar *bar = AG_PTR(1);
	AG_WidgetBinding *stateb;
	AG_Button *oBtn;
	int i;

	for (i = 0; i < bar->nRows; i++) {
		OBJECT_FOREACH_CHILD(oBtn, bar->rows[i], ag_button) {
			int *state;

			stateb = AG_WidgetGetBinding(oBtn, "state", &state);
			if (bar->flags & AG_TOOLBAR_MULTI_STICKY) {
				*state = !(*state);
			} else {
				*state = (oBtn == selBtn);
			}
			AG_WidgetBindingChanged(stateb);
			AG_WidgetUnlockBinding(stateb);
		}
	}
}

void
AG_ToolbarRow(AG_Toolbar *bar, int row)
{
#ifdef DEBUG
	if (row < 0 || row >= bar->nRows)
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
	AG_ButtonSurface(bu, icon);
	AG_ButtonSetFocusable(bu, 0);
	AG_ButtonSetSticky(bu, bar->flags & AG_TOOLBAR_STICKY);
	AG_WidgetSetBool(bu, "state", def);
	bar->nButtons++;
	
	ev = AG_SetEvent(bu, "button-pushed", handler, NULL);
	AG_EVENT_GET_ARGS(ev, fmt);
	
	if (bar->flags & (AG_TOOLBAR_STICKY|AG_TOOLBAR_MULTI_STICKY)) {
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
	bar->nButtons++;
	
	ev = AG_SetEvent(bu, "button-pushed", handler, NULL);
	AG_EVENT_GET_ARGS(ev, fmt);

	if (bar->flags & (AG_TOOLBAR_STICKY|AG_TOOLBAR_MULTI_STICKY)) {
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
AG_ToolbarSelect(AG_Toolbar *bar, AG_Button *bSel)
{
	AG_WidgetBinding *stateb;
	int *state;

	stateb = AG_WidgetGetBinding(bSel, "state", &state);
	*state = 1;
	AG_WidgetBindingChanged(stateb);
	AG_WidgetUnlockBinding(stateb);
}

void
AG_ToolbarDeselect(AG_Toolbar *bar, AG_Button *bSel)
{
	AG_WidgetBinding *stateb;
	int *state;

	stateb = AG_WidgetGetBinding(bSel, "state", &state);
	*state = 0;
	AG_WidgetBindingChanged(stateb);
	AG_WidgetUnlockBinding(stateb);
}

void
AG_ToolbarSelectOnly(AG_Toolbar *bar, AG_Button *bSel)
{
	AG_WidgetBinding *stateb;
	AG_Button *b;
	int i, *state;

	for (i = 0; i < bar->nRows; i++) {
		OBJECT_FOREACH_CHILD(b, bar->rows[i], ag_button) {
			stateb = AG_WidgetGetBinding(b, "state", &state);
			*state = (b == bSel);
			AG_WidgetBindingChanged(stateb);
			AG_WidgetUnlockBinding(stateb);
		}
	}
}

void
AG_ToolbarSelectAll(AG_Toolbar *bar)
{
	AG_WidgetBinding *stateb;
	AG_Button *b;
	int i, *state;

	for (i = 0; i < bar->nRows; i++) {
		OBJECT_FOREACH_CHILD(b, bar->rows[i], ag_button) {
			stateb = AG_WidgetGetBinding(b, "state", &state);
			*state = 1;
			AG_WidgetBindingChanged(stateb);
			AG_WidgetUnlockBinding(stateb);
		}
	}
}

void
AG_ToolbarDeselectAll(AG_Toolbar *bar)
{
	AG_WidgetBinding *stateb;
	AG_Button *b;
	int i, *state;

	for (i = 0; i < bar->nRows; i++) {
		OBJECT_FOREACH_CHILD(b, bar->rows[i], ag_button) {
			stateb = AG_WidgetGetBinding(b, "state", &state);
			*state = 0;
			AG_WidgetBindingChanged(stateb);
			AG_WidgetUnlockBinding(stateb);
		}
	}
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Box *box = p;
	AG_Widget *chld;
	AG_SizeReq rChld;
	int wMax = 0, hMax = 0;
	int nChld = 0;

	r->w = box->padding*2;
	r->h = box->padding*2;
	OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
		if (chld->flags & AG_WIDGET_HIDE) {
			continue;
		}
		AG_WidgetSizeReq(chld, &rChld);
		if (rChld.w > wMax) { wMax = rChld.w; }
		if (rChld.h > hMax) { hMax = rChld.h; }
		r->h = MAX(r->h, hMax + box->padding*2);
		r->w += rChld.w + box->spacing;
		nChld++;
	}
 	if (nChld > 0)
		r->w -= box->spacing;
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Toolbar *bar = p;
	AG_Box *box = p;
	AG_Widget *chld, *chldLast;
	AG_SizeAlloc aChld;
	AG_SizeReq rChld;
	int x, y, wChld, nChld;

	x = box->padding;
	y = box->padding;

	if (bar->flags & AG_TOOLBAR_HOMOGENOUS) {
		nChld = 0;
		OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
			if ((chld->flags & AG_WIDGET_HIDE) == 0)
				nChld++;
		}
		if (nChld == 0) {
			return (-1);
		}
		wChld = a->w / nChld;
		chldLast = NULL;
		OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
			if (chld->flags & AG_WIDGET_HIDE) {
				continue;
			}
			aChld.x = x;
			aChld.y = y;
			aChld.w = wChld;
			aChld.h = a->h;
			AG_WidgetSizeAlloc(chld, &aChld);
			aChld.x += aChld.w + box->spacing;
			chldLast = chld;
		}
		if (aChld.x < a->w)		/* Compensate for rounding */
			chldLast->w++;
	} else {
		OBJECT_FOREACH_CHILD(chld, box, ag_widget) {
			if (chld->flags & AG_WIDGET_HIDE) {
				continue;
			}
			AG_WidgetSizeReq(chld, &rChld);
			aChld.x = x;
			aChld.y = y;
			aChld.w = rChld.w;
			aChld.h = a->h;
			AG_WidgetSizeAlloc(chld, &aChld);
			aChld.x += aChld.w + box->spacing;
		}
	}
	return (0);
}

const AG_WidgetOps agToolbarOps = {
	{
		"AG_Widget:AG_Box:AG_Toolbar",
		sizeof(AG_Toolbar),
		{ 0,0 },
		NULL,			/* init */
		NULL,			/* reinit */
		AG_BoxDestroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	NULL,			/* draw */
	SizeRequest,
	SizeAllocate
};
