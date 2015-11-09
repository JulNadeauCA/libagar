/*
 * Copyright (c) 2004-2015 Hypertriton, Inc. <http://hypertriton.com/>
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
#include <agar/gui/menu.h>
#include <agar/gui/primitive.h>
#include <agar/gui/label.h>
#include <agar/gui/button.h>

#include <stdarg.h>
#include <string.h>

AG_Menu *agAppMenu = NULL;
AG_Window *agAppMenuWin = NULL;
AG_Mutex agAppMenuLock;

/* Initialize global application menu data; called from AG_InitGUI(). */
void
AG_InitAppMenu(void)
{
	AG_MutexInitRecursive(&agAppMenuLock);
	agAppMenu = NULL;
	agAppMenuWin = NULL;
}

/* Cleanup global application menu data; called from AG_DestroyGUI(). */
void
AG_DestroyAppMenu(void)
{
	agAppMenu = NULL;
	agAppMenuWin = NULL;
	AG_MutexDestroy(&agAppMenuLock);
}

/* Create a new Menu widget. */
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

/* Create a new global application menu. */
AG_Menu *
AG_MenuNewGlobal(Uint flags)
{
	AG_Menu *m;
	AG_Window *win;
	Uint wMax, hMax;
	Uint wFlags = AG_WINDOW_KEEPBELOW|AG_WINDOW_DENYFOCUS;

	AG_MutexLock(&agAppMenuLock);
	if (agAppMenu != NULL)
		goto exists;

	if (agDriverSw) {
		wFlags |= AG_WINDOW_PLAIN|AG_WINDOW_HMAXIMIZE;
	}
	win = AG_WindowNewNamedS(wFlags, "_agAppMenu");
	win->wmType = AG_WINDOW_WM_DOCK;
	if (win == NULL) {
		goto exists;
	}
	AG_WindowSetPadding(win, 0, 0, 0, 0);
	AG_WindowSetCaptionS(win, agProgName != NULL ? agProgName : "agarapp");

	m = AG_MenuNew(win, flags);
	m->style = AG_MENU_GLOBAL;
	AG_MenuSetPadding(m, 4, 4, -1, -1);
	AG_ExpandHoriz(m);

	agAppMenu = m;
	agAppMenuWin = win;

	if (agDriverSw) {
		AG_GetDisplaySize(WIDGET(win)->drv, &wMax, &hMax);
		AG_WindowSetGeometryAligned(win, AG_WINDOW_TC, wMax, -1);
	} else {
		AG_GetDisplaySize(WIDGET(win)->drv, &wMax, &hMax);
		AG_WindowSetGeometryAligned(win, AG_WINDOW_TL, wMax/3, -1);
	}
	AG_WindowShow(win);

	AG_MutexUnlock(&agAppMenuLock);
	return (m);
exists:
	AG_SetError("Application menu is already defined");
	AG_MutexUnlock(&agAppMenuLock);
	return (NULL);
}

static void
MenuCollapseAll(AG_Event *event)
{
	AG_Menu *m = AG_PTR(1);
	AG_MenuCollapseAll(m);
}

/*
 * Expand an AG_MenuItem. Create a window containing the AG_MenuView
 * at coordinates x1,y1 (relative to widget parent).
 *
 * The associated AG_Menu object must be locked.
 */
