/*
 * Copyright (c) 2001-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002  Sam Lantinga
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
 * 3. Neither the name of the author, nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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
 * Rendering/sizing of UCS-4 or UTF-8 text using FreeType (if available) or
 * an internal bitmap font engine.
 *
 * TextSizeFT() and TextRenderFT() were originally based on code from SDL_ttf
 * (http://libsdl.org/projects/SDL_ttf/), placed under BSD license with
 * kind permission from Sam Lantinga.
 */

#include <agar/config/have_freetype.h>
#include <agar/config/have_fontconfig.h>
#include <agar/config/ttfdir.h>

#include <agar/core/core.h>
#include <agar/core/config.h>
#include <agar/core/win32.h>

#ifdef HAVE_FREETYPE
# include <agar/gui/ttf.h>
#endif
#ifdef HAVE_FONTCONFIG
# include <fontconfig/fontconfig.h>
#endif

#include <agar/gui/window.h>
#include <agar/gui/box.h>
#include <agar/gui/label.h>
#include <agar/gui/textbox.h>
#include <agar/gui/button.h>
#include <agar/gui/ucombo.h>
#include <agar/gui/numerical.h>
#include <agar/gui/keymap.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/load_xcf.h>
#include <agar/gui/iconmgr.h>
#include <agar/gui/icons.h>
#include <agar/gui/fonts.h>
#include <agar/gui/packedpixel.h>
#ifdef HAVE_FLOAT
#include <agar/gui/gui_math.h>
#endif
#if AG_MODEL != AG_SMALL
#include <agar/gui/fonts_data.h>
#endif

#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#ifdef AG_SERIALIZATION
/* Default fonts */
const char *agDefaultFaceFT = "_agFontVera";
const char *agDefaultFaceBitmap = "_agFontMinimal";
#endif

/* Statically compiled fonts */
AG_StaticFont *agBuiltinFonts[] = {
#if AG_MODEL != AG_SMALL
	&agFontVera,
	&agFontMinimal,
#endif
	NULL
};

int agTextFontHeight = 0;		/* Default font height (px) */
int agTextFontAscent = 0;		/* Default font ascent (px) */
int agTextFontDescent = 0;		/* Default font descent (px) */
int agTextFontLineSkip = 0;		/* Default font line skip (px) */
int agFreetypeInited = 0;		/* Initialized Freetype library */
int agFontconfigInited = 0;		/* Initialized Fontconfig library */

static int agTextInitedSubsystem = 0;
static AG_TextState agTextStateStack[AG_TEXT_STATES_MAX];
static Uint curState = 0;
AG_TextState *agTextState = NULL;

/* #define SYMBOLS */		/* Escape $(x) type symbols */

const char *agFontTypeNames[] = {		/* For enum ag_font_type */
	N_("Vector"),
	N_("Bitmap"),
	N_("Dummy")
};
const char *agTextMsgTitles[] = {		/* For enum ag_text_msg_title */
	N_("Error"),
	N_("Warning"),
	N_("Information")
};

_Nonnull_Mutex AG_Mutex agTextLock;
static TAILQ_HEAD(ag_fontq, ag_font) fonts;
AG_Font *_Nullable agDefaultFont = NULL;

static void TextRenderFT(const AG_Char *_Nonnull, AG_Surface *_Nonnull,
                         const AG_TextMetrics *_Nonnull);
#ifdef HAVE_FREETYPE
static void TextRenderFT_Underline(AG_TTFFont *_Nonnull, AG_Surface *_Nonnull,
                                   int);
# ifdef SYMBOLS
static int  TextRenderSymbol(Uint, AG_Surface *_Nonnull, int,int);
# endif
#endif
static AG_Glyph *_Nonnull TextRenderGlyph_Miss(AG_Driver *_Nonnull, AG_Char);

static void AG_TextStateInit(void);

/* Load an individual glyph from a bitmap font file. */
static void
LoadBitmapGlyph(AG_Surface *_Nonnull su, const char *_Nonnull lbl,
    void *_Nonnull pFont)
{
	AG_Font *font = pFont;

	if (font->nglyphs == 0) {
		Strlcpy(font->bspec, lbl, sizeof(font->bspec));
	}
	font->bglyphs = Realloc(font->bglyphs,
	                        (font->nglyphs+1)*sizeof(AG_Surface *));
	font->bglyphs[font->nglyphs++] = su;
}

#ifdef AG_SERIALIZATION
static int
GetFontTypeFromSignature(const char *_Nonnull path,
    enum ag_font_type *_Nonnull pType)
{
	char buf[13];
	FILE *f;

	if ((f = fopen(path, "rb")) == NULL) {
		AG_SetError(_("Unable to open %s"), path);
		return (-1);
	}
	if (fread(buf, 13, 1, f) == 13) {
		if (strncmp(buf, "gimp xcf file", 13) == 0) {
			*pType = AG_FONT_BITMAP;
		} else {
			*pType = AG_FONT_VECTOR;
		}
	} else {
		*pType = AG_FONT_VECTOR;
	}
	fclose(f);
	return (0);
}

/* Load a bitmap font. */
static int
OpenBitmapFont(AG_Font *_Nonnull font)
{
	char *s;
	char *msig, *c0, *c1;
	AG_DataSource *ds;

	switch (font->spec.sourceType) {
	case AG_FONT_SOURCE_FILE:
		ds = AG_OpenFile(font->spec.source.file, "rb");
		break;
	case AG_FONT_SOURCE_MEMORY:
		ds = AG_OpenConstCore(font->spec.source.mem.data,
		                      font->spec.source.mem.size);
		break;
	default:
		ds = NULL;
		break;
	}
	if (ds == NULL)
		return (-1);

	/* Allocate the glyph array. */
	if ((font->bglyphs = TryMalloc(sizeof(AG_Surface *))) == NULL) {
		AG_CloseDataSource(ds);
		return (-1);
	}
	font->nglyphs = 0;

	/* Load the glyphs from the XCF layers. */
	if (AG_XCFLoad(ds, 0, LoadBitmapGlyph, font) == -1) {
		AG_CloseDataSource(ds);
		return (-1);
	}
	AG_CloseDataSource(ds);

	/* Get the range of characters from the "MAP:x-y" string. */
	s = font->bspec;
	msig = AG_Strsep(&s, ":");
	c0 = AG_Strsep(&s, "-");
	c1 = AG_Strsep(&s, "-");
	if (font->nglyphs < 1 ||
	    msig == NULL || strcmp(msig, "MAP") != 0 ||
	    c0 == NULL || c1 == NULL ||
	    c0[0] == '\0' || c1[0] == '\0') {
		AG_SetError("XCF is missing the \"MAP:x-y\" string");
		return (-1);
	}
	font->c0 = (AG_Char)strtol(c0, NULL, 10);
	font->c1 = (AG_Char)strtol(c1, NULL, 10);
	if (font->nglyphs < (font->c1 - font->c0)) {
		AG_SetErrorS("XCF has inconsistent bitmap fontspec");
		return (-1);
	}
	font->height = font->bglyphs[0]->h;
	font->ascent = font->height;
	font->descent = 0;
	font->lineskip = font->height;
	return (0);
}
#endif /* AG_SERIALIZATION */

/* Save the current text rendering state. */
void
AG_PushTextState(void)
{
	AG_TextState *newState, *prevState;
	AG_Font *prevFont;

	AG_MutexLock(&agTextLock);
	if ((curState+1) >= AG_TEXT_STATES_MAX) {
		AG_FatalError("Text state stack overflow");
	}
	prevState = &agTextStateStack[curState];
	prevFont = prevState->font;

	newState = &agTextStateStack[++curState];

	newState->font = AG_FetchFont(OBJECT(prevFont)->name,
	                              &prevFont->spec.size,
				      prevFont->flags);

	memcpy(&newState->color, &prevState->color, sizeof(AG_Color));
	memcpy(&newState->colorBG, &prevState->colorBG, sizeof(AG_Color));
	newState->justify = prevState->justify;
	newState->valign = prevState->valign;
	newState->tabWd = prevState->tabWd;
#ifdef AG_DEBUG
	snprintf(newState->name, sizeof(newState->name), "TS%d", curState);
#endif
	agTextState = newState;

	AG_MutexUnlock(&agTextLock);
}

