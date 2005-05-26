/*	$Csoft: notebook.c,v 1.4 2005/05/13 09:21:47 vedge Exp $	*/

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

static struct widget_ops notebook_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		notebook_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	notebook_draw,
	notebook_scale
};

#define SPACING 8

struct notebook *
notebook_new(void *parent, int flags)
{
	struct notebook *nb;

	nb = Malloc(sizeof(struct notebook), M_OBJECT);
	notebook_init(nb, flags);
	object_attach(parent, nb);
	return (nb);
}

static void
mousebuttondown(int argc, union evarg *argv)
{
	struct notebook *nb = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;

	if (y <= nb->bar_h) {
		struct notebook_tab *tab;
		int tx = SPACING;

		TAILQ_FOREACH(tab, &nb->tabs, tabs) {
			SDL_Surface *label = WIDGET_SURFACE(nb,tab->label);
	
			if (x >= tx && x < tx+(label->w + SPACING*2)) {
				notebook_select_tab(nb, tab);
				break;
			}
			tx += label->w + SPACING*2;
		}
	}
}

void
notebook_init(struct notebook *nb, int flags)
{
	widget_init(nb, "notebook", &notebook_ops, 0);

	nb->flags = flags;
	nb->tab_align = NOTEBOOK_TABS_TOP;
	nb->sel_tab = NULL;
	nb->bar_w = -1;
	nb->bar_h = -1;
	nb->cont_w = -1;
	nb->cont_h = -1;
	nb->tab_rad = (int)(text_font_height/1.5);
	pthread_mutex_init(&nb->lock, &recursive_mutexattr);
	TAILQ_INIT(&nb->tabs);

	if (flags & NOTEBOOK_WFILL)	WIDGET(nb)->flags |= WIDGET_WFILL;
	if (flags & NOTEBOOK_HFILL)	WIDGET(nb)->flags |= WIDGET_HFILL;

	event_new(nb, "window-mousebuttondown", mousebuttondown, NULL);
}

void
notebook_destroy(void *p)
{
	struct notebook *nb = p;

	pthread_mutex_destroy(&nb->lock);
}

void
notebook_draw(void *p)
{
	struct notebook *nb = p;
	struct notebook_tab *tab;
	int x = SPACING;
	int y = SPACING;
	SDL_Rect box;

	primitives.rect_filled(nb,
	    0, nb->bar_h,
	    WIDGET(nb)->w,
	    WIDGET(nb)->h - nb->bar_h,
	    COLOR(NOTEBOOK_SEL_COLOR));

	TAILQ_FOREACH(tab, &nb->tabs, tabs) {
		box.x = x;
		box.y = y;
		box.w = WIDGET_SURFACE(nb,tab->label)->w + SPACING*2;
		box.h = nb->bar_h - SPACING;
		primitives.box_chamfered(nb, &box,
		    nb->sel_tab==tab ? -1 : 1, nb->tab_rad,
		    nb->sel_tab==tab ?
		    COLOR(NOTEBOOK_SEL_COLOR) :
		    COLOR(NOTEBOOK_BG_COLOR));
		
		widget_blit_surface(nb, tab->label, x+SPACING, y+2);
		x += box.w;
	}
}

void
notebook_scale(void *p, int w, int h)
{
	struct notebook *nb = p;
	struct notebook_tab *tab;
	
	pthread_mutex_lock(&nb->lock);
	if (w == -1 || h == -1) {
		nb->bar_h = text_font_height + SPACING*2;
		nb->bar_w = SPACING*2;
		nb->cont_w = 0;
		nb->cont_h = 0;
		TAILQ_FOREACH(tab, &nb->tabs, tabs) {
			WIDGET_OPS(tab)->scale(tab, -1, -1);
			nb->cont_w = MAX(nb->cont_w,WIDGET(tab)->w);
			nb->cont_h = MAX(nb->cont_h,WIDGET(tab)->h);
			nb->bar_w += WIDGET_SURFACE(nb,tab->label)->w +
			    SPACING*2;
		}
		WIDGET(nb)->h = nb->cont_h + nb->bar_h;
		WIDGET(nb)->w = MAX(nb->cont_w, nb->bar_w);
	}
	if ((tab = nb->sel_tab) != NULL) {
		WIDGET(tab)->x = 0;
		WIDGET(tab)->y = nb->bar_h;
		if (nb->flags & NOTEBOOK_WFILL) nb->cont_w = WIDGET(nb)->w;
		if (nb->flags & NOTEBOOK_HFILL) nb->cont_h = WIDGET(nb)->h;
		WIDGET(tab)->w = nb->cont_w;
		WIDGET(tab)->h = nb->cont_h - nb->bar_h;
		WIDGET_OPS(tab)->scale(tab, WIDGET(tab)->w, WIDGET(tab)->h);
	}
	pthread_mutex_unlock(&nb->lock);
}

void
notebook_set_tab_alignment(struct notebook *nb, enum notebook_tab_alignment ta)
{
	pthread_mutex_lock(&nb->lock);
	nb->tab_align = ta;
	pthread_mutex_unlock(&nb->lock);
}

struct notebook_tab *
notebook_add_tab(struct notebook *nb, const char *label, enum box_type btype)
{
	struct notebook_tab *tab;

	tab = Malloc(sizeof(struct notebook_tab), M_OBJECT);
	box_init((struct box *)tab, btype, BOX_WFILL|BOX_HFILL);
	tab->label = widget_map_surface(nb,
	    text_render(NULL, -1, COLOR(NOTEBOOK_TXT_COLOR), label));
	TAILQ_INSERT_TAIL(&nb->tabs, tab, tabs);
	if (TAILQ_FIRST(&nb->tabs) == tab) {
		notebook_select_tab(nb, tab);
	}
	return (tab);
}

void
notebook_del_tab(struct notebook *nb, struct notebook_tab *tab)
{
	TAILQ_REMOVE(&nb->tabs, tab, tabs);
	widget_unmap_surface(nb, tab->label);
	object_destroy(tab);
	Free(tab, M_OBJECT);
}

void
notebook_select_tab(struct notebook *nb, struct notebook_tab *tab)
{
	struct window *pwin = widget_parent_window(nb);

#ifdef DEBUG
	if (pwin == NULL)
		fatal("no window is attached");
#endif
	if (nb->sel_tab != NULL) {
		object_detach(nb->sel_tab);
	}
	if (tab == NULL) {
		nb->sel_tab = NULL;
		return;
	}
	object_attach(nb, tab);
	nb->sel_tab = tab;

	WIDGET_OPS(tab)->scale(tab, -1, -1);
	WIDGET_OPS(tab)->scale(tab, WIDGET(tab)->w, WIDGET(tab)->h);
	notebook_scale(nb, WIDGET(nb)->w, WIDGET(nb)->h);
	widget_update_coords(pwin, WIDGET(pwin)->x, WIDGET(pwin)->y);
#if 0
	widget_focus(tab);
#endif
}
