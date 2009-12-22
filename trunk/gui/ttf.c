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

#include <config/have_freetype.h>

#ifdef HAVE_FREETYPE

#include <core/core.h>

#include "ttf.h"
#include "gui_math.h"

#include <stdio.h>
#include <string.h>

/* FIXME: Right now we assume the gray-scale renderer Freetype is using
   supports 256 shades of gray, but we should instead key off of num_grays
   in the result FT_Bitmap after the FT_Render_Glyph() call. */
#define NUM_GRAYS       256

/* Handy routines for converting from fixed point */
#define FT_FLOOR(X)	((X & -64) / 64)
#define FT_CEIL(X)	(((X + 63) & -64) / 64)

static FT_Library ftLibrary;

static void FlushCache(AG_TTFFont *);
static void FlushGlyph(AG_TTFGlyph *);

int
AG_TTFInit(void)
{
	if (FT_Init_FreeType(&ftLibrary) != 0) {
		AG_SetError(_("Font engine initialization failed."));
		return (-1);
	}
	return (0);
}

void
AG_TTFDestroy(void)
{
	FT_Done_FreeType(ftLibrary);
}

static int
InitFontAttributes(AG_TTFFont *font, int ptsize)
{
	FT_Face face = font->face;
	FT_Fixed scale;
	
	/* Make sure that our font face is scalable (global metrics). */
	if (FT_IS_SCALABLE(face)) {
	  	/* Set the character size and use default DPI (72). */
	  	if (FT_Set_Char_Size(font->face, 0, ptsize * 64, 0, 0)) {
			AG_SetError(_("Failed to set font size."));
			return (-1);
		}
		/* Get the scalable font metrics for this font */
		scale = face->size->metrics.y_scale;
		font->ascent  = FT_CEIL(FT_MulFix(face->bbox.yMax, scale));
		font->descent = FT_CEIL(FT_MulFix(face->bbox.yMin, scale));
		font->height  = font->ascent - font->descent + /* baseline */ 1;
		font->lineskip = FT_CEIL(FT_MulFix(face->height, scale));
		font->underline_offset = FT_FLOOR(FT_MulFix(
		    face->underline_position, scale));
		font->underline_height = FT_FLOOR(FT_MulFix(
		    face->underline_thickness, scale));
	} else {
		/*
		 * Non-scalable font case.  ptsize determines which family
		 * or series of fonts to grab from the non-scalable format.
		 * It is not the point size of the font.
		 */
		if (ptsize >= font->face->num_fixed_sizes) {
			ptsize = font->face->num_fixed_sizes - 1;
		}
		font->font_size_family = ptsize;
		FT_Set_Pixel_Sizes(face,
		    face->available_sizes[ptsize].height,
		    face->available_sizes[ptsize].width);
	  	
		/*
		 * With non-scalale fonts, Freetype2 likes to fill many of the
		 * font metrics with the value of 0. The size of the
		 * non-scalable fonts must be determined differently or
		 * sometimes cannot be determined.
		 */
	  	font->ascent = face->available_sizes[ptsize].height;
	  	font->descent = 0;
	  	font->height = face->available_sizes[ptsize].height;
	  	font->lineskip = FT_CEIL(font->ascent);
	  	font->underline_offset = FT_FLOOR(face->underline_position);
	  	font->underline_height = FT_FLOOR(face->underline_thickness);
	}
	if (font->underline_height < 1) {
		font->underline_height = 1;
	}
	font->style = TTF_STYLE_NORMAL;
	font->glyph_overhang = face->size->metrics.y_ppem / 10;
	font->glyph_italics = 0.207f;				/* 12 deg */
	font->glyph_italics *= font->height;
	return (0);
}

AG_TTFFont *
AG_TTFOpenFont(const char *file, int ptsize)
{
	AG_TTFFont *font;
	int rv;

	font = Malloc(sizeof(AG_TTFFont));
	memset(font, 0, sizeof(AG_TTFFont));
	if ((rv = FT_New_Face(ftLibrary, file, 0, &font->face)) != 0) {
		AG_SetError("%s: FreeType error %d", file, rv);
		goto fail;
	}
	if (InitFontAttributes(font, ptsize) == -1) {
		AG_TTFCloseFont(font);
		goto fail;
	}
	return (font);
fail:
	Free(font);
	return (NULL);
}

AG_TTFFont *
AG_TTFOpenFontFromMemory(const Uint8 *data, size_t size, int ptsize)
{
	AG_TTFFont *font;
	int rv;

	font = Malloc(sizeof(AG_TTFFont));
	memset(font, 0, sizeof(AG_TTFFont));
	if ((rv = FT_New_Memory_Face(ftLibrary, data, size, 0, &font->face))
	    != 0) {
		AG_SetError(_("FreeType error %d"), rv);
		goto fail;
	}
	if (InitFontAttributes(font, ptsize) == -1) {
		AG_TTFCloseFont(font);
		goto fail;
	}
	return (font);
fail:
	Free(font);
	return (NULL);
}

