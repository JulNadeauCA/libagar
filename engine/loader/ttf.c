/*	$Csoft: ttf.c,v 1.12 2005/02/08 15:48:36 vedge Exp $	*/
/*	Id: SDL_ttf.c,v 1.6 2002/01/18 21:46:04 slouken Exp	*/

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

#include <config/have_freetype.h>

#ifdef HAVE_FREETYPE

#include <engine/engine.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL_endian.h>

#include "ttf.h"

/* FIXME: Right now we assume the gray-scale renderer Freetype is using
   supports 256 shades of gray, but we should instead key off of num_grays
   in the result FT_Bitmap after the FT_Render_Glyph() call. */
#define NUM_GRAYS       256

/* Handy routines for converting from fixed point */
#define FT_FLOOR(X)	((X & -64) / 64)
#define FT_CEIL(X)	(((X + 63) & -64) / 64)

static FT_Library library;

static void ttf_flush_cache(struct ttf_font *);
static void ttf_flush_glyph(struct ttf_glyph *);
static int ttf_load_glyph(struct ttf_font *, Uint32, struct ttf_glyph *, int);

int
ttf_init(void)
{
	if (FT_Init_FreeType(&library) != 0) {
		error_set(_("Font engine initialization failed."));
		return (-1);
	}
	return (0);
}

void
ttf_destroy(void)
{
	FT_Done_FreeType(library);
}

