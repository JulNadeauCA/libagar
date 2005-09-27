/*	$Csoft: menu.c,v 1.23 2005/09/20 10:21:04 vedge Exp $	*/

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

static AG_WidgetOps menu_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		AG_MenuDestroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	AG_MenuDraw,
	AG_MenuScale
};

AG_Menu *
AG_MenuNew(void *parent)
{
	AG_Menu *m;

	m = Malloc(sizeof(AG_Menu), M_OBJECT);
	AG_MenuInit(m);
	AG_ObjectAttach(parent, m);
	return (m);
}

AG_Window *
AG_MenuExpand(AG_Menu *m, AG_MenuItem *item, int x, int y)
{
	AG_MenuView *mview;
	AG_Window *panel;

	if (item->nsubitems == 0)
		return (NULL);

	panel = AG_WindowNew(AG_WINDOW_NO_TITLEBAR|AG_WINDOW_NO_DECORATIONS,
	    NULL);
	AG_WindowSetCaption(panel, "win-popup");
	AG_WindowSetPadding(panel, 0, 0, 0);

	AGWIDGET(panel)->x = x;
	AGWIDGET(panel)->y = y;

	mview = Malloc(sizeof(AG_MenuView), M_OBJECT);
	AG_MenuViewInit(mview, panel, m, item);
	AG_ObjectAttach(panel, mview);
	item->view = mview;

	AGWIDGET_SCALE(panel, -1, -1);
	AG_WINDOW_UPDATE(panel);
	AG_WindowShow(panel);
	return (panel);
}

void
AG_MenuCollapse(AG_Menu *m, AG_MenuItem *item)
{
	int i;

	for (i = 0; i < item->nsubitems; i++) {
		if (item->subitems[i].nsubitems > 0)
			AG_MenuCollapse(m, &item->subitems[i]);
	}
	item->sel_subitem = NULL;
	if (item->view != NULL &&
	    item->view->panel != NULL) {
		AG_ViewDetach(item->view->panel);
		item->view = NULL;
	}
}

