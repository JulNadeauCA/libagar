/*	$Csoft: ttf.c,v 1.8 2003/03/25 13:48:06 vedge Exp $	*/
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

#include <engine/engine.h>

#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <freetype/freetype.h>
#include <freetype/ftoutln.h>

#include <SDL.h>

#include "ttf.h"

/* FIXME: Right now we assume the gray-scale renderer Freetype is using
   supports 256 shades of gray, but we should instead key off of num_grays
   in the result FT_Bitmap after the FT_Render_Glyph() call. */
#define NUM_GRAYS       256

/* Handy routines for converting from fixed point */
#define FT_FLOOR(X)	((X & -64) / 64)
#define FT_CEIL(X)	(((X + 63) & -64) / 64)

struct cached_glyph {
	int	stored;
#define CACHED_METRICS	0x10
#define CACHED_BITMAP	0x01
#define CACHED_PIXMAP	0x02
	FT_UInt	index;
	FT_Bitmap bitmap;
	FT_Bitmap pixmap;
	int	minx, maxx;
	int	miny, maxy;
	int	yoffset;
	int	advance;
	Uint16	cached;
};

struct _ttf_font {
	/* Freetype2 maintains all sorts of useful info itself */
	FT_Face	face;

	int	height;
	int	ascent;
	int	descent;
	int	lineskip;
	int	style;
	int	glyph_overhang;
	float	glyph_italics;
	int	underline_offset;
	int	underline_height;

	/* Cache for style-transformed glyphs */
	struct cached_glyph	*current;
	struct cached_glyph	 cache[256];
	struct cached_glyph	 scratch;
};

/* The FreeType font engine/library */
static FT_Library library;

static void	ttf_flush_cache(ttf_font *);
static void	ttf_flush_glyph(struct cached_glyph *);
static int	ttf_load_glyph(ttf_font *, Uint16, struct cached_glyph *,
		    int);
static int	ttf_find_glyph(ttf_font *, Uint16, int);

int
ttf_init(void)
{
	FT_Int maj, min, patch;

	if (FT_Init_FreeType(&library) != 0) {
		error_set("font engine init failed");
		return (-1);
	}
	FT_Library_Version(library, &maj, &min, &patch);
	printf("Font engine: Freetype %d.%d.%d\n", maj, min, patch);
	return (0);
}

void
ttf_destroy(void)
{
	FT_Done_FreeType(library);
}

ttf_font *
ttf_open_font_index(const char *file, int ptsize, long index)
{
	ttf_font *font;
	FT_Face face;
	FT_Fixed scale;

	font = Malloc(sizeof(ttf_font));
	memset(font, 0, sizeof(ttf_font));

	if (FT_New_Face(library, file, 0, &font->face) != 0) {
		error_set("error loading face `%s'", file);
		goto fail1;
	}
	if (index != 0) {
		if (font->face->num_faces > index) {
		  	FT_Done_Face(font->face);
			if (FT_New_Face(library, file, index, &font->face)
			    != 0) {
				error_set("error getting font face");
				goto fail1;
			}
		} else {
			error_set("bad font face");
			goto fail1;
		}
	}
	face = font->face;

	if (!FT_IS_SCALABLE(face)) {
		error_set("non-scalable font face");
		goto fail2;
	}

	if (FT_Set_Char_Size(font->face, 0, ptsize * 64, 0, 0) != 0) {
		error_set("error getting char size");
		goto fail2;
	}

	/* Get the scalable font metrics for this font */
	scale = face->size->metrics.y_scale;
	font->ascent  = FT_CEIL(FT_MulFix(face->bbox.yMax, scale));
	font->descent = FT_CEIL(FT_MulFix(face->bbox.yMin, scale));
	font->height  = font->ascent - font->descent + /* baseline */ 1;
	font->lineskip = FT_CEIL(FT_MulFix(face->height, scale));
	font->underline_offset = FT_FLOOR(FT_MulFix(face->underline_position,
	    scale));
	font->underline_height = FT_FLOOR(FT_MulFix(face->underline_thickness,
	    scale));
	if (font->underline_height < 1) {
		font->underline_height = 1;
	}

	font->style = TTF_STYLE_NORMAL;
	font->glyph_overhang = face->size->metrics.y_ppem / 10;

	/* x offset = cos(((90.0-12)/360)*2*M_PI), or 12 degree angle */
	font->glyph_italics = 0.207f;
	font->glyph_italics *= font->height;

	return (font);
fail2:
	ttf_close_font(font);
fail1:
	free(font);
	return (NULL);
}