struct ttf_font *
ttf_open_font(const char *file, int ptsize)
{
	struct ttf_font *font;
	FT_Face face;
	FT_Fixed scale;

	font = Malloc(sizeof(struct ttf_font), M_LOADER);
	memset(font, 0, sizeof(struct ttf_font));

	if (FT_New_Face(library, file, 0, &font->face) != 0) {
		error_set(_("Cannot find font face: `%s'."), file);
		goto fail1;
	}
	face = font->face;

	/* Make sure that our font face is scalable (global metrics). */
	if (FT_IS_SCALABLE(face)) {
	  	/* Set the character size and use default DPI (72). */
	  	if (FT_Set_Char_Size(font->face, 0, ptsize * 64, 0, 0)) {
			error_set(_("Failed to set font size."));
			goto fail2;
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
#ifdef DEBUG
	printf("%s:\n", file);
	printf("\tascent=%d, descent=%d, height=%d, lineskip=%d\n",
	    font->ascent, font->descent, font->height, font->lineskip);
	printf("\tunderline_offset=%d, underine_height=%d\n",
	    font->underline_offset, font->underline_height);
#endif

	font->style = TTF_STYLE_NORMAL;
	font->glyph_overhang = face->size->metrics.y_ppem / 10;

	/* x offset = cos(((90.0-12)/360)*2*M_PI), or 12 degree angle. */
	font->glyph_italics = 0.207f;
	font->glyph_italics *= font->height;
	return (font);
fail2:
	ttf_close_font(font);
fail1:
	Free(font, M_LOADER);
	return (NULL);
}

void
ttf_close_font(struct ttf_font *font)
{
	ttf_flush_cache(font);
	FT_Done_Face(font->face);
	Free(font, M_LOADER);
}

static void
ttf_flush_glyph(struct ttf_glyph *glyph)
{
	glyph->stored = 0;
	glyph->index = 0;
	if (glyph->bitmap.buffer) {
		free(glyph->bitmap.buffer);
		glyph->bitmap.buffer = NULL;
	}
	if (glyph->pixmap.buffer) {
		free(glyph->pixmap.buffer);
		glyph->pixmap.buffer = NULL;
	}
	glyph->cached = 0;
}
	
static void
ttf_flush_cache(struct ttf_font *font)
{
	int i, size = sizeof(font->cache) / sizeof(font->cache[0]);

	for (i = 0; i < size; i++) {
		if (font->cache[i].cached)
			ttf_flush_glyph(&font->cache[i]);
	}
	if (font->scratch.cached)
		ttf_flush_glyph(&font->scratch);
}

static int
ttf_load_glyph(struct ttf_font *font, Uint32 ch, struct ttf_glyph *cached,
    int want)
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
		error_set(_("Failed to load the TTF glyph."));
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
			cached->maxx += (int)ceil(font->glyph_italics);
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
				error_set(_("Cannot render mono glyph."));
				return (-1);
			}
		} else {
			if (FT_Render_Glyph(glyph, ft_render_mode_normal)
			    != 0) {
				error_set(_("Cannot render normal glyph."));
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
			
			bump = (int)ceil(font->glyph_italics);
			dst->pitch += bump;
			dst->width += bump;
		}

		if (dst->rows != 0) {
			dst->buffer = Malloc(dst->pitch * dst->rows, M_LOADER);
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
ttf_find_glyph(struct ttf_font *font, Uint32 ch, int want)
{
	int retval = 0;

	if (ch < 256) {
		font->current = &font->cache[ch];
	} else {
		if (font->scratch.cached != ch) {
			ttf_flush_glyph(&font->scratch);
		}
		font->current = &font->scratch;
	}
	if ((font->current->stored & want) != want) {
		retval = ttf_load_glyph(font, ch, font->current, want);
	}
	return (retval);
}

int
ttf_font_height(struct ttf_font *font)
{
	return (font->height);
}

int
ttf_font_ascent(struct ttf_font *font)
{
       return (font->ascent);
}

int
ttf_font_descent(struct ttf_font *font)
{
	return (font->descent);
}

int
ttf_font_line_skip(struct ttf_font *font)
{
	return (font->lineskip);
}

int
ttf_font_face_fixed_width(struct ttf_font *font)
{
	return (FT_IS_FIXED_WIDTH(font->face));
}

char *
ttf_font_face_family(struct ttf_font *font)
{
	return (font->face->family_name);
}

char *
ttf_font_face_style(struct ttf_font *font)
{
	return (font->face->style_name);
}

int
ttf_glyph_metrics(struct ttf_font *font, Uint32 ch, int *minx, int *maxx,
    int *miny, int *maxy, int *advance)
{
	if (ttf_find_glyph(font, ch, TTF_CACHED_METRICS) != 0) {
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

/* Predict the size of rendered UTF-8 text. */
int
ttf_size_text(struct ttf_font *font, const char *utf8, int *w, int *h)
{
	Uint32 *ucs;
	int status;

	ucs = unicode_import(UNICODE_FROM_UTF8, utf8);
	status = ttf_size_unicode(font, ucs, w, h);
	free(ucs);
	return (status);
}

/* Predict the size of rendered UCS-4 text. */
int
ttf_size_unicode(struct ttf_font *font, const Uint32 *ucs, int *w, int *h)
{
	struct ttf_glyph *glyph;
	const Uint32 *ch;
	int status;
	int x, z;
	int minx, maxx;
	int miny, maxy;

	status = 0;
	minx = maxx = 0;
	miny = maxy = 0;

	/* Load each character and sum it's bounding box. */
	x = 0;
	for (ch = ucs; *ch != '\0'; ch++) {
		if (ttf_find_glyph(font, *ch, TTF_CACHED_METRICS) != 0) {
			return (-1);
		}
		glyph = font->current;

		if ((ch == ucs) && (glyph->minx < 0)) {
			/*
			 * Fixes the texture wrapping bug when the first
			 * letter has a negative minx value or horibearing
			 * value.  The entire bounding box must be adjusted to
			 * be bigger so the entire letter can fit without any
			 * texture corruption or wrapping.
			 *
			 * Effects: First enlarges bounding box.
			 * Second, xstart has to start ahead of its normal
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
		if (font->style & TTF_STYLE_BOLD) {
			x += font->glyph_overhang;
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

		if (glyph->miny < miny)
			miny = glyph->miny;
		if (glyph->maxy > maxy)
			maxy = glyph->maxy;
	}

	/* Fill the bounds rectangle. */
	if (w)
		*w = (maxx - minx);
	if (h)
		*h = (maxy - miny);

	return (status);
}

/* Render UTF-8 text to a new surface. */
SDL_Surface *
ttf_render_text_solid(struct ttf_font *font, const char *utf8, SDL_Color fg)
{
	SDL_Surface *textsu;
	Uint32 *ucs;

	ucs = unicode_import(UNICODE_FROM_UTF8, utf8);
	textsu = ttf_render_unicode_solid(font, ucs, fg);
	free(ucs);
	return (textsu);
}

/* Render UCS-4 text to a new surface. */
SDL_Surface *
ttf_render_unicode_solid(struct ttf_font *font, const Uint32 *ucs, SDL_Color fg)
{
	struct ttf_glyph *glyph;
	SDL_Surface *textsu;
	SDL_Palette *palette;
	const Uint32 *ch;
	Uint8 *src, *dst;
	int row, col;
	int w, h;
	int xstart;

	if ((ttf_size_unicode(font, ucs, &w, NULL) < 0) || w== 0) {
		error_set(_("The text has zero width."));
		return (NULL);
	}
	h = font->height;
	textsu = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 8, 0, 0, 0, 0);
	if (textsu == NULL) {
		error_set("SDL_CreateRGBSurface: %s", SDL_GetError());
		return (NULL);
	}

	/* Fill the palette with the foreground color. */
	palette = textsu->format->palette;
	palette->colors[0].r = 255 - fg.r;
	palette->colors[0].g = 255 - fg.g;
	palette->colors[0].b = 255 - fg.b;
	palette->colors[1].r = fg.r;
	palette->colors[1].g = fg.g;
	palette->colors[1].b = fg.b;
	SDL_SetColorKey(textsu, SDL_SRCCOLORKEY, 0);

	/* Load and render each character. */
	xstart = 0;
	for (ch = ucs; *ch; ch++) {
		FT_Bitmap *current = NULL;

		if (ttf_find_glyph(font, *ch,
		    TTF_CACHED_METRICS|TTF_CACHED_BITMAP) != 0) {
		    	goto fail1;
		}
		glyph = font->current;
		current = &glyph->bitmap;

		/* Compensate for wrap around bug with negative minx's. */
		if ((ch == ucs) && (glyph->minx < 0))
			xstart -= glyph->minx;

		for (row = 0; row < current->rows; ++row) {
			/*
			 * Work around bug seen with FreeType 9.3.3 that
			 * occurs inconsistently with MALLOC_OPTIONS=AFGJ
			 * under OpenBSD 3.5.
			 */
			if (glyph->yoffset < 0)
				glyph->yoffset = 0;

			if (row+glyph->yoffset >= textsu->h)
				continue;

			dst = (Uint8 *)textsu->pixels +
				(row + glyph->yoffset)*textsu->pitch +
				xstart + glyph->minx;
			src = current->buffer + row*current->pitch;

			for (col = current->width; col > 0; --col)
				*dst++ |= *src++;
		}

		xstart += glyph->advance;
		if (font->style & TTF_STYLE_BOLD)
			xstart += font->glyph_overhang;
	}
	if (font->style & TTF_STYLE_UNDERLINE) {
		row = font->ascent - font->underline_offset - 1;
		if (row >= textsu->h) {
			row = (textsu->h-1) - font->underline_height;
		}
		dst = (Uint8 *)textsu->pixels + row * textsu->pitch;
		for (row = font->underline_height; row > 0; --row) {
			/* 1 because 0 is the bg color */
			memset(dst, 1, textsu->w);
			dst += textsu->pitch;
		}
	}
	return (textsu);
fail1:
	SDL_FreeSurface(textsu);
	return (NULL);
}

void
ttf_set_font_style(struct ttf_font *font, int style)
{
	font->style = style;
	ttf_flush_cache(font);
}

int
ttf_get_font_style(struct ttf_font *font)
{
	return (font->style);
}

#endif /* HAVE_FREETYPE */
