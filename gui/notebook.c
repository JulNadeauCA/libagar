/*
 * Copyright (c) 2005-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Notebook container widget with tabbed navigation.
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
	
	if (flags & AG_NOTEBOOK_HFILL) { WIDGET(nb)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_NOTEBOOK_VFILL) { WIDGET(nb)->flags |= AG_WIDGET_VFILL; }
	nb->flags |= flags;

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
	
	AG_WindowFocus(AG_ParentWindow(nb));
	
	if (nb->flags & AG_NOTEBOOK_HIDE_TABS)
		return;

	if (y <= nb->bar_h) {
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

	if (nb->selTab == NULL)
		AG_NotebookSelect(nb, TAILQ_FIRST(&nb->tabs));
}

static void
OnHide(AG_Event *event)
{
	AG_Notebook *nb = AG_NOTEBOOK_SELF();

	if (nb->selTab)
		AG_NotebookSelect(nb, NULL);
}

static void
Init(void *obj)
{
	AG_Notebook *nb = obj;

	WIDGET(nb)->flags |= AG_WIDGET_USE_TEXT |
	                     AG_WIDGET_UNFOCUSED_MOTION;

	memset(&nb->bar_w, 0xff, sizeof(int) + /* bar_w (= -1) */
	                         sizeof(int) + /* bar_h */
	                         sizeof(int) + /* cont_w */
	                         sizeof(int) + /* cont_h */
	                         sizeof(int) + /* mouseOver */
	                         sizeof(int)); /* selTabID */

	memset(&nb->flags, 0, sizeof(Uint) +              /* flags */
	                      sizeof(Uint) +              /* nTabs */
	                      sizeof(AG_NotebookTab *) +  /* selTab */
			      sizeof(AG_Rect));           /* r */

	TAILQ_INIT(&nb->tabs);

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
	const AG_Color *cBg = &WCOLOR(nb, BG_COLOR);
	const AG_Color *cHi = &WCOLOR(nb, HIGH_COLOR);
	const AG_Color *cLo = &WCOLOR(nb, LOW_COLOR);
	AG_Rect r;
	const int w = WIDTH(nb);
	const int h = HEIGHT(nb);
	int boxDia = WFONT(nb)->height;
	int x=0, y=0, xSelFirst=0, xSelLast=0, tabIdx=0;

	if (cBg->a == AG_OPAQUE) {
		AG_DrawRectFilled(nb, &nb->r, cBg);
	} else if (cBg->a > 0) {
		AG_DrawRectBlended(nb, &nb->r, cBg, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
	}

	AG_DrawLineV(nb, 0, nb->bar_h, h-1,
	    (nb->selTab == TAILQ_FIRST(&nb->tabs)) ? cLo : cHi);
	
	if (nb->selTab) {
/*		AG_PushClipRect(nb, &nb->r); */
		AG_WidgetDraw(nb->selTab);
/*		AG_PopClipRect(nb); */
	}

	if (nb->flags & AG_NOTEBOOK_HIDE_TABS)
		return;

	TAILQ_FOREACH(tab, &nb->tabs, tabs) {
		const int isSelected = (nb->selTab == tab);
		AG_Label *lbl = tab->lbl;

		r.x = x;
		r.y = isSelected ? y : y+2;
		r.w = (lbl ? WIDTH(lbl) : 0);
		r.h = isSelected ? nb->bar_h : nb->bar_h-2;
	
		if (r.x+r.w > w) {
			r.w = w - r.x;
			if (r.w <= boxDia) {
				boxDia -= (boxDia - r.w);
				if (boxDia < 3)
					break;
			}
		}

		AG_DrawBoxRoundedTop(nb, &r,
		    (isSelected || tabIdx == nb->mouseOver) ? -1 : +1,
		    (boxDia >> 1), &WCOLOR(nb, FG_COLOR));

		if (lbl) {
			if (isSelected) {
				AG_Color cHalf;
				const int boxRad = (boxDia >> 1);
				
				AG_ColorInterpolate(&cHalf,
				    &WCOLOR(nb, SELECTION_COLOR),
				    &WCOLOR(nb, FG_COLOR),
				    1,2);

				AG_PutPixel(nb, x+boxRad-1,   2, &cHalf);
				AG_PutPixel(nb, x+r.w-boxRad, 2, &cHalf);

				AG_DrawLineH(nb, x+boxRad, x+r.w-boxRad, 1,
				             &WCOLOR(nb, SELECTION_COLOR));
				AG_DrawLineH(nb, x+boxRad, x+r.w-boxRad, 2,
				             &cHalf);
			}

			AG_WidgetDraw(lbl);
		}
		x += r.w;

		if (isSelected) {
			xSelFirst = x - (r.w);
			xSelLast = x;
		}
		tabIdx++;
	}

	if (xSelFirst > 0) {
		AG_DrawLineH(nb, 0, xSelFirst, nb->bar_h, cHi);
	}
	if (xSelLast < w) {
		AG_DrawLineH(nb, xSelLast-1, w-1, nb->bar_h, cHi);
	}
	AG_DrawLineH(nb, 0, w-1,         h-1, cLo);
	AG_DrawLineV(nb, w, nb->bar_h+1, h,   cLo);
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
	int x=0, y=0, wMin;
	const int boxDia = WFONT(nb)->height;

	if (a->h < nb->bar_h || a->w < WFONT(nb)->height)
		return (-1);

#ifdef AG_UNICODE
	AG_TextSize("\xE2\x80\xA6", &wMin, NULL); /* U+2026 ELLIPSIS */
#else
	AG_TextSize("...", &wMin, NULL);
#endif
	wMin += boxDia;

	TAILQ_FOREACH(tab, &nb->tabs, tabs) {
		if (tab->lbl == NULL) {
			continue;
		}
		AG_WidgetSizeReq(tab->lbl, &rLbl);
		aLbl.x = x;
		aLbl.y = y;
		aLbl.w = MIN(rLbl.w, WIDTH(nb) - x);
		if (aLbl.w < wMin) {
			WIDGET(tab->lbl)->flags |= AG_WIDGET_UNDERSIZE;
			break;
		} else {
			WIDGET(tab->lbl)->flags &= ~(AG_WIDGET_UNDERSIZE);
		}
		aLbl.h = rLbl.h;
		AG_WidgetSizeAlloc(tab->lbl, &aLbl);

		x += aLbl.w;
		if (x > WIDTH(nb))
			break;
	}
	if ((tab = nb->selTab) != NULL) {
		aTab.x = 0;
		aTab.y = nb->bar_h;
		aTab.w = a->w;
		aTab.h = a->h - nb->bar_h;
		AG_WidgetSizeAlloc(tab, &aTab);
	}
	nb->r.x = WIDGET(nb)->paddingLeft;
	nb->r.y = nb->bar_h + WIDGET(nb)->paddingTop;
	nb->r.w = a->w - WIDGET(nb)->paddingRight;
	nb->r.h = a->h - nb->bar_h - WIDGET(nb)->paddingBottom;
	return (0);
}