static void
mousebuttondown(int argc, union evarg *argv)
{
	AG_Menu *m = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;
	int i;

	for (i = 0; i < m->nitems; i++) {
		AG_MenuItem *mitem = &m->items[i];
		SDL_Surface *label = AGWIDGET_SURFACE(m,mitem->label);

		if (x >= mitem->x &&
		    x < (mitem->x + label->w + m->hspace) &&
		    y >= mitem->y &&
		    y < (mitem->y + m->itemh)) {
		    	if (m->sel_item == mitem) {
				AG_MenuCollapse(m, mitem);
				m->sel_item = NULL;
				m->selecting = 0;
			} else {
				if (m->sel_item != NULL) {
					AG_MenuCollapse(m, m->sel_item);
				}
				m->sel_item = mitem;
				AG_MenuExpand(m, mitem,
				    AGWIDGET(m)->cx+mitem->x,
				    AGWIDGET(m)->cy+mitem->y+1+label->h);
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
	AG_Menu *m = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;

	if (m->sel_item != NULL && m->sel_subitem == NULL &&
	    x >= m->sel_item->x &&
	    x < (m->sel_item->x + AGWIDGET_SURFACE(m,m->sel_item->label)->w +
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
	AG_Menu *m = argv[0].p;
	int x = argv[1].i;
	int y = argv[2].i;
	int i;

	if (!m->selecting || y < 0 || y >= AGWIDGET(m)->h-1)
		return;

	if (x < 0 && m->sel_item != NULL) {
		AG_MenuCollapse(m, m->sel_item);
		m->sel_item = NULL;
		return;
	}

	for (i = 0; i < m->nitems; i++) {
		AG_MenuItem *mitem = &m->items[i];
		SDL_Surface *label = AGWIDGET_SURFACE(m,mitem->label);

		if (x >= mitem->x &&
		    x < (mitem->x + label->w + m->hspace) &&
		    y >= mitem->y &&
		    y < (mitem->y + m->itemh)) {
		    	if (mitem != m->sel_item) {
				if (m->sel_item != NULL) {
					AG_MenuCollapse(m, m->sel_item);
				}
				m->sel_item = mitem;
				AG_MenuExpand(m, mitem,
				    AGWIDGET(m)->cx+mitem->x,
				    AGWIDGET(m)->cy+mitem->y+1+label->h);
			}
			break;
		}
	}
#if 0
	if (i == m->nitems && m->sel_item != NULL) {
		AG_MenuCollapse(m, m->sel_item);
		m->sel_item = NULL;
	}
#endif
}

static void
attached(int argc, union evarg *argv)
{
	AG_Menu *m = argv[0].p;
	AG_Widget *pwid = argv[argc].p;
	AG_Window *pwin;

	/* Adjust the top padding of the parent window if any. */
	if ((pwin = AG_WidgetParentWindow(pwid)) != NULL)
		AG_WindowSetPadding(pwin, pwin->xpadding, 0,
		    pwin->ypadding_bot);
}

void
AG_MenuInit(AG_Menu *m)
{
	AG_WidgetInit(m, "AGMenu", &menu_ops, AG_WIDGET_WFILL|
					       AG_WIDGET_UNFOCUSED_MOTION|
	                                       AG_WIDGET_UNFOCUSED_BUTTONUP);

	m->items = Malloc(sizeof(AG_MenuItem), M_WIDGET);
	m->nitems = 0;
	m->vspace = 5;
	m->hspace = 17;
	m->selecting = 0;
	m->sel_item = NULL;
	m->itemh = agTextFontHeight + m->vspace;

	AG_SetEvent(m, "window-mousebuttondown", mousebuttondown, NULL);
#if 0
	AG_SetEvent(m, "window-mousebuttonup", mousebuttonup, NULL);
#endif
	AG_SetEvent(m, "window-mousemotion", mousemotion, NULL);
	AG_SetEvent(m, "attached", attached, NULL);
}

AG_MenuItem *
AG_MenuAddItem(AG_Menu *m, const char *text)
{
	AG_MenuItem *mitem;
	
	m->items = Realloc(m->items, (m->nitems+1)*sizeof(AG_MenuItem));
	mitem = &m->items[m->nitems++];
	mitem->text = text;
	mitem->label = (text != NULL) ?
	               AG_WidgetMapSurface(m, AG_TextRender(NULL, -1,
		           AG_COLOR(MENU_TXT_COLOR), text)) : -1;
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

static __inline__ AG_MenuItem *
add_subitem(AG_MenuItem *pitem, const char *text, SDL_Surface *icon,
    SDLKey key_equiv, SDLKey key_mod)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;
	
	if (pitem->subitems == NULL) {
		pitem->subitems = Malloc(sizeof(AG_MenuItem), M_WIDGET);
	} else {
		pitem->subitems = Realloc(pitem->subitems,
		    (pitem->nsubitems+1)*sizeof(AG_MenuItem));
	}
	mi = &pitem->subitems[pitem->nsubitems++];
	mi->pitem = pitem;
	mi->pmenu = m;
	mi->y = pitem->nsubitems*m->itemh - m->itemh;
	mi->key_equiv = key_equiv;
	mi->key_mod = key_mod;
	mi->subitems = NULL;
	mi->nsubitems = 0;
	mi->view = NULL;
	mi->pmenu = m;
	mi->sel_subitem = NULL;
	mi->pitem = NULL;
	mi->key_equiv = key_equiv;
	mi->key_mod = key_mod;
	mi->onclick = NULL;
	mi->poll = NULL;
	mi->bind_type = AG_MENU_NO_BINDING;
	mi->bind_flags = 0;
	mi->bind_invert = 0;
	mi->bind_lock = NULL;
	mi->text = text;
	mi->icon = (icon != NULL) ?
	    AG_WidgetMapSurface(m, AG_DupSurface(icon)) : -1;
	mi->label = (text != NULL) ?
	    AG_WidgetMapSurface(m, AG_TextRender(NULL, -1,
	    AG_COLOR(MENU_TXT_COLOR), text)) : -1;
	mi->state = -1;
	return (mi);
}

void
AG_MenuSetIcon(AG_MenuItem *mi, SDL_Surface *icon)
{
	if (mi->icon == -1) {
		mi->icon = (icon != NULL) ?
		    AG_WidgetMapSurface(mi->pmenu, AG_DupSurface(icon)) : -1;
	} else {
		AG_WidgetReplaceSurface(mi->pmenu, mi->icon,
		    icon != NULL ? AG_DupSurface(icon) : NULL);
	}
}

void
AG_MenuSetLabel(AG_MenuItem *mi, const char *text)
{
	if (mi->label == -1) {
		mi->label = (text != NULL) ?
		    AG_WidgetMapSurface(mi->pmenu,
		       AG_TextRender(NULL, -1, AG_COLOR(MENU_TXT_COLOR),
		       text)) : -1;
	} else {
		AG_WidgetReplaceSurface(mi->pmenu, mi->label,
		    AG_TextRender(NULL, -1, AG_COLOR(MENU_TXT_COLOR), text));
	}
}

AG_MenuItem *
AG_MenuSeparator(AG_MenuItem *pitem)
{
	return (add_subitem(pitem, NULL, NULL, 0, 0));
}

AG_MenuItem *
AG_MenuDynamic(AG_MenuItem *pitem, int nicon,
    void (*poll_fn)(int, union evarg *), const char *fmt, ...)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;
	va_list ap;

	mi = add_subitem(pitem, NULL, nicon >= 0 ? AGICON(nicon) : NULL, 0, 0);
	mi->poll = AG_SetEvent(m, NULL, poll_fn, NULL);
	if (fmt != NULL) {
		va_start(ap, fmt);
		for (; *fmt != '\0'; fmt++) {
			AG_EVENT_PUSH_ARG(ap, *fmt, mi->poll);
		}
		va_end(ap);
	}
	return (mi);
}

AG_MenuItem *
AG_MenuAction(AG_MenuItem *pitem, const char *text, int nicon,
    void (*fn)(int, union evarg *), const char *fmt, ...)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;
	va_list ap;

	mi = add_subitem(pitem, text, nicon >= 0 ? AGICON(nicon) : NULL, 0, 0);
	mi->onclick = AG_SetEvent(m, NULL, fn, NULL);
	if (fmt != NULL) {
		va_start(ap, fmt);
		for (; *fmt != '\0'; fmt++) {
			AG_EVENT_PUSH_ARG(ap, *fmt, mi->onclick);
		}
		va_end(ap);
	}
	return (mi);
}

AG_MenuItem *
AG_MenuActionKb(AG_MenuItem *pitem, const char *text,
    int nicon, SDLKey key_equiv, SDLMod key_mod,
    void (*fn)(int, union evarg *), const char *fmt, ...)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;
	va_list ap;

	mi = add_subitem(pitem, text, nicon >= 0 ? AGICON(nicon) : NULL,
	    key_equiv, key_mod);
	mi->onclick = AG_SetEvent(m, NULL, fn, NULL);
	if (fmt != NULL) {
		va_start(ap, fmt);
		for (; *fmt != '\0'; fmt++) {
			AG_EVENT_PUSH_ARG(ap, *fmt, mi->onclick);
		}
		va_end(ap);
	}
	return (mi);
}

AG_MenuItem *
AG_MenuTool(AG_MenuItem *pitem, AG_Toolbar *tbar,
    const char *text, int icon, SDLKey key_equiv, SDLMod key_mod,
    void (*fn)(int, union evarg *), const char *fmt, ...)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;
	AG_Button *bu;
	AG_Event *ev;
	va_list ap;
	const char *fmtp;
	
	bu = AG_ButtonNew(tbar->rows[0], NULL);
	AG_ButtonSetSurface(bu, AGICON(icon));
	AG_ButtonSetFocusable(bu, 0);

	mi = add_subitem(pitem, text, AGICON(icon), key_equiv, key_mod);
	
	mi->onclick = AG_SetEvent(m, NULL, fn, NULL);
	ev = AG_SetEvent(bu, "button-pushed", fn, NULL);
	if (fmt != NULL) {
		va_start(ap, fmt);
		for (fmtp = fmt; *fmtp != '\0'; fmtp++) {
			AG_EVENT_PUSH_ARG(ap, *fmtp, mi->onclick);
		}
		va_end(ap);
		va_start(ap, fmt);
		for (fmtp = fmt; *fmtp != '\0'; fmtp++) {
			AG_EVENT_PUSH_ARG(ap, *fmtp, ev);
		}
		va_end(ap);
	}
	return (mi);
}

AG_MenuItem *
AG_MenuIntBoolMp(AG_MenuItem *pitem, const char *text, int nicon,
    int *boolp, int inv, pthread_mutex_t *lock)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;

	mi = add_subitem(pitem, text, nicon >= 0 ? AGICON(nicon) : NULL, 0, 0);
	mi->bind_type = AG_MENU_INT_BOOL;
	mi->bind_p = (void *)boolp;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	return (mi);
}