/* Initialize the text state to default settings. */
void
AG_TextStateInit(void)
{
	AG_TextState *ts = agTextState;

	ts->font = agDefaultFont;
	AG_ColorWhite(&ts->color);
	AG_ColorNone(&ts->colorBG);
	ts->justify = AG_TEXT_LEFT;
	ts->valign = AG_TEXT_TOP;
	ts->tabWd = agTextTabWidth;
#ifdef AG_DEBUG
	AG_Strlcpy(ts->name, "TS0", sizeof(ts->name));
#endif
}

/* Select the font face to use in rendering text. */
AG_Font *
AG_TextFontLookup(const char *face, const AG_FontPts *size, Uint flags)
{
	AG_Font *newFont;

	AG_MutexLock(&agTextLock);
	if ((newFont = AG_FetchFont(face, size, flags)) == NULL) {
		goto fail;
	}
	agTextState->font = newFont;
	AG_MutexUnlock(&agTextLock);
	return (newFont);
fail:
	AG_MutexUnlock(&agTextLock);
	return (NULL);
}

/* Set font size in points. */
AG_Font *
AG_TextFontPts(const AG_FontPts *size)
{
	AG_Font *font, *fontNew;

	AG_MutexLock(&agTextLock);
	font = agTextState->font;
	fontNew = AG_FetchFont(OBJECT(font)->name, size, font->flags);
	if (fontNew == NULL) {
		goto fail;
	}
	agTextState->font = fontNew;
	AG_MutexUnlock(&agTextLock);
	return (agTextState->font);
fail:
	AG_MutexUnlock(&agTextLock);
	return (NULL);
}

/* Set font size as % of the current font size. */
AG_Font *
AG_TextFontPct(int pct)
{
	AG_Font *font, *fontNew;
	AG_FontPts size;

	AG_MutexLock(&agTextLock);
	font = agTextState->font;
#ifdef HAVE_FLOAT
	size = font->spec.size*pct/100.0;
#else
	size = font->spec.size*pct/100;
#endif
	fontNew = AG_FetchFont(OBJECT(font)->name, &size, font->flags);
	if (fontNew == NULL) {
		goto fail;
	}
	agTextState->font = fontNew;
	AG_MutexUnlock(&agTextLock);
	return (agTextState->font);
fail:
	AG_MutexUnlock(&agTextLock);
	return (NULL);
}

/*
 * Load the given font (or return a pointer to an existing one), with the
 * given specifications. Size may be fractional except in integer-only build.
 */
AG_Font *
AG_FetchFont(const char *face, const AG_FontPts *fontSize, Uint flags)
{
	char name[AG_OBJECT_NAME_MAX];
#ifdef AG_SERIALIZATION
	AG_Config *cfg = AG_ConfigObject();
#endif
	AG_StaticFont *builtin = NULL;
	AG_Font *font;
	AG_FontSpec *spec;
	AG_FontPts myFontSize;

	if (fontSize == NULL) {
#ifdef AG_SERIALIZATION
		myFontSize = (AG_FontPts)AG_GetInt(cfg, "font.size");
#else
		myFontSize = 12;
#endif
		fontSize = &myFontSize;
	}
	if (face != NULL) {
		Strlcpy(name, face, sizeof(name));
	} else {
#ifdef AG_SERIALIZATION
		AG_GetString(cfg, "font.face", name, sizeof(name));
#else
		name[0] = '\0';
#endif
	}

	AG_MutexLock(&agTextLock);

	TAILQ_FOREACH(font, &fonts, fonts) {
#ifdef HAVE_FLOAT
		if (Fabs(font->spec.size - *fontSize) < AG_FONT_PTS_EPSILON &&
		    font->flags == flags &&
		    strcmp(OBJECT(font)->name, name) == 0)
			break;
#else
		if (font->spec.size == *fontSize &&
		    font->flags == flags &&
		    strcmp(OBJECT(font)->name, name) == 0)
			break;
#endif
	}
	if (font != NULL)
		goto out;

	if ((font = TryMalloc(sizeof(AG_Font))) == NULL) {
		AG_MutexUnlock(&agTextLock);
		return (NULL);
	}
	AG_ObjectInit(font, &agFontClass);
	AG_ObjectSetNameS(font, name);
	spec = &font->spec;
	spec->size = *fontSize;
	font->flags = flags;

	if (name[0] == '_') {
		AG_StaticFont **pbf;

		for (pbf = &agBuiltinFonts[0]; *pbf != NULL; pbf++) {
			if (strcmp((*pbf)->name, &name[1]) == 0) {
				builtin = *pbf;
				break;
			}
		}
		if (builtin == NULL) {
			AG_SetError(_("No such builtin font: %s"), name);
			goto fail;
		}
		spec->type = builtin->type;
		spec->sourceType = AG_FONT_SOURCE_MEMORY;
		spec->source.mem.data = builtin->data;
		spec->source.mem.size = builtin->size;
	} else {
#if defined(HAVE_FONTCONFIG) && defined(HAVE_FLOAT)
		if (agFontconfigInited) {
			FcPattern *pattern, *fpat;
			FcResult fres = FcResultMatch;
			FcChar8 *filename;
			FcMatrix *mat;
			char *nameIn;
			size_t nameLen;

			nameLen = strlen(name)+8;
			nameIn = Malloc(nameLen);
			if ((*fontSize - floor(*fontSize)) > 0.0) {
				Snprintf(nameIn, nameLen, "%s-%.2f",
				    name, *fontSize);
			} else {
				Snprintf(nameIn, nameLen, "%s-%.0f",
				    name, *fontSize);
			}
			Debug(font, "fontconfig query \"%s\"\n", nameIn);
			if ((pattern = FcNameParse((FcChar8 *)nameIn)) == NULL ||
			    !FcConfigSubstitute(NULL, pattern, FcMatchPattern)) {
				AG_SetError(_("Fontconfig failed to parse: %s"), name);
				free(nameIn);
				goto fail;
			}
			free(nameIn);

			FcDefaultSubstitute(pattern);
			if ((fpat = FcFontMatch(NULL, pattern, &fres)) == NULL ||
			    fres != FcResultMatch) {
				AG_SetError(_("Fontconfig failed to match: %s"), name);
				goto fail;
			}
			if (FcPatternGetString(fpat, FC_FILE, 0,
			    &filename) != FcResultMatch) {
				AG_SetErrorS("Fontconfig FC_FILE missing");
				goto fail;
			}
			Strlcpy(spec->source.file, (const char *)filename,
			    sizeof(spec->source.file));
	
			if (FcPatternGetInteger(fpat, FC_INDEX, 0, &spec->index) 
			    != FcResultMatch) {
				AG_SetErrorS("Fontconfig FC_INDEX missing");
				goto fail;
			}
			if (FcPatternGetDouble(fpat, FC_SIZE, 0, &spec->size)
			    != FcResultMatch) {
				AG_SetErrorS("Fontconfig FC_SIZE missing");
				goto fail;
			}
			if (FcPatternGetMatrix(fpat, FC_MATRIX, 0, &mat) == FcResultMatch) {
				spec->matrix.xx = mat->xx;
				spec->matrix.yy = mat->yy;
				spec->matrix.xy = mat->xy;
				spec->matrix.yx = mat->yx;
			}
			spec->type = AG_FONT_VECTOR;
			FcPatternDestroy(fpat);
			FcPatternDestroy(pattern);
		} else
#endif /* HAVE_FONTCONFIG and HAVE_FLOAT */
#ifdef AG_SERIALIZATION
		{
			if (AG_ConfigFind(AG_CONFIG_PATH_FONTS, name,
			    spec->source.file, sizeof(spec->source.file)) == -1) {
				char ttfFile[AG_FILENAME_MAX];

				Strlcpy(ttfFile, name, sizeof(ttfFile));
				Strlcat(ttfFile, ".ttf", sizeof(ttfFile));
				if (AG_ConfigFind(AG_CONFIG_PATH_FONTS, ttfFile,
				    spec->source.file, sizeof(spec->source.file)) == -1)
					goto fail;
			}
			if (GetFontTypeFromSignature(spec->source.file,
			   &spec->type) == -1)
				goto fail;
		}
		spec->sourceType = AG_FONT_SOURCE_FILE;
#else
		/* TODO */
#endif
	}

	switch (spec->type) {
#ifdef HAVE_FREETYPE
	case AG_FONT_VECTOR:
		if (!agFreetypeInited) {
			AG_SetErrorS("FreeType is not initialized");
			goto fail;
		}
		if (AG_TTFOpenFont(font) == -1) {
			goto fail;
		}
		break;
#endif
#ifdef AG_SERIALIZATION
	case AG_FONT_BITMAP:
		if (OpenBitmapFont(font) == -1) {
			goto fail;
		}
		break;
#endif
	case AG_FONT_DUMMY:
		break;
	default:
		AG_SetErrorS("Unsupported font type");
		goto fail;
	}
	TAILQ_INSERT_HEAD(&fonts, font, fonts);
out:
	font->nRefs++;
	AG_MutexUnlock(&agTextLock);
	return (font);
fail:
	AG_MutexUnlock(&agTextLock);
	AG_ObjectDestroy(font);
	return (NULL);
}

