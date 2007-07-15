/*
 * Copyright (c) 2001-2007 Hypertriton, Inc. <http://www.hypertriton.com/>
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

#include <config/have_freetype.h>
#include <config/have_opengl.h>
#include <config/utf8.h>

#include <core/core.h>
#include <core/view.h>
#include <core/config.h>

#ifdef HAVE_FREETYPE
#include <core/loaders/ttf.h>
#endif
#include <core/loaders/xcf.h>

#include "window.h"
#include "vbox.h"
#include "box.h"
#include "label.h"
#include "textbox.h"
#include "button.h"
#include "ucombo.h"
#include "fspinbutton.h"
#include "keycodes.h"
#include "unicode.h"

#include <string.h>
#include <stdarg.h>
#include <ctype.h>

const AG_ObjectOps agFontOps = {
	"AG_Font",
	sizeof(AG_Font),
	{ 0, 0 },
	NULL,		/* init */
	NULL,		/* reinit */
	AG_FontDestroy,
	NULL,		/* load */
	NULL,		/* save */
	NULL,		/* edit */
};

int agTextComposition = 1;		/* Built-in input composition */
int agTextBidi = 0;			/* Bidirectionnal text display */
int agTextFontHeight = 0;		/* Default font height (px) */
int agTextFontAscent = 0;		/* Default font ascent (px) */
int agTextFontDescent = 0;		/* Default font descent (px) */
int agTextFontLineSkip = 0;		/* Default font line skip (px) */
int agTextTabWidth = 40;		/* Tab width (px) */
int agTextBlinkRate = 250;		/* Cursor blink rate (ms) */
int agTextSymbols = 1;			/* Process special symbols in text */
int agFreetype = 0;			/* Use Freetype font engine */

static AG_TextState states[AG_TEXT_STATES_MAX], *state;
static Uint curState = 0;

#define GLYPH_NBUCKETS 1024
#define SYMBOLS				/* Allow $(x) type symbols */

static const char *agTextMsgTitles[] = {
	N_("Error"),
	N_("Warning"),
	N_("Information")
};

AG_Mutex agTextLock = AG_MUTEX_INITIALIZER;
static SLIST_HEAD(ag_fontq, ag_font) fonts = SLIST_HEAD_INITIALIZER(&fonts);
AG_Font *agDefaultFont = NULL;

static struct {
	SLIST_HEAD(, ag_glyph) glyphs;
} agGlyphCache[GLYPH_NBUCKETS+1];

static AG_Timeout text_timeout;		/* Timer for AG_TextTmsg() */

/* Load an individual glyph from a bitmap font file. */
static void
AG_LoadBitmapGlyph(SDL_Surface *su, const char *lbl, void *p)
{
	AG_Font *font = p;

	if (font->nglyphs == 0) {
		strlcpy(font->bspec, lbl, sizeof(font->bspec));
	}
	font->bglyphs = Realloc(font->bglyphs,
	                        (font->nglyphs+1)*sizeof(SDL_Surface *));
	font->bglyphs[font->nglyphs++] = su;
}

void
AG_FontDestroy(void *p)
{
	AG_Font *font = p;
	int i;

	if (!agFreetype) {
		for (i = 0; i < font->nglyphs; i++) {
			SDL_FreeSurface(font->bglyphs[i]);
		}
		Free(font->bglyphs, M_TEXT);
	}
}

/*
 * Search for a given font face/size/flags combination and load it from
 * disk or cache.
 */