AG_Window *
AG_MenuExpand(void *parent, AG_MenuItem *mi, int x1, int y1)
{
	AG_Window *win, *winParent;
	AG_Menu *m;
	int x = x1;
	int y = y1;

	if (parent != NULL) {
		if (AG_OfClass(parent, "AG_Widget:AG_MenuView")) {
			m = ((AG_MenuView *)parent)->pmenu;
		} else if (AG_OfClass(parent, "AG_Widget:AG_Menu")) {
			m = parent;
		} else {
			m = mi->pmenu;
		}
		x += WIDGET(parent)->rView.x1;
		y += WIDGET(parent)->rView.y1;
		if ((winParent = WIDGET(parent)->window) == NULL) {
			AG_FatalError("AG_MenuExpand: %s has no window", OBJECT(parent)->name);
		}
		if (WIDGET(winParent)->drv != NULL &&
		    AGDRIVER_MULTIPLE(WIDGET(winParent)->drv)) {
			/* Convert to absolute coordinates */
			x += WIDGET(winParent)->x;
			y += WIDGET(winParent)->y;
		}
	} else {
		m = mi->pmenu;
		winParent = NULL;
	}

	AG_MenuUpdateItem(mi);

	if (mi->nSubItems == 0)
		return (NULL);

	if (mi->view != NULL) {
		win = WIDGET(mi->view)->window;
		AG_WindowSetGeometry(win, x, y, -1, -1);
		AG_WindowShow(win);
		return (win);
	}

	win = AG_WindowNew(
	    AG_WINDOW_MODAL|AG_WINDOW_NOTITLE|AG_WINDOW_NOBORDERS|
	    AG_WINDOW_NORESIZE|AG_WINDOW_DENYFOCUS|AG_WINDOW_KEEPABOVE);
	if (win == NULL) {
		return (NULL);
	}
	win->wmType = (m->style == AG_MENU_DROPDOWN) ?
	              AG_WINDOW_WM_DROPDOWN_MENU :
		      AG_WINDOW_WM_POPUP_MENU;
	AG_ObjectSetName(win, "_Popup-%s",
	    (parent != NULL) ? OBJECT(parent)->name : "generic");
	AG_WindowSetPadding(win, 0, 0, 0, 0);

	AG_SetEvent(win, "window-modal-close", MenuCollapseAll, "%p", m);
	AG_SetEvent(win, "window-close", MenuCollapseAll, "%p", m);
	
	mi->view = Malloc(sizeof(AG_MenuView));
	AG_ObjectInit(mi->view, &agMenuViewClass);
	mi->view->pmenu = m;
	mi->view->pitem = mi;
	AG_ObjectAttach(win, mi->view);

	if (winParent != NULL) {
		AG_WindowAttach(winParent, win);
		AG_WindowMakeTransient(winParent, win);
		AG_WindowPin(winParent, win);
	}
	AG_WindowSetGeometry(win, x, y, -1,-1);
	AG_WindowShow(win);
	return (win);
}

/*
 * Collapse the window displaying the specified item and its sub-menus
 * (if any).
 */
void
AG_MenuCollapse(AG_MenuItem *mi)
{
	AG_Menu *m;
	AG_MenuItem *miSub;

	if (mi == NULL || mi->view == NULL || (m = mi->pmenu) == NULL)
		return;

	AG_ObjectLock(m);

	TAILQ_FOREACH(miSub, &mi->subItems, items)
		AG_MenuCollapse(miSub);

	if (mi->view != NULL) {
		AG_WindowHide(WIDGET(mi->view)->window);
	}
	mi->sel_subitem = NULL;
	
	AG_ObjectUnlock(m);
}

static void
CollapseAll(AG_Menu *m, AG_MenuItem *mi)
{
	AG_MenuItem *miSub;

	TAILQ_FOREACH(miSub, &mi->subItems, items) {
		CollapseAll(m, miSub);
	}
	if (mi->view != NULL)
		AG_MenuCollapse(mi);
}

/*
 * Collapse the window displaying the contents of an item as well as
 * all expanded MenuView windows, up to the menu root.
 */
void
AG_MenuCollapseAll(AG_Menu *m)
{
	AG_ObjectLock(m);

	CollapseAll(m, m->root);
	m->itemSel = NULL;
	m->selecting = 0;
	
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
	AG_Redraw(m);
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
	AG_Redraw(m);
}

static __inline__ int
IntersectItem(AG_MenuItem *mi, int x, int y, int *hLbl)
{
	AG_Menu *m = mi->pmenu;
	int lbl, wLbl;

	lbl = (mi->lblMenu[1] != -1) ? mi->lblMenu[1] :
	      (mi->lblMenu[0] != -1) ? mi->lblMenu[0] :
	      -1;
	if (lbl != -1) {
		wLbl = WSURFACE(m,lbl)->w + m->lPadLbl + m->rPadLbl;
		*hLbl = WSURFACE(m,lbl)->h + m->tPadLbl + m->bPadLbl;
	} else {
		wLbl = 0;
		*hLbl = 0;
	}
	return (x >= mi->x && x < (mi->x + wLbl) &&
		y >= mi->y && y < (mi->y + m->itemh));
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Menu *m = AG_SELF();
	int x = AG_INT(2);
	int y = AG_INT(3);
	AG_MenuItem *mi;
	int hLbl;

	if (m->root == NULL)
		return;

	TAILQ_FOREACH(mi, &m->root->subItems, items) {
		if (!IntersectItem(mi, x, y, &hLbl)) {
			continue;
		}
	    	if (m->itemSel == mi) {
			AG_MenuCollapse(mi);
			m->itemSel = NULL;
			m->selecting = 0;
		} else {
			if (m->itemSel != NULL) {
				AG_MenuCollapse(m->itemSel);
			}
			m->itemSel = mi;
			AG_MenuExpand(m, mi,
			    mi->x,
			    mi->y + hLbl + m->bPad - 1);
			m->selecting = 1;
		}
		AG_Redraw(m);
		break;
	}
}

