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
 * Graphics driver for OpenGL. Currently we also require SDL in order to
 * set up the display. Direct support for APIs such as GLX and AGL will
 * be implemented later (and this is high priority because we want multiple
 * OpenGL window support).
 */

#include <config/have_opengl.h>
#include <config/have_sdl.h>
#if defined(HAVE_OPENGL) && defined(HAVE_SDL)

#include <core/core.h>
#include <core/config.h>

#include "gi.h"
#include "gi_gl.h"

static GI_SDL *glDisplay = NULL;		/* One display for now */
static AG_Mutex glLock = AG_MUTEX_INITIALIZER;	/* Lock on glDisplay */

static void
Init(void *p, const char *name)
{
	GI *gi = p;
	GI_GL *gl = p;

	glGL->flags = 0;
	giGL->vinfo = SDL_GetVideoInfo();
	gi->type = GI_DISPLAY_GL;
}

static int
GetDisplay(void *p, int w, int h, int depth, Uint flags)
{
	GI *gi = p;
	GI_GL *giGL = p;
	Uint32 sdlFlags = SDL_SWSURFACE;
	SDL_Surface *sNew;

	if (flags & GI_FULLSCREEN) { sdlFlags |= SDL_FULLSCREEN; }
	if (flags & GI_RESIZABLE) { sdlFlags |= SDL_RESIZABLE; }
	if (flags & GI_NOFRAME) { sdlFlags |= SDL_NOFRAME; }

	AG_MutexLock(&glLock);
	if (sdlDisplay != NULL) {
		AG_SetError("Multiple displays unsupported in SDL 1.2");
		goto fail;
	}
	if ((SDL_WasInit(SDL_INIT_VIDEO) & SDL_INIT_VIDEO) == 0 &&
	     SDL_InitSubSystem(SDL_INIT_VIDEO) == -1) {
		AG_SetError("SDL_INIT_VIDEO: %s", SDL_GetError());
		goto fail;
	}
	gi->w = sNew->w;
	gi->h = sNew->h;
	gi->pitch = sNew->pitch;

	gi->caps = 0;

	if (giGL->vinfo->wm_available)
		gi->flags |= (GI_DISPLAY_WINDOWED|GI_DISPLAY_RESIZABLE);
	if (giGL->vinfo->hw_available)
		gi->caps |= GI_CAP_HW_SURFACES;
	if (giGL->vinfo->blit_hw || giGL->vinfo->blit_sw)
		gi->caps |= GI_CAP_HW_BLITS;
	if (giGL->vinfo->blit_fill)
		gi->caps |= GI_CAP_HW_RECT_FILL;

	sdlDisplay = giSDL;
	AG_MutexUnlock(&glLock);
	return (0);
fail:
	AG_MutexUnlock(&glLock);
	return (-1);
}

static int
ResizeDisplay(void *obj, int w, int h)
{
	GI *gi = obj;
	GI_SDL *giSDL = obj;
	SDL_Surface *sNew;

	AG_MutexLock(&glLock);
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
	AG_MutexUnlock(&glLock);
	return (0);
fail:
	AG_MutexUnlock(&glLock);
	return (-1);
}

static GI_Color
GetPixel(void *obj, int x, int y)
{
	GI *gi = obj;
	GI_GL *giGL = obj;
	Uint32 pixel;
	GI_Color C;

	glReadPixels(x, (gi->h - y), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &pixel);
	C.r = (pixel & 0x000000ff);
	C.g = (pixel & 0x0000ff00) >> 8;
	C.b = (pixel & 0x00ff0000) >> 16;
	C.a = 255;
	return (C);
}

static void
PutPixel(void *obj, int x, int y, GI_Color C)
{
	GI *gi = obj;
	GI_GL *giGL = obj;

	glBegin(GL_POINTS);
	glColor3ub(r, g, b);
	glVertex2i(x, y);
	glEnd();
}

GI_Class giGLClass = {
	{
		"GI:GL",
		sizeof(GI_GL),
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
	GetPixel,
	PutPixel,
	NULL,		/* SetCaption */
	NULL,		/* GetCaption */
	NULL,		/* SetIcon */
	NULL,		/* GetIcon */
};

#endif /* HAVE_OPENGL && HAVE_SDL */
