/*
 * Copyright (c) 2001-2010 Hypertriton, Inc. <http://hypertriton.com/>
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
 * TextSizeFT() and TextRenderFT_Blended() are based on code from SDL_ttf
 * (http://libsdl.org/projects/SDL_ttf/), placed under BSD license with
 * kind permission from Sam Lantinga.
 */

#include <config/have_freetype.h>
#include <config/ttfdir.h>

#include <core/core.h>
#include <core/config.h>

#ifdef HAVE_FREETYPE
#include "ttf.h"
#endif

#include "window.h"
#include "vbox.h"
#include "box.h"
#include "label.h"
#include "textbox.h"
#include "button.h"
#include "ucombo.h"
#include "numerical.h"
#include "keymap.h"
#include "checkbox.h"

#include "load_xcf.h"

#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "iconmgr.h"
#include "icons.h"
#include "fonts.h"
#include "fonts_data.h"
#include "packedpixel.h"

/* Default fonts */
const char *agDefaultFaceFT = "_agFontVera";
const char *agDefaultFaceBitmap = "_agFontMinimal";

/* Statically compiled fonts */
AG_StaticFont *agBuiltinFonts[] = {
	&agFontVera,
	&agFontMinimal
};
const int agBuiltinFontCount = sizeof(agBuiltinFonts)/sizeof(agBuiltinFonts[0]);

int agTextFontHeight = 0;		/* Default font height (px) */
int agTextFontAscent = 0;		/* Default font ascent (px) */
int agTextFontDescent = 0;		/* Default font descent (px) */
int agTextFontLineSkip = 0;		/* Default font line skip (px) */
int agGlyphGC = 0;			/* Enable glyph garbage collector */
int agFreetypeInited = 0;		/* Initialized Freetype library */
int agRTL = 0;				/* Right-to-left mode */

static AG_TextState states[AG_TEXT_STATES_MAX];
static Uint curState = 0;
AG_TextState *agTextState;

/* #define SYMBOLS */		/* Escape $(x) type symbols */

static const char *agTextMsgTitles[] = {
	N_("Error"),
	N_("Warning"),
	N_("Information")
};

AG_Mutex agTextLock;
static SLIST_HEAD(ag_fontq, ag_font) fonts;
AG_Font *agDefaultFont = NULL;

static AG_Timeout textMsgTo = AG_TIMEOUT_INITIALIZER; /* For AG_TextTmsg() */

/* Load an individual glyph from a bitmap font file. */
static void
LoadBitmapGlyph(AG_Surface *su, const char *lbl, void *p)
{
	AG_Font *font = p;

	if (font->nglyphs == 0) {
		Strlcpy(font->bspec, lbl, sizeof(font->bspec));
	}
	font->bglyphs = Realloc(font->bglyphs,
	                        (font->nglyphs+1)*sizeof(AG_Surface *));
	font->bglyphs[font->nglyphs++] = su;
}

static void
FontDestroy(void *obj)
{
	AG_Font *font = obj;
	int i;

	switch (font->type) {
#ifdef HAVE_FREETYPE
	case AG_FONT_VECTOR:
		AG_TTFCloseFont(font->ttf);
		break;
#endif
	case AG_FONT_BITMAP:
		for (i = 0; i < font->nglyphs; i++) {
			AG_SurfaceFree(font->bglyphs[i]);
		}
		Free(font->bglyphs);
		break;
	default:
		break;
	}
}

static int
GetFontTypeFromSignature(const char *path, enum ag_font_type *pType)
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

/*
 * Search for a given font face/size/flags combination and load it from
 * disk or cache.
 */
