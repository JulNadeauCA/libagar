/*
 * Copyright (c) 2002-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * GUI Debugger. This allows the user to browse through the widget tree
 * and manipulate generic Widget and Window parameters.
 */

#include <config/debug.h>

#ifdef DEBUG

#include <core/core.h>
#include <core/view.h>

#include <gui/window.h>
#include <gui/vbox.h>
#include <gui/textbox.h>
#include <gui/tlist.h>
#include <gui/label.h>
#include <gui/button.h>
#include <gui/spinbutton.h>
#include <gui/mspinbutton.h>
#include <gui/checkbox.h>
#include <gui/separator.h>
#include <gui/notebook.h>

#include "dev.h"

#include <string.h>

static void
FindWidgets(AG_Widget *wid, AG_Tlist *widtl, int depth)
{
	char text[AG_TLIST_LABEL_MAX];
	AG_TlistItem *it;

	strlcpy(text, OBJECT(wid)->name, sizeof(text));
	if (AG_ObjectIsClass(wid, "AG_Widget:AG_Window:*")) {
		AG_Window *win = (AG_Window *)wid;

		strlcat(text, " (", sizeof(text));
		strlcat(text, win->caption, sizeof(text));
		strlcat(text, ")", sizeof(text));
	}
	it = AG_TlistAddPtr(widtl, NULL, text, wid);
	it->depth = depth;
	it->cat = "widget";
	
	if (!TAILQ_EMPTY(&OBJECT(wid)->children)) {
		it->flags |= AG_TLIST_HAS_CHILDREN;
	}
	if ((it->flags & AG_TLIST_HAS_CHILDREN) &&
	    AG_TlistVisibleChildren(widtl, it)) {
		AG_Widget *cwid;

		OBJECT_FOREACH_CHILD(cwid, wid, ag_widget)
			FindWidgets(cwid, widtl, depth+1);
	}
}

static void
FindWindows(AG_Tlist *tl, AG_Window *win, int depth)
{
	char text[AG_TLIST_LABEL_MAX];
	AG_Window *wSub;
	AG_Widget *wChild;
	AG_TlistItem *it;

	if (strcmp(win->caption, "win-popup") == 0)
		return;

	strlcpy(text, win->caption, sizeof(text));
	it = AG_TlistAdd(tl, NULL, "%s (%s)", win->caption,
	    OBJECT(win)->name);
	it->p1 = win;
	it->depth = depth;
	it->cat = "window";
	if (!TAILQ_EMPTY(&OBJECT(win)->children) ||
	    !TAILQ_EMPTY(&win->subwins)) {
		it->flags |= AG_TLIST_HAS_CHILDREN;
	}
	if ((it->flags & AG_TLIST_HAS_CHILDREN) &&
	    AG_TlistVisibleChildren(tl, it)) {
		TAILQ_FOREACH(wSub, &win->subwins, swins)
			FindWindows(tl, wSub, depth+1);
		OBJECT_FOREACH_CHILD(wChild, win, ag_widget)
			FindWidgets(wChild, tl, depth+1);
	}
}

static void
PollWindows(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_Window *win;

	AG_TlistClear(tl);
	AG_MutexLock(&agView->lock);
	TAILQ_FOREACH_REVERSE(win, &agView->windows, ag_windowq, windows) {
		FindWindows(tl, win, 0);
	}
	AG_MutexUnlock(&agView->lock);
	AG_TlistRestore(tl);
}

static void
ShowWindow(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);
	AG_WindowShow(win);
}

static void
HideWindow(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);
	AG_WindowHide(win);
}

static void
PollSurfaces(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_Widget *wid = AG_PTR(1);
	Uint i;

	AG_TlistBegin(tl);
	for (i = 0; i < wid->nsurfaces; i++) {
		SDL_Surface *su = WSURFACE(wid,i);

		AG_TlistAdd(tl, su, "Surface%u (%ux%u, %ubpp)",
		    i, su->w, su->h, su->format->BitsPerPixel);
	}
	AG_TlistEnd(tl);
}

static void
UpdateWindow(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);

	if (win != NULL)
		AG_WindowUpdate(win);
}

static void
UpdateWindowCaption(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);

	AG_WindowUpdateCaption(win);
}

