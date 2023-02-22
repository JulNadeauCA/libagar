/*
 * Copyright (c) 2001-2023 Julien Nadeau Carriere <vedge@csoft.net>
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
 * FreeType 2 module for the AG_Text(3) typography system.
 */

#include <agar/core/core.h>
#include <agar/gui/text.h>
#include <agar/gui/font_ft.h>
#include <agar/gui/font_bf.h>

/* Handy routines for converting from fixed point */
#undef  FT_FLOOR
#define FT_FLOOR(X) ((X & -64) / 64)
#undef  FT_CEIL
#define FT_CEIL(X)  (((X + 63) & -64) / 64)

static int agFtInited = 0;                /* FreeType library is initalized */
static FT_Library agFtLibrary;

/* #define DEBUG_FONTS */

/*
 * Create and load a FreeType font.
 * 
 * The name string may include weight/style suffices.
 * The nameBase string should be the bare font name (without any suffices).
 * Font names are case-insensitive.
 * Font names starting with `_' refer to embedded (built-in) fonts.
 * 
 */
AG_FontFt *
AG_FontFtNew(const char *name, const char *nameBase, const AG_FontSpec *spec,
    const char *path, Uint flags)
{
	AG_FontFt *fontFt;
	AG_Font *font;

	if ((fontFt = TryMalloc(sizeof(AG_FontFt))) == NULL) {
		return (NULL);
	}
	font = AGFONT(fontFt);
	AG_ObjectInit(fontFt, &agFontFtClass);
	AG_ObjectSetNameS(fontFt, name);

	Strlcpy(font->name, nameBase, sizeof(font->name));
	memcpy(&font->spec, spec, sizeof(AG_FontSpec));
	font->flags |= flags;
	
	if (AGFONT_OPS(fontFt)->open(fontFt, path) == -1) {
		AG_ObjectDestroy(fontFt);
		return (NULL);
	}
	return (fontFt);
}

static int
InitFreeType(void)
{
	FT_Error rv;

	if ((rv = FT_Init_FreeType(&agFtLibrary)) != 0) {
		AG_SetError("FT_Init_FreeType failed (0x%x)", rv);
		return (-1);
	}
#ifdef DEBUG_FONTS
	{
		FT_Int major, minor, patch;

		FT_Library_Version(agFtLibrary, &major, &minor, &patch);
		Debug(NULL,
		    "FreeType " AGSI_BOLD "%d.%d.%d" AGSI_RST " initialized "
		    "(built against %d.%d.%d)\n",
		    major, minor, patch,
		    FREETYPE_MAJOR, FREETYPE_MINOR, FREETYPE_PATCH);
	}
#endif
	agFtInited++;
	return (0);
}

