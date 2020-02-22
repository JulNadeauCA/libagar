/*
 * Copyright (c) 2008-2020 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <string.h>

#include <agar/config/have_fontconfig.h>
#ifdef HAVE_FONTCONFIG
#include <fontconfig/fontconfig.h>
extern int agFontconfigInited;		/* text.c */
#endif

const char *agFontsToIgnore[] = {
	"ClearlyU Alternate Glyphs",
	"ClearlyU PUA",
	"Cursor",
	"cursor.pcf",
	"deccurs.pcf",
	"decsess.pcf",
	"micro.pcf",
	"Newspaper",
	NULL
};

AG_FontSelector *
AG_FontSelectorNew(void *parent, Uint flags)
{
	AG_FontSelector *fs;

	fs = Malloc(sizeof(AG_FontSelector));
	AG_ObjectInit(fs, &agFontSelectorClass);
	fs->flags |= flags;
	if (flags & AG_FONTSELECTOR_HFILL) { AG_ExpandHoriz(fs); }
	if (flags & AG_FONTSELECTOR_VFILL) { AG_ExpandVert(fs); }
	
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

	AG_ObjectLock(fs);

	pFont = b->data.p;
	AG_SetPointer(fs, "font", *pFont);
	Strlcpy(fs->curFace, OBJECT(*pFont)->name, sizeof(fs->curFace));
	fs->curSize = (*pFont)->spec.size;
	fs->curStyle = (*pFont)->flags;
#if 0
	fs->tlFaces->flags |= AG_TLIST_SCROLLTOSEL;
	fs->tlSizes->flags |= AG_TLIST_SCROLLTOSEL;
#endif
	AG_ObjectUnlock(fs);
}

static void
UpdateFontSelection(AG_FontSelector *_Nonnull fs)
{
	AG_Variable *bFont;
	AG_Font *font, **pFont;

	font = AG_FetchFont(fs->curFace, fs->curSize, fs->curStyle);
	if (font == NULL) {
		Verbose(_("Error opening font: %s\n"), AG_GetError());
		return;
	}
	bFont = AG_GetVariable(fs, "font", (void *)&pFont);
	*pFont = font;
	AG_UnlockVariable(bFont);
}

static void
UpdatePreview(AG_FontSelector *_Nonnull fs)
{
	AG_Variable *bFont;
	AG_Font **pFont;
	AG_Surface *S;
	
	bFont = AG_GetVariable(fs, "font", (void *)&pFont);
	AG_PushTextState();

	AG_TextColor(&fs->cPreview);
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
	AG_UnlockVariable(bFont);
}

