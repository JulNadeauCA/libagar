/*	$Csoft: menu.c,v 1.3 2004/09/29 08:15:52 vedge Exp $	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
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

#include "menu.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

#include <stdarg.h>
#include <string.h>

static struct widget_ops ag_menu_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		ag_menu_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	ag_menu_draw,
	ag_menu_scale
};

enum {
	UNZEL_COLOR,
	SEL_COLOR
};

struct AGMenu *
ag_menu_new(void *parent)
{
	struct AGMenu *m;

	m = Malloc(sizeof(struct AGMenu), M_OBJECT);
	ag_menu_init(m);
	object_attach(parent, m);
	return (m);
}

void
ag_menu_expand(struct AGMenu *m, struct AGMenuItem *item, int x, int y)
{
	struct AGMenuView *mview;
	struct window *panel;

	panel = window_new(WINDOW_NO_TITLEBAR|WINDOW_NO_DECORATIONS, NULL);
	window_set_padding(panel, 0, 0);

	WIDGET(panel)->x = x;
	WIDGET(panel)->y = y;

	mview = Malloc(sizeof(struct AGMenuView), M_OBJECT);
	ag_menu_view_init(mview, panel, m, item);
	object_attach(panel, mview);
	item->view = mview;
	
	WIDGET_SCALE(panel, -1, -1);
	widget_update_coords(panel, WIDGET(panel)->x, WIDGET(panel)->y);
	window_show(panel);
}

void
ag_menu_collapse(struct AGMenu *m, struct AGMenuItem *item)
{
	int i;

	for (i = 0; i < item->nsubitems; i++) {
		if (item->subitems[i].nsubitems > 0)
			ag_menu_collapse(m, &item->subitems[i]);
	}
	item->sel_subitem = NULL;
	if (item->view != NULL &&
	    item->view->panel != NULL) {
		view_detach(item->view->panel);
		item->view = NULL;
	}
}

static void
mousebuttondown(int argc, union evarg *argv)
{
	struct AGMenu *m = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;
	int i;

	for (i = 0; i < m->nitems; i++) {
		struct AGMenuItem *mitem = &m->items[i];
		SDL_Surface *label = WIDGET_SURFACE(m,mitem->label);

		if (x >= mitem->x &&
		    x < (mitem->x + label->w + m->hspace) &&
		    y >= mitem->y &&
		    y < (mitem->y + m->itemh)) {
		    	if (m->sel_item == mitem) {
				ag_menu_collapse(m, mitem);
				m->sel_item = NULL;
				m->selecting = 0;
			} else {
				if (m->sel_item != NULL) {
					ag_menu_collapse(m, m->sel_item);
				}
				m->sel_item = mitem;
				ag_menu_expand(m, mitem,
				    WIDGET(m)->cx+mitem->x,
				    WIDGET(m)->cy+mitem->y+1+label->h);
				m->selecting = 1;
			}
			break;
		}
	}
}

static void
mousebuttonup(int argc, union evarg *argv)
{
	struct AGMenu *m = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;

#if 0
	if (m->sel_item != NULL && m->sel_subitem == NULL &&
	    x >= m->sel_item->x &&
	    x < (m->sel_item->x + WIDGET_SURFACE(m,m->sel_item->label)->w +
	        m->hspace) &&
	    y >= m->sel_item->y &&
	    y < (m->sel_item->y + m->itemh)) {
		m->sel_item = NULL;
	}
#endif
}

static void
mousemotion(int argc, union evarg *argv)
{
	struct AGMenu *m = argv[0].p;
	int x = argv[1].i;
	int y = argv[2].i;
	int i;

	if (!m->selecting || y < 0 || y >= WIDGET(m)->h-1)
		return;

	for (i = 0; i < m->nitems; i++) {
		struct AGMenuItem *mitem = &m->items[i];
		SDL_Surface *label = WIDGET_SURFACE(m,mitem->label);

		if (x >= mitem->x &&
		    x < (mitem->x + label->w + m->hspace) &&
		    y >= mitem->y &&
		    y < (mitem->y + m->itemh)) {
		    	if (mitem != m->sel_item) {
				if (m->sel_item != NULL) {
					ag_menu_collapse(m, m->sel_item);
				}
				m->sel_item = mitem;
				ag_menu_expand(m, mitem,
				    WIDGET(m)->cx+mitem->x,
				    WIDGET(m)->cy+mitem->y+1+label->h);
			}
			break;
		}
	}
	if (i == m->nitems && m->sel_item != NULL) {
		ag_menu_collapse(m, m->sel_item);
		m->sel_item = NULL;
	}
}

void
ag_menu_init(struct AGMenu *m)
{
	widget_init(m, "AGMenu", &ag_menu_ops, WIDGET_WFILL|
					       WIDGET_UNFOCUSED_MOTION|
	                                       WIDGET_UNFOCUSED_BUTTONUP);
	widget_map_color(m, UNZEL_COLOR, "unzelected", 100, 100, 100, 255);
	widget_map_color(m, SEL_COLOR, "selected", 50, 50, 120, 255);

	m->items = Malloc(sizeof(struct AGMenuItem), M_WIDGET);
	m->nitems = 0;
	m->vspace = 5;
	m->hspace = 17;
	m->selecting = 0;
	m->sel_item = NULL;
	m->itemh = text_font_height(NULL) + m->vspace;

	event_new(m, "window-mousebuttondown", mousebuttondown, NULL);
	event_new(m, "window-mousebuttonup", mousebuttonup, NULL);
	event_new(m, "window-mousemotion", mousemotion, NULL);
}

struct AGMenuItem *
ag_menu_add_item(struct AGMenu *m, const char *text)
{
	struct AGMenuItem *mitem;
	
	m->items = Realloc(m->items, (m->nitems+1)*sizeof(struct AGMenuItem));
	mitem = &m->items[m->nitems++];
	mitem->text = text;
	mitem->label = widget_map_surface(m,
	    text_render(NULL, -1, 0, text));
	mitem->icon = -1;
	mitem->key_equiv = 0;
	mitem->key_mod = 0;
	mitem->view = NULL;
	mitem->event = NULL;
	mitem->subitems = NULL;
	mitem->nsubitems = 0;
	mitem->pmenu = m;
	mitem->sel_subitem = NULL;
	mitem->pitem = NULL;
	return (mitem);
}

struct AGMenuItem *
ag_menu_add_subitem(struct AGMenuItem *pitem, const char *text,
    SDL_Surface *icon, SDLKey key_equiv, SDLMod key_mod,
    void (*fn)(int, union evarg *), const char *fmt, ...)
{
	char evname[EVENT_NAME_MAX];
	struct AGMenuItem *subitem;
	struct AGMenu *m = pitem->pmenu;
	SDL_Surface *text_su;
	va_list ap;
	
	if (pitem->subitems == NULL) {
		pitem->subitems = Malloc(sizeof(struct AGMenuItem), M_WIDGET);
	} else {
		pitem->subitems = Realloc(pitem->subitems,
		    (pitem->nsubitems+1)*sizeof(struct AGMenuItem));
	}

	text_su = text_render(NULL, -1, 0, text);

	subitem = &pitem->subitems[pitem->nsubitems++];
	subitem->text = text;
	subitem->label = widget_map_surface(m, text_su);
	subitem->icon = (icon != NULL) ?
	    widget_map_surface(m, view_copy_surface(icon)) : -1;

	subitem->key_equiv = key_equiv;
	subitem->key_mod = key_mod;
	subitem->subitems = NULL;
	subitem->nsubitems = 0;
	subitem->view = NULL;
	subitem->pmenu = m;
	subitem->sel_subitem = NULL;
	subitem->y = pitem->nsubitems*m->itemh - m->itemh;
	subitem->pitem = pitem;

	snprintf(evname, sizeof(evname), "select-%p", subitem);
	subitem->event = event_new(m, evname, fn, NULL);
	if (fmt != NULL) {
		va_start(ap, fmt);
		for (; *fmt != '\0'; fmt++) {
			EVENT_PUSH_ARG(ap, *fmt, subitem->event);
		}
		va_end(ap);
	}
	return (subitem);
}

void
ag_menu_free_subitems(struct AGMenuItem *mit)
{
	int i;

	if (mit->label != -1) {
		widget_unmap_surface(mit->pmenu, mit->label);
		mit->label = -1;
	}
	if (mit->icon != -1) {
		widget_unmap_surface(mit->pmenu, mit->icon);
		mit->icon = -1;
	}
	for (i = 0; i < mit->nsubitems; i++)
		ag_menu_free_subitems(&mit->subitems[i]);

	Free(mit->subitems, M_WIDGET);
	mit->subitems = NULL;
	mit->nsubitems = 0;
}

void
ag_menu_free_items(struct AGMenu *m)
{
	int i;

	for (i = 0; i < m->nitems; i++) {
		ag_menu_free_subitems(&m->items[i]);
	}
	m->nitems = 0;
}

void
ag_menu_destroy(void *p)
{
	struct AGMenu *m = p;

	ag_menu_free_items(m);
	Free(m->items, M_WIDGET);
	widget_destroy(m);
}

void
ag_menu_draw(void *p)
{
	struct AGMenu *m = p;
	int i;

	if (WIDGET(m)->w < m->hspace*2 ||
	    WIDGET(m)->h < m->vspace*2)
		return;

	primitives.box(m, 0, 0, WIDGET(m)->w, WIDGET(m)->h, 1, UNZEL_COLOR);
	
	for (i = 0; i < m->nitems; i++) {
		struct AGMenuItem *mitem = &m->items[i];
		SDL_Surface *label = WIDGET_SURFACE(m, mitem->label);

		if (mitem == m->sel_item) {
			primitives.rect_filled(m,
			    mitem->x,
			    mitem->y - m->vspace/2,
			    label->w + m->hspace,
			    m->itemh - 1,
			    SEL_COLOR);
		}
		widget_blit_surface(m, mitem->label,
		    mitem->x + m->hspace/2,
		    mitem->y + m->vspace/2);
	}
}

void
ag_menu_scale(void *p, int w, int h)
{
	struct AGMenu *m = p;

	if (w == -1 && h == -1) {
		int x, y;
		int i;

		x = WIDGET(m)->w = m->hspace/2;
		y = WIDGET(m)->h = m->vspace/2;

		for (i = 0; i < m->nitems; i++) {
			struct AGMenuItem *mitem = &m->items[i];
			SDL_Surface *label = WIDGET_SURFACE(m, mitem->label);

			mitem->x = x;
			mitem->y = y;

			x += label->w+m->hspace;
			if (WIDGET(m)->h < label->h) {
				WIDGET(m)->h += m->itemh;
			}
			if (x > view->w/2) {
				x = m->hspace;			/* Wrap */
				y += m->itemh;
				WIDGET(m)->h += label->h;
			} else {
				WIDGET(m)->w += label->w + m->hspace;
			}
		}
	}
}