AG_Font *
AG_FetchFont(const char *pname, int psize, int pflags)
{
	char path[MAXPATHLEN];
	char name[AG_OBJECT_NAME_MAX];
	char name_obj[AG_OBJECT_NAME_MAX];
	AG_Font *font;
	int size = (psize >= 0) ? psize : AG_Int(agConfig, "font.size");
	Uint flags = (pflags >= 0) ? pflags : AG_Uint(agConfig, "font.flags");
	char *c;

	if (pname != NULL) {
		strlcpy(name, pname, sizeof(name));
	} else {
		AG_StringCopy(agConfig, "font.face", name, sizeof(name));
	}
	memcpy(name_obj, name, sizeof(name_obj));
	for (c = &name_obj[0]; *c != '\0'; c++) {
		if (*c == '.')
			*c = '_';
	}

	AG_MutexLock(&agTextLock);
	SLIST_FOREACH(font, &fonts, fonts) {
#if 0
		printf("font size: %d == %d\n", font->size, size);
		printf("font flags: 0x%x == 0x%x\n", font->flags, flags);
		printf("font name: `%s' == `%s'\n", AGOBJECT(font)->name, name);
#endif
		if (font->size == size &&
		    font->flags == flags &&
		    strcmp(AGOBJECT(font)->name, name_obj) == 0)
			break;
	}
	if (font != NULL)
		goto out;

	font = Malloc(sizeof(AG_Font), M_TEXT);
	AG_ObjectInit(font, name, &agFontOps);
	font->size = size;
	font->flags = flags;
	font->c0 = 0;
	font->c1 = 0;
	font->height = 0;
	font->ascent = 0;
	font->descent = 0;
	font->lineskip = 0;
	
	if (AG_ConfigFile("font-path", name, NULL, path, sizeof(path)) == -1)
		goto fail;

	dprintf("Loading font: %s\n", path);

#ifdef HAVE_FREETYPE
	if (agFreetype) {
		int tflags = 0;

		dprintf("<%s>: Vector (%d pts)\n", name, size);
		if ((font->ttf = AG_TTFOpenFont(path, size)) == NULL) {
			goto fail;
		}
		if (flags & AG_FONT_BOLD)	{ tflags|=TTF_STYLE_BOLD; }
		if (flags & AG_FONT_ITALIC)	{ tflags|=TTF_STYLE_ITALIC; }
		if (flags & AG_FONT_UNDERLINE)	{ tflags|=TTF_STYLE_UNDERLINE; }
		AG_TTFSetFontStyle(font->ttf, tflags);

		font->type = AG_FONT_VECTOR;
		font->height = AG_TTFHeight(font->ttf);
		font->ascent = AG_TTFAscent(font->ttf);
		font->descent = AG_TTFDescent(font->ttf);
		font->lineskip = AG_TTFLineSkip(font->ttf);
	} else
#endif
	{
		char *s;
		char *msig, *c0, *c1, *flags;
		AG_Netbuf *buf;
		
		if ((buf = AG_NetbufOpen(path, "rb", AG_NETBUF_BIG_ENDIAN))
		    == NULL) {
			goto fail;
		}

		font->type = AG_FONT_BITMAP;
		font->bglyphs = Malloc(32*sizeof(SDL_Surface *), M_TEXT);
		font->nglyphs = 0;

		if (AG_XCFLoad(buf, 0, AG_LoadBitmapGlyph, font) == -1) {
			goto fail;
		}
		AG_NetbufClose(buf);

		/* Get the range of characters from the "MAP:x-y" string. */
		s = font->bspec;
		msig = AG_Strsep(&s, ":");
		c0 = AG_Strsep(&s, "-");
		c1 = AG_Strsep(&s, "-");
		if (font->nglyphs < 1 ||
		    msig == NULL || strcmp(msig, "MAP") != 0 ||
		    c0 == NULL || c1 == NULL ||
		    c0[0] == '\0' || c1[0] == '\0') {
			AG_SetError("Missing bitmap fontspec");
			goto fail;
		}
		font->c0 = (Uint32)strtol(c0, NULL, 10);
		font->c1 = (Uint32)strtol(c1, NULL, 10);
		if (font->nglyphs < (font->c1 - font->c0)) {
			AG_SetError("Inconsistent bitmap fontspec");
			goto fail;
		}

		font->height = font->bglyphs[0]->h;
		font->ascent = font->height;
		font->descent = 0;
		font->lineskip = font->height+2;
	
		dprintf("<%s>: Bitmap '%c'-'%c'\n", name,
		    (char)font->c0, (char)font->c1);
	}

	SLIST_INSERT_HEAD(&fonts, font, fonts);
out:
	AG_MutexUnlock(&agTextLock);
	return (font);
fail:
	AG_MutexUnlock(&agTextLock);
	AG_ObjectDestroy(font);
	Free(font, M_TEXT);
	return (NULL);
}

static Uint32
TextTmsgExpire(void *obj, Uint32 ival, void *arg)
{
	AG_Window *win = arg;

	AG_ViewDetach(win);
	return (0);
}

static void
InitTextState(void)
{
	state->font = agDefaultFont;
	state->color = SDL_MapRGB(agSurfaceFmt, 255,255,255);
	state->colorBG = SDL_MapRGBA(agSurfaceFmt, 0,0,0,0);
	state->justify = AG_TEXT_LEFT;
}