/* Restore the previous rendering state. */
void
AG_PopTextState(void)
{
	AG_MutexLock(&agTextLock);
	if (curState == 0) {
		AG_FatalError("Unexpected AG_PopTextState()");
	}
#if 0
	if (agTextState->font != NULL &&
	    agTextState->font != agDefaultFont)
		AG_UnusedFont(agTextState->font);
#endif
	AG_TextStateInit();
	agTextState = &agTextState[--curState];
	AG_MutexUnlock(&agTextLock);
}

/* Decrement reference count on a font, delete if it reaches zero. */
void
AG_UnusedFont(AG_Font *font)
{
	AG_MutexLock(&agTextLock);
	if (font != agDefaultFont) {
		if (--font->nRefs == 0) {
			TAILQ_REMOVE(&fonts, font, fonts);
			AG_ObjectDestroy(font);
		}
	}
	AG_MutexUnlock(&agTextLock);
}

/* Clear the glyph cache. */
void
AG_TextClearGlyphCache(AG_Driver *drv)
{
	int i;
	AG_Glyph *G, *Gnext;

	for (i = 0; i < AG_GLYPH_NBUCKETS; i++) {
		for (G = SLIST_FIRST(&drv->glyphCache[i].glyphs);
		     G != SLIST_END(&drv->glyphCache[i].glyphs);
		     G = Gnext) {
			Gnext = SLIST_NEXT(G, glyphs);
			AG_SurfaceFree(G->su);
			free(G);
		}
		SLIST_INIT(&drv->glyphCache[i].glyphs);
	}
}

#ifdef SYMBOLS
static __inline__ AG_Surface *_Nullable _Pure_Attribute
GetSymbolSurface(AG_Char ch)
{
	switch (ch) {
	case 'L': return agIconLeftButton.s;
	case 'M': return agIconMidButton.s;
	case 'R': return agIconRightButton.s;
	case 'C': return agIconCtrlKey.s;
	default: return (NULL);
	}
}
#endif /* SYMBOLS */

static __inline__ void
InitMetrics(AG_TextMetrics *_Nonnull tm)
{
	tm->w = 0;
	tm->h = 0;
	tm->wLines = NULL;
	tm->nLines = 0;
}

static __inline__ void
FreeMetrics(AG_TextMetrics *_Nonnull tm)
{
	Free(tm->wLines);
}

/* For DUMMY font engine */
static void
TextSizeDummy(const AG_Char *_Nonnull ucs, AG_TextMetrics *_Nonnull tm,
    int extended)
{
	tm->w = 0;
	tm->h = 0;
	tm->wLines = NULL;
	tm->nLines = 0;
}

/*
 * Compute the rendered size of UCS-4 text with a FreeType font. If the
 * string is multiline and nLines is non-NULL, the width of individual lines
 * is returned into wLines, and the number of lines into nLines.
 */
static void
TextSizeFT(const AG_Char *_Nonnull ucs, AG_TextMetrics *_Nonnull tm, int extended)
{
#ifdef HAVE_FREETYPE
	AG_Font *font = agTextState->font;
	AG_TTFFont *ftFont = font->ttf;
	AG_TTFGlyph *G;
	const AG_Char *ch;
	int xMin=0, xMax=0, yMin=0, yMax;
	int xMinLine=0, xMaxLine=0;
	int x, z;

	/* Compute the sum of the bounding box of the characters. */
	yMax = font->height;
	x = 0;
	for (ch = &ucs[0]; *ch != '\0'; ch++) {
		if (*ch == '\n') {
			if (extended) {
				tm->wLines = Realloc(tm->wLines,
				    (tm->nLines+2)*sizeof(Uint));
				tm->wLines[tm->nLines++] = (xMaxLine-xMinLine);
				xMinLine = 0;
				xMaxLine = 0;
			}
			yMax += font->lineskip;
			x = 0;
			continue;
		}
		if (*ch == '\t') {
			x += agTextState->tabWd;
			continue;
		}
		if (AG_TTFFindGlyph(ftFont, *ch, TTF_CACHED_METRICS) != 0) {
			continue;
		}
		G = ftFont->current;

		z = x + G->minx;
		if (xMin > z) { xMin = z; }
		if (xMinLine > z) { xMinLine = z; }

		if (ftFont->style & AG_TTF_STYLE_BOLD) {
			x += ftFont->glyph_overhang;
		}
		z = x + MAX(G->advance, G->maxx);
		if (xMax < z) { xMax = z; }
		if (xMaxLine < z) { xMaxLine = z; }
		x += G->advance;

		if (G->miny < yMin) { yMin = G->miny; }
		if (G->maxy > yMax) { yMax = G->maxy; }
	}
	if (*ch != '\n' && extended) {
		if (tm->nLines > 0) {
			tm->wLines = Realloc(tm->wLines,
			    (tm->nLines+2)*sizeof(Uint));
			tm->wLines[tm->nLines] = (xMaxLine-xMinLine);
		}
		tm->nLines++;
	}
	tm->w = (xMax-xMin);
	tm->h = (yMax-yMin);
#else /* !HAVE_FREETYPE */
	InitMetrics(tm);
#endif /* HAVE_FREETYPE */
}

static __inline__ AG_Surface *_Nonnull
GetBitmapGlyph(AG_Font *_Nonnull font, AG_Char c)
{
	if ((font->flags & AG_FONT_UPPERCASE) &&
	    (isalpha((int)c) && islower((int)c))) {
		c = (AG_Char)toupper((int)c);
	}
	if (c < font->c0 || c > font->c1) {
		return (agTextState->font->bglyphs[0]);
	}
	return (font->bglyphs[c - font->c0 + 1]);
}

/* Compute the rendered size of UCS-4 text with a bitmap font. */
static void
TextSizeBitmap(const AG_Char *_Nonnull ucs, AG_TextMetrics *_Nonnull tm,
    int extended)
{
#ifdef AG_SERIALIZATION
	AG_Font *font = agTextState->font;
	const AG_Char *c;
	AG_Surface *Sglyph;
	int wLine = 0, lineSkip = font->lineskip;

	for (c = &ucs[0]; *c != '\0'; c++) {
		if (*c == '\n') {
			if (extended) {
				tm->wLines = Realloc(tm->wLines,
				    (tm->nLines+2)*sizeof(Uint));
				tm->wLines[tm->nLines++] = wLine;
				wLine = 0;
			}
			tm->h += lineSkip;
			continue;
		}
		if (*c == '\t') {
			wLine += agTextState->tabWd;
			tm->w += agTextState->tabWd;
			continue;
		}
		Sglyph = GetBitmapGlyph(font, *c);
		wLine += Sglyph->w;
		tm->w += Sglyph->w;
		tm->h = MAX(tm->h, Sglyph->h);
	}
	if (*c != '\n' && extended) {
		if (tm->nLines > 0) {
			tm->wLines = Realloc(tm->wLines,
			    (tm->nLines+2)*sizeof(Uint));
			tm->wLines[tm->nLines] = wLine;
		}
		tm->nLines++;
	}
#else /* !AG_SERIALIZATION */
	InitMetrics(tm);
#endif /* AG_SERIALIZATION */
}