void
AG_TTFCloseFont(AG_TTFFont *font)
{
	FlushCache(font);
	FT_Done_Face(font->face);
	Free(font);
}

static void
FlushGlyph(AG_TTFGlyph *glyph)
{
	glyph->stored = 0;
	glyph->index = 0;
	if (glyph->bitmap.buffer) {
		Free(glyph->bitmap.buffer);
		glyph->bitmap.buffer = NULL;
	}
	if (glyph->pixmap.buffer) {
		Free(glyph->pixmap.buffer);
		glyph->pixmap.buffer = NULL;
	}
	glyph->cached = 0;
}
	
static void
FlushCache(AG_TTFFont *font)
{
	int i, size = sizeof(font->cache) / sizeof(font->cache[0]);

	for (i = 0; i < size; i++) {
		if (font->cache[i].cached)
			FlushGlyph(&font->cache[i]);
	}
	if (font->scratch.cached)
		FlushGlyph(&font->scratch);
}

static int
LoadGlyph(AG_TTFFont *font, Uint32 ch, AG_TTFGlyph *cached, int want)
{
	FT_Face face;
	FT_GlyphSlot glyph;
	FT_Glyph_Metrics *metrics;
	FT_Outline *outline;

	face = font->face;

	if (cached->index == 0) {
		cached->index = FT_Get_Char_Index(face, ch);
	}
	if (FT_Load_Glyph(face, cached->index, FT_LOAD_DEFAULT) != 0) {
		AG_SetError(_("Failed to load the TTF glyph."));
		return (-1);
	}

	/* Get our glyph shortcuts */
	glyph = face->glyph;
	metrics = &glyph->metrics;
	outline = &glyph->outline;

	/* Get the glyph metrics if desired */
	if ((want & TTF_CACHED_METRICS) &&
	    !(cached->stored & TTF_CACHED_METRICS)) {
		if (FT_IS_SCALABLE(face)) {
			/* Get the bounding box. */
			cached->minx = FT_FLOOR(metrics->horiBearingX);
			cached->maxx = cached->minx + FT_CEIL(metrics->width);
			cached->maxy = FT_FLOOR(metrics->horiBearingY);
			cached->miny = cached->maxy - FT_CEIL(metrics->height);
			cached->yoffset = font->ascent - cached->maxy;
			cached->advance = FT_CEIL(metrics->horiAdvance);
		} else {
			/*
			 * Get the bounding box for non-scalable format.
			 * Again, freetype2 fills in many of the font metrics
			 * with the value of 0, so some of the values we
			 * need must be calculated differently with certain
			 * assumptions about non-scalable formats.
			 */
			cached->minx = FT_FLOOR(metrics->horiBearingX);
			cached->maxx = cached->minx +
			    FT_CEIL(metrics->horiAdvance);
			cached->maxy = FT_FLOOR(metrics->horiBearingY);
			cached->miny = cached->maxy -
			    FT_CEIL(face->available_sizes
			            [font->font_size_family].height);
			cached->yoffset = 0;
			cached->advance = FT_CEIL(metrics->horiAdvance);
		}

		/* Adjust for bold and italic text. */
		if (font->style & TTF_STYLE_BOLD) {
			cached->maxx += font->glyph_overhang;
		}
		if (font->style & TTF_STYLE_ITALIC) {
			cached->maxx += (int)Ceil(font->glyph_italics);
		}
		cached->stored |= TTF_CACHED_METRICS;
	}

	if (((want & TTF_CACHED_BITMAP) &&
	    !(cached->stored & TTF_CACHED_BITMAP)) ||
	    ((want & TTF_CACHED_PIXMAP) &&
	    !(cached->stored & TTF_CACHED_PIXMAP))) {
		int i, mono = (want & TTF_CACHED_BITMAP);
		FT_Bitmap *src, *dst;

		/* Handle the italic style. */
		if (font->style & TTF_STYLE_ITALIC) {
			FT_Matrix shear;

			shear.xx = 1 << 16;
			shear.xy = (int) (font->glyph_italics * (1 << 16)) /
			                  font->height;
			shear.yx = 0;
			shear.yy = 1 << 16;
			FT_Outline_Transform(outline, &shear);
		}

		/* Render the glyph. */
		if (mono) {
			if (FT_Render_Glyph(glyph, ft_render_mode_mono) != 0) {
				AG_SetError("Error glyph 0x%x", (Uint)ch);
				return (-1);
			}
		} else {
			if (FT_Render_Glyph(glyph, ft_render_mode_normal)
			    != 0) {
				AG_SetError("Error norm glyph 0x%x", (Uint)ch);
				return (-1);
			}
		}

		/* Copy over information to cache. */
		src = &glyph->bitmap;
		if (mono) {
			dst = &cached->bitmap;
		} else {
			dst = &cached->pixmap;
		}
		memcpy(dst, src, sizeof(*dst));

		/*
		 * FT_Render_Glyph() and .fon fonts always generate a
		 * two-color (black and white) glyphslot surface, even
		 * when rendered in ft_render_mode_normal.  This is probably
		 * a freetype2 bug because it is inconsistent with the
		 * freetype2 documentation under FT_Render_Mode section.
		 */
		if (mono || !FT_IS_SCALABLE(face))
			dst->pitch *= 8;

		/* Adjust for bold and italic text. */
		if (font->style & TTF_STYLE_BOLD) {
			int bump = font->glyph_overhang;

			dst->pitch += bump;
			dst->width += bump;
		}
		if (font->style & TTF_STYLE_ITALIC) {
			int bump;
			
			bump = (int)Ceil(font->glyph_italics);
			dst->pitch += bump;
			dst->width += bump;
		}

		if (dst->rows != 0) {
			dst->buffer = Malloc(dst->pitch*dst->rows);
			memset(dst->buffer, 0, dst->pitch * dst->rows);

			for (i = 0; i < src->rows; i++) {
				int soffset = i * src->pitch;
				int doffset = i * dst->pitch;

				if (mono) {
					unsigned char *srcp, *dstp;
					int j;
					
					srcp = src->buffer + soffset;
					dstp = dst->buffer + doffset;

					for (j = 0; j < src->width; j += 8) {
						unsigned char ch = *srcp++;

						*dstp++ = (ch&0x80) >> 7;
						ch <<= 1;
						*dstp++ = (ch&0x80) >> 7;
						ch <<= 1;
						*dstp++ = (ch&0x80) >> 7;
						ch <<= 1;
						*dstp++ = (ch&0x80) >> 7;
						ch <<= 1;
						*dstp++ = (ch&0x80) >> 7;
						ch <<= 1;
						*dstp++ = (ch&0x80) >> 7;
						ch <<= 1;
						*dstp++ = (ch&0x80) >> 7;
						ch <<= 1;
						*dstp++ = (ch&0x80) >> 7;
					}
				} else if (!FT_IS_SCALABLE(face)) {
					/*
					 * This special case wouldn't
					 * be here if the FT_Render_Glyph()
					 * function wasn't buggy when it tried
					 * to render a .fon font with 256
					 * shades of gray.  Instead, it
					 * returns a black and white surface
					 * and we have to translate it back
					 * to a 256 gray shaded surface. 
					 */
					unsigned char *srcp, *dstp;
					unsigned char ch;
					int j, k;
					
					srcp = src->buffer + soffset;
					dstp = dst->buffer + doffset;

					for (j = 0; j < src->width; j += 8) {
						ch = *srcp++;
						for (k = 0; k < 8; ++k) {
							if ((ch&0x80) >> 7) {
								*dstp++ =
								    NUM_GRAYS-1;
							} else {
								*dstp++ = 0x00;
							}
							ch <<= 1;
						}
					}
				} else {
					memcpy(dst->buffer+doffset,
					       src->buffer+soffset, src->pitch);
				}
			}
		}

		/* Handle the bold style */
		if (font->style & TTF_STYLE_BOLD) {
			int row, col, offset, pixel;
			Uint8 *pixmap;

			/*
			 * The pixmap is a little hard, we have to add and
			 * clamp.
			 */
			for (row = dst->rows - 1; row >= 0; --row) {
				pixmap = (Uint8 *)dst->buffer + row*dst->pitch;
				for (offset = 1; offset <= font->glyph_overhang;
				    offset++) {
					for (col = dst->width - 1; col > 0;
					     --col) {
						pixel = (pixmap[col] +
						    pixmap[col-1]);
						if (pixel > NUM_GRAYS - 1) {
							pixel = NUM_GRAYS - 1;
						}
						pixmap[col] = (Uint8)pixel;
					}
				}
			}
		}

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

int
AG_TTFFindGlyph(AG_TTFFont *font, Uint32 ch, int want)
{
	if (ch < 256) {
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

int
AG_TTFFaceFixedWidth(AG_TTFFont *font)
{
	return (FT_IS_FIXED_WIDTH(font->face));
}

int
AG_TTFGlyphMetrics(AG_TTFFont *font, Uint32 ch, int *minx, int *maxx,
    int *miny, int *maxy, int *advance)
{
	if (AG_TTFFindGlyph(font, ch, TTF_CACHED_METRICS) != 0) {
		return (-1);
	}
	if (minx != NULL)
		*minx = font->current->minx;
	if (maxx != NULL)
		*maxx = font->current->maxx;
	if (miny != NULL)
		*miny = font->current->miny;
	if (maxy != NULL)
		*maxy = font->current->maxy;
	if (advance != NULL)
		*advance = font->current->advance;

	return (0);
}

void
AG_TTFSetFontStyle(AG_TTFFont *font, int style)
{
	font->style = style;
	FlushCache(font);
}

#endif /* HAVE_FREETYPE */
