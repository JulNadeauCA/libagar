/*	$Csoft: ttf.c,v 1.9 2002/12/28 10:12:17 vedge Exp $	*/
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

#include <config/floating_point.h>
#include <engine/debug.h>

#ifdef FLOATING_POINT
#include <math.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <freetype/freetype.h>
#include <freetype/ftoutln.h>

#include <SDL.h>

#include <engine/error.h>

#include "ttf.h"

/* FIXME: Right now we assume the gray-scale renderer Freetype is using
   supports 256 shades of gray, but we should instead key off of num_grays
   in the result FT_Bitmap after the FT_Render_Glyph() call. */
#define NUM_GRAYS       256

/* Handy routines for converting from fixed point */
#define FT_FLOOR(X)	((X & -64) / 64)
#define FT_CEIL(X)	(((X + 63) & -64) / 64)

#define CACHED_METRICS	0x10
#define CACHED_BITMAP	0x01
#define CACHED_PIXMAP	0x02

struct cached_glyph {
	int	stored;
	FT_UInt	index;
	FT_Bitmap bitmap;
	FT_Bitmap pixmap;
	int	minx, maxx;
	int	miny, maxy;
	int	yoffset;
	int	advance;
	FT_UInt	cached;
};

struct _TTF_Font {
	/* Freetype2 maintains all sorts of useful info itself */
	FT_Face	face;

	int	height;
	int	ascent;
	int	descent;
	int	lineskip;
	int	style;
	int	glyph_overhang;

#ifdef FLOATING_POINT
	float	glyph_italics;
#endif
	int	underline_offset;
	int	underline_height;

	/* Cache for style-transformed glyphs */
	struct	 cached_glyph *current;
	struct	 cached_glyph cache[256];
	struct	 cached_glyph scratch;
};

/* The FreeType font engine/library */
static FT_Library library;
static int TTF_initialized = 0;

int
TTF_Init(void)
{
	int status;
	FT_Error error;

	status = 0;
	error = FT_Init_FreeType(&library);
	if (error) {
		error_set("cannot initialize FreeType");
		status = -1;
	} else {
		TTF_initialized = 1;
	}
	return (status);
}

