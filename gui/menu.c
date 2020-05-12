/*
 * Copyright (c) 2004-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Menu bar widget (and base AG_Menu(3) API).
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/menu.h>
#include <agar/gui/primitive.h>
#include <agar/gui/label.h>
#include <agar/gui/button.h>

#include <stdarg.h>
#include <string.h>

/* #define DEBUG_EXPAND */

static int agMenuCounter = 0;
AG_Menu   *agAppMenu = NULL;		/* Global application menu (SW mode) */
AG_Window *agAppMenuWin = NULL;		/* Application menu window */

static void GetItemSize(AG_MenuItem *_Nonnull, int *_Nonnull, int *_Nonnull);

/* Create a new Menu widget. */
AG_Menu *
AG_MenuNew(void *parent, Uint flags)
{
	AG_Menu *m;

	m = Malloc(sizeof(AG_Menu));
	AG_ObjectInit(m, &agMenuClass);

	if (flags & AG_MENU_HFILL) { WIDGET(m)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_MENU_VFILL) { WIDGET(m)->flags |= AG_WIDGET_VFILL; }
	m->flags |= flags;

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
	Uint wFlags = AG_WINDOW_KEEPBELOW | AG_WINDOW_DENYFOCUS;

	if (agAppMenu)
		goto exists;

	if (agDriverSw) {
		wFlags |= AG_WINDOW_PLAIN | AG_WINDOW_HMAXIMIZE;
	}
	if ((win = AG_WindowNewNamedS(wFlags, "_agAppMenu")) == NULL) {
		goto exists;
	}
	win->wmType = AG_WINDOW_WM_DOCK;
	if (win == NULL) {
		goto exists;
	}
	AG_SetStyle(win, "padding", "0");
	AG_WindowSetCaptionS(win, agProgName ? agProgName : "agarapp");

	m = AG_MenuNew(win, flags);
	m->style = AG_MENU_GLOBAL;
/*	AG_SetStyleF(m, "padding", "4 0 4 0") */
	WIDGET(m)->flags |= AG_WIDGET_HFILL;

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
	return (m);
exists:
	AG_SetErrorS("appMenu exists");
	return (NULL);
}

static void
OnWindowClose(AG_Event *_Nonnull event)
{
	AG_Menu *m = AG_MENU_PTR(1);

#ifdef DEBUG_EXPAND
	Debug(m, "Collapse (by `window-close')\n");
#endif
	AG_MenuCollapseAll(m);
}

static void
OnMouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_SELF();
	AG_Menu *m = AG_MENU_PTR(1);
	const int x = AG_INT(2);
	const int y = AG_INT(3);

	if (x < 0 || y < 0 || x > WIDTH(win) || y > HEIGHT(win)) {
#ifdef DEBUG_EXPAND
		Debug(m, "Collapse (clicked %d,%d)\n", x,y);
#endif
		AG_MenuCollapseAll(m);
	}
}

/*
 * Test whether x,y intersects a menu item (and also store the height of
 * the item's label in hLbl).
 */
static __inline__ int
IntersectItem(AG_MenuItem *_Nonnull mi, int x, int y, int *_Nonnull hLbl)
{
	AG_Menu *m = mi->pmenu;
	int lbl, wLbl;

	lbl = (mi->lblMenu[1] != -1) ? mi->lblMenu[1] :
	      (mi->lblMenu[0] != -1) ? mi->lblMenu[0] :
	      -1;
	if (lbl != -1) {
		wLbl = WSURFACE(m,lbl)->w + m->lPadLbl + m->rPadLbl;
		*hLbl = WSURFACE(m,lbl)->h + m->tPadLbl + m->bPadLbl + 1;
	} else {
		wLbl = 0;
		*hLbl = 0;
	}
	return (x >= mi->x && x < (mi->x + wLbl) &&
		y >= mi->y && y < (mi->y + m->itemh));
}

static void
MouseMotion(AG_Event *_Nonnull event)
{
	AG_Menu *m = AG_MENU_SELF();
	const int x = AG_INT(1);
	const int y = AG_INT(2);
	const int bPad = WIDGET(m)->paddingBottom - 1;
	AG_MenuItem *mi;
	int hLbl;

	if (!m->selecting || y < 0 || y >= HEIGHT(m)-1)
		return;

	TAILQ_FOREACH(mi, &m->root->subItems, items) {
		if (!IntersectItem(mi, x,y, &hLbl)) {
			continue;
		}
	    	if (mi != m->itemSel) {
			if (m->itemSel) {
				AG_MenuCollapse(m->itemSel);
			}
			m->itemSel = mi;
			AG_MenuExpand(m, mi,
			    mi->x,
			    mi->y + hLbl + bPad);
		}
		AG_Redraw(m);
		break;
	}
}

/*
 * Expand an AG_MenuItem. Create a window containing the AG_MenuView
 * at coordinates x1,y1 (relative to widget parent).
 *
 * If not null, parent can be either an AG_Menu or an AG_MenuView.
 * Where unspecified, the parent AG_Menu of the MenuItem is used.

 * The associated AG_Menu object must be locked.
 */
