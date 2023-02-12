/*
 * Copyright (c) 2008-2023 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Font selection widget. It shows available types on the system (including
 * system fonts as well as Agar's core fonts). It allows the user to preview
 * a font in different styles, sizes and colors.
 */

#include <agar/core/core.h>
#include <agar/core/config.h>
#include <agar/gui/font_selector.h>
#include <agar/gui/icons.h>
#include <agar/gui/primitive.h>
#include <agar/gui/notebook.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/separator.h>

#include <string.h>

#include <agar/config/have_fontconfig.h>
#ifdef HAVE_FONTCONFIG
#include <fontconfig/fontconfig.h>
extern int agFontconfigInited;		/* text.c */
#endif

/*
 * Skip common fonts that do not usually include the ASCII range needed for
 * our Preview. TODO: Preview in different languages.
 */
const char *agFontsToIgnore[] = {
	"ClearlyU Alternate Glyphs",
	"ClearlyU PUA",
	"Cursor",
	"DejaVu Math TeX Gyre",
	"MUTT ClearlyU Alternate Glyphs Wide",
	"MUTT ClearlyU PUA",
	"Twitter Color Emoji",
	NULL
};

AG_FontSelector *
AG_FontSelectorNew(void *parent, Uint flags)
{
	AG_FontSelector *fs;

	fs = Malloc(sizeof(AG_FontSelector));
	AG_ObjectInit(fs, &agFontSelectorClass);

	if (flags & AG_FONTSELECTOR_HFILL) { WIDGET(fs)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_FONTSELECTOR_VFILL) { WIDGET(fs)->flags |= AG_WIDGET_VFILL; }
	fs->flags |= flags;
	
	AG_ObjectAttach(parent, fs);
	return (fs);
}

static void
Bound(AG_Event *_Nonnull event)
{
	AG_FontSelector *fs = AG_FONTSELECTOR_SELF();
	AG_Variable *b = AG_PTR(1);
	AG_Font **pFont;

	if (strcmp(b->name, "font") != 0)
		return;

	pFont = b->data.p;
	AG_SetPointer(fs, "font", *pFont);
	Strlcpy(fs->curFace, OBJECT(*pFont)->name, sizeof(fs->curFace));
	fs->curSize = (*pFont)->spec.size;
	fs->curStyle = (*pFont)->flags;
}

/* Load the requested font and update the "font" pointer. */
static AG_Font *_Nullable
LoadFont(AG_FontSelector *_Nonnull fs)
{
	AG_Variable *Vfont;
	AG_Font *font, **pFont;

	font = AG_FetchFont(fs->curFace, fs->curSize, fs->curStyle);
	if (font == NULL) {
		Verbose(_("Error opening font: %s\n"), AG_GetError());
		return (NULL);
	}
	Vfont = AG_GetVariable(fs, "font", (void *)&pFont);
	*pFont = font;
	AG_UnlockVariable(Vfont);
	return (font);
}

/* Update the weights, styles and width variants for a given font family. */
static void
UpdateStyles(AG_FontSelector *_Nonnull fs, AG_Font *_Nonnull font)
{
	AG_Tlist *tl = fs->tlStyles;
	AG_TlistItem *ti, *tiRegular = NULL;
	AG_TlistCompareFn cmpFnOrig;
	int i, selectionFound = 0;

	AG_TlistClear(tl);

	/* Find the style combinations available under this font family. */
	AG_FontGetFamilyStyles(font);

	for (i = 0; i < font->nFamilyStyles; i++) {
		char styleBuf[64];
		const Uint famStyle = font->familyStyles[i];
		const AG_FontStyleSort *fss;
	
		AG_FontGetStyleName(styleBuf, sizeof(styleBuf), famStyle);

		ti = AG_TlistAddS(tl, NULL, styleBuf);
		ti->depth = 1;
		ti->u = famStyle;

		/* Find the sort key for this style combination. */
		for (fss = &agFontStyleSort[0]; fss->key != -1; fss++) {
			if (famStyle == (Uint)fss->flags) {
				ti->v = (int)fss->key;          /* Sort key */
				Debug(font, "Sort key = %d (fl=0x%x)\n", ti->v,
				    fss->flags);
				break;
			}
		}

		if (font != NULL && famStyle == font->flags) {
			ti->selected++;
			selectionFound++;
		}
		if (famStyle == 0)
			tiRegular = ti;
	}

	/* Remove duplicate entries of the same style. */
	cmpFnOrig = AG_TlistSetCompareFn(tl, AG_TlistCompareStrings);
	AG_TlistUniq(tl);
	AG_TlistSetCompareFn(tl, cmpFnOrig);

	/* Sort according to the sort key (v). */
	AG_TlistSortByInt(tl);

	if (tiRegular != NULL) {
		if (!selectionFound)
			AG_TlistSelect(tl, tiRegular);
	}
}

