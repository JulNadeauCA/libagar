/*
 * Copyright (c) 2005-2010 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "notebook.h"
#include "window.h"
#include "primitive.h"

#define SPACING 8

AG_Notebook *
AG_NotebookNew(void *parent, Uint flags)
{
	AG_Notebook *nb;

	nb = Malloc(sizeof(AG_Notebook));
	AG_ObjectInit(nb, &agNotebookClass);
	nb->flags |= flags;
	
	if (flags & AG_NOTEBOOK_HFILL) { AG_ExpandHoriz(nb); }
	if (flags & AG_NOTEBOOK_VFILL) { AG_ExpandVert(nb); }

	AG_ObjectAttach(parent, nb);
	return (nb);
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Notebook *nb = AG_SELF();
	int x = AG_INT(2);
	int y = AG_INT(3);

	if ((nb->flags & AG_NOTEBOOK_HIDE_TABS) == 0 &&
	    y <= nb->bar_h) {
		AG_NotebookTab *tab;
		int tx = SPACING;

		TAILQ_FOREACH(tab, &nb->tabs, tabs) {
			AG_Surface *label = WSURFACE(nb,tab->label);

#if 0
			if (tx+label->w+SPACING*2 > WIDGET(nb)->w)
				break;
#endif
			if (x >= tx && x < tx+(label->w + SPACING*2)) {
				AG_NotebookSelectTab(nb, tab);
				AG_Redraw(nb);
				break;
			}
			tx += label->w + SPACING*2;
		}
	}
}

static void
Shown(AG_Event *event)
{
	AG_Notebook *nb = AG_SELF();

	if (nb->sel_tab != NULL) {
		AG_NotebookSelectTab(nb, nb->sel_tab);
	} else if (!TAILQ_EMPTY(&nb->tabs)) {
		AG_NotebookSelectTab(nb, TAILQ_FIRST(&nb->tabs));
	}
}

static void
Init(void *obj)
{
	AG_Notebook *nb = obj;

	nb->flags = 0;
	nb->tab_align = AG_NOTEBOOK_TABS_TOP;
	nb->sel_tab = NULL;
	nb->bar_w = -1;
	nb->bar_h = -1;
	nb->cont_w = -1;
	nb->cont_h = -1;
	nb->spacing = -1;
	nb->padding = -1;
	nb->lblPartial = -1;
	nb->tabFont = NULL;
	nb->r = AG_RECT(0,0,0,0);
	TAILQ_INIT(&nb->tabs);
	
	AG_NotebookSetTabFont(nb, AG_FetchFont(NULL, agDefaultFont->size-1, 0));
	AG_SetEvent(nb, "mouse-button-down", MouseButtonDown, NULL);
	AG_AddEvent(nb, "widget-shown", Shown, NULL);

#ifdef AG_DEBUG
	AG_BindInt(nb, "flags", &nb->flags);
	AG_BindUint(nb, "tab_align", &nb->tab_align);
	AG_BindInt(nb, "bar_w", &nb->bar_w);
	AG_BindInt(nb, "bar_h", &nb->bar_h);
	AG_BindInt(nb, "cont_w", &nb->cont_w);
	AG_BindInt(nb, "cont_h", &nb->cont_h);
	AG_BindInt(nb, "spacing", &nb->spacing);
	AG_BindInt(nb, "padding", &nb->padding);
	AG_BindPointer(nb, "tabFont", (void *)&nb->tabFont);
	AG_BindInt(nb, "lblPartial", &nb->lblPartial);
	AG_BindInt(nb, "lblPartialWidth", &nb->lblPartialWidth);
	AG_BindPointer(nb, "sel_tab", (void *)&nb->sel_tab);
#endif /* AG_DEBUG */
}