AG_Window *
AG_MenuExpand(void *parent, AG_MenuItem *mi, int x1, int y1)
{
	AG_Window *win, *winParent;
	AG_Menu *m;
	int x = x1;
	int y = y1;
	Uint winFlags;

	AG_MENU_ITEM_IS_VALID(mi);

	if (parent) {
		if (AG_OfClass(parent, "AG_Widget:AG_MenuView")) {
			m = ((AG_MenuView *)parent)->pmenu;
		} else if (AG_OfClass(parent, "AG_Widget:AG_Menu")) {
			m = parent;
		} else {
			m = mi->pmenu;
		}
		AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");

		/* Convert to absolute coordinates for AG_WindowSetGeometry(). */
		x += WIDGET(parent)->rView.x1;
		y += WIDGET(parent)->rView.y1;
		if ((winParent = WIDGET(parent)->window) == NULL) {
			AG_FatalError("AG_MenuExpand: No parent window");
		}
		AG_OBJECT_ISA(winParent, "AG_Widget:AG_Window:*");
		if (WIDGET(winParent)->drv && AGDRIVER_MULTIPLE(WIDGET(winParent)->drv)) {
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

	if (mi->view != NULL) {                                   /* Cached */
		win = WIDGET(mi->view)->window;
		AG_OBJECT_ISA(win, "AG_Widget:AG_Window:*");
#ifdef DEBUG_EXPAND
		Debug(m, "Expand [%s] -> " AGSI_RED "%s" AGSI_RST " (cached)\n",
		    mi->text, OBJECT(win)->name);
#endif
/*		AG_WidgetCompileStyle(win); */
		AG_WindowSetGeometry(win, x,y, -1,-1);
		AG_WindowRaise(win);
		AG_WindowShow(win);
		return (win);
	}

	winFlags = AG_WINDOW_MODAL | AG_WINDOW_NOTITLE |
	           AG_WINDOW_NOBORDERS | AG_WINDOW_NORESIZE |
	           AG_WINDOW_DENYFOCUS;

	if (agDriverSw)
		winFlags |= AG_WINDOW_NOBACKGROUND;

	if ((win = AG_WindowNew(winFlags)) == NULL) {
		return (NULL);
	}
	win->wmType = (m->style == AG_MENU_DROPDOWN) ?
	              AG_WINDOW_WM_DROPDOWN_MENU :
		      AG_WINDOW_WM_POPUP_MENU;
	AG_ObjectSetName(win, "_menu%u", agMenuCounter++);
	AG_SetStyle(win, "padding", "0");

#ifdef DEBUG_EXPAND
	Debug(m, "Expand [%s] -> " AGSI_BR_RED "%s" AGSI_RST " (new)\n",
	    mi->text, OBJECT(win)->name);
#endif
	/* Collapse if user clicks outside of the window boundaries. */
	WIDGET(win)->flags |= AG_WIDGET_UNFOCUSED_BUTTONDOWN;
	AG_SetEvent(win, "mouse-button-down", OnMouseButtonDown, "%p", m);
	AG_SetEvent(win, "window-close", OnWindowClose, "%p", m);
	
	mi->view = Malloc(sizeof(AG_MenuView));
	AG_ObjectInit(mi->view, &agMenuViewClass);
	mi->view->pmenu = m;
	mi->view->pitem = mi;
	AG_ObjectAttach(win, mi->view);
	AG_WidgetCompileStyle(win);

	if (winParent) {
		AG_WindowAttach(winParent, win);
		AG_WindowMakeTransient(winParent, win);
		AG_WindowPin(winParent, win);
	}
	AG_WindowSetGeometry(win, x,y, -1,-1);
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

	AG_MENU_ITEM_IS_VALID(mi);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	TAILQ_FOREACH(miSub, &mi->subItems, items)
		AG_MenuCollapse(miSub);

	if (mi->view) {
		AG_WindowHide(WIDGET(mi->view)->window);
	}
	mi->sel_subitem = NULL;
	
	AG_ObjectUnlock(m);
}

static void
CollapseAll(AG_MenuItem *_Nonnull mi)
{
	AG_MenuItem *miSub;

	AG_MENU_ITEM_IS_VALID(mi);

	TAILQ_FOREACH(miSub, &mi->subItems, items) {
		CollapseAll(miSub);
	}
	if (mi->view)
		AG_MenuCollapse(mi);
}

/*
 * Collapse the window displaying the contents of an item as well as
 * all expanded MenuView windows, up to the menu root.
 */
void
AG_MenuCollapseAll(AG_Menu *m)
{
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	CollapseAll(m->root);
	m->itemSel = NULL;
	m->selecting = 0;
	
	AG_ObjectUnlock(m);
}

void
AG_MenuSetLabelPadding(AG_Menu *m, int lPad, int rPad, int tPad, int bPad)
{
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	if (lPad != -1) { m->lPadLbl = lPad; }
	if (rPad != -1) { m->rPadLbl = rPad; }
	if (tPad != -1) { m->tPadLbl = tPad; }
	if (bPad != -1) { m->bPadLbl = bPad; }
	AG_Redraw(m);
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Menu *m = AG_MENU_SELF();
	const int x = AG_INT(2);
	const int y = AG_INT(3);
	const int bPad = WIDGET(m)->paddingBottom;
	AG_MenuItem *mi;
	int hLbl;

	TAILQ_FOREACH(mi, &m->root->subItems, items) {
		if (!IntersectItem(mi, x,y, &hLbl)) {
			continue;
		}
	    	if (m->itemSel == mi) {
			AG_MenuCollapseAll(m);
			m->itemSel = NULL;
			m->selecting = 0;
		} else {
			if (m->itemSel) {
				AG_MenuCollapse(m->itemSel);
			}
			m->itemSel = mi;
			AG_MenuExpand(m, mi,
			    mi->x,
			    mi->y + hLbl + bPad);
			m->selecting = 1;
		}
		AG_Redraw(m);
		break;
	}
}

#if 0
static void
OnAttach(AG_Event *_Nonnull event)
{
	AG_Widget *pwid = AG_PTR(1);
	AG_Window *win;
	
	AG_OBJECT_ISA(pwid, "AG_Widget:*");

	if ((win = AG_ParentWindow(pwid)) != NULL)
		AG_WindowSetPadding(win, -1, -1, 0, win->bPad);
}
#endif

/* Parent Menu (if any) must be locked. */
static AG_MenuItem *_Nonnull
CreateItem(AG_MenuItem *_Nullable miParent, const char *_Nullable text,
    const AG_Surface *_Nullable icon)
{
	AG_Menu *pmenu = (miParent) ? miParent->pmenu : NULL;
	AG_MenuItem *mi;
	
	mi = Malloc(sizeof(AG_MenuItem));
#ifdef AG_TYPE_SAFETY
	Strlcpy(mi->tag, AG_MENU_ITEM_TAG, sizeof(mi->tag));
#endif
	mi->text = (text) ? Strdup(text) : Strdup("");
	mi->lblMenu[1] = mi->lblMenu[0] = -1;
	mi->lblView[1] = mi->lblView[0] = -1;
	mi->iconSrc = (icon) ? AG_SurfaceDup(icon) : NULL;  /* TODO shared */
	mi->icon = -1;
	mi->value = -1;
	mi->stateFn = NULL;
	mi->state = (pmenu) ? pmenu->curState : 1;
	mi->key_equiv = 0;
	mi->key_mod = 0;
	mi->x = 0;
	mi->y = (miParent && pmenu) ?
	    (miParent->nSubItems * pmenu->itemh) - pmenu->itemh : 0;
	mi->flags = 0;
	mi->clickFn = NULL;
	mi->poll = NULL;
	mi->bind_type = AG_MENU_NO_BINDING;
	mi->bind_flags = 0;
#ifdef AG_THREADS
	mi->bind_lock = NULL;
#endif
	mi->view = NULL;
	mi->pmenu = pmenu;
	mi->sel_subitem = NULL;
	mi->tbButton = NULL;
	mi->parent = miParent;
	TAILQ_INIT(&mi->subItems);
	mi->nSubItems = 0;
	
	if (miParent) {
		if (icon) {
			miParent->flags |= AG_MENU_ITEM_ICONS;
		}
		TAILQ_INSERT_TAIL(&miParent->subItems, mi, items);
		miParent->nSubItems++;

		if (pmenu && (pmenu->style == AG_MENU_GLOBAL) &&
		    agDriverSw && agAppMenuWin) {
			Uint wMax, hMax;
			AG_SizeReq rMenu;

			AG_GetDisplaySize(agDriverSw, &wMax, &hMax);
			AG_WidgetSizeReq(pmenu, &rMenu);
			AG_WindowSetGeometry(agAppMenuWin, 0,0, wMax, rMenu.h);
			AG_Redraw(pmenu);
		}
	}
	return (mi);
}

static void
StyleChanged(AG_Event *_Nonnull event)
{
	AG_Menu *m = AG_MENU_SELF();
	const AG_Font *font = WFONT(m);
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
	m->itemh = font->lineskip + m->tPadLbl + m->bPadLbl;
}

static void
Init(void *_Nonnull obj)
{
	AG_Menu *m = obj;

	WIDGET(m)->flags |= AG_WIDGET_UNFOCUSED_MOTION |
	                    AG_WIDGET_UNFOCUSED_BUTTONUP |
			    AG_WIDGET_USE_TEXT;
	m->flags = 0;
	m->style = AG_MENU_DROPDOWN;
	m->root = CreateItem(NULL, NULL, NULL);
	m->root->pmenu = m;
	m->selecting = 0;

	m->lPadLbl=6;  m->rPadLbl=7;
	m->tPadLbl=3;  m->bPadLbl=3;

	m->itemh = agTextFontLineSkip + m->tPadLbl + m->bPadLbl;
	m->curState = 1;

	m->itemSel = NULL;
	m->curToolbar = NULL;
	m->r.x = 0;
	m->r.y = 0;
	m->r.w = 0;
	m->r.h = 0;

	AG_SetEvent(m, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(m, "mouse-motion", MouseMotion, NULL);
/*	AG_AddEvent(m, "attached", OnAttach, NULL); */
	AG_AddEvent(m, "font-changed", StyleChanged, NULL);
	AG_AddEvent(m, "palette-changed", StyleChanged, NULL);
}

/* Change the icon associated with a menu item. */
void
AG_MenuSetIcon(AG_MenuItem *mi, const AG_Surface *iconSrc)
{
	AG_Menu *m = mi->pmenu;

	AG_MENU_ITEM_IS_VALID(mi);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	if (mi->iconSrc) {
		AG_SurfaceFree(mi->iconSrc);
	}
	mi->iconSrc = iconSrc ? AG_SurfaceDup(iconSrc) : NULL;

	if (mi->icon != -1 &&
	    mi->parent &&
	    mi->parent->view) {
		AG_WidgetUnmapSurface(mi->parent->view, mi->icon);
		mi->icon = -1;
	}

	AG_Redraw(m);
	AG_ObjectUnlock(m);
}

/* Change menu item text (format string). */
void
AG_MenuSetLabel(AG_MenuItem *mi, const char *fmt, ...)
{
	AG_Menu *m = mi->pmenu;
	va_list ap;
	
	AG_MENU_ITEM_IS_VALID(mi);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	va_start(ap, fmt);
	Free(mi->text);
	Vasprintf(&mi->text, fmt, ap);
	va_end(ap);

	AG_MenuInvalidateLabels(mi);

	AG_Redraw(m);
	AG_ObjectUnlock(m);
}

/* Change menu item text (C string). */
void
AG_MenuSetLabelS(AG_MenuItem *mi, const char *s)
{
	AG_Menu *m = mi->pmenu;

	AG_MENU_ITEM_IS_VALID(mi);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	Free(mi->text);
	mi->text = Strdup(s);
	AG_MenuInvalidateLabels(mi);

	AG_Redraw(m);
	AG_ObjectUnlock(m);
}

/* Create a menu separator. */
AG_MenuItem *
AG_MenuSeparator(AG_MenuItem *pitem)
{
	AG_MenuItem *mi;
	AG_Menu *m = pitem->pmenu;

	AG_MENU_ITEM_IS_VALID(pitem);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);
	
	mi = CreateItem(pitem, NULL, NULL);
	mi->flags |= (AG_MENU_ITEM_NOSELECT | AG_MENU_ITEM_SEPARATOR);

	if (m->curToolbar) {
		AG_ToolbarSeparator(m->curToolbar);
	}
	AG_ObjectUnlock(m);
	return (mi);
}

/* Create a menu section label (format string). */
AG_MenuItem *
AG_MenuSection(AG_MenuItem *pitem, const char *fmt, ...)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;
	char *text;
	va_list ap;

	va_start(ap, fmt);
	Vasprintf(&text, fmt, ap);
	va_end(ap);

	AG_MENU_ITEM_IS_VALID(pitem);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	mi = CreateItem(pitem, text, NULL);
	mi->flags |= AG_MENU_ITEM_NOSELECT;

	AG_ObjectUnlock(m);
	free(text);
	return (mi);
}

/* Create a menu section label (C string). */
AG_MenuItem *
AG_MenuSectionS(AG_MenuItem *pitem, const char *label)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;

	AG_MENU_ITEM_IS_VALID(pitem);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	mi = CreateItem(pitem, label, NULL);
	mi->flags |= AG_MENU_ITEM_NOSELECT;

	AG_ObjectUnlock(m);
	return (mi);
}

