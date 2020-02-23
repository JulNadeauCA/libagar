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
 * FreeType loader.
 *
 * Based on code from SDL_ttf (http://libsdl.org/projects/SDL_ttf/),
 * placed under a BSD license with permission from Sam Lantinga.
 */

#include <agar/config/have_freetype.h>
#ifdef HAVE_FREETYPE

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/text.h>
#include <agar/gui/ttf.h>
#include <agar/gui/gui_math.h>

#include <stdio.h>
#include <string.h>

/* FIXME: Right now we assume the gray-scale renderer Freetype is using
   supports 256 shades of gray, but we should instead key off of num_grays
   in the result FT_Bitmap after the FT_Render_Glyph() call. */
#define NUM_GRAYS       256

/* Handy routines for converting from fixed point */
#define FT_FLOOR(X)	((X & -64) / 64)
#define FT_CEIL(X)	(((X + 63) & -64) / 64)

/* #define DEBUG_FONTS */

static FT_Library ftLibrary;

int
AG_TTFInit(void)
{
	FT_Error rv;

	if ((rv = FT_Init_FreeType(&ftLibrary)) != 0) {
		AG_SetError("FT_Init_FreeType failed (0x%x)", rv);
		return (-1);
	}
	return (0);
}

void
AG_TTFDestroy(void)
{
	FT_Done_FreeType(ftLibrary);
}

static void
FlushGlyph(AG_TTFGlyph *_Nonnull glyph)
{
	glyph->stored = 0;
	glyph->index = 0;
	Free(glyph->bitmap.buffer);	glyph->bitmap.buffer = NULL;
	Free(glyph->pixmap.buffer);	glyph->pixmap.buffer = NULL;
	glyph->cached = 0;
}

/* Flush the entire glyph cache. */
static void
FlushCache(AG_TTFFont *_Nonnull ttf)
{
	int i, size = sizeof(ttf->cache) / sizeof(ttf->cache[0]);

	for (i = 0; i < size; i++) {
		if (ttf->cache[i].cached)
			FlushGlyph(&ttf->cache[i]);
	}
	if (ttf->scratch.cached)
		FlushGlyph(&ttf->scratch);
}

/* Load a vector font (font->spec should be initialized). */
int
AG_TTFOpenFont(AG_Font *font, const char *path)
{
	AG_FontSpec *spec = &font->spec;
	AG_TTFFont *ttf;
	FT_Error rv;
	FT_Face face;
	FT_Fixed scale;

	if ((ttf = TryMalloc(sizeof(AG_TTFFont))) == NULL) {
		return (-1);
	}
	memset(ttf, 0, sizeof(AG_TTFFont));

	switch (spec->sourceType) {
	case AG_FONT_SOURCE_FILE:
#ifdef DEBUG_FONTS
		Debug(font, "FT_New_Face(%s, %d)\n", path, spec->index);
#endif
		rv = FT_New_Face(ftLibrary, path, spec->index, &ttf->face);
		break;
	case AG_FONT_SOURCE_MEMORY:
#ifdef DEBUG_FONTS
		Debug(font, "FT_New_Memory_Face(%p,%ld, %d)\n",
		            spec->source.mem.data, spec->source.mem.size,
			    spec->index);
#endif
		rv = FT_New_Memory_Face(ftLibrary, spec->source.mem.data,
		                        spec->source.mem.size, spec->index,
		                        &ttf->face);
		break;
	default:
		rv = 1;
		break;
	}
	if (rv) {
		AG_SetError("FT_New_Face failed: %x", rv);
		goto fail;
	}
	face = ttf->face;

	/* Apply the tranformation matrix (if not identity). */
	if (spec->matrix.xx != 1.0 || spec->matrix.yy != 1.0 ||
	    spec->matrix.xy != 0.0 || spec->matrix.yx != 0.0) {
		FT_Matrix m;
		FT_Vector vec;

		m.xx = spec->matrix.xx*65536;
		m.yy = spec->matrix.yy*65536;
		m.xy = spec->matrix.xy*65536;
		m.yx = spec->matrix.yx*65536;
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
		ttf->ascent  = FT_CEIL(FT_MulFix(face->bbox.yMax, scale));
		ttf->descent = FT_CEIL(FT_MulFix(face->bbox.yMin, scale));
		ttf->height  = ttf->ascent - ttf->descent + /* baseline */ 1;
		ttf->lineskip = FT_CEIL(FT_MulFix(face->height, scale));
		ttf->underline_offset = FT_FLOOR(FT_MulFix(face->underline_position, scale));
		ttf->underline_height = FT_FLOOR(FT_MulFix(face->underline_thickness, scale));

		if      (spec->size <= 10.4f) { adjRange = 0; }
		else if (spec->size <= 14.0f) { adjRange = 1; }
		else if (spec->size <= 21.0f) { adjRange = 2; }
		else if (spec->size <= 23.8f) { adjRange = 3; }
		else if (spec->size <= 35.0f) { adjRange = 4; }
		else                          { adjRange = 5; }

		for (fa = &agFontAdjustments[0]; fa->face != NULL; fa++) {
			if (strcmp(OBJECT(font)->name, fa->face) == 0) {
				ttf->ascent += fa->ascent_offset[adjRange];
				break;
			}
		}
	} else {
		Uint fixedSize = (Uint)spec->size;

		/* Non-scalable font */
		if (fixedSize >= face->num_fixed_sizes) {
			fixedSize = face->num_fixed_sizes - 1;
			spec->size = (double)fixedSize;
		}
		ttf->font_size_family = (int)spec->size;

		(void)FT_Set_Pixel_Sizes(face,
		    face->available_sizes[fixedSize].height,
		    face->available_sizes[fixedSize].width);
		/*
		 * With non-scalable fonts, Freetype2 likes to fill many of the
		 * font metrics with 0. The size of the non-scalable fonts must
		 * be determined differently or sometimes cannot be determined.
		 */
	  	ttf->ascent = face->available_sizes[fixedSize].height - 4;
		if (ttf->ascent < 0) {
			ttf->ascent = 0;
		}
	  	ttf->descent = 0;
	  	ttf->height = face->available_sizes[fixedSize].height;
	  	ttf->lineskip = ttf->height;
	  	ttf->underline_offset = FT_FLOOR(face->underline_position);
	  	ttf->underline_height = FT_FLOOR(face->underline_thickness);
	}
	if (ttf->underline_height < 1) {
		ttf->underline_height = 1;
	}
	if (ttf->style & AG_FONT_SW_BOLD) {
		ttf->glyph_overhang = face->size->metrics.y_ppem / 10;
	}
	if (ttf->style & AG_FONT_SW_ITALIC) {
		ttf->glyph_italics = 0.207;			/* 12 deg */
		ttf->glyph_italics *= ttf->height;
	}

	/* Apply the standard style modifiers */
	ttf->style = font->flags;
	
	/* The Agar font should inherit the metrics. */
	font->height = ttf->height;
	font->ascent = ttf->ascent;
	font->descent = ttf->descent;
	font->lineskip = ttf->lineskip;

	font->data.vec.ttf = ttf;
	return (0);
fail_face:
	FT_Done_Face(ttf->face);
fail:
	Free(ttf);
	return (-1);
}

void
AG_TTFCloseFont(AG_Font *font)
{
	AG_TTFFont *ttf = (AG_TTFFont *)font->data.vec.ttf;

	if (ttf == NULL) {
		return;
	}
	FlushCache(ttf);
	FT_Done_Face(ttf->face);
	free(ttf);
	font->data.vec.ttf = NULL;
}

/* Process the bold style. */
static void
ProcessBold(AG_TTFFont *_Nonnull ttf, FT_Bitmap *_Nonnull dst)
{
	int row, col, offset, pixel;
	Uint8 *pixmap;

	for (row = dst->rows - 1; row >= 0; --row) {
		pixmap = (Uint8 *)dst->buffer + row*dst->pitch;
		for (offset = 1; offset <= ttf->glyph_overhang; offset++) {
			for (col = dst->width - 1; col > 0; --col) {
				pixel = (pixmap[col] + pixmap[col-1]);
				if (pixel > NUM_GRAYS - 1) {
					pixel = NUM_GRAYS - 1;
				}
				pixmap[col] = (Uint8)pixel;
			}
		}
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
				*dstp++ = NUM_GRAYS-1;
			} else {
				*dstp++ = 0x00;
			}
			ch <<= 1;
		}
	}
}