/* Render UCS-4 text to a new surface using a bitmap font. */
static void
TextRenderBitmap(const AG_Char *_Nonnull ucs, AG_Surface *_Nonnull S,
    const AG_TextMetrics *_Nonnull tm)
{
#ifdef AG_SERIALIZATION
	AG_Font *font = agTextState->font;
	AG_Surface *Sglyph;
	const AG_Char *c;
	AG_Rect rd;
	int line;

	rd.x = (tm->nLines > 1) ? AG_TextJustifyOffset(tm->w, tm->wLines[0]) : 0;
	rd.y = 0;
	
	for (c=&ucs[0], line=0; *c != '\0'; c++) {
		if (*c == '\n') {
			rd.y += font->lineskip;
			rd.x = AG_TextJustifyOffset(tm->w, tm->wLines[++line]);
			continue;
		}
		if (*c == '\t') {
			rd.x += agTextState->tabWd;
			continue;
		}
		Sglyph = GetBitmapGlyph(font, *c);
		if (*c != ' ') {
			AG_SurfaceBlit(Sglyph, NULL, S, rd.x, rd.y);
		}
		rd.x += Sglyph->w;
	}
	AG_SurfaceSetColorKey(S, AG_SURFACE_COLORKEY,
	    AG_MapPixel_RGBA(&S->format, 0,0,0,0));
	AG_SurfaceSetAlpha(S, AG_SURFACE_ALPHA, font->bglyphs[0]->alpha);
#endif /* AG_SERIALIZATION */
}

static void
TextRenderDummy(const AG_Char *_Nonnull ucs, AG_Surface *_Nonnull S,
    const AG_TextMetrics *_Nonnull tm)
{
	/* no-op */
}

/*
 * Return the expected size in pixels of a rendered C string.
 * If Unicode is supported, the string may contain UTF-8.
 */
void
AG_TextSize(const char *text, int *w, int *h)
{
#ifdef AG_UNICODE
	AG_Char *s;
#endif
	if (text == NULL || text[0] == '\0') {
		if (w != NULL) { *w = 0; }
		if (h != NULL) { *h = 0; }
		return;
	}
#ifdef AG_UNICODE
	if ((s = AG_ImportUnicode("UTF-8", text, NULL, NULL)) != NULL) {
		AG_TextSizeNat(s, w,h);
		free(s);
	}
#else
	AG_TextSizeNat((const Uint8 *)text, w,h);
#endif
}

/*
 * Return the rendered size in pixels of a string of internal AG_Char (which
 * may be internal UCS-4 or ASCII).
 */
void
AG_TextSizeNat(const AG_Char *s, int *w, int *h)
{
	void (*pfSize[])(const AG_Char *_Nonnull, AG_TextMetrics *_Nonnull, int) = {
		TextSizeFT,
		TextSizeBitmap,
		TextSizeDummy
	};
	const enum ag_font_type fontEngine = agTextState->font->spec.type;
	AG_TextMetrics tm;

#ifdef AG_DEBUG
	if (fontEngine >= AG_FONT_TYPE_LAST)
		AG_FatalError("Bad font type");
	if (!AG_OfClass(agTextState->font, "AG_Font:*"))
		AG_FatalError("Bad AG_Font obj");
#endif
	InitMetrics(&tm);
	if (s != NULL && (char)(s[0]) != '\0') {
		pfSize[fontEngine](s, &tm, 0);
	}
	if (w != NULL) { *w = tm.w; }
	if (h != NULL) { *h = tm.h; }
	FreeMetrics(&tm);
}

/*
 * Return the rendered size in pixels of a C string (which may contain UTF-8).
 * Return a line count and an array of line widths.
 */
void
AG_TextSizeMulti(const char *text, int *w, int *h, Uint **wLines, Uint *nLines)
{
#ifdef AG_UNICODE
	AG_Char *s;

	if ((s = AG_ImportUnicode("UTF-8", text, NULL, NULL)) != NULL) {
		AG_TextSizeMultiNat(s, w, h, wLines, nLines);
		free(s);
		return;
	}
#else
	AG_TextSizeMultiNat((const Uint8 *)text, w, h, wLines, nLines);
#endif
}

/*
 * Compute the rendered size of a string of internal AG_Char (which may
 * be UCS-4 or ASCII). Return a line count and an array of line widths.
 */
void
AG_TextSizeMultiNat(const AG_Char *s, int *w, int *h, Uint **wLines,
    Uint *nLines)
{
	AG_TextMetrics tm;
	void (*pfSize[])(const AG_Char *_Nonnull, AG_TextMetrics *_Nonnull, int) = {
		TextSizeFT,
		TextSizeBitmap,
		TextSizeDummy
	};
	const enum ag_font_type fontEngine = agTextState->font->spec.type;

#ifdef AG_DEBUG
	if (fontEngine >= AG_FONT_TYPE_LAST)
		AG_FatalError("Bad font type");
	if (!AG_OfClass(agTextState->font, "AG_Font:*"))
		AG_FatalError("Bad AG_Font obj");
#endif
	InitMetrics(&tm);
	if (s != NULL && (char)(s[0]) != '\0') {
		pfSize[fontEngine](s, &tm, 1);
	}
	if (w != NULL) { *w = tm.w; }
	if (h != NULL) { *h = tm.h; }

	if (tm.nLines == 1) {
		tm.wLines = Realloc(tm.wLines, sizeof(Uint));
		tm.wLines[0] = tm.w;
	}
	if (wLines != NULL) { *wLines = tm.wLines; }
	if (nLines != NULL) { *nLines = tm.nLines; }
}

/*
 * Canned notification and dialog windows.
 */

/* Display an informational message window. */
void
AG_TextMsg(enum ag_text_msg_title title, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_TextMsgS(title, s);
	free(s);
}
void
AG_TextMsgS(enum ag_text_msg_title title, const char *s)
{
	AG_Window *win;
	AG_VBox *vb;
	AG_Button *btnOK;

	win = AG_WindowNew(AG_WINDOW_NORESIZE | AG_WINDOW_NOCLOSE |
	                   AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE);
	if (win == NULL)
		return;

	win->wmType = AG_WINDOW_WM_NOTIFICATION;
	AG_WindowSetCaptionS(win, _(agTextMsgTitles[title]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, 0);
	AG_LabelNewS(vb, 0, s);

	vb = AG_VBoxNew(win, AG_VBOX_HOMOGENOUS | AG_VBOX_HFILL | AG_VBOX_VFILL);
	btnOK = AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win));

	AG_WidgetFocus(btnOK);
	AG_WindowShow(win);
}

/* Display the last error message from AG_GetError(). */
void
AG_TextMsgFromError(void)
{
	AG_TextMsgS(AG_MSG_ERROR, AG_GetError());
}

#ifdef AG_TIMERS
static Uint32
TextTmsgExpire(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_PTR(1);

	AG_ObjectDetach(win);
	return (0);
}

