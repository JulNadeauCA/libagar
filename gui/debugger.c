/*
 * Copyright (c) 2002-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>

#if defined(AG_DEBUG) && defined(AG_TIMERS)

#include <agar/gui/gui.h>
#include <agar/gui/box.h>
#include <agar/gui/textbox.h>
#include <agar/gui/tlist.h>
#include <agar/gui/label.h>
#include <agar/gui/button.h>
#include <agar/gui/numerical.h>
#include <agar/gui/mspinbutton.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/separator.h>
#include <agar/gui/notebook.h>
#include <agar/gui/pane.h>
#include <agar/gui/scrollview.h>
#include <agar/gui/pixmap.h>
#include <agar/gui/file_dlg.h>
#include <agar/gui/cursors.h>
#include <agar/gui/icons.h>

#include <string.h>
#include <ctype.h>

static int agDbgrCounter = 0;

static void
FindWidgets(AG_Widget *_Nonnull wid, AG_Tlist *_Nonnull tl, int depth)
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
	it->flags |= AG_TLIST_ITEM_EXPANDED;
	
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
FindWindows(AG_Tlist *_Nonnull tl, const AG_Window *_Nonnull win, int depth)
{
	const char *name = OBJECT(win)->name;
	AG_Window *wSub;
	AG_Widget *wChild;
	AG_TlistItem *it;

	if ((strncmp(name, "menu", 4) == 0 ||
	     strncmp(name, "icon", 4) == 0) && isdigit(name[4]))
		return;

	if (strncmp(name, "win", 3) == 0 && isdigit(name[4])) {
		it = AG_TlistAddS(tl, NULL, win->caption[0] !='\0' ?
		                            win->caption : _("Untitled"));
	} else {
		it = AG_TlistAdd(tl, NULL, "<%s> (\"%s\")", name,
		    win->caption[0] != '\0' ? win->caption : _("Untitled"));
	}
	it->p1 = (AG_Window *)win;
	it->depth = depth;
	it->cat = "window";
	it->flags |= AG_TLIST_ITEM_EXPANDED;

	if (!TAILQ_EMPTY(&OBJECT(win)->children) ||
	    !TAILQ_EMPTY(&win->pvt.subwins)) {
		it->flags |= AG_TLIST_HAS_CHILDREN;
	}
	if ((it->flags & AG_TLIST_HAS_CHILDREN) &&
	    AG_TlistVisibleChildren(tl, it)) {
		TAILQ_FOREACH(wSub, &win->pvt.subwins, pvt.swins)
			FindWindows(tl, wSub, depth+1);
		OBJECT_FOREACH_CHILD(wChild, win, ag_widget)
			FindWidgets(wChild, tl, depth+1);
	}
}

static void
PollWidgets(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	const AG_Window *win = AG_CONST_WINDOW_PTR(1);
	AG_Driver *drv;
	
	AG_TlistClear(tl);
	if (win != NULL) {
		FindWindows(tl, win, 0);
	} else {
		AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
			AG_LockVFS(drv);
			AG_FOREACH_WINDOW(win, drv) {
				FindWindows(tl, win, 0);
			}
			AG_UnlockVFS(drv);
		}
	}
	AG_TlistRestore(tl);
}

static void
ShowWindow(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_PTR(1);

	AG_WindowShow(win);
}

static void
HideWindow(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_PTR(1);

	AG_WindowHide(win);
}

static void
SelectedSurface(AG_Event *_Nonnull event)
{
	AG_Pixmap *px = AG_PIXMAP_PTR(1);
	AG_TlistItem *it = AG_TLIST_ITEM_PTR(2);
	AG_Surface *S = it->p1;
	int Smapped;

	Smapped = AG_PixmapAddSurfaceScaled(px, S, S->w << 1, S->h << 1);
	AG_PixmapSetSurface(px, Smapped);
	AG_Redraw(px);
}

static void
ExportSurface(AG_Event *_Nonnull event)
{
	const AG_Pixmap *px = AG_CONST_PIXMAP_PTR(1);
	const char *path = AG_STRING(2);
	AG_Surface *S;

	if (px->n == -1)
		return;

	S = AG_PixmapGetSurface(px, px->n);
	if (AG_SurfaceExportFile(S, path) == 0) {
		AG_TextTmsg(AG_MSG_INFO, 2000,
		    _("Exported %u x %u x %ubpp surface to:\n%s"),
		    S->w, S->h, S->format.BitsPerPixel, path);
	} else {
		AG_TextMsgFromError();
	}
	AG_SurfaceFree(S);
}

static void
ExportSurfaceDlg(AG_Event *_Nonnull event)
{
	const AG_Pixmap *px = AG_CONST_PIXMAP_PTR(1);
	AG_Window *win;
	AG_FileDlg *fd;
	
	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaptionS(win, _("Export to image file..."));
	fd = AG_FileDlgNewMRU(win, "agar.debugger.image-dir",
	                      AG_FILEDLG_SAVE | AG_FILEDLG_CLOSEWIN |
	                      AG_FILEDLG_MASK_EXT | AG_FILEDLG_EXPAND);
	AG_FileDlgAddImageTypes(fd, ExportSurface, "%Cp", px);
	AG_WindowShow(win);
}

static void
PollSurfaces(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_Widget *wid = AG_WIDGET_PTR(1);
	AG_TlistItem *it;
	Uint i;

	AG_ObjectLock(wid);
	AG_TlistBegin(tl);
	for (i = 0; i < wid->nSurfaces; i++) {
		const AG_Surface *S = WSURFACE(wid,i);

		/* Sometimes WSURFACE returns NULL. This may be a bug
		 * elsewhere, maybe in the glxdriver, causing wid->nSurfaces
		 * to be inconsistent with the number of non-null surfaces.
		 * Without this check, S will get DE referenced and cause
		 * a segfault. */
		if (S == NULL) {
			AG_TlistAdd(tl, NULL, "Surface%u = NULL", i);
			continue;
		}
		it = AG_TlistAdd(tl, S, "Surface%u (%ux%u, %ubpp)",
		    i, S->w, S->h, S->format.BitsPerPixel);

		it->p1 = (AG_Surface *)S;
		it->cat = "surface";
	}
	AG_TlistEnd(tl);
	AG_ObjectUnlock(wid);
}

