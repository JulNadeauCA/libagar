/*
 * Copyright (c) 2004-2020 Julien Nadeau Carriere <vedge@csoft.net>
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

/*
 * Toolbar widget. This is a simple subclass of AG_Box(3) which packs a set
 * of AG_Button(3) in one or more rows. It can connect to an AG_Menu(3).
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

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
		AG_Box *row;
		Uint bflags = 0;

		if (flags & AG_TOOLBAR_HOMOGENOUS) {
			bflags = AG_BOX_HOMOGENOUS;
		}
		switch (type) {
		case AG_TOOLBAR_HORIZ:
			row = bar->rows[i] = AG_BoxNew(bar, AG_BOX_HORIZ, bflags);
			break;
		case AG_TOOLBAR_VERT:
			row = bar->rows[i] = AG_BoxNew(bar, AG_BOX_VERT, bflags);
			break;
		default:
			AG_FatalError("Bad type");
		}
		if (flags & AG_TOOLBAR_HFILL) { WIDGET(row)->flags |= AG_WIDGET_HFILL; }
		if (flags & AG_TOOLBAR_VFILL) { WIDGET(row)->flags |= AG_WIDGET_VFILL; }
		AG_SetStyle(row, "padding", "1");
		AG_SetStyle(row, "spacing", "1");
		bar->nRows++;
	}

	AG_ObjectAttach(parent, bar);
	return (bar);
}

static void
Init(void *_Nonnull obj)
{
	AG_Toolbar *bar = obj;
	
	bar->flags = 0;
	bar->type = AG_TOOLBAR_HORIZ;
	bar->nRows = 0;
	bar->curRow = 0;
	bar->nButtons = 0;
}

static void
StickyUpdate(AG_Event *_Nonnull event)
{
	const AG_Button *selBtn = AG_BUTTON_SELF();
	AG_Toolbar *bar = AG_TOOLBAR_PTR(1);
	AG_Variable *stateb;
	AG_Button *oBtn;
	int i;

	AG_ObjectLock(bar);
	for (i = 0; i < bar->nRows; i++) {
		OBJECT_FOREACH_CHILD(oBtn, bar->rows[i], ag_button) {
			int *state;

			stateb = AG_GetVariable(oBtn, "state", (void *)&state);
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

/*
 * Select the active row for subsequent AG_ToolbarButton*() calls
 * and AG_ToolbarSeparator() calls.
 */
void
AG_ToolbarRow(AG_Toolbar *bar, int row)
{
	AG_OBJECT_ISA(bar, "AG_Widget:AG_Box:AG_Toolbar:*");
	AG_ObjectLock(bar);

#ifdef AG_DEBUG
	if (row < 0 || row >= bar->nRows)
		AG_FatalError("Bad row");
#endif
	bar->curRow = row;

	AG_ObjectUnlock(bar);
}

/* Create a new Button (with icon) and attach to the Toolbar. */
AG_Button *
AG_ToolbarButtonIcon(AG_Toolbar *bar, const AG_Surface *icon, int def,
    void (*handler)(AG_Event *), const char *fmt, ...)
{
	AG_Button *bu;
	AG_Event *ev;

	AG_OBJECT_ISA(bar, "AG_Widget:AG_Box:AG_Toolbar:*");
	AG_ObjectLock(bar);

	bu = AG_ButtonNewS(bar->rows[bar->curRow], AG_BUTTON_NO_FOCUS, NULL);
	AG_ButtonSurface(bu, icon);
	AG_ButtonSetSticky(bu, bar->flags & AG_TOOLBAR_STICKY);
	AG_SetInt(bu, "state", def);
	bar->nButtons++;
	
	ev = AG_SetEvent(bu, "button-pushed", handler, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(ev, fmt, ap);
		va_end(ap);
	}
	if (bar->flags & (AG_TOOLBAR_STICKY | AG_TOOLBAR_MULTI_STICKY))
		AG_AddEvent(bu, "button-pushed", StickyUpdate, "%p", bar);

	AG_Redraw(bar);
	AG_ObjectUnlock(bar);
	return (bu);
}

