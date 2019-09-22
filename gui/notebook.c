/*
 * Copyright (c) 2005-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
#ifdef AG_WIDGETS

#include <agar/gui/notebook.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>

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
MouseMotion(AG_Event *event)
{
	AG_Notebook *nb = AG_NOTEBOOK_SELF();
	AG_NotebookTab *nt;
	const int x = AG_INT(1);
	const int y = AG_INT(2);
	int tx=0, i=0;
	
	nb->mouseOver = -1;

	if (y >= 0 && y <= nb->bar_h) {
		TAILQ_FOREACH(nt, &nb->tabs, tabs) {
			int wTab = (nt->lbl ? WIDTH(nt->lbl) : 0);
	
			if (x > tx && x < tx+wTab) {
				break;
			}
			tx += wTab;
			i++;
		}
		if (nt == NULL)
			i = -1;
	} else {
		i = -1;
	}
	if (nb->mouseOver != i) {
		nb->mouseOver = i;
		AG_Redraw(nb);
	}

}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Notebook *nb = AG_NOTEBOOK_SELF();
	AG_NotebookTab *nt;
	const int x = AG_INT(2);
	const int y = AG_INT(3);

	if ((nb->flags & AG_NOTEBOOK_HIDE_TABS) == 0 &&
	    y <= nb->bar_h) {
		int tx = 0;

		TAILQ_FOREACH(nt, &nb->tabs, tabs) {
			int wTab = (nt->lbl ? WIDTH(nt->lbl) : 0);

			if (x >= tx && x < tx+wTab) {
				AG_NotebookSelect(nb, nt);
				break;
			}
			tx += wTab;
		}
	}
}

static void
OnShow(AG_Event *event)
{
	AG_Notebook *nb = AG_NOTEBOOK_SELF();

	if (nb->sel_tab == NULL)
		AG_NotebookSelect(nb, TAILQ_FIRST(&nb->tabs));
}

static void
OnHide(AG_Event *event)
{
	AG_Notebook *nb = AG_NOTEBOOK_SELF();

	if (nb->sel_tab)
		AG_NotebookSelect(nb, NULL);
}

static void
Init(void *obj)
{
	AG_Notebook *nb = obj;

	WIDGET(nb)->flags |= AG_WIDGET_USE_TEXT |
	                     AG_WIDGET_UNFOCUSED_MOTION;

	nb->tab_align = AG_NOTEBOOK_TABS_TOP;
	nb->flags = 0;
	nb->bar_w = -1;
	nb->bar_h = -1;
	nb->cont_w = -1;
	nb->cont_h = -1;
	nb->spacing = -1;
	nb->padding = -1;
	nb->mouseOver = -1;
	nb->nTabs = 0;
	nb->sel_tab = NULL;
	TAILQ_INIT(&nb->tabs);
	nb->r.x = 0;
	nb->r.y = 0;
	nb->r.w = 0;
	nb->r.h = 0;

	AG_AddEvent(nb, "widget-shown", OnShow, NULL);
	AG_AddEvent(nb, "widget-hidden", OnHide, NULL);
	AG_SetEvent(nb, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(nb, "mouse-button-down", MouseButtonDown, NULL);
}

static void
Draw(void *obj)
{
	AG_Notebook *nb = obj;
	AG_NotebookTab *tab;
	AG_Color c,cDark, cHigh,cLow;
	AG_Rect r;
	int boxDia = WFONT(nb)->height;
	int x=0, y=0, xSelFirst=0, xSelLast=0, tabIdx=0;

	c = WCOLOR_SEL(nb,0);
	AG_ColorAddScaled(&cDark, &c, &agShade, 2);	/* Contrast */

	AG_DrawRectFilled(nb, &nb->r, &c);
	AG_ColorAdd(&cHigh, &c, &agHighColor);
	AG_ColorAdd(&cLow,  &c, &agLowColor);
	AG_DrawLineV(nb, 0, nb->bar_h, HEIGHT(nb)-1,
	    (nb->sel_tab == TAILQ_FIRST(&nb->tabs)) ? &cLow : &cHigh);
	
	if (nb->sel_tab) {
/*		AG_PushClipRect(nb, &nb->r); */
		AG_WidgetDraw(nb->sel_tab);
/*		AG_PopClipRect(nb); */
	}
	if (nb->flags & AG_NOTEBOOK_HIDE_TABS) {
		return;
	}
	TAILQ_FOREACH(tab, &nb->tabs, tabs) {
		const int isSelected = (nb->sel_tab == tab);
		AG_Label *lbl = tab->lbl;

		r.x = x;
		r.y = isSelected ? y : y+2;
		r.w = (lbl ? WIDTH(lbl) : 0);
		r.h = isSelected ? nb->bar_h : nb->bar_h-2;
	
		if (r.x+r.w > WIDTH(nb)) {
			r.w = WIDTH(nb) - r.x;
			if (r.w <= boxDia) {
				boxDia -= (boxDia - r.w);
				if (boxDia < 3)
					break;
			}
		}
		if (isSelected || tabIdx == nb->mouseOver) {
			AG_DrawBoxRoundedTop(nb, &r, -1, boxDia >> 1, &c);
		} else {
			AG_DrawBoxRoundedTop(nb, &r, +1, boxDia >> 1, &c);
		}

		if (lbl) {
			if (isSelected) {
				const int boxRad = (boxDia >> 1);
				AG_Color cHalf;

				AG_ColorInterpolate(&cHalf,
						    &WCOLOR_SEL(nb,SHAPE_COLOR),
						    &WCOLOR_SEL(nb,0),
						    1,2);
				lbl->tPad--;

				AG_PutPixel(nb, x+boxRad-1, 2, &cHalf);
				AG_PutPixel(nb, x+r.w-boxRad, 2, &cHalf);
				AG_DrawLineH(nb, x+boxRad, x+r.w-boxRad,
				             1, &WCOLOR_SEL(nb,SHAPE_COLOR));
				AG_DrawLineH(nb, x+boxRad, x+r.w-boxRad,
				             2, &cHalf);
			}

			AG_WidgetDraw(lbl);

			if (isSelected)
				lbl->tPad++;
		}
		x += r.w+1;

		if (isSelected) {
			xSelFirst = x - (r.w+1);
			xSelLast = x;
		}
	
		tabIdx++;
	}

	if (xSelFirst > 0) {
		AG_DrawLineH(nb, 0, xSelFirst, nb->bar_h, &cHigh);
	}
	if (xSelLast < WIDTH(nb)) {
		AG_DrawLineH(nb, xSelLast-1, WIDTH(nb)-1, nb->bar_h, &cHigh);
	}
	AG_DrawLineH(nb, 0, WIDTH(nb)-1, HEIGHT(nb)-1,       &cLow);
	AG_DrawLineV(nb, WIDTH(nb), nb->bar_h+1, HEIGHT(nb), &cLow);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Notebook *nb = obj;
	AG_NotebookTab *tab;
	AG_SizeReq rTab;
	Uint hMax = 0;
	
	TAILQ_FOREACH(tab, &nb->tabs, tabs) {
		AG_SizeReq rLbl;

		if (tab->lbl == NULL) {
			continue;
		}
		AG_WidgetSizeReq(tab->lbl, &rLbl);
		hMax = MAX(hMax, rLbl.h + 4);
	}

	if ((nb->flags & AG_NOTEBOOK_HIDE_TABS) == 0) {
		nb->bar_h = hMax;
		nb->bar_w = 0;
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
			if (tab->lbl)
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
	int x=0, y=0;

	if (a->h < nb->bar_h || a->w < WFONT(nb)->height) {
		return (-1);
	}
	TAILQ_FOREACH(tab, &nb->tabs, tabs) {
		if (tab->lbl == NULL) {
			continue;
		}
		AG_WidgetSizeReq(tab->lbl, &rLbl);
		aLbl.x = x;
		aLbl.y = y;
		aLbl.w = MIN(rLbl.w, WIDTH(nb) - x);
		aLbl.h = rLbl.h;
		AG_WidgetSizeAlloc(tab->lbl, &aLbl);

		x += aLbl.w;
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
	nb->tab_align = ta;
	AG_Redraw(nb);
}

void
AG_NotebookSetSpacing(AG_Notebook *nb, int spacing)
{
	nb->spacing = spacing;
	AG_Redraw(nb);
}

void
AG_NotebookSetPadding(AG_Notebook *nb, int padding)
{
	nb->padding = padding;
	AG_Redraw(nb);
}

AG_NotebookTab *
AG_NotebookAdd(AG_Notebook *nb, const char *label, enum ag_box_type btype)
{
	AG_NotebookTab *tab;

	tab = Malloc(sizeof(AG_NotebookTab));
	AG_ObjectInit(tab, &agNotebookTabClass);
	AG_ObjectSetName(tab, "tab%u", nb->nTabs);
	AG_BoxSetType(&tab->box, btype);
	AG_Expand(tab);

	AG_ObjectLock(nb);

	if (nb->padding >= 0)
		AG_BoxSetPadding(&tab->box, nb->padding);
	if (nb->spacing >= 0)
		AG_BoxSetSpacing(&tab->box, nb->spacing);

	if (label && label[0] != '\0') {
		tab->lbl = AG_LabelNewS(nb, 0, label);
		AG_LabelSetPadding(tab->lbl, 8,8,5,0);
		AG_SetStyle(tab->lbl, "font-size", "110%");
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
	if (tab->lbl) {
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
	} else if (nb->sel_tab) {
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
		NULL,		/* reset */
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
		"Agar(Widget:Box:NotebookTab)",
		sizeof(AG_NotebookTab),
		{ 0,0 },
		NULL,		/* init */
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_WidgetInheritDraw,
	AG_WidgetInheritSizeRequest,
	AG_WidgetInheritSizeAllocate
};

#endif /* AG_WIDGETS */