/* Display a message for a given period of time (format string). */
void
AG_TextTmsg(enum ag_text_msg_title title, Uint32 expire, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_TextTmsgS(title, expire, s);
	free(s);
}
void
AG_TextTmsgS(enum ag_text_msg_title title, Uint32 ticks, const char *s)
{
	AG_Window *win;
	AG_VBox *vb;
	AG_Timer *to;

	win = AG_WindowNew(AG_WINDOW_NORESIZE | AG_WINDOW_NOCLOSE |
	                   AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE);
	if (win == NULL)
		return;

	win->wmType = AG_WINDOW_WM_NOTIFICATION;
	AG_WindowSetCaptionS(win, _(agTextMsgTitles[title]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, 0);
	AG_LabelNewS(vb, 0, s);
	AG_WindowShow(win);

	to = AG_AddTimerAuto(NULL, ticks, TextTmsgExpire, "%p", win);
	if (to != NULL)
		Strlcpy(to->name, "textTmsg", sizeof(to->name));
}
#endif /* AG_TIMERS */

/*
 * Display an informational message with a "Don't tell me again" option.
 * The user preference is preserved in a persistent table. Unlike warnings,
 * the dialog window is not modal (format string).
 */
void
AG_TextInfo(const char *key, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_TextInfoS(key, s);
	free(s);
}
void
AG_TextInfoS(const char *key, const char *s)
{
#ifdef AG_SERIALIZATION
	AG_Config *cfg = AG_ConfigObject();
	char disableSw[64];
	AG_Variable *Vdisable;
#endif
	AG_Window *win;
	AG_VBox *vb;
	AG_Checkbox *cb;
	AG_Button *btnOK;
	
#ifdef AG_SERIALIZATION
	if (key != NULL) {
		Strlcpy(disableSw, "info.", sizeof(disableSw));
		Strlcat(disableSw, key, sizeof(disableSw));
		AG_ObjectLock(cfg);
		if (AG_Defined(cfg,disableSw) && AG_GetInt(cfg,disableSw) == 1)
			goto out;
	}
#endif
	win = AG_WindowNew(AG_WINDOW_NORESIZE | AG_WINDOW_NOCLOSE |
	                   AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE);
	if (win == NULL)
		return;

	win->wmType = AG_WINDOW_WM_DIALOG;

	AG_WindowSetCaptionS(win, _(agTextMsgTitles[AG_MSG_INFO]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, 0);
	AG_LabelNewS(vb, 0, s);

	vb = AG_VBoxNew(win, AG_VBOX_HOMOGENOUS | AG_VBOX_HFILL | AG_VBOX_VFILL);
	btnOK = AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win));
#ifdef AG_SERIALIZATION
	if (key != NULL) {
		cb = AG_CheckboxNewS(win, AG_CHECKBOX_HFILL,
		    _("Don't tell me again"));
		Vdisable = AG_SetInt(cfg, disableSw, 0);
		AG_BindInt(cb, "state", &Vdisable->data.i);
	}
#endif
	AG_WidgetFocus(btnOK);
	AG_WindowShow(win);
#ifdef AG_SERIALIZATION
out:
	if (key != NULL)
		AG_ObjectUnlock(cfg);
#endif
}

/*
 * Display a warning message with a "Don't tell me again" option.
 * The user preference is preserved in a persistent table.
 */
void
AG_TextWarning(const char *key, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_TextWarningS(key, s);
	free(s);
}
void
AG_TextWarningS(const char *key, const char *s)
{
#ifdef AG_SERIALIZATION
	AG_Config *cfg = AG_ConfigObject();
	char disableSw[64];
	AG_Variable *Vdisable;
#endif
	AG_Window *win;
	AG_VBox *vb;
	AG_Checkbox *cb;
	AG_Button *btnOK;

#ifdef AG_SERIALIZATION
	if (key != NULL) {
		Strlcpy(disableSw, "warn.", sizeof(disableSw));
		Strlcat(disableSw, key, sizeof(disableSw));
		AG_ObjectLock(cfg);
		if (AG_Defined(cfg,disableSw) && AG_GetInt(cfg,disableSw) == 1)
			goto out;
	}
#endif
	win = AG_WindowNew(AG_WINDOW_NORESIZE | AG_WINDOW_NOCLOSE |
	                   AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE);
	if (win == NULL)
		return;

	win->wmType = AG_WINDOW_WM_DIALOG;

	AG_WindowSetCaptionS(win, _(agTextMsgTitles[AG_MSG_WARNING]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, 0);
	AG_LabelNewS(vb, 0, s);

	vb = AG_VBoxNew(win, AG_VBOX_HOMOGENOUS | AG_VBOX_HFILL | AG_VBOX_VFILL);
	btnOK = AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win));

#ifdef AG_SERIALIZATION
	if (key != NULL) {
		cb = AG_CheckboxNewS(win, AG_CHECKBOX_HFILL,
		    _("Don't tell me again"));
		Vdisable = AG_SetInt(cfg, disableSw, 0);
		AG_BindInt(cb, "state", &Vdisable->data.i);
	}
#endif
	AG_WidgetFocus(btnOK);
	AG_WindowShow(win);

#ifdef AG_SERIALIZATION
out:
	if (key != NULL)
		AG_ObjectUnlock(cfg);
#endif
}

/* Display an error message. */
void
AG_TextError(const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_TextErrorS(s);
	free(s);
}
void
AG_TextErrorS(const char *s)
{
	AG_Window *win;
	AG_VBox *vb;
	AG_Button *btnOK;

	win = AG_WindowNew(AG_WINDOW_NORESIZE | AG_WINDOW_NOCLOSE |
	                   AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE);
	if (win == NULL)
		return;

	win->wmType = AG_WINDOW_WM_DIALOG;
	AG_WindowSetCaptionS(win, _(agTextMsgTitles[AG_MSG_ERROR]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, 0);
	AG_LabelNewS(vb, 0, s);

	vb = AG_VBoxNew(win, AG_VBOX_HOMOGENOUS|AG_VBOX_HFILL|AG_VBOX_VFILL);
	btnOK = AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win));

	AG_WidgetFocus(btnOK);
	AG_WindowShow(win);
}

/* Align a text surface inside a given space. */
void
AG_TextAlign(int *x, int *y, int wArea, int hArea, int wText, int hText,
    int lPad, int rPad, int tPad, int bPad, enum ag_text_justify justify,
    enum ag_text_valign valign)
{
	switch (justify) {
	case AG_TEXT_LEFT:
		*x = lPad;
		break;
	case AG_TEXT_CENTER:
		*x = ((wArea + lPad + rPad) >> 1) - (wText >> 1);
		break;
	case AG_TEXT_RIGHT:
		*x = wArea - rPad - wText;
		break;
	}
	switch (valign) {
	case AG_TEXT_TOP:
		*y = tPad;
		break;
	case AG_TEXT_MIDDLE:
		*y = ((hArea + tPad + bPad) >> 1) - (hText >> 1);
		break;
	case AG_TEXT_BOTTOM:
		*y = hArea - bPad - wText;
		break;
	}
}

/*
 * Return the offset in pixels needed to align text based on the current
 * justification mode.
 */
int
AG_TextJustifyOffset(int w, int wLine)
{
	switch (agTextState->justify) {
	case AG_TEXT_LEFT:	return (0);
	case AG_TEXT_CENTER:	return ((w >> 1) - (wLine >> 1));
	case AG_TEXT_RIGHT:	return (w - wLine);
	}
	return (0);
}

/*
 * Return the offset in pixels needed to align text based on the current
 * vertical alignment mode.
 */
int
AG_TextValignOffset(int h, int hLine)
{
	switch (agTextState->valign) {
	case AG_TEXT_TOP:	return (0);
	case AG_TEXT_MIDDLE:	return ((h >> 1) - (hLine >> 1));
	case AG_TEXT_BOTTOM:	return (h - hLine);
	}
	return (0);
}

/*
 * Allocate a transparent surface and render text on it (printf form).
 * The string may contain UTF-8 sequences.
 */
AG_Surface *
AG_TextRenderF(const char *fmt, ...)
{
	char *s;
	va_list args;
	AG_Surface *su;

	va_start(args, fmt);
	Vasprintf(&s, fmt, args);
	va_end(args);

	su = AG_TextRender(s);
	free(s);
	return (su);
}

