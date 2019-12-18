/*
 * Copyright (c) 2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * This tool allows the user to inspect Agar widget instances, and also:
 *
 * - set local style attributes on live widgets
 * - create/edit style rules at the CSS level
 * - edit Agar stylesheet files (with syntax highlighting)
 */

#include <agar/core/core.h>

#ifdef AG_WIDGETS

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

static int agStylCounter = 0;

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
	
	AG_TlistBegin(tl);

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

	AG_TlistEnd(tl);
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
PollVariables(AG_Event *_Nonnull event)
{
	char val[AG_LABEL_MAX];
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_Object *obj = AG_OBJECT_PTR(1);
	AG_Variable *V;
	Uint nVars=0;

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
			AG_TlistAdd(tl, NULL, AGSI_CYAN "%s" AGSI_RST " "
			                      AGSI_YEL "%s" AGSI_RST
			                      " [mask "
					      AGSI_RED "0x%x" AGSI_RST "] = "
					      AGSI_BOLD "%s" AGSI_RST,
			    agVariableTypes[V->type].name, V->name,
			    V->info.bitmask.u, val);
			break;
		default:
			AG_TlistAdd(tl, NULL, AGSI_CYAN "%s" AGSI_RST " "
			                      AGSI_YEL "%s" AGSI_RST " = "
			                      AGSI_BOLD "%s" AGSI_RST,
			    agVariableTypes[V->type].name, V->name, val);
			break;
		}
		nVars++;
	}
	if (nVars == 0) {
		AG_TlistAddS(tl, NULL, AGSI_FAINT "(no variables)" AGSI_RST);
	}
	AG_TlistEnd(tl);
	AG_ObjectUnlock(obj);
}

static void WidgetSelected(AG_Event *_Nonnull);

static void
InputAttribute(AG_Event *_Nonnull event)
{
	AG_Textbox *tb = AG_TEXTBOX_PTR(1);
	AG_Widget *tgt = AG_WIDGET_PTR(2);
	char *s = AG_TextboxDupString(tb), *ps = s;
	const char *key = Strsep(&ps, ":");
	const char *val = Strsep(&ps, ":");
	
	AG_OBJECT_ISA(tgt, "AG_Widget:*");

	if (key == NULL || val == NULL)
		return;

	while (isspace(*key)) { key++; }
	while (isspace(*val)) { val++; }

	AG_SetStyle(tgt, key, val[0] != '\0' ? val : NULL);
	AG_WindowUpdate(AG_ParentWindow(tgt));
	AG_TextboxClearString(tb);

	free(s);
}

static void
PollAttributes(AG_Event *_Nonnull event)
{
	char text[AG_TLIST_LABEL_MAX];
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_Widget *tgt = AG_WIDGET_PTR(1);
	const char **attr;

	AG_TlistBegin(tl);

	for (attr = &agStyleAttributes[0]; *attr != NULL; attr++) {
		char *attrVal;

		if (!AG_Defined(tgt, *attr)) {
			continue;
		}
		attrVal = AG_GetStringP(tgt, *attr);
		Strlcpy(text, *attr,   sizeof(text));
		Strlcat(text, ": ",    sizeof(text));
		Strlcat(text, attrVal, sizeof(text));
		AG_TlistAddPtr(tl, NULL, text, attrVal);
	}

	AG_TlistEnd(tl);
}