/* Initialize the font engine and configure the default font. */
int
AG_TextInit(void)
{
	int i;

#ifdef HAVE_FREETYPE
	if (AG_Bool(agConfig, "font.freetype")) {
		if (strcmp(AG_String(agConfig, "font.face"),"?") == 0) {
			AG_SetString(agConfig, "font.face", "Vera.ttf");
			AG_SetInt(agConfig, "font.size", 12);
			AG_SetUint(agConfig, "font.flags", 0);
		}
		if (AG_TTFInit() == -1) {
			AG_SetError("AG_TTFInit: %s", SDL_GetError());
			return (-1);
		}
		agFreetype = 1;
		if ((agDefaultFont = AG_FetchFont(NULL, -1, -1)) == NULL)
			fatal("%s", AG_GetError());
	} else
#endif
	{
		if (strcmp(AG_String(agConfig, "font.face"),"?") == 0) {
			AG_SetString(agConfig, "font.face", "minimal.xcf");
			AG_SetInt(agConfig, "font.size", 12);
			AG_SetUint(agConfig, "font.flags", 0);
		}
		agFreetype = 0;
		if ((agDefaultFont = AG_FetchFont(NULL, -1, -1)) == NULL)
			fatal("%s", AG_GetError());
	}
	agTextFontHeight = agDefaultFont->height;
	agTextFontAscent = agDefaultFont->ascent;
	agTextFontDescent = agDefaultFont->descent;
	agTextFontLineSkip = agDefaultFont->lineskip;

	curState = 0;
	state = &states[0];
	InitTextState();

	for (i = 0; i < GLYPH_NBUCKETS; i++) {
		SLIST_INIT(&agGlyphCache[i].glyphs);
	}
	return (0);
}

static void
FreeGlyph(AG_Glyph *gl)
{
	SDL_FreeSurface(gl->su);
#ifdef HAVE_OPENGL
	if (agView->opengl) {
		AG_LockGL();
		glDeleteTextures(1, (GLuint *)&gl->texture);
		AG_UnlockGL();
	}
#endif
	Free(gl, M_TEXT);
}

void
AG_TextDestroy(void)
{
	AG_Font *font, *nextfont;
#ifdef DEBUG
	int maxbucketsz = 0;
#endif
	int i;
	
	for (i = 0; i < GLYPH_NBUCKETS; i++) {
		AG_Glyph *gl, *ngl;
		int bucketsz = 0;

		for (gl = SLIST_FIRST(&agGlyphCache[i].glyphs);
		     gl != SLIST_END(&agGlyphCache[i].glyphs);
		     gl = ngl) {
			ngl = SLIST_NEXT(gl, glyphs);
			FreeGlyph(gl);
#ifdef DEBUG
			bucketsz++;
#endif
		}
		SLIST_INIT(&agGlyphCache[i].glyphs);
#ifdef DEBUG
		if (bucketsz > maxbucketsz)
			maxbucketsz = bucketsz;
#endif
	}
	
	for (font = SLIST_FIRST(&fonts);
	     font != SLIST_END(&fonts);
	     font = nextfont) {
		nextfont = SLIST_NEXT(font, fonts);
#ifdef HAVE_FREETYPE
		if (font->type == AG_FONT_VECTOR)
			AG_TTFCloseFont(font->ttf);
#endif
		AG_ObjectDestroy(font);
		Free(font, M_TEXT);
	}
#ifdef HAVE_FREETYPE
	if (agFreetype)
		AG_TTFDestroy();
#endif
}

static __inline__ Uint
hash_glyph(Uint32 ch)
{
	return (ch % GLYPH_NBUCKETS);
}

/* Lookup/insert a glyph in the glyph cache. */
AG_Glyph *
AG_TextRenderGlyph(Uint32 ch)
{
	AG_Glyph *gl;
	Uint h;

	h = hash_glyph(ch);
	SLIST_FOREACH(gl, &agGlyphCache[h].glyphs, glyphs) {
		if (state->font->size == gl->fontsize &&
		    state->color == gl->color &&
		    (strcmp(AGOBJECT(state->font)->name, gl->fontname) == 0) &&
		    ch == gl->ch)
			break;
	}
	if (gl == NULL) {
		Uint32 ucs[2];

		gl = Malloc(sizeof(AG_Glyph), M_TEXT);
		strlcpy(gl->fontname, AGOBJECT(state->font)->name,
		    sizeof(gl->fontname));
		gl->fontsize = state->font->size;
		gl->color = state->color;
		gl->ch = ch;
		ucs[0] = ch;
		ucs[1] = '\0';
		gl->su = AG_TextRenderUCS4(ucs);
#ifdef HAVE_OPENGL
		if (agView->opengl) {
			AG_LockGL();
			gl->texture = AG_SurfaceTexture(gl->su, gl->texcoord);
			AG_UnlockGL();
		}
#endif
		gl->nrefs = 1;
		SLIST_INSERT_HEAD(&agGlyphCache[h].glyphs, gl, glyphs);
	} else {
		gl->nrefs++;
	}
	return (gl);
}

