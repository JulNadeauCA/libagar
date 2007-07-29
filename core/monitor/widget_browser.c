/*	$Csoft: widget_browser.c,v 1.46 2005/10/04 17:34:52 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#include "monitor.h"

#include <string.h>

static void
FindWidgets(AG_Widget *wid, AG_Tlist *widtl, int depth)
{
	char text[AG_TLIST_LABEL_MAX];
	AG_TlistItem *it;

	strlcpy(text, AGOBJECT(wid)->ops->type, sizeof(text));
	if (AG_ObjectIsClass(wid, "AG_Widget:AG_Window:*")) {
		AG_Window *win = (AG_Window *)wid;

		strlcat(text, " (", sizeof(text));
		strlcat(text, win->caption, sizeof(text));
		strlcat(text, ")", sizeof(text));
	}
	it = AG_TlistAddPtr(widtl, NULL, text, wid);
	it->depth = depth;
	it->cat = "widget";
	
	if (!TAILQ_EMPTY(&AGOBJECT(wid)->children)) {
		it->flags |= AG_TLIST_HAS_CHILDREN;
	}
	if ((it->flags & AG_TLIST_HAS_CHILDREN) &&
	    AG_TlistVisibleChildren(widtl, it)) {
		AG_Widget *cwid;

		AGOBJECT_FOREACH_CHILD(cwid, wid, ag_widget)
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
	    AGOBJECT(win)->name);
	it->p1 = win;
	it->depth = depth;
	it->cat = "window";
	if (!TAILQ_EMPTY(&AGOBJECT(win)->children) ||
	    !TAILQ_EMPTY(&win->subwins)) {
		it->flags |= AG_TLIST_HAS_CHILDREN;
	}
	if ((it->flags & AG_TLIST_HAS_CHILDREN) &&
	    AG_TlistVisibleChildren(tl, it)) {
		TAILQ_FOREACH(wSub, &win->subwins, swins)
			FindWindows(tl, wSub, depth+1);
		AGOBJECT_FOREACH_CHILD(wChild, win, ag_widget)
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
	AG_Tlist *tl = AG_PTR(1);
	AG_TlistItem *it;
	AG_Window *win;

	if ((it = AG_TlistSelectedItem(tl)) == NULL)
		return;

	win = it->p1;
	AG_WindowShow(win);
}

static void
HideWindow(AG_Event *event)
{
	AG_Tlist *tl = AG_PTR(1);
	AG_TlistItem *it;
	AG_Window *win;

	if ((it = AG_TlistSelectedItem(tl)) == NULL)
		return;

	win = it->p1;
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
		SDL_Surface *su = AGWIDGET_SURFACE(wid,i);

		AG_TlistAdd(tl, su, "Surface%u (%ux%u, %ubpp)",
		    i, su->w, su->h, su->format->BitsPerPixel);
	}
	AG_TlistEnd(tl);
}

static void
WidgetParams(AG_Event *event)
{
	AG_Tlist *tl = AG_PTR(1);
	AG_Window *wExam = AG_PTR(2);
	AG_TlistItem *it;
	AG_Widget *wid;
	AG_Window *win;
	AG_Notebook *nb;
	AG_NotebookTab *nTab;

	if ((it = AG_TlistSelectedItem(tl)) == NULL) {
		return;
	}
	wid = it->p1;

	win = AG_WindowNew(0);
	AG_WindowSetCaption(win, _("Widget parameters: %s"),
	    AGOBJECT(wid)->name);

	nb = AG_NotebookNew(win, AG_NOTEBOOK_EXPAND);
	nTab = AG_NotebookAddTab(nb, _("Flags"), AG_BOX_VERT);
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
		    { AG_WIDGET_FOCUS_PARENT_WIN,	"FOCUS_PARENT_WIN",1 },
		    { AG_WIDGET_PRIO_MOTION,		"PRIO_MOTION",1 },
		    { 0,				NULL,0 }
		};
		AG_Label *lbl;

		AG_LabelNewStatic(nTab, 0, _("Class: %s"),
		    AGOBJECT(wid)->ops->type);
		AG_SeparatorNewHoriz(nTab);
		AG_CheckboxSetFromFlags(nTab, &wid->flags, flagDescr);
	}
	nTab = AG_NotebookAddTab(nb, _("Geometry"), AG_BOX_VERT);
	{
		AG_MSpinbutton *msb;

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
		AG_Tlist *tl;

		tl = AG_TlistNewPolled(nTab, AG_TLIST_EXPAND,
		    PollSurfaces, "%p", wid);
		AG_TlistSetItemHeight(tl, 16);
	}

	AG_WindowShow(win);
}

static void
ResizeWindow(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);

	AG_WindowScale(win, -1, -1);
	AG_WindowScale(win, AGWIDGET(win)->w, AGWIDGET(win)->h);
	AG_WINDOW_UPDATE(win);
}

static void
WindowParams(AG_Event *event)
{
	AG_Tlist *tl = AG_PTR(1);
	AG_TlistItem *it;
	AG_Window *wExam, *win;
	AG_MenuItem *mi;
	AG_Spinbutton *sb;
	AG_Label *lbl;

	if ((it = AG_TlistSelectedItem(tl)) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, _("No window is selected."));
		return;
	}
	wExam = it->p1;

	if ((win = AG_WindowNewNamed(0, "widget-browser-%s",
	    AGOBJECT(wExam)->name)) == NULL) {
		return;
	}
	AG_WindowSetCaption(win, "%s", AGOBJECT(wExam)->name);
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);

	AG_LabelNewStatic(win, 0, "Name: \"%s\"", AGOBJECT(wExam)->name);
	lbl = AG_LabelNewPolledMT(win, AG_LABEL_HFILL, &wExam->lock,
	    "Flags: <%[flags]>", &wExam->flags);
	AG_LabelFlag32(lbl,0,"MODAL",AG_WINDOW_MODAL);
	AG_LabelFlag32(lbl,0,"MAXIMIZED",AG_WINDOW_MAXIMIZED);
	AG_LabelFlag32(lbl,0,"MINIMIZED",AG_WINDOW_MINIMIZED);
	AG_LabelFlag32(lbl,0,"KEEPABOVE",AG_WINDOW_KEEPABOVE);
	AG_LabelFlag32(lbl,0,"KEEPBELOW",AG_WINDOW_KEEPBELOW);
	AG_LabelFlag32(lbl,0,"DENYFOCUS",AG_WINDOW_DENYFOCUS);
	AG_LabelFlag32(lbl,0,"NOTITLE",AG_WINDOW_NOTITLE);
	AG_LabelFlag32(lbl,0,"NOBORDERS",AG_WINDOW_NOBORDERS);
	AG_LabelFlag32(lbl,0,"NOHRESIZE",AG_WINDOW_NOHRESIZE);
	AG_LabelFlag32(lbl,0,"NOVRESIZE",AG_WINDOW_NOVRESIZE);
	AG_LabelFlag32(lbl,0,"NOCLOSE",AG_WINDOW_NOCLOSE);
	AG_LabelFlag32(lbl,0,"NOMINIMIZE",AG_WINDOW_NOMINIMIZE);
	AG_LabelFlag32(lbl,0,"NOMAXIMIZE",AG_WINDOW_NOMAXIMIZE);
	AG_LabelFlag32(lbl,0,"NOBACKGROUND",AG_WINDOW_NOBACKGROUND);
	AG_LabelFlag32(lbl,0,"NOUPDATERECT",AG_WINDOW_NOUPDATERECT);
	AG_LabelFlag32(lbl,0,"FOCUSONATTACH",AG_WINDOW_FOCUSONATTACH);

	sb = AG_SpinbuttonNew(win, 0, _("Widget spacing: "));
	AG_WidgetBind(sb, "value", AG_WIDGET_INT, &wExam->spacing);
	AG_SpinbuttonSetMin(sb, 0);
	AG_SetEvent(sb, "spinbutton-changed", ResizeWindow, "%p", wExam);
	
	sb = AG_SpinbuttonNew(win, 0, _("Top padding: "));
	AG_WidgetBind(sb, "value", AG_WIDGET_INT, &wExam->ypadding_top);
	AG_SpinbuttonSetMin(sb, 0);
	AG_SetEvent(sb, "spinbutton-changed", ResizeWindow, "%p", wExam);
	
	sb = AG_SpinbuttonNew(win, 0, _("Bottom padding: "));
	AG_WidgetBind(sb, "value", AG_WIDGET_INT, &wExam->ypadding_bot);
	AG_SpinbuttonSetMin(sb, 0);
	AG_SetEvent(sb, "spinbutton-changed", ResizeWindow, "%p", wExam);
	
	sb = AG_SpinbuttonNew(win, 0, _("Horizontal padding: "));
	AG_WidgetBind(sb, "value", AG_WIDGET_INT, &wExam->xpadding);
	AG_SpinbuttonSetMin(sb, 0);
	AG_SetEvent(sb, "spinbutton-changed", ResizeWindow, "%p", wExam);

	AG_WindowShow(win);
}

AG_Window *
AG_DebugWidgetBrowser(void)
{
	AG_Window *win;
	AG_Tlist *tl;
	AG_MenuItem *mi;

	if ((win = AG_WindowNewNamed(0, "widget-browser")) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, _("GUI Debugger"));
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);

	tl = AG_TlistNew(win, AG_TLIST_POLL|AG_TLIST_FOCUS|AG_TLIST_EXPAND);
	AG_SetEvent(tl, "tlist-poll", PollWindows, NULL);
	AG_SetEvent(tl, "tlist-dblclick", WidgetParams, "%p,%p", tl, win);

	mi = AG_TlistSetPopup(tl, "widget");
	{
		AG_MenuAction(mi, _("Widget parameters..."), -1,
		    WidgetParams, "%p,%p", tl, win);
	}
	mi = AG_TlistSetPopup(tl, "window");
	{
		AG_MenuAction(mi, _("Window parameters..."), -1,
		    WindowParams, "%p", tl);
		AG_MenuSeparator(mi);
		AG_MenuAction(mi, _("Show this window"), -1,
		    ShowWindow, "%p", tl);
		AG_MenuAction(mi, _("Hide this window"), -1,
		    HideWindow, "%p", tl);
	}
	AG_WindowSetGeometry(win, agView->w/4, agView->h/3, agView->w/2,
	    agView->h/4);
	return (win);
}

#endif	/* DEBUG */