static void
OnShow(AG_Event *_Nonnull event)
{
	AG_Variable *bFont;
	AG_Font **pFont;
	AG_FontSelector *fs = AG_FONTSELECTOR_SELF();
	AG_StaticFont **pbf;
	AG_TlistItem *ti;
	int i;
	const int stdSizes[] = { 4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
	                         22,24,26,28,32,48,64 }; /* XXX use fontconfig? */
	const int nStdSizes = sizeof(stdSizes) / sizeof(stdSizes[0]);
	
	bFont = AG_GetVariable(fs, "font", (void *)&pFont);
	AG_PushTextState();

	fs->flags &= ~(AG_FONTSELECTOR_UPDATE);

	for (pbf = &agBuiltinFonts[0]; *pbf != NULL; pbf++) {
		if (strchr((*pbf)->name, '_'))              /* Is a variant */
			continue;

		ti = AG_TlistAdd(fs->tlFaces, NULL, "%s", (*pbf)->name);
		ti->p1 = *pbf;

		if (*pFont && strcmp(ti->text, OBJECT(*pFont)->name) == 0)
			ti->selected++;
	}

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
				FcPattern *font = fset->fonts[i];
				FcChar8 *fam;
				const char **fignore;

				if (FcPatternGetString(font, FC_FAMILY, 0,
				    &fam) == FcResultMatch) {
					for (fignore = &agFontsToIgnore[0];
					    *fignore != NULL;
					     fignore++) {
						if (strcmp(*fignore, (char *)fam) == 0)
							break;
					}
					if (*fignore != NULL)
						continue;

					ti = AG_TlistAddS(fs->tlFaces, NULL,
					    (char *)fam);
					if (*pFont && strcmp((char *)fam,
					    OBJECT(*pFont)->name) == 0)
						ti->selected++;
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
	{
		AG_ConfigPath *fpath;

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
				ti = AG_TlistAddS(fs->tlFaces, NULL, file);
				if (*pFont && strcmp(file, OBJECT(*pFont)->name) == 0)
					ti->selected++;
			}
			AG_CloseDir(dir);
		}
	}

	AG_TlistSort(fs->tlFaces);

	/* XXX */
	for (i = 0; i < nStdSizes; i++) {
		ti = AG_TlistAdd(fs->tlSizes, NULL, "%d", stdSizes[i]);
		if (*pFont && stdSizes[i] == (*pFont)->spec.size)
			ti->selected++;
	}

	ti = AG_TlistAdd(fs->tlStyles, NULL, _("Styles:"));
	ti->flags |= AG_TLIST_NO_SELECT;
	AG_TlistSetFont(fs->tlFaces, ti,
	    AG_TextFontPctFlags(80, AG_FONT_UNDERLINE));

	ti = AG_TlistAdd(fs->tlStyles, NULL, _("Regular"));
	if (*pFont && (*pFont)->flags == 0) { ti->selected++; }
	ti = AG_TlistAdd(fs->tlStyles, NULL, _("Bold"));
	if (*pFont && (*pFont)->flags == AG_FONT_BOLD) { ti->selected++; }
	ti = AG_TlistAdd(fs->tlStyles, NULL, _("Italic"));
	if (*pFont && (*pFont)->flags == AG_FONT_ITALIC) { ti->selected++; }
	ti = AG_TlistAdd(fs->tlStyles, NULL, _("Bold Italic"));
	if (*pFont && ((*pFont)->flags & AG_FONT_BOLD &&
	               (*pFont)->flags & AG_FONT_ITALIC)) { ti->selected++; }
	ti = AG_TlistAdd(fs->tlStyles, NULL, _("Upright Italic"));
	if (*pFont && (*pFont)->flags == AG_FONT_UPRIGHT_ITALIC) { ti->selected++; }
	ti = AG_TlistAdd(fs->tlStyles, NULL, _("Oblique"));
	if (*pFont && (*pFont)->flags == AG_FONT_OBLIQUE) { ti->selected++; }
	ti = AG_TlistAdd(fs->tlStyles, NULL, _("Bold Oblique"));
	if (*pFont && ((*pFont)->flags & AG_FONT_BOLD &&
	               (*pFont)->flags & AG_FONT_OBLIQUE)) { ti->selected++; }

	ti = AG_TlistAdd(fs->tlStyles, NULL, _("Width variants:"));
	ti->flags |= AG_TLIST_NO_SELECT;
	AG_TlistSetFont(fs->tlFaces, ti,
	    AG_TextFontPctFlags(80, AG_FONT_UNDERLINE));

	ti = AG_TlistAdd(fs->tlStyles, NULL, _("Condensed"));
	if (*pFont && (*pFont)->flags == AG_FONT_CONDENSED) { ti->selected++; }
	ti = AG_TlistAdd(fs->tlStyles, NULL, _("Condensed Bold"));
	if (*pFont && ((*pFont)->flags & AG_FONT_CONDENSED &&
	               (*pFont)->flags & AG_FONT_BOLD)) { ti->selected++; }
	ti = AG_TlistAdd(fs->tlStyles, NULL, _("Condensed Italic"));
	if (*pFont && ((*pFont)->flags & AG_FONT_CONDENSED &&
	               (*pFont)->flags & AG_FONT_ITALIC)) { ti->selected++; }
	ti = AG_TlistAdd(fs->tlStyles, NULL, _("Condensed Bold Italic"));
	if (*pFont && ((*pFont)->flags & AG_FONT_CONDENSED &&
	               (*pFont)->flags & AG_FONT_BOLD &&
	               (*pFont)->flags & AG_FONT_ITALIC)) { ti->selected++; }
	ti = AG_TlistAdd(fs->tlStyles, NULL, _("Condensed Oblique"));
	if (*pFont && ((*pFont)->flags & AG_FONT_CONDENSED &&
	               (*pFont)->flags & AG_FONT_OBLIQUE)) { ti->selected++; }
	ti = AG_TlistAdd(fs->tlStyles, NULL, _("Condensed Bold Oblique"));
	if (*pFont && ((*pFont)->flags & AG_FONT_CONDENSED &&
	               (*pFont)->flags & AG_FONT_BOLD &&
	               (*pFont)->flags & AG_FONT_OBLIQUE)) { ti->selected++; }

	if (fs->flags & AG_FONTSELECTOR_SW_STYLES) {
		ti = AG_TlistAdd(fs->tlStyles, NULL, _("Software styles:"));
		ti->flags |= AG_TLIST_NO_SELECT;
		AG_TlistSetFont(fs->tlFaces, ti,
		    AG_TextFontPctFlags(80, AG_FONT_UNDERLINE));

		ti = AG_TlistAdd(fs->tlStyles, NULL, _("Software Bold"));
		if (*pFont && (*pFont)->flags == AG_FONT_SW_BOLD) { ti->selected++; }
		ti = AG_TlistAdd(fs->tlStyles, NULL, _("Software Oblique"));
		if (*pFont && (*pFont)->flags == AG_FONT_SW_OBLIQUE) { ti->selected++; }
		ti = AG_TlistAdd(fs->tlStyles, NULL, _("Software Bold Oblique"));
		if (*pFont && (*pFont)->flags == (AG_FONT_SW_BOLD | AG_FONT_SW_OBLIQUE)) { ti->selected++; }
	}
	UpdatePreview(fs);

	AG_UnlockVariable(bFont);
}

