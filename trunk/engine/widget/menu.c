/*	$Csoft: menu.c,v 1.10 2005/01/31 08:32:48 vedge Exp $	*/

/*
 * Copyright (c) 2004, 2005 CubeSoft Communications, Inc.
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
	SEL_COLOR,
	SEL_OPTION_COLOR,
	SEPARATOR1_COLOR,
	SEPARATOR2_COLOR
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

struct window *
ag_menu_expand(struct AGMenu *m, struct AGMenuItem *item, int x, int y)
{
	struct AGMenuView *mview;
	struct window *panel;

	if (item->nsubitems == 0)
		return (NULL);

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
	return (panel);
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

#if 0
static void
mousebuttonup(int argc, union evarg *argv)
{
	struct AGMenu *m = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;

	if (m->sel_item != NULL && m->sel_subitem == NULL &&
	    x >= m->sel_item->x &&
	    x < (m->sel_item->x + WIDGET_SURFACE(m,m->sel_item->label)->w +
	        m->hspace) &&
	    y >= m->sel_item->y &&
	    y < (m->sel_item->y + m->itemh)) {
		m->sel_item = NULL;
	}
}
#endif

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
	widget_map_color(m, SEL_OPTION_COLOR, "sel-option", 70, 70, 70, 255);
	widget_map_color(m, SEPARATOR1_COLOR, "separator1", 66, 85, 82, 255);
	widget_map_color(m, SEPARATOR2_COLOR, "separator2", 148, 150, 148, 255);

	m->items = Malloc(sizeof(struct AGMenuItem), M_WIDGET);
	m->nitems = 0;
	m->vspace = 5;
	m->hspace = 17;
	m->selecting = 0;
	m->sel_item = NULL;
	m->itemh = text_font_height + m->vspace;

	event_new(m, "window-mousebuttondown", mousebuttondown, NULL);
#if 0
	event_new(m, "window-mousebuttonup", mousebuttonup, NULL);
#endif
	event_new(m, "window-mousemotion", mousemotion, NULL);
}

struct AGMenuItem *
ag_menu_add_item(struct AGMenu *m, const char *text)
{
	struct AGMenuItem *mitem;
	
	m->items = Realloc(m->items, (m->nitems+1)*sizeof(struct AGMenuItem));
	mitem = &m->items[m->nitems++];
	mitem->text = text;
	mitem->label = (text != NULL) ?
	               widget_map_surface(m, text_render(NULL, -1, 0, text)) :
		       -1;
	mitem->icon = -1;
	mitem->key_equiv = 0;
	mitem->key_mod = 0;
	mitem->view = NULL;
	mitem->onclick = NULL;
	mitem->subitems = NULL;
	mitem->nsubitems = 0;
	mitem->pmenu = m;
	mitem->sel_subitem = NULL;
	mitem->pitem = NULL;
	return (mitem);
}

static void
init_menuitem(struct AGMenuItem *mi, const char *text, SDL_Surface *icon,
    SDLKey key_equiv, SDLKey key_mod)
{
	struct AGMenu *m = mi->pmenu;

	mi->text = text;
	mi->label = (text != NULL) ?
	    widget_map_surface(m, text_render(NULL, -1, 0, text)) : -1;

	if (icon != NULL) {
		SDL_Surface *sicon = NULL;

		view_scale_surface(icon, m->itemh-1, m->itemh-1, &sicon);
		mi->icon = widget_map_surface(m, sicon);
	} else {
		mi->icon = -1;
	}
	mi->key_equiv = key_equiv;
	mi->key_mod = key_mod;
	mi->subitems = NULL;
	mi->nsubitems = 0;
	mi->view = NULL;
	mi->pmenu = m;
	mi->sel_subitem = NULL;
	mi->pitem = NULL;
	mi->y = 0;
	mi->key_equiv = key_equiv;
	mi->key_mod = key_mod;
	mi->onclick = NULL;

	mi->bind_type = MENU_NO_BINDING;
	mi->bind_flags = 0;
	mi->bind_invert = 0;
	mi->bind_lock = NULL;
}

static __inline__ struct AGMenuItem *
add_subitem(struct AGMenuItem *pitem, const char *text, SDL_Surface *icon,
    SDLKey key_equiv, SDLKey key_mod)
{
	struct AGMenu *m = pitem->pmenu;
	struct AGMenuItem *mi;
	
	if (pitem->subitems == NULL) {
		pitem->subitems = Malloc(sizeof(struct AGMenuItem), M_WIDGET);
	} else {
		pitem->subitems = Realloc(pitem->subitems,
		    (pitem->nsubitems+1)*sizeof(struct AGMenuItem));
	}
	mi = &pitem->subitems[pitem->nsubitems++];
	mi->pitem = pitem;
	mi->pmenu = m;
	mi->y = pitem->nsubitems*m->itemh - m->itemh;
	init_menuitem(mi, text, icon, key_equiv, key_mod);
	return (mi);

}

struct AGMenuItem *
ag_menu_separator(struct AGMenuItem *pitem)
{
	return (add_subitem(pitem, NULL, NULL, 0, 0));
}

struct AGMenuItem *
ag_menu_action(struct AGMenuItem *pitem, const char *text,
    SDL_Surface *icon, SDLKey key_equiv, SDLMod key_mod,
    void (*fn)(int, union evarg *), const char *fmt, ...)
{
	struct AGMenu *m = pitem->pmenu;
	struct AGMenuItem *mi;
	va_list ap;

	mi = add_subitem(pitem, text, icon, key_equiv, key_mod);
	mi->onclick = event_new(m, NULL, fn, NULL);
	if (fmt != NULL) {
		va_start(ap, fmt);
		for (; *fmt != '\0'; fmt++) {
			EVENT_PUSH_ARG(ap, *fmt, mi->onclick);
		}
		va_end(ap);
	}
	return (mi);
}

struct AGMenuItem *
ag_menu_int_bool(struct AGMenuItem *pitem, const char *text, SDL_Surface *icon,
    SDLKey key_equiv, SDLMod key_mod,
    int *boolp, pthread_mutex_t *lock, int inv)
{
	struct AGMenu *m = pitem->pmenu;
	struct AGMenuItem *mi;

	mi = add_subitem(pitem, text, icon, key_equiv, key_mod);
	mi->bind_type = MENU_INT_BOOL;
	mi->bind_p = (void *)boolp;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	return (mi);
}

struct AGMenuItem *
ag_menu_int8_bool(struct AGMenuItem *pitem, const char *text, SDL_Surface *icon,
    SDLKey key_equiv, SDLMod key_mod,
    Uint8 *boolp, pthread_mutex_t *lock, int inv)
{
	struct AGMenu *m = pitem->pmenu;
	struct AGMenuItem *mi;

	mi = add_subitem(pitem, text, icon, key_equiv, key_mod);
	mi->bind_type = MENU_INT8_BOOL;
	mi->bind_p = (void *)boolp;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	return (mi);
}

struct AGMenuItem *
ag_menu_int_flags(struct AGMenuItem *pitem, const char *text, SDL_Surface *icon,
    SDLKey key_equiv, SDLMod key_mod,
    int *flagsp, int flags, pthread_mutex_t *lock, int inv)
{
	struct AGMenu *m = pitem->pmenu;
	struct AGMenuItem *mi;

	mi = add_subitem(pitem, text, icon, key_equiv, key_mod);
	mi->bind_type = MENU_INT_FLAGS;
	mi->bind_p = (void *)flagsp;
	mi->bind_flags = flags;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	return (mi);
}

struct AGMenuItem *
ag_menu_int8_flags(struct AGMenuItem *pitem, const char *text,
    SDL_Surface *icon, SDLKey key_equiv, SDLMod key_mod,
    Uint8 *flagsp, Uint8 flags, pthread_mutex_t *lock, int inv)
{
	struct AGMenu *m = pitem->pmenu;
	struct AGMenuItem *mi;

	mi = add_subitem(pitem, text, icon, key_equiv, key_mod);
	mi->bind_type = MENU_INT8_FLAGS;
	mi->bind_p = (void *)flagsp;
	mi->bind_flags = flags;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	return (mi);
}

struct AGMenuItem *
ag_menu_int16_flags(struct AGMenuItem *pitem, const char *text,
    SDL_Surface *icon, SDLKey key_equiv, SDLMod key_mod,
    Uint16 *flagsp, Uint16 flags, pthread_mutex_t *lock, int inv)
{
	struct AGMenu *m = pitem->pmenu;
	struct AGMenuItem *mi;

	mi = add_subitem(pitem, text, icon, key_equiv, key_mod);
	mi->bind_type = MENU_INT16_FLAGS;
	mi->bind_p = (void *)flagsp;
	mi->bind_flags = flags;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	return (mi);
}

struct AGMenuItem *
ag_menu_int32_flags(struct AGMenuItem *pitem, const char *text,
    SDL_Surface *icon, SDLKey key_equiv, SDLMod key_mod,
    Uint32 *flagsp, Uint32 flags, pthread_mutex_t *lock, int inv)
{
	struct AGMenu *m = pitem->pmenu;
	struct AGMenuItem *mi;

	mi = add_subitem(pitem, text, icon, key_equiv, key_mod);
	mi->bind_type = MENU_INT32_FLAGS;
	mi->bind_p = (void *)flagsp;
	mi->bind_flags = flags;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	return (mi);
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
				x = m->hspace/2;		/* Wrap */
				y += m->itemh;
				WIDGET(m)->h += label->h + m->vspace;
				WIDGET(m)->w += m->hspace;
			} else {
				WIDGET(m)->w += label->w + m->hspace;
			}
		}
	}
}