/*
 * Allocate a transparent surface and render text on it (string form).
 * The string may contain UTF-8 sequences.
 */
AG_Surface *
AG_TextRender(const char *text)
{
	AG_Surface *su;
#ifdef AG_UNICODE
	AG_Char *s;

	if ((s = AG_ImportUnicode("UTF-8", text, NULL, NULL)) == NULL) {
		AG_FatalError(NULL);
	}
	su = AG_TextRenderNat(s);
	free(s);
#else
	su = AG_TextRenderNat((const Uint8 *)text);
#endif
	return (su);
}

/*
 * Render text (in internal AG_Char format which is UCS-4 or ASCII)
 * onto a newly allocated, native-format transparent surface.
 */
AG_Surface *
AG_TextRenderNat(const AG_Char *text)
{
	void (*pfSize[])(const AG_Char *_Nonnull, AG_TextMetrics *_Nonnull, int) = {
		TextSizeFT,
		TextSizeBitmap,
		TextSizeDummy
	};
	void (*pfRender[])(const AG_Char *_Nonnull, AG_Surface *_Nonnull,
	                   const AG_TextMetrics *_Nonnull) = {
		TextRenderFT,
		TextRenderBitmap,
		TextRenderDummy
	};
	const enum ag_font_type fontEngine = agTextState->font->spec.type;
	const AG_Color *colorBG = &agTextState->colorBG;
	AG_TextMetrics tm;
	AG_Surface *S;

#ifdef AG_DEBUG
	if (fontEngine >= AG_FONT_TYPE_LAST)
		AG_FatalError("Bad font type");
#endif

	/* Size the text and allocate the new surface. */
	InitMetrics(&tm);
	pfSize[fontEngine](text, &tm, 1);
	S = AG_SurfaceNew(agSurfaceFmt, tm.w, tm.h, 0);

	/*
	 * Fill the background. If fully transparent, set a colorkey to avoid
	 * further unnecessary blending ops with transparent pixels.
	 */
	AG_FillRect(S, NULL, colorBG);
	if (colorBG->a == AG_TRANSPARENT)
		AG_SurfaceSetColorKey(S, AG_SURFACE_COLORKEY,
		    AG_MapPixel(&S->format, colorBG));

	/* Finally render the text. */
	if (tm.w > 0 && tm.h > 0)
		pfRender[fontEngine](text, S, &tm);

	FreeMetrics(&tm);
	return (S);
}

/*
 * TODO use a separate routine for transparent vs. non-transparent
 * agTextState->colorBG.
 */
static void
TextRenderFT(const AG_Char *_Nonnull ucs, AG_Surface *_Nonnull S,
    const AG_TextMetrics *_Nonnull tm)
{
#ifdef HAVE_FREETYPE
	const AG_Font *font = agTextState->font;
	AG_Color c = agTextState->color;
	AG_TTFFont *ttf = font->ttf;
	AG_TTFGlyph *G;
	const AG_Char *ch;
	Uint8 *src, *dst;
	FT_UInt prev_index = 0;
	const int BytesPerPixel = S->format.BytesPerPixel;
	const int tabWd = agTextState->tabWd;
	const int lineSkip = font->lineskip;
	int xStart, yStart, line, x,y, w;
	AG_Color cBG = agTextState->colorBG;

 	xStart = (tm->nLines > 1) ? AG_TextJustifyOffset(tm->w, tm->wLines[0]) : 0;
 	yStart = 0;

	for (ch=&ucs[0], line=0; *ch != '\0'; ch++) {
		if (*ch == '\n') {
			yStart += lineSkip;
			xStart = AG_TextJustifyOffset(tm->w, tm->wLines[++line]);
			continue;
		}
		if (*ch == '\t') {
			xStart += tabWd;
			continue;
		}
# ifdef SYMBOLS
		if (ch[0] == '$' && agTextSymbols &&
		    ch[1] == '(' && ch[2] != '\0' && ch[3] == ')') {
			xStart += TextRenderSymbol(ch[2], S, xStart, yStart);
			ch += 3;
			continue;
		}
# endif
		if (AG_TTFFindGlyph(ttf, *ch,
		    TTF_CACHED_METRICS | TTF_CACHED_PIXMAP)) {
			Debug(NULL, "TextRenderFT: No 0x%x\n", *ch);
			return;
		}
		G = ttf->current;
		/*
		 * Ensure the width of the pixmap is correct. On some cases,
		 * freetype may report a larger pixmap than possible.
		 * XXX is this test always necessary?
		 */
		w = G->pixmap.width;
		if (w > G->maxx - G->minx) { w = G->maxx - G->minx; }

		if (FT_HAS_KERNING(ttf->face) && prev_index && G->index) {
			FT_Vector delta; 
			FT_Get_Kerning(ttf->face, prev_index, G->index,
			    ft_kerning_default, &delta); 
			xStart += delta.x >> 6;
		}
		
		/* Prevent texture wrapping with first glyph. */
		if ((ch == &ucs[0]) && (G->minx < 0))
			xStart -= G->minx;

		for (y = 0; y < G->pixmap.rows; y++) {
			if (y + G->yoffset < 0 ||
			    y + G->yoffset >= S->h) {
				continue;
			}
			dst = S->pixels +
			    (yStart + y + G->yoffset)*S->pitch +
			    (xStart + G->minx)*BytesPerPixel;

			/* Adjust src for pixmaps to account for pitch. */
			src = (Uint8 *)(G->pixmap.buffer +
			                G->pixmap.pitch*y);

			/* XXX TODO separate routine to avoid branch */
			if (cBG.a == AG_TRANSPARENT) {
				for (x = 0; x < w; x++) {
					c.a = AG_8toH(*src++);
					AG_SurfacePut_At(S, dst,
					    AG_MapPixel(&S->format, &c));
					dst += BytesPerPixel;
				}
			} else {
				for (x = 0; x < w; x++) {
					c.a = AG_8toH(*src++);
					AG_SurfaceBlend_At(S, dst, &c,
					    AG_ALPHA_DST);
					dst += BytesPerPixel;
				}
			}
		}
		xStart += G->advance;
		if (ttf->style & AG_TTF_STYLE_BOLD) {
			xStart += ttf->glyph_overhang;
		}
		prev_index = G->index;
	}
	if (ttf->style & AG_TTF_STYLE_UNDERLINE) {
		TextRenderFT_Underline(ttf, S, tm->nLines);
	}
#endif /* HAVE_FREETYPE */
}

#ifdef HAVE_FREETYPE
# ifdef SYMBOLS
static int
TextRenderSymbol(Uint ch, AG_Surface *_Nonnull s, int x, int y)
{
	AG_Surface *sym;
	AG_Pixel colorkey;
	Uint BytesPerPixel;
	int row, wSym;

	If ((sym = GetSymbolSurface(ch)) == NULL) {
		return (0);
	}
	wSym = sym->w;
	colorkey = sym->colorkey;
	BytesPerPixel = sym->format.BytesPerPixel;
	for (row = 0; row < sym->h; row++) {
		Uint8 *dst = s->pixels + (y + row)*(s->pitch >> 2) + (x+2);
		Uint8 *src = sym->pixels + row*sym->pitch;
		int col;

		for (col = 0; col < wSym; col++) {
			if (AG_SurfaceGet(sym,src) != colorkey) {
				dst[0] = 0xff;
				dst[1] = 0xff;
				dst[2] = 0xff;
			}
			src += BytesPerPixel;
			dst += 3;
		}
	}
	return (wSym + 4);
}
# endif /* SYMBOLS */

/*
 * Render underline style.
 * Surface must be at least ftFont->underline_height pixels high.
 */