AG_Font *
AG_FetchFont(const char *pname, int psize, int pflags)
{
	char path[AG_PATHNAME_MAX];
	char name[AG_OBJECT_NAME_MAX];
	int ptsize = (psize >= 0) ? psize : AG_CfgInt("font.size");
	Uint flags = (pflags >= 0) ? pflags : AG_CfgUint("font.flags");
	enum ag_font_type type;
	AG_StaticFont *builtin;
	AG_Font *font;
	int i;

	if (pname != NULL) {
		Strlcpy(name, pname, sizeof(name));
	} else {
		AG_CopyCfgString("font.face", name, sizeof(name));
	}
	AG_MutexLock(&agTextLock);
	SLIST_FOREACH(font, &fonts, fonts) {
		if (font->size == ptsize &&
		    font->flags == flags &&
		    strcmp(OBJECT(font)->name, name) == 0)
			break;
	}
	if (font != NULL)
		goto out;

	if ((font = TryMalloc(sizeof(AG_Font))) == NULL) {
		AG_MutexUnlock(&agTextLock);
		return (NULL);
	}
	AG_ObjectInit(font, &agFontClass);
	AG_ObjectSetNameS(font, name);

	font->size = ptsize;
	font->flags = flags;
	font->c0 = 0;
	font->c1 = 0;
	font->height = 0;
	font->ascent = 0;
	font->descent = 0;
	font->lineskip = 0;

	if (name[0] == '_') {
		for (i = 0; i < agBuiltinFontCount; i++) {
			if (strcmp(agBuiltinFonts[i]->name, &name[1]) == 0)
				break;
		}
		if (i == agBuiltinFontCount) {
			AG_SetError(_("No such builtin font: %s"), name);
			goto fail;
		}
		builtin = agBuiltinFonts[i];
		type = builtin->type;
	} else {
		if (AG_ConfigFile("font-path", name, NULL, path, sizeof(path))
		    == -1) {
			goto fail;
		}
		builtin = NULL;
		if (GetFontTypeFromSignature(path, &type) == -1)
			goto fail;
	}

#ifdef HAVE_FREETYPE
	if (type == AG_FONT_VECTOR) {
		int tflags = 0;
		AG_TTFFont *ttf;

		if (builtin != NULL) {
			Verbose("Using builtin vector font: %s\n", name);
			if ((font->ttf = ttf = AG_TTFOpenFontFromMemory(
			    builtin->data, builtin->size, ptsize)) == NULL) {
				goto fail;
			}
		} else {
			Verbose("Loading vector font: %s\n", name);
			if ((font->ttf = ttf = AG_TTFOpenFont(path, ptsize))
			    == NULL)
				goto fail;
		}
		if (flags & AG_FONT_BOLD)      { tflags |= TTF_STYLE_BOLD; }
		if (flags & AG_FONT_ITALIC)    { tflags |= TTF_STYLE_ITALIC; }
		if (flags & AG_FONT_UNDERLINE) { tflags |= TTF_STYLE_UNDERLINE;}

		AG_TTFSetFontStyle(ttf, tflags);

		font->type = AG_FONT_VECTOR;
		font->height = ttf->height;
		font->ascent = ttf->ascent;
		font->descent = ttf->descent;
		font->lineskip = ttf->lineskip;
	} else
#endif
	{
		char *s;
		char *msig, *c0, *c1;
		AG_DataSource *ds;
	
		if (builtin != NULL) {
			Verbose("Using builtin bitmap font: %s\n", name);
			ds = AG_OpenConstCore(builtin->data, builtin->size);
		} else {
			Verbose("Loading bitmap font: %s\n", name);
			if ((ds = AG_OpenFile(path, "rb")) == NULL)
				goto fail;
		}

		font->type = AG_FONT_BITMAP;
		font->bglyphs = Malloc(sizeof(AG_Surface *));
		font->nglyphs = 0;

		if (AG_XCFLoad(ds, 0, LoadBitmapGlyph, font) == -1) {
			goto fail;
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
			AG_SetError(_("Missing bitmap fontspec"));
			goto fail;
		}
		font->c0 = (Uint32)strtol(c0, NULL, 10);
		font->c1 = (Uint32)strtol(c1, NULL, 10);
		if (font->nglyphs < (font->c1 - font->c0)) {
			AG_SetError(_("Inconsistent bitmap fontspec"));
			goto fail;
		}

		font->height = font->bglyphs[0]->h;
		font->ascent = font->height;
		font->descent = 0;
		font->lineskip = font->height+2;
	}

	SLIST_INSERT_HEAD(&fonts, font, fonts);
out:
	AG_MutexUnlock(&agTextLock);
	return (font);
fail:
	AG_MutexUnlock(&agTextLock);
/*	AG_ObjectDestroy(font); XXX */
	return (NULL);
}

void
AG_DestroyFont(AG_Font *font)
{
	if (font != agDefaultFont)
		AG_ObjectDestroy(font);
}

void
AG_SetDefaultFont(AG_Font *font)
{
	AG_MutexLock(&agTextLock);
	agDefaultFont = font;
	AG_MutexUnlock(&agTextLock);
}

void
AG_SetRTL(int enable)
{
	AG_MutexLock(&agTextLock);
	agRTL = enable;
	AG_MutexUnlock(&agTextLock);
}

static Uint32
TextTmsgExpire(void *obj, Uint32 ival, void *arg)
{
	AG_Window *win = arg;

	AG_ObjectDetach(win);
	return (0);
}

static void
InitTextState(void)
{
	agTextState->font = agDefaultFont;
	agTextState->color = AG_ColorRGB(255,255,255);
	agTextState->colorBG = AG_ColorRGBA(0,0,0,0);
	agTextState->justify = AG_TEXT_LEFT;
	agTextState->valign = AG_TEXT_TOP;
}

