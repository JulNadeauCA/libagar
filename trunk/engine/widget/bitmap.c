/*	$Csoft: bitmap.c,v 1.19 2004/03/18 21:27:48 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <engine/engine.h>
#include <engine/view.h>

#include "bitmap.h"

const struct widget_ops bitmap_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		bitmap_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	bitmap_draw,
	bitmap_scale
};

struct bitmap *
bitmap_new(void *parent)
{
	struct bitmap *bitmap;

	bitmap = Malloc(sizeof(struct bitmap), M_OBJECT);
	bitmap_init(bitmap);
	object_attach(parent, bitmap);
	return (bitmap);
}

void
bitmap_init(struct bitmap *bmp)
{
	widget_init(bmp, "bitmap", &bitmap_ops, 0);

	bmp->surface = NULL;
	bmp->surface_s = NULL;
}

void
bitmap_scale(void *p, int rw, int rh)
{
	struct bitmap *bmp = p;

	if (rw == -1 && rh == -1) {
		WIDGET(bmp)->w = bmp->surface != NULL ? bmp->surface->w : 32;
		WIDGET(bmp)->h = bmp->surface != NULL ? bmp->surface->h : 32;
	}

	if (bmp->surface_s != NULL) {
		SDL_FreeSurface(bmp->surface_s);
	}
	bmp->surface_s = (bmp->surface != NULL) ?
	    view_scale_surface(bmp->surface, WIDGET(bmp)->w, WIDGET(bmp)->h) :
	    NULL;
}

void
bitmap_draw(void *p)
{
	struct bitmap *bmp = p;
	
	if (bmp->surface_s != NULL) {
		widget_blit(bmp, bmp->surface_s, 0, 0);
	}
}

void
bitmap_destroy(void *p)
{
	struct bitmap *bmp = p;

	if (bmp->surface != NULL)
		SDL_FreeSurface(bmp->surface);
	if (bmp->surface_s != NULL)
		SDL_FreeSurface(bmp->surface_s);

	widget_destroy(bmp);
}

/* Update the visible surface. */
void
bitmap_set_surface(struct bitmap *bmp, SDL_Surface *su)
{
	if (bmp->surface != NULL)
		SDL_FreeSurface(bmp->surface);

	bmp->surface = view_copy_surface(su);
	bitmap_scale(bmp, 0, 0);
}