static void
WidgetParams(AG_Event *event)
{
	AG_Tlist *tl = AG_PTR(1);
	AG_TlistItem *it;
	AG_Widget *wid;
	AG_Window *win;
	AG_Notebook *nb;
	AG_NotebookTab *nTab;
	AG_Textbox *tb;
	AG_Label *lbl;
	AG_Spinbutton *sb;
	AG_MSpinbutton *msb;

	if ((it = AG_TlistSelectedItem(tl)) == NULL) {
		return;
	}
	wid = it->p1;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("GUI Debugger: <%s>"), OBJECT(wid)->name);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_EXPAND);

	nTab = AG_NotebookAddTab(nb, _("Widget"), AG_BOX_VERT);
	{
		static const AG_FlagDescr flagDescr[] = {
		    { AG_WIDGET_FOCUSABLE,		"FOCUSABLE",1 },
		    { AG_WIDGET_FOCUSED,		"FOCUSED",0 },
		    { AG_WIDGET_UNFOCUSED_MOTION,     "UNFOCUSED_MOTION",1 },
		    { AG_WIDGET_UNFOCUSED_BUTTONUP,   "UNFOCUSED_BUTTONUP",1 },
		    { AG_WIDGET_UNFOCUSED_BUTTONDOWN, "UNFOCUSED_BUTTONDOWN",1},
		    { AG_WIDGET_CLIPPING,		"CLIPPING",1 },
		    { AG_WIDGET_HFILL,			"HFILL",1 },
		    { AG_WIDGET_VFILL,			"VFILL",1 },
		    { AG_WIDGET_EXCEDENT,		"EXCEDENT",0 },
		    { AG_WIDGET_HIDE,			"HIDE",1 },
		    { AG_WIDGET_DISABLED,		"DISABLED",1 },
		    { AG_WIDGET_STATIC,			"STATIC",0 },
		    { AG_WIDGET_CATCH_TAB,		"CATCH_TAB",1 },
		    { AG_WIDGET_PRIO_MOTION,		"PRIO_MOTION",1 },
		    { AG_WIDGET_UNDERSIZE,		"UNDERSIZE",0 },
		    { AG_WIDGET_IGNORE_PADDING,		"IGNORE_PADDING",1 },
		    { 0,				NULL,0 }
		};

		tb = AG_TextboxNew(nTab, AG_TEXTBOX_HFILL, _("Name: "));
		AG_WidgetBindString(tb, "string", OBJECT(wid)->name,
		    sizeof(OBJECT(wid)->name));
		AG_LabelNewStatic(nTab, 0, _("Class: %s"),
		    OBJECT(wid)->ops->type);
		AG_SeparatorNewHoriz(nTab);
		AG_CheckboxSetFromFlags(nTab, &wid->flags, flagDescr);
	}

	if (AG_ObjectIsClass(wid, "AG_Widget:AG_Window:*")) {
		AG_Window *ww = (AG_Window *)wid;
		static const AG_FlagDescr flagDescr[] = {
		    { AG_WINDOW_MODAL,		"MODAL",1 },
		    { AG_WINDOW_MAXIMIZED,	"MAXIMIZED",1 },
		    { AG_WINDOW_MINIMIZED,	"MINIMIZED",1 },
		    { AG_WINDOW_KEEPABOVE,	"KEEPABOVE",1 },
		    { AG_WINDOW_KEEPBELOW,	"KEEPBELOW",1 },
		    { AG_WINDOW_DENYFOCUS,	"DENYFOCUS",1 },
		    { AG_WINDOW_NOBORDERS,	"NOBORDERS",1 },
		    { AG_WINDOW_NOHRESIZE,	"NOHRESIZE",1 },
		    { AG_WINDOW_NOVRESIZE,	"NOVRESIZE",1 },
		    { AG_WINDOW_NOBACKGROUND,	"NOBACKGROUND",1 },
		    { AG_WINDOW_NOUPDATERECT,	"NOUPDATERECT",1 },
		    { 0,			NULL,0 }
		};

		nTab = AG_NotebookAddTab(nb, _("Window"), AG_BOX_VERT);

		tb = AG_TextboxNew(nTab, AG_TEXTBOX_HFILL, _("Caption: "));
		AG_WidgetBindString(tb, "string", ww->caption,
		    sizeof(ww->caption));
		AG_SetEvent(tb, "textbox-postchg", UpdateWindowCaption,
		    "%p", ww);

		lbl = AG_LabelNewPolledMT(nTab, AG_LABEL_HFILL, &ww->lock,
		    "Flags: <%[flags]>", &ww->flags);
		AG_SeparatorNewHoriz(nTab);
		AG_CheckboxSetFromFlags(nTab, &ww->flags, flagDescr);
		AG_SeparatorNewHoriz(nTab);

		sb = AG_SpinbuttonNew(nTab, 0, _("Widget spacing: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_INT, &ww->spacing);
		AG_SpinbuttonSetMin(sb, 0);
		AG_SetEvent(sb, "spinbutton-changed", UpdateWindow, "%p", ww);
		sb = AG_SpinbuttonNew(nTab, 0, _("Top padding: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_INT, &ww->tPad);
		AG_SpinbuttonSetMin(sb, 0);
		AG_SetEvent(sb, "spinbutton-changed", UpdateWindow, "%p", ww);
	
		sb = AG_SpinbuttonNew(nTab, 0, _("Bottom padding: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_INT, &ww->bPad);
		AG_SpinbuttonSetMin(sb, 0);
		AG_SetEvent(sb, "spinbutton-changed", UpdateWindow, "%p", ww);
	
		sb = AG_SpinbuttonNew(nTab, 0, _("Left padding: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_INT, &ww->lPad);
		AG_SpinbuttonSetMin(sb, 0);
		AG_SetEvent(sb, "spinbutton-changed", UpdateWindow, "%p", ww);
	
		sb = AG_SpinbuttonNew(nTab, 0, _("Right padding: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_INT, &ww->rPad);
		AG_SpinbuttonSetMin(sb, 0);
		AG_SetEvent(sb, "spinbutton-changed", UpdateWindow, "%p", ww);
	}

	if (AG_ObjectIsClass(wid, "AG_Widget:AG_Box:*")) {
		AG_Box *box = (AG_Box *)wid;
		AG_Window *wp = AG_WidgetParentWindow(box);
		AG_Spinbutton *sb;
		
		nTab = AG_NotebookAddTab(nb, _("Box"), AG_BOX_VERT);
		
		sb = AG_SpinbuttonNew(nTab, 0, _("Padding: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_INT, &box->padding);
		AG_SpinbuttonSetMin(sb, 0);
		AG_SetEvent(sb, "spinbutton-changed", UpdateWindow, "%p", wp);
		
		sb = AG_SpinbuttonNew(nTab, 0, _("Spacing: "));
		AG_WidgetBind(sb, "value", AG_WIDGET_INT, &box->spacing);
		AG_SpinbuttonSetMin(sb, 0);
		AG_SetEvent(sb, "spinbutton-changed", UpdateWindow, "%p", wp);
	}
	
	nTab = AG_NotebookAddTab(nb, _("Geometry"), AG_BOX_VERT);
	{
		msb = AG_MSpinbuttonNew(nTab, 0, ",", "Container coords: ");
		AG_WidgetBindInt(msb, "xvalue", &wid->x);
		AG_WidgetBindInt(msb, "yvalue", &wid->y);

		msb = AG_MSpinbuttonNew(nTab, 0, "x", "Geometry: ");
		AG_WidgetBindInt(msb, "xvalue", &wid->w);
		AG_WidgetBindInt(msb, "yvalue", &wid->h);
		
		msb = AG_MSpinbuttonNew(nTab, 0, ",", "View coords (UL): ");
		AG_WidgetBindInt(msb, "xvalue", &wid->cx);
		AG_WidgetBindInt(msb, "yvalue", &wid->cy);
		
		msb = AG_MSpinbuttonNew(nTab, 0, ",", "View coords (LR): ");
		AG_WidgetBindInt(msb, "xvalue", &wid->cx2);
		AG_WidgetBindInt(msb, "yvalue", &wid->cy2);
	}
	nTab = AG_NotebookAddTab(nb, _("Surfaces"), AG_BOX_VERT);
	{
		AG_Tlist *tlSurf;

		tlSurf = AG_TlistNewPolled(nTab, AG_TLIST_EXPAND,
		    PollSurfaces, "%p", wid);
		AG_TlistSetItemHeight(tlSurf, 16);
	}

	AG_WindowShow(win);
}

static void
CreateWindowMenu(AG_Event *event)
{
	AG_MenuItem *mi = AG_SENDER();
	AG_Tlist *tl = AG_PTR(1);
	AG_TlistItem *ti = AG_TlistSelectedItem(tl);
	AG_Window *win;

	if (ti == NULL || !AG_ObjectIsClass(ti->p1, "AG_Widget:AG_Window:*")) {
		return;
	}
	win = ti->p1;

	if (win->visible) {
		AG_MenuAction(mi, _("Hide window"), -1, HideWindow, "%p", win);
	} else {
		AG_MenuAction(mi, _("Show window"), -1, ShowWindow, "%p", win);
	}
}

AG_Window *
DEV_GuiDebugger(void)
{
	AG_Window *win;
	AG_Tlist *tl;
	AG_MenuItem *mi;

	if ((win = AG_WindowNewNamed(0, "DEV_GuiDebugger")) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, _("GUI Debugger"));
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);

	tl = AG_TlistNewPolled(win, AG_TLIST_FOCUS|AG_TLIST_EXPAND,
	    PollWindows, NULL);
	AG_SetEvent(tl, "tlist-dblclick", WidgetParams, "%p", tl);

	mi = AG_TlistSetPopup(tl, "window");
	AG_MenuSetPollFn(mi, CreateWindowMenu, "%p", tl);
	AG_WindowSetGeometry(win, agView->w/4, agView->h/3, agView->w/2,
	    agView->h/4);
	return (win);
}

#endif	/* DEBUG */