/* Initialize the font engine and configure the default font. */
int
AG_TextInit(void)
{
	AG_Font *font;
	
	AG_MutexInitRecursive(&agTextLock);
	SLIST_INIT(&fonts);

	/* Set the default font search path. */
	if (!AG_CfgDefined("font-path")) {
#if defined(__APPLE__)
# if defined(HAVE_GETPWUID) && defined(HAVE_GETUID)
		char savePath[AG_PATHNAME_MAX];
		char home[AG_PATHNAME_MAX];
		AG_CopyCfgString("save-path", savePath, sizeof(savePath));
		AG_CopyCfgString("home", home, sizeof(home));
		AG_SetCfgString("font-path",
		    "%s/fonts:%s:%s/Library/Fonts:/Library/Fonts:/System/Library/Fonts",
		    savePath, TTFDIR, home);
# else
		char savePath[AG_PATHNAME_MAX];
		AG_CopyCfgString("save-path", savePath, sizeof(savePath));
		AG_SetCfgString("font-path",
		    "%s/fonts:%s:/Library/Fonts:/System/Library/Fonts",
		    savePath, TTFDIR);
# endif
#elif defined(_WIN32)
		AG_SetCfgString("font-path", "fonts:.");
#else
		char savePath[AG_PATHNAME_MAX];
		AG_CopyCfgString("save-path", savePath, sizeof(savePath));
		AG_SetCfgString("font-path", "%s/fonts:%s", savePath, TTFDIR);
#endif
	}
	
	/* Initialize FreeType if available. */
#ifdef HAVE_FREETYPE
	if (AG_TTFInit() == 0) {
		agFreetypeInited = 1;
	} else {
		AG_Verbose("Failed to initialize FreeType (%s); falling back "
		           "to monospace font engine", AG_GetError());
	}
#endif

	/* Load the default font. */
	if (!AG_CfgDefined("font.face")) {
		AG_SetCfgString("font.face", agFreetypeInited ?
		                             agDefaultFaceFT :
					     agDefaultFaceBitmap);
	}
	if (!AG_CfgDefined("font.size")) {
		AG_SetCfgInt("font.size", 10);
	}
	if (!AG_CfgDefined("font.flags")) {
		AG_SetCfgUint("font.flags", 0);
	}
	if ((font = AG_FetchFont(NULL, -1, -1)) == NULL) {
		AG_SetError("Failed to load default font: %s", AG_GetError());
		goto fail;
	}
	agDefaultFont = font;
	agTextFontHeight = font->height;
	agTextFontAscent = font->ascent;
	agTextFontDescent = font->descent;
	agTextFontLineSkip = font->lineskip;

	/* Initialize the state engine and cache. */
	curState = 0;
	agTextState = &states[0];
	InitTextState();
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
AG_TextDestroy(void)
{
	AG_Font *font, *nextfont;

	for (font = SLIST_FIRST(&fonts);
	     font != SLIST_END(&fonts);
	     font = nextfont) {
		nextfont = SLIST_NEXT(font, fonts);
		AG_ObjectDestroy(font);
		if (font == agDefaultFont)
			agDefaultFont = NULL;
	}
	SLIST_INIT(&fonts);

#ifdef HAVE_FREETYPE
	if (agFreetypeInited) {
		AG_TTFDestroy();
		agFreetypeInited = 0;
	}
#endif
	AG_MutexDestroy(&agTextLock);
}

/* Initialize the glyph cache. */
void
AG_TextInitGlyphCache(AG_Driver *drv)
{
	Uint i;

	drv->glyphCache = Malloc(AG_GLYPH_NBUCKETS*sizeof(AG_GlyphCache));
	for (i = 0; i < AG_GLYPH_NBUCKETS; i++)
		SLIST_INIT(&drv->glyphCache[i].glyphs);
}

/* Clear the glyph cache. */
void
AG_TextClearGlyphCache(AG_Driver *drv)
{
	int i;
	AG_Glyph *gl, *ngl;

	for (i = 0; i < AG_GLYPH_NBUCKETS; i++) {
		for (gl = SLIST_FIRST(&drv->glyphCache[i].glyphs);
		     gl != SLIST_END(&drv->glyphCache[i].glyphs);
		     gl = ngl) {
			ngl = SLIST_NEXT(gl, glyphs);
			AG_SurfaceFree(gl->su);
			Free(gl);
		}
		SLIST_INIT(&drv->glyphCache[i].glyphs);
	}
}

/* Render a glyph following a cache miss; called from AG_TextRenderGlyph(). */
AG_Glyph *
AG_TextRenderGlyphMiss(AG_Driver *drv, Uint32 ch)
{
	AG_Glyph *gl;
	Uint32 ucs[2];

	gl = Malloc(sizeof(AG_Glyph));
	gl->font = agTextState->font;
	gl->color = agTextState->color;
	gl->ch = ch;
	ucs[0] = ch;
	ucs[1] = '\0';
	gl->su = AG_TextRenderUCS4(ucs);

	switch (agTextState->font->type) {
#ifdef HAVE_FREETYPE
	case AG_FONT_VECTOR:
		{
			AG_TTFGlyph *gt;

			if (AG_TTFFindGlyph(agTextState->font->ttf, ch,
			    TTF_CACHED_METRICS|TTF_CACHED_BITMAP) == 0) {
				gt = ((AG_TTFFont *)agTextState->font->ttf)->current;
				gl->advance = gt->advance;
			} else {
				gl->advance = gl->su->w;
			}
		}
		break;
#endif
	case AG_FONT_BITMAP:
		gl->advance = gl->su->w;
		break;
	default:
		break;
	}
	AGDRIVER_CLASS(drv)->updateGlyph(drv, gl);
	return (gl);
}

/* Save the current text rendering state. */
void
AG_PushTextState(void)
{
	AG_MutexLock(&agTextLock);
	if ((curState+1) >= AG_TEXT_STATES_MAX) {
		AG_FatalError("Text state stack overflow");
	}
	agTextState = &states[++curState];
	InitTextState();
	AG_MutexUnlock(&agTextLock);
}

/* Restore the previous rendering state. */
void
AG_PopTextState(void)
{
	AG_MutexLock(&agTextLock);
	if (curState == 0) {
		AG_FatalError("No text state to pop");
	}
	agTextState = &states[--curState];
	AG_MutexUnlock(&agTextLock);
}

/* Select the font face to use in rendering text. */
AG_Font *
AG_TextFontLookup(const char *face, int size, Uint flags)
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
AG_TextFontPts(int pts)
{
	AG_Font *font, *newFont;

	AG_MutexLock(&agTextLock);
	font = agTextState->font;
	newFont = AG_FetchFont(OBJECT(font)->name, pts, font->flags);
	if (newFont == NULL) {
		goto fail;
	}
	agTextState->font = newFont;
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
	AG_Font *font, *newFont;

	AG_MutexLock(&agTextLock);
	font = agTextState->font;
	newFont = AG_FetchFont(OBJECT(font)->name,
	    font->size*pct/100,
	    font->flags);
	if (newFont == NULL) {
		goto fail;
	}
	agTextState->font = newFont;
	AG_MutexUnlock(&agTextLock);
	return (agTextState->font);
fail:
	AG_MutexUnlock(&agTextLock);
	return (NULL);
}

/* Varargs variant of TextRender(). */
AG_Surface *
AG_TextRenderf(const char *fmt, ...)
{
	char *text;
	va_list args;
	AG_Surface *su;

	va_start(args, fmt);
	Vasprintf(&text, fmt, args);
	va_end(args);
	su = AG_TextRender(text);
	Free(text);
	return (su);
}
#ifdef SYMBOLS
static __inline__ AG_Surface *
GetSymbolSurface(Uint32 ch)
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
InitMetrics(AG_TextMetrics *tm)
{
	tm->w = 0;
	tm->h = 0;
	tm->wLines = NULL;
	tm->nLines = 0;
}

static __inline__ void
FreeMetrics(AG_TextMetrics *tm)
{
	Free(tm->wLines);
}

#ifdef HAVE_FREETYPE
/*
 * Compute the rendered size of UCS-4 text with a FreeType font. If the
 * string is multiline and nLines is non-NULL, the width of individual lines
 * is returned into wLines, and the number of lines into nLines.
 */
static void
TextSizeFT(const Uint32 *ucs, AG_TextMetrics *tm, int extended)
{
	AG_Font *font = agTextState->font;
	AG_TTFFont *ftFont = font->ttf;
	AG_TTFGlyph *glyph;
	const Uint32 *ch;
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
		if (AG_TTFFindGlyph(ftFont, *ch, TTF_CACHED_METRICS) != 0) {
			continue;
		}
		glyph = ftFont->current;

		z = x + glyph->minx;
		if (xMin > z) { xMin = z; }
		if (xMinLine > z) { xMinLine = z; }

		if (ftFont->style & TTF_STYLE_BOLD) {
			x += ftFont->glyph_overhang;
		}
		z = x + MAX(glyph->advance,glyph->maxx);
		if (xMax < z) { xMax = z; }
		if (xMaxLine < z) { xMaxLine = z; }
		x += glyph->advance;

		if (glyph->miny < yMin) { yMin = glyph->miny; }
		if (glyph->maxy > yMax) { yMax = glyph->maxy; }
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
}

# ifdef SYMBOLS

static int
TextRenderSymbol(Uint ch, AG_Surface *su, int x, int y)
{
	AG_Surface *sym;
	int row;

	if ((sym = GetSymbolSurface(ch)) == NULL) {
		return (0);
	}
	for (row = 0; row < sym->h; row++) {
		Uint8 *dst = (Uint8 *)su->pixels + (y+row)*su->pitch +
		                                   (x+2);
		Uint8 *src = (Uint8 *)sym->pixels + row*sym->pitch;
		int col;

		for (col = 0; col < sym->w; col++) {
			if (AG_GET_PIXEL(sym,src) != sym->format->colorkey) {
				*dst = 1;
			}
			src += sym->format->BytesPerPixel;
			dst++;
		}
	}
	return (sym->w + 4);
}

static int
TextRenderSymbol_Blended(Uint ch, AG_Surface *su, int x, int y)
{
	AG_Surface *sym;
	Uint32 alpha;
	int row;

	if ((sym = GetSymbolSurface(ch)) == NULL) {
		return (0);
	}
	for (row = 0; row < sym->h; row++) {
		Uint8 *dst = (Uint8 *)su->pixels + (y+row)*(su->pitch/4) + (x+2);
		Uint8 *src = (Uint8 *)sym->pixels + row*sym->pitch;
		int col;

		for (col = 0; col < sym->w; col++) {
			alpha = *src;
			if (AG_GET_PIXEL(sym,src) != sym->format->colorkey) {
				dst[0] = 0xff;
				dst[1] = 0xff;
				dst[2] = 0xff;
			}
			src += sym->format->BytesPerPixel;
			dst += 3;
		}
	}
	return (sym->w + 4);
}
# endif /* SYMBOLS */

/* Underline rendered text. */
/* XXX does not handle multiline/alignment properly */
static void
TextRenderFT_Blended_Underline(AG_TTFFont *ftFont, AG_Surface *su, int nLines)
{
	AG_Color C = agTextState->color;
	Uint32 pixel;
	Uint8 *pDst;
	int x, y, line;

	pixel = AG_MapPixelRGBA(su->format, C.r, C.g, C.b, 255);
	for (line = 0; line < nLines; line++) {
		y = ftFont->ascent - ftFont->underline_offset - 1;
		y *= (line+1);
		if (y >= su->h) {
			y = (su->h - 1) - ftFont->underline_height;
		}
		pDst = (Uint8 *)su->pixels + y*su->pitch;
		for (y = 0; y < ftFont->underline_height; y++) {
			for (x = 0; x < su->w; x++) {
				AG_PACKEDPIXEL_PUT(su->format->BytesPerPixel,
				    pDst, pixel);
				pDst += su->format->BytesPerPixel;
			}
		}
	}
}

static AG_Surface *
TextRenderFT_Blended(const Uint32 *ucs)
{
	AG_TextMetrics tm;
	AG_Font *font = agTextState->font;
	AG_Color C = agTextState->color;
	AG_TTFFont *ftFont = font->ttf;
	AG_TTFGlyph *glyph;
	const Uint32 *ch;
	int xStart, yStart;
	int line;
	AG_Surface *su;
	Uint32 pixel;
	Uint8 *src, *dst;
	int x, y;
	FT_UInt prev_index = 0;
	int w;

	InitMetrics(&tm);
	TextSizeFT(ucs, &tm, 1);
	if (tm.w <= 0 || tm.h <= 0)
		goto empty;

	if ((su = AG_SurfaceStdRGBA(tm.w, tm.h)) == NULL) {
		Verbose("TextRenderFT_Blended: %s\n", AG_GetError());
		goto empty;
	}

	/* Load and render each character */
 	line = 0;
 	xStart = (tm.nLines > 1) ? AG_TextJustifyOffset(tm.w, tm.wLines[0]) : 0;
 	yStart = 0;

	AG_FillRect(su, NULL, agTextState->colorBG);
	if (agTextState->colorBG.a == AG_ALPHA_TRANSPARENT)
		AG_SurfaceSetColorKey(su, AG_SRCCOLORKEY,
		    AG_MapColorRGBA(su->format, agTextState->colorBG));

	for (ch = &ucs[0]; *ch != '\0'; ch++) {
		if (*ch == '\n') {
			yStart += font->lineskip;
			xStart = AG_TextJustifyOffset(tm.w, tm.wLines[++line]);
			continue;
		}
#ifdef SYMBOLS
		if (ch[0] == '$' && agTextSymbols &&
		    ch[1] == '(' && ch[2] != '\0' && ch[3] == ')') {
			xStart += TextRenderSymbol_Blended(ch[2], su, xStart,
			    yStart);
			ch += 3;
			continue;
		}
#endif
		if (AG_TTFFindGlyph(ftFont, *ch,
		    TTF_CACHED_METRICS|TTF_CACHED_PIXMAP)) {
			AG_SurfaceFree(su);
			goto empty;
		}
		glyph = ftFont->current;
		/*
		 * Ensure the width of the pixmap is correct. On some cases,
		 * freetype may report a larger pixmap than possible.
		 * XXX is this test necessary?
		 */
		w = glyph->pixmap.width;
		if (w > glyph->maxx - glyph->minx) {
			w = glyph->maxx - glyph->minx;
		}
		if (FT_HAS_KERNING(ftFont->face) && prev_index &&
		    glyph->index) {
			FT_Vector delta; 

			FT_Get_Kerning(ftFont->face, prev_index, glyph->index,
			    ft_kerning_default, &delta); 
			xStart += delta.x >> 6;
		}
		
		/* Prevent texture wrapping with first glyph. */
		if ((ch == &ucs[0]) && (glyph->minx < 0))
			xStart -= glyph->minx;

		for (y = 0; y < glyph->pixmap.rows; y++) {
			if (y+glyph->yoffset < 0 ||
			    y+glyph->yoffset >= su->h) {
				continue;
			}
			dst = (Uint8 *)su->pixels +
			    (yStart + y + glyph->yoffset)*su->pitch +
			    (xStart + glyph->minx)*su->format->BytesPerPixel;

			/* Adjust src for pixmaps to account for pitch. */
			src = (Uint8 *)(glyph->pixmap.buffer +
			                glyph->pixmap.pitch*y);
	
			if (agTextState->colorBG.a == AG_ALPHA_TRANSPARENT) {
				for (x = 0; x < w; x++) {
					Uint32 alpha = *src++;

					pixel = AG_MapPixelRGBA(su->format,
					    C.r, C.g, C.b, alpha);
					AG_PACKEDPIXEL_PUT(
					    su->format->BytesPerPixel,
					    dst, pixel);
					dst += su->format->BytesPerPixel;
				}
			} else {
				for (x = 0; x < w; x++) {
					Uint32 alpha = *src++;

					AG_BLEND_RGBA(su, dst,
					    C.r, C.g, C.b, alpha,
					    AG_ALPHA_SRC);
					dst += su->format->BytesPerPixel;
				}
			}
		}
		xStart += glyph->advance;
		if (ftFont->style & TTF_STYLE_BOLD) {
			xStart += ftFont->glyph_overhang;
		}
		prev_index = glyph->index;
	}
	if (ftFont->style & TTF_STYLE_UNDERLINE) {
		TextRenderFT_Blended_Underline(ftFont, su, tm.nLines);
	}
	FreeMetrics(&tm);
	return (su);
empty:
	FreeMetrics(&tm);
	return AG_SurfaceEmpty();
}

#endif /* HAVE_FREETYPE */

static __inline__ AG_Surface *
GetBitmapGlyph(AG_Font *font, Uint32 c)
{
	if ((font->flags & AG_FONT_UPPERCASE) &&
	    (isalpha(c) && islower(c))) {
		c = toupper(c);
	}
	if (c < font->c0 || c > font->c1) {
		return (agTextState->font->bglyphs[0]);
	}
	return (font->bglyphs[c - font->c0 + 1]);
}

/* Compute the rendered size of UCS-4 text with a bitmap font. */
static __inline__ void
TextSizeBitmap(const Uint32 *ucs, AG_TextMetrics *tm, int extended)
{
	AG_Font *font = agTextState->font;
	const Uint32 *c;
	AG_Surface *sGlyph;
	int wLine = 0;

	for (c = &ucs[0]; *c != '\0'; c++) {
		sGlyph = GetBitmapGlyph(font, *c);
		if (*c == '\n') {
			if (extended) {
				tm->wLines = Realloc(tm->wLines,
				    (tm->nLines+2)*sizeof(Uint));
				tm->wLines[tm->nLines++] = wLine;
				wLine = 0;
			}
			tm->h += agTextState->font->lineskip;
			continue;
		}
		wLine += sGlyph->w;
		tm->w += sGlyph->w;
		tm->h = MAX(tm->h, sGlyph->h);
	}
	if (*c != '\n' && extended) {
		if (tm->nLines > 0) {
			tm->wLines = Realloc(tm->wLines,
			    (tm->nLines+2)*sizeof(Uint));
			tm->wLines[tm->nLines] = wLine;
		}
		tm->nLines++;
	}
}

/* Render UCS-4 text to a new surface using a bitmap font. */
/* TODO: blend colors */
static AG_Surface *
TextRenderBitmap(const Uint32 *ucs)
{
	AG_TextMetrics tm;
	AG_Font *font = agTextState->font;
	AG_Rect rd;
	int line;
	const Uint32 *c;
	AG_Surface *sGlyph, *su;

	InitMetrics(&tm);
	TextSizeBitmap(ucs, &tm, 1);

	if ((su = AG_SurfaceStdRGBA(tm.w, tm.h)) == NULL) {
		AG_FatalError(NULL);
	}
	AG_FillRect(su, NULL, AG_ColorRGBA(0,0,0,0));

	line = 0;
	rd.x = (tm.nLines > 1) ? AG_TextJustifyOffset(tm.w, tm.wLines[0]) : 0;
	rd.y = 0;
	
	for (c = &ucs[0]; *c != '\0'; c++) {
		if (*c == '\n') {
			rd.y += font->lineskip;
			rd.x = AG_TextJustifyOffset(tm.w, tm.wLines[++line]);
			continue;
		}
		sGlyph = GetBitmapGlyph(font, *c);
		if (*c != ' ') {
			AG_SurfaceBlit(sGlyph, NULL, su, rd.x, rd.y);
		}
		rd.x += sGlyph->w;
	}
	AG_SurfaceSetColorKey(su, AG_SRCCOLORKEY,
	    AG_MapPixelRGBA(su->format, 0,0,0,0));
	AG_SurfaceSetAlpha(su, AG_SRCALPHA, font->bglyphs[0]->format->alpha);

	FreeMetrics(&tm);
	return (su);
}

/* Render an UCS-4 text string onto a new 8-bit surface. */
AG_Surface *
AG_TextRenderUCS4(const Uint32 *text)
{
	switch (agTextState->font->type) {
#ifdef HAVE_FREETYPE
	case AG_FONT_VECTOR:
		return TextRenderFT_Blended(text);
#endif
	case AG_FONT_BITMAP:
		return TextRenderBitmap(text);
	default:
		return AG_SurfaceEmpty();
	}
}

/* Return the rendered size in pixels of a UCS4-encoded string. */
void
AG_TextSizeUCS4(const Uint32 *ucs4, int *w, int *h)
{
	AG_TextMetrics tm;

	InitMetrics(&tm);
	switch (agTextState->font->type) {
#ifdef HAVE_FREETYPE
	case AG_FONT_VECTOR:
		TextSizeFT(ucs4, &tm, 0);
		break;
#endif
	case AG_FONT_BITMAP:
		TextSizeBitmap(ucs4, &tm, 0);
		break;
	default:
		break;
	}
	if (w != NULL) { *w = tm.w; }
	if (h != NULL) { *h = tm.h; }
	FreeMetrics(&tm);
}

/*
 * Return the rendered size in pixels of a UCS4-encoded string, along with
 * a line count and the width of each line in an array.
 */
void
AG_TextSizeMultiUCS4(const Uint32 *ucs4, int *w, int *h, Uint **wLines,
    Uint *nLines)
{
	AG_TextMetrics tm;

	InitMetrics(&tm);
	switch (agTextState->font->type) {
#ifdef HAVE_FREETYPE
	case AG_FONT_VECTOR:
		TextSizeFT(ucs4, &tm, 1);
		break;
#endif
	case AG_FONT_BITMAP:
		TextSizeBitmap(ucs4, &tm, 1);
		break;
	default:
		break;
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

/* Return the rendered size in pixels of a text string. */
void
AG_TextSize(const char *text, int *w, int *h)
{
	Uint32 *ucs4;

	if (text == NULL || text[0] == '\0') {
		if (w != NULL) { *w = 0; }
		if (h != NULL) { *h = 0; }
		return;
	}
	ucs4 = AG_ImportUnicode(AG_UNICODE_FROM_UTF8, text, 0);
	AG_TextSizeUCS4(ucs4, w, h);
	Free(ucs4);
}

/*
 * Return the rendered size in pixels of a text string, along with a line
 * count and the width of each line in an array.
 */
void
AG_TextSizeMulti(const char *text, int *w, int *h, Uint **wLines, Uint *nLines)
{
	Uint32 *ucs4;

	ucs4 = AG_ImportUnicode(AG_UNICODE_FROM_UTF8, text, 0);
	AG_TextSizeMultiUCS4(ucs4, w, h, wLines, nLines);
	Free(ucs4);
}

/*
 * Parse a command-line font specification and set the default font.
 * The format is <face>,<size>,<flags>. Acceptable flags include 'b'
 * (bold), 'i' (italic) and 'U' (uppercase).
 */
void
AG_TextParseFontSpec(const char *fontspec)
{
	char buf[128];
	char *fs, *s, *c;

	Strlcpy(buf, fontspec, sizeof(buf));
	fs = &buf[0];

	if ((s = AG_Strsep(&fs, ":,/")) != NULL &&
	    s[0] != '\0') {
		AG_SetCfgString("font.face", s);
	}
	if ((s = AG_Strsep(&fs, ":,/")) != NULL &&
	    s[0] != '\0') {
		AG_SetCfgInt("font.size", atoi(s));
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
		AG_SetCfgUint("font.flags", flags);
	}
}

/*
 * Canned dialogs.
 */

/* Display a message (format string). */
void
AG_TextMsg(enum ag_text_msg_title title, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);
	AG_TextMsgS(title, s);
}

/* Display a message (C string). */
void
AG_TextMsgS(enum ag_text_msg_title title, const char *s)
{
	AG_Window *win;
	AG_VBox *vb;
	AG_Button *btnOK;

	win = AG_WindowNew(AG_WINDOW_DIALOG|AG_WINDOW_MODAL|AG_WINDOW_NORESIZE|
	                   AG_WINDOW_NOCLOSE|AG_WINDOW_NOMINIMIZE|
			   AG_WINDOW_NOMAXIMIZE|AG_WINDOW_NOBORDERS);
	AG_WindowSetCaptionS(win, _(agTextMsgTitles[title]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, 0);
	AG_LabelNewS(vb, 0, s);

	vb = AG_VBoxNew(win, AG_VBOX_HOMOGENOUS|AG_VBOX_HFILL|AG_VBOX_VFILL);
	btnOK = AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win));

	AG_WidgetFocus(btnOK);
	AG_WindowShow(win);
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
}

/* Display a message for a given period of time (C string). */
void
AG_TextTmsgS(enum ag_text_msg_title title, Uint32 expire, const char *s)
{
	AG_Window *win;
	AG_VBox *vb;

	win = AG_WindowNew(AG_WINDOW_DIALOG|AG_WINDOW_NORESIZE|AG_WINDOW_NOCLOSE|
	                   AG_WINDOW_NOMINIMIZE|AG_WINDOW_NOMAXIMIZE|
			   AG_WINDOW_NOBORDERS);
	AG_WindowSetCaptionS(win, _(agTextMsgTitles[title]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, 0);
	AG_LabelNewS(vb, 0, s);
	AG_WindowShow(win);

	AG_LockTimeouts(NULL);
	if (AG_TimeoutIsScheduled(NULL, &textMsgTo)) {
		AG_ObjectDetach((AG_Window *)textMsgTo.arg);
		AG_DelTimeout(NULL, &textMsgTo);
	}
	AG_UnlockTimeouts(NULL);

	AG_SetTimeout(&textMsgTo, TextTmsgExpire, win, 0);
	AG_ScheduleTimeout(NULL, &textMsgTo, expire);
}

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
}

/*
 * Display an informational message with a "Don't tell me again" option.
 * The user preference is preserved in a persistent table. Unlike warnings,
 * the dialog window is not modal (C string).
 */
void
AG_TextInfoS(const char *key, const char *s)
{
	char disableSw[64];
	AG_Window *win;
	AG_VBox *vb;
	AG_Checkbox *cb;
	AG_Button *btnOK;
	AG_Variable *Vdisable;
	
	Strlcpy(disableSw, "info.", sizeof(disableSw));
	Strlcat(disableSw, key, sizeof(disableSw));

	if (AG_Defined(agConfig,disableSw) &&
	    AG_GetInt(agConfig,disableSw) == 1)
		return;

	win = AG_WindowNew(AG_WINDOW_DIALOG|AG_WINDOW_NORESIZE|AG_WINDOW_NOCLOSE|
	                   AG_WINDOW_NOMINIMIZE|AG_WINDOW_NOMAXIMIZE|
			   AG_WINDOW_NOBORDERS);
	AG_WindowSetCaptionS(win, _(agTextMsgTitles[AG_MSG_INFO]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, 0);
	AG_LabelNewS(vb, 0, s);

	vb = AG_VBoxNew(win, AG_VBOX_HOMOGENOUS|AG_VBOX_HFILL|AG_VBOX_VFILL);
	btnOK = AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win));

	cb = AG_CheckboxNewS(win, AG_CHECKBOX_HFILL, _("Don't tell me again"));
	Vdisable = AG_SetInt(agConfig,disableSw,0);
	AG_BindInt(cb, "state", &Vdisable->data.i);

	AG_WidgetFocus(btnOK);
	AG_WindowShow(win);
}

