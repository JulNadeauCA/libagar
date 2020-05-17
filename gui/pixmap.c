/*
 * Copyright (c) 2005-2020 Julien Nadeau Carriere <vedge@csoft.net>
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

/*
 * Image display widget. It simply displays a surface or an animation.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/pixmap.h>
#include <agar/gui/primitive.h>
#include <agar/gui/window.h>
#include <agar/gui/opengl.h>

/* Create new, empty pixmap of the given size. */
AG_Pixmap *
AG_PixmapNew(void *parent, Uint flags, Uint w, Uint h)
{
	AG_Pixmap *px;

	px = Malloc(sizeof(AG_Pixmap));
	AG_ObjectInit(px, &agPixmapClass);

	if (flags & AG_PIXMAP_HFILL) { WIDGET(px)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_PIXMAP_VFILL) { WIDGET(px)->flags |= AG_WIDGET_VFILL; }
	px->flags |= (flags | AG_PIXMAP_FORCE_SIZE);
	px->wPre = w;
	px->hPre = h;

	AG_WidgetMapSurface(px, AG_SurfaceEmpty());
	AG_ObjectAttach(parent, px);
	return (px);
}

/* Create new pixmap from the copy of the contents of a given surface. */
AG_Pixmap *
AG_PixmapFromSurface(void *parent, Uint flags, const AG_Surface *su)
{
	AG_Pixmap *px;

	px = Malloc(sizeof(AG_Pixmap));
	AG_ObjectInit(px, &agPixmapClass);

	if (flags & AG_PIXMAP_HFILL) { WIDGET(px)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_PIXMAP_VFILL) { WIDGET(px)->flags |= AG_WIDGET_VFILL; }
	px->flags |= flags;

	AG_ObjectAttach(parent, px);

	AG_WidgetMapSurface(px, (su) ? AG_SurfaceConvert(su, agSurfaceFmt) :
	                               AG_SurfaceEmpty());
	return (px);
}

/* Create new pixmap by mapping the given surface (potentially unsafe). */
AG_Pixmap *
AG_PixmapFromSurfaceNODUP(void *parent, Uint flags, AG_Surface *su)
{
	AG_Pixmap *px;

	px = Malloc(sizeof(AG_Pixmap));
	AG_ObjectInit(px, &agPixmapClass);

	if (flags & AG_PIXMAP_HFILL) { WIDGET(px)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_PIXMAP_VFILL) { WIDGET(px)->flags |= AG_WIDGET_VFILL; }
	px->flags |= flags;

	AG_ObjectAttach(parent, px);

	AG_WidgetMapSurfaceNODUP(px, su);
	return (px);
}

/* Create a new pixmap from the contents of a surface scaled to w x h. */
AG_Pixmap *
AG_PixmapFromSurfaceScaled(void *parent, Uint flags, const AG_Surface *su,
    Uint w, Uint h)
{
	AG_Pixmap *px;
	AG_Surface *suScaled = NULL;

	px = Malloc(sizeof(AG_Pixmap));
	AG_ObjectInit(px, &agPixmapClass);

	if (flags & AG_PIXMAP_HFILL) { WIDGET(px)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_PIXMAP_VFILL) { WIDGET(px)->flags |= AG_WIDGET_VFILL; }
	px->flags |= flags;

	AG_ObjectAttach(parent, px);

	if (su != NULL) {
		if ((suScaled = AG_SurfaceScale(su, w,h, 0)) == NULL) {
			AG_FatalError(NULL);
		}
		AG_WidgetMapSurface(px, suScaled);
	} else {
		AG_WidgetMapSurface(px, AG_SurfaceEmpty());
	}
	return (px);
}

/* Create a new pixmap from the given image file. */
AG_Pixmap *
AG_PixmapFromFile(void *parent, Uint flags, const char *file)
{
	AG_Pixmap *px;
	AG_Surface *S;

	if ((S = AG_SurfaceFromFile(file)) == NULL)
		AG_FatalError(NULL);

	px = Malloc(sizeof(AG_Pixmap));
	AG_ObjectInit(px, &agPixmapClass);

	if (flags & AG_PIXMAP_HFILL) { WIDGET(px)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_PIXMAP_VFILL) { WIDGET(px)->flags |= AG_WIDGET_VFILL; }
	px->flags |= flags;

	AG_ObjectAttach(parent, px);
	AG_WidgetMapSurface(px, S);
/*	AG_WidgetMapSurface(px, AG_SurfaceConvert(S, agSurfaceFmt)); */
	return (px);
}

