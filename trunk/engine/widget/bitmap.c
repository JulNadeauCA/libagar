/*	$Csoft: bitmap.c,v 1.21 2005/01/23 11:53:05 vedge Exp $	*/

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
		widget_destroy,
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
	struct bitmap *bmp;

	bmp = Malloc(sizeof(struct bitmap), M_OBJECT);
	bitmap_init(bmp);
	object_attach(parent, bmp);
	return (bmp);
}

void
bitmap_init(struct bitmap *bmp)
{
	widget_init(bmp, "bitmap", &bitmap_ops, 0);
	widget_map_surface(bmp, NULL);
	WIDGET(bmp)->flags |= WIDGET_CLIPPING|WIDGET_WFILL|WIDGET_HFILL;
	bmp->pre_w = 320;
	bmp->pre_h = 240;
}

void
bitmap_prescale(struct bitmap *bmp, int w, int h)
{
	bmp->pre_w = w;
	bmp->pre_h = h;
}

void
bitmap_scale(void *p, int rw, int rh)
{
	struct bitmap *bmp = p;
	
	if (rw == -1 && rh == -1) {
		WIDGET(bmp)->w = bmp->pre_w;
		WIDGET(bmp)->h = bmp->pre_h;
	}
}

void
bitmap_draw(void *p)
{
	struct bitmap *bmp = p;
	
	widget_blit_surface(bmp, 0, 0, 0);
}

/* Update the visible surface. */
void
bitmap_set_surface(struct bitmap *bmp, SDL_Surface *su)
{
	widget_replace_surface(bmp, 0, su);
}