static void
MouseMotion(AG_Event *event)
{
	AG_Menu *m = AG_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);
	AG_MenuItem *mi;
	int hLbl;

	if (!m->selecting || y < 0 || y >= HEIGHT(m)-1 ||
	    m->root == NULL)
		return;

	TAILQ_FOREACH(mi, &m->root->subItems, items) {
		if (!IntersectItem(mi, x, y, &hLbl)) {
			continue;
		}
	    	if (mi != m->itemSel) {
			if (m->itemSel != NULL) {
				AG_MenuCollapse(m->itemSel);
			}
			m->itemSel = mi;
			AG_MenuExpand(m, mi,
			    mi->x,
			    mi->y + hLbl + m->bPad - 1);
		}
		AG_Redraw(m);
		break;
	}
}

static void
Attached(AG_Event *event)
{
	AG_Widget *pwid = AG_SENDER();
	AG_Window *pwin;

	if ((pwin = AG_ParentWindow(pwid)) != NULL)
		AG_WindowSetPadding(pwin, -1, -1, 0, pwin->bPad);
}

/* Generic constructor for menu items. Menu must be locked. */
static AG_MenuItem *
CreateItem(AG_MenuItem *miParent, const char *text, const AG_Surface *icon)
{
	AG_Menu *m;
	AG_MenuItem *mi;
	
	mi = Malloc(sizeof(AG_MenuItem));
	mi->parent = miParent;
	mi->stateFn = NULL;

	if (miParent != NULL) {
		m = mi->pmenu = miParent->pmenu;
		mi->y = miParent->nSubItems*m->itemh - m->itemh;
		mi->state = m->curState;

		TAILQ_INSERT_TAIL(&miParent->subItems, mi, items);
		miParent->nSubItems++;
	} else {
		m = mi->pmenu = NULL;
		mi->y = 0;
		mi->state = 1;
	}
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
	mi->lblMenu[0] = -1;
	mi->lblMenu[1] = -1;
	mi->lblView[0] = -1;
	mi->lblView[1] = -1;
	mi->value = -1;
	mi->flags = 0;
	mi->icon = -1;
	mi->tbButton = NULL;
	TAILQ_INIT(&mi->subItems);
	mi->nSubItems = 0;

	if (icon != NULL) {
		if (miParent != NULL) {
			/* Request that the parent allocate space for icons. */
			miParent->flags |= AG_MENU_ITEM_ICONS;
		}
		/* TODO: NODUP */
		mi->iconSrc = AG_SurfaceDup(icon);
	} else {
		mi->iconSrc = NULL;
	}
	if (m != NULL) {
		if ((m->style == AG_MENU_GLOBAL) &&
		    agDriverSw != NULL && agAppMenuWin != NULL) {
			Uint wMax, hMax;
			AG_SizeReq rMenu;

			AG_GetDisplaySize(agDriverSw, &wMax, &hMax);
			AG_WidgetSizeReq(m, &rMenu);
			AG_WindowSetGeometry(agAppMenuWin, 0, 0, wMax, rMenu.h);
		}
		AG_Redraw(m);
	}
	return (mi);
}

static void
OnFontChange(AG_Event *event)
{
	AG_Menu *m = AG_SELF();
	AG_Font *font = WIDGET(m)->font;
	AG_MenuItem *mi;
	int j;

	TAILQ_FOREACH(mi, &m->root->subItems, items) {
		for (j = 0; j < 2; j++) {
			if (mi->lblMenu[j] != -1) {
				AG_WidgetUnmapSurface(m, mi->lblMenu[j]);
				mi->lblMenu[j] = -1;
			}
		}
	}
	m->itemh = font->height + m->tPadLbl + m->bPadLbl;
}

