/*
 * Copyright (c) 2005-2015 Hypertriton, Inc. <http://hypertriton.com/>
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
#include <agar/gui/notebook.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>

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
			int wTab = (tab->lbl ? WIDTH(tab->lbl) : 0) + SPACING*2;

			if (x >= tx && x < tx+wTab) {
				AG_NotebookSelect(nb, tab);
				break;
			}
			tx += wTab;
		}
	}
}

static void
OnShow(AG_Event *event)
{
	AG_Notebook *nb = AG_SELF();

	if (nb->sel_tab == NULL)
		AG_NotebookSelect(nb, TAILQ_FIRST(&nb->tabs));
}

static void
OnHide(AG_Event *event)
{
	AG_Notebook *nb = AG_SELF();

	if (nb->sel_tab != NULL)
		AG_NotebookSelect(nb, NULL);
}

static void
Init(void *obj)
{
	AG_Notebook *nb = obj;

	WIDGET(nb)->flags |= AG_WIDGET_USE_TEXT;

	nb->flags = 0;
	nb->tab_align = AG_NOTEBOOK_TABS_TOP;
	nb->sel_tab = NULL;
	nb->bar_w = -1;
	nb->bar_h = -1;
	nb->cont_w = -1;
	nb->cont_h = -1;
	nb->spacing = -1;
	nb->padding = -1;
	nb->r = AG_RECT(0,0,0,0);
	nb->nTabs = 0;
	TAILQ_INIT(&nb->tabs);

	AG_AddEvent(nb, "widget-shown", OnShow, NULL);
	AG_AddEvent(nb, "widget-hidden", OnHide, NULL);
	AG_SetEvent(nb, "mouse-button-down", MouseButtonDown, NULL);
}

static void
Draw(void *obj)
{
	AG_Notebook *nb = obj;
	AG_Font *font = WIDGET(nb)->font;
	AG_NotebookTab *tab;
	int x = SPACING;
	int y = SPACING;
	AG_Rect r;

	AG_DrawRectFilled(nb, nb->r, WCOLOR_HOV(nb,0));
	
	if (nb->sel_tab != NULL) {
		AG_PushClipRect(nb, nb->r);
		AG_WidgetDraw(nb->sel_tab);
		AG_PopClipRect(nb);
	}

	if (nb->flags & AG_NOTEBOOK_HIDE_TABS) {
		return;
	}
	TAILQ_FOREACH(tab, &nb->tabs, tabs) {
		int isSelected = (nb->sel_tab == tab);
		int wLbl = tab->lbl ? WIDTH(tab->lbl) : 0;

		r.x = x;
		r.y = y;
		r.w = wLbl + SPACING*2;
		r.h = nb->bar_h - SPACING;
	
		if (r.x+r.w > WIDTH(nb)) {
			r.w = WIDTH(nb) - r.x;
			if (r.w <= SPACING*4)
				break;
		}
		AG_DrawBoxRoundedTop(nb, r,
		    isSelected ? -1 : 1, (int)(font->height/1.5),
		    isSelected ? WCOLOR_HOV(nb,0) :
	 	                 WCOLOR(nb,0));
		if (tab->lbl != NULL) {
			AG_WidgetDraw(tab->lbl);
		}
		x += r.w;
	}
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Notebook *nb = obj;
	AG_Font *font = WIDGET(nb)->font;
	AG_NotebookTab *tab;
	AG_SizeReq rTab;

	if ((nb->flags & AG_NOTEBOOK_HIDE_TABS) == 0) {
		nb->bar_h = font->height + SPACING*2;
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
		if ((nb->flags & AG_NOTEBOOK_HIDE_TABS) == 0) {
			nb->bar_w += SPACING*2;
			if (tab->lbl != NULL)
				nb->bar_w += WIDTH(tab->lbl);
		}
	}
	r->h = nb->cont_h + nb->bar_h;
	r->w = MAX(nb->cont_w, nb->bar_w);
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Notebook *nb = obj;
	AG_NotebookTab *tab;
	AG_SizeAlloc aTab, aLbl;
	AG_SizeReq rLbl;

	int x = nb->padding+SPACING;
	int y = nb->padding+SPACING;

	if (a->h < nb->bar_h) {
		return (-1);
	}
	TAILQ_FOREACH(tab, &nb->tabs, tabs) {
		if (tab->lbl == NULL) {
			x += SPACING*4;
			continue;
		}
		AG_WidgetSizeReq(tab->lbl, &rLbl);
		aLbl.x = x+SPACING;
		aLbl.y = y+SPACING;
		aLbl.w = MIN(rLbl.w, WIDTH(nb)-SPACING-x);
		aLbl.h = rLbl.h;
		AG_WidgetSizeAlloc(tab->lbl, &aLbl);
		x += aLbl.w + SPACING*2;
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
AG_NotebookSetPadding(AG_Notebook *nb, int padding)
{
	AG_ObjectLock(nb);
	nb->padding = padding;
	AG_ObjectUnlock(nb);
	AG_Redraw(nb);
}

AG_NotebookTab *
AG_NotebookAdd(AG_Notebook *nb, const char *label, enum ag_box_type btype)
{
	AG_NotebookTab *tab;

	tab = Malloc(sizeof(AG_NotebookTab));
	AG_ObjectInit(tab, &agNotebookTabClass);
	AG_ObjectSetName(tab, "_Tab%u", nb->nTabs);
	AG_BoxSetType(&tab->box, btype);
	AG_Expand(tab);

	AG_ObjectLock(nb);

	if (nb->padding >= 0)
		AG_BoxSetPadding(&tab->box, nb->padding);
	if (nb->spacing >= 0)
		AG_BoxSetSpacing(&tab->box, nb->spacing);

	if (label != NULL && label[0] != '\0') {
		tab->lbl = AG_LabelNewS(nb, 0, label);
	} else {
		tab->lbl = NULL;
	}

	AG_ObjectAttach(nb, tab);
	TAILQ_INSERT_TAIL(&nb->tabs, tab, tabs);
	nb->nTabs++;

	AG_ObjectUnlock(nb);
	AG_Redraw(nb);
	return (tab);
}

void
AG_NotebookDel(AG_Notebook *nb, AG_NotebookTab *tab)
{
	AG_ObjectLock(nb);

	if (nb->sel_tab == tab) {
		AG_NotebookSelect(nb, NULL);
	}
	if (tab->lbl != NULL) {
		AG_ObjectDetach(tab->lbl);
		AG_ObjectDestroy(tab->lbl);
	}
	TAILQ_REMOVE(&nb->tabs, tab, tabs);
	nb->nTabs--;

	AG_ObjectDetach(tab);
	AG_ObjectDestroy(tab);

	AG_ObjectUnlock(nb);
	AG_Redraw(nb);
}

void
AG_NotebookSelect(AG_Notebook *nb, AG_NotebookTab *tab)
{
	AG_SizeReq rTab;
	AG_SizeAlloc aTab;

	AG_LockVFS(nb);
	AG_ObjectLock(nb);

	if (nb->sel_tab == tab) {
		goto out;
	} else if (nb->sel_tab != NULL) {
		AG_WidgetHideAll(nb->sel_tab);
	}
	if (tab == NULL) {
		nb->sel_tab = NULL;
		goto out;
	}
	nb->sel_tab = tab;

	AG_WidgetSizeReq(tab, &rTab);
	aTab.x = 0;
	aTab.y = nb->bar_h;
	aTab.w = WIDGET(nb)->w;
	aTab.h = WIDGET(nb)->h - nb->bar_h;
	AG_WidgetSizeAlloc(tab, &aTab);
	AG_WidgetShowAll(tab);

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
