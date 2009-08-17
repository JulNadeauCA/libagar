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
 * This tool allows the user to browse through the widget tree
 * and manipulate generic Widget and Window parameters.
 */

#include <core/core.h>

#include <gui/window.h>
#include <gui/vbox.h>
#include <gui/textbox.h>
#include <gui/tlist.h>
#include <gui/label.h>
#include <gui/button.h>
#include <gui/numerical.h>
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

	Strlcpy(text, OBJECT(wid)->name, sizeof(text));
	if (AG_OfClass(wid, "AG_Widget:AG_Window:*")) {
		AG_Window *win = (AG_Window *)wid;

		Strlcat(text, " (", sizeof(text));
		Strlcat(text, win->caption, sizeof(text));
		Strlcat(text, ")", sizeof(text));
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

	Strlcpy(text, win->caption, sizeof(text));
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
	AG_LockVFS(agView);
	TAILQ_FOREACH_REVERSE(win, &agView->windows, ag_windowq, windows) {
		FindWindows(tl, win, 0);
	}
	AG_UnlockVFS(agView);
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
		AG_Surface *su = WSURFACE(wid,i);
		AG_TlistAdd(tl, su, "Surface%u (%ux%u, %ubpp)",
		    i, su->w, su->h, su->format->BitsPerPixel);
	}
	AG_TlistEnd(tl);
}

static void
UpdateWindow(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);

	AG_WindowUpdate(win);
}

static void
UpdateWindowCaption(AG_Event *event)
{
	AG_Window *win = AG_PTR(1);

	AG_WindowUpdateCaption(win);
}

