/*
 * Copyright (c) 2002-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * This tool allows a developer to inspect a live VFS of Agar widgets
 * and manipulate both generic and class-specific attributes of them.
 */

#include <agar/core/core.h>

#if defined(AG_WIDGETS) && defined(AG_DEBUG)

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

static AG_Window *_Nullable agDebuggerWindow = NULL;
static AG_Tlist  *_Nullable agDebuggerTlist = NULL;
static AG_Label  *_Nullable agDebuggerLabel = NULL;
static AG_Box    *_Nullable agDebuggerBox = NULL;

static void
FindWidgets(AG_Widget *_Nonnull wid, AG_Tlist *_Nonnull tl, int depth,
    Uint *_Nonnull nContainers, Uint *_Nonnull nLeaves)
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
		(*nContainers)++;
	} else {
		(*nLeaves)++;
	}
	if ((it->flags & AG_TLIST_HAS_CHILDREN) &&
	    AG_TlistVisibleChildren(tl, it)) {
		OBJECT_FOREACH_CHILD(widChld, wid, ag_widget)
			FindWidgets(widChld, tl, depth+1, nContainers, nLeaves);
	}
}

static void
FindWindows(AG_Tlist *_Nonnull tl, const AG_Window *_Nonnull win, int depth,
    Uint *_Nonnull nWindows, Uint *_Nonnull nContainers, Uint *_Nonnull nLeaves)
{
	const char *name = OBJECT(win)->name;
/*	AG_Window *wSub; */
	AG_Widget *wChild;
	AG_TlistItem *it;

	if (strcmp(name, "_agDbgr") == 0)			/* Unsafe */
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

	(*nWindows)++;

	if (!TAILQ_EMPTY(&OBJECT(win)->children) ||
	    !TAILQ_EMPTY(&win->pvt.subwins)) {
		it->flags |= AG_TLIST_HAS_CHILDREN;
	}
	if ((it->flags & AG_TLIST_HAS_CHILDREN) &&
	    AG_TlistVisibleChildren(tl, it)) {
//		TAILQ_FOREACH(wSub, &win->pvt.subwins, pvt.swins)
//			FindWindows(tl, wSub, depth+1, nWindows, nContainers, nLeaves);
		OBJECT_FOREACH_CHILD(wChild, win, ag_widget)
			FindWidgets(wChild, tl, depth+1, nContainers, nLeaves);
	}
}

static void
TargetRoot(void)
{
	agDebuggerTgt = NULL;
	if (agDebuggerWindow) {
		AG_WindowSetCaptionS(agDebuggerWindow,
		    _("Agar GUI Debugger: / (root)"));

		if (agDebuggerTlist)
			AG_TlistDeselectAll(agDebuggerTlist);

		if (agDebuggerLabel)
			AG_LabelText(agDebuggerLabel, _("Target: " AGSI_YEL "/" AGSI_RST));
	}
}

static void
PollWidgets(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_Label *lblStats = AG_LABEL_PTR(1);
	const AG_Window *tgt = agDebuggerTgtWindow;
	AG_Driver *drv;
	Uint nWindows=0, nContainers=0, nLeaves=0;

	AG_TlistBegin(tl);

	if (tgt != NULL && AG_OBJECT_VALID(tgt)) {
		FindWindows(tl, tgt, 0, &nWindows, &nContainers, &nLeaves);
	} else {					/* Retarget root */
		TargetRoot();
		AG_LockVFS(&agDrivers);
		AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
			AG_FOREACH_WINDOW(tgt, drv) {
				if (tgt == agDebuggerWindow) {
					continue;
				}
				FindWindows(tl, tgt, 0, &nWindows, &nContainers,
				    &nLeaves);
			}
		}
		AG_UnlockVFS(&agDrivers);
	}

	AG_TlistEnd(tl);

	AG_LabelText(lblStats, _("%u windows, %u containers & %u leaves (t = %ums)"),
	    nWindows, nContainers, nLeaves, (Uint)AG_GetTicks());
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

	if (S == NULL || strcmp(it->cat, "surface") != 0)
		return;
	
	AG_PixmapSetSurface(px, AG_PixmapAddSurfaceScaled(px, S, S->w, S->h));
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
	fd = AG_FileDlgNewMRU(win, "debugger-imgs",
	                      AG_FILEDLG_SAVE | AG_FILEDLG_CLOSEWIN |
	                      AG_FILEDLG_MASK_EXT | AG_FILEDLG_EXPAND);
	AG_FileDlgAddImageTypes(fd, ExportSurface, "%Cp", px);
	AG_WindowShow(win);
}