/*
 * Display a warning message with a "Don't tell me again" option.
 * The user preference is preserved in a persistent table
 * (format string).
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
}

/*
 * Display a warning message with a "Don't tell me again" option.
 * The user preference is preserved in a persistent table
 * (C string).
 */
void
AG_TextWarningS(const char *key, const char *s)
{
	char disableSw[64];
	AG_Window *win;
	AG_VBox *vb;
	AG_Checkbox *cb;
	AG_Button *btnOK;
	AG_Variable *Vdisable;
	
	Strlcpy(disableSw, "warn.", sizeof(disableSw));
	Strlcat(disableSw, key, sizeof(disableSw));

	if (AG_Defined(agConfig,disableSw) &&
	    AG_GetInt(agConfig,disableSw) == 1)
		return;

	win = AG_WindowNew(AG_WINDOW_DIALOG|AG_WINDOW_MODAL|AG_WINDOW_NORESIZE|
	                   AG_WINDOW_NOCLOSE|AG_WINDOW_NOMINIMIZE|
			   AG_WINDOW_NOMAXIMIZE|AG_WINDOW_NOBORDERS);
	AG_WindowSetCaptionS(win, _(agTextMsgTitles[AG_MSG_WARNING]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, 0);
	AG_LabelNewS(vb, 0, s);

	vb = AG_VBoxNew(win, AG_VBOX_HOMOGENOUS|AG_VBOX_HFILL|AG_VBOX_VFILL);
	btnOK = AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win));

	cb = AG_CheckboxNewS(win, AG_CHECKBOX_HFILL, _("Don't tell me again"));
	Vdisable = AG_SetInt(agConfig,disableSw,0);
	AG_BindInt(cb, "state", &Vdisable->data.i);

	AG_WidgetFocus(btnOK);
	AG_WindowShow(win);
}

