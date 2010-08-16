/*
 * Copyright (c) 2005-2010 Hypertriton, Inc. <http://hypertriton.com/>
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
#include "load_xcf.h"
#endif
#include "pixmap.h"
#include "primitive.h"
#include "window.h"
#include "opengl.h"

AG_Pixmap *
AG_PixmapNew(void *parent, Uint flags, Uint w, Uint h)
{
	AG_Pixmap *px;

	px = Malloc(sizeof(AG_Pixmap));
	AG_ObjectInit(px, &agPixmapClass);
	px->flags |= flags;
	px->flags |= AG_PIXMAP_FORCE_SIZE;
	px->pre_w = w;
	px->pre_h = h;

	if (flags & AG_PIXMAP_HFILL) { AG_ExpandHoriz(px); }
	if (flags & AG_PIXMAP_VFILL) { AG_ExpandVert(px); }

	AG_WidgetMapSurface(px, AG_SurfaceEmpty());
	AG_ObjectAttach(parent, px);
	return (px);
}

AG_Pixmap *
AG_PixmapFromSurface(void *parent, Uint flags, AG_Surface *su)
{
	AG_Pixmap *px;

	px = Malloc(sizeof(AG_Pixmap));
	AG_ObjectInit(px, &agPixmapClass);
	px->flags |= flags;

	if (flags & AG_PIXMAP_HFILL) { AG_ExpandHoriz(px); }
	if (flags & AG_PIXMAP_VFILL) { AG_ExpandVert(px); }
	
	AG_ObjectAttach(parent, px);
	AG_WidgetMapSurface(px, su);
	return (px);
}

AG_Pixmap *
AG_PixmapFromSurfaceCopy(void *parent, Uint flags, AG_Surface *su)
{
	AG_Pixmap *px;

	px = Malloc(sizeof(AG_Pixmap));
	AG_ObjectInit(px, &agPixmapClass);
	px->flags |= flags;

	if (flags & AG_PIXMAP_HFILL) { AG_ExpandHoriz(px); }
	if (flags & AG_PIXMAP_VFILL) { AG_ExpandVert(px); }
	
	AG_ObjectAttach(parent, px);
	AG_WidgetMapSurface(px, AG_SurfaceDup(su));	/* XXX leak */
	return (px);
}

AG_Pixmap *
AG_PixmapFromSurfaceScaled(void *parent, Uint flags, AG_Surface *su,
    Uint w, Uint h)
{
	AG_Pixmap *px;
	AG_Surface *su2 = NULL;

	px = Malloc(sizeof(AG_Pixmap));
	AG_ObjectInit(px, &agPixmapClass);
	px->flags |= flags;

	if (flags & AG_PIXMAP_HFILL) { AG_ExpandHoriz(px); }
	if (flags & AG_PIXMAP_VFILL) { AG_ExpandVert(px); }
	
	AG_ObjectAttach(parent, px);
	if (AG_ScaleSurface(su, w, h, &su2) == -1) {
		AG_FatalError(NULL);
	}
	AG_WidgetMapSurface(px, su2);
	return (px);
}

AG_Pixmap *
AG_PixmapFromBMP(void *parent, Uint flags, const char *bmpfile)
{
	AG_Pixmap *px;
	AG_Surface *bmp;

	if ((bmp = AG_SurfaceFromBMP(bmpfile)) == NULL) {
		return (NULL);
	}
	px = Malloc(sizeof(AG_Pixmap));
	AG_ObjectInit(px, &agPixmapClass);
	px->flags |= flags;

	if (flags & AG_PIXMAP_HFILL) { AG_ExpandHoriz(px); }
	if (flags & AG_PIXMAP_VFILL) { AG_ExpandVert(px); }
	
	AG_ObjectAttach(parent, px);
	AG_WidgetMapSurface(px, bmp);
	return (px);
}

#ifdef HAVE_OPENGL

AG_Pixmap *
AG_PixmapFromTexture(void *parent, Uint flags, Uint name, int lod)
{
	AG_Pixmap *px;
	AG_Surface *su;
	GLint w, h;

	glBindTexture(GL_TEXTURE_2D, (GLuint)name);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, lod, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, lod, GL_TEXTURE_HEIGHT, &h);

	su = AG_SurfaceRGBA(w, h, 32, 0,
#if AG_BYTEORDER == AG_BIG_ENDIAN
		0xff000000,
		0x00ff0000,
		0x0000ff00,
		0x000000ff
#else
		0x000000ff,
		0x0000ff00,
		0x00ff0000,
		0xff000000
#endif
	);
	if (su == NULL) {
		AG_SetError("Allocating texture: %s", AG_GetError());
		glBindTexture(GL_TEXTURE_2D, 0);
		return (NULL);
	}
	glGetTexImage(GL_TEXTURE_2D, lod, GL_RGBA, GL_UNSIGNED_BYTE,
	    su->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	px = Malloc(sizeof(AG_Pixmap));
	AG_ObjectInit(px, &agPixmapClass);
	px->flags |= flags;

	if (flags & AG_PIXMAP_HFILL) { AG_ExpandHoriz(px); }
	if (flags & AG_PIXMAP_VFILL) { AG_ExpandVert(px); }
	
	AG_ObjectAttach(parent, px);
	AG_WidgetMapSurface(px, su);
	return (px);
}

#endif /* HAVE_OPENGL */

/*
 * Map an existing surface. Returned surface ID is valid as long as pixmap
 * is locked.
 */
int
AG_PixmapAddSurface(AG_Pixmap *px, AG_Surface *su)
{
	int name;

	AG_ObjectLock(px);
	name = AG_WidgetMapSurfaceNODUP(px, su);
	px->flags |= AG_PIXMAP_UPDATE;
	AG_ObjectUnlock(px);
	return (name);
}

