/*
 * Copyright (c) 2004-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "menu.h"

#include "primitive.h"
#include "label.h"

#include <stdarg.h>
#include <string.h>

AG_Menu *agAppMenu = NULL;
AG_Window *agAppMenuWin = NULL;
AG_Mutex agAppMenuLock = AG_MUTEX_INITIALIZER;

AG_Menu *
AG_MenuNew(void *parent, Uint flags)
{
	AG_Menu *m;

	m = Malloc(sizeof(AG_Menu));
	AG_ObjectInit(m, &agMenuClass);

	m->flags |= flags;

	if (flags & AG_MENU_HFILL) { AG_ExpandHoriz(m); }
	if (flags & AG_MENU_VFILL) { AG_ExpandVert(m); }

	AG_ObjectAttach(parent, m);
	return (m);
}

AG_Menu *
AG_MenuNewGlobal(Uint flags)
{
	AG_Menu *m;

	m = Malloc(sizeof(AG_Menu));
	AG_ObjectInit(m, &agMenuClass);

	m->flags |= flags;

	AG_MutexLock(&agAppMenuLock);

	if (agAppMenu != NULL) {
		AG_ViewDetach(agAppMenuWin);
		AG_ObjectDestroy(agAppMenu);
	}
	
	m->flags |= AG_MENU_GLOBAL;
	AG_MenuSetPadding(m, 4, 4, -1, -1);
	
	if (flags & AG_MENU_VFILL) {
		AG_ExpandVert(m);
	}
	AG_ExpandHoriz(m);

	agAppMenu = m;
	agAppMenuWin = AG_WindowNewNamed(AG_WINDOW_PLAIN|AG_WINDOW_KEEPBELOW|
					 AG_WINDOW_DENYFOCUS|
					 AG_WINDOW_HMAXIMIZE,
					 "_agAppMenu");
	AG_ObjectAttach(agAppMenuWin, m);
	AG_WindowSetPadding(agAppMenuWin, 0, 0, 0, 0);
	AG_WindowShow(agAppMenuWin);
	
	AG_MutexUnlock(&agAppMenuLock);
	return (m);
}

AG_Window *
AG_MenuExpand(AG_Menu *m, AG_MenuItem *item, int x, int y)
{
	AG_Window *win;

	AG_MenuUpdateItem(item);

	if (item->nsubitems == 0)
		return (NULL);

	win = AG_WindowNew(AG_WINDOW_NOTITLE|AG_WINDOW_NOBORDERS|
	                   AG_WINDOW_DENYFOCUS|AG_WINDOW_KEEPABOVE);
	AG_WindowSetCaption(win, "win-popup");
	AG_WindowSetPadding(win, 0, 0, 0, 0);

	item->view = Malloc(sizeof(AG_MenuView));
	AG_ObjectInit(item->view, &agMenuViewClass);
	item->view->panel = win;
	item->view->pmenu = m;
	item->view->pitem = item;
	AG_ObjectAttach(win, item->view);

	AG_WindowShow(win);
	AG_WindowSetGeometry(win, x, y, -1, -1);
	return (win);
}

void
AG_MenuCollapse(AG_Menu *m, AG_MenuItem *item)
{
	int i;

	if (item == NULL)
		return;

	AG_ObjectLock(m);
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
	AG_ObjectUnlock(m);
}

void
AG_MenuSetPadding(AG_Menu *m, int lPad, int rPad, int tPad, int bPad)
{
	AG_ObjectLock(m);
	if (lPad != -1) { m->lPad = lPad; }
	if (rPad != -1) { m->rPad = rPad; }
	if (tPad != -1) { m->tPad = tPad; }
	if (bPad != -1) { m->bPad = bPad; }
	AG_ObjectUnlock(m);
}

void
AG_MenuSetLabelPadding(AG_Menu *m, int lPad, int rPad, int tPad, int bPad)
{
	AG_ObjectLock(m);
	if (lPad != -1) { m->lPadLbl = lPad; }
	if (rPad != -1) { m->rPadLbl = rPad; }
	if (tPad != -1) { m->tPadLbl = tPad; }
	if (bPad != -1) { m->bPadLbl = bPad; }
	AG_ObjectUnlock(m);
}

static void
mousebuttondown(AG_Event *event)
{
	AG_Menu *m = AG_SELF();
	int x = AG_INT(2);
	int y = AG_INT(3);
	int i;

	if (m->root == NULL)
		return;

	for (i = 0; i < m->root->nsubitems; i++) {
		AG_MenuItem *item = &m->root->subitems[i];
		int lbl = (item->lblEnabled!=-1) ? item->lblEnabled :
			  (item->lblDisabled!=-1) ? item->lblDisabled :
			  -1;
		int wLbl, hLbl;

		if (lbl == -1) { continue; }
		wLbl = WSURFACE(m,lbl)->w + m->lPadLbl + m->rPadLbl;
		hLbl = WSURFACE(m,lbl)->h + m->tPadLbl + m->bPadLbl;

		if (x >= item->x &&
		    x < (item->x + wLbl) &&
		    y >= item->y &&
		    y < (item->y + m->itemh)) {
		    	if (m->itemSel == item) {
				AG_MenuCollapse(m, item);
				m->itemSel = NULL;
				m->selecting = 0;
			} else {
				if (m->itemSel != NULL) {
					AG_MenuCollapse(m, m->itemSel);
				}
				m->itemSel = item;
				AG_MenuExpand(m, item,
				    WIDGET(m)->cx + item->x,
				    WIDGET(m)->cy + item->y + hLbl+m->bPad-1);
				m->selecting = 1;
			}
			break;
		}
	}
}

#if 0
static void
mousebuttonup(AG_Event *event)
{
	AG_Menu *m = AG_SELF();
	int button = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);

	if (m->itemSel != NULL && m->sel_subitem == NULL &&
	    x >= m->itemSel->x &&
	    x < (m->itemSel->x + WSURFACE(m,m->itemSel->label)->w) &&
	    y >= m->itemSel->y &&
	    y < (m->itemSel->y + m->itemh)) {
		m->itemSel = NULL;
	}
}
#endif

static void
mousemotion(AG_Event *event)
{
	AG_Menu *m = AG_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);
	int i;

	if (!m->selecting || y < 0 || y >= WIDGET(m)->h-1)
		return;

	if (x < 0 && m->itemSel != NULL) {
		AG_MenuCollapse(m, m->itemSel);
		m->itemSel = NULL;
		return;
	}
	if (m->root == NULL) {
		return;
	}
	for (i = 0; i < m->root->nsubitems; i++) {
		AG_MenuItem *item = &m->root->subitems[i];
		int lbl = (item->lblEnabled != -1) ? item->lblEnabled :
			  (item->lblDisabled != -1) ? item->lblDisabled :
			  -1;
		int wLbl, hLbl;

		if (lbl == -1) { continue; }
		wLbl = WSURFACE(m,lbl)->w + m->lPadLbl + m->rPadLbl;
		hLbl = WSURFACE(m,lbl)->h + m->tPadLbl + m->bPadLbl;

		if (x >= item->x &&
		    x < (item->x + wLbl) &&
		    y >= item->y &&
		    y < (item->y + m->itemh)) {
		    	if (item != m->itemSel) {
				if (m->itemSel != NULL) {
					AG_MenuCollapse(m, m->itemSel);
				}
				m->itemSel = item;
				AG_MenuExpand(m, item,
				    WIDGET(m)->cx + item->x,
				    WIDGET(m)->cy + item->y + hLbl+m->bPad-1);
			}
			break;
		}
	}
#if 0
	if (i == m->root->nsubitems && m->itemSel != NULL) {
		AG_MenuCollapse(m, m->itemSel);
		m->itemSel = NULL;
	}
#endif
}

static void
attached(AG_Event *event)
{
	AG_Widget *pwid = AG_SENDER();
	AG_Window *pwin;

	/* Adjust the top padding of the parent window if any. */
	if ((pwin = AG_WidgetParentWindow(pwid)) != NULL)
		AG_WindowSetPadding(pwin, -1, -1, 0, pwin->bPad);
}