TTF_Font *
TTF_OpenFontIndex(const char *file, int ptsize, long index)
{
	TTF_Font* font;
	FT_Error error;
	FT_Face face;
	FT_Fixed scale;

	font = emalloc(sizeof(TTF_Font));
	memset(font, 0, sizeof(TTF_Font));

	error = FT_New_Face(library, file, 0, &font->face);
	if (error != 0) {
		error_set("could not load font \"%s\"", file);
		free(font);
		return (NULL);
	}
	if (index != 0) {
		if (font->face->num_faces > index) {
		  	FT_Done_Face(font->face);
			error = FT_New_Face(library, file, index, &font->face);
			if (error) {
				error_set("could not get font face");
				free (font);
				return (NULL);
			}
		} else {
			error_set("no such font face");
			free(font);
			return (NULL);
		}
	}
	face = font->face;

	if (!FT_IS_SCALABLE(face)) {
		error_set("font face is not scalable");
		TTF_CloseFont(font);
		return (NULL);
	}

	error = FT_Set_Char_Size(font->face, 0, ptsize * 64, 0, 0);
	if (error != 0) {
		error_set("could not set font size");
		TTF_CloseFont(font);
		return (NULL);
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

#ifdef FLOATING_POINT
	/* x offset = cos(((90.0-12)/360)*2*M_PI), or 12 degree angle */
	font->glyph_italics = 0.207f;
	font->glyph_italics *= font->height;
#endif

	return (font);
}

TTF_Font *
TTF_OpenFont(const char *file, int ptsize)
{
	return (TTF_OpenFontIndex(file, ptsize, 0));
}

static void
Flush_Glyph(struct cached_glyph *glyph)
{
	glyph->stored = 0;
	glyph->index = 0;
	if (glyph->bitmap.buffer) {
		free(glyph->bitmap.buffer);
		glyph->bitmap.buffer = 0;
	}
	if (glyph->pixmap.buffer) {
		free(glyph->pixmap.buffer);
		glyph->pixmap.buffer = 0;
	}
	glyph->cached = 0;
}
	
static void
Flush_Cache(TTF_Font *font)
{
	int size = sizeof(font->cache) / sizeof(font->cache[0]);
	int i;

	for (i = 0; i < size; ++i) {
		if (font->cache[i].cached) {
			Flush_Glyph(&font->cache[i]);
		}
	}

	if (font->scratch.cached) {
		Flush_Glyph(&font->scratch);
	}
}

static FT_Error
Load_Glyph(TTF_Font *font, FT_UInt ch, struct cached_glyph *cached, int want)
{
	FT_Face face;
	FT_Error error;
	FT_GlyphSlot glyph;
	FT_Glyph_Metrics *metrics;
	FT_Outline *outline;

	face = font->face;
#ifndef FLOATING_POINT
	if (font->style & TTF_STYLE_ITALIC) {
		fatal("TTF_STYLE_ITALIC needs FLOATING_POINT\n");
	}
#endif

	if (cached->index == 0) {
		cached->index = FT_Get_Char_Index(face, ch);
	}

	error = FT_Load_Glyph(face, cached->index, FT_LOAD_DEFAULT);
	if (error != 0) {
		return (error);
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
#ifdef FLOATING_POINT
		if (font->style & TTF_STYLE_ITALIC) {
			cached->maxx += (int)ceil(font->glyph_italics);
		}
#endif
		cached->stored |= CACHED_METRICS;
	}

	if (((want & CACHED_BITMAP) && !(cached->stored & CACHED_BITMAP)) ||
	    ((want & CACHED_PIXMAP) && !(cached->stored & CACHED_PIXMAP))) {
		int i, mono = (want & CACHED_BITMAP);
		FT_Bitmap *src, *dst;

#ifdef FLOATING_POINT
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
#endif

		/* Render the glyph. */
		if (mono) {
			error = FT_Render_Glyph(glyph, ft_render_mode_mono);
		} else {
			error = FT_Render_Glyph(glyph, ft_render_mode_normal);
		}
		if (error) {
			return (error);
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
#ifdef FLOATING_POINT
		if (font->style & TTF_STYLE_ITALIC) {
			int bump;
			
			bump = (int)ceil(font->glyph_italics);
			dst->pitch += bump;
			dst->width += bump;
		}
#endif

#if 0
		printf("copying: %s font, pitch %d, width %d, rows %d\n",
		    mono ? "mono" : "normal",
		    dst->pitch,
		    dst->width, dst->rows);
#endif
		if (dst->rows != 0) {
			dst->buffer = emalloc(dst->pitch * dst->rows);
			memset(dst->buffer, 0, dst->pitch * dst->rows);

#if 1
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
#endif
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

static FT_Error
Find_Glyph(TTF_Font *font, FT_UInt ch, int want)
{
	int retval = 0;

	if (ch < 256) {
		font->current = &font->cache[ch];
	} else {
		if (font->scratch.cached != ch) {
			Flush_Glyph(&font->scratch);
		}
		font->current = &font->scratch;
	}
	if ((font->current->stored & want) != want) {
		retval = Load_Glyph(font, ch, font->current, want);
	}
	return (retval);
}

void
TTF_CloseFont(TTF_Font *font)
{
	Flush_Cache(font);
	FT_Done_Face(font->face);
	free(font);
}

static __inline__ FT_UInt *
ASCII_to_UNICODE(FT_UInt *unicode, char *text, int len)
{
	int i;

	for (i = 0; i < len; ++i) {
#if defined(__OpenBSD__) && defined(__alpha__)
		/*
		 * XXX work around mysterious freetype crash seen on
		 * OpenBSD/alpha as of 3.2.
		 */
		if (text[i] == 'N' || text[i] == 'K') {
			unicode[i] = '_';
			continue;
		}
#endif
		unicode[i] = ((unsigned char *)text)[i];
	}
	unicode[i] = 0;

	return (unicode);
}

static __inline__ FT_UInt *
UTF8_to_UNICODE(FT_UInt *unicode, char *utf8, int len)
{
	int i, j;
	FT_UInt ch;

	for (i = 0, j = 0; i < len; ++i, ++j) {
		ch = ((unsigned char *)utf8)[i];
		if (ch >= 0xF0) {
			ch  =  (FT_UInt)(utf8[i]&0x07) << 18;
			ch |=  (FT_UInt)(utf8[++i]&0x3F) << 12;
			ch |=  (FT_UInt)(utf8[++i]&0x3F) << 6;
			ch |=  (FT_UInt)(utf8[++i]&0x3F);
		} else if (ch >= 0xE0) {
			ch  =  (FT_UInt)(utf8[i]&0x3F) << 12;
			ch |=  (FT_UInt)(utf8[++i]&0x3F) << 6;
			ch |=  (FT_UInt)(utf8[++i]&0x3F);
		} else if (ch >= 0xC0) {
			ch  =  (FT_UInt)(utf8[i]&0x3F) << 6;
			ch |=  (FT_UInt)(utf8[++i]&0x3F);
		}
		unicode[j] = ch;
	}
	unicode[j] = 0;

	return (unicode);
}

int
TTF_FontHeight(TTF_Font *font)
{
	return (font->height);
}

int
TTF_FontAscent(TTF_Font *font)
{
       return (font->ascent);
}

int
TTF_FontDescent(TTF_Font *font)
{
	return (font->descent);
}

int
TTF_FontLineSkip(TTF_Font *font)
{
	return (font->lineskip);
}

long
TTF_FontFaces(TTF_Font *font)
{
	return (font->face->num_faces);
}

int
TTF_FontFaceIsFixedWidth(TTF_Font *font)
{
	return (FT_IS_FIXED_WIDTH(font->face));
}

char *
TTF_FontFaceFamilyName(TTF_Font *font)
{
	return (font->face->family_name);
}

char *
TTF_FontFaceStyleName(TTF_Font *font)
{
	return (font->face->style_name);
}

int
TTF_GlyphMetrics(TTF_Font *font, unsigned int ch, int *minx, int *maxx, int *miny,
    int *maxy, int *advance)
{
	FT_Error error;

	error = Find_Glyph(font, ch, CACHED_METRICS);
	if (error) {
		return (-1);
	}

	if (minx)
		*minx = font->current->minx;
	if (maxx)
		*maxx = font->current->maxx;
	if (miny)
		*miny = font->current->miny;
	if (maxy)
		*maxy = font->current->maxy;
	if (advance)
		*advance = font->current->advance;

	return (0);
}

int
TTF_SizeText(TTF_Font *font, char *text, int *w, int *h)
{
	FT_UInt *unicode_text;
	int unicode_len;
	int status;

	/* Copy the Latin-1 text to a UNICODE text buffer. */
	unicode_len = strlen(text);
	unicode_text = emalloc((unicode_len+1)*(sizeof *unicode_text));
	ASCII_to_UNICODE(unicode_text, text, unicode_len);

	/* Render the new text. */
	status = TTF_SizeUNICODE(font, unicode_text, w, h);

	free(unicode_text);
	return (status);
}

int
TTF_SizeUTF8(TTF_Font *font, char *text, int *w, int *h)
{
	FT_UInt *unicode_text;
	int unicode_len;
	int status;

	/* Copy the UTF-8 text to a UNICODE text buffer. */
	unicode_len = strlen(text);
	unicode_text = emalloc((unicode_len + 1) * sizeof(FT_UInt));
	UTF8_to_UNICODE(unicode_text, text, unicode_len);

	/* Render the new text. */
	status = TTF_SizeUNICODE(font, unicode_text, w, h);

	free(unicode_text);
	return (status);
}

int
TTF_SizeUNICODE(TTF_Font *font, unsigned int *text, int *w, int *h)
{
	int status;
	FT_UInt *ch;
	int x, z;
	int minx, maxx;
	int miny, maxy;
	struct cached_glyph *glyph;
	FT_Error error;

	/* Initialize everything to 0. */
	if (!TTF_initialized) {
		return (-1);
	}
	status = 0;
	minx = maxx = 0;
	miny = maxy = 0;

	/* Load each character and sum it's bounding box. */
	x = 0;
	for (ch = text; *ch; ch++) {
		error = Find_Glyph(font, *ch, CACHED_METRICS);
		if (error) {
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
TTF_RenderText_Solid(TTF_Font *font, char *text, SDL_Color fg)
{
	SDL_Surface *textbuf;
	FT_UInt *unicode_text;
	int unicode_len;

	/* Copy the Latin-1 text to a UNICODE text buffer. */
	unicode_len = strlen(text);
	unicode_text = emalloc((unicode_len + 1) * sizeof(FT_UInt));
	ASCII_to_UNICODE(unicode_text, text, unicode_len);

	/* Render the new text. */
	textbuf = TTF_RenderUNICODE_Solid(font, unicode_text, fg);

	free(unicode_text);
	return (textbuf);
}

/* Convert the UTF-8 text to UNICODE and render it. */
SDL_Surface *
TTF_RenderUTF8_Solid(TTF_Font *font, char *text, SDL_Color fg)
{
	SDL_Surface *textbuf;
	FT_UInt *unicode_text;
	int unicode_len;

	/* Copy the UTF-8 text to a UNICODE text buffer. */
	unicode_len = strlen(text);
	unicode_text = emalloc((unicode_len + 1) * sizeof(FT_UInt));
	UTF8_to_UNICODE(unicode_text, text, unicode_len);

	/* Render the new text. */
	textbuf = TTF_RenderUNICODE_Solid(font, unicode_text, fg);

	free(unicode_text);
	return (textbuf);
}

SDL_Surface *
TTF_RenderUNICODE_Solid(TTF_Font *font, unsigned int *text, SDL_Color fg)
{
	int xstart;
	int width, height;
	SDL_Surface *textbuf;
	SDL_Palette *palette;
	FT_UInt *ch;
	Uint8 *src, *dst;
	int row, col;
	struct cached_glyph *glyph;
	FT_Error error;

	/* Get the dimensions of the text surface. */
	if ((TTF_SizeUNICODE(font, text, &width, NULL) < 0) || !width) {
		fatal("text has zero width\n");
		return (NULL);
	}
	height = font->height;

	/* Create the target surface. */
	textbuf = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height,
	    8, 0, 0, 0, 0);
	if (textbuf == NULL) {
		fatal("SDL_CreateRGBSurface: %s\n", SDL_GetError());
		return (NULL);
	}

	/* Fill the palette with the foreground color. */
	palette = textbuf->format->palette;
	palette->colors[0].r = 255 - fg.r;
	palette->colors[0].g = 255 - fg.g;
	palette->colors[0].b = 255 - fg.b;
	palette->colors[1].r = fg.r;
	palette->colors[1].g = fg.g;
	palette->colors[1].b = fg.b;
	SDL_SetColorKey(textbuf, SDL_SRCCOLORKEY, 0);

	/* Load and render each character. */
	xstart = 0;

	for (ch = text; *ch; ch++) {
		FT_Bitmap *current = NULL;

		error = Find_Glyph(font, *ch, CACHED_METRICS|CACHED_BITMAP);
		if (error) {
			SDL_FreeSurface(textbuf);
			/* XXX return an empty surface */
			fatal("cannot find glyph %d\n", *ch);
			return (NULL);
		}
		glyph = font->current;

		current = &glyph->bitmap;
		for (row = 0; row < current->rows; ++row) {
			dst = (Uint8 *)textbuf->pixels +
				(row + glyph->yoffset)*textbuf->pitch +
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
		if (row >= textbuf->h) {
			row = (textbuf->h-1) - font->underline_height;
		}
		dst = (Uint8 *)textbuf->pixels + row * textbuf->pitch;
		for (row = font->underline_height; row > 0; --row) {
			/* 1 because 0 is the bg color */
			memset(dst, 1, textbuf->w);
			dst += textbuf->pitch;
		}
	}
	return (textbuf);
}

SDL_Surface *
TTF_RenderGlyph_Solid(TTF_Font *font, unsigned int ch, SDL_Color fg)
{
	SDL_Surface *textbuf;
	SDL_Palette *palette;
	Uint8 *src, *dst;
	int row;
	FT_Error error;
	struct cached_glyph *glyph;

	/* Get the glyph itself */
	error = Find_Glyph(font, ch, CACHED_METRICS|CACHED_BITMAP);
	if (error) {
		fatal("cannot find glyph %d\n", ch);
		return (NULL);
	}
	glyph = font->current;

	/* Create the target surface */
	textbuf = SDL_CreateRGBSurface(SDL_SWSURFACE, glyph->pixmap.width,
	    glyph->pixmap.rows, 8, 0, 0, 0, 0);
	if (textbuf == NULL) {
		error_set("SDL_CreateRGBSurface: %s", SDL_GetError());
		return (NULL);
	}

	/* Fill the palette with the foreground color */
	palette = textbuf->format->palette;
	palette->colors[0].r = 255-fg.r;
	palette->colors[0].g = 255-fg.g;
	palette->colors[0].b = 255-fg.b;
	palette->colors[1].r = fg.r;
	palette->colors[1].g = fg.g;
	palette->colors[1].b = fg.b;
	SDL_SetColorKey(textbuf, SDL_SRCCOLORKEY, 0);

	/* Copy the character from the pixmap */
	src = glyph->pixmap.buffer;
	dst = (Uint8 *)textbuf->pixels;
	for (row = 0; row < textbuf->h; ++row) {
		memcpy(dst, src, glyph->pixmap.pitch);
		src += glyph->pixmap.pitch;
		dst += textbuf->pitch;
	}

	/* Handle the underline style */
	if (font->style & TTF_STYLE_UNDERLINE) {
		row = font->ascent - font->underline_offset - 1;
		if (row >= textbuf->h) {
			row = (textbuf->h-1) - font->underline_height;
		}
		dst = (Uint8 *)textbuf->pixels + row * textbuf->pitch;
		for (row = font->underline_height; row > 0; --row) {
			/* 1 because 0 is the bg color */
			memset(dst, 1, textbuf->w);
			dst += textbuf->pitch;
		}
	}
	return (textbuf);
}

void
TTF_SetFontStyle(TTF_Font *font, int style)
{
	font->style = style;
	Flush_Cache(font);
}

int
TTF_GetFontStyle(TTF_Font *font)
{
	return (font->style);
}

void
TTF_Quit(void)
{
	if (TTF_initialized) {
		FT_Done_FreeType(library);
	}
	TTF_initialized = 0;
}