static void
Draw(void *obj)
{
	AG_Notebook *nb = obj;
	AG_NotebookTab *tab;
	int x = SPACING;
	int y = SPACING;
	int idx = 0;
	AG_Rect r;

	STYLE(nb)->NotebookBackground(nb, nb->r);
	
	if (nb->sel_tab != NULL) {
		AG_PushClipRect(nb, nb->r);
		AG_WidgetDraw(&nb->sel_tab->box);
		AG_PopClipRect(nb);
	}

	if (nb->flags & AG_NOTEBOOK_HIDE_TABS) {
		return;
	}
	TAILQ_FOREACH(tab, &nb->tabs, tabs) {
		if (tab->label == -1) {
			AG_PushTextState();
			AG_TextFont(nb->tabFont);
			AG_TextColor(agColors[NOTEBOOK_TXT_COLOR]);
			tab->label = AG_WidgetMapSurface(nb,
			    AG_TextRender(tab->labelText));
			AG_PopTextState();
		}
		r.x = x;
		r.y = y;
		r.w = WSURFACE(nb,tab->label)->w + SPACING*2;
		r.h = nb->bar_h - SPACING;
		
		if (r.x+r.w > WIDTH(nb)) {
			if ((r.w = WIDTH(nb) - r.x) <=
			    nb->lblPartialWidth + SPACING*2) {
				break;
			}
			if (nb->lblPartial == -1) {
				AG_PushTextState();
				AG_TextFont(nb->tabFont);
				AG_TextColor(agColors[NOTEBOOK_TXT_COLOR]);
				nb->lblPartial = AG_WidgetMapSurface(nb,
				    AG_TextRender("..."));
				AG_PopTextState();
			}
			STYLE(nb)->NotebookTabBackground(nb, r, idx,
			    (nb->sel_tab == tab));
			AG_WidgetBlitSurface(nb, nb->lblPartial, x+SPACING,
			    y+(r.h/2 - WSURFACE(nb,nb->lblPartial)->h/2));
			break;
		}

		STYLE(nb)->NotebookTabBackground(nb, r, idx++,
		    (nb->sel_tab == tab));
		
		AG_WidgetBlitSurface(nb, tab->label, x+SPACING,
		    y+(r.h/2 - WSURFACE(nb,tab->label)->h/2));
		x += r.w;
	}
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Notebook *nb = obj;
	AG_NotebookTab *tab;
	AG_SizeReq rTab;

	if ((nb->flags & AG_NOTEBOOK_HIDE_TABS) == 0) {
		nb->bar_h = agTextFontHeight + SPACING*2;
		nb->bar_w = SPACING*2;
	} else {
		nb->bar_h = 0;
		nb->bar_w = 0;
	}
	nb->cont_w = 0;
	nb->cont_h = 0;
	TAILQ_FOREACH(tab, &nb->tabs, tabs) {
		AG_WidgetSizeReq(tab, &rTab);

		nb->cont_w = MAX(nb->cont_w,rTab.w);
		nb->cont_h = MAX(nb->cont_h,rTab.h);
		if ((nb->flags & AG_NOTEBOOK_HIDE_TABS) == 0)
			nb->bar_w += SPACING*2;
			/* + WSURFACE(nb,tab->label)->w */
	}
	r->h = nb->cont_h + nb->bar_h;
	r->w = MAX(nb->cont_w, nb->bar_w);
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Notebook *nb = obj;
	AG_NotebookTab *tab;
	AG_SizeAlloc aTab;

	if (a->h < nb->bar_h) {
		return (-1);
	}
	if ((tab = nb->sel_tab) != NULL) {
		aTab.x = 0;
		aTab.y = nb->bar_h;
		aTab.w = a->w;
		aTab.h = a->h - nb->bar_h;
		AG_WidgetSizeAlloc(tab, &aTab);
	}
	nb->r.x = 0;
	nb->r.y = nb->bar_h;
	nb->r.w = a->w;
	nb->r.h = a->h - nb->bar_h;
	return (0);
}

void
AG_NotebookSetTabAlignment(AG_Notebook *nb, enum ag_notebook_tab_alignment ta)
{
	AG_ObjectLock(nb);
	nb->tab_align = ta;
	AG_ObjectUnlock(nb);
	AG_Redraw(nb);
}

void
AG_NotebookSetSpacing(AG_Notebook *nb, int spacing)
{
	AG_ObjectLock(nb);
	nb->spacing = spacing;
	AG_ObjectUnlock(nb);
	AG_Redraw(nb);
}