static int
Open(void *_Nonnull obj, const char *_Nonnull path)
{
	AG_FontFt *fontFt = obj;
	AG_Font *font = AGFONT(fontFt);
	const AG_FontSpec *spec = &font->spec;
	FT_Error rv;
	FT_Face face;
	FT_Fixed scale;

	if (agFtInited == 0) {
		if (InitFreeType() != 0)
			return (-1);
	}
	agFtInited++;

	switch (spec->sourceType) {
	case AG_FONT_SOURCE_FILE:
#ifdef DEBUG_FONTS
		Debug(fontFt, "FT_New_Face(\""
		    AGSI_PATH "%s" AGSI_RST "\", "
		    AGSI_RED "%.02f" AGSI_RST " pts, "
		    AGSI_RED "#%d" AGSI_RST ")\n",
		    path, spec->size, spec->index);
#endif
		rv = FT_New_Face(agFtLibrary, path, spec->index, &fontFt->face);
		break;
	case AG_FONT_SOURCE_MEMORY:
#ifdef DEBUG_FONTS
		Debug(fontFt, "FT_New_Memory_Face("
		    "%p, %ldKB, "
		    AGSI_RED "%.02f" AGSI_RST " pts, "
		    AGSI_RED "#%d" AGSI_RST ")\n",
		    spec->source.mem.data,
		    (spec->source.mem.size>1024) ? (spec->source.mem.size>>10):1,
		     spec->size, spec->index);
#endif
		rv = FT_New_Memory_Face(agFtLibrary, spec->source.mem.data,
		    spec->source.mem.size, spec->index, &fontFt->face);
		break;
	default:
		rv = 1;
		break;
	}
	if (rv) {
		AG_SetError("FT_New_Face failed: %x", rv);
		goto fail;
	}

	face = fontFt->face;

	/* Apply the tranformation matrix (if not identity). */
	if (spec->matrix.xx != 1.0 || spec->matrix.yy != 1.0 ||
	    spec->matrix.xy != 0.0 || spec->matrix.yx != 0.0) {
		FT_Matrix m;
		FT_Vector vec;

		m.xx = spec->matrix.xx * 65536;
		m.yy = spec->matrix.yy * 65536;
		m.xy = spec->matrix.xy * 65536;
		m.yx = spec->matrix.yx * 65536;
		vec.x = 0.0;
		vec.y = 0.0;
		FT_Set_Transform(face, &m, &vec);
	}

	if (FT_IS_SCALABLE(face)) {
		const AG_FontAdjustment *fa;
		int adjRange;

		if ((rv = FT_Set_Char_Size(face, 0, spec->size*64, 0, 0)) != 0) {
			AG_SetError("FT_Set_Char_Size failed: %x", rv);
			goto fail_face;
		}

		scale = face->size->metrics.y_scale;

		font->ascent = FT_CEIL(FT_MulFix(face->bbox.yMax, scale));
		font->descent = FT_CEIL(FT_MulFix(face->bbox.yMin, scale));
		font->height  = font->ascent - font->descent + 1;
		font->lineskip = FT_CEIL(FT_MulFix(face->height, scale));

		font->underlinePos = FT_FLOOR(FT_MulFix(face->underline_position, scale));
		font->underlineThk = FT_FLOOR(FT_MulFix(face->underline_thickness, scale));

		/* Apply size range specific ascent adjustment */
		if      (spec->size <= 10.4f) { adjRange = 0; }
		else if (spec->size <= 14.0f) { adjRange = 1; }
		else if (spec->size <= 21.0f) { adjRange = 2; }
		else if (spec->size <= 23.8f) { adjRange = 3; }
		else if (spec->size <= 35.0f) { adjRange = 4; }
		else                          { adjRange = 5; }
		for (fa = &agFontAdjustments[0]; fa->face != NULL; fa++) {
			if (strcmp(OBJECT(font)->name, fa->face) == 0) {
				font->ascent += fa->ascent_offset[adjRange];
				break;
			}
		}
	} else {                                       /* Non-scalable font */
		Uint fixedSize = (Uint)spec->size;

		if (fixedSize >= face->num_fixed_sizes) {
			fixedSize = face->num_fixed_sizes - 1;
		}
		fontFt->fixedSize = (int)fixedSize;

		(void)FT_Set_Pixel_Sizes(face,
		    face->available_sizes[fixedSize].height,
		    face->available_sizes[fixedSize].width);
		/*
		 * With non-scalable fonts, Freetype2 likes to fill many of the
		 * font metrics with 0. The size of the non-scalable fonts must
		 * be determined differently or sometimes cannot be determined.
		 */
	  	font->ascent = face->available_sizes[fixedSize].height - 4;
		if (font->ascent < 0) {
			/* TODO remember this correction */
			font->ascent = 0;
		}
	  	font->descent = 0;
	  	font->height = face->available_sizes[fixedSize].height;
	  	font->lineskip = font->height + 4; /* XXX TODO */
	  	font->underlinePos = FT_FLOOR(face->underline_position);
	  	font->underlineThk = FT_FLOOR(face->underline_thickness);
	}
	if (font->underlineThk < 1) {
		font->underlineThk = 1;
	}
	return (0);
fail_face:
	FT_Done_Face(fontFt->face);
fail:
	return (-1);
}

