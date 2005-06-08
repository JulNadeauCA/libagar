/*	$Csoft: bitmap.c,v 1.22 2005/01/28 12:49:51 vedge Exp $	*/

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

const AG_WidgetOps bitmap_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		AG_WidgetDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_BitmapDraw,
	AG_BitmapScale
};

AG_Bitmap *
AG_BitmapNew(void *parent)
{
	AG_Bitmap *bmp;

	bmp = Malloc(sizeof(AG_Bitmap), M_OBJECT);
	AG_BitmapInit(bmp);
	AG_ObjectAttach(parent, bmp);
	return (bmp);
}

void
AG_BitmapInit(AG_Bitmap *bmp)
{
	AG_WidgetInit(bmp, "bitmap", &bitmap_ops, 0);
	AG_WidgetMapSurface(bmp, NULL);
	AGWIDGET(bmp)->flags |= AG_WIDGET_CLIPPING|AG_WIDGET_WFILL|
	                        AG_WIDGET_HFILL;
	bmp->pre_w = 320;
	bmp->pre_h = 240;
}

void
AG_BitmapPrescale(AG_Bitmap *bmp, int w, int h)
{
	bmp->pre_w = w;
	bmp->pre_h = h;
}

void
AG_BitmapScale(void *p, int rw, int rh)
{
	AG_Bitmap *bmp = p;
	
	if (rw == -1 && rh == -1) {
		AGWIDGET(bmp)->w = bmp->pre_w;
		AGWIDGET(bmp)->h = bmp->pre_h;
	}
}

void
AG_BitmapDraw(void *p)
{
	AG_Bitmap *bmp = p;
	
	AG_WidgetBlitSurface(bmp, 0, 0, 0);
}

/* Update the visible surface. */
void
AG_BitmapSetSurface(AG_Bitmap *bmp, SDL_Surface *su)
{
	AG_WidgetReplaceSurface(bmp, 0, su);
}