/* Display an error message (format string). */
void
AG_TextError(const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_TextErrorS(s);
}

/* Display an error message (C string). */
void
AG_TextErrorS(const char *s)
{
	AG_Window *win;
	AG_VBox *vb;
	AG_Button *btnOK;

	win = AG_WindowNew(AG_WINDOW_DIALOG|AG_WINDOW_MODAL|AG_WINDOW_NORESIZE|
	                   AG_WINDOW_NOCLOSE|AG_WINDOW_NOMINIMIZE|
			   AG_WINDOW_NOMAXIMIZE|AG_WINDOW_NOBORDERS);
	AG_WindowSetCaptionS(win, _(agTextMsgTitles[AG_MSG_ERROR]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, 0);
	AG_LabelNewS(vb, 0, s);

	vb = AG_VBoxNew(win, AG_VBOX_HOMOGENOUS|AG_VBOX_HFILL|AG_VBOX_VFILL);
	btnOK = AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win));

	AG_WidgetFocus(btnOK);
	AG_WindowShow(win);
}

/* Prompt the user with a choice of options. */
AG_Window *
AG_TextPromptOptions(AG_Button **bOpts, Uint nbOpts, const char *fmt, ...)
{
	char text[AG_LABEL_MAX];
	AG_Window *win;
	AG_Box *bo;
	va_list ap;
	Uint i;

	va_start(ap, fmt);
	Vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);

	win = AG_WindowNew(AG_WINDOW_DIALOG|AG_WINDOW_MODAL|AG_WINDOW_NORESIZE|
	                   AG_WINDOW_NOTITLE);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_WindowSetSpacing(win, 8);

	AG_LabelNewS(win, 0, text);

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	for (i = 0; i < nbOpts; i++) {
		bOpts[i] = AG_ButtonNewS(bo, 0, "XXXXXXXXXXX");
	}
	AG_WindowShow(win);
	return (win);
}

