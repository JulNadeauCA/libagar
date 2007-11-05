/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>
#if 0
#include <core/load_xcf.h>
#endif

#include "pixmap.h"
#include "primitive.h"

AG_Pixmap *
AG_PixmapNew(void *parent, Uint flags, Uint w, Uint h)
{
	AG_Pixmap *px;

	px = Malloc(sizeof(AG_Pixmap));
	AG_PixmapInit(px, flags|AG_PIXMAP_FORCE_SIZE);
	AG_ObjectAttach(parent, px);
	AG_WidgetMapSurface(px,
	    SDL_CreateRGBSurface(SDL_SWSURFACE, 0, 0, 8, 0,0,0,0));
	px->pre_w = w;
	px->pre_h = h;
	return (px);
}

AG_Pixmap *
AG_PixmapFromSurface(void *parent, Uint flags, SDL_Surface *su)
{
	AG_Pixmap *px;

	px = Malloc(sizeof(AG_Pixmap));
	AG_PixmapInit(px, flags);
	AG_ObjectAttach(parent, px);
	AG_WidgetMapSurface(px, su);
	return (px);
}

AG_Pixmap *
AG_PixmapFromSurfaceCopy(void *parent, Uint flags, SDL_Surface *su)
{
	AG_Pixmap *px;

	px = Malloc(sizeof(AG_Pixmap));
	AG_PixmapInit(px, flags);
	AG_ObjectAttach(parent, px);
	AG_WidgetMapSurface(px, AG_DupSurface(su));
	/* XXX leak */
	return (px);
}

AG_Pixmap *
AG_PixmapFromSurfaceScaled(void *parent, Uint flags, SDL_Surface *su,
    Uint w, Uint h)
{
	AG_Pixmap *px;
	SDL_Surface *su2 = NULL;

	px = Malloc(sizeof(AG_Pixmap));
	AG_PixmapInit(px, flags);
	AG_ObjectAttach(parent, px);
	AG_ScaleSurface(su, w, h, &su2);
	AG_WidgetMapSurface(px, su2);
	return (px);
}

AG_Pixmap *
AG_PixmapFromBMP(void *parent, Uint flags, const char *bmpfile)
{
	AG_Pixmap *px;
	SDL_Surface *bmp;

	if ((bmp = SDL_LoadBMP(bmpfile)) == NULL) {
		AG_SetError("%s: %s", bmpfile, SDL_GetError());
		return (NULL);
	}
	px = Malloc(sizeof(AG_Pixmap));
	AG_PixmapInit(px, flags);
	AG_ObjectAttach(parent, px);
	AG_WidgetMapSurface(px, bmp);
	return (px);
}

#if 0
AG_Pixmap *
AG_PixmapFromXCF(void *parent, Uint flags, const char *path)
{
	AG_Object tmpObj;
	AG_Pixmap *px;
	SDL_Surface *su;
	AG_DataSource *ds;
	Uint i;
	
	if ((ds = AG_OpenFile(path, "rb")) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "%s: %s", path, AG_GetError());
		return (NULL);
	}

	/* XXX hack */
	AG_ObjectInit(&tmpObj, "tmp", NULL);
	tmpObj.gfx = AG_GfxNew(&tmpObj);
	if (AG_XCFLoad(ds, 0, tmpObj.gfx) == -1)
		goto fail;
	
	px = Malloc(sizeof(AG_Pixmap));
	AG_PixmapInit(px, flags);
	AG_ObjectAttach(parent, px);
	for (i = 0; i < tmpObj.gfx->nsprites; i++) {
		AG_WidgetMapSurface(px, AG_DupSurface(AG_SPRITE(&tmpObj,i).su));
	}
	AG_CloseFile(ds);
	AG_ObjectDestroy(&tmpObj);
	return (px);
fail:
	AG_CloseFile(ds);
	AG_ObjectDestroy(&tmpObj);
	return (NULL);
}
#endif

int
AG_PixmapAddSurface(AG_Pixmap *px, SDL_Surface *su)
{
	return (AG_WidgetMapSurface(px, su));
}

int
AG_PixmapAddSurfaceFromBMP(AG_Pixmap *px, const char *path)
{
	SDL_Surface *bmp;

	if ((bmp = SDL_LoadBMP(path)) == NULL) {
		return (-1);
	}
	return AG_WidgetMapSurface(px, bmp);
}

int
AG_PixmapAddSurfaceCopy(AG_Pixmap *px, SDL_Surface *su)
{
	return (AG_WidgetMapSurface(px, AG_DupSurface(su)));
}

int
AG_PixmapAddSurfaceScaled(AG_Pixmap *px, SDL_Surface *su, Uint w, Uint h)
{
	SDL_Surface *su2 = NULL;
	
	AG_ScaleSurface(su, w, h, &su2);
	return (AG_WidgetMapSurface(px, su2));
}

void
AG_PixmapReplaceSurfaceScaled(AG_Pixmap *px, SDL_Surface *su, Uint w, Uint h)
{
	SDL_Surface *su2 = NULL;

	AG_ScaleSurface(su, w, h, &su2);
	AG_WidgetReplaceSurface(px, px->n, su2);
}

void
AG_PixmapInit(AG_Pixmap *px, Uint flags)
{
	Uint wflags = AG_WIDGET_CLIPPING;

	if (flags & AG_PIXMAP_HFILL) { wflags |= AG_WIDGET_HFILL; }
	if (flags & AG_PIXMAP_VFILL) { wflags |= AG_WIDGET_VFILL; }

	AG_WidgetInit(px, &agPixmapOps, wflags);
	px->flags = flags;
	px->n = 0;
	px->s = 0;
	px->t = 0;
	px->pre_w = 64;
	px->pre_h = 64;
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Pixmap *px = p;

	if ((px->flags & AG_PIXMAP_FORCE_SIZE) == 0) {
		r->w = WSURFACE(px,px->n)->w;
		r->h = WSURFACE(px,px->n)->h;
	} else {
		r->w = px->pre_w;
		r->h = px->pre_h;
	}
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	return (a->w < 1 || a->h < 1) ? -1 : 0;
}

static void
Draw(void *p)
{
	AG_Pixmap *px = p;

	if (px->n >= 0)
		AG_WidgetBlitSurface(px, px->n, px->s, px->t);
}

const AG_WidgetOps agPixmapOps = {
	{
		"AG_Widget:AG_Pixmap",
		sizeof(AG_Pixmap),
		{ 0,0 },
		NULL,		/* init */
		NULL,		/* free */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