/* Set a common "spacing" attribute over all tabs. */
void
AG_NotebookSetSpacing(AG_Notebook *nb, int spacing)
{
	AG_NotebookTab *nt;

	AG_OBJECT_ISA(nb, "AG_Widget:AG_Notebook:*");
	AG_ObjectLock(nb);

	TAILQ_FOREACH(nt, &nb->tabs, tabs)
		AG_SetStyleF(nt, "spacing", "%d", spacing);

	AG_ObjectUnlock(nb);
}

/* Set a common "padding" attribute over all tabs. */
void
AG_NotebookSetPadding(AG_Notebook *nb, int padding)
{
	AG_NotebookTab *nt;

	AG_OBJECT_ISA(nb, "AG_Widget:AG_Notebook:*");
	AG_ObjectLock(nb);

	TAILQ_FOREACH(nt, &nb->tabs, tabs)
		AG_SetStyleF(nt, "padding", "%d", padding);

	AG_ObjectUnlock(nb);
}

AG_NotebookTab *
AG_NotebookAdd(AG_Notebook *nb, const char *label, enum ag_box_type btype)
{
	AG_NotebookTab *tab;

	AG_OBJECT_ISA(nb, "AG_Widget:AG_Notebook:*");

	tab = Malloc(sizeof(AG_NotebookTab));
	AG_ObjectInit(tab, &agNotebookTabClass);
	AG_ObjectSetName(tab, "tab%u", nb->nTabs);
	AG_BoxSetType(&tab->box, btype);

	WIDGET(tab)->flags |= AG_WIDGET_EXPAND;

	AG_ObjectLock(nb);

	if (label && label[0] != '\0') {
		tab->lbl = AG_LabelNew(nb, 0, " %s ", label);
		AG_SetStyle(tab->lbl, "padding", "5 10 5 10");  /* TODO E>F */
	} else {
		tab->lbl = NULL;
	}

	AG_ObjectAttach(nb, tab);
	TAILQ_INSERT_TAIL(&nb->tabs, tab, tabs);
	tab->id = nb->nTabs++;

	AG_Redraw(nb);
	AG_ObjectUnlock(nb);

	return (tab);
}