static void
PollVariables(AG_Event *_Nonnull event)
{
	char val[AG_LABEL_MAX];
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_Object *obj = AG_OBJECT_PTR(1);
	AG_Variable *V;

	AG_ObjectLock(obj);
	AG_TlistBegin(tl);
	TAILQ_FOREACH(V, &obj->vars, vars) {
		if ((V->type == AG_VARIABLE_P_UINT ||
		     V->type == AG_VARIABLE_P_INT) &&
		     strcmp(V->name, "flags") == 0) {
			Snprintf(val, sizeof(val), "0x%08x",
			    *(Uint *)V->data.p);
		} else {
			AG_PrintVariable(val, sizeof(val), V);
		}
		switch (V->type) {
		case AG_VARIABLE_P_FLAG:
		case AG_VARIABLE_P_FLAG8:
		case AG_VARIABLE_P_FLAG16:
		case AG_VARIABLE_P_FLAG32:
			AG_TlistAdd(tl, NULL, "%s %s [mask 0x%x] = %s",
			    agVariableTypes[V->type].name, V->name,
			        V->info.bitmask.u,
			        val);
			break;
		default:
			AG_TlistAdd(tl, NULL, "%s %s = %s",
			    agVariableTypes[V->type].name, V->name, val);
			break;
		}
	}
	AG_TlistEnd(tl);
	AG_ObjectUnlock(obj);
}

static void
PollCursors(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_Widget *wid = AG_WIDGET_PTR(1);
	AG_Driver *drv;
	AG_TlistItem *it;
	AG_Cursor *cu;

	AG_ObjectLock(wid);
	drv = wid->drv;
	AG_TlistBegin(tl);
	TAILQ_FOREACH(cu, &drv->cursors, cursors) {
		it = AG_TlistAdd(tl, NULL, "%dx%d (%d,%d); p=%p", cu->w, cu->h, 
		    cu->xHot, cu->yHot, cu->p);
		it->p1 = cu;
	}
	AG_TlistEnd(tl);
	AG_ObjectUnlock(wid);
}