static void
PollSurfaces(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_Widget *wid = agDebuggerTgt;
	AG_TlistItem *it;
	Uint i;

	if (wid == NULL || !AG_OBJECT_VALID(wid) ||
	    !AG_OfClass(wid, "AG_Widget:*")) {
		TargetRoot();
		return;
	}

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
			it = AG_TlistAdd(tl, NULL, "Surface%u = NULL", i);
			it->cat = "null-surface";
		} else {
			if (wid->textures[i] != -1) {
				it = AG_TlistAdd(tl, S, "#%u (%ux%u, %ubpp, texture #%d%s%s)",
				    i, S->w, S->h, S->format.BitsPerPixel, wid->textures[i],
				    (wid->surfaceFlags[i] & AG_WIDGET_SURFACE_NODUP) ? ", <NODUP>" : "",
				    (wid->surfaceFlags[i] & AG_WIDGET_SURFACE_REGEN) ? ", <REGEN>" : "");
			} else {
				it = AG_TlistAdd(tl, S, "#%u (%ux%u, %ubpp%s%s)",
				    i, S->w, S->h, S->format.BitsPerPixel,
				    (wid->surfaceFlags[i] & AG_WIDGET_SURFACE_NODUP) ? ", <NODUP>" : "",
				    (wid->surfaceFlags[i] & AG_WIDGET_SURFACE_REGEN) ? ", <REGEN>" : "");
			}
			it->cat = "surface";
		}
		it->p1 = (AG_Surface *)S;
	}
	AG_TlistEnd(tl);
	AG_ObjectUnlock(wid);
}