/* Generic constructor for menu items. Menu must be locked. */
static AG_MenuItem *
CreateItem(AG_MenuItem *pitem, const char *text, SDL_Surface *icon)
{
	AG_MenuItem *mi;

	if (pitem != NULL) {
		if (pitem->subitems == NULL) {
			pitem->subitems = Malloc(sizeof(AG_MenuItem));
		} else {
			pitem->subitems = Realloc(pitem->subitems,
					  (pitem->nsubitems+1) * 
					  sizeof(AG_MenuItem));
		}
		mi = &pitem->subitems[pitem->nsubitems++];
		mi->pmenu = pitem->pmenu;
		mi->pitem = pitem;
		mi->y = pitem->nsubitems*mi->pmenu->itemh - mi->pmenu->itemh;
		mi->state = mi->pmenu->curState;
	} else {
		mi = Malloc(sizeof(AG_MenuItem));
		mi->pmenu = NULL;
		mi->pitem = NULL;
		mi->y = 0;
		mi->state = 1;
	}
	mi->subitems = NULL;
	mi->nsubitems = 0;
	mi->view = NULL;
	mi->sel_subitem = NULL;
	mi->key_equiv = 0;
	mi->key_mod = 0;
	mi->clickFn = NULL;
	mi->poll = NULL;
	mi->bind_type = AG_MENU_NO_BINDING;
	mi->bind_flags = 0;
	mi->bind_invert = 0;
	mi->bind_lock = NULL;
	mi->text = Strdup((text != NULL) ? text : "");
	mi->lblEnabled = -1;
	mi->lblDisabled = -1;
	mi->value = -1;
	mi->flags = 0;
	mi->icon = -1;

	if (icon != NULL) {
		if (pitem != NULL) {
			/* Request that the parent allocate space for icons. */
			pitem->flags |= AG_MENU_ITEM_ICONS;
		}
		/* TODO: NODUP */
		mi->iconSrc = AG_DupSurface(icon);
	} else {
		mi->iconSrc = NULL;
	}

	/* If this is the application menu, resize its window. */
	if (mi->pmenu != NULL && (mi->pmenu->flags & AG_MENU_GLOBAL)) {
		AG_SizeReq rMenu;

		AG_WidgetSizeReq(mi->pmenu, &rMenu);
		AG_WindowSetGeometry(agAppMenuWin, 0, 0, agView->w, rMenu.h);
	}
	return (mi);
}