void
AG_TextUnusedGlyph(AG_Glyph *gl)
{
	if (--gl->nrefs == 0) {
		Uint h;

		h = hash_glyph(gl->ch);
		SLIST_REMOVE(&agGlyphCache[h].glyphs, gl, ag_glyph, glyphs);
		FreeGlyph(gl);
	}
}

/* Save the current text rendering state. */
void
AG_PushTextState(void)
{
	AG_MutexLock(&agTextLock);
	if ((curState+1) >= AG_TEXT_STATES_MAX) {
		fatal("Text state stack overflow");
	}
	state = &states[++curState];
	InitTextState();
	AG_MutexUnlock(&agTextLock);
}

/* Restore the previous rendering state. */
void
AG_PopTextState(void)
{
	AG_MutexLock(&agTextLock);
	if (curState == 0) {
		fatal("No text state to pop");
	}
	state = &states[--curState];
	AG_MutexUnlock(&agTextLock);
}

/* Select the font face to use in rendering text. */
int
AG_TextFontLookup(const char *face, int size, Uint flags)
{
	int rv;

	AG_MutexLock(&agTextLock);
	state->font = AG_FetchFont(face, size, flags);
	rv = (state->font != NULL) ? 0 : -1;
	AG_MutexUnlock(&agTextLock);
	return (rv);
}

/* Select the font face to use in rendering text. */
void
AG_TextFont(AG_Font *font)
{
	int rv;

	AG_MutexLock(&agTextLock);
	state->font = font;
	AG_MutexUnlock(&agTextLock);
}

/* Select the justification mode to use in rendering text. */
void
AG_TextJustify(enum ag_text_justify mode)
{
	AG_MutexLock(&agTextLock);
	state->justify = mode;
	AG_MutexUnlock(&agTextLock);
}

/* Set text color from a 32-bit pixel value (agDisplayFormat). */
void
AG_TextColorVideo32(Uint32 pixel)
{
	AG_MutexLock(&agTextLock);
	state->color = AG_SurfacePixel(pixel);
	AG_MutexUnlock(&agTextLock);
}

/* Set text color from a 32-bit pixel value (agSurfaceFormat). */
void
AG_TextColor32(Uint32 pixel)
{
	AG_MutexLock(&agTextLock);
	state->color = pixel;
	AG_MutexUnlock(&agTextLock);
}

/* Set an opaque text color from RGB components. */
void
AG_TextColorRGB(Uint8 r, Uint8 g, Uint8 b)
{
	AG_MutexLock(&agTextLock);
	state->color = SDL_MapRGB(agSurfaceFmt, r, g, b);
	AG_MutexUnlock(&agTextLock);
}

/* Set text color from RGBA components. */
void
AG_TextColorRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	AG_MutexLock(&agTextLock);
	state->color = SDL_MapRGBA(agSurfaceFmt, r, g, b, a);
	AG_MutexUnlock(&agTextLock);
}

/* Set BG color from a 32-bit pixel value (agDisplayFormat). */
void
AG_TextBGColorVideo32(Uint32 pixel)
{
	AG_MutexLock(&agTextLock);
	state->colorBG = AG_SurfacePixel(pixel);
	AG_MutexUnlock(&agTextLock);
}

/* Set BG color from a 32-bit pixel value (agSurfaceFormat). */
void
AG_TextBGColor32(Uint32 pixel)
{
	AG_MutexLock(&agTextLock);
	state->colorBG = pixel;
	AG_MutexUnlock(&agTextLock);
}

/* Set text BG color from RGB components. */
void
AG_TextBGColorRGB(Uint8 r, Uint8 g, Uint8 b)
{
	AG_MutexLock(&agTextLock);
	state->colorBG = SDL_MapRGB(agSurfaceFmt, r, g, b);
	AG_MutexUnlock(&agTextLock);
}

/* Set text BG color from RGBA components. */
void
AG_TextBGColorRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	AG_MutexLock(&agTextLock);
	state->colorBG = SDL_MapRGBA(agSurfaceFmt, r, g, b, a);
	AG_MutexUnlock(&agTextLock);
}

/*
 * Allocate a transparent surface and render text from a standard C string
 * (possibly with UTF-8 sequences), onto it.
 */
SDL_Surface *
AG_TextRender(const char *text)
{
	Uint32 *ucs;
	SDL_Surface *su;
	
#ifdef UTF8
	ucs = AG_ImportUnicode(AG_UNICODE_FROM_UTF8, text);
#else
	ucs = AG_ImportUnicode(AG_UNICODE_FROM_USASCII, text);
#endif
	su = AG_TextRenderUCS4(ucs);
	free(ucs);
	return (su);
}