/* Update the list of standard sizes for a given font family. */
static void
UpdateSizes(AG_FontSelector *_Nonnull fs, AG_Font *_Nullable font)
{
	const int stdSizes[] = { 4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
	                         22,24,26,28,32,48,64 };
	const int nStdSizes = sizeof(stdSizes) / sizeof(stdSizes[0]);
	int i;

	/* XXX TODO */
	for (i = 0; i < nStdSizes; i++) {
		AG_TlistItem *ti;

		ti = AG_TlistAdd(fs->tlSizes, NULL, "%d", stdSizes[i]);
		if (font != NULL && stdSizes[i] == font->spec.size)
			ti->selected++;
	}
}

/* Update the Preview surface and data displayed under "Metrics" tab. */
static void
UpdatePreview(AG_FontSelector *_Nonnull fs, AG_Font *_Nullable fontNew)
{
	const AG_FontAdjustment *fa;
	AG_Variable *Vfont;
	AG_Font **pFont, *font;
	float pts;
	int adjRange;

	if (fs->sPreview != -1) {
		AG_WidgetUnmapSurface(fs, fs->sPreview);
		fs->sPreview = -1;
	}

	if (fontNew != NULL) {
		font = fontNew;
	} else {
		Vfont = AG_GetVariable(fs, "font", (void *)&pFont);
		font = *pFont;
	}

	pts = font->spec.size;
	if      (pts <= 10.4f) { adjRange = 0; }
	else if (pts <= 14.0f) { adjRange = 1; }
	else if (pts <= 21.0f) { adjRange = 2; }
	else if (pts <= 23.8f) { adjRange = 3; }
	else if (pts <= 35.0f) { adjRange = 4; }
	else                   { adjRange = 5; }

	for (fa = &agFontAdjustments[0]; fa->face != NULL; fa++) {
		if (Strcasecmp(OBJECT(font)->name, fa->face) == 0)
			break;
	}
	if (fa->face != NULL) {
		AG_LabelText(fs->lblMetrics,
		    "Size: " AGSI_BOLD "%.01f" AGSI_RST " pts\n"
		    "Ascent: " AGSI_BOLD "%d" AGSI_RST " px\n"
		    "Descent: " AGSI_BOLD "%d" AGSI_RST " px\n"
		    "Line Skip: " AGSI_BOLD "%d" AGSI_RST " px\n\n"
		    "Adjustment: #%d\n"
		    "Scaling Adj: " AGSI_BOLD "%+.01f" AGSI_RST "\n"
		    "Ascent Adj: " AGSI_BOLD "%+d" AGSI_RST "\n",
		    pts, font->ascent, font->descent, font->lineskip, adjRange,
		    fa->size_factor, fa->ascent_offset[adjRange]);
	} else {
		AG_LabelText(fs->lblMetrics,
		    "Size: " AGSI_BOLD "%.01f" AGSI_RST " pts\n"
		    "Ascent: " AGSI_BOLD "%d" AGSI_RST " px\n"
		    "Descent: " AGSI_BOLD "%d" AGSI_RST " px\n"
		    "Line Skip: " AGSI_BOLD "%d" AGSI_RST " px\n",
		    pts, font->ascent, font->descent, font->lineskip);
	}

	if (fontNew == NULL)
		AG_UnlockVariable(Vfont);
}