static void
Init(void *obj)
{
	AG_Menu *m = obj;

	WIDGET(m)->flags |= AG_WIDGET_UNFOCUSED_MOTION|
	                    AG_WIDGET_UNFOCUSED_BUTTONUP|
	                    AG_WIDGET_NOSPACING|
			    AG_WIDGET_USE_TEXT;

	m->flags = 0;
	m->lPad = 5;
	m->rPad = 5;
	m->tPad = 2;
	m->bPad = 2;
	m->lPadLbl = 6;
	m->rPadLbl = 7;
	m->tPadLbl = 3;
	m->bPadLbl = 3;
	m->r = AG_RECT(0,0,0,0);

	m->curToolbar = NULL;
	m->curState = 1;
	m->selecting = 0;
	m->itemSel = NULL;
	m->itemh = agTextFontHeight + m->tPadLbl + m->bPadLbl;
	m->style = AG_MENU_DROPDOWN;
	
	m->root = CreateItem(NULL, NULL, NULL);
	m->root->pmenu = m;

	AG_SetEvent(m, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(m, "mouse-motion", MouseMotion, NULL);
	AG_AddEvent(m, "attached", Attached, NULL);
	AG_AddEvent(m, "font-changed", OnFontChange, NULL);
}

/* Change the icon associated with a menu item. */
void
AG_MenuSetIcon(AG_MenuItem *mi, const AG_Surface *iconSrc)
{
	AG_Menu *m = mi->pmenu;

	AG_ObjectLock(m);
	if (mi->iconSrc != NULL) {
		AG_SurfaceFree(mi->iconSrc);
	}
	mi->iconSrc = iconSrc != NULL ? AG_SurfaceDup(iconSrc) : NULL;

	if (mi->icon != -1 &&
	    mi->parent != NULL &&
	    mi->parent->view != NULL) {
		AG_WidgetUnmapSurface(mi->parent->view, mi->icon);
		mi->icon = -1;
	}
	AG_ObjectUnlock(m);
	AG_Redraw(m);
}

/* Unmap cached Menu/MenuView label surfaces for the specified item. */
static void
InvalidateLabelSurfaces(AG_MenuItem *mi)
{
	int i;

	for (i = 0; i < 2; i++) {
		if (mi->lblMenu[i] != -1) {
			AG_WidgetUnmapSurface(mi->pmenu, mi->lblMenu[i]);
			mi->lblMenu[i] = -1;
		}
		if (mi->lblView[i] != -1 &&
		    mi->parent != NULL &&
		    mi->parent->view != NULL) {
			AG_WidgetUnmapSurface(mi->parent->view, mi->lblView[i]);
			mi->lblView[i] = -1;
		}
	}
}

/* Change menu item text (format string). */
void
AG_MenuSetLabel(AG_MenuItem *mi, const char *fmt, ...)
{
	AG_Menu *m = mi->pmenu;
	va_list ap;
	
	AG_ObjectLock(m);

	va_start(ap, fmt);
	Free(mi->text);
	Vasprintf(&mi->text, fmt, ap);
	va_end(ap);

	InvalidateLabelSurfaces(mi);

	AG_ObjectUnlock(m);
	AG_Redraw(m);
}

/* Change menu item text (C string). */
void
AG_MenuSetLabelS(AG_MenuItem *mi, const char *s)
{
	AG_Menu *m = mi->pmenu;

	AG_ObjectLock(m);

	Free(mi->text);
	mi->text = Strdup(s);
	InvalidateLabelSurfaces(mi);

	AG_ObjectUnlock(m);
	AG_Redraw(m);
}

/* Create a menu separator. */
AG_MenuItem *
AG_MenuSeparator(AG_MenuItem *pitem)
{
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	
	mi = CreateItem(pitem, NULL, NULL);
	mi->flags |= AG_MENU_ITEM_NOSELECT|AG_MENU_ITEM_SEPARATOR;

	if (pitem->pmenu->curToolbar != NULL)
		AG_ToolbarSeparator(pitem->pmenu->curToolbar);

	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

/* Create a menu section label (format string). */
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

/* Create a menu section label (C string). */
AG_MenuItem *
AG_MenuSectionS(AG_MenuItem *pitem, const char *label)
{
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, label, NULL);
	mi->flags |= AG_MENU_ITEM_NOSELECT;
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

/* Create a dynamically-updated menu item. */
AG_MenuItem *
AG_MenuDynamicItem(AG_MenuItem *pitem, const char *text, const AG_Surface *icon,
    AG_EventFn fn, const char *fmt, ...)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;

	AG_ObjectLock(m);
	mi = CreateItem(pitem, text, icon);
	mi->poll = AG_SetEvent(m, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(mi->poll, fmt);
	AG_ObjectUnlock(m);
	return (mi);
}

/* Create a dynamically-updated menu item with a keyboard binding. */
AG_MenuItem *
AG_MenuDynamicItemKb(AG_MenuItem *pitem, const char *text, const AG_Surface *icon,
    AG_KeySym key, AG_KeyMod kmod, AG_EventFn fn, const char *fmt, ...)
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
	AG_Menu *m = mi->pmenu;

	AG_ObjectLock(m);
	if (mi->poll != NULL) {
		AG_UnsetEvent(m, mi->poll->name);
	}
	mi->poll = AG_SetEvent(m, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(mi->poll, fmt);
	AG_ObjectUnlock(m);
}

/* Create a menu item without any associated action. */
AG_MenuItem *
AG_MenuNode(AG_MenuItem *pitem, const char *text, const AG_Surface *icon)
{
	AG_MenuItem *node;

	AG_ObjectLock(pitem->pmenu);
	node = CreateItem(pitem, text, icon);
	AG_ObjectUnlock(pitem->pmenu);
	return (node);
}

static AG_Button *
CreateToolbarButton(AG_MenuItem *mi, const AG_Surface *icon, const char *text)
{
	AG_Menu *m = mi->pmenu;
	AG_Button *bu;

	if (icon != NULL) {
		bu = AG_ButtonNewS(m->curToolbar->rows[0], 0, NULL);
		AG_ButtonSurface(bu, icon);
	} else {
		bu = AG_ButtonNewS(m->curToolbar->rows[0], 0, text);
	}
	AG_ButtonSetFocusable(bu, 0);
	m->curToolbar->nButtons++;
	mi->tbButton = bu;
	return (bu);
}

static __inline__ AG_Button *
CreateToolbarButtonBool(AG_MenuItem *mi, const AG_Surface *icon, const char *text,
    int inv)
{
	AG_Button *bu;

	bu = CreateToolbarButton(mi, icon, text);
	AG_ButtonSetSticky(bu, 1);
	AG_ButtonInvertState(bu, inv);
	return (bu);
}

/* Create a menu item associated with a function. */
AG_MenuItem *
AG_MenuAction(AG_MenuItem *pitem, const char *text, const AG_Surface *icon,
    AG_EventFn fn, const char *fmt, ...)
{
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, text, icon);
	mi->clickFn = AG_SetEvent(pitem->pmenu, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(mi->clickFn, fmt);
	if (pitem->pmenu->curToolbar != NULL) {
		AG_Event *buEv;
		mi->tbButton = CreateToolbarButton(pitem, icon, text);
		buEv = AG_SetEvent(mi->tbButton, "button-pushed", fn, NULL);
		AG_EVENT_GET_ARGS(buEv, fmt);
	}
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

AG_MenuItem *
AG_MenuActionKb(AG_MenuItem *pitem, const char *text, const AG_Surface *icon,
    AG_KeySym key, AG_KeyMod kmod, AG_EventFn fn, const char *fmt, ...)
{
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, text, icon);
	mi->key_equiv = key;
	mi->key_mod = kmod;
	mi->clickFn = AG_SetEvent(pitem->pmenu, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(mi->clickFn, fmt);
	
	if (pitem->pmenu->curToolbar != NULL) {
		AG_Event *buEv;
		mi->tbButton = CreateToolbarButton(pitem, icon, text);
		buEv = AG_SetEvent(mi->tbButton, "button-pushed", fn, NULL);
		AG_EVENT_GET_ARGS(buEv, fmt);
	}
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

AG_MenuItem *
AG_MenuTool(AG_MenuItem *pitem, AG_Toolbar *tbar, const char *text,
    const AG_Surface *icon, AG_KeySym key, AG_KeyMod kmod,
    void (*fn)(AG_Event *), const char *fmt, ...)
{
	AG_MenuItem *mi;
	AG_Button *bu;
	AG_Event *btn_ev;
	
	AG_ObjectLock(pitem->pmenu);
	AG_ObjectLock(tbar);

	if (icon != NULL) {
		bu = AG_ButtonNewS(tbar->rows[0], 0, NULL);
		AG_ButtonSurface(bu, icon);
	} else {
		bu = AG_ButtonNewS(tbar->rows[0], 0, text);
	}
	AG_ButtonSetFocusable(bu, 0);
	btn_ev = AG_SetEvent(bu, "button-pushed", fn, NULL);
	AG_EVENT_GET_ARGS(btn_ev, fmt);
	tbar->nButtons++;

	mi = CreateItem(pitem, text, icon);
	mi->key_equiv = key;
	mi->key_mod = kmod;
	mi->clickFn = AG_SetEvent(pitem->pmenu, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(mi->clickFn, fmt);
	
	AG_ObjectUnlock(tbar);
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

AG_MenuItem *
AG_MenuIntBoolMp(AG_MenuItem *pitem, const char *text, const AG_Surface *icon,
    int *pBool, int inv, AG_Mutex *lock)
{
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, text, icon);
	mi->bind_type = AG_MENU_INT_BOOL;
	mi->bind_p = (void *)pBool;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	if (pitem->pmenu->curToolbar != NULL) {
		mi->tbButton = CreateToolbarButtonBool(pitem, icon, text, inv);
		AG_BindIntMp(mi->tbButton, "state", pBool, lock);
		AG_ButtonInvertState(mi->tbButton, inv);
	}
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

AG_MenuItem *
AG_MenuInt8BoolMp(AG_MenuItem *pitem, const char *text, const AG_Surface *icon,
    Uint8 *pBool, int inv, AG_Mutex *lock)
{
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, text, icon);
	mi->bind_type = AG_MENU_INT8_BOOL;
	mi->bind_p = (void *)pBool;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	if (pitem->pmenu->curToolbar != NULL) {
		mi->tbButton = CreateToolbarButtonBool(pitem, icon, text, inv);
		AG_BindUint8Mp(mi->tbButton, "state", pBool, lock);
		AG_ButtonInvertState(mi->tbButton, inv);
	}
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

AG_MenuItem *
AG_MenuIntFlagsMp(AG_MenuItem *pitem, const char *text, const AG_Surface *icon,
    int *pFlags, int flags, int inv, AG_Mutex *lock)
{
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, text, icon);
	mi->bind_type = AG_MENU_INT_FLAGS;
	mi->bind_p = (void *)pFlags;
	mi->bind_flags = flags;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	if (pitem->pmenu->curToolbar != NULL) {
		mi->tbButton = CreateToolbarButtonBool(pitem, icon, text, inv);
		AG_BindFlagMp(mi->tbButton, "state", (Uint *)pFlags,
		    (Uint)flags, lock);
		AG_ButtonInvertState(mi->tbButton, inv);
	}
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

AG_MenuItem *
AG_MenuInt8FlagsMp(AG_MenuItem *pitem, const char *text, const AG_Surface *icon,
    Uint8 *pFlags, Uint8 flags, int inv, AG_Mutex *lock)
{
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, text, icon);
	mi->bind_type = AG_MENU_INT8_FLAGS;
	mi->bind_p = (void *)pFlags;
	mi->bind_flags = flags;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	if (pitem->pmenu->curToolbar != NULL) {
		mi->tbButton = CreateToolbarButtonBool(pitem, icon, text, inv);
		AG_BindFlag8Mp(mi->tbButton, "state", pFlags, flags, lock);
		AG_ButtonInvertState(mi->tbButton, inv);
	}
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

AG_MenuItem *
AG_MenuInt16FlagsMp(AG_MenuItem *pitem, const char *text, const AG_Surface *icon,
    Uint16 *pFlags, Uint16 flags, int inv, AG_Mutex *lock)
{
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, text, icon);
	mi->bind_type = AG_MENU_INT16_FLAGS;
	mi->bind_p = (void *)pFlags;
	mi->bind_flags = flags;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	if (pitem->pmenu->curToolbar != NULL) {
		mi->tbButton = CreateToolbarButtonBool(pitem, icon, text, inv);
		AG_BindFlag16Mp(mi->tbButton, "state", pFlags, flags, lock);
		AG_ButtonInvertState(mi->tbButton, inv);
	}
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

AG_MenuItem *
AG_MenuInt32FlagsMp(AG_MenuItem *pitem, const char *text, const AG_Surface *icon, 
    Uint32 *pFlags, Uint32 flags, int inv, AG_Mutex *lock)
{
	AG_MenuItem *mi;

	AG_ObjectLock(pitem->pmenu);
	mi = CreateItem(pitem, text, icon);
	mi->bind_type = AG_MENU_INT32_FLAGS;
	mi->bind_p = (void *)pFlags;
	mi->bind_flags = flags;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	if (pitem->pmenu->curToolbar != NULL) {
		mi->tbButton = CreateToolbarButtonBool(pitem, icon, text, inv);
		AG_BindFlag32Mp(mi->tbButton, "state", pFlags, flags, lock);
		AG_ButtonInvertState(mi->tbButton, inv);
	}
	AG_ObjectUnlock(pitem->pmenu);
	return (mi);
}

void
AG_MenuSetIntBoolMp(AG_MenuItem *mi, int *pBool, int inv, AG_Mutex *lock)
{
	AG_ObjectLock(mi->pmenu);
	mi->bind_type = AG_MENU_INT_BOOL;
	mi->bind_p = (void *)pBool;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	if (mi->tbButton != NULL) {
		AG_BindIntMp(mi->tbButton, "state", pBool, lock);
		AG_ButtonInvertState(mi->tbButton, inv);
		AG_ButtonSetSticky(mi->tbButton, 1);
	}
	AG_ObjectUnlock(mi->pmenu);
}

void
AG_MenuSetIntFlagsMp(AG_MenuItem *mi, int *pFlags, int flags, int inv,
    AG_Mutex *lock)
{
	AG_ObjectLock(mi->pmenu);
	mi->bind_type = AG_MENU_INT_FLAGS;
	mi->bind_p = (void *)pFlags;
	mi->bind_flags = flags;
	mi->bind_invert = inv;
	mi->bind_lock = lock;
	if (mi->tbButton != NULL) {
		AG_BindFlag(mi->tbButton, "state", (Uint *)pFlags, (Uint)flags);
		AG_ButtonInvertState(mi->tbButton, inv);
		AG_ButtonSetSticky(mi->tbButton, 1);
	}
	AG_ObjectUnlock(mi->pmenu);
}

/*
 * Free a menu item's children.
 * The parent AG_Menu must be locked.
 */
static void
AG_MenuItemFreeChildren(AG_MenuItem *mi)
{
	AG_MenuItem *miSub, *miSubNext;

	for (miSub = TAILQ_FIRST(&mi->subItems);
	     miSub != TAILQ_END(&mi->subItems);
	     miSub = miSubNext) {
		miSubNext = TAILQ_NEXT(miSub, items);
		AG_MenuItemFree(miSub);
	}
	TAILQ_INIT(&mi->subItems);
	mi->nSubItems = 0;
}

/*
 * Free a menu item and its children.
 * The parent AG_Menu must be locked.
 */
void
AG_MenuItemFree(AG_MenuItem *mi)
{
	AG_MenuItemFreeChildren(mi);

	if (mi->iconSrc != NULL) {
		AG_SurfaceFree(mi->iconSrc);
	}
	Free(mi->text);
	free(mi);
}

/* Delete a menu item and its children. */
void
AG_MenuDel(AG_MenuItem *mi)
{
	AG_Menu *m = mi->pmenu;
	AG_MenuItem *miParent = mi->parent;

	AG_ObjectLock(m);

	TAILQ_REMOVE(&miParent->subItems, mi, items);
	miParent->nSubItems--;
	AG_MenuItemFree(mi);

	AG_ObjectUnlock(m);
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
	AG_Menu *m = mi->pmenu;

	AG_ObjectLock(m);
	if (mi->poll != NULL) {
		InvalidateLabelSurfaces(mi);
		AG_MenuItemFreeChildren(mi);
		AG_PostEventByPtr(mi, m, mi->poll, NULL);
	}
	AG_ObjectUnlock(m);
}

void
AG_MenuState(AG_MenuItem *mi, int state)
{
	AG_Menu *m = mi->pmenu;

	AG_ObjectLock(m);
	m->curState = state;
	AG_ObjectUnlock(m);
	AG_Redraw(m);
}

void
AG_MenuToolbar(AG_MenuItem *mi, AG_Toolbar *tb)
{
	AG_Menu *m = mi->pmenu;

	AG_ObjectLock(m);
	m->curToolbar = tb;
	AG_ObjectUnlock(m);
}

static void
Draw(void *obj)
{
	AG_Menu *m = obj;
	AG_MenuItem *mi;
	int lbl, wLbl, hLbl;

	AG_DrawBox(m,
	    AG_RECT(0, 0, WIDTH(m), HEIGHT(m)), 1,
	    WCOLOR(m,0));

	if (m->root == NULL)
		return;

	AG_PushClipRect(m, m->r);

	TAILQ_FOREACH(mi, &m->root->subItems, items) {
		int activeState = mi->stateFn ? mi->stateFn->fn.fnInt(mi->stateFn) :
		                                mi->state;
		if (activeState) {
			if (mi->lblMenu[1] == -1) {
				AG_TextColor(WCOLOR(m,TEXT_COLOR));
				mi->lblMenu[1] = (mi->text == NULL) ? -1 :
				    AG_WidgetMapSurface(m, AG_TextRender(mi->text));
			}
			lbl = mi->lblMenu[1];
		} else {
			if (mi->lblMenu[0] == -1) {
				AG_TextColor(WCOLOR_DIS(m,TEXT_COLOR));
				mi->lblMenu[0] = (mi->text == NULL) ? -1 :
				    AG_WidgetMapSurface(m, AG_TextRender(mi->text));
			}
			lbl = mi->lblMenu[0];
		}
		wLbl = WSURFACE(m,lbl)->w;
		hLbl = WSURFACE(m,lbl)->h;
		if (mi == m->itemSel) {
			AG_DrawRect(m,
			    AG_RECT(mi->x, mi->y,
	    		            m->lPadLbl + wLbl + m->rPadLbl,
	    		            m->tPadLbl + hLbl + m->bPadLbl),
			    WCOLOR_SEL(m,0));
		}
		AG_WidgetBlitSurface(m, lbl,
		    mi->x + m->lPadLbl,
		    mi->y + m->tPadLbl);
	}

	AG_PopClipRect(m);
}

static void
GetItemSize(AG_MenuItem *item, int *w, int *h)
{
	AG_Menu *m = item->pmenu;
	int lbl;

	if (item->lblMenu[1] != -1) {
		lbl = item->lblMenu[1];
	} else if (item->lblMenu[0] != -1) {
		lbl = item->lblMenu[0];
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
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Menu *m = obj;
	AG_Driver *drv = WIDGET(m)->drv;
	AG_MenuItem *mi;
	int x, y, wLbl, hLbl;
	Uint wView, hView;

	x = m->lPad;
	y = m->tPad;
	r->h = 0;
	r->w = x;

	AG_GetDisplaySize(drv, &wView, &hView);

	if (m->root == NULL) {
		return;
	}
	TAILQ_FOREACH(mi, &m->root->subItems, items) {
		GetItemSize(mi, &wLbl, &hLbl);
		if (r->h == 0) {
			r->h = m->tPad+hLbl+m->bPad;
		}
		if (x+wLbl > wView) {			/* Wrap */
			x = m->lPad;
			y += hLbl;
			r->h += hLbl+m->bPad;
		}
		if (r->w < MIN(x+wLbl,wView)) {
			r->w = MIN(x+wLbl,wView);
		}
		x += wLbl;
	}
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Menu *m = obj;
	AG_MenuItem *mi;
	int wLbl, hLbl;
	int x, y;
	
	if (a->w < (m->lPad + m->rPad) ||
	    a->h < (m->tPad + m->bPad)) {
		return (-1);
	}
	m->r.x = m->lPad;
	m->r.y = m->tPad;
	m->r.w = a->w - m->rPad;
	m->r.h = a->h - m->bPad;

	if (m->root == NULL) {
		return (-1);
	}
	x = m->lPad;
	y = m->tPad;
	TAILQ_FOREACH(mi, &m->root->subItems, items) {
		GetItemSize(mi, &wLbl, &hLbl);
		mi->x = x;
		mi->y = y;
		if (x+wLbl > a->w) {
			mi->x = m->lPad;
			mi->y += hLbl;
			y += hLbl;
		}
		x += wLbl;
	}
	return (0);
}

AG_PopupMenu *
AG_PopupNew(void *obj)
{
	AG_Widget *wid = obj;
	AG_PopupMenu *pm;

	pm = Malloc(sizeof(AG_PopupMenu));
	pm->widget = wid;
	pm->menu = AG_MenuNew(NULL, 0);
	pm->menu->style = AG_MENU_POPUP;
	pm->root = AG_MenuNode(pm->menu->root, NULL, NULL);
	pm->menu->itemSel = pm->root;
	pm->win = NULL;
#ifdef AG_LEGACY
	pm->item = pm->root;
#endif
	return (pm);
}

void
AG_PopupShow(AG_PopupMenu *pm)
{
	AG_Driver *drv;
	AG_Window *winParent;
	int x = 0, y = 0;

	AG_LockVFS(pm->widget);
	
	if ((drv = WIDGET(pm->widget)->drv) != NULL &&
	    (winParent = AG_ParentWindow(pm->widget)) != NULL) {
	    	x = drv->mouse->x;
	    	y = drv->mouse->y;
		if (AGDRIVER_SINGLE(drv)) {
			x -= WIDGET(winParent)->x;
			y -= WIDGET(winParent)->y;
		}
		AG_ObjectLock(pm->menu);
		pm->win = AG_MenuExpand(winParent, pm->root, x, y);
		AG_ObjectUnlock(pm->menu);
	}

	AG_UnlockVFS(pm->widget);
}

void
AG_PopupShowAt(AG_PopupMenu *pm, int x, int y)
{
	AG_LockVFS(pm->widget);
	AG_ObjectLock(pm->menu);
	pm->win = AG_MenuExpand(pm->widget, pm->root, x, y);
	AG_ObjectUnlock(pm->menu);
	AG_UnlockVFS(pm->widget);
}

static void
PopupHideAll(AG_MenuItem *mi)
{
	AG_MenuItem *miSub;

	if (mi->view != NULL) {
		AG_WindowHide(WIDGET(mi->view)->window);
	}
	TAILQ_FOREACH(miSub, &mi->subItems, items)
		PopupHideAll(miSub);
}

void
AG_PopupHide(AG_PopupMenu *pm)
{
	AG_ObjectLock(pm->menu);
	PopupHideAll(pm->root);
	AG_ObjectUnlock(pm->menu);
}

void
AG_PopupDestroy(AG_PopupMenu *pm)
{
	AG_ObjectDestroy(pm->menu);
	free(pm);
}

AG_WidgetClass agMenuClass = {
	{
		"Agar(Widget:Menu)",
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
