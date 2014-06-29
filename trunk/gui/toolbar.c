/*
 * Copyright (c) 2004-2010 Hypertriton, Inc. <http://hypertriton.com/>
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
#include <agar/gui/toolbar.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>
#include <agar/gui/separator.h>

AG_Toolbar *
AG_ToolbarNew(void *parent, enum ag_toolbar_type type, int nRows, Uint flags)
{
	AG_Toolbar *bar;
	int i;

	bar = Malloc(sizeof(AG_Toolbar));
	AG_ObjectInit(bar, &agToolbarClass);
	bar->flags |= flags;
	bar->type = type;

	for (i = 0; i < nRows && i < AG_TOOLBAR_MAX_ROWS; i++) {
		int bflags = 0;

		if (flags & AG_TOOLBAR_HOMOGENOUS) {
			bflags = AG_BOX_HOMOGENOUS;
		}
		switch (type) {
		case AG_TOOLBAR_HORIZ:
			bar->rows[i] = AG_BoxNew(bar, AG_BOX_HORIZ, bflags);
			break;
		case AG_TOOLBAR_VERT:
			bar->rows[i] = AG_BoxNew(bar, AG_BOX_VERT, bflags);
			break;
		}
		if (flags & AG_TOOLBAR_HFILL) { AG_ExpandHoriz(bar->rows[i]); }
		if (flags & AG_TOOLBAR_VFILL) { AG_ExpandVert(bar->rows[i]); }
		AG_BoxSetPadding(bar->rows[i], 1);
		AG_BoxSetSpacing(bar->rows[i], 1);
		bar->nRows++;
	}
	AG_BoxSetPadding(AGBOX(bar), 0);
	AG_BoxSetSpacing(AGBOX(bar), 1);
	AG_ObjectAttach(parent, bar);
	return (bar);
}

static void
Init(void *obj)
{
	AG_Toolbar *bar = obj;
	
	WIDGET(bar)->flags |= AG_WIDGET_NOSPACING;

	bar->flags = 0;
	bar->type = AG_TOOLBAR_HORIZ;
	bar->nRows = 0;
	bar->curRow = 0;
	bar->nButtons = 0;
}

static void
StickyUpdate(AG_Event *event)
{
	AG_Button *selBtn = AG_SELF();
	AG_Toolbar *bar = AG_PTR(1);
	AG_Variable *stateb;
	AG_Button *oBtn;
	int i;

	AG_ObjectLock(bar);
	for (i = 0; i < bar->nRows; i++) {
		OBJECT_FOREACH_CHILD(oBtn, bar->rows[i], ag_button) {
			int *state;

			stateb = AG_GetVariable(oBtn, "state", &state);
			if (bar->flags & AG_TOOLBAR_MULTI_STICKY) {
				*state = !(*state);
			} else {
				*state = (oBtn == selBtn);
			}
			AG_UnlockVariable(stateb);
		}
	}
	AG_ObjectUnlock(bar);
}

void
AG_ToolbarRow(AG_Toolbar *bar, int row)
{
	AG_ObjectLock(bar);
#ifdef AG_DEBUG
	if (row < 0 || row >= bar->nRows)
		AG_FatalError("no such row %d", row);
#endif
	bar->curRow = row;
	AG_ObjectUnlock(bar);
}

AG_Button *
AG_ToolbarButtonIcon(AG_Toolbar *bar, AG_Surface *icon, int def,
    void (*handler)(AG_Event *), const char *fmt, ...)
{
	AG_Button *bu;
	AG_Event *ev;

	AG_ObjectLock(bar);

	bu = AG_ButtonNewS(bar->rows[bar->curRow], 0, NULL);
	AG_ButtonSurface(bu, icon);
	AG_ButtonSetFocusable(bu, 0);
	AG_ButtonSetSticky(bu, bar->flags & AG_TOOLBAR_STICKY);
	AG_SetInt(bu, "state", def);
	bar->nButtons++;
	
	ev = AG_SetEvent(bu, "button-pushed", handler, NULL);
	AG_EVENT_GET_ARGS(ev, fmt);
	
	if (bar->flags & (AG_TOOLBAR_STICKY|AG_TOOLBAR_MULTI_STICKY)) {
		AG_AddEvent(bu, "button-pushed", StickyUpdate, "%p", bar);
	}
	
	AG_ObjectUnlock(bar);
	AG_Redraw(bar);
	return (bu);
}

AG_Button *
AG_ToolbarButton(AG_Toolbar *bar, const char *text, int def,
    void (*handler)(AG_Event *), const char *fmt, ...)
{
	AG_Button *bu;
	AG_Event *ev;
	
	AG_ObjectLock(bar);

	bu = AG_ButtonNewS(bar->rows[bar->curRow], 0, text);
	AG_ButtonSetFocusable(bu, 0);
	AG_ButtonSetSticky(bu, bar->flags & AG_TOOLBAR_STICKY);
	AG_SetInt(bu, "state", def);
	bar->nButtons++;
	
	ev = AG_SetEvent(bu, "button-pushed", handler, NULL);
	AG_EVENT_GET_ARGS(ev, fmt);

	if (bar->flags & (AG_TOOLBAR_STICKY|AG_TOOLBAR_MULTI_STICKY)) {
		AG_AddEvent(bu, "button-pushed", StickyUpdate, "%p", bar);
	}
	
	AG_ObjectUnlock(bar);
	AG_Redraw(bar);
	return (bu);
}

void
AG_ToolbarSeparator(AG_Toolbar *bar)
{
	AG_ObjectLock(bar);
	AG_SeparatorNew(bar->rows[bar->curRow],
	    (bar->type == AG_TOOLBAR_HORIZ) ?
	    AG_SEPARATOR_VERT : AG_SEPARATOR_HORIZ);
	AG_ObjectUnlock(bar);
	AG_Redraw(bar);
}

void
AG_ToolbarSelect(AG_Toolbar *bar, AG_Button *bSel)
{
	AG_SetInt(bSel, "state", 1);
	AG_Redraw(bar);
}

void
AG_ToolbarDeselect(AG_Toolbar *bar, AG_Button *bSel)
{
	AG_SetInt(bSel, "state", 0);
	AG_Redraw(bar);
}

void
AG_ToolbarSelectOnly(AG_Toolbar *bar, AG_Button *bSel)
{
	AG_Variable *stateb;
	AG_Button *b;
	int i, *state;

	AG_ObjectLock(bar);
	for (i = 0; i < bar->nRows; i++) {
		OBJECT_FOREACH_CHILD(b, bar->rows[i], ag_button) {
			stateb = AG_GetVariable(b, "state", &state);
			*state = (b == bSel);
			AG_UnlockVariable(stateb);
		}
	}
	AG_ObjectUnlock(bar);
	AG_Redraw(bar);
}

void
AG_ToolbarSelectAll(AG_Toolbar *bar)
{
	AG_Variable *stateb;
	AG_Button *b;
	int i, *state;

	AG_ObjectLock(bar);
	for (i = 0; i < bar->nRows; i++) {
		OBJECT_FOREACH_CHILD(b, bar->rows[i], ag_button) {
			stateb = AG_GetVariable(b, "state", &state);
			*state = 1;
			AG_UnlockVariable(stateb);
		}
	}
	AG_ObjectUnlock(bar);
	AG_Redraw(bar);
}

void
AG_ToolbarDeselectAll(AG_Toolbar *bar)
{
	AG_Variable *stateb;
	AG_Button *b;
	int i, *state;

	AG_ObjectLock(bar);
	for (i = 0; i < bar->nRows; i++) {
		OBJECT_FOREACH_CHILD(b, bar->rows[i], ag_button) {
			stateb = AG_GetVariable(b, "state", &state);
			*state = 0;
			AG_UnlockVariable(stateb);
		}
	}
	AG_ObjectUnlock(bar);
	AG_Redraw(bar);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Toolbar *tbar = p;
	AG_SizeReq rBar;
	int i;

	WIDGET_SUPER_OPS(tbar)->size_request(tbar, r);

	rBar = *r;
	rBar.h = (tbar->type == AG_TOOLBAR_HORIZ) ? r->h/tbar->nRows :
	                                            r->w/tbar->nRows;
	for (i = 0; i < tbar->nRows; i++)
		WIDGET_SUPER_OPS(tbar)->size_request(tbar->rows[i], &rBar);
}

AG_WidgetClass agToolbarClass = {
	{
		"Agar(Widget:Box:Toolbar)",
		sizeof(AG_Toolbar),
		{ 0,0 },
		Init,
		NULL,		/* free */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_WidgetInheritDraw,
	SizeRequest,
	AG_WidgetInheritSizeAllocate
};
