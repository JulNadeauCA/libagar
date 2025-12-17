/*
 * Copyright (c) 2019-2025 Julien Nadeau Carriere <vedge@csoft.net>
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
#include <agar/gui/checkbox.h>
#include <agar/gui/separator.h>
#include <agar/gui/notebook.h>
#include <agar/gui/pane.h>
#include <agar/gui/scrollview.h>
#include <agar/gui/pixmap.h>
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

static AG_Window *_Nullable agStyleEditorWindow = NULL;   /* Style editor window instance */
static AG_Tlist  *_Nullable agStyleEditorTlist = NULL;    /* List of windows and widgets */
static AG_Box    *_Nullable agStyleEditorBox = NULL;      /* Target widget edit container */

static char agStyleEditorFilter[AG_TLIST_LABEL_MAX];  /* Style editor widget filter pattern */
static int agStyleEditorCapture = 0;                  /* Capture graphics surfaces */
static int agStyleEdListChldWindows = 0;              /* Include sub-windows in list */

static int
FindWidgets(AG_Widget *_Nonnull wid, AG_Tlist *_Nonnull tl, int depth)
{
	char text[AG_TLIST_LABEL_MAX];
	AG_TlistItem *it;
	AG_Widget *widChld;
	int matchingChld = 0;

	Strlcpy(text, OBJECT(wid)->name, sizeof(text));
	it = AG_TlistAddPtr(tl, NULL, text, wid);
	it->depth = depth;
	it->cat = "widget";
	it->flags |= AG_TLIST_ITEM_EXPANDED;
	
	if (!TAILQ_EMPTY(&OBJECT(wid)->children)) {
		it->flags |= AG_TLIST_HAS_CHILDREN;
	}
	if (AG_TlistVisibleChildren(tl, it) || agStyleEditorFilter[0] != '\0') {
		OBJECT_FOREACH_CHILD(widChld, wid, ag_widget) {
			if (FindWidgets(widChld, tl, depth+1) == 1)
				matchingChld++;
		}
	}

	if (agStyleEditorFilter[0] != '\0' &&
	    AG_Strcasestr(OBJECT(wid)->name, agStyleEditorFilter) == NULL) {
		if (!matchingChld) {
			AG_TlistDel(tl, it);
		}
		return (0);
	}
	return (1);
}