static void
SelectedFace(AG_Event *_Nonnull event)
{
	AG_FontSelector *fs = AG_FONTSELECTOR_PTR(1);
	AG_TlistItem *it = AG_TLIST_ITEM_PTR(2);

	Strlcpy(fs->curFace, it->text, sizeof(fs->curFace));
	UpdateFontSelection(fs);
	UpdatePreview(fs);
}

static void
SelectedStyle(AG_Event *_Nonnull event)
{
	AG_FontSelector *fs = AG_FONTSELECTOR_PTR(1);
	const AG_TlistItem *it = AG_TLIST_ITEM_PTR(2);
	Uint flags = 0;

	/* XXX */
	if (!strcmp(it->text, _("Bold")))                   { flags |=  AG_FONT_BOLD; }
	if (!strcmp(it->text, _("Italic")))                 { flags |=  AG_FONT_ITALIC; }
	if (!strcmp(it->text, _("Bold Italic")))            { flags |= (AG_FONT_BOLD | AG_FONT_ITALIC); }
	if (!strcmp(it->text, _("Upright Italic")))         { flags |=  AG_FONT_UPRIGHT_ITALIC; }
	if (!strcmp(it->text, _("Oblique")))                { flags |=  AG_FONT_OBLIQUE; }
	if (!strcmp(it->text, _("Bold Oblique")))           { flags |= (AG_FONT_BOLD | AG_FONT_OBLIQUE); }

	if (!strcmp(it->text, _("Condensed")))              { flags |=  AG_FONT_CONDENSED; }
	if (!strcmp(it->text, _("Condensed Italic")))       { flags |= (AG_FONT_CONDENSED | AG_FONT_ITALIC); }
	if (!strcmp(it->text, _("Condensed Oblique")))      { flags |= (AG_FONT_CONDENSED | AG_FONT_OBLIQUE); }
	if (!strcmp(it->text, _("Condensed Bold")))         { flags |= (AG_FONT_CONDENSED | AG_FONT_BOLD); }
	if (!strcmp(it->text, _("Condensed Bold Italic")))  { flags |= (AG_FONT_CONDENSED | AG_FONT_BOLD | AG_FONT_ITALIC); }
	if (!strcmp(it->text, _("Condensed Bold Oblique"))) { flags |= (AG_FONT_CONDENSED | AG_FONT_BOLD | AG_FONT_OBLIQUE); }

	if (fs->flags & AG_FONTSELECTOR_SW_STYLES) {
		if (!strcmp(it->text, _("Software Oblique")))        { flags |=  AG_FONT_SW_ITALIC; }
		if (!strcmp(it->text, _("Software Bold")))           { flags |=  AG_FONT_SW_BOLD; }
		if (!strcmp(it->text, _("Software Bold Oblique")))   { flags |= (AG_FONT_SW_BOLD | AG_FONT_SW_ITALIC); }
	}

	fs->curStyle = flags;
	UpdateFontSelection(fs);
	UpdatePreview(fs);
}

static void
SelectedSize(AG_Event *_Nonnull event)
{
	AG_FontSelector *fs = AG_FONTSELECTOR_PTR(1);
	const AG_TlistItem *it = AG_TLIST_ITEM_PTR(2);

	fs->curSize = (float)strtod(it->text, NULL);
	UpdateFontSelection(fs);
	UpdatePreview(fs);
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_FontSelector *fs = AG_FONTSELECTOR_SELF();
	const int x = AG_INT(2);
	const int y = AG_INT(3);

	if (AG_RectInside(&fs->rPreview, x,y)) {
		if (fs->flags & AG_FONTSELECTOR_ALT_PHRASE) {
			fs->flags &= ~(AG_FONTSELECTOR_ALT_PHRASE);
		} else {
			fs->flags |= AG_FONTSELECTOR_ALT_PHRASE;
		}
		if (fs->sPreview != -1) {
			AG_WidgetUnmapSurface(fs, fs->sPreview);
		}
		UpdatePreview(fs);
		AG_Redraw(fs);
	}
}