static void
RenderPreview(AG_FontSelector *_Nonnull fs)
{
	AG_Variable *Vfont;
	AG_Font **pFont;
	AG_Surface *S;
	
	Vfont = AG_GetVariable(fs, "font", (void *)&pFont);

	AG_PushTextState();

	AG_TextColor(&fs->cPreviewFG);
	if (*pFont != NULL) {
		AG_TextFont(*pFont);
	}
	if (fs->flags & AG_FONTSELECTOR_ALT_PHRASE) {
		S = AG_TextRender("ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789\n"
		                  "abcdefghijklmnopqrstuvwxyz 0123456789");
	} else {
		S = AG_TextRender("The Quick Brown Fox Jumps Over The Lazy Dog");
	}
	if (fs->sPreview == -1) {
		fs->sPreview = AG_WidgetMapSurface(fs, S);
	} else {
		AG_WidgetReplaceSurface(fs, fs->sPreview, S);
	}

	AG_PopTextState();

	AG_UnlockVariable(Vfont);
}

static void
OnShow(AG_Event *_Nonnull event)
{
	AG_Variable *Vfont;
	AG_Font **pFont, *font;
	AG_FontSelector *fs = AG_FONTSELECTOR_SELF();
/*	AG_StaticFont **pbf; */
	AG_TlistItem *ti;
	AG_ConfigPath *fpath;
	AG_Tlist *tlFaces = fs->tlFaces;
	AG_TlistCompareFn cmpFnOrig;
	int i;
	
	Vfont = AG_GetVariable(fs, "font", (void *)&pFont);
	font = *pFont;

	AG_PushTextState();

	fs->flags &= ~(AG_FONTSELECTOR_UPDATE);

#if 0
	for (pbf = &agBuiltinFonts[0]; *pbf != NULL; pbf++) {
		if (strchr((*pbf)->name, '_'))              /* Is a variant */
			continue;

		ti = AG_TlistAdd(tlFaces, NULL, "%s", (*pbf)->name);
		ti->p1 = *pbf;

		if (font != NULL && strcmp(ti->text, OBJECT(font)->name) == 0)
			ti->selected++;
	}
#endif
	/*
	 * System fonts via fontconfig.
	 */
#ifdef HAVE_FONTCONFIG
	if (agFontconfigInited) {
		FcObjectSet *os;
		FcFontSet *fset;
		FcPattern *pat;
		
		pat = FcPatternCreate();
		os = FcObjectSetBuild(FC_FAMILY, (char *)0);
		fset = FcFontList(NULL, pat, os);
		if (fset != NULL) {
			for (i = 0; i < fset->nfont; i++) {
				FcPattern *fcfont = fset->fonts[i];
				const char **fignore;
				const char *fext;
				FcChar8 *pFam;
				char *fam;

				if (FcPatternGetString(fcfont, FC_FAMILY, 0,
				    &pFam) != FcResultMatch) {
					continue;
				}
				fam = (char *)pFam;
				fext = strrchr(fam, '.');
				if (fext && strcmp(fext, ".pcf") == 0) {
					continue;
				}
				for (fignore = &agFontsToIgnore[0];
				    *fignore != NULL;
				     fignore++) {
					if (strcmp(*fignore, fam) == 0)
						break;
				}
				if (*fignore != NULL)
					continue;

				ti = AG_TlistAddS(tlFaces, NULL, fam);

				if (font != NULL &&
				    strcmp(fam, OBJECT(font)->name) == 0)
					ti->selected++;
			}
			FcFontSetDestroy(fset);
		}
		FcObjectSetDestroy(os);
		FcPatternDestroy(pat);
	}
#endif /* HAVE_FONTCONFIG */

	/*
	 * Fonts present under CONFIG_PATH_FONTS.
	 */
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

			/*
			 * Style the item and show the file extension to
			 * make sure there is no confusion with respect
			 * to fontconfig-discovered fonts.
			 */
			/* *pExt = '\0'; */

			ti = AG_TlistAddS(tlFaces, NULL, file);
			AG_TlistSetFont(tlFaces, ti, "monoalgue", 1.0f, 0);

			if (font && strcmp(file, OBJECT(font)->name) == 0)
				ti->selected++;
		}
		AG_CloseDir(dir);
	}

	cmpFnOrig = AG_TlistSetCompareFn(tlFaces, AG_TlistCompareStrings);
	AG_TlistUniq(fs->tlFaces);
	AG_TlistSetCompareFn(tlFaces, cmpFnOrig);

	AG_TlistSort(fs->tlFaces);

	UpdateSizes(fs, font);
	UpdatePreview(fs, font);

	AG_UnlockVariable(Vfont);
}

