/*
 * Copyright (c) 2019-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * This tool allows the user to inspect a live VFS of Agar widgets
 * and to define or edit style attributes in real time.
 */

#include <agar/core/core.h>
#include <agar/core/config.h>

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

/* For "font-family" autocomplete. */
#include <agar/config/have_fontconfig.h>
#ifdef HAVE_FONTCONFIG
#include <fontconfig/fontconfig.h>
extern int agFontconfigInited;		/* text.c */
#endif

static AG_Window *_Nullable agStyleEditorWindow = NULL;
static AG_Tlist  *_Nullable agStyleEditorTlist = NULL;
static AG_Box    *_Nullable agStyleEditorBox = NULL;

static int agStyleEditorCapture = 0;

static void
FindWidgets(AG_Widget *_Nonnull wid, AG_Tlist *_Nonnull tl, int depth)
{
	char text[AG_TLIST_LABEL_MAX];
	AG_TlistItem *it;
	AG_Widget *widChld;

	Strlcpy(text, OBJECT(wid)->name, sizeof(text));
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
TargetRoot(void)
{
	agStyleEditorTgt = NULL;
	if (agStyleEditorWindow) {
		if (agStyleEditorTlist)
			AG_TlistDeselectAll(agStyleEditorTlist);
	}
}

static void
PollWidgets(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	const AG_Window *tgt = agStyleEditorTgtWindow;
	AG_Driver *drv;
	
	AG_TlistBegin(tl);

	if (tgt != NULL && AG_OBJECT_VALID(tgt)) {
		FindWindows(tl, tgt, 0);
	} else {
		TargetRoot();
		AG_LockVFS(&agDrivers);
		AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
			AG_FOREACH_WINDOW(tgt, drv) {
				if (tgt == agStyleEditorWindow) {
					continue;
				}
				FindWindows(tl, tgt, 0);
			}
		}
		AG_UnlockVFS(&agDrivers);
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

#if 0
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
#endif

static void TargetWidget(AG_Event *_Nonnull);

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
/*	AG_TextboxClearString(tb); */

	free(s);
}

static void
PollAttributes(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_Widget *tgt = AG_WIDGET_PTR(1);
	const char **attr;

	if (!AG_OBJECT_VALID(tgt))
		return;

	AG_TlistBegin(tl);

	for (attr = &agStyleAttributes[0]; *attr != NULL; attr++) {
		AG_Surface *S;
		char *value;
		AG_TlistItem *it;

		if (!AG_Defined(tgt, *attr)) {
			continue;
		}
		value = AG_GetStringP(tgt, *attr);
		if (strcmp(*attr, "color") == 0 ||
		    strstr(*attr, "-color") != NULL) {
			AG_Color c;

			S = AG_SurfaceStdRGB(tl->icon_w, tl->icon_w);
			AG_ColorFromString(&c, value, NULL);
			AG_FillRect(S, NULL, &c);
		} else {
			S = NULL;
		}
		it = AG_TlistAdd(tl, S, "%s: %s", *attr, value);
		it->p1 = value;
		if (S)
			AG_SurfaceFree(S);
	}

	AG_TlistEnd(tl);
}

static void
CompleteColor(const char *_Nonnull key, const char *_Nonnull val,
    AG_Tlist *_Nonnull tl)
{
	const AG_ColorName *cn;
	const int icon_w = tl->icon_w;

	for (cn = &agColorNames[0]; cn->name != NULL; cn++) {
		if (val[0] == '\0' || val[0] == '*' ||
		    Strncasecmp(cn->name, val, strlen(val)) == 0) {
			AG_Surface *S;

			S = AG_SurfaceStdRGB(icon_w, icon_w);
			AG_FillRect(S, NULL, &cn->c);
			AG_TlistAdd(tl, S, "%s: %s", key, cn->name);
			AG_SurfaceFree(S);
		}
	}
}

static void
CompleteFontWeight(const char *_Nonnull key, const char *_Nonnull val,
    AG_Tlist *_Nonnull tl)
{
	const char *values[] = {
		"normal",
		"bold",
		"!parent",
		NULL
	}, **vp;

	for (vp = values; *vp != NULL; vp++)
		if (val[0] == '\0' || val[0] == '*' ||
		    Strncasecmp(*vp, val, strlen(val)) == 0)
			AG_TlistAdd(tl, NULL, "%s: %s", key, *vp);
}

static void
CompleteFontStyle(const char *_Nonnull key, const char *_Nonnull val,
    AG_Tlist *_Nonnull tl)
{
	const char *values[] = {
		"normal",
		"italic",
		"upright-italic",
		"!parent",
		NULL
	}, **vp;

	for (vp = values; *vp != NULL; vp++)
		if (val[0] == '\0' || val[0] == '*' ||
		    Strncasecmp(*vp, val, strlen(val)) == 0)
			AG_TlistAdd(tl, NULL, "%s: %s", key, *vp);
}

static void
CompleteFontStretch(const char *_Nonnull key, const char *_Nonnull val,
    AG_Tlist *_Nonnull tl)
{
	const char *values[] = {
		"normal",
		"condensed",
		"semi-condensed",
		"!parent",
		NULL
	}, **vp;

	for (vp = values; *vp != NULL; vp++)
		if (val[0] == '\0' || val[0] == '*' ||
		    Strncasecmp(*vp, val, strlen(val)) == 0)
			AG_TlistAdd(tl, NULL, "%s: %s", key, *vp);
}

static void
CompleteFontFamily(const char *_Nonnull key, const char *_Nonnull val,
    AG_Tlist *_Nonnull tl)
{
	AG_StaticFont **pbf;
	AG_ConfigPath *fpath;
	AG_TlistItem *it;

	/*
	 * Agar core fonts.
	 */
	for (pbf = &agBuiltinFonts[0]; *pbf != NULL; pbf++) {
		if (strchr((*pbf)->name, '_')) {            /* Is a variant */
			continue;
		}
		if (val[0] == '\0' || val[0] == '*' ||
		    Strncasecmp((*pbf)->name, val, strlen(val)) == 0) {
			it = AG_TlistAdd(tl, NULL, "%s: %s", key, (*pbf)->name);
			it->p1 = (void *)(*pbf)->name;
		}
	}
	TAILQ_FOREACH(fpath, &agConfig->paths[AG_CONFIG_PATH_FONTS], paths) {
		AG_Dir *dir;
		int i;

		if ((dir = AG_OpenDir(fpath->s)) == NULL) {
			continue;
		}
		for (i = 0; i < dir->nents; i++) {
			char path[AG_FILENAME_MAX];
			AG_FileInfo info;
			char *file = dir->ents[i], *pExt;
			const char **ffe;

			if (file[0] == '.' ||
			    (pExt = strrchr(file, '.')) == NULL) {
				continue;
			}
			for (ffe = &agFontFileExts[0]; *ffe != NULL;
			     ffe++) {
				if (Strcasecmp(pExt, *ffe) == 0)
					break;
			}
			if (*ffe == NULL)
				continue;

			Strlcpy(path, fpath->s, sizeof(path));
			Strlcat(path, AG_PATHSEP, sizeof(path));
			Strlcat(path, file, sizeof(path));

			if (AG_GetFileInfo(path, &info) == -1 ||
			    info.type != AG_FILE_REGULAR) {
				continue;
			}
			*pExt = '\0';

			if (val[0] == '\0' || val[0] == '*' ||
			    Strncasecmp(file, val, strlen(val)) == 0) {
				it = AG_TlistAdd(tl, NULL, "%s: %s", key, file);
				it->p1 = file;
			}
		}
		AG_CloseDir(dir);
	}

	/*
	 * System fonts.
	 */
#ifdef HAVE_FONTCONFIG
	if (agFontconfigInited) {
		FcObjectSet *os;
		FcFontSet *fset;
		FcPattern *pat;
		int i;
		
		pat = FcPatternCreate();
		os = FcObjectSetBuild(FC_FAMILY, (char *)0);
		fset = FcFontList(NULL, pat, os);
		if (fset != NULL) {
			for (i = 0; i < fset->nfont; i++) {
				FcPattern *font = fset->fonts[i];
				FcChar8 *fam;

				if (FcPatternGetString(font, FC_FAMILY, 0, &fam)
				    != FcResultMatch) {
					continue;
				}
				if (val[0] == '\0' || val[0] == '*' ||
				    Strncasecmp((char *)fam, val, strlen(val)) == 0) {
					it = AG_TlistAdd(tl, NULL, "%s: %s",
					    key, (char *)fam);
					it->p1 = (void *)fam;
				}
			}
			FcFontSetDestroy(fset);
		}
		FcObjectSetDestroy(os);
		FcPatternDestroy(pat);
	}
#endif /* HAVE_FONTCONFIG */
}

static void
CompleteAttribute(AG_Event *_Nonnull event)
{
	static const struct {
		const char *_Nonnull key;
		void (*_Nonnull fn)(const char *_Nonnull, const char *_Nonnull,
		                    AG_Tlist *_Nonnull);
	} dict[] = {
		{ "color",            CompleteColor },
		{ "background-color", CompleteColor },
		{ "text-color",       CompleteColor },
		{ "line-color",       CompleteColor },
		{ "high-color",       CompleteColor },
		{ "low-color",        CompleteColor },
		{ "selection-color",  CompleteColor },
		{ "font-weight",      CompleteFontWeight },
		{ "font-style",       CompleteFontStyle },
		{ "font-stretch",     CompleteFontStretch },
		{ "font-family",      CompleteFontFamily },
#if 0
		{ "font-size",        CompleteFontSize },
#endif
		{ NULL,               NULL }
	}, *dp;
	AG_Editable *ed = AG_EDITABLE_SELF();
	AG_Tlist *tl = AG_TLIST_PTR(1);
	char *s = AG_EditableDupString(ed), *sp = s;
	const char *key, *val, **attr;

	while (*sp == ' ' || *sp == '\t') {        /* Skip leading whitespace */
		sp++;
	}
	key = AG_Strsep(&sp, ":");
	val = AG_Strsep(&sp, ":");

	AG_TlistBegin(tl);

	if (key == NULL || key[0] == '*' || key[0] == ' ') {      /* All keys */
		for (attr = agStyleAttributes; *attr != NULL; attr++)
			AG_TlistAdd(tl, NULL, "%s: ", *attr);
	} else {
		if (val == NULL) {                             /* Partial key */
			for (attr = agStyleAttributes; *attr != NULL; attr++) {
				if (Strncasecmp(*attr, key, strlen(key)) != 0) {
					continue;
				}
				AG_TlistAdd(tl, NULL, "%s: ", *attr);
			}
		} else if (val[0] == '*' ||                     /* All values */
		          (val[0] == ' ' && val[1] == '\0')) {
			for (dp = &dict[0]; dp->key != NULL; dp++) {
				if (Strcasecmp(key, dp->key) == 0) {
					dp->fn(key, "", tl);
					break;
				}
			}
		} else {                       /* Partial (or complete) value */
			while (*val == ' ' || *val == '\t') {
				val++;
			}
			for (dp = &dict[0]; dp->key != NULL; dp++) {
				if (Strncasecmp(key, dp->key, strlen(key)) != 0) {
					continue;
				}
				dp->fn(key, val, tl);
				break;
			}
		}
	}

	AG_TlistEnd(tl);

	if (tl->nItems == 0) {
		AG_EditableCloseAutocomplete(ed);
	} else if (tl->nItems == 1) {
		char *sOrig = AG_EditableDupString(ed);

		if (AG_TlistFindText(tl, sOrig)) {
			AG_EditableCloseAutocomplete(ed);
		}
		Free(sOrig);
	} else {
		AG_TlistSort(tl);
	}
	Free(s);
}

static void
SelectedAttribute(AG_Event *_Nonnull event)
{
	AG_Textbox *tb = AG_TEXTBOX_PTR(1);
	const AG_TlistItem *it = AG_TLIST_ITEM_PTR(2);

	AG_TextboxSetString(tb, it->text);
}

static void
TargetWidget(AG_Event *_Nonnull event)
{
	AG_Box *box = agStyleEditorBox;
	AG_TlistItem *ti = AG_TLIST_ITEM_PTR(1);
	AG_Widget *tgt = ti->p1;
	AG_Notebook *nb;
	AG_NotebookTab *nt;
	AG_Tlist *tlAttrs;
	int savedTabID;

	AG_OBJECT_ISA(tgt, "AG_Widget:*");

	agStyleEditorTgt = tgt;

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

		tb = AG_TextboxNewS(nt, AG_TEXTBOX_HFILL |
		                        AG_TEXTBOX_RETURN_BUTTON, "+ ");
		AG_TextboxAutocomplete(tb, CompleteAttribute, NULL);
		AG_SetEvent(tb, "textbox-return",
		    InputAttribute, "%p,%p", tb,tgt);

		tlAttrs = AG_TlistNewPolledMs(nt, AG_TLIST_EXPAND, 333,
		    PollAttributes, "%p", tgt);

		AG_SetEvent(tlAttrs, "tlist-selected",
		    SelectedAttribute, "%p", tb);
	}
#if 0
	nt = AG_NotebookAdd(nb, _("Variables"), AG_BOX_VERT);
	{
		AG_Tlist *tl;

		tl = AG_TlistNewPolledMs(nt, AG_TLIST_EXPAND, 333,
		                         PollVariables,"%p",tgt);
		AG_SetStyle(tl, "font-family", "courier-prime");
		AG_SetStyle(tl, "font-size", "120%");
	}
#endif
	nt = AG_NotebookAdd(nb, _("Appearance"), AG_BOX_VERT);
	if (agStyleEditorCapture) {
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

			AG_PixmapFromSurface(sv, AG_PIXMAP_EXPAND, Sx);
			AG_SurfaceFree(Sx);
#if 0
			AG_SeparatorNewHoriz(nt);
			AG_ButtonNewFn(nt, 0, _("Export image..."),
			    ExportImageDlg, "%p", S); /* TODO */
#endif
		} else {
			AG_LabelNew(nt, 0, _("* Capture failed (%s)"), AG_GetError());
		}
	} else {
		AG_LabelNewS(nt, AG_LABEL_HFILL,
		    _("* Capture is disabled. Click on \xe2\x96\xa6 (and refresh) to enable."));
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

#if 0
static void
SetPickStatus(AG_Event *_Nonnull event)
{
#ifdef AG_DEBUG
	AG_Window *winSted = AG_WINDOW_PTR(1);
/*	AG_Tlist *tl = AG_TLIST_PTR(2); */
	const int enable = AG_INT(3);

	if (enable)
		Debug(winSted, "PickStatus(%d)\n", enable);
#endif
}
#endif

static void
SetAutorefresh(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_PTR(1);
	const int enable = AG_INT(2);

	AG_TlistSetRefresh(tl, enable ? 250 : -1);
}

static void
CloseStyleEditorWindow(AG_Event *_Nonnull event)
{
	agStyleEditorWindow = NULL;
	agStyleEditorTlist = NULL;
	agStyleEditorBox = NULL;
	agStyleEditorTgtWindow = NULL;
}

/*
 * Open the Style Editor window (with tgt at the root).
 */
AG_Window *_Nullable
AG_StyleEditor(AG_Window *_Nonnull tgt)
{
	AG_Window *win;
	AG_Pane *pane;
	AG_MenuItem *mi;
	AG_Box *toolbar;
	AG_Tlist *tl;

	if (tgt == NULL) {
		AG_TextError(_("No window is focused.\n"
		               "Focus on a window to target it in Style Editor."));
		return (NULL);
	}
	if (tgt == agStyleEditorWindow) {			/* Unsafe */
		return (NULL);
	}
	if ((win = agStyleEditorWindow) != NULL) {
		AG_WindowFocus(win);
		if (agStyleEditorTgtWindow != tgt) {
			agStyleEditorTgtWindow = tgt;
			AG_ObjectFreeChildren(agStyleEditorBox);
		}
		return (win);
	}

	if ((win = agStyleEditorWindow = AG_WindowNewNamedS(0, "_agSted")) == NULL)
		return (NULL);

	agStyleEditorTgtWindow = tgt;

	AG_WindowSetCaption(win, _("Agar Style Editor: <%s> (\"%s\")"),
	    AGOBJECT(tgt)->name, AGWINDOW(tgt)->caption);
	
	tl = agStyleEditorTlist = AG_TlistNewPolledMs(NULL, AG_TLIST_EXPAND, 80,
	    PollWidgets, NULL);

	AG_TlistSizeHint(tl, "<XXXXX/XXXXX/XXXXX/XXXXX>", 25);
	AG_SetStyle(tl, "font-size", "80%");
	AG_SetEvent(tl, "tlist-selected", TargetWidget, NULL);

	toolbar = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	AG_SetStyle(toolbar, "font-size", "150%");
	{
		AG_Button *btn;
#if 0
		/* Set pick mode */
		btn = AG_ButtonNewFn(toolbar, AG_BUTTON_STICKY,
		    "\xe2\x87\xb1",                             /* U+21F1 */
		    SetPickStatus, "%p,%p", win, tl);
		AG_SetStyle(btn, "padding", "0 5 3 5");
#endif
		/* Toggle VFS autorefresh */
		btn = AG_ButtonNewFn(toolbar, AG_BUTTON_STICKY | AG_BUTTON_SET,
		    "\xe2\xa5\x81",                             /* U+2941 */
		    SetAutorefresh, "%p", tl);
		AG_SetStyle(btn, "padding", "0 10 3 5");

		/* Toggle appearance capture */
		btn = AG_ButtonNewInt(toolbar, AG_BUTTON_STICKY,
		    "\xe2\x96\xa6",                             /* U+2941 */
		    &agStyleEditorCapture);
		AG_SetStyle(btn, "padding", "0 7 0 5");
	}

	pane = AG_PaneNewHoriz(win, AG_PANE_EXPAND);
	agStyleEditorBox = pane->div[1];
	{
		AG_ObjectAttach(pane->div[0], tl);

		AG_LabelNewS(pane->div[1], AG_LABEL_HFILL,
		    _("Select a widget on the left "
		     "(or use \xe2\x87\xb1 to pick)."));

		mi = AG_TlistSetPopup(tl, "window");
		AG_MenuSetPollFn(mi, MenuForWindow, "%p", tl);
	}

	AG_AddEvent(win, "window-close", CloseStyleEditorWindow, NULL);
	
	AG_WindowSetPosition(win, AG_WINDOW_BR, 1);
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);
	AG_WidgetFocus(tl);
	return (win);
}

void
AG_StyleEditorDetachTarget(void)
{
	if (agStyleEditorBox) {
		AG_ObjectFreeChildren(agStyleEditorBox);
	}
	agStyleEditorTgt = NULL;
}

void
AG_StyleEditorDetachWindow(void)
{
	if (agStyleEditorBox) {
		AG_ObjectFreeChildren(agStyleEditorBox);
	}
	agStyleEditorTgtWindow = NULL;
	TargetRoot();
}

#endif /* AG_WIDGETS */