/*
 * Map an existing surface from a BMP file. Returned surface ID is valid as
 * long as pixmap is locked.
 */
int
AG_PixmapAddSurfaceFromBMP(AG_Pixmap *px, const char *path)
{
	AG_Surface *bmp;
	int name;

	if ((bmp = AG_SurfaceFromBMP(path)) == NULL) {
		return (-1);
	}
	AG_ObjectLock(px);
	name = AG_WidgetMapSurface(px, bmp);
	px->flags |= AG_PIXMAP_UPDATE;
	AG_ObjectUnlock(px);
	return (name);
}

/*
 * Create a copy of a surface and map it. Returned surface ID is valid as
 * long as pixmap is locked.
 */
int
AG_PixmapAddSurfaceCopy(AG_Pixmap *px, AG_Surface *su)
{
	AG_Surface *dup;
	int name;

	dup = AG_SurfaceDup(su);
	AG_ObjectLock(px);
	name = AG_WidgetMapSurface(px, dup);
	px->flags |= AG_PIXMAP_UPDATE;
	AG_ObjectUnlock(px);
	return (name);
}

/*
 * Create a scaled version of a surface and map it. Returned surface ID
 * is valid as long as pixmap is locked.
 */
int
AG_PixmapAddSurfaceScaled(AG_Pixmap *px, AG_Surface *su, Uint w, Uint h)
{
	AG_Surface *scaled = NULL;
	int name;
	
	if (AG_ScaleSurface(su, w, h, &scaled) == -1) {
		AG_FatalError(NULL);
	}
	AG_ObjectLock(px);
	name = AG_WidgetMapSurface(px, scaled);
	px->flags |= AG_PIXMAP_UPDATE;
	AG_ObjectUnlock(px);
	return (name);
}

/* Replace the specified surface with a scaled version of another surface. */
void
AG_PixmapReplaceSurfaceScaled(AG_Pixmap *px, int surface_name, AG_Surface *su,
    Uint w, Uint h)
{
	AG_Surface *scaled = NULL;

	if (AG_ScaleSurface(su, w, h, &scaled) == -1) {
		AG_FatalError(NULL);
	}
	AG_ObjectLock(px);
	AG_WidgetReplaceSurface(px, surface_name, scaled);
	if (surface_name == px->n) {
		px->flags |= AG_PIXMAP_UPDATE;
		AG_Redraw(px);
	}
	AG_ObjectUnlock(px);
}

static void
Init(void *obj)
{
	AG_Pixmap *px = obj;
	
	WIDGET(px)->flags |= AG_WIDGET_TABLE_EMBEDDABLE;

	px->flags = AG_PIXMAP_UPDATE;
	px->n = 0;
	px->s = 0;
	px->t = 0;
	px->pre_w = 64;
	px->pre_h = 64;
	px->rClip = AG_RECT(0,0,0,0);
	px->sScaled = -1;

#ifdef AG_DEBUG
	AG_BindUint(px, "flags", &px->flags);
	AG_BindInt(px, "n", &px->n);
	AG_BindInt(px, "s", &px->s);
	AG_BindInt(px, "t", &px->t);
	AG_BindInt(px, "pre_w", &px->pre_w);
	AG_BindInt(px, "pre_h", &px->pre_h);
	AG_BindInt(px, "sScaled", &px->sScaled);
#endif
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Pixmap *px = obj;

	if ((px->flags & AG_PIXMAP_FORCE_SIZE) == 0) {
		r->w = WSURFACE(px,px->n)->w;
		r->h = WSURFACE(px,px->n)->h;
	} else {
		r->w = px->pre_w;
		r->h = px->pre_h;
	}
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Pixmap *px = obj;
	
	if (a->w < 1 || a->h < 1) {
		return (-1);
	}
	px->rClip.w = a->w;
	px->rClip.h = a->h;
	px->flags |= AG_PIXMAP_UPDATE;
	return (0);
}

static void
UpdateScaled(AG_Pixmap *px)
{
	AG_Surface *scaled = NULL;

	if (px->n < 0 || WIDTH(px) == 0 || HEIGHT(px) == 0) {
		goto fail;
	}
	if (AG_ScaleSurface(WSURFACE(px,px->n), WIDTH(px), HEIGHT(px), &scaled)
	    == -1) {
		goto fail;
	}
	if (px->sScaled == -1) {
		px->sScaled = AG_WidgetMapSurface(px, scaled);
	} else {
		AG_WidgetReplaceSurface(px, px->sScaled, scaled);
	}
	return;
fail:
	if (px->sScaled != -1) {
		AG_WidgetUnmapSurface(px, px->sScaled);
		px->sScaled = -1;
	}
}

static void
Draw(void *obj)
{
	AG_Pixmap *px = obj;

	if (px->n < 0)
		return;

	if (px->flags & AG_PIXMAP_RESCALE) {
		if (px->flags & AG_PIXMAP_UPDATE) {
			UpdateScaled(px);
			px->flags &= ~(AG_PIXMAP_UPDATE);
		}
		AG_WidgetBlitSurface(px, px->sScaled, px->s, px->t);
	} else {
		AG_PushClipRect(px, px->rClip);
		AG_WidgetBlitSurface(px, px->n, px->s, px->t);
		AG_PopClipRect(px);
	}
}

AG_WidgetClass agPixmapClass = {
	{
		"Agar(Widget:Pixmap)",
		sizeof(AG_Pixmap),
		{ 0,0 },
		Init,
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