static void
SelectedFace(AG_Event *_Nonnull event)
{
	AG_FontSelector *fs = AG_FONTSELECTOR_PTR(1);
	AG_TlistItem *it = AG_TLIST_ITEM_PTR(2);
	AG_Font *font;

	Strlcpy(fs->curFace, it->text, sizeof(fs->curFace));

	if ((font = LoadFont(fs)) != NULL) {
		UpdateStyles(fs, font);
		UpdatePreview(fs, font);
	}
}

static void
SelectedStyle(AG_Event *_Nonnull event)
{
	AG_FontSelector *fs = AG_FONTSELECTOR_PTR(1);
	const AG_TlistItem *it = AG_TLIST_ITEM_PTR(2);
	AG_Font *font;

	fs->curStyle = it->u;

	if ((font = LoadFont(fs)) != NULL)
		UpdatePreview(fs, font);
}

static void
SelectedSize(AG_Event *_Nonnull event)
{
	AG_FontSelector *fs = AG_FONTSELECTOR_PTR(1);
	const AG_TlistItem *it = AG_TLIST_ITEM_PTR(2);
	AG_Font *font;

	fs->curSize = (float)strtod(it->text, NULL);

	if ((font = LoadFont(fs)) != NULL)
		UpdatePreview(fs, font);
}

static void
MouseButtonDown(void *obj, AG_MouseButton button, int x, int y)
{
	AG_FontSelector *fs = obj;

	if (!AG_RectInside(&fs->rPreview, x,y))
		return;

	if (fs->flags & AG_FONTSELECTOR_ALT_PHRASE) {
		fs->flags &= ~(AG_FONTSELECTOR_ALT_PHRASE);
	} else {
		fs->flags |= AG_FONTSELECTOR_ALT_PHRASE;
	}
	if (fs->sPreview != -1) {
		AG_WidgetUnmapSurface(fs, fs->sPreview);
	}
	UpdatePreview(fs, NULL);
	AG_Redraw(fs);
}

static void
PreviewColorChanged(AG_Event *_Nonnull event)
{
	AG_FontSelector *fs = AG_FONTSELECTOR_PTR(1);

	UpdatePreview(fs, NULL);
}

static void
EditBgFgColor(AG_Event *_Nonnull event)
{
	AG_Button *btn = AG_BUTTON_SELF();
	AG_FontSelector *fs = AG_FONTSELECTOR_PTR(1);
	void *c = AG_PTR(2);
	AG_Button *btnOther = AG_BUTTON_PTR(3);

	if (AG_GetPointer(fs->pal,"agcolor") == c) {           /* No change */
		AG_ButtonToggle(btn);
		return;
	}
	AG_BindPointer(fs->pal, "agcolor", c);
	AG_ButtonToggle(btnOther);
}