/* Create a new Button (no icon) and attach to the Toolbar. */
AG_Button *
AG_ToolbarButton(AG_Toolbar *bar, const char *text, int def,
    void (*handler)(AG_Event *), const char *fmt, ...)
{
	AG_Button *bu;
	AG_Event *ev;
	
	AG_OBJECT_ISA(bar, "AG_Widget:AG_Box:AG_Toolbar:*");
	AG_ObjectLock(bar);

	bu = AG_ButtonNewS(bar->rows[bar->curRow], AG_BUTTON_NO_FOCUS, text);
	AG_ButtonSetSticky(bu, bar->flags & AG_TOOLBAR_STICKY);
	AG_SetInt(bu, "state", def);
	bar->nButtons++;
	
	ev = AG_SetEvent(bu, "button-pushed", handler, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(ev, fmt, ap);
		va_end(ap);
	}
	if (bar->flags & (AG_TOOLBAR_STICKY | AG_TOOLBAR_MULTI_STICKY))
		AG_AddEvent(bu, "button-pushed", StickyUpdate, "%p", bar);

	AG_Redraw(bar);
	AG_ObjectUnlock(bar);
	return (bu);
}

/* Create a new separator. */
void
AG_ToolbarSeparator(AG_Toolbar *bar)
{
	AG_OBJECT_ISA(bar, "AG_Widget:AG_Box:AG_Toolbar:*");
	AG_ObjectLock(bar);

	AG_SeparatorNew(bar->rows[bar->curRow],
	    (bar->type == AG_TOOLBAR_HORIZ) ?
	    AG_SEPARATOR_VERT : AG_SEPARATOR_HORIZ);

	AG_Redraw(bar);
	AG_ObjectUnlock(bar);
}

/* Set the boolean state of a Button to 1. */
void
AG_ToolbarSelect(AG_Toolbar *bar, AG_Button *bSel)
{
	AG_SetInt(bSel, "state", 1);
	AG_Redraw(bar);
}

/* Set the boolean state of a Button to 0. */
void
AG_ToolbarDeselect(AG_Toolbar *bar, AG_Button *bSel)
{
	AG_SetInt(bSel, "state", 0);
	AG_Redraw(bar);
}

/* Set the boolean state of a Button to 1 and reset all others to 0. */
void
AG_ToolbarSelectOnly(AG_Toolbar *bar, AG_Button *bSel)
{
	AG_Variable *stateb;
	AG_Button *b;
	int i, *state;

	AG_OBJECT_ISA(bar, "AG_Widget:AG_Box:AG_Toolbar:*");
	AG_ObjectLock(bar);

	for (i = 0; i < bar->nRows; i++) {
		OBJECT_FOREACH_CHILD(b, bar->rows[i], ag_button) {
			stateb = AG_GetVariable(b, "state", (void *)&state);
			*state = (b == bSel);
			AG_UnlockVariable(stateb);
		}
	}

	AG_Redraw(bar);
	AG_ObjectUnlock(bar);
}

/* Set the boolean state of all buttons to 1. */
void
AG_ToolbarSelectAll(AG_Toolbar *bar)
{
	AG_Variable *stateb;
	AG_Button *b;
	int i, *state;

	AG_OBJECT_ISA(bar, "AG_Widget:AG_Box:AG_Toolbar:*");
	AG_ObjectLock(bar);

	for (i = 0; i < bar->nRows; i++) {
		OBJECT_FOREACH_CHILD(b, bar->rows[i], ag_button) {
			stateb = AG_GetVariable(b, "state", (void *)&state);
			*state = 1;
			AG_UnlockVariable(stateb);
		}
	}

	AG_Redraw(bar);
	AG_ObjectUnlock(bar);
}

/* Set the boolean state of all buttons to 0. */
void
AG_ToolbarDeselectAll(AG_Toolbar *bar)
{
	AG_Variable *stateb;
	AG_Button *b;
	int i, *state;

	AG_OBJECT_ISA(bar, "AG_Widget:AG_Box:AG_Toolbar:*");
	AG_ObjectLock(bar);

	for (i = 0; i < bar->nRows; i++) {
		OBJECT_FOREACH_CHILD(b, bar->rows[i], ag_button) {
			stateb = AG_GetVariable(b, "state", (void *)&state);
			*state = 0;
			AG_UnlockVariable(stateb);
		}
	}

	AG_Redraw(bar);
	AG_ObjectUnlock(bar);
}

static void
SizeRequest(void *_Nonnull p, AG_SizeReq *_Nonnull r)
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
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	NULL,			/* draw */
	SizeRequest,
	NULL			/* size_allocate */
};

#endif /* AG_WIDGETS */