/* Render and cache the glyph corresponding to the given Unicode character. */
static int
LoadGlyph(AG_TTFFont *_Nonnull ttf, AG_Char ch, AG_TTFGlyph *_Nonnull cached,
    int want)
{
	FT_Face face = ttf->face;
	FT_GlyphSlot glyph;
	FT_Glyph_Metrics *metrics;
	FT_Outline *outline;
	FT_Error rv;

	if (cached->index == 0) {
		cached->index = FT_Get_Char_Index(face, ch);
	}
	if ((rv = FT_Load_Glyph(face, cached->index, FT_LOAD_DEFAULT)) != 0) {
		AG_SetError("FT_LoadGlyph failed (%d)", rv);
		return (-1);
	}

	/* Get our glyph shortcuts */
	glyph = face->glyph;
	metrics = &glyph->metrics;
	outline = &glyph->outline;

	/* Get the glyph metrics if desired */
	if ((want & TTF_CACHED_METRICS) && !(cached->stored & TTF_CACHED_METRICS)) {
		if (FT_IS_SCALABLE(face)) {
			/* Get the bounding box. */
			cached->minx = FT_FLOOR(metrics->horiBearingX);
			cached->maxx = cached->minx + FT_CEIL(metrics->width);
			cached->maxy = FT_FLOOR(metrics->horiBearingY);
			cached->miny = cached->maxy - FT_CEIL(metrics->height);
		} else {
			/*
			 * Get the bounding box for non-scalable format.
			 * Again, freetype2 fills in many of the font metrics
			 * with the value of 0, so some of the values we
			 * need must be calculated differently with certain
			 * assumptions about non-scalable formats.
			 */
			cached->minx = FT_FLOOR(metrics->horiBearingX);
			cached->maxx = cached->minx + FT_CEIL(metrics->horiAdvance);
			cached->maxy = FT_FLOOR(metrics->horiBearingY);
			cached->miny = cached->maxy - FT_CEIL(face->available_sizes[ttf->font_size_family].height);
		}
		cached->yoffset = ttf->ascent - cached->maxy;
		cached->advance = FT_CEIL(metrics->horiAdvance);

		/* Adjust for software-generated bold and italic. */
		if (ttf->style & AG_FONT_SW_BOLD) {
			cached->maxx += ttf->glyph_overhang;
		}
		if (ttf->style & AG_FONT_SW_ITALIC) {
			cached->maxx += (int)Ceil(ttf->glyph_italics);
		}
		cached->stored |= TTF_CACHED_METRICS;
	}

	if (((want & TTF_CACHED_BITMAP) && !(cached->stored & TTF_CACHED_BITMAP)) ||
	    ((want & TTF_CACHED_PIXMAP) && !(cached->stored & TTF_CACHED_PIXMAP))) {
		FT_Bitmap *src, *dst;
	    	const int mono = (want & TTF_CACHED_BITMAP);

		if (ttf->style & AG_FONT_SW_ITALIC) {    /* Software Italic */
			FT_Matrix shear;

			shear.xx = 1 << 16;
			shear.xy = (int) (ttf->glyph_italics * (1 << 16)) /
			                  ttf->height;
			shear.yx = 0;
			shear.yy = 1 << 16;
			FT_Outline_Transform(outline, &shear);
		}

		/* Render the glyph. */
		if (FT_Render_Glyph(glyph, mono ? ft_render_mode_mono :
		                                  ft_render_mode_normal) != 0) {
			AG_SetError("FT_RenderGlyph(0x%x) failed", (Uint)ch);
			return (-1);
		}

		/* Copy over information to cache. */
		src = &glyph->bitmap;
		dst = (mono) ? &cached->bitmap : &cached->pixmap;
		memcpy(dst, src, sizeof(*dst));

		/*
		 * FT_Render_Glyph() and .fon fonts always generate a
		 * two-color (black and white) glyphslot surface, even
		 * when rendered in ft_render_mode_normal.  This is probably
		 * a freetype2 bug because it is inconsistent with the
		 * freetype2 documentation under FT_Render_Mode section.
		 */
		if (mono || !FT_IS_SCALABLE(face))
			dst->pitch <<= 3;

		if (ttf->style & AG_FONT_SW_BOLD) {        /* Software Bold */
			dst->pitch += ttf->glyph_overhang;
			dst->width += ttf->glyph_overhang;
		}
		if (ttf->style & AG_FONT_SW_ITALIC) {    /* Software Italic */
			const int bump = (int)Ceil(ttf->glyph_italics);

			dst->pitch += bump;
			dst->width += bump;
		}

		if (dst->rows != 0) {
			int i;

			if ((dst->buffer = TryMalloc(dst->pitch*dst->rows)) == NULL) {
				return (-1);
			}
			memset(dst->buffer, 0, dst->pitch * dst->rows);

			for (i = 0; i < src->rows; i++) {
				const int soffset = i * src->pitch;
				const int doffset = i * dst->pitch;

				if (mono) {
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
					                         soffset,
					                         doffset);
				} else {
					memcpy(dst->buffer + doffset,
					       src->buffer + soffset,
					       src->pitch);
				}
			}
		}

		if (ttf->style & AG_FONT_SW_BOLD)          /* Software Bold */
			ProcessBold(ttf, dst);

		/* Mark that we rendered this format */
		if (mono) {
			cached->stored |= TTF_CACHED_BITMAP;
		} else {
			cached->stored |= TTF_CACHED_PIXMAP;
		}
	}

	/* We're done, mark this glyph cached. */
	cached->cached = ch;
	return (0);
}

/* Load the glyph corresponding to the specified Unicode character. */
int
AG_TTFFindGlyph(AG_TTFFont *font, AG_Char ch, int want)
{
#ifdef AG_UNICODE
	if (ch < 256) {
#else
	if (1) {
#endif
		font->current = &font->cache[ch];
	} else {
		if (font->scratch.cached != ch) {
			FlushGlyph(&font->scratch);
		}
		font->current = &font->scratch;
	}
	return (((font->current->stored & want) != want) ?
	    LoadGlyph(font, ch, font->current, want) :
	    0);
}
#endif /* HAVE_FREETYPE */