SDL_Surface *
AG_TextFormat(const char *fmt, ...)
{
	char *text;
	va_list args;

	va_start(args, fmt);
	Vasprintf(&text, fmt, args);
	va_end(args);
	return (AG_TextRender(text));
}

#ifdef SYMBOLS
static __inline__ SDL_Surface *
GetSymbolSurface(Uint32 ch)
{
	switch (ch) {
	case 'L': return (AGICON(LEFT_BUTTON_SYMBOL));
	case 'M': return (AGICON(MID_BUTTON_SYMBOL));
	case 'R': return (AGICON(RIGHT_BUTTON_SYMBOL));
	case 'C': return (AGICON(CTRL_SYMBOL));
	default: return (NULL);
	}
}
#endif /* SYMBOLS */

#ifdef HAVE_FREETYPE

/* Compute the rendered size of UCS-4 text with a FreeType font. */
static void
TextSizeFT(const Uint32 *ucs, int *w, int *h)
{
	AG_Font *font = state->font;
	AG_TTFFont *ftFont = font->ttf;
	AG_TTFGlyph *glyph;
	const Uint32 *ch;
	int x, z;
	int minx, maxx;
	int miny, maxy;

	minx = maxx = 0;
	miny = 0;
	maxy = font->height;

	/* Load each character and sum it's bounding box. */
	x = 0;
	for (ch = &ucs[0]; *ch != '\0'; ch++) {
		if (*ch == '\n') {
			maxy += font->lineskip;
			x = 0;
			continue;
		}
		if (AG_TTFFindGlyph(ftFont, *ch, TTF_CACHED_METRICS) != 0) {
			continue;
		}
		glyph = ftFont->current;

		if ((ch == &ucs[0]) && (glyph->minx < 0)) {
			/*
			 * Fixes the texture wrapping bug when the first
			 * letter has a negative minx value or horibearing
			 * value.  The entire bounding box must be adjusted to
			 * be bigger so the entire letter can fit without any
			 * texture corruption or wrapping.
			 *
			 * Effects: First enlarges bounding box.
			 * Second, xStart has to start ahead of its normal
			 * spot in the negative direction of the negative minx
			 * value. (pushes everything to the right).
			 *
			 * This will make the memory copy of the glyph bitmap
			 * data work out correctly.
			 */
			z -= glyph->minx;
		}

		z = x + glyph->minx;
		if (minx > z) {
			minx = z;
		}
		if (ftFont->style & TTF_STYLE_BOLD) {
			x += ftFont->glyph_overhang;
		}
		if (glyph->advance > glyph->maxx) {
			z = x + glyph->advance;
		} else {
			z = x + glyph->maxx;
		}
		if (maxx < z) {
			maxx = z;
		}
		x += glyph->advance;

		if (glyph->miny < miny) { miny = glyph->miny; }
		if (glyph->maxy > maxy) { maxy = glyph->maxy; }
	}
out:
	if (w != NULL) { *w = (maxx - minx); }
	if (h != NULL) { *h = (maxy - miny); }
}