static void
TextRenderFT_Underline(AG_TTFFont *_Nonnull ftFont, AG_Surface *_Nonnull S,
    int nLines)
{
	AG_Color c;
	AG_Pixel px;
	Uint8 *pDst;
	const int pad = 2;
	int w = S->w - pad;
	int lh = ftFont->underline_height;
	int incr = ftFont->ascent - ftFont->underline_offset - lh;
	int line, y0, lineskip = ftFont->lineskip;

	c = agTextState->color;
	px = AG_MapPixel(&S->format, &c);

	for (line=0, y0=incr; line < nLines; line++) {
		int x, y;

		if (y0 >= S->h) {
			y0 = S->h - lh;
		}
		pDst = S->pixels + y0*S->pitch;
		for (y = 0; y < lh; y++) {
			for (x = pad; x < w; x++) {
				AG_SurfacePut_At(S, pDst, px);
				pDst += S->format.BytesPerPixel;
			}
			pDst += S->padding;
		}
		y0 += lineskip;
	}
}
#endif /* HAVE_FREETYPE */

/*
 * Lookup/insert a glyph in the glyph cache.
 * Must be called from GUI rendering context.
 */
AG_Glyph *
AG_TextRenderGlyph(AG_Driver *drv, AG_Char ch)
{
	AG_Glyph *gl;
	Uint h = (Uint)(ch % AG_GLYPH_NBUCKETS);

	AG_SLIST_FOREACH(gl, &drv->glyphCache[h].glyphs, glyphs) {
		if (ch == gl->ch &&
		    agTextState->font == gl->font &&
		    AG_ColorCompare(&agTextState->color, &gl->color) == 0)
			break;
	}
	if (gl == NULL) {
		gl = TextRenderGlyph_Miss(drv, ch);
		AG_SLIST_INSERT_HEAD(&drv->glyphCache[h].glyphs, gl, glyphs);
	}
	return (gl);
}

/* Render a glyph following a cache miss; called from AG_TextRenderGlyph(). */
static AG_Glyph *_Nonnull
TextRenderGlyph_Miss(AG_Driver *_Nonnull drv, AG_Char ch)
{
	AG_Glyph *G;
	AG_Char s[2];

	G = Malloc(sizeof(AG_Glyph));
	G->font = agTextState->font;
	G->color = agTextState->color;
	G->ch = ch;

	s[0] = ch;
	s[1] = '\0';
	G->su = AG_TextRenderNat(s);

	switch (agTextState->font->spec.type) {
#ifdef HAVE_FREETYPE
	case AG_FONT_VECTOR:
		{
			AG_TTFGlyph *Gttf;

			if (AG_TTFFindGlyph(agTextState->font->ttf, ch,
			    TTF_CACHED_METRICS | TTF_CACHED_BITMAP) == 0) {
				Gttf = ((AG_TTFFont *)agTextState->font->ttf)->current;
				G->advance = Gttf->advance;
			} else {
				G->advance = G->su->w;
			}
		}
		break;
#endif
#ifdef AG_SERIALIZATION
	case AG_FONT_BITMAP:
		G->advance = G->su->w;
		break;
#endif
	case AG_FONT_DUMMY:
		break;
	}
	AGDRIVER_CLASS(drv)->updateGlyph(drv, G);
	return (G);
}

/* Set active text color. */
void
AG_TextColor(const AG_Color *c)
{
	AG_MutexLock(&agTextLock);
	memcpy(&agTextState->color, c, sizeof(AG_Color));
	AG_MutexUnlock(&agTextLock);
}
void
AG_TextColorRGB(Uint8 r, Uint8 g, Uint8 b)
{
	AG_MutexLock(&agTextLock);
	AG_ColorRGB_8(&agTextState->color, r,g,b);
	AG_MutexUnlock(&agTextLock);
}
void
AG_TextColorRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	AG_MutexLock(&agTextLock);
	AG_ColorRGBA_8(&agTextState->color, r,g,b,a);
	AG_MutexUnlock(&agTextLock);
}

/* Set text color from 0xRRGGBBAA format. */
void
AG_TextColorHex(Uint32 c)
{
	AG_MutexLock(&agTextLock);
	AG_ColorHex32(&agTextState->color, c);
	AG_MutexUnlock(&agTextLock);
}

/* Set active text background color. */
void
AG_TextBGColor(const AG_Color *c)
{
	AG_MutexLock(&agTextLock);
	memcpy(&agTextState->colorBG, c, sizeof(AG_Color));
	AG_MutexUnlock(&agTextLock);
}
void
AG_TextBGColorRGB(Uint8 r, Uint8 g, Uint8 b)
{
	AG_MutexLock(&agTextLock);
	AG_ColorRGB_8(&agTextState->colorBG, r,g,b);
	AG_MutexUnlock(&agTextLock);
}
void
AG_TextBGColorRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	AG_MutexLock(&agTextLock);
	AG_ColorRGBA_8(&agTextState->colorBG, r,g,b,a);
	AG_MutexUnlock(&agTextLock);
}

/* Set text BG color from 0xRRGGBBAA format. */
void
AG_TextBGColorHex(Uint32 c)
{
	AG_MutexLock(&agTextLock);
	AG_ColorHex(&agTextState->colorBG, c);
	AG_MutexUnlock(&agTextLock);
}

/* Select a specific font face to use in rendering text. */
void
AG_TextFont(AG_Font *font)
{
	AG_MutexLock(&agTextLock);
	agTextState->font = font;
	AG_MutexUnlock(&agTextLock);
}

/* Select the justification mode to use in rendering text. */
void
AG_TextJustify(enum ag_text_justify mode)
{
	AG_MutexLock(&agTextLock);
	agTextState->justify = mode;
	AG_MutexUnlock(&agTextLock);
}

/* Select the vertical alignment mode to use in rendering text. */
void
AG_TextValign(enum ag_text_valign mode)
{
	AG_MutexLock(&agTextLock);
	agTextState->valign = mode;
	AG_MutexUnlock(&agTextLock);
}

/* Select the tab width in pixels for rendering text. */
void
AG_TextTabWidth(int px)
{
	AG_MutexLock(&agTextLock);
	agTextState->tabWd = px;
	AG_MutexUnlock(&agTextLock);
}

#ifdef AG_SERIALIZATION
void
AG_SetDefaultFont(AG_Font *font)
{
	AG_Config *cfg;

	AG_MutexLock(&agTextLock);
	agDefaultFont = font;
	agTextFontHeight = font->height;
	agTextFontAscent = font->ascent;
	agTextFontDescent = font->descent;
	agTextFontLineSkip = font->lineskip;
	agTextState->font = font;
	cfg = AG_ConfigObject();
	AG_SetString(cfg, "font.face", OBJECT(font)->name);
	AG_SetInt(cfg, "font.size", (int)font->spec.size);
	AG_SetInt(cfg, "font.flags", font->flags);
	AG_MutexUnlock(&agTextLock);
}

/*
 * Parse a command-line font specification and set the default font.
 * The format is <face>,<size>,<flags>. Acceptable flags include 'b'
 * (bold), 'i' (italic) and 'U' (uppercase).
 */
void
AG_TextParseFontSpec(const char *fontspec)
{
	AG_Config *cfg = AG_ConfigObject();
	char buf[128];
	char *fs, *s, *c;

	Strlcpy(buf, fontspec, sizeof(buf));
	fs = &buf[0];

	if ((s = AG_Strsep(&fs, ":,/")) != NULL &&
	    s[0] != '\0') {
		AG_SetString(cfg, "font.face", s);
	}
	if ((s = AG_Strsep(&fs, ":,/")) != NULL &&
	    s[0] != '\0') {
		AG_SetInt(cfg, "font.size", atoi(s));
	}
	if ((s = AG_Strsep(&fs, ":,/")) != NULL &&
	    s[0] != '\0') {
		Uint flags = 0;

		for (c = &s[0]; *c != '\0'; c++) {
			switch (*c) {
			case 'b': flags |= AG_FONT_BOLD;	break;
			case 'i': flags |= AG_FONT_ITALIC;	break;
			case 'U': flags |= AG_FONT_UPPERCASE;	break;
			}
		}
		AG_SetUint(cfg, "font.flags", flags);
	}
}
#endif /* AG_SERIALIZATION */

