/*	$Csoft: notebook.c,v 1.6 2005/06/13 06:06:15 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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

#include "notebook.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

static AG_WidgetOps notebook_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		AG_NotebookDestroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	AG_NotebookDraw,
	AG_NotebookScale
};

#define SPACING 8

AG_Notebook *
AG_NotebookNew(void *parent, int flags)
{
	AG_Notebook *nb;

	nb = Malloc(sizeof(AG_Notebook), M_OBJECT);
	AG_NotebookInit(nb, flags);
	AG_ObjectAttach(parent, nb);
	return (nb);
}

static void
mousebuttondown(int argc, union evarg *argv)
{
	AG_Notebook *nb = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;

	if (y <= nb->bar_h) {
		AG_NotebookTab *tab;
		int tx = SPACING;

		TAILQ_FOREACH(tab, &nb->tabs, tabs) {
			SDL_Surface *label = AGWIDGET_SURFACE(nb,tab->label);

			if (tx+label->w+SPACING*2 > AGWIDGET(nb)->w)
				break;

			if (x >= tx && x < tx+(label->w + SPACING*2)) {
				AG_NotebookSelectTab(nb, tab);
				break;
			}
			tx += label->w + SPACING*2;
		}
	}
}

void
AG_NotebookInit(AG_Notebook *nb, int flags)
{
	AG_WidgetInit(nb, "notebook", &notebook_ops, 0);

	nb->flags = flags;
	nb->tab_align = AG_NOTEBOOK_TABS_TOP;
	nb->sel_tab = NULL;
	nb->bar_w = -1;
	nb->bar_h = -1;
	nb->cont_w = -1;
	nb->cont_h = -1;
	nb->tab_rad = (int)(agTextFontHeight/1.5);
	pthread_mutex_init(&nb->lock, &agRecursiveMutexAttr);
	TAILQ_INIT(&nb->tabs);

	if (flags & AG_NOTEBOOK_WFILL)	AGWIDGET(nb)->flags |= AG_WIDGET_WFILL;
	if (flags & AG_NOTEBOOK_HFILL)	AGWIDGET(nb)->flags |= AG_WIDGET_HFILL;

	AG_SetEvent(nb, "window-mousebuttondown", mousebuttondown, NULL);
}

void
AG_NotebookDestroy(void *p)
{
	AG_Notebook *nb = p;

	pthread_mutex_destroy(&nb->lock);
}

void
AG_NotebookDraw(void *p)
{
	AG_Notebook *nb = p;
	AG_NotebookTab *tab;
	int x = SPACING;
	int y = SPACING;
	SDL_Rect box;

	agPrim.rect_filled(nb,
	    0, nb->bar_h,
	    AGWIDGET(nb)->w,
	    AGWIDGET(nb)->h - nb->bar_h,
	    AG_COLOR(NOTEBOOK_SEL_COLOR));

	TAILQ_FOREACH(tab, &nb->tabs, tabs) {
		box.x = x;
		box.y = y;
		box.w = AGWIDGET_SURFACE(nb,tab->label)->w + SPACING*2;
		box.h = nb->bar_h - SPACING;

		if (box.x+box.w > AGWIDGET(nb)->w)
			break;

		agPrim.box_chamfered(nb, &box,
		    nb->sel_tab==tab ? -1 : 1, nb->tab_rad,
		    nb->sel_tab==tab ?
		    AG_COLOR(NOTEBOOK_SEL_COLOR) :
		    AG_COLOR(NOTEBOOK_BG_COLOR));
		
		AG_WidgetBlitSurface(nb, tab->label, x+SPACING, y+2);
		x += box.w;
	}
}

void
AG_NotebookScale(void *p, int w, int h)
{
	AG_Notebook *nb = p;
	AG_NotebookTab *tab;
	
	pthread_mutex_lock(&nb->lock);
	if (w == -1 || h == -1) {
		nb->bar_h = agTextFontHeight + SPACING*2;
		nb->bar_w = SPACING*2;
		nb->cont_w = 0;
		nb->cont_h = 0;
		TAILQ_FOREACH(tab, &nb->tabs, tabs) {
			AGWIDGET_OPS(tab)->scale(tab, -1, -1);
			nb->cont_w = MAX(nb->cont_w,AGWIDGET(tab)->w);
			nb->cont_h = MAX(nb->cont_h,AGWIDGET(tab)->h);
			nb->bar_w += AGWIDGET_SURFACE(nb,tab->label)->w +
			    SPACING*2;
		}
		AGWIDGET(nb)->h = nb->cont_h + nb->bar_h;
		AGWIDGET(nb)->w = MAX(nb->cont_w, nb->bar_w);
	}
	if ((tab = nb->sel_tab) != NULL) {
		AGWIDGET(tab)->x = 0;
		AGWIDGET(tab)->y = nb->bar_h;
		if (nb->flags & AG_NOTEBOOK_WFILL) nb->cont_w = AGWIDGET(nb)->w;
		if (nb->flags & AG_NOTEBOOK_HFILL) nb->cont_h = AGWIDGET(nb)->h;
		AGWIDGET(tab)->w = nb->cont_w;
		AGWIDGET(tab)->h = nb->cont_h - nb->bar_h;
		AGWIDGET_OPS(tab)->scale(tab,
		    AGWIDGET(tab)->w,
		    AGWIDGET(tab)->h);
	}
	pthread_mutex_unlock(&nb->lock);
}

void
AG_NotebookSetTabAlignment(AG_Notebook *nb, enum ag_notebook_tab_alignment ta)
{
	pthread_mutex_lock(&nb->lock);
	nb->tab_align = ta;
	pthread_mutex_unlock(&nb->lock);
}

AG_NotebookTab *
AG_NotebookAddTab(AG_Notebook *nb, const char *label, enum ag_box_type btype)
{
	AG_NotebookTab *tab;

	tab = Malloc(sizeof(AG_NotebookTab), M_OBJECT);
	AG_BoxInit((AG_Box *)tab, btype, AG_BOX_WFILL|AG_BOX_HFILL);
	tab->label = AG_WidgetMapSurface(nb,
	    AG_TextRender(NULL, 9, AG_COLOR(NOTEBOOK_TXT_COLOR), label));
	TAILQ_INSERT_TAIL(&nb->tabs, tab, tabs);
	if (TAILQ_FIRST(&nb->tabs) == tab) {
		AG_NotebookSelectTab(nb, tab);
	}
	return (tab);
}

void
AG_NotebookDelTab(AG_Notebook *nb, AG_NotebookTab *tab)
{
	TAILQ_REMOVE(&nb->tabs, tab, tabs);
	AG_WidgetUnmapSurface(nb, tab->label);
	AG_ObjectDestroy(tab);
	Free(tab, M_OBJECT);
}

void
AG_NotebookSelectTab(AG_Notebook *nb, AG_NotebookTab *tab)
{
	AG_Window *pwin = AG_WidgetParentWindow(nb);

#ifdef DEBUG
	if (pwin == NULL)
		fatal("no window is attached");
#endif
	if (nb->sel_tab != NULL) {
		AG_ObjectDetach(nb->sel_tab);
	}
	if (tab == NULL) {
		nb->sel_tab = NULL;
		return;
	}
	AG_ObjectAttach(nb, tab);
	nb->sel_tab = tab;

	AGWIDGET_OPS(tab)->scale(tab, -1, -1);
	AGWIDGET_OPS(tab)->scale(tab, AGWIDGET(tab)->w, AGWIDGET(tab)->h);
	AG_NotebookScale(nb, AGWIDGET(nb)->w, AGWIDGET(nb)->h);
	AG_WidgetUpdateCoords(pwin, AGWIDGET(pwin)->x, AGWIDGET(pwin)->y);
#if 0
	AG_WidgetFocus(tab);
#endif
}