static void
EditWidgetParams(AG_Event *event)
{
	AG_TlistItem *ti = AG_PTR(2);
	AG_Widget *wid = ti->p1;
	AG_Window *win;
	AG_Notebook *nb;
	AG_NotebookTab *nTab;
	AG_Textbox *tb;
	AG_Label *lbl;
	AG_MSpinbutton *msb;

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
		    { AG_WIDGET_HFILL,			"HFILL",1 },
		    { AG_WIDGET_VFILL,			"VFILL",1 },
		    { AG_WIDGET_HIDE,			"HIDE",1 },
		    { AG_WIDGET_DISABLED,		"DISABLED",1 },
		    { AG_WIDGET_CATCH_TAB,		"CATCH_TAB",1 },
		    { AG_WIDGET_PRIO_MOTION,		"PRIO_MOTION",1 },
		    { AG_WIDGET_UNDERSIZE,		"UNDERSIZE",0 },
		    { AG_WIDGET_NOSPACING,		"NOSPACING",1 },
		    { 0,				NULL,0 }
		};

		tb = AG_TextboxNew(nTab, 0, _("Name: "));
		AG_TextboxBindUTF8(tb, OBJECT(wid)->name,
		    sizeof(OBJECT(wid)->name));
		AG_LabelNew(nTab, 0, _("Class: %s"), OBJECT(wid)->cls->name);
		AG_SeparatorNewHoriz(nTab);
		AG_CheckboxSetFromFlags(nTab, 0, &wid->flags, flagDescr);
	}

	if (AG_OfClass(wid, "AG_Widget:AG_Window:*")) {
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
		AG_Numerical *nums[4];
		int i;

		nTab = AG_NotebookAddTab(nb, _("Window"), AG_BOX_VERT);

		tb = AG_TextboxNew(nTab, 0, _("Caption: "));
		AG_TextboxBindUTF8(tb, ww->caption, sizeof(ww->caption));
		AG_SetEvent(tb, "textbox-postchg", UpdateWindowCaption,
		    "%p", ww);

#ifdef AG_THREADS
		lbl = AG_LabelNewPolledMT(nTab, AG_LABEL_HFILL,
		    &OBJECT(ww)->lock,
		    "Flags: <%[flags]>", &ww->flags);
#else
		lbl = AG_LabelNewPolled(nTab, AG_LABEL_HFILL,
		    "Flags: <%[flags]>", &ww->flags);
#endif

		AG_SeparatorNewHoriz(nTab);
		AG_CheckboxSetFromFlags(nTab, 0, &ww->flags, flagDescr);
		AG_SeparatorNewHoriz(nTab);

		nums[0] = AG_NumericalNewIntR(nTab, 0, "px",
		    _("Widget spacing: "), &ww->spacing, 0, 255);
		nums[1] = AG_NumericalNewIntR(nTab, 0, "px",
		    _("Top padding: "), &ww->tPad, 0, 255);
		nums[2] = AG_NumericalNewIntR(nTab, 0, "px",
		    _("Bottom padding: "), &ww->bPad, 0, 255);
		nums[3] = AG_NumericalNewIntR(nTab, 0, "px",
		    _("Left padding: "), &ww->lPad, 0, 255);
		nums[4] = AG_NumericalNewIntR(nTab, 0, "px",
		    _("Right padding: "), &ww->rPad, 0, 255);
	
		for (i = 0; i < 4; i++) {
			AG_SetEvent(nums[i], "numerical-changed",
			    UpdateWindow, "%p", ww);
		}
	}

	if (AG_OfClass(wid, "AG_Widget:AG_Box:*")) {
		AG_Box *box = (AG_Box *)wid;
		AG_Window *wp = AG_ParentWindow(box);
		AG_Numerical *num;
		
		nTab = AG_NotebookAddTab(nb, _("Box"), AG_BOX_VERT);
		
		num = AG_NumericalNewIntR(nTab, 0, "px", _("Padding: "),
		    &box->padding, 0, 255);
		AG_SetEvent(num, "numerical-changed", UpdateWindow, "%p", wp);
		
		num = AG_NumericalNewIntR(nTab, 0, "px", _("Spacing: "),
		    &box->spacing, 0, 255);
		AG_SetEvent(num, "numerical-changed", UpdateWindow, "%p", wp);
		
		AG_CheckboxNewFlag(nTab, 0, _("Homogenous"),
		    &box->flags, AG_BOX_HOMOGENOUS);
		AG_CheckboxNewFlag(nTab, 0, _("Visual frame"),
		    &box->flags, AG_BOX_FRAME);
	}

	if (AG_OfClass(wid, "AG_Widget:AG_Editable:*")) {
		AG_Editable *ed = (AG_Editable *)wid;
		
		nTab = AG_NotebookAddTab(nb, _("Editable"), AG_BOX_VERT);
		AG_NumericalNewInt(nTab, 0, "px", "x: ", &ed->x);
		AG_NumericalNewInt(nTab, 0, "px", "xMax: ", &ed->xMax);
		AG_NumericalNewInt(nTab, 0, "px", "y: ", &ed->y);
		AG_NumericalNewInt(nTab, 0, "px", "yMax: ", &ed->yMax);
		AG_NumericalNewInt(nTab, 0, "px", "yVis: ", &ed->yVis);
		AG_SeparatorNewHoriz(nTab);
		AG_NumericalNewInt(nTab, 0, "px", "pos: ", &ed->pos);
		AG_NumericalNewInt(nTab, 0, "px", "xCurs: ", &ed->xCurs);
		AG_NumericalNewInt(nTab, 0, "px", "yCurs: ", &ed->yCurs);
	}

	nTab = AG_NotebookAddTab(nb, _("Geometry"), AG_BOX_VERT);
	{
		msb = AG_MSpinbuttonNew(nTab, 0, ",", "Container coords: ");
		AG_BindInt(msb, "xvalue", &wid->x);
		AG_BindInt(msb, "yvalue", &wid->y);

		msb = AG_MSpinbuttonNew(nTab, 0, "x", "Geometry: ");
		AG_BindInt(msb, "xvalue", &wid->w);
		AG_BindInt(msb, "yvalue", &wid->h);
		
		msb = AG_MSpinbuttonNew(nTab, 0, ",", "View coords (UL): ");
		AG_BindInt(msb, "xvalue", &wid->rView.x1);
		AG_BindInt(msb, "yvalue", &wid->rView.y1);
		
		msb = AG_MSpinbuttonNew(nTab, 0, ",", "View coords (LR): ");
		AG_BindInt(msb, "xvalue", &wid->rView.x2);
		AG_BindInt(msb, "yvalue", &wid->rView.y2);
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

	if (ti == NULL || !AG_OfClass(ti->p1, "AG_Widget:AG_Window:*")) {
		return;
	}
	win = ti->p1;

	if (win->visible) {
		AG_MenuAction(mi, _("Hide window"), NULL,
		    HideWindow, "%p", win);
	} else {
		AG_MenuAction(mi, _("Show window"), NULL,
		    ShowWindow, "%p", win);
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

	tl = AG_TlistNewPolled(win, AG_TLIST_EXPAND, PollWindows, NULL);
	AG_SetEvent(tl, "tlist-dblclick", EditWidgetParams, "%p", tl);
	AG_WidgetFocus(tl);

	mi = AG_TlistSetPopup(tl, "window");
	AG_MenuSetPollFn(mi, CreateWindowMenu, "%p", tl);
	AG_WindowSetGeometry(win, agView->w/4, agView->h/3, agView->w/2,
	    agView->h/4);
	return (win);
}