ttf_font *
ttf_open_font(const char *file, int ptsize)
{
	return (ttf_open_font_index(file, ptsize, 0));
}

void
ttf_close_font(ttf_font *font)
{
	ttf_flush_cache(font);
	FT_Done_Face(font->face);
	free(font);
}

static void
ttf_flush_glyph(struct cached_glyph *glyph)
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
ttf_flush_cache(ttf_font *font)
{
	int i, size = sizeof(font->cache) / sizeof(font->cache[0]);

	for (i = 0; i < size; i++) {
		if (font->cache[i].cached) {
			ttf_flush_glyph(&font->cache[i]);
		}
	}
	if (font->scratch.cached) {
		ttf_flush_glyph(&font->scratch);
	}
}

static int
ttf_load_glyph(ttf_font *font, Uint16 ch, struct cached_glyph *cached,
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
		error_set("error loading glyph");
		return (-1);
	}

	glyph = face->glyph;
	metrics = &glyph->metrics;
	outline = &glyph->outline;

	if ((want & CACHED_METRICS) && !(cached->stored & CACHED_METRICS)) {
		/* Get the bounding box. */
		cached->minx = FT_FLOOR(metrics->horiBearingX);
		cached->maxx = cached->minx + FT_CEIL(metrics->width);
		cached->maxy = FT_FLOOR(metrics->horiBearingY);
		cached->miny = cached->maxy - FT_CEIL(metrics->height);
		cached->yoffset = font->ascent - cached->maxy;
		cached->advance = FT_CEIL(metrics->horiAdvance);

#if 0
		printf("min %dx%d, max %dx%d, yoffs %d, advance %d\n",
		    cached->minx, cached->miny,
		    cached->maxx, cached->maxy,
		    cached->yoffset,
		    cached->advance);
#endif

		/* Adjust for bold and italic text. */
		if (font->style & TTF_STYLE_BOLD) {
			cached->maxx += font->glyph_overhang;
		}
		if (font->style & TTF_STYLE_ITALIC) {
			cached->maxx += (int)ceil(font->glyph_italics);
		}
		cached->stored |= CACHED_METRICS;
	}

	if (((want & CACHED_BITMAP) && !(cached->stored & CACHED_BITMAP)) ||
	    ((want & CACHED_PIXMAP) && !(cached->stored & CACHED_PIXMAP))) {
		int i, mono = (want & CACHED_BITMAP);
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
				error_set("error drawing mono glyph");
				return (-1);
			}
		} else {
			if (FT_Render_Glyph(glyph, ft_render_mode_normal)
			    != 0) {
				error_set("error rendering normal glyph");
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
		if (mono) {
			dst->pitch *= 8;
		}

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

#if 0
		printf("copying: %s font, pitch %d, width %d, rows %d\n",
		    mono ? "mono" : "normal",
		    dst->pitch,
		    dst->width, dst->rows);
#endif
		if (dst->rows != 0) {
			dst->buffer = Malloc(dst->pitch * dst->rows);
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
			cached->stored |= CACHED_BITMAP;
		} else {
			cached->stored |= CACHED_PIXMAP;
		}
	}

	/* We're done, mark this glyph cached. */
	cached->cached = ch;
	return (0);
}

static int
ttf_find_glyph(ttf_font *font, Uint16 ch, int want)
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

static __inline__ Uint16 *
ASCII_to_UNICODE(Uint16 *unicode, char *text, int len)
{
	int i;

	for (i = 0; i < len; ++i) {
		unicode[i] = ((unsigned char *)text)[i];
	}
	unicode[i] = 0;

	return (unicode);
}

static __inline__ Uint16 *
UTF8_to_UNICODE(Uint16 *unicode, char *utf8, int len)
{
	int i, j;
	Uint16 ch;

	for (i = 0, j = 0; i < len; ++i, ++j) {
		ch = ((unsigned char *)utf8)[i];
		if (ch >= 0xF0) {
			ch  =  (Uint16)(utf8[i]&0x07) << 18;
			ch |=  (Uint16)(utf8[++i]&0x3F) << 12;
			ch |=  (Uint16)(utf8[++i]&0x3F) << 6;
			ch |=  (Uint16)(utf8[++i]&0x3F);
		} else if (ch >= 0xE0) {
			ch  =  (Uint16)(utf8[i]&0x3F) << 12;
			ch |=  (Uint16)(utf8[++i]&0x3F) << 6;
			ch |=  (Uint16)(utf8[++i]&0x3F);
		} else if (ch >= 0xC0) {
			ch  =  (Uint16)(utf8[i]&0x3F) << 6;
			ch |=  (Uint16)(utf8[++i]&0x3F);
		}
		unicode[j] = ch;
	}
	unicode[j] = 0;

	return (unicode);
}

