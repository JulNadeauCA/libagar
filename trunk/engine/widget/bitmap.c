/*	$Csoft: bitmap.c,v 1.13 2003/04/12 01:41:31 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
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

#include <engine/widget/region.h>

#include "bitmap.h"

const struct widget_ops bitmap_ops = {
	{
		NULL,		/* init */
		bitmap_destroy,
		NULL,		/* load */
		NULL		/* save */
	},
	bitmap_draw,
	NULL		/* update */
};

struct bitmap *
bitmap_new(struct region *reg, int w, int h)
{
	struct bitmap *bitmap;

	bitmap = Malloc(sizeof(struct bitmap));
	bitmap_init(bitmap, w, h);
	region_attach(reg, bitmap);
	return (bitmap);
}

void
bitmap_init(struct bitmap *bitmap, int w, int h)
{
	widget_init(&bitmap->wid, "bitmap", &bitmap_ops, w, h);

	bitmap->surface = NULL;
	bitmap->surface_s = NULL;
	event_new(bitmap, "widget-scaled", bitmap_scaled, NULL);
}

/* Update the visible surface. */
void
bitmap_set_surface(struct bitmap *bmp, SDL_Surface *su)
{
	/* Allocate a copy of the original. */
	if (bmp->surface != NULL)
		SDL_FreeSurface(bmp->surface);
	bmp->surface = view_scale_surface(su, su->w, su->h);

	/* Update the scaled version. */
	event_post(bmp, "widget-scaled", NULL);
}

void
bitmap_scaled(int argc, union evarg *argv)
{
	struct bitmap *bmp = argv[0].p;

	if (bmp->surface == NULL)
		return;

	if (WIDGET(bmp)->rw == -1)
		WIDGET(bmp)->w = bmp->surface->w;
	if (WIDGET(bmp)->rh == -1)
		WIDGET(bmp)->h = bmp->surface->h;

	if (bmp->surface_s != NULL)
		SDL_FreeSurface(bmp->surface_s);
	bmp->surface_s = view_scale_surface(bmp->surface,
	    WIDGET(bmp)->w, WIDGET(bmp)->h);
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

	if (bmp->surface_s != NULL) {
		SDL_FreeSurface(bmp->surface_s);
	}
	widget_destroy(bmp);
}

