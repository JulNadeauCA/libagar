/*	$Csoft: bitmap.c,v 1.2 2002/09/06 01:28:47 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include <sys/types.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <engine/engine.h>

#include "widget.h"
#include "window.h"
#include "bitmap.h"

static const struct widget_ops bitmap_ops = {
	{
		bitmap_destroy,
		NULL,	/* load */
		NULL	/* save */
	},
	bitmap_draw,
	NULL		/* animate */
};

struct bitmap *
bitmap_new(struct region *reg, SDL_Surface *surface, int w, int h)
{
	struct bitmap *bitmap;

	bitmap = emalloc(sizeof(struct bitmap));
	bitmap_init(bitmap, surface, w, h);

	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, bitmap);
	pthread_mutex_unlock(&reg->win->lock);

	return (bitmap);
}

void
bitmap_init(struct bitmap *bitmap, SDL_Surface *surface, int w, int h)
{
	widget_init(&bitmap->wid, "bitmap", "widget", &bitmap_ops, w, h);

	bitmap->surface = surface;
	bitmap->surface_s = NULL;

	if (surface != NULL) {
		WIDGET(bitmap)->w = surface->w;
		WIDGET(bitmap)->h = surface->h;
	}

	event_new(bitmap, "widget-scaled", 0, bitmap_scaled, NULL);
}

void
bitmap_scaled(int argc, union evarg *argv)
{
	struct bitmap *bmp = argv[0].p;

	if (bmp->surface == NULL) {
		return;
	}

	if (bmp->surface_s != NULL) {
		view_unused_surface(bmp->surface_s);
	}
	bmp->surface_s = view_scale_surface(bmp->surface, WIDGET(bmp)->w,
	    WIDGET(bmp)->h);
}

void
bitmap_draw(void *p)
{
	struct bitmap *bmp = p;
	
	if (bmp->surface == NULL) {
		return;
	}
	WIDGET_DRAW(bmp, bmp->surface_s, 0, 0);
}

void
bitmap_destroy(void *p)
{
	struct bitmap *bmp = p;

	if (bmp->surface_s != NULL) {
		view_unused_surface(bmp->surface_s);
	}
}