static void
FlushGlyph(AG_GlyphFt *_Nonnull G)
{
	G->stored = 0;
	G->index = 0;
	Free(G->bitmap.buffer);
	G->bitmap.buffer = NULL;
	Free(G->pixmap.buffer);
	G->pixmap.buffer = NULL;
	G->cached = 0;
}

/* Flush the entire glyph cache. */
static void
FlushCache(void *_Nonnull obj)
{
	AG_FontFt *fontFt = obj;
	const int size = sizeof(fontFt->cache) / sizeof(fontFt->cache[0]);
	int i;

	for (i = 0; i < size; i++) {
		AG_GlyphFt *G = &fontFt->cache[i];

		if (G->cached)
			FlushGlyph(G);
	}
	if (fontFt->scratch.cached)
		FlushGlyph(&fontFt->scratch);
}

static void
Close(void *_Nonnull obj)
{
	AG_FontFt *fontFt = obj;

	FlushCache(fontFt);
	FT_Done_Face(fontFt->face);

	if (--agFtInited == 0) {
		FT_Done_FreeType(agFtLibrary);
#ifdef DEBUG_FONTS
		Debug(NULL, "FreeType finalized\n");
#endif
	}
}

/*
 * This special case wouldn't be here if the FT_Render_Glyph() function
 * wasn't buggy when it tried to render a .fon font with 256 shades of
 * gray.  Instead, it returns a black and white surface and we have to
 * translate it back to a 256 gray shaded surface.
 * XXX is this still needed?
 */
static void
ProcessNonScalablePixmap(FT_Bitmap *_Nonnull src, FT_Bitmap *_Nonnull dst,
    int soffset, int doffset)
{
	Uint8 *srcp, *dstp;
	Uint8 ch;
	int j, k;
					
	srcp = src->buffer + soffset;
	dstp = dst->buffer + doffset;

	for (j = 0; j < src->width; j += 8) {
		ch = *srcp++;
		for (k = 0; k < 8; ++k) {
			if ((ch&0x80) >> 7) {
				/*
				 * FIXME: Right now we assume the gray-scale
				 * renderer Freetype is using supports 256
				 * shades of gray, but we should instead key off
				 * of num_grays in the result FT_Bitmap after
				 * the FT_Render_Glyph() call.
				 */
				*dstp++ = 0xff;
			} else {
				*dstp++ = 0x00;
			}
			ch <<= 1;
		}
	}
}

