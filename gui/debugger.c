/*
 * Copyright (c) 2002-2009 Hypertriton, Inc. <http://hypertriton.com/>
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
 * This tool allows the user to browse through the widget tree and manipulate
 * generic Widget and Window parameters.
 */

#include <core/core.h>

#ifdef AG_DEBUG

#include "gui.h"
#include "box.h"
#include "textbox.h"
#include "tlist.h"
#include "label.h"
#include "button.h"
#include "numerical.h"
#include "mspinbutton.h"
#include "checkbox.h"
#include "separator.h"
#include "notebook.h"
#include "pane.h"
#include "scrollview.h"

#include <string.h>

static void
FindWidgets(AG_Widget *wid, AG_Tlist *tl, int depth)
{
	char text[AG_TLIST_LABEL_MAX];
	AG_TlistItem *it;
	AG_Widget *widChld;

	Strlcpy(text, OBJECT(wid)->name, sizeof(text));
	if (AG_OfClass(wid, "AG_Widget:AG_Window:*")) {
		AG_Window *win = (AG_Window *)wid;

		Strlcat(text, " (\"", sizeof(text));
		Strlcat(text, win->caption, sizeof(text));
		Strlcat(text, "\")", sizeof(text));
	}
	it = AG_TlistAddPtr(tl, NULL, text, wid);
	it->depth = depth;
	it->cat = "widget";
	
	if (!TAILQ_EMPTY(&OBJECT(wid)->children)) {
		it->flags |= AG_TLIST_HAS_CHILDREN;
	}
	if ((it->flags & AG_TLIST_HAS_CHILDREN) &&
	    AG_TlistVisibleChildren(tl, it)) {
		OBJECT_FOREACH_CHILD(widChld, wid, ag_widget)
			FindWidgets(widChld, tl, depth+1);
	}
}