int
ttf_font_height(ttf_font *font)
{
	return (font->height);
}

int
ttf_font_ascent(ttf_font *font)
{
       return (font->ascent);
}

int
ttf_font_descent(ttf_font *font)
{
	return (font->descent);
}

int
ttf_font_line_skip(ttf_font *font)
{
	return (font->lineskip);
}

long
ttf_font_faces(ttf_font *font)
{
	return (font->face->num_faces);
}

int
ttf_font_face_fixed_width(ttf_font *font)
{
	return (FT_IS_FIXED_WIDTH(font->face));
}

char *
ttf_font_face_family(ttf_font *font)
{
	return (font->face->family_name);
}

char *
ttf_font_face_style(ttf_font *font)
{
	return (font->face->style_name);
}

int
ttf_glyph_metrics(ttf_font *font, Uint16 ch, int *minx,
    int *maxx, int *miny, int *maxy, int *advance)
{
	if (ttf_find_glyph(font, ch, CACHED_METRICS) != 0) {
		return (-1);
	}
	if (minx)	*minx = font->current->minx;
	if (maxx)	*maxx = font->current->maxx;
	if (miny)	*miny = font->current->miny;
	if (maxy)	*maxy = font->current->maxy;
	if (advance)	*advance = font->current->advance;
	return (0);
}

int
ttf_size_text(ttf_font *font, char *text, int *w, int *h)
{
	Uint16 *unicode_text;
	size_t unicode_len;
	int status;

	/* Copy the Latin-1 text to a UNICODE text buffer. */
	unicode_len = strlen(text);
	unicode_text = Malloc((unicode_len + 1) * (sizeof *unicode_text));
	ASCII_to_UNICODE(unicode_text, text, unicode_len);

	/* Render the new text. */
	status = ttf_size_unicode(font, unicode_text, w, h);

	free(unicode_text);
	return (status);
}

int
ttf_size_utf8(ttf_font *font, char *text, int *w, int *h)
{
	Uint16 *unicode_text;
	size_t unicode_len;
	int status;

	/* Copy the UTF-8 text to a UNICODE text buffer. */
	unicode_len = strlen(text);
	unicode_text = Malloc((unicode_len + 1) * sizeof(*unicode_text));
	UTF8_to_UNICODE(unicode_text, text, unicode_len);

	/* Render the new text. */
	status = ttf_size_unicode(font, unicode_text, w, h);

	free(unicode_text);
	return (status);
}

int
ttf_size_unicode(ttf_font *font, Uint16 *text, int *w, int *h)
{
	int status;
	Uint16 *ch;
	int x, z;
	int minx, maxx;
	int miny, maxy;
	struct cached_glyph *glyph;

	status = 0;
	minx = maxx = 0;
	miny = maxy = 0;

	/* Load each character and sum it's bounding box. */
	x = 0;
	for (ch = text; *ch != '\0'; ch++) {
		if (ttf_find_glyph(font, *ch, CACHED_METRICS) != 0) {
			return (-1);
		}
		glyph = font->current;

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

		if (glyph->miny < miny) {
			miny = glyph->miny;
		}
		if (glyph->maxy > maxy) {
			maxy = glyph->maxy;
		}
	}

	/* Fill the bounds rectangle. */
	if (w) {
		*w = (maxx - minx);
	}
	if (h) {
#if 1 /* This is correct, but breaks many applications. */
		*h = (maxy - miny);
#else
		*h = font->height;
#endif
	}
	return status;
}

/* Convert the Latin-1 text to UNICODE and render it. */
SDL_Surface *
ttf_render_text_solid(ttf_font *font, char *text, SDL_Color fg)
{
	SDL_Surface *textsu;
	Uint16 *unicode_text;
	size_t unicode_len;

	/* Copy the Latin-1 text to a UNICODE text buffer. */
	unicode_len = strlen(text);
	unicode_text = Malloc((unicode_len + 1) * sizeof(*unicode_text));
	ASCII_to_UNICODE(unicode_text, text, unicode_len);

	/* Render the new text. */
	textsu = ttf_render_unicode_solid(font, unicode_text, fg);

	free(unicode_text);
	return (textsu);
}