/*
 * Return an active tab by numerical ID.
 * The Notebook must be locked.
 */
AG_NotebookTab *
AG_NotebookGetByID(AG_Notebook *nb, int id)
{
	AG_NotebookTab *nt;

	TAILQ_FOREACH(nt, &nb->tabs, tabs) {
		if (nt->id == id)
			return (nt);
	}
	return (NULL);
}

/*
 * Return an active tab by text contents.
 * The Notebook must be locked.
 */
AG_NotebookTab *
AG_NotebookGetByName(AG_Notebook *nb, const char *label)
{
	AG_NotebookTab *nt;

	TAILQ_FOREACH(nt, &nb->tabs, tabs) {
		if (nt->lbl && nt->lbl->text &&
		    strcmp(nt->lbl->text, label) == 0)
			return (nt);
	}
	return (NULL);
}

void
AG_NotebookDel(AG_Notebook *nb, AG_NotebookTab *tab)
{
	AG_OBJECT_ISA(nb, "AG_Widget:AG_Notebook:*");
	AG_ObjectLock(nb);

	if (nb->selTab == tab) {
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

	AG_Redraw(nb);
	AG_ObjectUnlock(nb);
}

void
AG_NotebookSelectByID(AG_Notebook *nb, int id)
{
	AG_NotebookTab *nt;

	AG_OBJECT_ISA(nb, "AG_Widget:AG_Notebook:*");
	AG_ObjectLock(nb);
	AG_LockVFS(nb);

	if ((nt = AG_NotebookGetByID(nb,id)) != NULL)
		AG_NotebookSelect(nb, nt);

	AG_UnlockVFS(nb);
	AG_ObjectUnlock(nb);
}

void
AG_NotebookSelect(AG_Notebook *nb, AG_NotebookTab *tab)
{
	AG_SizeReq rTab;
	AG_SizeAlloc aTab;

	AG_OBJECT_ISA(nb, "AG_Widget:AG_Notebook:*");
	AG_ObjectLock(nb);
	AG_LockVFS(nb);

	if (nb->selTab == tab) {
		goto no_change;
	} else if (nb->selTab) {
		AG_WidgetHideAll(nb->selTab);
	}
	if (tab == NULL) {
		nb->selTab = NULL;
		nb->selTabID = -1;
		goto out;
	}
	nb->selTab = tab;
	nb->selTabID = tab->id;

	AG_WidgetSizeReq(tab, &rTab);
	aTab.x = 0;
	aTab.y = nb->bar_h;
	aTab.w = WIDGET(nb)->w;
	aTab.h = WIDGET(nb)->h - nb->bar_h;
	AG_WidgetSizeAlloc(tab, &aTab);
	AG_WidgetShowAll(tab);

	WIDGET(nb)->flags |= AG_WIDGET_UPDATE_WINDOW;
/* 	AG_WidgetFocus(tab); */
out:
	AG_Redraw(nb);
no_change:
	AG_UnlockVFS(nb);
	AG_ObjectUnlock(nb);
}

void
AG_NotebookSetTabVisibility(AG_Notebook *nb, int flag)
{
	AG_OBJECT_ISA(nb, "AG_Widget:AG_Notebook:*");
	AG_ObjectLock(nb);

	AG_SETFLAGS(nb->flags, AG_NOTEBOOK_HIDE_TABS, flag);

	AG_Redraw(nb);
	AG_ObjectUnlock(nb);
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
	NULL,			/* draw */
	NULL,			/* size_request */
	NULL			/* size_allocate */
};

#endif /* AG_WIDGETS */