static void
Init(void *obj)
{
	AG_Menu *m = obj;

	WIDGET(m)->flags |= AG_WIDGET_UNFOCUSED_MOTION|
	                    AG_WIDGET_UNFOCUSED_BUTTONUP|
	                    AG_WIDGET_IGNORE_PADDING|
	                    AG_WIDGET_CLIPPING;

	m->flags = 0;
	m->lPad = 5;
	m->rPad = 5;
	m->tPad = 2;
	m->bPad = 2;
	m->lPadLbl = 6;
	m->rPadLbl = 7;
	m->tPadLbl = 3;
	m->bPadLbl = 3;

	m->curState = 1;
	m->selecting = 0;
	m->itemSel = NULL;
	m->itemh = agTextFontHeight + m->tPadLbl + m->bPadLbl;
	m->root = CreateItem(NULL, NULL, NULL);
	m->root->pmenu = m;

	AG_SetEvent(m, "window-mousebuttondown", mousebuttondown, NULL);
#if 0
	AG_SetEvent(m, "window-mousebuttonup", mousebuttonup, NULL);
#endif
	AG_SetEvent(m, "window-mousemotion", mousemotion, NULL);
	AG_SetEvent(m, "attached", attached, NULL);
}

/* Change the icon associated with a menu item. */
void
AG_MenuSetIcon(AG_MenuItem *mi, SDL_Surface *icon)
{
	AG_ObjectLock(mi->pmenu);
	if (mi->icon == -1) {
		mi->icon = (icon != NULL) ?
		    AG_WidgetMapSurface(mi->pmenu, AG_DupSurface(icon)) : -1;
	} else {
		AG_WidgetReplaceSurface(mi->pmenu, mi->icon,
		    icon != NULL ? AG_DupSurface(icon) : NULL);
	}
	AG_ObjectUnlock(mi->pmenu);
}