/* Convert the UTF-8 text to UNICODE and render it. */
SDL_Surface *
ttf_render_utf8_solid(ttf_font *font, char *text, SDL_Color fg)
{
	SDL_Surface *textsu;
	Uint16 *unicode_text;
	size_t unicode_len;

	/* Copy the UTF-8 text to a UNICODE text buffer. */
	unicode_len = strlen(text);
	unicode_text = Malloc((unicode_len + 1) * sizeof(*unicode_text));
	UTF8_to_UNICODE(unicode_text, text, unicode_len);

	/* Render the new text. */
	textsu = ttf_render_unicode_solid(font, unicode_text, fg);

	free(unicode_text);
	return (textsu);
}

SDL_Surface *
ttf_render_unicode_solid(ttf_font *font, Uint16 *text, SDL_Color fg)
{
	int xstart;
	int width, height;
	SDL_Surface *textsu;
	SDL_Palette *palette;
	Uint16 *ch;
	Uint8 *src, *dst;
	int row, col;
	struct cached_glyph *glyph;

	/* Get the dimensions of the text surface. */
	if ((ttf_size_unicode(font, text, &width, NULL) < 0) || !width) {
		error_set("text has zero width");
		return (NULL);
	}
	height = font->height;

	/* Create the target surface. */
	textsu = SDL_CreateRGBSurface(SDL_SWSURFACE,
	    width, height, 8,
	    0, 0, 0, 0);
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
	if (SDL_SetColorKey(textsu, SDL_SRCCOLORKEY|SDL_RLEACCEL, 0) != 0) {
		error_set("SDL_SetColorKey: %s", SDL_GetError());
		goto fail1;
	}

	/* Load and render each character. */
	xstart = 0;

	for (ch = text; *ch; ch++) {
		FT_Bitmap *current = NULL;

		if (ttf_find_glyph(font, *ch, CACHED_METRICS|CACHED_BITMAP)
		    != 0) {
		    	goto fail1;
		}
		glyph = font->current;

		current = &glyph->bitmap;
		for (row = 0; row < current->rows; ++row) {
			dst = (Uint8 *)textsu->pixels +
				(row + glyph->yoffset)*textsu->pitch +
				xstart + glyph->minx;
			src = current->buffer + row*current->pitch;

			for (col = current->width; col > 0; --col) {
				*dst++ |= *src++;
			}
		}

		xstart += glyph->advance;
		if (font->style & TTF_STYLE_BOLD) {
			xstart += font->glyph_overhang;
		}
	}

	/* Handle the underline style. */
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

SDL_Surface *
ttf_render_glyph_solid(ttf_font *font, Uint16 ch, SDL_Color fg)
{
	SDL_Surface *textsu;
	SDL_Palette *palette;
	Uint8 *src, *dst;
	int row;
	struct cached_glyph *glyph;

	/* Get the glyph itself */
	if (ttf_find_glyph(font, ch, CACHED_METRICS|CACHED_BITMAP) != 0) {
		fatal("ttf_find_glyph: %s", error_get());
	}
	glyph = font->current;

	/* Create the target surface */
	textsu = SDL_CreateRGBSurface(SDL_SWSURFACE,
	    glyph->pixmap.width, glyph->pixmap.rows, 8,
	    0, 0, 0, 0);
	if (textsu == NULL) {
		error_set("SDL_CreateRGBSurface: %s", SDL_GetError());
		return (NULL);
	}

	/* Fill the palette with the foreground color */
	palette = textsu->format->palette;
	palette->colors[0].r = 255 - fg.r;
	palette->colors[0].g = 255 - fg.g;
	palette->colors[0].b = 255 - fg.b;
	palette->colors[1].r = fg.r;
	palette->colors[1].g = fg.g;
	palette->colors[1].b = fg.b;
	if (SDL_SetColorKey(textsu, SDL_SRCCOLORKEY|SDL_RLEACCEL, 0) != 0) {
		error_set("SDL_SetColorKey: %s", SDL_GetError());
		SDL_FreeSurface(textsu);
		return (NULL);
	}


	/* Copy the character from the pixmap */
	src = glyph->pixmap.buffer;
	dst = (Uint8 *)textsu->pixels;
	for (row = 0; row < textsu->h; row++) {
		memcpy(dst, src, glyph->pixmap.pitch);
		src += glyph->pixmap.pitch;
		dst += textsu->pitch;
	}

	/* Handle the underline style */
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
}

void
ttf_set_font_style(ttf_font *font, int style)
{
	font->style = style;
	ttf_flush_cache(font);
}

int
ttf_get_font_style(ttf_font *font)
{
	return (font->style);
}