static void
Init(void *_Nonnull obj)
{
	AG_FontSelector *fs = obj;
	
	fs->flags = AG_FONTSELECTOR_UPDATE;
	fs->curFace[0] = '\0';
	fs->curStyle = 0;
	fs->sPreview = -1;
	fs->curSize = 0.0f;

	fs->hPane = AG_PaneNewHoriz(fs, AG_PANE_EXPAND);
	{
		AG_Box *box;
		AG_Notebook *nb;
		AG_NotebookTab *nt;

		fs->tlFaces = AG_TlistNew(fs->hPane->div[0], AG_TLIST_EXPAND);
		fs->hPane2 = AG_PaneNewHoriz(fs->hPane->div[1], AG_PANE_EXPAND);
		fs->tlStyles = AG_TlistNew(fs->hPane2->div[0], AG_TLIST_EXPAND);
		fs->tlSizes = AG_TlistNew(fs->hPane2->div[1], AG_TLIST_EXPAND);

		nb = AG_NotebookNew(fs->hPane2->div[1], AG_NOTEBOOK_HFILL);
		nt = AG_NotebookAdd(nb, _("Color"), AG_BOX_VERT);
		AG_SetStyle(nt, "padding", "5");
		{
			AG_HSVPal *pal;
			AG_Button *btnBG, *btnFG;

			box = AG_BoxNewHoriz(nt, AG_BOX_HFILL | AG_BOX_HOMOGENOUS);
			AG_SetStyle(box, "font-size", "80%");

			btnBG = AG_ButtonNewS(box, AG_BUTTON_STICKY, _("BG"));
			btnFG = AG_ButtonNewS(box, AG_BUTTON_STICKY, _("FG"));
			AG_ButtonSetState(btnFG, 1);
			AG_SetEvent(btnBG, "button-pushed",
			    EditBgFgColor,"%p,%p,%p", fs, &fs->cPreviewBG, btnFG);
			AG_SetEvent(btnFG, "button-pushed",
			    EditBgFgColor,"%p,%p,%p", fs, &fs->cPreviewFG, btnBG);

			pal = fs->pal = AG_HSVPalNew(nt, AG_HSVPAL_NOPREVIEW |
			                                 AG_HSVPAL_NOALPHA |
		                                         AG_HSVPAL_HFILL);
			AG_BindPointer(pal, "agcolor", (void *)&fs->cPreviewFG);
			AG_SetEvent(pal, "h-changed", PreviewColorChanged,"%p",fs);
			AG_SetEvent(pal, "sv-changed", PreviewColorChanged,"%p",fs);
		}

		nt = AG_NotebookAdd(nb, _("Metrics"), AG_BOX_VERT);
		AG_SetStyle(nt, "padding", "5");
		{
			AG_CheckboxNewFlag(nt, 0, _("Baseline"),
			    &fs->flags, AG_FONTSELECTOR_BASELINE);
			AG_CheckboxNewFlag(nt, 0, _("Corrections"),
			    &fs->flags, AG_FONTSELECTOR_CORRECTIONS);
			AG_CheckboxNewFlag(nt, 0, _("Bounding box"),
			    &fs->flags, AG_FONTSELECTOR_BOUNDING_BOX);

			AG_SeparatorNewHoriz(nt);

			fs->lblMetrics = AG_LabelNewS(nt, AG_LABEL_EXPAND,
			    _("No data."));
		}
	}
	
	fs->font = NULL;
	fs->rPreview.x = 0;
	fs->rPreview.y = 0;
	fs->rPreview.w = 0;
	fs->rPreview.h = 80;
	AG_ColorNone(&fs->cPreviewBG);
	AG_ColorWhite(&fs->cPreviewFG);

	AG_TlistSizeHint(fs->tlFaces, "<New Century Schoolbook>", 15);
	AG_TlistSizeHint(fs->tlStyles, "<Condensed Bold Oblique>", 15);
	AG_TlistSizeHint(fs->tlSizes, "<XXXXXXX>", 10);
	
	/* Handle "font" binding programmatically */
	AG_BindPointer(fs, "font", (void *)&fs->font);
	AG_SetEvent(fs, "bound", Bound, NULL);
	OBJECT(fs)->flags |= AG_OBJECT_BOUND_EVENTS;

	AG_AddEvent(fs, "widget-shown", OnShow, NULL);

	AG_SetEvent(fs->tlFaces, "tlist-selected", SelectedFace, "%p", fs);
	AG_SetEvent(fs->tlStyles, "tlist-selected", SelectedStyle, "%p", fs);
	AG_SetEvent(fs->tlSizes, "tlist-selected", SelectedSize, "%p", fs);
}