/* Change menu item text. */
void
AG_MenuSetLabel(AG_MenuItem *mi, const char *fmt, ...)
{
	va_list ap;
	
	AG_ObjectLock(mi->pmenu);

	va_start(ap, fmt);
	Free(mi->text);
	Vasprintf(&mi->text, fmt, ap);
	va_end(ap);

	if (mi->lblEnabled != -1) {
		mi->lblEnabled = -1;
		AG_WidgetUnmapSurface(mi->pmenu, mi->lblEnabled);
	}
	if (mi->lblDisabled != -1) {
		mi->lblDisabled = -1;
		AG_WidgetUnmapSurface(mi->pmenu, mi->lblDisabled);
	}
	
	AG_ObjectUnlock(mi->pmenu);
}

/* Create a menu separator. */
AG_MenuItem *
AG_MenuSeparator(AG_MenuItem *pitem)
{
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, NULL, NULL);
	mi->flags |= AG_MENU_ITEM_NOSELECT|AG_MENU_ITEM_SEPARATOR;
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

/* Create a menu section label. */
AG_MenuItem *
AG_MenuSection(AG_MenuItem *pitem, const char *fmt, ...)
{
	char text[AG_LABEL_MAX];
	AG_MenuItem *mi;
	va_list ap;

	va_start(ap, fmt);
	Vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, text, NULL);
	mi->flags |= AG_MENU_ITEM_NOSELECT;
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