/* Create a dynamically-updated menu item. */
AG_MenuItem *
AG_MenuDynamicItem(AG_MenuItem *pitem, const char *text, const AG_Surface *icon,
    AG_EventFn fn, const char *fmt, ...)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;

	AG_MENU_ITEM_IS_VALID(pitem);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	mi = CreateItem(pitem, text, icon);
	mi->poll = AG_SetEvent(m, NULL, fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(mi->poll, fmt, ap);
		va_end(ap);
	}
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

	AG_MENU_ITEM_IS_VALID(pitem);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	mi = CreateItem(pitem, text, icon);
	mi->key_equiv = key;
	mi->key_mod = kmod;
	mi->poll = AG_SetEvent(m, NULL, fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(mi->poll, fmt, ap);
		va_end(ap);
	}
	AG_ObjectUnlock(m);
	return (mi);
}

/* Set a dynamic update function for an existing menu item. */
void
AG_MenuSetPollFn(AG_MenuItem *mi, AG_EventFn fn, const char *fmt, ...)
{
	AG_Menu *m = mi->pmenu;

	AG_MENU_ITEM_IS_VALID(mi);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	if (mi->poll) {
		AG_UnsetEvent(m, mi->poll->name);
	}
	mi->poll = AG_SetEvent(m, NULL, fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(mi->poll, fmt, ap);
		va_end(ap);
	}
	AG_ObjectUnlock(m);
}