static void
WidgetSelected(AG_Event *_Nonnull event)
{
	AG_Box *box = AG_BOX_PTR(1);
	AG_Button *buCapture = AG_BUTTON_PTR(2);
	AG_TlistItem *ti = AG_TLIST_ITEM_PTR(3);
	AG_Widget *tgt = ti->p1;
	AG_Notebook *nb;
	AG_NotebookTab *nt;
	AG_Tlist *tlAttrs;
	int savedTabID;

	AG_OBJECT_ISA(tgt, "AG_Widget:*");

	if ((nb = AG_ObjectFindChild(box, "notebook0")) != NULL) {
		savedTabID = nb->selTabID;
	} else {
		savedTabID = -1;
	}

	AG_ObjectFreeChildren(box);
	
	nb = AG_NotebookNew(box, AG_NOTEBOOK_EXPAND);
	nt = AG_NotebookAdd(nb, _("Style Attributes"), AG_BOX_VERT);
	{
		AG_Textbox *tb;
		AG_Box *hBox;

		tlAttrs = AG_TlistNewPolledMs(nt, AG_TLIST_EXPAND, 333,
		    PollAttributes, "%p", tgt);

		hBox = AG_BoxNewHoriz(nt, AG_BOX_HFILL);
		tb = AG_TextboxNewS(hBox, AG_TEXTBOX_HFILL |
		                          AG_TEXTBOX_RETURN_BUTTON, "+");
		AG_TextboxSizeHint(tb, "<XXXXXXXXXXX>: <XXXXXXXXXXX>");
		AG_SetEvent(tb, "textbox-return", InputAttribute, "%p,%p", tb,tgt);
		AG_WidgetFocus(tb);
	}

	nt = AG_NotebookAdd(nb, _("Variables"), AG_BOX_VERT);
	{
		AG_Tlist *tl;

		tl = AG_TlistNewPolled(nt, AG_TLIST_EXPAND, PollVariables, "%p", tgt);
		AG_SetStyle(tl, "font-family", "courier-prime");
		AG_SetStyle(tl, "font-size", "120%");
	}
	
	nt = AG_NotebookAdd(nb, _("Appearance"), AG_BOX_VERT);
	if (AG_ButtonGetState(buCapture)) {
		AG_Pixmap *px;
		AG_Label *lbl;
		AG_Surface *S;

		if ((S = AG_WidgetSurface(tgt)) != NULL) {
			AG_Scrollview *sv;
			AG_Surface *Sx;
#if 0
			Sx = AG_SurfaceScale(S, (S->w << 1), (S->h << 1), 0);
#else
			Sx = S;
#endif
			lbl = AG_LabelNew(nt, 0, _("Size: %d x %d px"), S->w, S->h);
			AG_SetStyle(lbl, "font-size", "70%");

			sv = AG_ScrollviewNew(nt, AG_SCROLLVIEW_EXPAND |
	                                          AG_SCROLLVIEW_BY_MOUSE |
				                  AG_SCROLLVIEW_PAN_LEFT |
				                  AG_SCROLLVIEW_PAN_RIGHT |
						  AG_SCROLLVIEW_FRAME);

			px = AG_PixmapFromSurface(sv, AG_PIXMAP_EXPAND, Sx);
			AG_SurfaceFree(Sx);
#if 0
			AG_SeparatorNewHoriz(nt);
			AG_ButtonNewFn(nt, 0, _("Export image..."),
			    ExportImageDlg, "%p", S); /* TODO */
#endif
		} else {
			AG_LabelNewS(nt, 0, AG_GetError());
		}
	} else {
		AG_Label *lbl;

		lbl = AG_LabelNewS(nt, 0,
		    _("Capture is disabled. Click on \xe2\x96\xa6 (and refresh) to enable."));
		AG_SetStyle(lbl, "font-family", "dejavu-sans");
	}
	
	AG_NotebookSelectByID(nb, savedTabID);		/* Restore active tab */
	AG_WidgetShowAll(box);
	AG_WidgetUpdate(box);
}

static void
MenuForWindow(AG_Event *_Nonnull event)
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
SetPickStatus(AG_Event *_Nonnull event)
{
}

static void
SetListRefresh(AG_Event *_Nonnull event)
{
	AG_Tlist *tlVFS = AG_TLIST_PTR(1);
	const int enable = AG_INT(2);

	AG_TlistSetRefresh(tlVFS, enable ? 250 : -1);
}

/* Open the specified window in the GUI Style Editor */
AG_Window *_Nullable
AG_StyleEditor(AG_Window *_Nonnull tgt)
{
	AG_Window *win;
	AG_Pane *pane;
	AG_MenuItem *mi;
	AG_Box *hBox;
	AG_Tlist *tlVFS;
	AG_Button *buCapture;

	if (tgt == NULL) {
		AG_TextError(_("No window is focused.\n"
		               "Focus on a window to target it in Style Editor."));
		return (NULL);
	}
	if ((win = AG_WindowNewNamed(0, "sted%u", agStylCounter++)) == NULL) {
		return (NULL);
	}
	AG_WindowSetCaption(win, _("Style Editor: <%s> (\"%s\")"),
	    AGOBJECT(tgt)->name,
	    AGWINDOW(tgt)->caption);
	
	tlVFS = AG_TlistNewPolledMs(NULL, 0, 125, PollWidgets, "%Cp", tgt);
	AG_TlistSizeHint(tlVFS, "<XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX>", 10);
	AG_Expand(tlVFS);

	hBox = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	AG_SetStyle(hBox, "font-family", "dejavu-sans");
	AG_SetStyle(hBox, "font-size", "150%");
	{
		AG_Button *btn;

		/* Set pick mode */
		btn = AG_ButtonNewFn(hBox, AG_BUTTON_STICKY,
		    "\xe2\x87\xb1",				/* U+21F1 */
		    SetPickStatus, "%p", win);
		AG_ButtonSetPadding(btn, 5,5,0,0);

		/* Toggle VFS autorefresh */
		btn = AG_ButtonNewFn(hBox, AG_BUTTON_STICKY | AG_BUTTON_SET,
		    "\xe2\xa5\x81",				/* U+2941 */
		    SetListRefresh, "%p", tlVFS);
		AG_ButtonSetPadding(btn, 5,10,0,0);

		/* Toggle appearance capture */
		buCapture = AG_ButtonNewS(hBox, AG_BUTTON_STICKY,
		                                "\xe2\x96\xa6"); /* U+2941 */
		AG_ButtonSetPadding(buCapture, 4,7,0,0);
	}

	pane = AG_PaneNewHoriz(win, AG_PANE_EXPAND);
	{
		AG_SetEvent(tlVFS, "tlist-dblclick",
		    WidgetSelected, "%p,%p", pane->div[1], buCapture);
		AG_ObjectAttach(pane->div[0], tlVFS);
		AG_WidgetFocus(tlVFS);

		(AG_SpacerNewHoriz(pane->div[1]))->minLen = 400;

		mi = AG_TlistSetPopup(tlVFS, "window");
		AG_MenuSetPollFn(mi, MenuForWindow, "%p", tlVFS);
	}

	AG_WindowSetPosition(win, AG_WINDOW_BC, 1);
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);
	return (win);
}

#endif /* AG_WIDGETS */