/* Create a dynamically-updated menu item. */
AG_MenuItem *
AG_MenuDynamicItem(AG_MenuItem *pitem, const char *text, SDL_Surface *icon,
    AG_EventFn fn, const char *fmt, ...)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, text, icon);
	mi->poll = AG_SetEvent(m, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(mi->poll, fmt);
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

/* Create a dynamically-updated menu item with a keyboard binding. */
AG_MenuItem *
AG_MenuDynamicItemKb(AG_MenuItem *pitem, const char *text, SDL_Surface *icon,
    SDLKey key, SDLMod kmod, AG_EventFn fn, const char *fmt, ...)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, text, icon);
	mi->key_equiv = key;
	mi->key_mod = kmod;
	mi->poll = AG_SetEvent(m, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(mi->poll, fmt);
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

/* Set a dynamic update function for an existing menu item. */
void
AG_MenuSetPollFn(AG_MenuItem *mi, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(mi->pmenu);
	if (mi->poll != NULL) {
		AG_UnsetEvent(mi->pmenu, mi->poll->name);
	}
	mi->poll = AG_SetEvent(mi->pmenu, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(mi->poll, fmt);
	AG_ObjectUnlock(mi->pmenu);
}

/* Create a menu item without any associated action. */
AG_MenuItem *
AG_MenuNode(AG_MenuItem *pitem, const char *text, SDL_Surface *icon)
{
	AG_MenuItem *node;

	AG_ObjectLock(pitem->pmenu);
	node = CreateItem(pitem, text, icon);
	AG_ObjectUnlock(pitem->pmenu);
	return (node);
}

/* Create a menu item associated with a function. */
AG_MenuItem *
AG_MenuAction(AG_MenuItem *pitem, const char *text, SDL_Surface *icon,
    AG_EventFn fn, const char *fmt, ...)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, text, icon);
	mi->clickFn = AG_SetEvent(m, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(mi->clickFn, fmt);
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

AG_MenuItem *
AG_MenuActionKb(AG_MenuItem *pitem, const char *text, SDL_Surface *icon,
    SDLKey key, SDLMod kmod, AG_EventFn fn, const char *fmt, ...)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, text, icon);
	mi->key_equiv = key;
	mi->key_mod = kmod;
	mi->clickFn = AG_SetEvent(m, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(mi->clickFn, fmt);
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

AG_MenuItem *
AG_MenuTool(AG_MenuItem *pitem, AG_Toolbar *tbar, const char *text,
    SDL_Surface *icon, SDLKey key, SDLMod kmod, void (*fn)(AG_Event *),
    const char *fmt, ...)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;
	AG_Button *bu;
	AG_Event *btn_ev;
	
	AG_ObjectLock(pitem->pmenu);
	AG_ObjectLock(tbar);

	bu = AG_ButtonNew(tbar->rows[0], 0, NULL);
	AG_ButtonSurfaceNODUP(bu, icon);
	AG_ButtonSetFocusable(bu, 0);
	btn_ev = AG_SetEvent(bu, "button-pushed", fn, NULL);
	AG_EVENT_GET_ARGS(btn_ev, fmt);

	mi = CreateItem(pitem, text, icon);
	mi->key_equiv = key;
	mi->key_mod = kmod;
	mi->clickFn = AG_SetEvent(m, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(mi->clickFn, fmt);
	
	AG_ObjectUnlock(tbar);
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

AG_MenuItem *
AG_MenuIntBoolMp(AG_MenuItem *pitem, const char *text, SDL_Surface *icon,
    int *boolp, int inv, AG_Mutex *lock)
{
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, text, icon);
	mi->bind_type = AG_MENU_INT_BOOL;
	mi->bind_p = (void *)boolp;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

AG_MenuItem *
AG_MenuInt8BoolMp(AG_MenuItem *pitem, const char *text, SDL_Surface *icon,
    Uint8 *boolp, int inv, AG_Mutex *lock)
{
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, text, icon);
	mi->bind_type = AG_MENU_INT8_BOOL;
	mi->bind_p = (void *)boolp;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

AG_MenuItem *
AG_MenuIntFlagsMp(AG_MenuItem *pitem, const char *text, SDL_Surface *icon,
    int *flagsp, int flags, int inv, AG_Mutex *lock)
{
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, text, icon);
	mi->bind_type = AG_MENU_INT_FLAGS;
	mi->bind_p = (void *)flagsp;
	mi->bind_flags = flags;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

AG_MenuItem *
AG_MenuInt8FlagsMp(AG_MenuItem *pitem, const char *text, SDL_Surface *icon,
    Uint8 *flagsp, Uint8 flags, int inv, AG_Mutex *lock)
{
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, text, icon);
	mi->bind_type = AG_MENU_INT8_FLAGS;
	mi->bind_p = (void *)flagsp;
	mi->bind_flags = flags;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

AG_MenuItem *
AG_MenuInt16FlagsMp(AG_MenuItem *pitem, const char *text, SDL_Surface *icon,
    Uint16 *flagsp, Uint16 flags, int inv, AG_Mutex *lock)
{
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, text, icon);
	mi->bind_type = AG_MENU_INT16_FLAGS;
	mi->bind_p = (void *)flagsp;
	mi->bind_flags = flags;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

AG_MenuItem *
AG_MenuInt32FlagsMp(AG_MenuItem *pitem, const char *text, SDL_Surface *icon, 
    Uint32 *flagsp, Uint32 flags, int inv, AG_Mutex *lock)
{
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, text, icon);
	mi->bind_type = AG_MENU_INT32_FLAGS;
	mi->bind_p = (void *)flagsp;
	mi->bind_flags = flags;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

/* Free the subitems of an item. */
void
AG_MenuItemFreeChildren(AG_MenuItem *mi)
{
	int i;

	AG_ObjectLock(mi->pmenu);
	for (i = 0; i < mi->nsubitems; i++) {
		AG_MenuItemFree(&mi->subitems[i]);
	}
	Free(mi->subitems);
	mi->subitems = NULL;
	mi->nsubitems = 0;
	AG_ObjectUnlock(mi->pmenu);
}

/* Free an item as well as its subitems. */
void
AG_MenuItemFree(AG_MenuItem *mi)
{
	AG_ObjectLock(mi->pmenu);
	AG_MenuItemFreeChildren(mi);
	if (mi->lblEnabled != -1) {
		AG_WidgetUnmapSurface(mi->pmenu, mi->lblEnabled);
		mi->lblEnabled = -1;
	}
	if (mi->lblDisabled != -1) {
		AG_WidgetUnmapSurface(mi->pmenu, mi->lblDisabled);
		mi->lblDisabled = -1;
	}
	if (mi->icon != -1) {
		AG_WidgetUnmapSurface(mi->pmenu, mi->icon);
		mi->icon = -1;
	}
	Free(mi->text);
	AG_ObjectUnlock(mi->pmenu);
}

static void
Destroy(void *p)
{
	AG_Menu *m = p;

	if (m->root != NULL)
		AG_MenuItemFree(m->root);
}

void
AG_MenuUpdateItem(AG_MenuItem *mi)
{
	AG_ObjectLock(mi->pmenu);
	if (mi->poll != NULL) {
		AG_MenuItemFreeChildren(mi);
		AG_PostEvent(mi, mi->pmenu, mi->poll->name, NULL);
	}
	AG_ObjectUnlock(mi->pmenu);
}

void
AG_MenuState(AG_MenuItem *mi, int state)
{
	AG_ObjectLock(mi->pmenu);
	mi->pmenu->curState = state;
	AG_ObjectUnlock(mi->pmenu);
}

static void
Draw(void *p)
{
	AG_Menu *m = p;
	int lbl, wLbl, hLbl;
	int i;

	STYLE(m)->MenuRootBackground(m);

	if (m->root == NULL) {
		return;
	}
	for (i = 0; i < m->root->nsubitems; i++) {
		AG_MenuItem *item = &m->root->subitems[i];

		if (item->state) {
			if (item->lblEnabled == -1) {
				AG_TextColor(MENU_TXT_COLOR);
				item->lblEnabled = (item->text == NULL) ? -1 :
				    AG_WidgetMapSurface(m,
				    AG_TextRender(item->text));
			}
			lbl = item->lblEnabled;
		} else {
			if (item->lblDisabled == -1) {
				AG_TextColor(MENU_TXT_DISABLED_COLOR);
				item->lblDisabled = (item->text == NULL) ? -1 :
				    AG_WidgetMapSurface(m,
				    AG_TextRender(item->text));
			}
			lbl = item->lblDisabled;
		}
		wLbl = WSURFACE(m,lbl)->w;
		hLbl = WSURFACE(m,lbl)->h;
		if (item == m->itemSel) {
			STYLE(m)->MenuRootSelectedItemBackground(m,
			    AG_RECT(item->x,
			            item->y,
	    		            m->lPadLbl + wLbl + m->rPadLbl,
	    		            m->tPadLbl + hLbl + m->bPadLbl));
		}
		AG_WidgetBlitSurface(m, lbl,
		    item->x + m->lPadLbl,
		    item->y + m->tPadLbl);
	}
}

static void
GetItemSize(AG_MenuItem *item, int *w, int *h)
{
	AG_Menu *m = item->pmenu;
	int lbl;

	if (item->lblEnabled != -1) {
		lbl = item->lblEnabled;
	} else if (item->lblDisabled != -1) {
		lbl = item->lblDisabled;
	} else {
		lbl = -1;
	}
	if (lbl != -1) {
		*w = WSURFACE(m,lbl)->w;
		*h = WSURFACE(m,lbl)->h;
	} else {
		AG_TextSize(item->text, w, h);
	}
	(*w) += m->lPadLbl + m->rPadLbl;
	(*h) += m->tPadLbl + m->bPadLbl;
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Menu *m = p;
	int i, x, y;
	int wLbl, hLbl;

	x = m->lPad;
	y = m->tPad;
	r->h = 0;
	r->w = x;

	if (m->root == NULL) {
		return;
	}
	for (i = 0; i < m->root->nsubitems; i++) {
		GetItemSize(&m->root->subitems[i], &wLbl, &hLbl);
		if (r->h == 0) {
			r->h = m->tPad+hLbl+m->bPad;
		}
		if (x+wLbl > agView->w) {			/* Wrap */
			x = m->lPad;
			y += hLbl;
			r->h += hLbl+m->bPad;
		}
		if (r->w < MIN(x+wLbl,agView->w)) {
			r->w = MIN(x+wLbl,agView->w);
		}
		x += wLbl;
	}
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Menu *m = p;
	int wLbl, hLbl;
	int x, y, i;
	
	if (WIDGET(m)->w < (m->lPad + m->rPad) ||
	    WIDGET(m)->h < (m->tPad + m->bPad)) {
		return (-1);
	}
	if (m->root == NULL) {
		return (-1);
	}
	x = m->lPad;
	y = m->tPad;
	for (i = 0; i < m->root->nsubitems; i++) {
		AG_MenuItem *item = &m->root->subitems[i];

		GetItemSize(item, &wLbl, &hLbl);
		item->x = x;
		item->y = y;
		if (x+wLbl > a->w) {
			item->x = m->lPad;
			item->y += hLbl;
			y += hLbl;
		}
		x += wLbl;
	}
	return (0);
}

AG_PopupMenu *
AG_PopupNew(void *pwid)
{
	AG_Widget *wid = pwid;
	AG_PopupMenu *pm;

	pm = Malloc(sizeof(AG_PopupMenu));
	pm->menu = AG_MenuNew(NULL, 0);
	pm->item = pm->menu->itemSel = AG_MenuAddItem(pm->menu, NULL);
	/* XXX redundant */
	pm->win = NULL;

	AG_ObjectLock(wid);
	SLIST_INSERT_HEAD(&wid->menus, pm, menus);
	AG_ObjectUnlock(wid);
	return (pm);
}

void
AG_PopupShow(AG_PopupMenu *pm)
{
	int x, y;

	AG_ObjectLock(pm->menu);
	AG_PopupHide(pm);
	SDL_GetMouseState(&x, &y);
	pm->win = AG_MenuExpand(pm->menu, pm->item, x, y);
	AG_ObjectUnlock(pm->menu);
}

void
AG_PopupShowAt(AG_PopupMenu *pm, int x, int y)
{
	AG_ObjectLock(pm->menu);
	AG_PopupHide(pm);
	pm->win = AG_MenuExpand(pm->menu, pm->item, x, y);
	AG_ObjectUnlock(pm->menu);
}

void
AG_PopupHide(AG_PopupMenu *pm)
{
	AG_ObjectLock(pm->menu);
	if (pm->win != NULL) {
		AG_MenuCollapse(pm->menu, pm->item);
		pm->win = NULL;
	}
	AG_ObjectUnlock(pm->menu);
}

void
AG_PopupDestroy(void *pWid, AG_PopupMenu *pm)
{
	if (pWid != NULL) {
		AG_ObjectLock(pWid);
		SLIST_REMOVE(&WIDGET(pWid)->menus, pm, ag_popup_menu, menus);
		AG_ObjectUnlock(pWid);
	}
	if (pm->menu != NULL) {
		AG_MenuCollapse(pm->menu, pm->item);
		AG_ObjectDestroy(pm->menu);
	}
	pm->menu = NULL;
	pm->item = NULL;
	pm->win = NULL;
	Free(pm);
}

AG_WidgetClass agMenuClass = {
	{
		"AG_Widget:AG_Menu",
		sizeof(AG_Menu),
		{ 0,0 },
		Init,
		NULL,			/* free */
		Destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};