static void *
GetGlyph(void *_Nonnull obj, AG_Char ch, Uint want)
{
	AG_FontFt *fontFt = obj;
	AG_Font *font = AGFONT(fontFt);
	FT_Face face = fontFt->face;
	AG_GlyphFt *G;
	FT_GlyphSlot Gslot;
	FT_Glyph_Metrics *Gmetrics;
	FT_Error rv;

#ifdef AG_UNICODE
	if (ch < 256) {
#else
	if (1) {
#endif
		fontFt->current = &fontFt->cache[ch];
	} else {
		if (fontFt->scratch.cached != ch) {
			FlushGlyph(&fontFt->scratch);
		}
		fontFt->current = &fontFt->scratch;
	}
	G = fontFt->current;

	if ((G->stored & want) == want) 
		return (void *)(G);

	if (G->index == 0) {
		G->index = FT_Get_Char_Index(face, ch);
	}
	if ((rv = FT_Load_Glyph(face, G->index, FT_LOAD_DEFAULT)) != 0) {
		AG_SetError("FT_Load_Glyph failed (%d)", rv);
		return (NULL);
	}

	/* Get our glyph shortcuts */
	Gslot = face->glyph;
	Gmetrics = &Gslot->metrics;

	/*
	 * Glyph Metrics.
	 */
	if ((want & AG_GLYPH_FT_METRICS) && !(G->stored & AG_GLYPH_FT_METRICS)) {
		if (FT_IS_SCALABLE(face)) {
			/* Get the bounding box. */
			G->xMin = FT_FLOOR(Gmetrics->horiBearingX);
			G->xMax = G->xMin + FT_CEIL(Gmetrics->width);
			G->yMax = FT_FLOOR(Gmetrics->horiBearingY);
			G->yMin = G->yMax - FT_CEIL(Gmetrics->height);
		} else {
			/*
			 * Get the bounding box for non-scalable format.
			 * Again, freetype2 fills in many of the font metrics
			 * with the value of 0, so some of the values we
			 * need must be calculated differently with certain
			 * assumptions about non-scalable formats.
			 */
			G->xMin = FT_FLOOR(Gmetrics->horiBearingX);
			G->xMax = G->xMin + FT_CEIL(Gmetrics->horiAdvance);
			G->yMax = FT_FLOOR(Gmetrics->horiBearingY);
			G->yMin = G->yMax - FT_CEIL(face->available_sizes[fontFt->fixedSize].height);
		}
		G->yOffset = font->ascent - G->yMax;
		G->advance = FT_CEIL(Gmetrics->horiAdvance);
		G->stored |= AG_GLYPH_FT_METRICS;
	}

	/*
	 * Rendered two-color bitmap / pixmap.
	 */
	if (((want & AG_GLYPH_FT_BITMAP) && !(G->stored & AG_GLYPH_FT_BITMAP)) ||
	    ((want & AG_GLYPH_FT_PIXMAP) && !(G->stored & AG_GLYPH_FT_PIXMAP))) {
		FT_Bitmap *src, *dst;
	    	const int wantMono = (want & AG_GLYPH_FT_BITMAP);

		/* Render the glyph. */
		if (FT_Render_Glyph(Gslot, wantMono ? FT_RENDER_MODE_MONO :
		                                      FT_RENDER_MODE_NORMAL) != 0) {
			AG_SetError("FT_Render_Glyph(0x%x) failed", (Uint)ch);
			return (NULL);
		}

		/* Copy over information to cache. */
		src = &Gslot->bitmap;
		dst = (wantMono) ? &G->bitmap : &G->pixmap;
		memcpy(dst, src, sizeof(FT_Bitmap));

		/*
		 * FT_Render_Glyph() and .fon fonts always generate a
		 * two-color (black and white) glyph slot surface, even
		 * when rendered in FT_RENDER_MODE_NORMAL.  This is probably
		 * a freetype2 bug because it is inconsistent with the
		 * freetype2 documentation under FT_Render_Mode section.
		 */
		if (wantMono || !FT_IS_SCALABLE(face))
			dst->pitch <<= 3;

		if (dst->rows != 0) {
			const AG_Size bufferSize = (dst->pitch * dst->rows);
			int i;

			if ((dst->buffer = TryMalloc(bufferSize)) == NULL) {
				return (NULL);
			}
			memset(dst->buffer, 0, bufferSize);

			for (i = 0; i < src->rows; i++) {
				const int soffset = i * src->pitch;
				const int doffset = i * dst->pitch;

				if (wantMono) {
					Uint8 *srcp = src->buffer + soffset;
					Uint8 *dstp = dst->buffer + doffset;
					int j;
					
					for (j = 0; j < src->width; j += 8) {
						Uint8 ch = *srcp++;

						*dstp++ = (ch&0x80) >> 7; ch <<= 1;
						*dstp++ = (ch&0x80) >> 7; ch <<= 1;
						*dstp++ = (ch&0x80) >> 7; ch <<= 1;
						*dstp++ = (ch&0x80) >> 7; ch <<= 1;
						*dstp++ = (ch&0x80) >> 7; ch <<= 1;
						*dstp++ = (ch&0x80) >> 7; ch <<= 1;
						*dstp++ = (ch&0x80) >> 7; ch <<= 1;
						*dstp++ = (ch&0x80) >> 7;
					}
				} else if (!FT_IS_SCALABLE(face)) {
					ProcessNonScalablePixmap(src, dst,
					    soffset, doffset);
				} else {
					memcpy(dst->buffer + doffset,
					       src->buffer + soffset,
					       src->pitch);
				}
			}
		}

		/* Mark that we rendered this format */
		if (wantMono) {
			G->stored |= AG_GLYPH_FT_BITMAP;      /* Monochrome */
		} else {
			G->stored |= AG_GLYPH_FT_PIXMAP;     /* Gray levels */
		}
	}
	G->cached = ch;                           /* Mark this glyph cached */
	return (G);
}

static void
GetGlyphMetrics(void *_Nonnull obj, AG_Glyph *G)
{
	AG_GlyphFt *Gft;
	AG_Font *font = G->font;

	if ((Gft = GetGlyph(font, G->ch, AG_GLYPH_FT_METRICS)) != NULL) {
		G->advance = Gft->advance;
	} else {
		G->advance = G->su->w;                          /* Fallback */
	}
}

/*
 * Calculate the offset in pixels needed to align text based on the
 * current justification mode.
 */
static __inline__ int
JustifyOffset(const AG_TextState *_Nonnull ts, int w, int wLine)
{
	switch (ts->justify) {
	case AG_TEXT_LEFT:   return (0);
	case AG_TEXT_CENTER: return ((w >> 1) - (wLine >> 1));
	case AG_TEXT_RIGHT:  return (w - wLine);
	}
	return (0);
}

static void
Render(const AG_Char *_Nonnull ucs, AG_Surface *_Nonnull S,
    const AG_TextMetrics *_Nonnull Tm, AG_Font *_Nonnull fontOrig,
    const AG_Color *_Nonnull cBgOrig, const AG_Color *_Nonnull cFgOrig)
{
	const AG_TextState *ts = AG_TEXT_STATE_CUR();
	AG_Font *fontCur = fontOrig;
	AG_FontFt *fontFtCur;
	const AG_Char *ch;
	Uint8 *src, *dst;
	AG_Color cBg = *cBgOrig;
	AG_Color cFg = *cFgOrig;
	FT_UInt indexPrev = 0;
	const int BytesPerPixel = S->format.BytesPerPixel;
	const int lineskip = fontOrig->lineskip;
	int xStart, yStart, line, x,y, w;
	int ascentCur;

 	xStart = (Tm->nLines>1) ? JustifyOffset(ts, Tm->w, Tm->wLines[0]) : 0;
 	yStart = 0;

	ascentCur = fontCur->ascent;
	S->guides[0] = (Uint16)(fontCur->height - ascentCur);

	for (ch = &ucs[0], line=0;
	     *ch != '\0';
	     ch++) {
		switch (*ch) {
		case '\n':
			yStart += lineskip;
			xStart = JustifyOffset(ts, Tm->w, Tm->wLines[++line]);
			continue;
		case '\r':
			xStart = 0;
			continue;
		case '\t':
			xStart += ts->tabWd;                        /* TODO */
			continue;
		default:
			break;
		}
		if (ch[0] == 0x1b &&
		    ch[1] >= 0x40 && ch[1] <= 0x5f && ch[2] != '\0') {
			AG_TextANSI ansi;

			if (AG_TextParseANSI(ts, &ansi, &ch[1]) != 0) {
			/* 	AG_Verbose("%s; ignoring\n", AG_GetError()); */
				continue;
			}
			if (ansi.ctrl != AG_ANSI_CSI_SGR) {
				ch += ansi.len;
				continue;
			}
			switch (ansi.sgr) {
			case AG_SGR_RESET:
			case AG_SGR_NO_FG_NO_BG:
				fontCur = fontOrig;
				ascentCur = fontCur->ascent;
				cFg = *cFgOrig;
				cBg = *cBgOrig;
				break;
			case AG_SGR_FG:
				cFg = ansi.color;
				break;
			case AG_SGR_BG:
				cBg = ansi.color;
				break;
			case AG_SGR_BOLD:
				fontCur = AG_FetchFont(fontCur->name,
				    fontCur->spec.size,
				    fontCur->flags | AG_FONT_BOLD);
				if (fontCur == NULL) { fontCur = fontOrig; }
				ascentCur = MAX(fontOrig->ascent, fontCur->ascent);
				break;
			case AG_SGR_ITALIC:
				fontCur = AG_FetchFont(fontCur->name,
				    fontCur->spec.size,
				    fontCur->flags | AG_FONT_ITALIC);
				if (fontCur == NULL) { fontCur = fontOrig; }
				ascentCur = MAX(fontOrig->ascent, fontCur->ascent);
				break;
			case AG_SGR_FAINT:
				/* TODO */
				break;
			case AG_SGR_UNDERLINE:
				/* TODO */
				break;
			case AG_SGR_PRI_FONT:
			case AG_SGR_ALT_FONT_1:
			case AG_SGR_ALT_FONT_2:
			case AG_SGR_ALT_FONT_3:
			case AG_SGR_ALT_FONT_4:
			case AG_SGR_ALT_FONT_5:
			case AG_SGR_ALT_FONT_6:
			case AG_SGR_ALT_FONT_7:
			case AG_SGR_ALT_FONT_8:
			case AG_SGR_ALT_FONT_9:
			case AG_SGR_FRAKTUR:
				fontCur = AG_FetchFont(
				    agCoreFonts[ansi.sgr - 10],
				    fontCur->spec.size, fontCur->flags);
				if (fontCur == NULL) { fontCur = fontOrig; }
				ascentCur = MAX(fontOrig->ascent, fontCur->ascent);
				break;
			case AG_SGR_ALT_FONT_11:
			case AG_SGR_ALT_FONT_12:
			case AG_SGR_ALT_FONT_13:
			case AG_SGR_ALT_FONT_14:
			case AG_SGR_ALT_FONT_15:
			case AG_SGR_ALT_FONT_16:
			case AG_SGR_ALT_FONT_17:
				fontCur = AG_FetchFont(
				    agCoreFonts[11 + (ansi.sgr - 66)],
				    fontOrig->spec.size, fontOrig->flags);
				if (fontCur == NULL) { fontCur = fontOrig; }
				ascentCur = MAX(fontOrig->ascent, fontCur->ascent);
				break;
			default:
				break;
			}
			ch += ansi.len;
			continue;
		}
		if (fontCur->spec.type == AG_FONT_FREETYPE) {
			AG_GlyphFt *G;
			int yOffset;

			fontFtCur = AGFONTFT(fontCur);

			G = AGFONT_OPS(fontCur)->get_glyph(fontCur, *ch,
			    AG_GLYPH_FT_METRICS | AG_GLYPH_FT_PIXMAP);

			if (G == NULL) {
				/* TODO render a blank box and continue */
				return;
			}
			yOffset = ascentCur - G->yMax;

			/*
			 * Ensure the width of the pixmap is correct. On some cases,
			 * freetype may report a larger pixmap than possible.
			 */
			if ((w = G->pixmap.width) > G->xMax - G->xMin)
				w = G->xMax - G->xMin;

			if (FT_HAS_KERNING(fontFtCur->face) && indexPrev &&
			    G->index) {
				FT_Vector delta; 
	
				FT_Get_Kerning(fontFtCur->face, indexPrev,
				    G->index, FT_KERNING_DEFAULT, &delta);

				xStart += delta.x >> 6;
			}
	
			/* Prevent texture wrapping with first glyph. */
			if ((ch == &ucs[0]) && (G->xMin < 0)) {
				xStart -= G->xMin;
			}
			if ((xStart + G->xMin) < 0 ||
			    (xStart + G->xMin) >= S->w) {
				continue;
			}
			if (cBg.a == AG_TRANSPARENT) {
				/*
				 * Lower blit - Write on transparent BG.
				 */
				for (y = 0; y < G->pixmap.rows; y++) {
					if ((yStart + y + yOffset) < 0 ||
					    (yStart + y + yOffset) >= S->h)
						continue;
	
					src = (Uint8 *)(G->pixmap.buffer +
					                G->pixmap.pitch * y);
					dst = S->pixels +
					    (yStart + y + yOffset) * S->pitch +
					    (xStart + G->xMin) * BytesPerPixel;
	
					for (x = 0; x < w; x++) {
						if ((cFg.a = AG_8toH(*src++)) > 0) {
							AG_SurfacePut_At(S, dst,
							    AG_MapPixel(&S->format, &cFg));
						}
						dst += BytesPerPixel;
					}
				}
			} else {
				/*
				 * Lower blit - Blend against colored BG.
				 */
				for (y = 0; y < G->pixmap.rows; y++) {
					if (y + yOffset < 0 ||
					    y + yOffset >= S->h)
						continue;

					src = (Uint8 *)(G->pixmap.buffer +
					                G->pixmap.pitch * y);
					dst = S->pixels +
					    (yStart + y + yOffset) * S->pitch +
					    (xStart + G->xMin) * BytesPerPixel;
	
					for (x = 0; x < w; x++) {
						if ((cFg.a = AG_8toH(*src++)) > 0) {
							AG_SurfaceBlend_At(S, dst, &cFg,
							    AG_ALPHA_DST);
						}
						dst += BytesPerPixel;
					}
				}
			}

			xStart += G->advance;
			indexPrev = G->index;

		} else if (fontCur->spec.type == AG_FONT_BITMAP) {
			AG_FontBf *fontBfCur = AGFONTBF(fontCur);
			AG_GlyphBf *Gbf;

			Gbf = AGFONT_OPS(fontBfCur)->get_glyph(fontBfCur, *ch, 0);
			if (Gbf == NULL) {
#ifdef DEBUG_FONTS
				Debug(fontBfCur, "Not found: `%c' (0x%x)\n",
				    (char)*ch, *ch);
#endif
				xStart += fontBfCur->wdRef;
				continue;
			}
			(void)cBg;
			(void)cFg;
			if (*ch != ' ') {
				AG_SurfaceBlit(fontBfCur->S, &Gbf->rs, S,
				    xStart,
				    yStart + ascentCur - Gbf->rs.h + Gbf->yOffset);
			}

			xStart += Gbf->rs.w + fontBfCur->advance;
		}
	}
}

static void
Size(const AG_Font *_Nonnull font, const AG_Char *_Nonnull ucs,
    AG_TextMetrics *_Nonnull Tm, int extended)
{
	AG_TextState *ts = AG_TEXT_STATE_CUR();
	AG_Font *fontOrig = ts->font, *fontCur = fontOrig;
	const AG_Char *ch;
	const int lineskip = fontCur->lineskip;
	int xMin=0, xMax=0, yMin=0, yMax;
	int xMinLine=0, xMaxLine=0;
	int x, z;

	/* Compute the sum of the bounding box of the characters. */
	yMax = fontCur->lineskip;
	x = 0;
	for (ch = &ucs[0]; *ch != '\0'; ch++) {
		if (*ch == '\n') {
			if (extended) {
				Tm->wLines = Realloc(Tm->wLines,
				    (Tm->nLines + 2) * sizeof(Uint));
				Tm->wLines[Tm->nLines++] = (xMaxLine - xMinLine);
				xMinLine = 0;
				xMaxLine = 0;
			}
			yMax += lineskip;
			x = 0;
			continue;
		}
		if (*ch == '\t') {
			x += ts->tabWd;  /* XXX TODO */
			continue;
		}
		if (ch[0] == 0x1b &&
		    ch[1] >= 0x40 && ch[1] <= 0x5f && ch[2] != '\0') {
			AG_TextANSI ansi;
			
			if (AG_TextParseANSI(ts, &ansi, &ch[1]) != 0) {
				continue;
			}
			if (ansi.ctrl != AG_ANSI_CSI_SGR) {
				ch += ansi.len;
				continue;
			}
			switch (ansi.sgr) {
			case AG_SGR_RESET:
			case AG_SGR_NO_FG_NO_BG:
				fontCur = fontOrig;
				break;
			case AG_SGR_BOLD:
				fontCur = AG_FetchFont(fontOrig->name,
				    fontOrig->spec.size,
				    fontOrig->flags | AG_FONT_BOLD);
				if (fontCur == NULL) { fontCur = fontOrig; }
				break;
			case AG_SGR_ITALIC:
				fontCur = AG_FetchFont(fontOrig->name,
				    fontOrig->spec.size,
				    fontOrig->flags | AG_FONT_ITALIC);
				if (fontCur == NULL) { fontCur = fontOrig; }
				break;
			case AG_SGR_PRI_FONT:
			case AG_SGR_ALT_FONT_1:
			case AG_SGR_ALT_FONT_2:
			case AG_SGR_ALT_FONT_3:
			case AG_SGR_ALT_FONT_4:
			case AG_SGR_ALT_FONT_5:
			case AG_SGR_ALT_FONT_6:
			case AG_SGR_ALT_FONT_7:
			case AG_SGR_ALT_FONT_8:
			case AG_SGR_ALT_FONT_9:
			case AG_SGR_FRAKTUR:
				fontCur = AG_FetchFont(
				    agCoreFonts[ansi.sgr - 10],
				    fontCur->spec.size, fontCur->flags);
				if (fontCur == NULL) { fontCur = fontOrig; }
				break;
			case AG_SGR_ALT_FONT_11:
			case AG_SGR_ALT_FONT_12:
			case AG_SGR_ALT_FONT_13:
			case AG_SGR_ALT_FONT_14:
			case AG_SGR_ALT_FONT_15:
			case AG_SGR_ALT_FONT_16:
			case AG_SGR_ALT_FONT_17:
				fontCur = AG_FetchFont(
				    agCoreFonts[11 + (ansi.sgr - 66)],
				    fontCur->spec.size, fontCur->flags);
				if (fontCur == NULL) { fontCur = fontOrig; }
				break;
			default:
				break;
			}
			ch += ansi.len;
			continue;
		}
		if (fontCur->spec.type == AG_FONT_FREETYPE) {
			AG_GlyphFt *Gft;

			Gft = AGFONT_OPS(fontCur)->get_glyph(fontCur, *ch,
			    AG_GLYPH_FT_METRICS);

			z = x + Gft->xMin;
			if (xMin > z) { xMin = z; }
			if (xMinLine > z) { xMinLine = z; }

			z = x + MAX(Gft->advance, Gft->xMax);
			if (xMax < z) { xMax = z; }
			if (xMaxLine < z) { xMaxLine = z; }
			x += Gft->advance;

			if (Gft->yMin < yMin) { yMin = Gft->yMin; }
			if (Gft->yMax > yMax) { yMax = Gft->yMax; }
		} else if (fontCur->spec.type == AG_FONT_BITMAP) {
			AG_GlyphBf *Gbf;
			int advance;

			Gbf = AGFONT_OPS(fontCur)->get_glyph(fontCur, *ch, 0);
			if (Gbf == NULL) {
				x += AGFONTBF(fontCur)->wdRef;
				continue;
			}
			if (xMin > x) { xMin = x; }
			if (xMinLine > x) { xMinLine = x; }

			advance = AGFONTBF(fontCur)->advance;
			z = x + Gbf->rs.w + advance;
			if (xMax < z) { xMax = z; }
			if (xMaxLine < z) { xMaxLine = z; }

			x += Gbf->rs.w + advance;
		}
	}
	if (*ch != '\n' && extended) {
		if (Tm->nLines > 0) {
			Tm->wLines = Realloc(Tm->wLines, (Tm->nLines + 2) *
			                                 sizeof(Uint));
			Tm->wLines[Tm->nLines] = (xMaxLine - xMinLine);
		}
		Tm->nLines++;
	}
	Tm->w = (xMax - xMin) + 1;
	Tm->h = (yMax - yMin) + 1;
}

static void
Init(void *_Nonnull obj)
{
	AG_FontFt *font = obj;

	memset(&font->current, 0, sizeof(AG_GlyphFt *) +   /* current */
	                          sizeof(AG_GlyphFt)*256 + /* cache */
	                          sizeof(AG_GlyphFt) +     /* scratch */
	                          sizeof(int));            /* fixedSize */
}

AG_FontClass agFontFtClass = {
	{
		"Agar(Font:FontFt)",
		sizeof(AG_FontFt),
		{ 0, 0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Open,
	FlushCache,
	Close,
	GetGlyph,
	GetGlyphMetrics,
	Render,
	Size
};