/* Create a menu item without any associated action. */
AG_MenuItem *
AG_MenuNode(AG_MenuItem *pitem, const char *text, const AG_Surface *icon)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *node;
	
	AG_MENU_ITEM_IS_VALID(pitem);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	node = CreateItem(pitem, text, icon);

	AG_ObjectUnlock(m);
	return (node);
}

static AG_Button *_Nonnull
CreateToolbarButton(AG_MenuItem *_Nonnull mi, const AG_Surface *_Nullable icon,
    const char *_Nullable text)
{
	AG_Menu *m = mi->pmenu;
	AG_Button *bu;

	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");

	if (icon) {
		bu = AG_ButtonNewS(m->curToolbar->rows[0], AG_BUTTON_NO_FOCUS, NULL);
		AG_ButtonSurface(bu, icon);
	} else {
		bu = AG_ButtonNewS(m->curToolbar->rows[0], AG_BUTTON_NO_FOCUS, text);
	}
	m->curToolbar->nButtons++;
	mi->tbButton = bu;
	return (bu);
}

static __inline__ AG_Button *_Nonnull
CreateToolbarButtonBool(AG_MenuItem *_Nonnull mi,
    const AG_Surface *_Nullable icon, const char *_Nullable text, int inv)
{
	AG_Button *bu;

	bu = CreateToolbarButton(mi, icon, text);
	AG_ButtonSetSticky(bu, 1);
	AG_ButtonSetInverted(bu, inv);
	return (bu);
}