/* Render UCS-4 text to a new surface using FreeType. */
static SDL_Surface *
TextRenderFT(const Uint32 *ucs)
{
	AG_Font *font = state->font;
	AG_TTFFont *ftFont = font->ttf;
	AG_TTFGlyph *glyph;
	SDL_Surface *su;
	SDL_Palette *palette;
	const Uint32 *ch;
	Uint8 *src, *dst, a;
	int row, col;
	int w, h;
	int xStart, yStart;
	
	AG_TextSizeUCS4(ucs, &w, &h);
	if (w <= 0 || h <= 0) {
		goto empty;
	}
	su = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 8, 0, 0, 0, 0);
	if (su == NULL) {
		dprintf("TextRenderFT: CreateRGBSurface: %s\n", SDL_GetError());
		goto empty;
	}
	palette = su->format->palette;

	SDL_GetRGBA(state->colorBG, agSurfaceFmt,
	    &palette->colors[0].r,
	    &palette->colors[0].g,
	    &palette->colors[0].b,
	    &a);
	if (a == 0) {
		SDL_SetColorKey(su, SDL_SRCCOLORKEY, 0);
	}
	SDL_GetRGB(state->color, agSurfaceFmt,
	    &palette->colors[1].r,
	    &palette->colors[1].g,
	    &palette->colors[1].b);

	/* Load and render each character. */
	yStart = 0;
	xStart = 0;
	for (ch = ucs; *ch != '\0'; ch++) {
		FT_Bitmap *current = NULL;

		if (*ch == '\n') {
			yStart += font->lineskip;
			xStart = 0;
			continue;
		}
#ifdef SYMBOLS
		if (agTextSymbols &&
		    ch[0] == '$' &&
		    ch[1] == '(' && ch[2] != '\0' && ch[3] == ')') {	
			SDL_Surface *sym;

			if ((sym = GetSymbolSurface(ch[2])) == NULL)
				continue;

			for (row = 0; row < sym->h; row++) {
				dst = (Uint8 *)su->pixels +
				    (yStart+row)*su->pitch +
				    (xStart+2);
				src = (Uint8 *)sym->pixels +
				    row*sym->pitch;

				for (col = 0; col < sym->w; col++) {
					if (AG_GET_PIXEL(sym,src) !=
					    sym->format->colorkey) {
						*dst = 1;
					}
					src += sym->format->BytesPerPixel;
					dst++;
				}
			}
			xStart += sym->w + 4;
			ch += 3;
			continue;
		}
#endif /* SYMBOLS */
		if (AG_TTFFindGlyph(ftFont, *ch,
		    TTF_CACHED_METRICS|TTF_CACHED_BITMAP) != 0) {
			dprintf("TTFFindGlyph: %s\n", AG_GetError());
		    	continue;
		}
		glyph = ftFont->current;
		current = &glyph->bitmap;

		/* Compensate for wrap around bug with negative minx's. */
		if ((ch == ucs) && (glyph->minx < 0))
			xStart -= glyph->minx;

		for (row = 0; row < current->rows; row++) {
			if (glyph->yoffset < 0) {
				glyph->yoffset = 0;
			}
			if (row+glyph->yoffset >= su->h)
				continue;

			dst = (Uint8 *)su->pixels +
				(yStart+row+glyph->yoffset)*su->pitch +
				xStart + glyph->minx;
			src = current->buffer + row*current->pitch;

			for (col = current->width; col > 0; --col)
				*dst++ |= *src++;
		}

		xStart += glyph->advance;
		if (ftFont->style & TTF_STYLE_BOLD)
			xStart += ftFont->glyph_overhang;
	}
	if (ftFont->style & TTF_STYLE_UNDERLINE) {
		row = ftFont->ascent - ftFont->underline_offset - 1;
		if (row >= su->h) {
			row = (su->h-1) - ftFont->underline_height;
		}
		dst = (Uint8 *)su->pixels + row * su->pitch;
		for (row = ftFont->underline_height; row > 0; --row) {
			/* 1 because 0 is the bg color */
			memset(dst, 1, su->w);
			dst += su->pitch;
		}
	}
	return (su);
empty:
	return (SDL_CreateRGBSurface(SDL_SWSURFACE,0,0,8,0,0,0,0));
}

#endif /* HAVE_FREETYPE */

static __inline__ SDL_Surface *
GetBitmapGlyph(Uint32 c)
{
	if ((state->font->flags & AG_FONT_UPPERCASE) &&
	    (isalpha(c) && islower(c))) {
		c = toupper(c);
	}
	if (c < state->font->c0 || c > state->font->c1) {
		return (state->font->bglyphs[0]);
	}
	return (state->font->bglyphs[c - state->font->c0 + 1]);
}

/* Compute the rendered size of UCS-4 text with a bitmap font. */
static __inline__ void
TextSizeBitmap(const Uint32 *ucs, int *w, int *h)
{
	const Uint32 *c;
	SDL_Surface *sGlyph;

	if (w != NULL) { *w = 0; }
	if (h != NULL) { *h = 0; }
	for (c = &ucs[0]; *c != '\0'; c++) {
		sGlyph = GetBitmapGlyph(*c);
		if (*c == '\n') {
			if (h != NULL) { *h += state->font->lineskip; }
		} else {
			if (w != NULL) { *w += sGlyph->w; }
			if (h != NULL) { *h = MAX(*h,sGlyph->h); }
		}
	}
}

