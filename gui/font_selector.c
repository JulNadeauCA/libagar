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
	AG_Tlist *tl = fs->tlSizes;
	const int stdSizes[] = { 4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
	                         22,24,26,28,32,48,64 };
	const int nStdSizes = sizeof(stdSizes) / sizeof(stdSizes[0]);
	int i;

	AG_TlistClear(tl);

	/* XXX TODO */
	for (i = 0; i < nStdSizes; i++) {
		AG_TlistItem *ti;

		ti = AG_TlistAdd(tl, NULL, "%d", stdSizes[i]);
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
		    "Scaling Adj: " AGSI_BOLD "%.01f" AGSI_RST "\n"
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

	S = fs->previewFn(fs, *pFont);

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
	int selFound = 0;
	
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
			int i;

			for (i = 0; i < fset->nfont; i++) {
				FcPattern *fcfont = fset->fonts[i];
				const char *fext;
				FcChar8 *pFam;
				char *fam;

				if (FcPatternGetString(fcfont, FC_FAMILY, 0,
				    &pFam) != FcResultMatch) {
					continue;
				}
				fam = (char *)pFam;
				fext = strrchr(fam, '.');

				/*
				 * Skip .pcf files and fonts which usually
				 * contains no Unicode-addressable glyphs.
				 */
				if ((fext && strcmp(fext, ".pcf") == 0) ||
				    strcmp(fam, "Cursor") == 0 ||
				    strcmp(fam, "DejaVu Math TeX Gyre") == 0)
					continue;

				ti = AG_TlistAddS(tlFaces, NULL, fam);

				if (font != NULL &&
				    strcmp(fam, OBJECT(font)->name) == 0) {
					ti->selected++;
					selFound = 1;
				}
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

			if (font && strcmp(file, OBJECT(font)->name) == 0) {
				ti->selected++;
				selFound = 1;
			}
		}
		AG_CloseDir(dir);
	}

	if (!selFound) {
		if (strcmp(agDefaultFont->name, "_agFontAlgue") == 0) {
			AG_TlistSelectText(tlFaces, "algue.ttf");
		} else {
			AG_TlistSelectText(tlFaces, agDefaultFont->name);
		}
		AG_TlistScrollToSelection(tlFaces);
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

/*
 * Default Preview function (previewFn).
 */
static AG_Surface *
PreviewDefault(AG_FontSelector *fs, AG_Font *font)
{
	const int altPhrase = (fs->flags & AG_FONTSELECTOR_ALT_PHRASE);
	AG_Surface *S;

	if (AG_Strcasestr(font->name, "Arabic")) {
		if (altPhrase) {
			/*
			 * Al-arabiyyah (Arabic).
			 */
			S = AG_TextRenderRTL(
			    "\xD8\xA7" "\xd9\x8E" "\xd9\x84" "\xd9\x92"
			    "\xD8\xB9" "\xd9\x8E" "\xd8\xB1" "\xd9\x8E"
			    "\xD8\xA8" "\xd9\x90" "\xd9\x8A" "\xd9\x8E"
			    "\xD9\x91" "\xd8\xA9" "\xd9\x8F");
		} else {
			/*
			 * As-aalaam alaikum ("Peace be upon you")
			 */
			S = AG_TextRenderRTL(
			    "\xD8\xA7" "\xD9\x84" "\xD8\xB3" "\xD9\x84"
			    "\xD8\xA7" " "
			    "\xD9\x85" "\xD8\xB9" "\xD9\x84" "\xD9\x8A"
			    "\xD9\x83" "\xD9\x85");
		}
	} else if (AG_Strcasestr(font->name, "Armenian")) {
		if (altPhrase) {
			/*
			 * Kpareik' indz het?
			 * ("Would you like to dance with me?")
			 */
			S = AG_TextRender(
			    "\xD4\xBF" "\xd5\xBA" "\xd5\xA1" "\xD6\x80"
			    "\xD5\xA5" "\xD5\xAB" "\xd5\x9E" "\xd6\x84" " "
			    "\xD5\xAB" "\xd5\xB6" "\xd5\xB1" " "
			    "\xD5\xB0" "\xd5\xA5" "\xd5\xBF" AGSI_ALGUE " ?");
		} else {
			/*
			 * Bari galu'st! ("Welcome!")
			 */
			S = AG_TextRender(
			    "\xD4\xB2" "\xD5\xA1" "\xD6\x80" "\xD5\xAB" " "
			    "\xD5\xA3" "\xD5\xA1" "\xD5\xAC" "\xD5\xB8"
			    "\xD6\x82" "\xD5\xBD" "\xD5\xBF" AGSI_ALGUE " !");
		}
	} else if (AG_Strcasestr(font->name, "CJK SC") ||
	           AG_Strcasestr(font->name, "Sans SC") ||
	           AG_Strcasestr(font->name, "Serif SC")) {
		if (altPhrase) {
			/*
			 * Zhongwen (Chinese)
			 */
			S = AG_TextRender("\xE4\xB8" "\xAD\xE6" "\x96\x87");
		} else {
			/*
			 * Youqing yinshuibao, wuqing shifanji
			 * ("With love water is enough; without love, food
			 * doesn't satisfy.")
			 */
			S = AG_TextRender(
			    "\xE6\x9C\x89" "\xE6\x83\x85" "\xE9\xA5\xAE"
			    "\xE6\xB0\xB4" "\xE9\xA5\xB1" "\xEf\xBC\x8C"
			    "\xE6\x97\xA0" "\xE6\x83\x85" "\xE9\xA3\x9F"
			    "\xE9\xA5\xAD" "\xE9\xA5\xA5" "\xE3\x80\x82");
		}
	} else if (AG_Strcasestr(font->name, "Estrangelo") ||
	           AG_Strcasestr(font->name, "East Syriac")) {
		if (altPhrase) {
			/*
			 * Lessana Suryaya (Syriac)
			 */
			S = AG_TextRenderRTL(
			    "\xDC\xA0" "\xDC\xAB" "\xDC\xA2" "\xDC\x90" " "
			    "\xDC\xA3" "\xDC\x98" "\xDC\xAA" "\xDC\x9D"
			    "\xDC\x9D" "\xDC\x90");
		} else {
			/*
			 * Tubayhon l-aylen da-dken b-lebbhon d-hennon nehzon l-alaha 
			 * ("Blessed are the pure in heart for they shall see God").
			 */
			S = AG_TextRenderRTL(
			    "\xDC\x9B" "\xDC\x98" "\xDC\xBC" "\xDC\x92" "\xDC\xB2"
			    "\xDC\x9D" "\xDC\x97" "\xDC\x98" "\xDC\xBF" "\xDC\xA2" " "
			    "\xDC\xA0" "\xDC\x90" "\xDC\xB2" "\xDC\x9D"
			    "\xDC\xA0" "\xDC\xB9" "\xDC\x9D" "\xDC\xA2" " "
			    "\xDC\x95" "\xDC\xB2" "\xDC\x95" "\xDD\x82"
			    "\xDC\x9F" "\xDC\xB9" "\xDC\x9D" "\xDC\xA2" " "
			    "\xDC\x92" "\xDC\xA0" "\xDC\xB8" "\xDC\x92"
			    "\xCC\x87" "\xDC\x97" "\xDC\x98" "\xDC\xBF"
			    "\xDC\xA2" "\xDC\x84" " "
			    "\xDC\x95" "\xDC\x97" "\xDC\xB8" "\xDC\xA2" "\xDD\x82"
			    "\xDC\x98" "\xDC\xBF" "\xDC\xA2" " "
			    "\xDC\xA2" "\xDC\xB8" "\xDC\x9A" "\xDC\x99" "\xDC\x98"
			    "\xDC\xBF" "\xDC\xA2" " "
			    "\xDC\xA0" "\xDC\x90" "\xDC\xB2" "\xDC\xA0" "\xDC\xB5"
			    "\xDC\x97" "\xDC\xB5" "\xDC\x90" "\xDC\x82");
		}

	} else if (AG_Strcasestr(font->name, "Ethiopic")) {
		if (altPhrase) {
			/*
			 * Amarenna (Amharic)
			 */
			S = AG_TextRender(
			    "\xE1\x8A\xA0" "\xE1\x88\x9B" "\xE1\x88\xAD"
			    "\xE1\x8A\x9B");
		} else {
			/*
			 * Siletewaweqin dess bilognal
			 * ("Pleased to meet you")
			 */
			S = AG_TextRender(
			    "\xE1\x88\xB5" "\xE1\x88\x88" "\xE1\x89\xB0"
			    "\xE1\x8B\x8B" "\xE1\x8B\x88" "\xE1\x89\x85"
			    "\xE1\x8A\x95" " "
			    "\xE1\x8B\xB0" "\xE1\x88\xB5" " "
			    "\xE1\x89\xA5" "\xE1\x88\x8E" "\xE1\x8A\x9B"
			    "\xE1\x88\x8D");
		}

	} else if (AG_Strcasestr(font->name, "Georgian")) {
		if (altPhrase) {
			/*
			 * Kartuli ena (Georgian)
			 */
			S = AG_TextRender(
			    "\xE1\x83\xA5" "\xE1\x83\x90" "\xE1\x83\xA0"
			    "\xE1\x83\x97" "\xE1\x83\xA3" "\xE1\x83\x9A"
			    "\xE1\x83\x98" " "
			    "\xE1\x83\x94" "\xE1\x83\x9C" "\xE1\x83\x90");
		} else {
			/*
			 * ketil mgzavrobas gisurvebta!
			 * ("Have a good journey!")
			 */
			S = AG_TextRender(
			    "\xE1\x83\x99" "\xE1\x83\x94" "\xE1\x83\x97"
			    "\xE1\x83\x98" "\xE1\x83\x9A" " "
			    "\xE1\x83\x9B" "\xE1\x83\x92" "\xE1\x83\x96"
			    "\xE1\x83\x90" "\xE1\x83\x95" "\xE1\x83\xA0"
			    "\xE1\x83\x9D" "\xE1\x83\x91" "\xE1\x83\x90"
			    "\xE1\x83\xA1" " "
			    "\xE1\x83\x92" "\xE1\x83\x98" "\xE1\x83\xA1"
			    "\xE1\x83\xA3" "\xE1\x83\xA0" "\xE1\x83\x95"
			    "\xE1\x83\x94" "\xE1\x83\x91"
			    "\xE1\x83\x97" AGSI_ALGUE " !");
		}
	} else if (AG_Strcasestr(font->name, "Hebrew")) {
		if (altPhrase) {
			/*
			 * Ivrit (Hebrew)
			 */
			S = AG_TextRenderRTL(
			    "\xD7\xA2" "\xD6\xB4" "\xD7\x91" "\xd6\xB0"
			    "\xD7\xA8" "\xD6\xB4" "\xD7\x99" "\xD7\xAA");
		} else {
			/*
			 * Hachaim shelanu tutim
			 * ("Our life is strawberries").
			 */
			S = AG_TextRenderRTL(
			    "\xD7\x94" "\xD7\x97" "\xD7\x99" "\xD7\x99"
			    "\xD7\x9D" " "
			    "\xD7\xA9" "\xD7\x9C" "\xD7\xA0" "\xD7\x95" " "
			    "\xD7\xAA" "\xD7\x95" "\xD7\xAA" "\xD7\x99"
			    "\xD7\x9D");
		}
	} else if (AG_Strcasestr(font->name, "Japanese")) {
		if (altPhrase) {
			/*
			 * Nihongo (Japanese)
			 */
			S = AG_TextRender(
			    "\xE6\x97\xA5" "\xE6\x9C\xAC" "\xE8\xAA\x9E");
		} else {
			/*
			 * Ohayo gozaimasu ("Good morning")
			 */
			S = AG_TextRender(
			    "\xE3\x81\x8A" "\xE3\x81\xAF" "\xE3\x82\x88"
			    "\xE3\x81\x86" "\xE3\x81\x94" "\xE3\x81\x96"
			    "\xE3\x81\x84" "\xE3\x81\xBE" "\xE3\x81\x99"
			    "\xE3\x80\x82");
		}

	} else if (AG_Strcasestr(font->name, "MUTT ClearlyU Alternate Glyphs Wide")) {

		S = AG_TextRender("\xC5\xA2"      "\xC4\xA3"     "\xC4\xBD"
		   "\xC4\xBE"     "\xC5\x9E"      "\xC5\x9F"     "\xC5\xA2"
		   "\xC5\xA3"     "\xC5\xA5"      "\xCF\x9E"     "\xCF\xA1"
		   "\xCF\xA2"     "\xCF\xA3"      "\xD9\xAB"     "\xDB\x81"
		   "\xDB\x82"     "\xDB\x83"      "\xDB\xB4"     "\xDB\xB7"
		   "\xE0\xA4\x96" "\xE1\x82\xA0"  "\xE1\x82\xA1" "\xE1\x82\xA2");

	} else if (AG_Strcasestr(font->name, "MUTT ClearlyU PUA")) {

		S = AG_TextRender( "\xEE\x84\xAE" "\xEE\x84\xAF" "\xEE\x87\xB0"
		    "\xEE\x88\xB4" "\xEE\x89\x9F" "\xEE\xB7\xAD" "\xEE\xB7\xAE"
		    "\xEF\x83\x86" "\xEF\x83\x89" "\xEF\x83\xB7" "\xEF\x83\xB8"
		    "\xEF\x83\xB9" "\xEF\xA3\x90" "\xEF\xA3\x91" "\xEF\xA3\x92"
		    "\xEF\xA3\x93" "\xEF\xA3\x94" "\xEF\xA3\x95" "\xEF\xA3\x96"
		    "\xEF\xA3\x97" "\xEF\xA3\x98" "\xEF\xA3\x99" "\xEF\xA3\x9A"
		    "\xEF\xA3\x9B");

	} else if (Strcasecmp(font->name, "Noto Sans Linear B") == 0) {

		if (altPhrase) {
			/* Linear B Ideograms */
			S = AG_TextRender(
			    "\xF0\x90\x83\xA1" "\xF0\x90\x83\xA2" "\xF0\x90\x83\xA3" 
			    "\xF0\x90\x83\xA4" "\xF0\x90\x83\xA5" "\xF0\x90\x83\xA6" 
			    "\xF0\x90\x83\xA7" "\xF0\x90\x83\xA8" "\xF0\x90\x83\xA9" 
			    "\xF0\x90\x83\xAA" "\xF0\x90\x83\xAB" "\xF0\x90\x83\xAC" 
			    "\xF0\x90\x83\xAD" "\xF0\x90\x83\xAE" "\xF0\x90\x83\xAF"
			    "\n"
			    "\xF0\x90\x83\xB0" "\xF0\x90\x83\xB1" "\xF0\x90\x83\xB2"
			    "\xF0\x90\x83\xB3" "\xF0\x90\x83\xB4" "\xF0\x90\x83\xB5"
			    "\xF0\x90\x83\xB6" "\xF0\x90\x83\xB7" "\xF0\x90\x83\xB8"
			    "\xF0\x90\x83\xBF" "\xF0\x90\x83\xC0" "\xF0\x90\x83\xC1");
		} else {
			/* Linear B Syllables */
			S = AG_TextRender(
			    "\xF0\x90\x80\x80" "\xF0\x90\x80\x81" "\xF0\x90\x80\x82"
			    "\xF0\x90\x80\x83" "\xF0\x90\x80\x84" "\xF0\x90\x80\x85"
			    "\xF0\x90\x80\x86" "\xF0\x90\x80\x87" "\xF0\x90\x80\x88"
			    "\xF0\x90\x80\x8A" "\xF0\x90\x80\x8B" "\xF0\x90\x80\x8D"
			    "\xF0\x90\x80\x8E" "\xF0\x90\x80\x8F" "\xF0\x90\x80\x90"
			    "\xF0\x90\x80\x91" "\xF0\x90\x80\x92" "\n"
			    "\xF0\x90\x80\x93" "\xF0\x90\x80\x94" "\xF0\x90\x80\x95" 
			    "\xF0\x90\x80\x96" "\xF0\x90\x80\x97" "\xF0\x90\x80\x98" 
			    "\xF0\x90\x80\x9F" "\xF0\x90\x80\xA0" "\xF0\x90\x80\xA1"
			    "\xF0\x90\x80\xA2" "\xF0\x90\x80\xA3" "\xF0\x90\x80\xA4"
			    "\xF0\x90\x80\xA5" "\xF0\x90\x80\xA6" "\xF0\x90\x80\xA7"
			    "\xF0\x90\x80\xA8" "\xF0\x90\x80\xA9" "\xF0\x90\x80\xAA"
			    );
		}


	} else if (Strcasecmp(font->name, "Twitter Color Emoji") == 0) {

		S = AG_TextRender(
		    "\xF0\x9F\x98\x80" "\xF0\x9F\x98\x81" "\xF0\x9F\x98\x82"
		    "\xF0\x9F\x98\x83" "\xF0\x9F\x98\x84" "\xF0\x9F\x98\x85"
		    "\xF0\x9F\x98\x86" "\xF0\x9F\x98\x87" "\xF0\x9F\x98\x88"
		    "\xF0\x9F\x98\x89" "\xF0\x9F\x98\x8A" "\xF0\x9F\x98\x8B"
		    "\xF0\x9F\x98\x8C" "\xF0\x9F\x98\x8D" "\xF0\x9F\x98\x8E");
	
	} else if (strcmp(font->name, "agar-ideograms.agbf") == 0) {

		/* 34 wide */
		S = AG_TextRender(
		    AGSI_SPKR_W_3_SOUND_WAVES AGSI_BEZIER AGSI_BUTTON
		    AGSI_CHARSETS AGSI_CHECKBOX AGSI_WINDOW_GRADIENT
		    AGSI_CONSOLE AGSI_CUSTOM_WIDGET AGSI_FIXED_LAYOUT
		    AGSI_WIDGET_FOCUS AGSI_TYPOGRAPHY AGSI_FILESYSTEM
		    AGSI_WIREFRAME_CUBE AGSI_LOAD_IMAGE AGSI_SAVE_IMAGE
		    AGSI_KEYBOARD_KEY AGSI_MATH_X_EQUALS
		    AGSI_H_MAXIMIZE AGSI_V_MAXIMIZE AGSI_MEDIUM_WINDOW
		    AGSI_SMALL_WINDOW AGSI_SMALL_SPHERE AGSI_LARGE_SPHERE
		    AGSI_ARTISTS_PALETTE AGSI_WINDOW_PANE AGSI_SINE_WAVE
		    AGSI_RADIO_BUTTON AGSI_RENDER_TO_SURFACE
		    AGSI_HORIZ_SCROLLBAR AGSI_VERT_SCROLLBAR AGSI_SCROLLVIEW
		    AGSI_SWORD AGSI_NUL_TERMINATION AGSI_TABLE 
		    "\n"
		    AGSI_TEXTBOX AGSI_PROGRESS_BAR AGSI_CANNED_DIALOG
		    AGSI_THREADS AGSI_EMPTY_HOURGLASS AGSI_UNIT_CONVERSION
		    AGSI_USER_ACCESS AGSI_POPULATED_WINDOW AGSI_TWO_WINDOWS
		    AGSI_MENUBOOL_TRUE AGSI_MENUBOOL_FALSE AGSI_MENU_EXPANDER
		    AGSI_BOX_VERT AGSI_BOX_HORIZ
		    AGSI_ALICE AGSI_BOB AGSI_TEE_SHIRT
		    AGSI_JEANS AGSI_USER_W_3_SOUND_WAVES AGSI_PILE_OF_POO
		    AGSI_FOLDED_DIAPER AGSI_UNFOLDED_DIAPER AGSI_PAPER_ROLL
		    AGSI_CONTAINER AGSI_PARCEL AGSI_SIZE_XS AGSI_SIZE_SM
		    AGSI_SIZE_MD AGSI_SIZE_LG AGSI_SIZE_XL AGSI_SIZE_2XL
		    AGSI_SIZE_3XL AGSI_SIZE_4XL AGSI_LOWER_R_PENCIL 
		    "\n"
		    AGSI_LOWER_L_PENCIL AGSI_CLOSE_X AGSI_GEAR AGSI_EXPORT_DOCUMENT
		    AGSI_PAD AGSI_DEBUGGER AGSI_L_MENU_EXPANDER AGSI_USB_STICK
		    AGSI_VERTICAL_SPOOL AGSI_HORIZONTAL_SPOOL AGSI_WHEELCHAIR_SYMBOL
		    AGSI_DIP_CHIP AGSI_SURFACE_MOUNT_CHIP AGSI_VACUUM_TUBE
		    AGSI_STOPWATCH AGSI_ZOOM_IN AGSI_ZOOM_OUT
		    AGSI_ZOOM_RESET AGSI_AGAR_AG AGSI_AGAR_AR AGSI_CUT AGSI_COPY
		    AGSI_LH_COPY AGSI_CLIPBOARD AGSI_PASTE AGSI_LH_PASTE
		    AGSI_SELECT_ALL AGSI_FLOPPY_DISK AGSI_DVD AGSI_CLEAR_ALL
		    AGSI_JOYSTICK AGSI_GAME_CONTROLLER AGSI_TOUCHSCREEN
		    AGSI_TWO_BUTTON_MOUSE
		    "\n"
		    AGSI_TRI_CONSTRUCTION_SIGN AGSI_CONSTRUCTION_SIGN
		    AGSI_EDGAR_ALLAN_POE AGSI_AGARIAN AGSI_AGARIAN_WARRIOR
		    AGSI_UNDO AGSI_REDO AGSI_ALPHA_ARCH AGSI_AMIGA_BALL
		    AGSI_COMMODORE_LOGO AGSI_AMD_LOGO AGSI_6502_ARCH
		    AGSI_AMIGA_LOGO AGSI_MOTOROLA_LOGO AGSI_MAMISMOKE
		    AGSI_TGT_FG_COLOR AGSI_TGT_BG_COLOR AGSI_ARM_ARCH
		    AGSI_DREAMCAST AGSI_GAMECUBE AGSI_SEGA AGSI_PA_RISC_ARCH
		    AGSI_X86_ARCH AGSI_X64_ARCH AGSI_I386_ARCH AGSI_JSON
		    AGSI_NES_CONTROLLER AGSI_MIPS32_ARCH AGSI_MIPS64_ARCH
		    AGSI_N64_LOGO AGSI_IA64_ARCH AGSI_PPC32_ARCH AGSI_PPC64_ARCH
		    AGSI_SNES_LOGO
		    "\n"
		    AGSI_RISCV_ARCH);

	} else {
		if (altPhrase) {
			S = AG_TextRender(
			    "ABCDEFGHIJKLMNOPQRSTUVWXYZ 0123456789\n"
			    "abcdefghijklmnopqrstuvwxyz 0123456789");
		} else {
			S = AG_TextRender(
			    "The Quick Brown Fox Jumps Over The Lazy Dog");
		}
	}
	return (S);
}

static void
Init(void *_Nonnull obj)
{
	AG_FontSelector *fs = obj;
	AG_Notebook *nb;
	AG_NotebookTab *nt;
	
	fs->flags = AG_FONTSELECTOR_UPDATE;
	fs->curFace[0] = '\0';
	fs->curStyle = 0;
	fs->sPreview = -1;
	fs->curSize = 0.0f;

	fs->hPane = AG_PaneNewHoriz(fs, AG_PANE_EXPAND);
	fs->tlFaces = AG_TlistNew(fs->hPane->div[0], AG_TLIST_EXPAND);
	fs->hPane2 = AG_PaneNewHoriz(fs->hPane->div[1], AG_PANE_EXPAND);
	fs->tlStyles = AG_TlistNew(fs->hPane2->div[0], AG_TLIST_EXPAND);
	fs->tlSizes = AG_TlistNew(fs->hPane2->div[1], AG_TLIST_EXPAND);

	nb = AG_NotebookNew(fs->hPane2->div[1], AG_NOTEBOOK_HFILL);
	AG_SetFontSize(nb, "80%");

	nt = AG_NotebookAdd(nb, _("Color"), AG_BOX_VERT);
	AG_SetPadding(nt, "2");
	{
		AG_Box *boxHoriz, *boxColor;
		AG_HSVPal *pal;
		AG_Button *btnBG, *btnFG;

		boxHoriz = AG_BoxNewHoriz(nt, AG_BOX_EXPAND);

		pal = fs->pal = AG_HSVPalNew(boxHoriz,
		    AG_HSVPAL_NOPREVIEW | AG_HSVPAL_NOALPHA |
		    AG_HSVPAL_HFILL);

		AG_BindPointer(pal, "agcolor", (void *)&fs->cPreviewFG);
		AG_SetEvent(pal, "h-changed", PreviewColorChanged,"%p",fs);
		AG_SetEvent(pal, "sv-changed", PreviewColorChanged,"%p",fs);

		boxColor = AG_BoxNewVert(boxHoriz, AG_BOX_VFILL |
		                                   AG_BOX_HOMOGENOUS);
		AG_SetFontSize(boxColor, "80%");
		{
			btnFG = AG_ButtonNewS(boxColor,
			    AG_BUTTON_STICKY | AG_BUTTON_HFILL,
			    AGSI_IDEOGRAM AGSI_TGT_FG_COLOR AGSI_RST "\n\n"
			    "FG");
			btnBG = AG_ButtonNewS(boxColor,
			    AG_BUTTON_STICKY | AG_BUTTON_HFILL,
			    AGSI_IDEOGRAM AGSI_TGT_BG_COLOR AGSI_RST "\n\n"
			    "BG");

			AG_ButtonSetState(btnFG, 1);

			AG_SetPadding(btnFG, "4");
			AG_SetPadding(btnBG, "4");

			AG_SetEvent(btnFG, "button-pushed",
			    EditBgFgColor,"%p,%p,%p", fs, &fs->cPreviewFG, btnBG);
			AG_SetEvent(btnBG, "button-pushed",
			    EditBgFgColor,"%p,%p,%p", fs, &fs->cPreviewBG, btnFG);
		}
	}

	nt = AG_NotebookAdd(nb, _("Metrics"), AG_BOX_VERT);
	AG_SetPadding(nt, "5");
	{
		AG_CheckboxNewFlag(nt, 0, _("Adjusted Baseline"), &fs->flags,
		    AG_FONTSELECTOR_BASELINE);
		AG_CheckboxNewFlag(nt, 0, _("Original Baseline"), &fs->flags,
		    AG_FONTSELECTOR_CORRECTIONS);
		AG_CheckboxNewFlag(nt, 0, _("Bounding Box"), &fs->flags,
		    AG_FONTSELECTOR_BOUNDING_BOX);

		AG_SeparatorNewHoriz(nt);

		fs->lblMetrics = AG_LabelNewS(nt, AG_LABEL_EXPAND, _("No data."));
	}
	
	fs->font = NULL;
	fs->rPreview.x = 0;   /* TODO use pane */
	fs->rPreview.y = 0;
	fs->rPreview.w = 0;
	fs->rPreview.h = 80;
	AG_ColorNone(&fs->cPreviewBG);
	AG_ColorWhite(&fs->cPreviewFG);
	fs->previewFn = PreviewDefault;

	AG_TlistSizeHint(fs->tlFaces, "<Adobe New Century Schoolbook>", 10);
	AG_TlistSizeHint(fs->tlStyles, "<Condensed Bold Oblique>", 8);
	AG_TlistSizeHint(fs->tlSizes, "<XXXXXXX>", 8);
	
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