/* Create a menu item associated with a function. */
AG_MenuItem *
AG_MenuAction(AG_MenuItem *pitem, const char *text, const AG_Surface *icon,
    AG_EventFn fn, const char *fmt, ...)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;
	
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	mi = CreateItem(pitem, text, icon);
	mi->clickFn = AG_SetEvent(m, NULL, fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(mi->clickFn, fmt, ap);
		va_end(ap);
	}

	if (m->curToolbar) {
		AG_Event *buEv;

		mi->tbButton = CreateToolbarButton(pitem, icon, text);
		buEv = AG_SetEvent(mi->tbButton, "button-pushed", fn, NULL);
		if (fmt) {
			va_list ap;

			va_start(ap, fmt);
			AG_EventGetArgs(buEv, fmt, ap);
			va_end(ap);
		}
	}
	AG_ObjectUnlock(m);
	return (mi);
}

AG_MenuItem *
AG_MenuActionKb(AG_MenuItem *pitem, const char *text, const AG_Surface *icon,
    AG_KeySym key, AG_KeyMod kmod, AG_EventFn fn, const char *fmt, ...)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;
	
	AG_MENU_ITEM_IS_VALID(pitem);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	mi = CreateItem(pitem, text, icon);
	mi->key_equiv = key;
	mi->key_mod = kmod;
	mi->clickFn = AG_SetEvent(m, NULL, fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(mi->clickFn, fmt, ap);
		va_end(ap);
	}

	if (m->curToolbar) {
		AG_Event *buEv;

		mi->tbButton = CreateToolbarButton(pitem, icon, text);
		buEv = AG_SetEvent(mi->tbButton, "button-pushed", fn, NULL);
		if (fmt) {
			va_list ap;

			va_start(ap, fmt);
			AG_EventGetArgs(buEv, fmt, ap);
			va_end(ap);
		}
	}
	AG_ObjectUnlock(m);
	return (mi);
}

AG_MenuItem *
AG_MenuTool(AG_MenuItem *pitem, AG_Toolbar *toolbar, const char *text,
    const AG_Surface *icon, AG_KeySym key, AG_KeyMod kmod,
    void (*fn)(AG_Event *), const char *fmt, ...)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;
	AG_Button *bu;
	AG_Event *btn_ev;
	
	AG_MENU_ITEM_IS_VALID(pitem);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);
	AG_OBJECT_ISA(toolbar, "AG_Widget:AG_Box:AG_Toolbar:*");
	AG_ObjectLock(toolbar);

	if (icon) {
		bu = AG_ButtonNewS(toolbar->rows[0], AG_BUTTON_NO_FOCUS, NULL);
		AG_ButtonSurface(bu, icon);
	} else {
		bu = AG_ButtonNewS(toolbar->rows[0], AG_BUTTON_NO_FOCUS, text);
	}
	btn_ev = AG_SetEvent(bu, "button-pushed", fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(btn_ev, fmt, ap);
		va_end(ap);
	}
	toolbar->nButtons++;

	mi = CreateItem(pitem, text, icon);
	mi->key_equiv = key;
	mi->key_mod = kmod;
	mi->clickFn = AG_SetEvent(m, NULL, fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(mi->clickFn, fmt, ap);
		va_end(ap);
	}
	
	AG_ObjectUnlock(toolbar);
	AG_ObjectUnlock(m);
	return (mi);
}

AG_MenuItem *
AG_MenuIntBoolMp(AG_MenuItem *pitem, const char *text, const AG_Surface *icon,
    int *pBool, int inv, AG_Mutex *lock)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;

	AG_MENU_ITEM_IS_VALID(pitem);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	mi = CreateItem(pitem, text, icon);
	mi->bind_type = AG_MENU_INT_BOOL;
	mi->bind_p = (void *)pBool;
	if (inv) { mi->flags |= AG_MENU_ITEM_INVERTED; }
#ifdef AG_THREADS
	mi->bind_lock = lock;
#endif
	if (m->curToolbar) {
		mi->tbButton = CreateToolbarButtonBool(pitem, icon, text, inv);
#ifdef AG_THREADS
		if (lock) {
			AG_BindIntMp(mi->tbButton, "state", pBool, lock);
		} else
#endif
		{
			AG_BindInt(mi->tbButton, "state", pBool);
		}
		AG_ButtonSetInverted(mi->tbButton, inv);
	}
	AG_ObjectUnlock(m);
	return (mi);
}

AG_MenuItem *
AG_MenuInt8BoolMp(AG_MenuItem *pitem, const char *text, const AG_Surface *icon,
    Uint8 *pBool, int inv, AG_Mutex *lock)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;

	AG_MENU_ITEM_IS_VALID(pitem);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	mi = CreateItem(pitem, text, icon);
	mi->bind_type = AG_MENU_INT8_BOOL;
	mi->bind_p = (void *)pBool;
	if (inv) { mi->flags |= AG_MENU_ITEM_INVERTED; }
#ifdef AG_THREADS
	mi->bind_lock = lock;
#endif
	if (m->curToolbar) {
		mi->tbButton = CreateToolbarButtonBool(pitem, icon, text, inv);
#ifdef AG_THREADS
		if (lock) {
			AG_BindUint8Mp(mi->tbButton, "state", pBool, lock);
		} else
#endif
		{
			AG_BindUint8(mi->tbButton, "state", pBool);
		}
		AG_ButtonSetInverted(mi->tbButton, inv);
	}
	AG_ObjectUnlock(m);
	return (mi);
}