/* Render UCS-4 text to a new surface using a bitmap font. */
/* TODO: blend colors */
static SDL_Surface *
TextRenderBitmap(const Uint32 *ucs)
{
	AG_Font *font = state->font;
	size_t i;
	SDL_Rect rd;
	int w, h;
	const Uint32 *c;
	SDL_Surface *sGlyph, *su;

	TextSizeBitmap(ucs, &w, &h);
	su = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32,
	    agSurfaceFmt->Rmask,
	    agSurfaceFmt->Gmask,
	    agSurfaceFmt->Bmask, 0);
	if (su == NULL) {
		fatal("CreateRGBSurface: %s", SDL_GetError());
	}
	rd.x = 0;
	rd.y = 0;
	for (c = &ucs[0]; *c != '\0'; c++) {
		if (*c == '\n') {
			rd.x = 0;
			rd.y += font->lineskip;
			continue;
		}
		sGlyph = GetBitmapGlyph(*c);
		rd.w = sGlyph->w;
		rd.h = sGlyph->h;
		SDL_BlitSurface(sGlyph, NULL, su, &rd);
		rd.x += sGlyph->w;
	}
	SDL_SetColorKey(su, SDL_SRCCOLORKEY|SDL_RLEACCEL, 0);
	SDL_SetAlpha(su, font->bglyphs[0]->flags & (SDL_SRCALPHA|SDL_RLEACCEL),
	                 font->bglyphs[0]->format->alpha);
	return (su);
}

/* Render an UCS-4 text string onto a new 8-bit surface. */
SDL_Surface *
AG_TextRenderUCS4(const Uint32 *text)
{
	switch (state->font->type) {
#ifdef HAVE_FREETYPE
	case AG_FONT_VECTOR:
		return TextRenderFT(text);
#endif
	case AG_FONT_BITMAP:
		return TextRenderBitmap(text);
	}
	return (SDL_CreateRGBSurface(SDL_SWSURFACE,0,0,8,0,0,0,0));
}

/* Return the expected size of an Unicode text element. */
void
AG_TextSizeUCS4(const Uint32 *ucs4, int *w, int *h)
{
	switch (state->font->type) {
#ifdef HAVE_FREETYPE
	case AG_FONT_VECTOR:
		TextSizeFT(ucs4, w, h);
		break;
#endif
	case AG_FONT_BITMAP:
		TextSizeBitmap(ucs4, w, h);
		break;
	}
}

/* Return the expected surface size for a UTF-8 string. */
void
AG_TextSize(const char *text, int *w, int *h)
{
	Uint32 *ucs4;

#ifdef UTF8
	ucs4 = AG_ImportUnicode(AG_UNICODE_FROM_UTF8, text);
#else
	ucs4 = AG_ImportUnicode(AG_UNICODE_FROM_USASCII, text);
#endif
	AG_TextSizeUCS4(ucs4, w, h);
	free(ucs4);
}

/* Display a message. */
void
AG_TextMsg(enum ag_text_msg_title title, const char *format, ...)
{
	char msg[AG_LABEL_MAX];
	AG_Window *win;
	AG_VBox *vb;
	va_list args;

	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);

	win = AG_WindowNew(AG_WINDOW_NORESIZE|AG_WINDOW_NOCLOSE|
	    AG_WINDOW_NOMINIMIZE|AG_WINDOW_NOMAXIMIZE|AG_WINDOW_NOBORDERS);
	AG_WindowSetCaption(win, "%s", _(agTextMsgTitles[title]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, 0);
	AG_LabelNewStaticString(vb, 0, msg);

	vb = AG_VBoxNew(win, AG_VBOX_HOMOGENOUS|AG_VBOX_HFILL|AG_VBOX_VFILL);
	AG_WidgetFocus(AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win)));

	AG_WindowShow(win);
}

/* Display a message for a given period of time. */
void
AG_TextTmsg(enum ag_text_msg_title title, Uint32 expire, const char *format,
    ...)
{
	char msg[AG_LABEL_MAX];
	AG_Window *win;
	AG_VBox *vb;
	va_list args;

	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);

	win = AG_WindowNew(AG_WINDOW_NORESIZE|AG_WINDOW_NOCLOSE|
	    AG_WINDOW_NOMINIMIZE|AG_WINDOW_NOMAXIMIZE|AG_WINDOW_NOBORDERS);
	AG_WindowSetCaption(win, "%s", _(agTextMsgTitles[title]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, 0);
	AG_LabelNewStaticString(vb, 0, msg);
	AG_WindowShow(win);

	AG_LockTimeouts(NULL);
	if (AG_TimeoutIsScheduled(NULL, &text_timeout)) {
		AG_ViewDetach((AG_Window *)text_timeout.arg);
		AG_DelTimeout(NULL, &text_timeout);
	}
	AG_UnlockTimeouts(NULL);

	AG_SetTimeout(&text_timeout, TextTmsgExpire, win, AG_TIMEOUT_LOADABLE);
	AG_AddTimeout(NULL, &text_timeout, expire);
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
	vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);

	win = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_NORESIZE|
	    AG_WINDOW_NOTITLE);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_WindowSetSpacing(win, 8);

	AG_LabelNewStaticString(win, 0, text);

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	for (i = 0; i < nbOpts; i++) {
		bOpts[i] = AG_ButtonNew(bo, 0, "XXXXXXXXXXX");
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
	AG_FSpinbutton *fsb;

	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);

	win = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_NOVRESIZE);
	AG_WindowSetCaption(win, "%s", _("Enter real number"));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, AG_VBOX_HFILL);
	AG_LabelNewStaticString(vb, 0, msg);
	
	vb = AG_VBoxNew(win, AG_VBOX_HFILL);
	{
		fsb = AG_FSpinbuttonNew(vb, 0, unit, _("Number: "));
		AG_WidgetBind(fsb, "value", AG_WIDGET_DOUBLE, fp);
		AG_FSpinbuttonSetRange(fsb, min, max);
		AG_SetEvent(fsb, "fspinbutton-return", AGWINDETACH(win));
	}
	
	vb = AG_VBoxNew(win, AG_VBOX_HOMOGENOUS|AG_VBOX_HFILL|AG_VBOX_VFILL);
	AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win));

	/* TODO test type */

	AG_WindowShow(win);
	AG_WidgetFocus(fsb->input);
}