static void
AG_Font_Init(void *_Nonnull obj)
{
	AG_Font *font = obj;
	AG_FontSpec *spec = &font->spec;

#ifdef HAVE_FLOAT
	spec->size = 0.0;
#else
	spec->size = 0;
#endif
	spec->index = 0;
	spec->type = AG_FONT_VECTOR;
	spec->sourceType = AG_FONT_SOURCE_FILE;
#ifdef HAVE_FLOAT
	spec->matrix.xx = 1.0;
	spec->matrix.xy = 0.0;
	spec->matrix.yx = 0.0;
	spec->matrix.yy = 1.0;
#endif
	spec->source.file[0] = '\0';

	font->flags = 0;
	font->height = 0;
	font->ascent = 0;
	font->descent = 0;
	font->lineskip = 0;
	font->bspec[0] = '\0';
	font->ttf = NULL;

	font->c0 = 0;
	font->c1 = 0;
	font->nRefs = 0;
}

static void
AG_Font_Destroy(void *_Nonnull obj)
{
	AG_Font *font = obj;

	switch (font->spec.type) {
#ifdef HAVE_FREETYPE
	case AG_FONT_VECTOR:
		AG_TTFCloseFont(font);
		break;
#endif
#ifdef AG_SERIALIZATION
	case AG_FONT_BITMAP:
		{
			int i;

			for (i = 0; i < font->nglyphs; i++) {
				AG_SurfaceFree(font->bglyphs[i]);
			}
			Free(font->bglyphs);
		}
		break;
#endif
	case AG_FONT_DUMMY:
		break;
	}
}

/* Prompt the user with a choice of options. */
AG_Window *
AG_TextPromptOptions(AG_Button **bOpts, Uint nbOpts, const char *fmt, ...)
{
	char *text;
	AG_Window *win;
	AG_Box *bo;
	va_list ap;
	Uint i;

	va_start(ap, fmt);
	Vasprintf(&text, fmt, ap);
	va_end(ap);

	if ((win = AG_WindowNew(AG_WINDOW_MODAL | AG_WINDOW_NOTITLE |
	                        AG_WINDOW_NORESIZE)) == NULL) {
		AG_FatalError(NULL);
	}
	win->wmType = AG_WINDOW_WM_DIALOG;
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_WindowSetSpacing(win, 8);

	AG_LabelNewS(win, 0, text);
	free(text);

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS | AG_BOX_HFILL);
	for (i = 0; i < nbOpts; i++) {
		bOpts[i] = AG_ButtonNewS(bo, 0, "XXXXXXXXXXX");
	}
	AG_WindowShow(win);
	return (win);
}

/* Initialize the font engine and configure the default font. */
int
AG_InitTextSubsystem(void)
{
#ifdef AG_SERIALIZATION
	AG_Config *cfg = AG_ConfigObject();
	AG_User *sysUser;
#endif
	if (agTextInitedSubsystem++ > 0)
		return (0);

	AG_MutexInitRecursive(&agTextLock);
	TAILQ_INIT(&fonts);

#ifdef AG_SERIALIZATION
	/* Set the default font search path. */
	AG_ObjectLock(cfg);
	sysUser = AG_GetRealUser();

	if (strcmp(TTFDIR, "NONE") != 0)
		AG_ConfigAddPathS(AG_CONFIG_PATH_FONTS, TTFDIR);

# if defined(__APPLE__)
	if (sysUser != NULL &&
	    sysUser->home != NULL) {
		AG_ConfigAddPath(AG_CONFIG_PATH_FONTS, "%s/Library/Fonts",
		    sysUser->home);
	}
	AG_ConfigAddPathS(AG_CONFIG_PATH_FONTS, "/Library/Fonts");
	AG_ConfigAddPathS(AG_CONFIG_PATH_FONTS, "/System/Library/Fonts");
# elif defined(_WIN32)
	{
		char windir[AG_PATHNAME_MAX];

		if (sysUser != NULL && sysUser->home != NULL) {
			AG_ConfigAddPath(AG_CONFIG_PATH_FONTS, "%s\\Fonts",
			    sysUser->home);
		}
		if (GetWindowsDirectoryA(windir, sizeof(windir)) > 0)
			AG_ConfigAddPath(AG_CONFIG_PATH_FONTS, "%s\\Fonts",
			    windir);
	}
# else /* !WIN32 & !APPLE */
	if (sysUser != NULL && sysUser->home != NULL) {
		AG_ConfigAddPath(AG_CONFIG_PATH_FONTS, "%s%s.fonts",
		    sysUser->home, AG_PATHSEP);
	}
# endif
	if (sysUser != NULL)
		AG_UserFree(sysUser);

#endif /* AG_SERIALIZATION */

#ifdef HAVE_FREETYPE
	if (AG_TTFInit() == 0) {		/* Initialize FreeType */
		agFreetypeInited = 1;
	} else {
		AG_Verbose("Failed to initialize FreeType (%s); falling back "
		           "to monospace font engine\n", AG_GetError());
	}
#endif
#ifdef HAVE_FONTCONFIG
	if (FcInit()) {
		agFontconfigInited = 1;
	} else {
		AG_Verbose("Failed to initialize fontconfig; ignoring\n");
	}
#endif
#ifdef AG_SERIALIZATION
	{
# ifdef AG_DEBUG
		int debugLvlSave = agDebugLvl;

		agDebugLvl = 0;
# endif

		/* Load the default font. */
		if (agFreetypeInited) {
			if (!AG_Defined(cfg,"font.face"))
				AG_SetString(cfg, "font.face", agDefaultFaceFT);
			if (!AG_Defined(cfg,"font.size"))
				AG_SetInt(cfg, "font.size", 14);
		} else {
			if (!AG_Defined(cfg,"font.face"))
				AG_SetString(cfg, "font.face", agDefaultFaceBitmap);
			if (!AG_Defined(cfg,"font.size"))
				AG_SetInt(cfg, "font.size", 16);
		}
		if (!AG_Defined(cfg,"font.flags"))
			AG_SetUint(cfg, "font.flags", 0);
# ifdef AG_DEBUG
		agDebugLvl = debugLvlSave;
# endif
	}
	AG_ObjectUnlock(cfg);
#endif /* AG_SERIALIZATION */

	/* Load the default font. */
	if ((agDefaultFont = AG_FetchFont(NULL, NULL, 0)) == NULL) {
		goto fail;
	}
	agTextFontHeight = agDefaultFont->height;
	agTextFontAscent = agDefaultFont->ascent;
	agTextFontDescent = agDefaultFont->descent;
	agTextFontLineSkip = agDefaultFont->lineskip;

	/* Initialize the rendering state. */
	curState = 0;
	agTextState = &agTextStateStack[0];
	AG_TextStateInit();
	return (0);
fail:
#ifdef HAVE_FREETYPE
	if (agFreetypeInited) {
		AG_TTFDestroy();
		agFreetypeInited = 0;
	}
#endif
	return (-1);
}

void
AG_DestroyTextSubsystem(void)
{
	AG_Font *font, *fontNext;
	
	if (--agTextInitedSubsystem > 0) {
		return;
	}
	for (font = TAILQ_FIRST(&fonts);
	     font != TAILQ_END(&fonts);
	     font = fontNext) {
		fontNext = TAILQ_NEXT(font, fonts);
		AG_ObjectDestroy(font);
	}
#ifdef HAVE_FREETYPE
	if (agFreetypeInited) {
		AG_TTFDestroy();
		agFreetypeInited = 0;
	}
#endif
#ifdef HAVE_FONTCONFIG
	if (agFontconfigInited) {
		FcFini();
		agFontconfigInited = 0;
	}
#endif
	AG_MutexDestroy(&agTextLock);
}


AG_ObjectClass agFontClass = {
	"AG_Font",
	sizeof(AG_Font),
	{ 0, 0 },
	AG_Font_Init,
	NULL,		/* reset */
	AG_Font_Destroy,
	NULL,		/* load */
	NULL,		/* save */
	NULL,		/* edit */
};