AG_MenuItem *
AG_MenuIntFlagsMp(AG_MenuItem *pitem, const char *text, const AG_Surface *icon,
    int *pFlags, int flags, int inv, AG_Mutex *lock)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;

	AG_MENU_ITEM_IS_VALID(pitem);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	mi = CreateItem(pitem, text, icon);
	mi->bind_type = AG_MENU_INT_FLAGS;
	mi->bind_flags = flags;
	mi->bind_p = (void *)pFlags;
	if (inv) { mi->flags |= AG_MENU_ITEM_INVERTED; }
#ifdef AG_THREADS
	mi->bind_lock = lock;
#endif
	if (m->curToolbar) {
		mi->tbButton = CreateToolbarButtonBool(pitem, icon, text, inv);
#ifdef AG_THREADS
		if (lock) {
			AG_BindFlagMp(mi->tbButton, "state", (Uint *)pFlags,
			    (Uint)flags, lock);
		} else
#endif
		{
			AG_BindFlag(mi->tbButton, "state", (Uint *)pFlags,
			    (Uint)flags);
		}
		AG_ButtonSetInverted(mi->tbButton, inv);
	}
	AG_ObjectUnlock(m);
	return (mi);
}

AG_MenuItem *
AG_MenuInt8FlagsMp(AG_MenuItem *pitem, const char *text, const AG_Surface *icon,
    Uint8 *pFlags, Uint8 flags, int inv, AG_Mutex *lock)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;

	AG_MENU_ITEM_IS_VALID(pitem);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	mi = CreateItem(pitem, text, icon);
	mi->bind_type = AG_MENU_INT8_FLAGS;
	mi->bind_flags = flags;
	mi->bind_p = (void *)pFlags;
	if (inv) { mi->flags |= AG_MENU_ITEM_INVERTED; }
#ifdef AG_THREADS
	mi->bind_lock = lock;
#endif
	if (m->curToolbar) {
		mi->tbButton = CreateToolbarButtonBool(pitem, icon, text, inv);
#ifdef AG_THREADS
		if (lock) {
			AG_BindFlag8Mp(mi->tbButton, "state", pFlags, flags, lock);
		} else
#endif
		{
			AG_BindFlag8(mi->tbButton, "state", pFlags, flags);
		}
		AG_ButtonSetInverted(mi->tbButton, inv);
	}
	AG_ObjectUnlock(m);
	return (mi);
}

AG_MenuItem *
AG_MenuInt16FlagsMp(AG_MenuItem *pitem, const char *text, const AG_Surface *icon,
    Uint16 *pFlags, Uint16 flags, int inv, AG_Mutex *lock)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;
	
	AG_MENU_ITEM_IS_VALID(pitem);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	mi = CreateItem(pitem, text, icon);
	mi->bind_type = AG_MENU_INT16_FLAGS;
	mi->bind_flags = flags;
	mi->bind_p = (void *)pFlags;
	if (inv) { mi->flags |= AG_MENU_ITEM_INVERTED; }
# ifdef AG_THREADS
	mi->bind_lock = lock;
# endif
	if (m->curToolbar) {
		mi->tbButton = CreateToolbarButtonBool(pitem, icon, text, inv);
# ifdef AG_THREADS
		if (lock) {
			AG_BindFlag16Mp(mi->tbButton, "state", pFlags, flags,
			    lock);
		} else
# endif
		{
			AG_BindFlag16(mi->tbButton, "state", pFlags, flags);
		}
		AG_ButtonSetInverted(mi->tbButton, inv);
	}
	AG_ObjectUnlock(m);
	return (mi);
}

AG_MenuItem *
AG_MenuInt32FlagsMp(AG_MenuItem *pitem, const char *text, const AG_Surface *icon, 
    Uint32 *pFlags, Uint32 flags, int inv, AG_Mutex *lock)
{
	AG_Menu *m = pitem->pmenu;
	AG_MenuItem *mi;

	AG_MENU_ITEM_IS_VALID(pitem);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	mi = CreateItem(pitem, text, icon);
	mi->bind_type = AG_MENU_INT32_FLAGS;
	mi->bind_flags = flags;
	mi->bind_p = (void *)pFlags;
	if (inv) { mi->flags |= AG_MENU_ITEM_INVERTED; }
# ifdef AG_THREADS
	mi->bind_lock = lock;
# endif
	if (m->curToolbar) {
		mi->tbButton = CreateToolbarButtonBool(pitem, icon, text, inv);
# ifdef AG_THREADS
		if (lock) {
			AG_BindFlag32Mp(mi->tbButton, "state", pFlags, flags,
			    lock);
		} else
# endif
		{
			AG_BindFlag32(mi->tbButton, "state", pFlags, flags);
		}
		AG_ButtonSetInverted(mi->tbButton, inv);
	}
	AG_ObjectUnlock(m);
	return (mi);
}

