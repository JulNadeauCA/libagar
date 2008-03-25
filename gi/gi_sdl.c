/*
 * Copyright (c) 2008 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Graphics driver for the SDL (http://libsdl.org/).
 */
#include <config/have_sdl.h>
#ifdef HAVE_SDL

#include <core/core.h>
#include <core/config.h>

#include "gi.h"
#include "gi_sdl.h"

static GI_SDL *sdlDisplay = NULL;		/* Limited to one display */
static AG_Mutex sdlLock = AG_MUTEX_INITIALIZER;	/* Lock on sdlDisplay */

static void
Init(void *p, const char *name)
{
	GI *gi = p;
	GI_SDL *giSDL = p;

	giSDL->s = NULL;
	giSDL->vinfo = SDL_GetVideoInfo();
	gi->type = GI_DISPLAY_SOFTWARE_FB;
}

/* Convert GI color to pixel in current display format. */
static __inline__ Uint32
ColorToPixel32(GI_SDL *giSDL, const GI_Color *C)
{
	return SDL_MapRGB(giSDL->s->format, C->r, C->g, C->b);
}

/* Convert SDL palette to GI palette. */
static GI_Palette *
ConvertPalette(SDL_Palette *sp)
{
	GI_Palette *gp;
	Uint i;

	if (sp == NULL) {
		return (NULL);
	}
	gp = Malloc(sizeof(GI_Palette));
	gp->ncolors = sp->ncolors;
	gp->colors = Malloc(sizeof(GI_Color)*gp->ncolors);
	for (i = 0; i < gp->ncolors; i++) {
		SDL_Color *C = &sp->colors[i];
		gp->colors[i] = GI_ColorRGB(C->r, C->g, C->b);
	}
	return (gp);
}

static int
GetDisplay(void *p, int w, int h, int depth, Uint flags)
{
	GI *gi = p;
	GI_SDL *giSDL = p;
	Uint32 sdlFlags = SDL_SWSURFACE;
	SDL_Surface *sNew;
	
	if (flags & GI_FULLSCREEN) { sdlFlags |= SDL_FULLSCREEN; }
	if (flags & GI_RESIZABLE) { sdlFlags |= SDL_RESIZABLE; }
	if (flags & GI_NOFRAME) { sdlFlags |= SDL_NOFRAME; }

	AG_MutexLock(&sdlLock);
	if (sdlDisplay != NULL) {
		AG_SetError("Multiple displays unsupported in SDL 1.2");
		goto fail;
	}
	if ((SDL_WasInit(SDL_INIT_VIDEO) & SDL_INIT_VIDEO) == 0 &&
	     SDL_InitSubSystem(SDL_INIT_VIDEO) == -1) {
		AG_SetError("SDL_INIT_VIDEO: %s", SDL_GetError());
		goto fail;
	}

	giSDL->depth = SDL_VideoModeOK(w, h, depth, sdlFlags);
	if (giSDL->depth == 4 || giSDL->depth == 8) {
		sdlFlags |= SDL_HWPALETTE;
	}
	if ((giSDL->s = SDL_SetVideoMode(w, h, giSDL->depth, sdlFlags))
	    == NULL) {
		AG_SetError("SDL_SetVideoMode(%d,%d,%d): %s", w, h,
		    giSDL->depth, SDL_GetError());
		goto fail;
	}
	giSDL->s = sNew;
	gi->w = sNew->w;
	gi->h = sNew->h;
	gi->pitch = sNew->pitch;
	gi->fb = GI_SurfaceNew(sNew->w, sNew->h,
	    GI_PixelFormatFromSDL(sNew->format));
	gi->Bpp = sNew->format->BytesPerPixel;
	gi->bpp = sNew->format->BitsPerPixel;
	gi->caps = 0;

	if (giSDL->vinfo->wm_available)
		gi->flags |= (GI_DISPLAY_WINDOWED|GI_DISPLAY_RESIZABLE);
	if (giSDL->vinfo->hw_available)
		gi->caps |= GI_CAP_HW_SURFACES;
	if (giSDL->vinfo->blit_hw || giSDl->vinfo->blit_sw)
		gi->caps |= GI_CAP_HW_BLITS;
	if (giSDl->vinfo->blit_fill)
		gi->caps |= GI_CAP_HW_RECT_FILL;

	sdlDisplay = giSDL;
	AG_MutexUnlock(&sdlLock);
	return (0);
fail:
	AG_MutexUnlock(&sdlLock);
	return (-1);
}