static void
PollVariables(AG_Event *_Nonnull event)
{
	char val[AG_LABEL_MAX];
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_Object *obj = AGOBJECT(agDebuggerTgt);
	AG_Variable *V;

	if (obj == NULL || !AG_OBJECT_VALID(obj)) {
		TargetRoot();
		return;
	}
	
	val[0] = '\0';

	AG_ObjectLock(obj);
	AG_TlistBegin(tl);
	TAILQ_FOREACH(V, &obj->vars, vars) {
		if ((V->type == AG_VARIABLE_P_UINT ||
		     V->type == AG_VARIABLE_P_INT) &&
		     strcmp(V->name, "flags") == 0) {
			Snprintf(val, sizeof(val), "0x%08x", *(Uint *)V->data.p);
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

/* Select a widget for inspection in Debugger. */
static void
TargetWidget(AG_Event *_Nonnull event)
{
	AG_Box *box = agDebuggerBox;
	AG_TlistItem *ti = AG_TLIST_ITEM_PTR(1);
	AG_Widget *tgt = ti->p1;
	AG_Notebook *nb;
	AG_NotebookTab *nt;
	AG_Textbox *tb;
	int savedTabID;

	agDebuggerTgt = tgt;

	if ((nb = AG_ObjectFindChild(box, "notebook0")) != NULL) {
		savedTabID = nb->selTabID;
	} else {
		savedTabID = -1;
	}
	AG_ObjectFreeChildren(box);

	nb = AG_NotebookNew(box, AG_NOTEBOOK_EXPAND);
	nt = AG_NotebookAdd(nb, "AG_Widget", AG_BOX_VERT);
	nt->id = 1;
	{
		static const AG_FlagDescr flagDescr[] = {
		    { AG_WIDGET_FOCUSABLE,		"FOCUSABLE",		1 },
		    { AG_WIDGET_FOCUSED,		"FOCUSED",		0 },
		    { AG_WIDGET_DISABLED,		"DISABLED",		1 },
		    { AG_WIDGET_HIDE,			"HIDE",			1 },
		    { AG_WIDGET_VISIBLE,		"VISIBLE",		0 },
		    { AG_WIDGET_UNDERSIZE,		"UNDERSIZE",		0 },
		    { AG_WIDGET_HFILL,			"HFILL",		1 },
		    { AG_WIDGET_VFILL,			"VFILL",		1 },
		    { AG_WIDGET_USE_MOUSEOVER,		"USE_MOUSEOVER",	1 },
		    { AG_WIDGET_MOUSEOVER,		"MOUSEOVER",		0 },
#if 0
		    { AG_WIDGET_UPDATE_WINDOW,		"UPDATE_WINDOW",	1 },
		    { AG_WIDGET_USE_TEXT,		"USE_TEXT",		0 },
		    { AG_WIDGET_CATCH_TAB,		"CATCH_TAB",		1 },
		    { AG_WIDGET_USE_OPENGL,		"USE_OPENGL",		0 },
		    { AG_WIDGET_NOSPACING,		"NOSPACING",		1 },
#endif
		    { AG_WIDGET_UNFOCUSED_MOTION,	"UNFOCUSED_MOTION",	1 },
		    { AG_WIDGET_UNFOCUSED_BUTTONUP,	"UNFOCUSED_BUTTONUP",	1 },
		    { AG_WIDGET_UNFOCUSED_BUTTONDOWN,	"UNFOCUSED_BUTTONDOWN",	1 },
		    { AG_WIDGET_UNFOCUSED_KEYDOWN,	"UNFOCUSED_KEYDOWN",	1 },
		    { AG_WIDGET_UNFOCUSED_KEYUP,	"UNFOCUSED_KEYUP",	1 },
		    { 0,				NULL,0 }
		};
		AG_MSpinbutton *msb;
		AG_Box *vbox;

		AG_LabelNew(nt, AG_LABEL_HFILL,
		    _("Class: " AGSI_BOLD AGSI_GRN "%s" AGSI_RST),
		    OBJECT(tgt)->cls->name);

		tb = AG_TextboxNewS(nt, AG_TEXTBOX_HFILL | AG_TEXTBOX_NO_SHADING,
		    _("Name: "));
#ifdef AG_UNICODE
		AG_TextboxBindUTF8(tb, OBJECT(tgt)->name, sizeof(OBJECT(tgt)->name));
#else
		AG_TextboxBindASCII(tb, OBJECT(tgt)->name, sizeof(OBJECT(tgt)->name));
#endif
		AG_LabelNewPolledMT(nt, AG_LABEL_SLOW | AG_LABEL_HFILL,
		    &OBJECT(tgt)->lock,
		    _("Parent window: " AGSI_YEL "%[objName]" AGSI_RST
		      " @ (" AGSI_CYAN AGSI_COURIER "AG_Window" AGSI_RST AGSI_COURIER " *)%p"),
		    &tgt->window, &tgt->window);

		AG_LabelNewPolledMT(nt, AG_LABEL_SLOW | AG_LABEL_HFILL,
		    &OBJECT(tgt)->lock,
		    _("Parent driver: " AGSI_YEL "%[objName]" AGSI_RST
		      " @ (" AGSI_CYAN AGSI_COURIER "AG_Driver" AGSI_RST AGSI_COURIER " *)%p"),
		    &tgt->drv, &tgt->drv);

		AG_SeparatorNewHoriz(nt);

		vbox = AG_BoxNewVert(nt, AG_BOX_HFILL);
		AG_SetStyle(vbox, "padding", "4");
		{
			const Uint fl = AG_MSPINBUTTON_HFILL;

			msb = AG_MSpinbuttonNew(vbox, fl, "x", _("Size: "));
			AG_BindInt(msb, "xvalue", &tgt->w);
			AG_BindInt(msb, "yvalue", &tgt->h);
			AG_SetStyle(msb, "font-size", "120%");

			msb = AG_MSpinbuttonNew(vbox, fl, ",", _("Position: "));
			AG_BindInt(msb, "xvalue", &tgt->x);
			AG_BindInt(msb, "yvalue", &tgt->y);
			AG_SetStyle(msb, "font-size", "120%");

			msb = AG_MSpinbuttonNew(vbox, fl, ",", _("View UL: "));
			AG_BindInt(msb, "xvalue", &tgt->rView.x1);
			AG_BindInt(msb, "yvalue", &tgt->rView.y1);
			AG_SetStyle(msb, "font-size", "120%");

			msb = AG_MSpinbuttonNew(vbox, fl, ",", _("View LR: "));
			AG_BindInt(msb, "xvalue", &tgt->rView.x2);
			AG_BindInt(msb, "yvalue", &tgt->rView.y2);
			AG_SetStyle(msb, "font-size", "120%");
		}

		AG_SeparatorNewHoriz(nt);

		vbox = AG_BoxNewVert(nt, AG_BOX_VFILL);
		AG_CheckboxSetFromFlags(vbox, 0, &tgt->flags, flagDescr);
	}

	if (OBJECT_CLASS(tgt)->edit != NULL) {
		AG_Widget *editRv;

		nt = AG_NotebookAdd(nb, OBJECT(tgt)->cls->name, AG_BOX_VERT);
		nt->id = 2;

		editRv = OBJECT_CLASS(tgt)->edit(tgt);
		AG_ObjectAttach(nt, editRv);
		AG_WidgetCompileStyle(editRv);
		AG_Redraw(editRv);
	}
	nt = AG_NotebookAdd(nb, _("Variables"), AG_BOX_VERT);
	nt->id = 3;
	{
		AG_TlistNewPolledMs(nt, AG_TLIST_EXPAND, 333, PollVariables, NULL);
	}

	nt = AG_NotebookAdd(nb, _("Surfaces"), AG_BOX_VERT);
	nt->id = 4;
	{
		AG_Pane *pane;
		AG_Pixmap *px;
		AG_Tlist *tl;
		AG_MenuItem *mi;

		pane = AG_PaneNewVert(nt, AG_PANE_EXPAND);
		px = AG_PixmapNew(pane->div[0], AG_PIXMAP_EXPAND, 320, 240);

		tl = AG_TlistNewPolled(pane->div[1], AG_TLIST_EXPAND,
		    PollSurfaces, NULL);

		AG_SetEvent(tl, "tlist-selected", SelectedSurface,"%p",px);

		mi = AG_TlistSetPopup(tl, "surface");
		AG_MenuAction(mi, _("Export to image file..."), agIconSave.s,
		    ExportSurfaceDlg, "%Cp", px);
	}

	AG_NotebookSelectByID(nb, savedTabID);		/* Restore active tab */
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

static void
CloseDebuggerWindow(AG_Event *_Nonnull event)
{
	agDebuggerWindow = NULL;
	agDebuggerTlist = NULL;
	agDebuggerBox = NULL;
	agDebuggerTgtWindow = NULL;
}

void
AG_GuiDebuggerDetachTarget(void)
{
	if (agDebuggerBox) {
		AG_ObjectFreeChildren(agDebuggerBox);
	}
	agDebuggerTgt = NULL;
}

void
AG_GuiDebuggerDetachWindow(void)
{
	if (agDebuggerBox) {
		AG_ObjectFreeChildren(agDebuggerBox);
	}
	agDebuggerTgtWindow = NULL;
	TargetRoot();
}

static void
SetAutorefresh(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_PTR(1);
	const int enable = AG_INT(2);

	AG_TlistSetRefresh(tl, enable ? 250 : -1);
}

/*
 * Open the GUI debugger window (with tgt at the root).
 */
AG_Window *_Nullable
AG_GuiDebugger(AG_Window *_Nonnull tgt)
{
	char path[AG_OBJECT_PATH_MAX];
	AG_Window *win;
	AG_Pane *pane;
	AG_Tlist *tl;
	AG_MenuItem *mi;
	AG_Button *buRefresh;
	AG_Box *div, *toolbar;
	AG_Label *lblStats;

	if (tgt == NULL) {
		AG_TextError(_("No window is focused.\n"
		               "Focus on a window to target it in Debugger."));
		return (NULL);
	}
	if (tgt == agDebuggerWindow) {				/* Unsafe */
		return (NULL);
	}
	if ((win = agDebuggerWindow) != NULL) {
		AG_WindowFocus(win);
		if (agDebuggerTgtWindow != tgt) {
			agDebuggerTgtWindow = tgt;

			AG_WindowSetCaption(win,
			    _("Agar GUI Debugger: <%s> (\"%s\")"),
	   		    OBJECT(tgt)->name, AGWINDOW(tgt)->caption);

			if (agDebuggerBox)
				AG_ObjectFreeChildren(agDebuggerBox);

			AG_ObjectCopyName(tgt, path, sizeof(path));
			AG_LabelText(agDebuggerLabel,
	                    _("Target: " AGSI_YEL "%s" AGSI_RST), path);
		}
		return (win);
	}

	if ((win = agDebuggerWindow = AG_WindowNewNamedS(0, "_agDbgr")) == NULL)
		return (NULL);

	agDebuggerTgtWindow = tgt;

	AG_WindowSetCaption(win, _("Agar GUI Debugger: <%s> (\"%s\")"),
	                    OBJECT(tgt)->name, AGWINDOW(tgt)->caption);

	toolbar = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	AG_SetStyle(toolbar, "font-size", "150%");
	{
#if 0
		/* Set pick mode */
		btn = AG_ButtonNewFn(toolbar, AG_BUTTON_STICKY,
		    "\xe2\x87\xb1",                             /* U+21F1 */
		    SetPickStatus, "%p,%p", win, tl);
		AG_SetStyle(btn, "padding", "0 5 3 5");
#endif
		/* Toggle VFS autorefresh */
		buRefresh = AG_ButtonNewS(toolbar,
		    AG_BUTTON_STICKY | AG_BUTTON_SET,
		    "\xe2\xa5\x81");                             /* U+2941 */
		AG_SetStyle(buRefresh, "padding", "0 10 3 5");
	}

	pane = AG_PaneNewHoriz(win, AG_PANE_EXPAND);
	div = pane->div[0];
	agDebuggerBox = pane->div[1];

	AG_ObjectCopyName(tgt, path, sizeof(path));
	agDebuggerLabel = AG_LabelNew(div, AG_LABEL_HFILL,
	    _("Target: " AGSI_YEL "%s" AGSI_RST), path);

	lblStats = AG_LabelNewS(div, AG_LABEL_HFILL,
	    _("XX windows, XX containers & XX leaves (t = XXXXXms)"));
	AG_SetStyle(lblStats, "font-size", "80%");

	tl = agDebuggerTlist = AG_TlistNewPolledMs(div,
	    AG_TLIST_EXPAND, 125,
	    PollWidgets, "%p", lblStats);

	AG_TlistSizeHint(tl, "<XXXXXXXXXXXXXXXXXXXXXX>", 15);
	AG_SetEvent(tl, "tlist-selected", TargetWidget, NULL);
	AG_SetEvent(buRefresh, "button-pushed", SetAutorefresh, "%p", tl);

	mi = AG_TlistSetPopup(tl, "window");
	AG_MenuSetPollFn(mi, ContextualMenu, "%p", tl);

	AG_AddEvent(win, "window-close", CloseDebuggerWindow, NULL);
	AG_WindowSetGeometryAlignedPct(win, AG_WINDOW_BR, 40, 40);
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);
	AG_WidgetFocus(tl);
	return (win);
}

#endif /* AG_DEBUG */