/* Create a dialog to edit a string value. */
void
AG_TextEditString(char **sp, size_t len, const char *msgfmt, ...)
{
	char msg[AG_LABEL_MAX];
	AG_Window *win;
	AG_VBox *vb;
	va_list args;
	AG_Textbox *tb;

	va_start(args, msgfmt);
	vsnprintf(msg, sizeof(msg), msgfmt, args);
	va_end(args);

	win = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_NOVRESIZE);
	AG_WindowSetCaption(win, "%s", _("Edit string"));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, AG_VBOX_HFILL);
	AG_LabelNewStaticString(vb, 0, msg);
	
	vb = AG_VBoxNew(win, AG_VBOX_HFILL);
	{
		tb = AG_TextboxNew(vb, AG_TEXTBOX_HFILL|AG_TEXTBOX_FOCUS, NULL);
		AG_WidgetBind(tb, "string", AG_WIDGET_STRING, sp, len);
		AG_SetEvent(tb, "textbox-return", AGWINDETACH(win));
	}
	vb = AG_VBoxNew(win, AG_VBOX_HOMOGENOUS|AG_VBOX_HFILL|AG_VBOX_VFILL);
	AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win));
	AG_WindowShow(win);
}

/* Prompt the user for a string. */
void
AG_TextPromptString(const char *prompt, void (*ok_fn)(AG_Event *),
    const char *fmt, ...)
{
	AG_Window *win;
	AG_Box *bo;
	va_list args;
	AG_Button *btn;
	AG_Textbox *tb;
	AG_Event *ev;

	win = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_NOVRESIZE|
	    AG_WINDOW_NOTITLE);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_WindowSetSpacing(win, 8);

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);
	AG_LabelNewStaticString(bo, 0, prompt);
	
	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);
	{
		tb = AG_TextboxNew(bo, AG_TEXTBOX_HFILL|AG_TEXTBOX_FOCUS, NULL);
		ev = AG_SetEvent(tb, "textbox-return", ok_fn, NULL);
		AG_EVENT_GET_ARGS(ev, fmt)
		AG_EVENT_INS_VAL(ev, AG_EVARG_STRING, "string", s,
		    &tb->string[0]);
		AG_AddEvent(tb, "textbox-return", AGWINDETACH(win));
	}

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	{
		btn = AG_ButtonNew(bo, 0, _("Ok"));
		ev = AG_SetEvent(btn, "button-pushed", ok_fn, NULL);
		AG_EVENT_GET_ARGS(ev, fmt);
		AG_EVENT_INS_VAL(ev, AG_EVARG_STRING, "string", s,
		    &tb->string[0]);
		AG_AddEvent(btn, "button-pushed", AGWINDETACH(win));

		AG_ButtonNewFn(bo, 0, _("Cancel"), AGWINDETACH(win));
	}

	AG_WindowShow(win);
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

	strlcpy(buf, fontspec, sizeof(buf));
	fs = &buf[0];

	if ((s = AG_Strsep(&fs, ":,/")) != NULL &&
	    s[0] != '\0') {
		AG_SetString(agConfig, "font.face", s);
		dprintf("set: %s\n", AG_String(agConfig, "font.face"));
	}
	if ((s = AG_Strsep(&fs, ":,/")) != NULL &&
	    s[0] != '\0') {
		AG_SetInt(agConfig, "font.size", atoi(s));
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
		AG_SetUint(agConfig, "font.flags", flags);
	}
}