/* Prompt the user for a floating-point value. */
void
AG_TextEditFloat(double *fp, double min, double max, const char *unit,
    const char *format, ...)
{
	char msg[AG_LABEL_MAX];
	AG_Window *win;
	AG_VBox *vb;
	va_list args;
	AG_Numerical *num;

	va_start(args, format);
	Vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);

	win = AG_WindowNew(AG_WINDOW_DIALOG|AG_WINDOW_MODAL|AG_WINDOW_NOVRESIZE);
	AG_WindowSetCaptionS(win, _("Enter real number"));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, AG_VBOX_HFILL);
	AG_LabelNewS(vb, 0, msg);
	
	vb = AG_VBoxNew(win, AG_VBOX_HFILL);
	{
		num = AG_NumericalNewDblR(vb, 0, unit, _("Number: "),
		    fp, min, max);
		AG_SetEvent(num, "numerical-return", AGWINDETACH(win));
	}
	
	vb = AG_VBoxNew(win, AG_VBOX_HOMOGENOUS|AG_VBOX_HFILL|AG_VBOX_VFILL);
	AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win));

	/* TODO test type */

	AG_WidgetFocus(num);
	AG_WindowShow(win);
}

/* Create a dialog to edit a string value. */
void
AG_TextEditString(char *sp, size_t len, const char *msgfmt, ...)
{
	char msg[AG_LABEL_MAX];
	AG_Window *win;
	AG_VBox *vb;
	va_list args;
	AG_Textbox *tb;

	va_start(args, msgfmt);
	Vsnprintf(msg, sizeof(msg), msgfmt, args);
	va_end(args);

	win = AG_WindowNew(AG_WINDOW_DIALOG|AG_WINDOW_MODAL|AG_WINDOW_NOVRESIZE);
	AG_WindowSetCaptionS(win, _("Edit string"));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, AG_VBOX_HFILL);
	AG_LabelNewS(vb, 0, msg);
	
	vb = AG_VBoxNew(win, AG_VBOX_HFILL);
	tb = AG_TextboxNewS(vb, AG_TEXTBOX_STATIC, NULL);
	AG_ExpandHoriz(tb);
	AG_TextboxBindUTF8(tb, sp, len);
	AG_SetEvent(tb, "textbox-return", AGWINDETACH(win));

	vb = AG_VBoxNew(win, AG_VBOX_HOMOGENOUS|AG_VBOX_HFILL|AG_VBOX_VFILL);
	AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win));

	AG_WidgetFocus(tb);
	AG_WindowShow(win);
}