static void
Draw(void *_Nonnull obj)
{
	AG_FontSelector *fs = obj;
	AG_Widget *chld;
	const AG_Surface *S;
	const AG_Color *cLine = &WCOLOR(fs,LINE_COLOR);
	int x,y;

	OBJECT_FOREACH_CHILD(chld, obj, ag_widget)
		AG_WidgetDraw(chld);

	AG_DrawBoxSunk(fs, &fs->rPreview, &WCOLOR(fs,BG_COLOR));

	AG_PushBlendingMode(fs, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);

	if (fs->sPreview == -1) {
		RenderPreview(fs);
	}
	S = WSURFACE(fs,fs->sPreview);

	if (fs->cPreviewBG.a > 0)
		AG_DrawRectFilled(fs, &fs->rPreview, &fs->cPreviewBG);

	AG_PushClipRect(fs, &fs->rPreview);

	x = fs->rPreview.x + (fs->rPreview.w >> 1) - (S->w >> 1);
	y = fs->rPreview.y + (fs->rPreview.h >> 1) - (S->h >> 1);

	AG_WidgetBlitSurface(fs, fs->sPreview, x,y);

	if (fs->flags & AG_FONTSELECTOR_BOUNDING_BOX) {
		AG_DrawLineH(fs, 1, WIDTH(fs)-2, y, cLine);
		AG_DrawLineH(fs, 1, WIDTH(fs)-2, y + S->h, cLine);
	}
	if (fs->flags & AG_FONTSELECTOR_CORRECTIONS) {
		const AG_FontAdjustment *fa;
		AG_Variable *Vfont;
		AG_Font **pFont;
		float pts;
		int adjRange;
	
		Vfont = AG_GetVariable(fs, "font", (void *)&pFont);
		pts = (*pFont)->spec.size;

		if      (pts <= 10.4f) { adjRange = 0; }
		else if (pts <= 14.0f) { adjRange = 1; }
		else if (pts <= 21.0f) { adjRange = 2; }
		else if (pts <= 23.8f) { adjRange = 3; }
		else if (pts <= 35.0f) { adjRange = 4; }
		else                   { adjRange = 5; }

		for (fa = &agFontAdjustments[0]; fa->face != NULL; fa++) {
			if (Strcasecmp(OBJECT(*pFont)->name, fa->face) == 0)
				break;
		}
		if (fa->face != NULL) {
			AG_Color cOrig, cCorrected;
			const int yCorrected = y + S->h - S->guides[0];
			const int adj = fa->ascent_offset[adjRange];

			AG_ColorRGB_8(&cOrig, 200,0,0);
			AG_ColorRGB_8(&cCorrected, 0,150,0);
			AG_DrawLineH(fs, 1, WIDTH(fs)-2, yCorrected - adj, &cOrig);
			AG_DrawLineH(fs, 1, WIDTH(fs)-2, yCorrected, &cCorrected);
		} else if (fs->flags & AG_FONTSELECTOR_BASELINE) {
			AG_DrawLineH(fs, 1, WIDTH(fs)-2, y + S->h - S->guides[0], cLine);
		}
	
		AG_UnlockVariable(Vfont);
	} else if (fs->flags & AG_FONTSELECTOR_BASELINE) {
		AG_DrawLineH(fs, 1, WIDTH(fs)-2, y + S->h - S->guides[0], cLine);
	}

	AG_PopClipRect(fs);
	
	AG_PopBlendingMode(fs);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_FontSelector *fs = obj;
	AG_SizeReq rChld;

	AG_WidgetSizeReq(fs->hPane, &rChld);

	r->w = WIDGET(fs)->paddingLeft + rChld.w +
	       WIDGET(fs)->paddingRight;

	r->h = WIDGET(fs)->paddingTop + rChld.h + fs->rPreview.h +
	       WIDGET(fs)->paddingBottom;
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_FontSelector *fs = obj;
	AG_SizeAlloc aChld;
	const int paddingLeft   = WIDGET(fs)->paddingLeft;
	const int paddingRight  = WIDGET(fs)->paddingRight;
	const int paddingTop    = WIDGET(fs)->paddingTop;
	const int paddingBottom = WIDGET(fs)->paddingBottom;

	/* Size horizontal pane */
	aChld.x = paddingLeft;
	aChld.y = paddingTop;
	aChld.w = a->w - paddingLeft - paddingRight;
	aChld.h = a->h - fs->rPreview.h - paddingTop - paddingBottom;
	AG_WidgetSizeAlloc(fs->hPane, &aChld);

	fs->rPreview.x = paddingLeft;
	fs->rPreview.y = a->h - fs->rPreview.h - paddingBottom;
	fs->rPreview.w = a->w - paddingLeft - paddingRight;
	return (0);
}

AG_WidgetClass agFontSelectorClass = {
	{
		"Agar(Widget:FontSelector)",
		sizeof(AG_FontSelector),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate,
	MouseButtonDown,
	NULL,			/* mouse_button_up */
	NULL,			/* mouse_motion */
	NULL,			/* key_down */
	NULL,			/* key_up */
	NULL,			/* touch */
	NULL,			/* ctrl */
	NULL			/* joy */
};