void
AG_MenuSetIntBoolMp(AG_MenuItem *mi, int *pBool, int inv, AG_Mutex *lock)
{
	AG_Menu *m = mi->pmenu;

	AG_MENU_ITEM_IS_VALID(mi);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	mi->bind_type = AG_MENU_INT_BOOL;
	mi->bind_p = (void *)pBool;
	if (inv) { mi->flags |= AG_MENU_ITEM_INVERTED; }
#ifdef AG_THREADS
	mi->bind_lock = lock;
#endif
	if (mi->tbButton) {
#ifdef AG_THREADS
		if (lock) {
			AG_BindIntMp(mi->tbButton, "state", pBool, lock);
		} else
#endif
		{
			AG_BindInt(mi->tbButton, "state", pBool);
		}
		AG_ButtonSetInverted(mi->tbButton, inv);
		AG_ButtonSetSticky(mi->tbButton, 1);
	}
	AG_ObjectUnlock(m);
}

void
AG_MenuSetIntFlagsMp(AG_MenuItem *mi, int *pFlags, int flags, int inv,
    AG_Mutex *lock)
{
	AG_Menu *m = mi->pmenu;

	AG_MENU_ITEM_IS_VALID(mi);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	mi->bind_type = AG_MENU_INT_FLAGS;
	mi->bind_flags = flags;
	mi->bind_p = (void *)pFlags;
	if (inv) { mi->flags |= AG_MENU_ITEM_INVERTED; }
#ifdef AG_THREADS
	mi->bind_lock = lock;
#endif
	if (mi->tbButton) {
#ifdef AG_THREADS
		if (lock) {
			AG_BindFlagMp(mi->tbButton, "state", (Uint *)pFlags,
			    (Uint)flags, lock);
		} else
#endif
		{
			AG_BindFlag(mi->tbButton, "state", (Uint *)pFlags,
			    (Uint)flags);
		}
		AG_ButtonSetInverted(mi->tbButton, inv);
		AG_ButtonSetSticky(mi->tbButton, 1);
	}
	AG_ObjectUnlock(m);
}

/*
 * Free a menu item's children.
 * The parent AG_Menu must be locked.
 */