static void
WidgetSelected(AG_Event *_Nonnull event)
{
	AG_Box *box = AG_BOX_PTR(1);
	AG_TlistItem *ti = AG_TLIST_ITEM_PTR(2);
	AG_Widget *wid = ti->p1;
	AG_Notebook *nb;
	AG_NotebookTab *nt;
	AG_Textbox *tb;
	AG_MSpinbutton *msb;
	AG_Scrollview *sv;

	AG_ObjectFreeChildren(box);

	nb = AG_NotebookNew(box, AG_NOTEBOOK_EXPAND);
	/* XXX workaround style issue */
	AG_SetStyle(nb, "color#selected", "rgb(125,125,125)");
	nt = AG_NotebookAdd(nb, "AG_Widget", AG_BOX_VERT);
	{
		static const AG_FlagDescr flagDescr[] = {
		    { AG_WIDGET_FOCUSABLE,		"FOCUSABLE",		1 },
		    { AG_WIDGET_FOCUSED,		"FOCUSED",		0 },
		    { AG_WIDGET_DISABLED,		"DISABLED",		1 },
		    { AG_WIDGET_HIDE,			"HIDE",			1 },
		    { AG_WIDGET_VISIBLE,		"VISIBLE",		0 },
		    { AG_WIDGET_UNDERSIZE,		"UNDERSIZE",		0 },
		    { AG_WIDGET_UPDATE_WINDOW,		"UPDATE_WINDOW",	1 },
		    { AG_WIDGET_HFILL,			"HFILL",		1 },
		    { AG_WIDGET_VFILL,			"VFILL",		1 },
		    { AG_WIDGET_USE_MOUSEOVER,		"USE_MOUSEOVER",	1 },
		    { AG_WIDGET_MOUSEOVER,		"MOUSEOVER",		0 },
		    { AG_WIDGET_USE_TEXT,		"USE_TEXT",		0 },
		    { AG_WIDGET_CATCH_TAB,		"CATCH_TAB",		1 },
		    { AG_WIDGET_USE_OPENGL,		"USE_OPENGL",		0 },
		    { AG_WIDGET_NOSPACING,		"NOSPACING",		1 },
		    { AG_WIDGET_UNFOCUSED_MOTION,	"UNFOCUSED_MOTION",	1 },
		    { AG_WIDGET_UNFOCUSED_BUTTONUP,	"UNFOCUSED_BUTTONUP",	1 },
		    { AG_WIDGET_UNFOCUSED_BUTTONDOWN,	"UNFOCUSED_BUTTONDOWN",	1 },
		    { AG_WIDGET_UNFOCUSED_KEYDOWN,	"UNFOCUSED_KEYDOWN",	1 },
		    { AG_WIDGET_UNFOCUSED_KEYUP,	"UNFOCUSED_KEYUP",	1 },
		    { 0,				NULL,0 }
		};

		tb = AG_TextboxNewS(nt, AG_TEXTBOX_HFILL, _("Name: "));
#ifdef AG_UNICODE
		AG_TextboxBindUTF8(tb, OBJECT(wid)->name,
		    sizeof(OBJECT(wid)->name));
#else
		AG_TextboxBindASCII(tb, OBJECT(wid)->name,
		    sizeof(OBJECT(wid)->name));
#endif
		AG_LabelNew(nt, 0, _("Class: " AGSI_BOLD "%s" AGSI_RST "(3)"), OBJECT(wid)->cls->name);
#ifdef AG_ENABLE_STRING
		AG_LabelNewPolledMT(nt, AG_LABEL_HFILL, &OBJECT(wid)->pvt.lock,
		    _("Parent window: " AGSI_YEL "%[objName]" AGSI_RST
		      " @ (" AGSI_CYAN "AG_Window" AGSI_RST " *)%p"),
		    &wid->window, &wid->window);
		AG_LabelNewPolledMT(nt, AG_LABEL_HFILL, &OBJECT(wid)->pvt.lock,
		    _("Parent driver: " AGSI_YEL "%[objName]" AGSI_RST
		      " @ (" AGSI_CYAN "AG_Driver" AGSI_RST " *)%p"),
		    &wid->drv, &wid->drv);
		AG_LabelNewPolledMT(nt, AG_LABEL_HFILL, &OBJECT(wid)->pvt.lock,
		    _("Driver class: " AGSI_BOLD "%[objClassName]" AGSI_RST "(3)"
		      " @ (" AGSI_CYAN "AG_DriverClass" AGSI_RST " *)%p"),
		    &wid->drvOps, &wid->drvOps);
#endif
		AG_SeparatorNewHoriz(nt);

		sv = AG_ScrollviewNew(nt, AG_SCROLLVIEW_BY_MOUSE |
		                          AG_SCROLLVIEW_EXPAND);
		AG_CheckboxSetFromFlags(sv, 0, &wid->flags, flagDescr);
		AG_SetStyle(sv, "font-family", "Courier");
		AG_SetStyle(sv, "font-size", "80%");
	}

	if (OBJECT_CLASS(wid)->edit != NULL) {
		nt = AG_NotebookAdd(nb, OBJECT(wid)->cls->name, AG_BOX_VERT);
		AG_ObjectAttach(nt, OBJECT_CLASS(wid)->edit(wid));
	}
	nt = AG_NotebookAdd(nb, _("Variables"), AG_BOX_VERT);
	{
		AG_TlistNewPolled(nt, AG_TLIST_EXPAND,
		    PollVariables, "%p", wid);
	}
	nt = AG_NotebookAdd(nb, _("Geometry"), AG_BOX_VERT);
	{
		msb = AG_MSpinbuttonNew(nt, 0, ",", "Container coords: ");
		AG_BindInt(msb, "xvalue", &wid->x);
		AG_BindInt(msb, "yvalue", &wid->y);

		msb = AG_MSpinbuttonNew(nt, 0, "x", "Geometry: ");
		AG_BindInt(msb, "xvalue", &wid->w);
		AG_BindInt(msb, "yvalue", &wid->h);
		
		msb = AG_MSpinbuttonNew(nt, 0, ",", "View coords (UL): ");
		AG_BindInt(msb, "xvalue", &wid->rView.x1);
		AG_BindInt(msb, "yvalue", &wid->rView.y1);
		
		msb = AG_MSpinbuttonNew(nt, 0, ",", "View coords (LR): ");
		AG_BindInt(msb, "xvalue", &wid->rView.x2);
		AG_BindInt(msb, "yvalue", &wid->rView.y2);
	}
	nt = AG_NotebookAdd(nb, _("Surfaces"), AG_BOX_VERT);
	AG_BoxSetHomogenous(&nt->box, 1);
	{
		AG_Pane *pane;
		AG_Pixmap *px;
		AG_Tlist *tl;
		AG_MenuItem *mi;

		pane = AG_PaneNewVert(nt, AG_PANE_EXPAND);
		px = AG_PixmapNew(pane->div[0], AG_PIXMAP_EXPAND, 320, 240);

		tl = AG_TlistNewPolled(pane->div[1], AG_TLIST_EXPAND,
		    PollSurfaces, "%p", wid);
		AG_SetEvent(tl, "tlist-selected",
		    SelectedSurface, "%p", px);

		mi = AG_TlistSetPopup(tl, "surface");
		AG_MenuAction(mi, _("Export to image file..."), agIconSave.s,
		    ExportSurfaceDlg, "%Cp", px);

		AG_PaneMoveDividerPct(pane, 50);
	}

	/* Don't show the cursors tab if the widget doesn't have a driver (i.e.
	 * if it is the driver), as this will cause a segfault when we try to
	 * generate the cursor list. */
	if (wid->drv != NULL) {
		nt = AG_NotebookAdd(nb, _("Cursors"), AG_BOX_VERT);
		{
#ifdef AG_ENABLE_STRING
			AG_Driver *drv = wid->drv;

			AG_LabelNewPolled(nt, AG_LABEL_HFILL, _("Cursor driver: %s"),
			    OBJECT(drv)->name);
			AG_LabelNewPolled(nt, AG_LABEL_HFILL, _("Total cursors: %u"),
			    &drv->nCursors);
#endif
			AG_TlistNewPolled(nt, AG_TLIST_EXPAND,
			    PollCursors, "%p", wid);
		}
	}

	AG_WidgetShowAll(box);
	AG_WidgetUpdate(box);
}