AG_MenuItem *
AG_MenuInt8BoolMp(AG_MenuItem *pitem, const char *text, int nicon,
    Uint8 *boolp, int inv, pthread_mutex_t *lock)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;

	mi = add_subitem(pitem, text, nicon >= 0 ? AGICON(nicon) : NULL, 0, 0);
	mi->bind_type = AG_MENU_INT8_BOOL;
	mi->bind_p = (void *)boolp;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	return (mi);
}

AG_MenuItem *
AG_MenuIntFlagsMp(AG_MenuItem *pitem, const char *text, int nicon,
    int *flagsp, int flags, int inv, pthread_mutex_t *lock)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;

	mi = add_subitem(pitem, text, nicon >= 0 ? AGICON(nicon) : NULL, 0, 0);
	mi->bind_type = AG_MENU_INT_FLAGS;
	mi->bind_p = (void *)flagsp;
	mi->bind_flags = flags;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	return (mi);
}

AG_MenuItem *
AG_MenuInt8FlagsMp(AG_MenuItem *pitem, const char *text, int nicon, 
    Uint8 *flagsp, Uint8 flags, int inv, pthread_mutex_t *lock)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;

	mi = add_subitem(pitem, text, nicon >= 0 ? AGICON(nicon) : NULL, 0, 0);
	mi->bind_type = AG_MENU_INT8_FLAGS;
	mi->bind_p = (void *)flagsp;
	mi->bind_flags = flags;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	return (mi);
}