void
AG_MenuFreeSubitems(AG_MenuItem *_Nonnull mi)
{
	AG_MenuItem *miSub, *miSubNext;

	AG_MENU_ITEM_IS_VALID(mi);

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
	AG_MenuFreeSubitems(mi);

	if (mi->iconSrc) {
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

	AG_MENU_ITEM_IS_VALID(mi);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	TAILQ_REMOVE(&miParent->subItems, mi, items);
	miParent->nSubItems--;
	AG_MenuItemFree(mi);

	AG_ObjectUnlock(m);
}

static void
Destroy(void *_Nonnull p)
{
	AG_Menu *m = p;

	AG_MenuItemFree(m->root);
}

void
AG_MenuUpdateItem(AG_MenuItem *mi)
{
	AG_Menu *m = mi->pmenu;
	AG_Window *win;

	AG_MENU_ITEM_IS_VALID(mi);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	if (mi->poll) {
		AG_MenuInvalidateLabels(mi);
		AG_MenuFreeSubitems(mi);
		AG_PostEventByPtr(m, mi->poll, "%p", mi);

		if (mi->view && (win = WIDGET(mi->view)->window)) {
			AG_ObjectDetach(win);
			mi->view = NULL;
		}
	}

	AG_ObjectUnlock(m);
}

/*
 * Invalidate all cached Menu/MenuView label surfaces.
 * The parent Menu must be locked.
 */
void
AG_MenuInvalidateLabels(AG_MenuItem *_Nonnull mi)
{
	int i;

	AG_MENU_ITEM_IS_VALID(mi);

	for (i = 0; i < 2; i++) {
		if (mi->lblMenu[i] != -1) {
			AG_OBJECT_ISA(mi->pmenu, "AG_Widget:AG_Menu:*");
			AG_WidgetUnmapSurface(mi->pmenu, mi->lblMenu[i]);
			mi->lblMenu[i] = -1;
		}
		if (mi->lblView[i] != -1 &&
		    mi->parent &&
		    mi->parent->view) {
			AG_OBJECT_ISA(mi->parent->view, "AG_Widget:AG_MenuView:*");
			AG_WidgetUnmapSurface(mi->parent->view, mi->lblView[i]);
			mi->lblView[i] = -1;
		}
	}
}

void
AG_MenuState(AG_MenuItem *mi, int state)
{
	AG_Menu *m = mi->pmenu;

	AG_MENU_ITEM_IS_VALID(mi);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");

	m->curState = state;
}

void
AG_MenuToolbar(AG_MenuItem *mi, AG_Toolbar *tb)
{
	AG_Menu *m = mi->pmenu;

	AG_MENU_ITEM_IS_VALID(mi);
	AG_OBJECT_ISA(m, "AG_Widget:AG_Menu:*");
	AG_ObjectLock(m);

	m->curToolbar = tb;

	AG_ObjectUnlock(m);
}

static void
Draw(void *_Nonnull obj)
{
	AG_Menu *m = obj;
	AG_MenuItem *mi;
	int lbl, wLbl, hLbl;

	AG_DrawBoxRaised(m, &WIDGET(m)->r, &WCOLOR(m,FG_COLOR));

	AG_PushBlendingMode(m, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
	AG_PushClipRect(m, &m->r);

	TAILQ_FOREACH(mi, &m->root->subItems, items) {
		int activeState = mi->state;

		if (mi->stateFn) {
			AG_PostEventByPtr(m, mi->stateFn, "%p", &activeState);
		}
		if (activeState) {
			if (mi->lblMenu[1] == -1) {
				AG_TextColor(&WCOLOR(m, TEXT_COLOR));
				mi->lblMenu[1] = (mi->text == NULL) ? -1 :
				    AG_WidgetMapSurface(m, AG_TextRender(mi->text));
			}
			lbl = mi->lblMenu[1];
		} else {
			if (mi->lblMenu[0] == -1) {
				AG_TextColor(&WCOLOR_DISABLED(m, TEXT_COLOR));
				mi->lblMenu[0] = (mi->text == NULL) ? -1 :
				    AG_WidgetMapSurface(m, AG_TextRender(mi->text));
			}
			lbl = mi->lblMenu[0];
		}
		wLbl = WSURFACE(m,lbl)->w;
		hLbl = WSURFACE(m,lbl)->h;
		if (mi == m->itemSel) {
			AG_Rect r;

			r.x = mi->x;
			r.y = mi->y;
			r.w = m->lPadLbl + wLbl + m->rPadLbl;
			r.h = m->tPadLbl + hLbl + m->bPadLbl;
			AG_DrawRect(m, &r, &WCOLOR(m, SELECTION_COLOR));
		}
		AG_WidgetBlitSurface(m, lbl,
		    mi->x + m->lPadLbl,
		    mi->y + m->tPadLbl);
	}

	AG_PopClipRect(m);
	AG_PopBlendingMode(m);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Menu *m = obj;
	const int lPad = WIDGET(m)->paddingLeft;
	const int tPad = WIDGET(m)->paddingTop;
	const int bPad = WIDGET(m)->paddingBottom;
	AG_Driver *drv = WIDGET(m)->drv;
	AG_MenuItem *mi;
	int x, y, wLbl, hLbl;
	Uint wView, hView;

	x = lPad;
	y = tPad;
	r->h = 0;
	r->w = x;

	AG_GetDisplaySize(drv, &wView, &hView);

	TAILQ_FOREACH(mi, &m->root->subItems, items) {
		GetItemSize(mi, &wLbl, &hLbl);
		if (r->h == 0) {
			r->h = tPad + hLbl + bPad;
		}
		if (x+wLbl > wView) {			/* Wrap */
			x = lPad;
			y += hLbl;
			r->h += hLbl + bPad;
		}
		if (r->w < MIN(x+wLbl,wView)) {
			r->w = MIN(x+wLbl,wView);
		}
		x += wLbl;
	}
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_Menu *m = obj;
	const int tPad = WIDGET(m)->paddingTop;
	const int rPad = WIDGET(m)->paddingRight;
	const int bPad = WIDGET(m)->paddingBottom;
	const int lPad = WIDGET(m)->paddingLeft;
	AG_MenuItem *mi;
	int wLbl, hLbl;
	int x, y;
	
	if (a->w < (lPad + rPad) ||
	    a->h < (tPad + bPad)) {
		return (-1);
	}
	m->r.x = lPad;
	m->r.y = tPad;
	m->r.w = a->w - rPad;
	m->r.h = a->h - bPad;

	x = lPad;
	y = tPad;
	TAILQ_FOREACH(mi, &m->root->subItems, items) {
		GetItemSize(mi, &wLbl, &hLbl);
		mi->x = x;
		mi->y = y;
		if (x+wLbl > a->w) {
			mi->x = lPad;
			mi->y += hLbl;
			y += hLbl;
		}
		x += wLbl;
	}
	return (0);
}

static void
GetItemSize(AG_MenuItem *_Nonnull item, int *_Nonnull w, int *_Nonnull h)
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
PopupHideAll(AG_MenuItem *_Nonnull mi)
{
	AG_MenuItem *miSub;

	if (mi->view) {
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

#ifdef AG_TYPE_SAFETY
/*
 * Accessor for AG_[CONST_]MENU_ITEM_PTR().
 */
AG_MenuItem *
AG_MenuGetItemPtr(const AG_Event *event, int idx, int isConst)
{
	const AG_Variable *V = &event->argv[idx];

	if (idx > event->argc || V->type != AG_VARIABLE_POINTER) {
		AG_GenericMismatch("by AG_MENU_ITEM_PTR(idx)");
	}
	if (isConst) {
		if ((V->info.pFlags & AG_VARIABLE_P_READONLY) == 0)
			AG_FatalError("AG_MENU_CONST_ITEM_PTR() argument isn't const. "
			              "Did you mean AG_MENU_ITEM_PTR()?");
	} else {
		if (V->info.pFlags & AG_VARIABLE_P_READONLY)
			AG_FatalError("AG_MENU_ITEM_PTR() argument is const. "
			              "Did you mean AG_CONST_MENU_ITEM_PTR()?");
	}
	if (V->data.p == NULL) {
		return (NULL);
	}
	if (!AG_MENU_ITEM_VALID(V->data.p)) {
		AG_GenericMismatch("by AG_MENU_ITEM_PTR(tag)");
	}
	return (V->data.p);
}
#endif /* AG_TYPE_SAFETY */

#ifdef AG_LEGACY
void
AG_MenuSetPadding(AG_Menu *m, int lPad, int rPad, int tPad, int bPad)
{
	AG_SetStyleF(m, "padding", "%d %d %d %d",
	    (tPad != -1) ? tPad : 0,
	    (rPad != -1) ? rPad : 0,
	    (bPad != -1) ? bPad : 0,
	    (lPad != -1) ? lPad : 0);
}
#endif /* LEGACY */

AG_WidgetClass agMenuClass = {
	{
		"Agar(Widget:Menu)",
		sizeof(AG_Menu),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

#endif /* AG_WIDGETS */