static void
ContextualMenu(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_PTR(1);
	AG_MenuItem *mi = AG_MENU_ITEM_PTR(2);
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
AG_Window *_Nullable
AG_GuiDebugger(AG_Window *_Nonnull obj)
{
	AG_Window *win;
	AG_Pane *pane;
	AG_Tlist *tl;
	AG_MenuItem *mi;

	if ((win = AG_WindowNewNamed(0, "dbgr%u", agDbgrCounter++)) == NULL) {
		return (NULL);
	}
	if (win != NULL) {
		AG_WindowSetCaption(win,
		    _("Agar GUI Debugger: <%s> (\"%s\")"),
		    OBJECT(obj)->name, AGWINDOW(obj)->caption);
	} else {
		AG_WindowSetCaptionS(win, _("Agar GUI Debugger"));
	}

	pane = AG_PaneNewHoriz(win, AG_PANE_EXPAND);

	tl = AG_TlistNewPolled(pane->div[0], 0, PollWidgets, "%Cp", obj);
	AG_TlistSizeHint(tl, "<XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX>", 10);
	AG_SetEvent(tl, "tlist-dblclick", WidgetSelected, "%p", pane->div[1]);
	AG_Expand(tl);
	AG_WidgetFocus(tl);
	mi = AG_TlistSetPopup(tl, "window");
	AG_MenuSetPollFn(mi, ContextualMenu, "%p", tl);
	AG_WindowSetGeometryAligned(win, AG_WINDOW_BR, 1000, 550);
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);
	return (win);
}

#endif /* AG_DEBUG and AG_TIMERS */