#ifdef HAVE_OPENGL

/* Create a new pixmap from the given OpenGL texture by name and lod. */
AG_Pixmap *
AG_PixmapFromTexture(void *parent, Uint flags, Uint name, int lod)
{
	AG_Pixmap *px;
	AG_Surface *S;
	GLint w, h;

	glBindTexture(GL_TEXTURE_2D, (GLuint)name);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, lod, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, lod, GL_TEXTURE_HEIGHT, &h);

	S = AG_SurfaceRGBA(w, h, 32, 0,
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
	if (S == NULL) {
		AG_FatalError(NULL);
	}
	glGetTexImage(GL_TEXTURE_2D, lod, GL_RGBA, GL_UNSIGNED_BYTE, S->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	px = Malloc(sizeof(AG_Pixmap));
	AG_ObjectInit(px, &agPixmapClass);

	if (flags & AG_PIXMAP_HFILL) { WIDGET(px)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_PIXMAP_VFILL) { WIDGET(px)->flags |= AG_WIDGET_VFILL; }
	px->flags |= flags;
	
	AG_ObjectAttach(parent, px);
	AG_WidgetMapSurface(px, S);
	return (px);
}

#else /* HAVE_OPENGL */

AG_Pixmap *
AG_PixmapFromTexture(void *parent, Uint flags, Uint name, int lod)
{
	AG_SetErrorS("Agar was not compiled with OpenGL support");
	return (NULL);
}

#endif /* HAVE_OPENGL */

/*
 * Map a copy of the specified surface.
 * Returned surface ID is valid as long as pixmap is locked.
 */
int
AG_PixmapAddSurface(AG_Pixmap *px, const AG_Surface *Sorig)
{
	AG_Surface *S;
	int name;

	if ((S = AG_SurfaceConvert(Sorig, agSurfaceFmt)) == NULL)
		return (-1);

	AG_OBJECT_ISA(px, "AG_Widget:AG_Pixmap:*");
	AG_ObjectLock(px);

	name = AG_WidgetMapSurface(px, S);
	px->flags |= AG_PIXMAP_UPDATE;

	AG_ObjectUnlock(px);
	return (name);
}

/*
 * Map a resized copy of the specified surface.
 * Returned surface ID is valid as long as pixmap is locked.
 */
int
AG_PixmapAddSurfaceScaled(AG_Pixmap *px, const AG_Surface *Sorig,
    Uint w, Uint h)
{
	AG_Surface *S = NULL;
	int name;
	
	if ((S = AG_SurfaceScale(Sorig, w,h, 0)) == NULL)
		return (-1);

	AG_OBJECT_ISA(px, "AG_Widget:AG_Pixmap:*");
	AG_ObjectLock(px);

	name = AG_WidgetMapSurface(px, S);
	px->flags |= AG_PIXMAP_UPDATE;

	AG_ObjectUnlock(px);
	return (name);
}

/*
 * Map a surface created from an image file (format autodetected).
 * Returned surface ID is valid as long as pixmap is locked.
 */
int
AG_PixmapAddSurfaceFromFile(AG_Pixmap *px, const char *path)
{
	AG_Surface *suFile, *su;
	int name;

	if ((suFile = AG_SurfaceFromFile(path)) == NULL) {
		return (-1);
	}
	if ((su = AG_SurfaceConvert(suFile, agSurfaceFmt)) == NULL) {
		AG_SurfaceFree(suFile);
		return (-1);
	}
	
	AG_OBJECT_ISA(px, "AG_Widget:AG_Pixmap:*");
	AG_ObjectLock(px);

	name = AG_WidgetMapSurface(px, su);
	px->flags |= AG_PIXMAP_UPDATE;

	AG_ObjectUnlock(px);

	AG_SurfaceFree(suFile);
	return (name);
}

/* Replace the contents of a mapped surface. */
void
AG_PixmapReplaceSurface(AG_Pixmap *px, int name, AG_Surface *s)
{
	AG_WidgetReplaceSurface(px, name, s);
	AG_Redraw(px);
}

/* Invalidate any cached/hardware copy of a mapped surface. */
void
AG_PixmapUpdateSurface(AG_Pixmap *px, int name)
{
	AG_WidgetUpdateSurface(px, name);
	AG_Redraw(px);
}

/* Select the mapped surface to display. */
int
AG_PixmapSetSurface(AG_Pixmap *px, int name)
{
	AG_OBJECT_ISA(px, "AG_Widget:AG_Pixmap:*");
	AG_ObjectLock(px);

	if (name < 0 || name >= (int)AGWIDGET(px)->nSurfaces) {
		AG_ObjectUnlock(px);
		return (-1);
	}
	px->n = name;
	px->flags |= AG_PIXMAP_UPDATE;

	AG_Redraw(px);
	AG_ObjectUnlock(px);
	return (0);
}

/* Request an explicit size requisition in pixels. */
void
AG_PixmapSizeHint(AG_Pixmap *px, int w, int h)
{
	AG_OBJECT_ISA(px, "AG_Widget:AG_Pixmap:*");
	px->wPre = w;
	px->hPre = h;
}

/* Set texture coordinates. */
void
AG_PixmapSetCoords(AG_Pixmap *px, int s, int t)
{
	AG_OBJECT_ISA(px, "AG_Widget:AG_Pixmap:*");
	AG_ObjectLock(px);

	px->s = s;
	px->t = t;

	AG_Redraw(px);
	AG_ObjectUnlock(px);
}

/* Return a copy of the surface at given index. */
AG_Surface *
AG_PixmapGetSurface(const AG_Pixmap *px, int name)
{
	return AG_SurfaceDup(WSURFACE(px,name));
}

static void
Init(void *_Nonnull obj)
{
	AG_Pixmap *px = obj;
	
	px->flags = AG_PIXMAP_UPDATE;
	px->n = 0;
	px->s = 0;
	px->t = 0;
	px->wPre = 64;
	px->hPre = 64;
	px->sScaled = -1;
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Pixmap *px = obj;

	r->w = WIDGET(px)->paddingLeft + WIDGET(px)->paddingRight;
	r->h = WIDGET(px)->paddingTop + WIDGET(px)->paddingBottom;

	if ((px->flags & AG_PIXMAP_FORCE_SIZE) == 0 && px->n >= 0) {
		r->w += WSURFACE(px,px->n)->w;
		r->h += WSURFACE(px,px->n)->h;
	} else {
		r->w += px->wPre;
		r->h += px->hPre;
	}
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_Pixmap *px = obj;
	
	if (a->w < WIDGET(px)->paddingLeft + 1 + WIDGET(px)->paddingRight ||
	    a->h < WIDGET(px)->paddingTop + 1 + WIDGET(px)->paddingBottom) {
		return (-1);
	}
	px->flags |= AG_PIXMAP_UPDATE;
	return (0);
}

static void
UpdateScaled(AG_Pixmap *_Nonnull px)
{
	AG_Surface *Sorig, *S;

	if (px->n < 0 || WIDTH(px) == 0 || HEIGHT(px) == 0)
		goto fail;

	Sorig = WSURFACE(px, px->n);
	if ((S = AG_SurfaceScale(Sorig, WIDTH(px),HEIGHT(px), 0)) == NULL) {
		goto fail;
	}
	if (px->sScaled == -1) {
		px->sScaled = AG_WidgetMapSurface(px, S);
	} else {
		AG_WidgetReplaceSurface(px, px->sScaled, S);
	}
	return;
fail:
	if (px->sScaled != -1) {
		AG_WidgetUnmapSurface(px, px->sScaled);
		px->sScaled = -1;
	}
}

static void
Draw(void *_Nonnull obj)
{
	AG_Pixmap *px = obj;

	if (px->n < 0)
		return;

	if (px->flags & AG_PIXMAP_RESCALE) {
		if (px->flags & AG_PIXMAP_UPDATE) {
			UpdateScaled(px);
			px->flags &= ~(AG_PIXMAP_UPDATE);
		}
		AG_PushBlendingMode(px, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);

		AG_WidgetBlitSurface(px, px->sScaled,
		    WIDGET(px)->paddingLeft + px->s,
		    WIDGET(px)->paddingTop  + px->t);

		AG_PopBlendingMode(px);
	} else {
		AG_PushClipRect(px, &WIDGET(px)->r);
		AG_PushBlendingMode(px, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);

		AG_WidgetBlitSurface(px, px->n,
		    WIDGET(px)->paddingLeft + px->s,
		    WIDGET(px)->paddingTop  + px->t);

		AG_PopBlendingMode(px);
		AG_PopClipRect(px);
	}
}

AG_WidgetClass agPixmapClass = {
	{
		"Agar(Widget:Pixmap)",
		sizeof(AG_Pixmap),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

#endif /* AG_WIDGETS */