static void
FindWindows(AG_Tlist *tl, AG_Window *win, int depth)
{
	char text[AG_TLIST_LABEL_MAX];
	AG_Window *wSub;
	AG_Widget *wChild;
	AG_TlistItem *it;

	if (strncmp(OBJECT(win)->name, "_Popup-", sizeof("_Popup-")) == 0)
		return;

	Strlcpy(text, win->caption, sizeof(text));
	if (strcmp(OBJECT(win)->name, "generic") == 0) {
		it = AG_TlistAddS(tl, NULL,
		    win->caption[0] != '\0' ? win->caption : _("Untitled"));
	} else {
		it = AG_TlistAdd(tl, NULL, "%s (<%s>)",
		    win->caption[0] != '\0' ? win->caption : _("Untitled"),
		    OBJECT(win)->name);
	}
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
PollWidgets(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_Driver *drv = WIDGET(tl)->drv;
	AG_Window *win;

	AG_TlistClear(tl);
	AG_LockVFS(drv);
	AG_FOREACH_WINDOW_REVERSE(win, drv) {
		FindWindows(tl, win, 0);
	}
	AG_UnlockVFS(drv);
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
PollVariables(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_Object *obj = AG_PTR(1);
	AG_Variable *V;
	Uint i;

	AG_TlistBegin(tl);
	AGOBJECT_FOREACH_VARIABLE(V, i, obj) {
		char val[1024];

		if ((V->type == AG_VARIABLE_P_UINT ||
		     V->type == AG_VARIABLE_P_INT) &&
		     strcmp(V->name, "flags") == 0) {
			Snprintf(val, sizeof(val), "0x%08x",
			    *(Uint *)V->data.p);
		} else {
			AG_PrintVariable(val, sizeof(val), V);
		}
		AG_TlistAdd(tl, NULL, "%s: %s", V->name, val);
	}
	AG_TlistEnd(tl);
}

static void
WidgetSelected(AG_Event *event)
{
	AG_Box *box = AG_PTR(1);
	AG_TlistItem *ti = AG_PTR(2);
	AG_Widget *wid = ti->p1;
	AG_Notebook *nb;
	AG_NotebookTab *nTab;
	AG_Textbox *tb;
	AG_MSpinbutton *msb;
	AG_Scrollview *sv;

	AG_ObjectFreeChildren(box);

	nb = AG_NotebookNew(box, AG_NOTEBOOK_EXPAND);
	nTab = AG_NotebookAddTab(nb, _("General Settings"), AG_BOX_VERT);
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
		    { AG_WIDGET_UNDERSIZE,		"UNDERSIZE",0 },
		    { AG_WIDGET_NOSPACING,		"NOSPACING",1 },
		    { 0,				NULL,0 }
		};

		tb = AG_TextboxNewS(nTab, AG_TEXTBOX_HFILL, _("Name: "));
		AG_TextboxBindUTF8(tb, OBJECT(wid)->name,
		    sizeof(OBJECT(wid)->name));
		AG_LabelNew(nTab, 0, _("Class: %s"), OBJECT(wid)->cls->name);
		AG_LabelNewPolled(nTab, AG_LABEL_HFILL, _("Parent window: %p"), &wid->window);
		AG_LabelNewPolled(nTab, AG_LABEL_HFILL, _("Parent driver: %p"), &wid->drv);
		AG_LabelNewPolled(nTab, AG_LABEL_HFILL, _("Parent driver ops: %p"), &wid->drvOps);
		AG_SeparatorNewHoriz(nTab);

		sv = AG_ScrollviewNew(nTab, AG_SCROLLVIEW_EXPAND);
		AG_CheckboxSetFromFlags(sv, 0, &wid->flags, flagDescr);
	}

	if (AGOBJECT_CLASS(wid)->edit != NULL) {
		nTab = AG_NotebookAddTab(nb, _("Widget-specific"), AG_BOX_VERT);
		AG_ObjectAttach(nTab, AGOBJECT_CLASS(wid)->edit(wid));
	}
	
	nTab = AG_NotebookAddTab(nb, _("Variables"), AG_BOX_VERT);
	{
		AG_Tlist *tlSurf;
		
		tlSurf = AG_TlistNewPolled(nTab, AG_TLIST_EXPAND,
		    PollVariables, "%p", wid);
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

	AG_WidgetUpdate(box);
}

static void
ContextualMenu(AG_Event *event)
{
	AG_MenuItem *mi = AG_SENDER();
	AG_Tlist *tl = AG_PTR(1);
	AG_TlistItem *ti = AG_TlistSelectedItem(tl);

	if (ti == NULL)
		return;
	
	if (AG_OfClass(ti->p1, "AG_Widget:AG_Window:*")) {
		AG_Window *win = ti->p1;

		if (win->visible) {
			AG_MenuAction(mi, _("Hide window"), NULL,
			    HideWindow, "%p", win);
		} else {
			AG_MenuAction(mi, _("Show window"), NULL,
			    ShowWindow, "%p", win);
		}
	}
}

/* Create the GUI debugger window. Return NULL if window exists. */
AG_Window *
AG_GuiDebugger(void)
{
	AG_Window *win;
	AG_Pane *pane;
	AG_Tlist *tl;
	AG_MenuItem *mi;

	if ((win = AG_WindowNewNamedS(0, "AG_GuiDebugger")) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, _("GUI Debugger"));

	pane = AG_PaneNewHoriz(win, AG_PANE_EXPAND);

	tl = AG_TlistNewPolled(pane->div[0], 0, PollWidgets, NULL);
	AG_TlistSizeHint(tl, "<XXXXXXXXXXXXXXXXXXXX>", 10);
	AG_SetEvent(tl, "tlist-dblclick", WidgetSelected, "%p", pane->div[1]);
	AG_Expand(tl);
	AG_WidgetFocus(tl);

	mi = AG_TlistSetPopup(tl, "window");
	AG_MenuSetPollFn(mi, ContextualMenu, "%p", tl);

	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_MC, 70, 50);
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);
	return (win);
}

#endif /* AG_DEBUG */