void
AG_NotebookSetTabFont(AG_Notebook *nb, AG_Font *font)
{
	AG_ObjectLock(nb);
	nb->tabFont = font;
	AG_TextSize("...", &nb->lblPartialWidth, NULL);
	AG_ObjectUnlock(nb);
	AG_Redraw(nb);
}

void
AG_NotebookSetPadding(AG_Notebook *nb, int padding)
{
	AG_ObjectLock(nb);
	nb->padding = padding;
	AG_ObjectUnlock(nb);
	AG_Redraw(nb);
}

AG_NotebookTab *
AG_NotebookAddTab(AG_Notebook *nb, const char *label, enum ag_box_type btype)
{
	AG_NotebookTab *tab;

	tab = Malloc(sizeof(AG_NotebookTab));
	AG_ObjectInit(tab, &agNotebookTabClass);
	AG_BoxSetType(&tab->box, btype);
	AG_Expand(tab);

	AG_ObjectLock(nb);

	if (nb->padding >= 0)
		AG_BoxSetPadding(&tab->box, nb->padding);
	if (nb->spacing >= 0)
		AG_BoxSetSpacing(&tab->box, nb->spacing);

	Strlcpy(tab->labelText, label, sizeof(tab->labelText));
	tab->label = -1;

	TAILQ_INSERT_TAIL(&nb->tabs, tab, tabs);
	if (TAILQ_FIRST(&nb->tabs) == tab)
		AG_NotebookSelectTab(nb, tab);
	
	AG_ObjectUnlock(nb);
	AG_Redraw(nb);
	return (tab);
}

void
AG_NotebookDelTab(AG_Notebook *nb, AG_NotebookTab *tab)
{
	AG_ObjectLock(nb);
	TAILQ_REMOVE(&nb->tabs, tab, tabs);
	AG_WidgetUnmapSurface(nb, tab->label);
	AG_ObjectDestroy(tab);
	AG_ObjectUnlock(nb);
	AG_Redraw(nb);
}

void
AG_NotebookSelectTab(AG_Notebook *nb, AG_NotebookTab *tab)
{
	AG_SizeReq rTab;
	AG_SizeAlloc aTab;

	AG_LockVFS(nb);
	AG_ObjectLock(nb);

	if (nb->sel_tab != NULL) {
		AG_WidgetHiddenRecursive(nb->sel_tab);
		AG_ObjectDetach(nb->sel_tab);
		OBJECT(nb->sel_tab)->flags &= ~(AG_OBJECT_NAME_ONATTACH);
	}
	if (tab == NULL) {
		nb->sel_tab = NULL;
		goto out;
	}
	AG_ObjectAttach(nb, tab);
	nb->sel_tab = tab;

	AG_WidgetSizeReq(tab, &rTab);
	aTab.x = 0;
	aTab.y = nb->bar_h;
	aTab.w = WIDGET(nb)->w;
	aTab.h = WIDGET(nb)->h - nb->bar_h;
	AG_WidgetSizeAlloc(tab, &aTab);
	AG_WidgetShownRecursive(tab);

	AG_WidgetUpdate(nb);
/* 	AG_WidgetFocus(tab); */
out:
	AG_ObjectUnlock(nb);
	AG_UnlockVFS(nb);
	AG_Redraw(nb);
}

void
AG_NotebookSetTabVisibility(AG_Notebook *nb, int flag)
{
	AG_ObjectLock(nb);
	AG_SETFLAGS(nb->flags, AG_NOTEBOOK_HIDE_TABS, flag);
	AG_ObjectUnlock(nb);
	AG_Redraw(nb);
}

AG_WidgetClass agNotebookClass = {
	{
		"Agar(Widget:Notebook)",
		sizeof(AG_Notebook),
		{ 0,0 },
		Init,
		NULL,		/* free */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

AG_WidgetClass agNotebookTabClass = {
	{
		"AG_Widget:AG_Box:AG_NotebookTab",
		sizeof(AG_NotebookTab),
		{ 0,0 },
		NULL,			/* init */
		NULL,			/* free */
		NULL,			/* destroy */
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	AG_WidgetInheritDraw,
	AG_WidgetInheritSizeRequest,
	AG_WidgetInheritSizeAllocate
};