/* Prompt the user for a string. */
void
AG_TextPromptString(const char *prompt, void (*ok_fn)(AG_Event *),
    const char *fmt, ...)
{
	AG_Window *win;
	AG_Box *bo;
	AG_Button *btnOK;
	AG_Textbox *tb;
	AG_Event *ev;

	win = AG_WindowNew(AG_WINDOW_DIALOG|AG_WINDOW_MODAL|AG_WINDOW_NOVRESIZE|
	                   AG_WINDOW_NOTITLE);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_WindowSetSpacing(win, 8);

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);
	AG_LabelNewS(bo, 0, prompt);
	
	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);
	{
		tb = AG_TextboxNewS(bo, AG_TEXTBOX_STATIC, NULL);
		AG_ExpandHoriz(tb);
		ev = AG_SetEvent(tb, "textbox-return", ok_fn, NULL);
		AG_EVENT_GET_ARGS(ev, fmt)
		AG_EVENT_INS_VAL(ev, AG_VARIABLE_STRING, "string", s,
		    &tb->ed->string[0]);
		AG_AddEvent(tb, "textbox-return", AGWINDETACH(win));
	}

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	{
		btnOK = AG_ButtonNewS(bo, 0, _("Ok"));
		ev = AG_SetEvent(btnOK, "button-pushed", ok_fn, NULL);
		AG_EVENT_GET_ARGS(ev, fmt);
		AG_EVENT_INS_VAL(ev, AG_VARIABLE_STRING, "string", s,
		    &tb->ed->string[0]);
		AG_AddEvent(btnOK, "button-pushed", AGWINDETACH(win));

		AG_ButtonNewFn(bo, 0, _("Cancel"), AGWINDETACH(win));
	}

	AG_WidgetFocus(tb);
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
		*x = (wArea + lPad + rPad)/2 - wText/2;
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
		*y = (hArea + tPad + bPad)/2 - hText/2;
		break;
	case AG_TEXT_BOTTOM:
		*y = hArea - bPad - wText;
		break;
	}
}

AG_ObjectClass agFontClass = {
	"AG_Font",
	sizeof(AG_Font),
	{ 0, 0 },
	NULL,		/* init */
	NULL,		/* free */
	FontDestroy,
	NULL,		/* load */
	NULL,		/* save */
	NULL,		/* edit */
};
