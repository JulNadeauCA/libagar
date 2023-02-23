/*
 * Copyright (c) 2002-2023 Julien Nadeau Carriere <vedge@csoft.net>
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
	if (AG_TlistVisibleChildren(tl, it)) {
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
	if (AG_TlistVisibleChildren(tl, it)) {
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
			AG_LabelText(agDebuggerLabel, _("Target: " AGSI_PATH AGSI_YEL "/" AGSI_RST));
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

	AG_LabelText(lblStats,
	    _("%u windows, %u containers & %u leaves (t = %ums)"),
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
	AG_Label *lblInfo = AG_LABEL_PTR(2);
	AG_TlistItem *it = AG_TLIST_ITEM_PTR(3);
	AG_Surface *S = it->p1;
	const char *grayscaleModeNames[] = {  /* Sync with AG_GrayscaleMode */
		"BT.709",
		"R-Y",
		"Y"
	};

	if (S == NULL || strcmp(it->cat, "surface") != 0)
		return;

	if (S->w < 1 || S->w > 2560 ||
	    S->h < 1 || S->h > 1440)
		return;

	switch (S->format.mode) {
	case AG_SURFACE_PACKED:
		AG_LabelText(lblInfo,
		    _("Surface Mode: Packed.\n"
		      "Surface Flags: "  AGSI_CODE "0x%04x"  AGSI_RST ".\n"
		      "Surface Size: %u x %u.\n"
		      "Pitch: %u, Padding: %u.\n"
		      "Guides: %u %u %u %u.\n"
		      "Bits Per Pixel: %d.\n"
		      "RGBA Masks: " AGSI_CODE AGSI_RED "0x%08lx" AGSI_RST ","
		                     AGSI_CODE AGSI_GRN "0x%08lx" AGSI_RST ","
		                     AGSI_CODE AGSI_BLU "0x%08lx" AGSI_RST ","
		                              AGSI_CODE "0x%08lx" AGSI_RST "\n"),
		    S->flags, S->w, S->h, S->pitch, S->padding,
		    S->guides[0], S->guides[1], S->guides[2], S->guides[3],
		    S->format.BitsPerPixel,
		    (Ulong)S->format.Rmask,
		    (Ulong)S->format.Gmask,
		    (Ulong)S->format.Bmask,
		    (Ulong)S->format.Amask);
		break;
	case AG_SURFACE_INDEXED:
		AG_LabelText(lblInfo,
		    _("Surface Mode: Indexed.\n"
		      "Surface Flags: " AGSI_CODE "0x%x" AGSI_RST ".\n"
		      "Surface Size: %d x %d.\n"
		      "Pitch: %u, Padding: %u.\n"
		      "Bits Per Pixel: %d.\n"
		      "Palette Entries: " AGSI_BOLD "%d" AGSI_RST ".\n"),
		    S->flags, S->w, S->h, S->pitch, S->padding,
		    S->format.BitsPerPixel,
		    S->format.palette->nColors);
		break;
	case AG_SURFACE_GRAYSCALE:
		AG_LabelText(lblInfo,
		    _("Surface Mode: Grayscale.\n"
		      "Surface Flags: " AGSI_CODE "0x%x" AGSI_RST ".\n"
		      "Surface Size: %d x %d.\n"
		      "Pitch: %u, Padding: %u.\n"
		      "Bits Per Pixel: %d.\n"
		      "Grayscale Mode: " AGSI_BOLD "%s" AGSI_RST ".\n"),
		    S->flags, S->w, S->h, S->pitch, S->padding,
		    S->format.BitsPerPixel,
		    grayscaleModeNames[S->format.graymode]);
		break;
	default:
		break;
	}

	AG_PixmapSetSurface(px, AG_PixmapAddSurfaceScaled(px, S, S->w, S->h));
	AG_WindowUpdate(WIDGET(px)->window);
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
				it = AG_TlistAdd(tl, S,
				    AGSI_ALGUE AGSI_L_ARROW AGSI_RST
				    " Surface#%u (%ux%u, %ubpp, texture #%d%s%s)",
				    i, S->w, S->h, S->format.BitsPerPixel, wid->textures[i],
				    (wid->surfaceFlags[i] & AG_WIDGET_SURFACE_NODUP) ? ", <NODUP>" : "",
				    (wid->surfaceFlags[i] & AG_WIDGET_SURFACE_REGEN) ? ", <REGEN>" : "");
			} else {
				it = AG_TlistAdd(tl, S,
				    AGSI_ALGUE AGSI_L_ARROW AGSI_RST
				    " Surface#%u (%ux%u, %ubpp%s%s)",
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
		AG_TlistItem *it;

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
			it = AG_TlistAdd(tl, NULL, "%s %s [mask 0x%x] = %s",
			    agVariableTypes[V->type].name, V->name,
			        V->info.bitmask.u,
			        val);
			it->p1 = V;
			break;
		default:
			it = AG_TlistAdd(tl, NULL, "%s %s = %s",
			    agVariableTypes[V->type].name, V->name, val);
			it->p1 = V;
			break;
		}
	}
	AG_TlistEnd(tl);
	AG_ObjectUnlock(obj);
}