static int
ResizeDisplay(void *obj, int w, int h)
{
	GI *gi = obj;
	GI_SDL *giSDL = obj;
	SDL_Surface *sNew;

	AG_MutexLock(&sdlLock);
	if ((sNew = SDL_SetVideoMode(w, h, giSDL->depth, sdlFlags)) == NULL) {
		AG_SetError("SDL_SetVideoMode(%d,%d,%d): %s", w, h,
		    giSDL->depth, SDL_GetError());
		goto fail;
	}
	giSDL->s = sNew;
	gi->fb = sNew->pixels;
	gi->pitch = sNew->pitch;
	gi->w = sNew->w;
	gi->h = sNew->h;
	AG_MutexUnlock(&sdlLock);
	return (0);
fail:
	AG_MutexUnlock(&sdlLock);
	return (-1);
}

static int
ClearDisplay(void *obj, int w, int h, GI_Color C)
{
	GI_SDL *giSDL = obj;
	SDL_FillRect(giSDL->s, NULL, ColorToPixel32(giSDL, &C));
}

static int
LockDisplay(void *obj)
{
	GI_SDL *giSDL = obj;
	SDL_LockSurface(giSDL->s);
}

static int
UnlockDisplay(void *obj)
{
	GI_SDL *giSDL = obj;
	SDL_UnlockSurface(giSDL->s);
}

static GI_Color
GetPixel(void *obj, int x, int y)
{
	GI *gi = obj;
	GI_SDL *giSDL = obj;
	Uint8 *p = gi->fb + y*gi->pitch + x*gi->Bpp;
	GI_Color c;
	Uint32 pixel;

	switch (gi->Bpp) {
	case 4:
		pixel = *(Uint32 *)p;
	case 3:
#if AG_BYTEORDER == AG_BIG_ENDIAN
		pixel = ((p[0] << 16) + (p[1] << 8) + p[2]);
#else
		pixel = (p[0] + (p[1] << 8) + (p[2] << 16));
#endif
	case 2:
		pixel = *(Uint16 *)p;
	default:
		pixel = *p;
	}
	SDL_GetRGB(pixel, giSDL->s->format, &c.r, &c.g, &c.b);
	c.a = 255;
	return (c);
}

static void
PutPixel(void *obj, int x, int y, GI_Color C)
{
	GI *gi = obj;
	GI_SDL *giSDL = obj;
	Uint8 *p = gi->fb + y*gi->pitch + x*gi->Bpp;
	Uint32 pixel = ColorToPixel32(giSDL, &C);

	switch (gi->Bpp) {
	case 4:
		*(Uint32 *)p = pixel;
		break;
	case 3:
#if AG_BYTEORDER == AG_BIG_ENDIAN
		p[0] = (pixel>>16) & 0xff;
		p[1] = (pixel>>8) & 0xff;
		p[2] = pixel & 0xff;
#else
		p[2] = (pixel>>16) & 0xff;
		p[1] = (pixel>>8) & 0xff;
		p[0] = pixel & 0xff;
#endif
		break;
	case 2:
		*(Uint16 *)p = pixel;
		break;
	default:
		*p = pixel;
		break;
	}
}

GI_Class giSDLClass = {
	{
		"GI:SDL",
		sizeof(GI_SDL),
		{ 0,0 },
		Init,
		NULL,		/* reinit */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		Edit
	},
	GetDisplay,
	ResizeDisplay,
	ClearDisplay,
	LockDisplay,
	UnlockDisplay,
	NULL,		/* SetCaption */
	NULL,		/* GetCaption */
	NULL,		/* SetIcon */
	NULL,		/* GetIcon */
	GetPixel,
	PutPixel,
};

#endif /* HAVE_SDL */