static void
PreviewColorChanged(AG_Event *_Nonnull event)
{
	AG_FontSelector *fs = AG_FONTSELECTOR_PTR(1);

	UpdatePreview(fs);
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
		fs->tlFaces = AG_TlistNew(fs->hPane->div[0], AG_TLIST_EXPAND);
		fs->hPane2 = AG_PaneNewHoriz(fs->hPane->div[1], AG_PANE_EXPAND);
		fs->tlStyles = AG_TlistNew(fs->hPane2->div[0], AG_TLIST_EXPAND);
		fs->sizeBox = AG_BoxNewVert(fs->hPane2->div[1], AG_BOX_EXPAND);
		{
			fs->tlSizes = AG_TlistNew(fs->sizeBox, AG_TLIST_EXPAND);
			fs->pal = AG_HSVPalNew(fs->sizeBox, AG_HSVPAL_NOPREVIEW |
			                                    AG_HSVPAL_NOALPHA |
		                                            AG_HSVPAL_HFILL);
			AG_BindPointer(fs->pal, "agcolor", (void *)&fs->cPreview);
			AG_SetEvent(fs->pal, "h-changed", PreviewColorChanged, "%p", fs);
			AG_SetEvent(fs->pal, "sv-changed", PreviewColorChanged, "%p", fs);
		}
	}
	
	fs->font = NULL;
	fs->rPreview.x = 0;
	fs->rPreview.y = 0;
	fs->rPreview.w = 0;
	fs->rPreview.h = 80;
	AG_ColorWhite(&fs->cPreview);

	AG_TlistSizeHint(fs->tlFaces, "XXXXXXXXXXXXXXXXXXXX", 20);
	AG_TlistSizeHint(fs->tlStyles, "<Condensed Bold Oblique>", 20);
	AG_TlistSizeHint(fs->tlSizes, "100", 20);
	
	/* Handle "font" binding programmatically */
	AG_BindPointer(fs, "font", (void *)&fs->font);
	AG_SetEvent(fs, "bound", Bound, NULL);
	OBJECT(fs)->flags |= AG_OBJECT_BOUND_EVENTS;

	AG_AddEvent(fs, "widget-shown", OnShow, NULL);
	AG_SetEvent(fs, "mouse-button-down", MouseButtonDown, NULL);

	AG_SetEvent(fs->tlFaces, "tlist-selected", SelectedFace, "%p", fs);
	AG_SetEvent(fs->tlStyles, "tlist-selected", SelectedStyle, "%p", fs);
	AG_SetEvent(fs->tlSizes, "tlist-selected", SelectedSize, "%p", fs);
}

static void
Draw(void *_Nonnull obj)
{
	AG_FontSelector *fs = obj;
	AG_Widget *chld;

	OBJECT_FOREACH_CHILD(chld, obj, ag_widget)
		AG_WidgetDraw(chld);

	AG_DrawBoxSunk(fs, &fs->rPreview, &WCOLOR(fs,BG_COLOR));

	if (fs->sPreview != -1) {
		const AG_Surface *S = WSURFACE(fs,fs->sPreview);

		AG_PushClipRect(fs, &fs->rPreview);
		AG_WidgetBlitSurface(fs, fs->sPreview,
		    fs->rPreview.x + (fs->rPreview.w >> 1) - (S->w >> 1),
		    fs->rPreview.y + (fs->rPreview.h >> 1) - (S->h >> 1));
		AG_PopClipRect(fs);
	}
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_FontSelector *fs = obj;
	AG_SizeReq rChld;

	AG_WidgetSizeReq(fs->hPane, &rChld);
	r->w = rChld.w;
	r->h = rChld.h + fs->rPreview.h;
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_FontSelector *fs = obj;
	AG_SizeAlloc aChld;

	/* Size horizontal pane */
	aChld.x = 0;
	aChld.y = 0;
	aChld.w = a->w;
	aChld.h = a->h - fs->rPreview.h;
	AG_WidgetSizeAlloc(fs->hPane, &aChld);

	fs->rPreview.x = 0;
	fs->rPreview.y = a->h - fs->rPreview.h;
	fs->rPreview.w = a->w;
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
	SizeAllocate
};