static void
FlagChanged(AG_Event *_Nonnull event)
{
	AG_Widget *tgt = AG_WIDGET_PTR(1);

	Debug(tgt, "Flags changed in debugger (0x%x)\n",
	    WIDGET(tgt)->flags);

	AG_Redraw(tgt);
}

static void
InputVariable(AG_Event *_Nonnull event)
{
	AG_Textbox *tb = AG_TEXTBOX_PTR(1);
	AG_Widget *tgt = (AG_Widget *)AG_PTR(2);
	char *s = AG_TextboxDupString(tb), *ps = s;
	const char *key = Strsep(&ps, ":=");
	const char *val = Strsep(&ps, ":=");
	const char *c;
	int floatChars;

	if (!AG_OBJECT_VALID(tgt) || !AG_OfClass(tgt, "AG_Widget:*")) {
		Debug(NULL, "Invalid target %p\n", tgt);
		return;
	}
	if (key == NULL || val == NULL)
		return;

	while (isspace(*key)) { key++; }
	while (isspace(*val)) { val++; }

	/*
	 * Try to infer type from the value.
	 */
	for (c=val, floatChars=0;
	     *c != '\0';
	     c++) {
		if (*c == '.' || *c == 'e') {
			floatChars++;
			continue;
		}
		if (!isdigit(*c) && *c != '-')
			break;
	}
	if (*c == '\0') {
		if (floatChars) {
			AG_SetFloat(tgt, key, atof(val));
		} else {
			AG_SetInt(tgt, key, atoi(val));
		}
	} else {
		if (val[0] == '0' && (val[1] == 'x' || val[1] == 'X') &&
		    val[2] != '\0') {
			AG_SetUint(tgt, key, (Uint)strtol(val,NULL,16));
		} else {
			if (Strcasecmp(val, "TRUE") == 0) {
				AG_SetBool(tgt, key, 1);
			} else if (Strcasecmp(val, "FALSE") == 0) {
				AG_SetBool(tgt, key, 0);
			} else {
				AG_SetString(tgt, key, val);
			}
		}
	}

	AG_WindowUpdate(AG_ParentWindow(tgt));
	AG_TextboxClearString(tb);

	free(s);
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
	nt = AG_NotebookAdd(nb,
	    AGSI_IDEOGRAM AGSI_SMALL_WINDOW AGSI_RST " AG_Widget",
	    AG_BOX_VERT);
	AG_SetPadding(nt, "3 4 3 4");
	nt->id = 1;
	{
		static const AG_FlagDescr flagDescr[] = {
		    { AG_WIDGET_HIDE,          "HIDE",          1 },
		    { AG_WIDGET_DISABLED,      "DISABLED",      1 },
		    { AG_WIDGET_FOCUSABLE,     "FOCUSABLE",     1 },
		    { AG_WIDGET_VISIBLE,       "VISIBLE",       0 },
		    { AG_WIDGET_FOCUSED,       "FOCUSED",       0 },
		    { AG_WIDGET_UNDERSIZE,     "UNDERSIZE",     0 },
		    { AG_WIDGET_HFILL,         "HFILL",         0 },
		    { AG_WIDGET_VFILL,         "VFILL",         0 },
		    { AG_WIDGET_USE_MOUSEOVER, "USE_MOUSEOVER", 1 },
		    { AG_WIDGET_MOUSEOVER,     "MOUSEOVER",     0 },
		    { AG_WIDGET_USE_TEXT,      "USE_TEXT",      0 },
		    { AG_WIDGET_USE_OPENGL,    "USE_OPENGL",    0 },
		    { 0,                       NULL,            0 }
		};
		AG_Box *boxWide, *boxVert;
		AG_MSpinbutton *msb;
		AG_Label *lbl;

		lbl = AG_LabelNew(nt, AG_LABEL_HFILL,
		    _("Widget Structure of " AGSI_BOLD "%s" AGSI_RST " "
		      "(" AGSI_CYAN "%s" AGSI_RST ")"),
		    OBJECT(tgt)->name, OBJECT(tgt)->cls->hier);
		AG_LabelJustify(lbl, AG_TEXT_CENTER);
		AG_SetFontFamily(lbl, "league-spartan");
		AG_SetFontSize(lbl, "130%");

		AG_SeparatorNewHoriz(nt);

		AG_LabelNewPolledMT(nt, AG_LABEL_SLOW | AG_LABEL_HFILL,
		    &OBJECT(tgt)->lock,
		    _("Parent Window: %[objName]"
		      " @ (" AGSI_CYAN AGSI_CODE "AG_Window" AGSI_RST " *)%p\n"
		      "Parent Driver: %[objName]"
		      " @ (" AGSI_CYAN AGSI_CODE "AG_Driver" AGSI_RST " *)%p\n"),
		    &tgt->window, &tgt->window,
		    &tgt->drv, &tgt->drv);

		boxWide = AG_BoxNewHoriz(nt, AG_BOX_EXPAND);
		boxVert = AG_BoxNewVert(boxWide, AG_BOX_HFILL);
		AG_SetFontSize(boxVert, "110%");
		AG_SetPadding(boxVert, "4");

		tb = AG_TextboxNewS(boxVert, AG_TEXTBOX_HFILL, _("Name: "));
#ifdef AG_UNICODE
		AG_TextboxBindUTF8(tb, OBJECT(tgt)->name, sizeof(OBJECT(tgt)->name));
#else
		AG_TextboxBindASCII(tb, OBJECT(tgt)->name, sizeof(OBJECT(tgt)->name));
#endif
		msb = AG_MSpinbuttonNew(boxVert, AG_MSPINBUTTON_HFILL,
		    "x", _("Size: "));
		AG_BindInt(msb, "xvalue", &tgt->w);
		AG_BindInt(msb, "yvalue", &tgt->h);

		msb = AG_MSpinbuttonNew(boxVert, AG_MSPINBUTTON_HFILL,
		    ",", _("Position: "));
		AG_BindInt(msb, "xvalue", &tgt->x);
		AG_BindInt(msb, "yvalue", &tgt->y);

		msb = AG_MSpinbuttonNew(boxVert, AG_MSPINBUTTON_HFILL,
		    ",", _("View UL: "));
		AG_BindInt(msb, "xvalue", &tgt->rView.x1);
		AG_BindInt(msb, "yvalue", &tgt->rView.y1);

		msb = AG_MSpinbuttonNew(boxVert, AG_MSPINBUTTON_HFILL,
		    ",", _("View LR: "));
		AG_BindInt(msb, "xvalue", &tgt->rView.x2);
		AG_BindInt(msb, "yvalue", &tgt->rView.y2);

		boxVert = AG_BoxNewVert(boxWide, AG_BOX_VFILL);
		AG_CheckboxSetFromFlagsFn(boxVert, 0, &tgt->flags, flagDescr,
		    FlagChanged,"%p",tgt);
	}

	if (OBJECT_CLASS(tgt)->edit != NULL) {
		AG_Widget *editRv;
		char label[64];

		if (AG_OfClass(tgt, "AG_Widget:AG_Scrollbar:*")) {
			Strlcpy(label,
			    AGSI_IDEOGRAM AGSI_HORIZ_SCROLLBAR AGSI_RST " ",
			    sizeof(label));
		} else if (AG_OfClass(tgt, "AG_Widget:AG_Box:*")) {
			Strlcpy(label,
			    AGSI_IDEOGRAM AGSI_BOX_VERT AGSI_RST " ",
			    sizeof(label));
		} else if (AG_OfClass(tgt, "AG_Widget:AG_Editable:*") ||
		           AG_OfClass(tgt, "AG_Widget:AG_Textbox:*") ||
		           AG_OfClass(tgt, "AG_Widget:AG_Label:*") ||
		           AG_OfClass(tgt, "AG_Widget:AG_Numerical:*")) {
			Strlcpy(label,
			    AGSI_IDEOGRAM AGSI_TEXTBOX AGSI_RST " ",
			    sizeof(label));
		} else {
			Strlcpy(label,
			    AGSI_IDEOGRAM AGSI_SMALL_SPHERE AGSI_RST " ",
			    sizeof(label));
		}
		Strlcat(label, OBJECT(tgt)->cls->name, sizeof(label));

		nt = AG_NotebookAdd(nb, label, AG_BOX_VERT);
		nt->id = 2;
		AG_SetPadding(nt, "3");

		editRv = OBJECT_CLASS(tgt)->edit(tgt);
		AG_ObjectAttach(nt, editRv);
		AG_WidgetCompileStyle(editRv);
		AG_Redraw(editRv);
	}

	nt = AG_NotebookAdd(nb,
	    _(AGSI_IDEOGRAM AGSI_MATH_X_EQUALS AGSI_RST " Variables"),
	    AG_BOX_VERT);
	nt->id = 3;
	{
		AG_Textbox *tb;

		tb = AG_TextboxNewS(nt, AG_TEXTBOX_HFILL |
		                        AG_TEXTBOX_RETURN_BUTTON, "+ ");
		AG_SetEvent(tb, "textbox-return",
		    InputVariable, "%p,%p", tb,tgt);

		AG_TlistNewPolledMs(nt, AG_TLIST_EXPAND, 333, PollVariables, NULL);
	}

	nt = AG_NotebookAdd(nb,
	    _(AGSI_IDEOGRAM AGSI_LOAD_IMAGE AGSI_RST " Surfaces"),
	    AG_BOX_VERT);
	nt->id = 4;
	{
		AG_Pane *pane;
		AG_Pixmap *px;
		AG_Tlist *tl;
		AG_MenuItem *mi;
		AG_Label *lblInfo;
		AG_Box *paneTop, *paneBottom;

		pane = AG_PaneNewVert(nt, AG_PANE_EXPAND);
		AG_PaneResizeAction(pane, AG_PANE_DIVIDE_EVEN); 
		paneTop = pane->div[0];
		paneBottom = pane->div[1];

		px = AG_PixmapNew(paneTop, AG_PIXMAP_EXPAND, 320, 240);
		lblInfo = AG_LabelNew(paneTop, AG_LABEL_HFILL, _("(No data)"));
		AG_LabelSizeHint(lblInfo, 5, "<XXXXXXXXXXXXXXXXXXXXXXXXX>");
		AG_SetFontSize(lblInfo, "90%");

		tl = AG_TlistNewPolled(paneBottom, AG_TLIST_EXPAND,
		    PollSurfaces,NULL);

		AG_SetEvent(tl, "tlist-selected",
		    SelectedSurface,"%p,%p",px,lblInfo);

		mi = AG_TlistSetPopup(tl, "surface");
		AG_MenuAction(mi, _("Export to image file..."), agIconSave.s,
		    ExportSurfaceDlg,"%Cp",px);

		AG_PaneMoveDividerPct(pane, 50);
	}

	AG_NotebookSelectByID(nb, savedTabID);		/* Restore active tab */
	AG_WidgetCompileStyle(box);
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
			    HideWindow,"%p",win);
		} else {
			AG_MenuAction(mi, _("Show window"), NULL,
			    ShowWindow,"%p",win);
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
	                    _("Target: " AGSI_PATH AGSI_YEL "%s" AGSI_RST), path);
		}
		return (win);
	}

	if ((win = agDebuggerWindow = AG_WindowNewNamedS(0, "_agDbgr")) == NULL)
		return (NULL);

	agDebuggerTgtWindow = tgt;

	AG_WindowSetCaption(win, _("Agar GUI Debugger: <%s> (\"%s\")"),
	                    OBJECT(tgt)->name, AGWINDOW(tgt)->caption);

	toolbar = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	AG_SetFontSize(toolbar, "150%");
	{
#if 0
		/* Set pick mode */
		btn = AG_ButtonNewFn(toolbar, AG_BUTTON_STICKY,
		    AGSI_ALGUE AGSI_NW_ARROW_TO_CORNER,
		    SetPickStatus, "%p,%p", win, tl);
		AG_SetPadding(btn, "0 5 3 5");
#endif
		/* Toggle VFS autorefresh */
		buRefresh = AG_ButtonNewS(toolbar,
		    AG_BUTTON_STICKY | AG_BUTTON_SET,
		    AGSI_BR_GRN AGSI_ALGUE AGSI_CCW_CLOSED_CIRCLE_ARROW);
		AG_SetPadding(buRefresh, "0 10 3 5");
	}

	pane = AG_PaneNewHoriz(win, AG_PANE_EXPAND);
	div = pane->div[0];
	agDebuggerBox = pane->div[1];

	AG_ObjectCopyName(tgt, path, sizeof(path));
	agDebuggerLabel = AG_LabelNew(div, AG_LABEL_HFILL,
	    _("Target: " AGSI_PATH AGSI_YEL "%s" AGSI_RST), path);

	lblStats = AG_LabelNewS(div, AG_LABEL_HFILL,
	    _("XX windows, XX containers & XX leaves (t = XXXXXms)"));
	AG_SetFontSize(lblStats, "80%");

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