static void
FindWindows(AG_Tlist *_Nonnull tl, const AG_Window *_Nonnull win, int depth)
{
	const char *name = OBJECT(win)->name;
	AG_Window *wSub;
	AG_Widget *wChild;
	AG_TlistItem *it;

	switch (win->wmType) {
	case AG_WINDOW_WM_DROPDOWN_MENU:
	case AG_WINDOW_WM_POPUP_MENU:
	case AG_WINDOW_WM_TOOLTIP:
	case AG_WINDOW_WM_COMBO:
		return;
	default:
		break;
	}

	if (strncmp(name, "win", 3) == 0 && isdigit((int)name[4])) {
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
	if (AG_TlistVisibleChildren(tl, it)) {
		if (agStyleEdListChldWindows)
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

	if (tgt != NULL && AG_OBJECT_VALID(tgt) && AG_WIDGET_ISA(tgt)) {
		FindWindows(tl, tgt, 0);
	} else {
		TargetRoot();
		AG_LockVFS(&agDrivers);
		AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
			AG_FOREACH_WINDOW(tgt, drv) {
				if ((tgt->wmType == AG_WINDOW_WM_DROPDOWN_MENU) ||
				    (tgt->wmType == AG_WINDOW_WM_POPUP_MENU) ||
				    (tgt->wmType == AG_WINDOW_WM_TOOLTIP) ||
				    (tgt->wmType == AG_WINDOW_WM_COMBO)) {
					continue;
				}
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

static void TargetWidget(AG_Event *_Nonnull);

static void
InputAttribute(AG_Event *_Nonnull event)
{
	AG_Textbox *tb = AG_TEXTBOX_PTR(1);
	AG_Widget *tgt = AG_WIDGET_PTR(2);
	AG_Tlist *tlVars = AG_TLIST_PTR(3);
	char *s = AG_TextboxDupString(tb), *ps = s;
	const char *key = Strsep(&ps, ":=");
	const char *val = Strsep(&ps, ":=");
	int i;

	if (tgt == NULL || !AG_OBJECT_VALID(tgt) || !AG_WIDGET_ISA(tgt))
		return;
	if (key == NULL || val == NULL)
		return;

	while (isspace((int)*key)) { key++; }
	while (isspace((int)*val)) { val++; }

	AG_SetStyle(tgt, key, (val[0] != '\0') ? val : NULL);

	for (i = 0; i < AG_WIDGET_NCOLORS; i++) {
		if (strcmp(agStyleAttributes[i], key) == 0)
			break;
	}
	if (i < AG_WIDGET_NCOLORS) {
		for (i = 0; i < AG_WIDGET_NSTATES; i++) {
			char keyState[AG_VARIABLE_NAME_MAX];

			Strlcpy(keyState, key, sizeof(keyState));
			Strlcat(keyState, agWidgetStateNames[i], sizeof(keyState));
			if (!AG_Defined(tgt, keyState)) {
				AG_SetStyle(tgt, keyState, (val[0] != '\0') ? val : NULL);
			}
		}
	}

	AG_WindowUpdate(AG_ParentWindow(tgt));
/*	AG_TextboxClearString(tb); */

	tlVars->flags |= AG_TLIST_REFRESH;
	AG_Redraw(tlVars);

	free(s);
}

static void
PollStyleVariables(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_Widget *tgt = AG_PTR(1);
	AG_Variable *V;

	if (!AG_OBJECT_VALID(tgt) || !AG_WIDGET_ISA(tgt))
		return;

	AG_TlistBegin(tl);

	TAILQ_FOREACH(V, &OBJECT(tgt)->vars, vars) {
		AG_Surface *S;
		char *val;
		AG_TlistItem *it;

		if (V->type != AG_VARIABLE_STRING)
			continue;

		val = AG_GetStringP(tgt, V->name);
		if (strcmp(V->name, "color") == 0 ||
		    strstr(V->name, "-color") != NULL) {
			AG_Color c;

			S = AG_SurfaceStdRGB(tl->icon_w, tl->icon_w);
			AG_ColorFromString(&c, val, NULL);
			AG_FillRect(S, NULL, &c);
		} else {
			S = NULL;
		}
		it = AG_TlistAdd(tl, S, "%s: %s", V->name, val);
		it->p1 = val;
		if (S)
			AG_SurfaceFree(S);
	}

	AG_TlistEnd(tl);
}

static void
PollStyleBlock(AG_Tlist *_Nonnull tl, AG_StyleBlock *_Nonnull blk)
{
	AG_StyleEntry *ent;

	TAILQ_FOREACH(ent, &blk->ents, ents) {
		AG_Surface *S;
		AG_TlistItem *it;

		if (strcmp(ent->key, "color") == 0 ||
		    strstr(ent->key, "-color") != NULL) {
			AG_Color c;

			S = AG_SurfaceStdRGB(tl->icon_w, tl->icon_w);
			AG_ColorFromString(&c, ent->value, NULL);
			AG_FillRect(S, NULL, &c);
		} else {
			S = NULL;
		}
		it = AG_TlistAdd(tl, S, "%s: %s", ent->key, ent->value);
		it->p1 = ent->value;
		it->depth = 1;
		if (S)
			AG_SurfaceFree(S);
	}
}

static __inline__ void
PrintCondition(char *_Nonnull dst, size_t dstLen, const AG_StyleBlock *_Nonnull blk)
{
	switch (blk->cond) {
	case AG_SELECTOR_COND_WIDTH:
		if (blk->y == (AG_INT_MAX - 1)) {
			Snprintf(dst, dstLen, "width %d - " AGSI_INFINITY, blk->x);
			break;
		} else {
			Snprintf(dst, dstLen, "width %d - %d", blk->x, blk->y);
			break;
		}
		break;
	case AG_SELECTOR_COND_HEIGHT:
		if (blk->y == (AG_INT_MAX - 1)) {
			Snprintf(dst, dstLen, "height %d - " AGSI_INFINITY, blk->x);
			break;
		} else {
			Snprintf(dst, dstLen, "height %d - %d", blk->x, blk->y);
			break;
		}
		break;
	case AG_SELECTOR_COND_ZOOM:
		if (blk->y == (AG_INT_MAX - 1)) {
			Snprintf(dst, dstLen, "zoom %d - " AGSI_INFINITY, blk->x);
			break;
		} else {
			Snprintf(dst, dstLen, "zoom %d - %d", blk->x, blk->y);
			break;
		}
		break;
	default:
		Strlcpy(dst, "?", dstLen);
		break;
	}
}

static void
PollStyleComputedRules(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_Widget *tgt = AG_PTR(1);
	const AG_Object *tgtParent;
	AG_StyleSheet *css;
	AG_StyleBlock *blk;
	const char *tgtClass;
	AG_TlistItem *it;
	char cond[32];

	if (!AG_OBJECT_VALID(tgt) || !AG_WIDGET_ISA(tgt))
		return;

	tgtParent = OBJECT(tgt)->parent;
	tgtClass = OBJECT(tgt)->cls->name;
	css = (tgt->css != NULL) ? tgt->css : &agDefaultCSS;

	AG_TlistBegin(tl);

	/* Level 4: `E > F' blocks with a condition. */
	if (tgtParent != NULL) {
		TAILQ_FOREACH(blk, &css->blksCond, blks) {
			if (blk->selector == AG_SELECTOR_CHILD_NAMED) {
				if (strcmp(blk->f, OBJECT(tgt)->name) == 0 &&
				    strcmp(blk->e, AGOBJECT_CLASS(tgtParent)->name) == 0 &&
				    AG_WidgetStyleConditionTest(blk, tgt)) {
					break;
				}
			} else if (blk->selector == AG_SELECTOR_CHILD_OF_CLASS) {
				if (strcmp(blk->f, AGOBJECT_CLASS(tgt)->name) == 0 &&
				    strcmp(blk->e, AGOBJECT_CLASS(tgtParent)->name) == 0 &&
				    AG_WidgetStyleConditionTest(blk, tgt)) {
					break;
				}
			}
		}
 		if (blk != NULL) {
			PrintCondition(cond, sizeof(cond), blk);
			if (blk->selector == AG_SELECTOR_CHILD_NAMED) {
				it = AG_TlistAdd(tl, NULL, "%s > \"%s\" (%s) {", blk->e, blk->f, cond);
			} else {
				it = AG_TlistAdd(tl, NULL, "%s > %s (%s) {", blk->e, blk->f, cond);
			}
			it->depth = 0;
			it->flags |= (AG_TLIST_HAS_CHILDREN | AG_TLIST_NO_SELECT);
		
			PollStyleBlock(tl, blk);

			it = AG_TlistAddS(tl, NULL, "}");
			it->flags |= AG_TLIST_NO_SELECT;
		}
	}

	/* Level 3: `E' blocks with a condition. */
	TAILQ_FOREACH(blk, &css->blksCond, blks) {
		if (blk->selector == AG_SELECTOR_CLASS_NAME) {
			if (strcmp(blk->e, tgtClass) == 0 &&
			    AG_WidgetStyleConditionTest(blk, tgt)) {
				break;
			}
		} else if (blk->selector == AG_SELECTOR_CLASS_PATTERN) {
			if (AG_OfClass(tgt, blk->e) &&
			    AG_WidgetStyleConditionTest(blk, tgt)) {
				break;
			}
		}
	}
 	if (blk != NULL) {
		PrintCondition(cond, sizeof(cond), blk);
		it = AG_TlistAdd(tl, NULL, "%s (%s) {", blk->e, cond);
		it->depth = 0;
		it->flags |= (AG_TLIST_HAS_CHILDREN | AG_TLIST_NO_SELECT);
		
		PollStyleBlock(tl, blk);

		it = AG_TlistAddS(tl, NULL, "}");
		it->flags |= AG_TLIST_NO_SELECT;
	}

	/* Level 2: `E > F' blocks with no condition. */
	if (tgtParent != NULL) {
		TAILQ_FOREACH(blk, &css->blks, blks) {
			if (blk->selector == AG_SELECTOR_CHILD_NAMED) {
				if (strcmp(blk->f, OBJECT(tgt)->name) == 0 &&
				    strcmp(blk->e, AGOBJECT_CLASS(tgtParent)->name) == 0)
					break;
			} else if (blk->selector == AG_SELECTOR_CHILD_OF_CLASS) {
				if (strcmp(blk->f, AGOBJECT_CLASS(tgt)->name) == 0 &&
				    strcmp(blk->e, AGOBJECT_CLASS(tgtParent)->name) == 0)
					break;
			}
		}
 		if (blk != NULL) {
			if (blk->selector == AG_SELECTOR_CHILD_NAMED) {
				it = AG_TlistAdd(tl, NULL, "%s > \"%s\" {", blk->e, blk->f);
			} else {
				it = AG_TlistAdd(tl, NULL, "%s > %s {", blk->e, blk->f);
			}
			it->depth = 0;
			it->flags |= (AG_TLIST_HAS_CHILDREN | AG_TLIST_NO_SELECT);
		
			PollStyleBlock(tl, blk);
		
			it = AG_TlistAddS(tl, NULL, "}");
			it->flags |= AG_TLIST_NO_SELECT;
		}
	}

	/* Level 1: `E' blocks with no condition (lowest precedence). */
	TAILQ_FOREACH(blk, &css->blks, blks) {
		if (blk->selector == AG_SELECTOR_CLASS_NAME) {
			if (strcmp(blk->e, tgtClass) == 0)
				break;
		} else if (blk->selector == AG_SELECTOR_CLASS_PATTERN) {
			if (AG_OfClass(tgt, blk->e))
				break;
		}
	}
 	if (blk != NULL) {
		it = AG_TlistAdd(tl, NULL, "%s {", blk->e);
		it->depth = 0;
		it->flags |= (AG_TLIST_HAS_CHILDREN | AG_TLIST_NO_SELECT);

		PollStyleBlock(tl, blk);

		it = AG_TlistAddS(tl, NULL, "}");
		it->flags |= AG_TLIST_NO_SELECT;
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
		"thin", "extralight", "light", "regular", "semibold", "bold",
		"extrabold", "black", "!parent", NULL
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
		"normal", "italic", "oblique", "!parent", NULL
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
		"ultracondensed", "condensed", "semicondensed",
		"normal", "semiexpanded", "expanded", "ultraexpanded",
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
CompleteTextAlign(const char *_Nonnull key, const char *_Nonnull val, AG_Tlist *_Nonnull tl)
{
	const char *values[] = {
		"center", "start", "end", "justify", NULL
	}, **vp;

	for (vp = values; *vp != NULL; vp++)
		if (val[0] == '\0' || val[0] == '*' ||
		    Strncasecmp(*vp, val, strlen(val)) == 0)
			AG_TlistAdd(tl, NULL, "%s: %s", key, *vp);
}

static void
CompleteVerticalAlign(const char *_Nonnull key, const char *_Nonnull val, AG_Tlist *_Nonnull tl)
{
	const char *values[] = {
		"middle", "top", "bottom",  NULL
	}, **vp;

	for (vp = values; *vp != NULL; vp++)
		if (val[0] == '\0' || val[0] == '*' ||
		    Strncasecmp(*vp, val, strlen(val)) == 0)
			AG_TlistAdd(tl, NULL, "%s: %s", key, *vp);
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
		{ "text-align",       CompleteTextAlign },
		{ "vertical-align",   CompleteVerticalAlign },
		{ NULL,               NULL }
	}, *dp;
	AG_Editable *ed = AG_EDITABLE_SELF();
	AG_Tlist *tl = AG_TLIST_PTR(1);
	char *s, *sp;
	const char *key, *val, **attr;

	sp = s = AG_EditableDupString(ed);
	while (*sp == ' ' || *sp == '\t') {  /* Skip any leading whitespace */
		sp++;
	}
	key = AG_Strsep(&sp, ":=");
	val = AG_Strsep(&sp, ":=");

	AG_TlistBegin(tl);

	if (key == NULL || key[0] == '*' || key[0] == ' ') {    /* All keys */
		for (attr = agStyleAttributes; *attr != NULL; attr++)
			AG_TlistAdd(tl, NULL, "%s: ", *attr);
	} else {
		if (val == NULL) {                           /* Partial key */
			for (attr = agStyleAttributes; *attr != NULL; attr++) {
				if (Strncasecmp(*attr, key, strlen(key)) != 0) {
					continue;
				}
				AG_TlistAdd(tl, NULL, "%s: ", *attr);
			}
		} else if (val[0] == '*' ||                   /* All values */
		          (val[0] == ' ' && val[1] == '\0')) {
			for (dp = &dict[0]; dp->key != NULL; dp++) {
				if (Strcasecmp(key, dp->key) == 0) {
					dp->fn(key, "", tl);
					break;
				}
			}
		} else {                     /* Partial (or complete) value */
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
	const AG_TlistItem *it = AG_TLISTITEM_PTR(2);

	AG_TextboxSetString(tb, it->text);
}

static void
ClearVarListPointer(AG_Event *_Nonnull event)
{
	agStyleEditorVarList = NULL;
}

static void
ClearRuleListPointer(AG_Event *_Nonnull event)
{
	agStyleEditorRuleList = NULL;
}

static void
TargetWidget(AG_Event *_Nonnull event)
{
	AG_Box *box = agStyleEditorBox;
	AG_TlistItem *ti = AG_TLISTITEM_PTR(1);
	AG_Widget *tgt = ti->p1;
	AG_Notebook *nb;
	AG_NotebookTab *nt;
	int savedTabID;

	if (tgt == NULL || !AG_OBJECT_VALID(tgt) || !AG_WIDGET_ISA(tgt))
		return;

	agStyleEditorTgt = tgt;

	if ((nb = AG_ObjectFindChild(box, "notebook0")) != NULL) {
		savedTabID = nb->selTabID;
	} else {
		savedTabID = -1;
	}

	AG_ObjectFreeChildren(box);
	
	nb = AG_NotebookNew(box, AG_NOTEBOOK_EXPAND);
	nt = AG_NotebookAdd(nb, _("Attributes"), AG_BOX_VERT);
	AG_SetFontFamily(nt, "monoalgue");
	AG_SetFontSize(nt, "90%");
	{
		AG_Textbox *tb;
		AG_Pane *vPane;
		AG_Tlist *tl;

		tb = AG_TextboxNewS(nt, AG_TEXTBOX_HFILL |
		                        AG_TEXTBOX_RETURN_BUTTON, "+ ");
		AG_SetMargin(tb, "0 0 10 0");

		AG_TextboxAutocomplete(tb, CompleteAttribute, NULL);

		vPane = AG_PaneNewVert(nt, AG_PANE_EXPAND);

		agStyleEditorVarList = tl = AG_TlistNewPolledMs(vPane->div[0], AG_TLIST_EXPAND, 500,
		    PollStyleVariables, "%p", tgt);
		AG_SetEvent(tl, "tlist-selected", SelectedAttribute, "%p", tb);
		AG_AddEvent(tl, "detached", ClearVarListPointer, NULL);

		agStyleEditorRuleList = tl = AG_TlistNewPolledMs(vPane->div[1], AG_TLIST_EXPAND, 0,
		    PollStyleComputedRules, "%p", tgt);
		AG_SetEvent(tl, "tlist-selected", SelectedAttribute, "%p", tb);
		AG_AddEvent(tl, "detached", ClearRuleListPointer, NULL);

		AG_SetEvent(tb, "textbox-return", InputAttribute, "%p,%p,%p", tb, tgt, agStyleEditorVarList);

		AG_PaneMoveDividerPct(vPane, 50);
		AG_PaneResizeAction(vPane, AG_PANE_DIVIDE_EVEN);
	}

	nt = AG_NotebookAdd(nb, _("Capture"), AG_BOX_VERT);
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
			AG_SetFontSize(lbl, "70%");

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
			AG_LabelNew(nt, 0, _("* Video Capture failed (%s)"), AG_GetError());
		}
	} else {
		AG_LabelNewS(nt, AG_LABEL_HFILL,
		    _("Video Capture is currently disabled.\n"
		      "Use "
		      "["
		      AGSI_IDEOGRAM AGSI_CHECKBOX AGSI_RST
		      AGSI_ITALIC "Enable Video Capture" AGSI_RST
		      " ] "
		      "below to enable."));
	}
	
	AG_NotebookSelectByID(nb, savedTabID);		/* Restore active tab */
	AG_WidgetCompileStyle(box);
	AG_WidgetShowAll(box);
	AG_WidgetUpdate(box);
}

static void
MenuForWindow(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_PTR(1);
	AG_MenuItem *mi = AG_MENUITEM_PTR(2);
	AG_TlistItem *ti = AG_TlistSelectedItem(tl);
	AG_Window *win = ti->p1;

	if (ti == NULL)
		return;

	if (win == NULL || !AG_OBJECT_VALID(win) || !AG_WINDOW_ISA(win))
		return;

	if (win->visible) {
		AG_MenuAction(mi, _("Hide window"), NULL,
		    HideWindow, "%p", win);
	} else {
		AG_MenuAction(mi, _("Show window"), NULL,
		    ShowWindow, "%p", win);
	}
}

static void
PausePolling(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_PTR(1);
	const int state = AG_INT(2);

	AG_TlistSetRefresh(tl, state ? -1 : 250);
}

static void
CloseStyleEditorWindow(AG_Event *_Nonnull event)
{
	agStyleEditorFilter[0] = '\0';
	agStyleEditorWindow = NULL;
	agStyleEditorTlist = NULL;
	agStyleEditorBox = NULL;
	agStyleEditorTgtWindow = NULL;
}

static void
FilterChanged(AG_Event *event)
{
	AG_Textbox *tb = AG_TEXTBOX_SELF();
	AG_Tlist *tlWidgets = AG_TLIST_PTR(1);
	char s[4];

	AG_TextboxCopyString(tb, s, sizeof(s));

	if (s[0] != '\0') {
		tlWidgets->flags |= AG_TLIST_NO_LINES;
	} else {
		tlWidgets->flags &= ~(AG_TLIST_NO_LINES);
	}
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
	AG_Box *box;
	AG_Textbox *tb;
	AG_Tlist *tl;
	AG_Label *lbl;

	if (tgt == NULL) {
		AG_TextError(_("No window is focused.\n"
		               "Focus on a window to target it in Style Editor."));
		return (NULL);
	}
	if (tgt == agStyleEditorWindow)                           /* Unsafe */
		return (NULL);

	agStyleEditorFilter[0] = '\0';

	if ((win = agStyleEditorWindow) != NULL) {
		AG_WindowFocus(win);
		if (agStyleEditorTgtWindow != tgt) {
			agStyleEditorTgtWindow = tgt;
			AG_ObjectFreeChildren(agStyleEditorBox);
		}
		return (win);
	}

	if ((win = agStyleEditorWindow = AG_WindowNewNamedS(0, "_agStyleEditor")) == NULL)
		return (NULL);

	agStyleEditorTgtWindow = tgt;

	AG_WindowSetCaption(win,
	    _("Agar Style Editor: <%s> (\"%s\")"),
	    AGOBJECT(tgt)->name, AGWINDOW(tgt)->caption);

	lbl = AG_LabelNewS(win, AG_LABEL_HFILL,
	    _(AGSI_IDEOGRAM AGSI_AGAR_AG AGSI_RST " Style Editor "
	      AGSI_IDEOGRAM AGSI_AGAR_AR));
	AG_SetFontFamily(lbl, "charter");
	AG_SetFontSize(lbl, "150%");
	AG_LabelJustify(lbl, AG_TEXT_CENTER);

	tl = agStyleEditorTlist = AG_TlistNewPolledMs(NULL,
	    AG_TLIST_EXPAND, 80,
	    PollWidgets, NULL);

	AG_TlistSizeHint(tl, "<XXXXXXXXXXXXXXXX>", 25);
	AG_SetFontSize(tl, "90%");
	AG_SetEvent(tl, "tlist-selected", TargetWidget, NULL);

	pane = AG_PaneNewHoriz(win, AG_PANE_EXPAND);
	agStyleEditorBox = pane->div[1];
	{
		tb = AG_TextboxNewS(pane->div[0],  AG_TEXTBOX_HFILL, NULL);
		AG_TextboxSizeHint(tb, "<XXXXXXXXXXXXXXXXXX>");
		AG_TextboxBindUTF8(tb, agStyleEditorFilter,
		    sizeof(agStyleEditorFilter));
		AG_TextboxSetPlaceholderS(tb, _("Filter widgets..."));
		AG_SetEvent(tb, "textbox-postchg", FilterChanged, "%p", tl);
		AG_WidgetFocus(tb);

		AG_ObjectAttach(pane->div[0], tl);

		AG_LabelNewS(pane->div[1], 0, _("No widget selected."));

		mi = AG_TlistSetPopup(tl, "window");
		AG_MenuSetPollFn(mi, MenuForWindow, "%p", tl);
	}

	box = AG_BoxNewHoriz(win, AG_BOX_HFILL);
	AG_SetFontSize(box, "80%");
	AG_SetSpacing(box, "20 0");
	{
		AG_CheckboxNewInt(box, 0, _("Include Child Windows"),
		    &agStyleEdListChldWindows);

		AG_CheckboxNewFn(box, 0, _("Pause Polling"),
		    PausePolling,"%p",tl);

		AG_CheckboxNewInt(box, 0, _("Enable Video Capture"),
		    &agStyleEditorCapture);
	}

	AG_AddEvent(win, "window-close", CloseStyleEditorWindow, NULL);

	AG_WindowSetGeometryAligned(win, AG_WINDOW_MR, 900, 500);
	AG_WindowSetPosition(win, AG_WINDOW_BR, 1);
	AG_WindowSetCloseAction(win, AG_WINDOW_DETACH);
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
