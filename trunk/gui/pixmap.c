/*	$Csoft: pixmap.c,v 1.23 2005/09/27 00:25:22 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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

#include <engine/loader/xcf.h>
#include <engine/widget/primitive.h>

#include "pixmap.h"

const AG_WidgetOps agPixmapOps = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		AG_WidgetDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_PixmapDraw,
	AG_PixmapScale
};

AG_Pixmap *
AG_PixmapFromSurface(void *parent, SDL_Surface *su)
{
	AG_Pixmap *px;

	px = Malloc(sizeof(AG_Pixmap), M_OBJECT);
	AG_PixmapInit(px);
	AG_ObjectAttach(parent, px);
	AG_WidgetMapSurface(px, su);
	return (px);
}

AG_Pixmap *
AG_PixmapFromSurfaceCopy(void *parent, SDL_Surface *su)
{
	AG_Pixmap *px;

	px = Malloc(sizeof(AG_Pixmap), M_OBJECT);
	AG_PixmapInit(px);
	AG_ObjectAttach(parent, px);
	AG_WidgetMapSurface(px, AG_DupSurface(su));
	return (px);
}

AG_Pixmap *
AG_PixmapFromSurfaceScaled(void *parent, SDL_Surface *su, u_int w, u_int h)
{
	AG_Pixmap *px;
	SDL_Surface *su2;

	px = Malloc(sizeof(AG_Pixmap), M_OBJECT);
	AG_PixmapInit(px);
	AG_ObjectAttach(parent, px);
	AG_ScaleSurface(su, w, h, &su2);
	AG_WidgetMapSurface(px, su2);
	return (px);
}

AG_Pixmap *
AG_PixmapFromBMP(void *parent, const char *bmpfile)
{
	AG_Pixmap *px;
	SDL_Surface *bmp;

	if ((bmp = SDL_LoadBMP(bmpfile)) == NULL) {
		AG_SetError("%s: %s", bmpfile, SDL_GetError());
		return (NULL);
	}
	px = Malloc(sizeof(AG_Pixmap), M_OBJECT);
	AG_PixmapInit(px);
	AG_ObjectAttach(parent, px);
	AG_WidgetMapSurface(px, bmp);
	return (px);
}

AG_Pixmap *
AG_PixmapFromXCF(void *parent, const char *path)
{
	AG_Object tmpObj;
	AG_Pixmap *px;
	SDL_Surface *su;
	AG_Netbuf *buf;
	u_int i;
	
	if ((buf = AG_NetbufOpen(path, "rb", AG_NETBUF_BIG_ENDIAN)) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", path, AG_GetError());
		return (NULL);
	}

	/* XXX hack */
	AG_ObjectInit(&tmpObj, "object", "tmp", NULL);
	tmpObj.gfx = AG_GfxNew(&tmpObj);
	if (AG_XCFLoad(buf, 0, tmpObj.gfx) == -1)
		goto fail;
	
	px = Malloc(sizeof(AG_Pixmap), M_OBJECT);
	AG_PixmapInit(px);
	AG_ObjectAttach(parent, px);
	for (i = 0; i < tmpObj.gfx->nsprites; i++) {
		AG_WidgetMapSurface(px, AG_DupSurface(AG_SPRITE(&tmpObj,i).su));
	}
	AG_NetbufClose(buf);
	AG_ObjectDestroy(&tmpObj);
	return (px);
fail:
	AG_NetbufClose(buf);
	AG_ObjectDestroy(&tmpObj);
	return (NULL);
}

int
AG_PixmapAddSurface(AG_Pixmap *px, SDL_Surface *su)
{
	return (AG_WidgetMapSurface(px, su));
}

int
AG_PixmapAddSurfaceCopy(AG_Pixmap *px, SDL_Surface *su)
{
	return (AG_WidgetMapSurface(px, AG_DupSurface(su)));
}

int
AG_PixmapAddSurfaceScaled(AG_Pixmap *px, SDL_Surface *su, u_int w, u_int h)
{
	SDL_Surface *su2;
	
	AG_ScaleSurface(su, w, h, &su2);
	return (AG_WidgetMapSurface(px, su2));
}

void
AG_PixmapInit(AG_Pixmap *px)
{
	AG_WidgetInit(px, "pixmap", &agPixmapOps, 0);
	AGWIDGET(px)->flags |= AG_WIDGET_CLIPPING|AG_WIDGET_WFILL|
	                       AG_WIDGET_HFILL;
	px->n = 0;
	px->s = 0;
	px->t = 0;
}

void
AG_PixmapScale(void *p, int rw, int rh)
{
	AG_Pixmap *px = p;
	
	if (rw == -1 && rh == -1) {
		if (px->n >= 0) {
			AGWIDGET(px)->w = AGWIDGET_SURFACE(px,px->n)->w;
			AGWIDGET(px)->h = AGWIDGET_SURFACE(px,px->n)->h;
		}
		return;
	}
	if (px->n >= 0) {
		AGWIDGET(px)->w = AGWIDGET_SURFACE(px,px->n)->w;
		AGWIDGET(px)->h = AGWIDGET_SURFACE(px,px->n)->h;
	}
}

void
AG_PixmapDraw(void *p)
{
	AG_Pixmap *px = p;

	if (px->n >= 0)
		AG_WidgetBlitSurface(px, px->n, px->s, px->t);
}

void
AG_PixmapSetSurface(AG_Pixmap *px, int name)
{
#ifdef DEBUG
	if (name >= AGWIDGET(px)->nsurfaces)
		fatal("no such surface: %d", name);
#endif
	px->n = name;
}

void
AG_PixmapSetCoords(AG_Pixmap *px, int s, int t)
{
	px->s = s;
	px->t = t;
}