AG_MenuItem *
AG_MenuInt16FlagsMp(AG_MenuItem *pitem, const char *text, int nicon,
    Uint16 *flagsp, Uint16 flags, int inv, pthread_mutex_t *lock)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;

	mi = add_subitem(pitem, text, nicon >= 0 ? AGICON(nicon) : NULL, 0, 0);
	mi->bind_type = AG_MENU_INT16_FLAGS;
	mi->bind_p = (void *)flagsp;
	mi->bind_flags = flags;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	return (mi);
}

AG_MenuItem *
AG_MenuInt32FlagsMp(AG_MenuItem *pitem, const char *text, int nicon, 
    Uint32 *flagsp, Uint32 flags, int inv, pthread_mutex_t *lock)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;

	mi = add_subitem(pitem, text, nicon >= 0 ? AGICON(nicon) : NULL, 0, 0);
	mi->bind_type = AG_MENU_INT32_FLAGS;
	mi->bind_p = (void *)flagsp;
	mi->bind_flags = flags;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	return (mi);
}

void
AG_MenuFreeSubItems(AG_MenuItem *mit)
{
	int i;

	if (mit->label != -1) {
		AG_WidgetUnmapSurface(mit->pmenu, mit->label);
		mit->label = -1;
	}
	if (mit->icon != -1) {
		AG_WidgetUnmapSurface(mit->pmenu, mit->icon);
		mit->icon = -1;
	}
	for (i = 0; i < mit->nsubitems; i++)
		AG_MenuFreeSubItems(&mit->subitems[i]);

	Free(mit->subitems, M_WIDGET);
	mit->subitems = NULL;
	mit->nsubitems = 0;
}

void
AG_MenuFreeItems(AG_Menu *m)
{
	int i;

	for (i = 0; i < m->nitems; i++) {
		AG_MenuFreeSubItems(&m->items[i]);
	}
	m->nitems = 0;
}

void
AG_MenuDestroy(void *p)
{
	AG_Menu *m = p;

	AG_MenuFreeItems(m);
	Free(m->items, M_WIDGET);
	AG_WidgetDestroy(m);
}

void
AG_MenuDraw(void *p)
{
	AG_Menu *m = p;
	int i;

	if (AGWIDGET(m)->w < m->hspace*2 ||
	    AGWIDGET(m)->h < m->vspace*2)
		return;

	agPrim.box(m, 0, 0, AGWIDGET(m)->w, AGWIDGET(m)->h, 1,
	    AG_COLOR(MENU_UNSEL_COLOR));
	
	for (i = 0; i < m->nitems; i++) {
		AG_MenuItem *mitem = &m->items[i];
		SDL_Surface *label = AGWIDGET_SURFACE(m, mitem->label);

		if (mitem == m->sel_item) {
			agPrim.rect_filled(m,
			    mitem->x,
			    mitem->y - m->vspace/2,
			    label->w + m->hspace,
			    m->itemh - 1,
			    AG_COLOR(MENU_SEL_COLOR));
		}
		AG_WidgetBlitSurface(m, mitem->label,
		    mitem->x + m->hspace/2,
		    mitem->y + m->vspace/2);
	}
}

void
AG_MenuScale(void *p, int w, int h)
{
	AG_Menu *m = p;

	if (w == -1 && h == -1) {
		int x, y;
		int i;

		x = AGWIDGET(m)->w = m->hspace/2;
		y = AGWIDGET(m)->h = m->vspace/2;

		for (i = 0; i < m->nitems; i++) {
			AG_MenuItem *mitem = &m->items[i];
			SDL_Surface *label = AGWIDGET_SURFACE(m, mitem->label);

			mitem->x = x;
			mitem->y = y;

			x += label->w+m->hspace;
			if (AGWIDGET(m)->h < label->h) {
				AGWIDGET(m)->h += m->itemh;
			}
			if (x > agView->w/2) {
				x = m->hspace/2;		/* Wrap */
				y += m->itemh;
				AGWIDGET(m)->h += label->h + m->vspace;
				AGWIDGET(m)->w += m->hspace;
			} else {
				AGWIDGET(m)->w += label->w + m->hspace;
			}
		}
	}
}

