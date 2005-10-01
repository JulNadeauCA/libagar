/*	$Csoft: view.c,v 1.194 2005/09/28 05:10:20 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#include <config/have_jpeg.h>
#include <compat/dir.h>

#include <engine/engine.h>
#include <engine/config.h>
#include <engine/view.h>
#include <engine/prop.h>

#ifdef MAP
#include <engine/map/map.h>
#endif

#include <engine/widget/widget.h>
#include <engine/widget/window.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef HAVE_JPEG
#include <jpeglib.h>
#endif

/* Read-only as long as the engine is running. */
AG_Display *agView = NULL;
SDL_PixelFormat *agVideoFmt = NULL;
SDL_PixelFormat *agSurfaceFmt = NULL;
const SDL_VideoInfo *agVideoInfo;
int agScreenshotQuality = 75;

const char *agBlendFuncNames[] = {
	N_("Overlay"),
	N_("Source"),
	N_("Destination"),
	N_("Mean"),
	N_("Source minus destination"),
	N_("Destination minus source"),
	N_("Euclidean distance"),
	NULL
};

int
AG_ViewInit(int w, int h, int bpp, u_int flags)
{
	Uint32 screenflags = 0;
	int depth;

	if (flags & AG_VIDEO_HWSURFACE) {
		screenflags |= SDL_HWSURFACE;
	} else {
		screenflags |= SDL_SWSURFACE;
	}
	if (flags & AG_VIDEO_RESIZABLE) { screenflags |= SDL_RESIZABLE; }
	if (flags & AG_VIDEO_ASYNCBLIT) { screenflags |= SDL_ASYNCBLIT; }
	if (flags & AG_VIDEO_ANYFORMAT) { screenflags |= SDL_ANYFORMAT; }
	if (flags & AG_VIDEO_HWPALETTE) { screenflags |= SDL_HWPALETTE; }
	if (flags & AG_VIDEO_DOUBLEBUF) { screenflags |= SDL_DOUBLEBUF; }
	if (flags & AG_VIDEO_FULLSCREEN) { screenflags |= SDL_FULLSCREEN; }
	if (flags & AG_VIDEO_RESIZABLE) { screenflags |= SDL_RESIZABLE; }
	if (flags & AG_VIDEO_NOFRAME) { screenflags |= SDL_NOFRAME; }

	agView = Malloc(sizeof(AG_Display), M_VIEW);
	agView->winop = AG_WINOP_NONE;
	agView->ndirty = 0;
	agView->maxdirty = 4;
	agView->dirty = Malloc(agView->maxdirty * sizeof(SDL_Rect), M_VIEW);
	agView->opengl = 0;
	agView->modal_win = NULL;
	agView->wop_win = NULL;
	agView->focus_win = NULL;
	agView->rNom = 1000/AG_Uint(agConfig, "view.nominal-fps");
	agView->rCur = 0;
	dprintf("%u fps\n", agView->rNom);
	TAILQ_INIT(&agView->windows);
	TAILQ_INIT(&agView->detach);
	pthread_mutex_init(&agView->lock, &agRecursiveMutexAttr);

	depth = bpp > 0 ? bpp : AG_Uint8(agConfig, "view.depth");
	agView->w = w > 0 ? w : AG_Uint16(agConfig, "view.w");
	agView->h = h > 0 ? h : AG_Uint16(agConfig, "view.h");

	if (AG_Bool(agConfig, "view.full-screen"))
		screenflags |= SDL_FULLSCREEN;
	if (AG_Bool(agConfig, "view.async-blits"))
		screenflags |= SDL_HWSURFACE|SDL_ASYNCBLIT;

	agView->depth = SDL_VideoModeOK(agView->w, agView->h, depth,
	    screenflags);
	if (agView->depth == 8)
		screenflags |= SDL_HWPALETTE;
	
	dprintf("Mode: %ux%ux%u\n", agView->w, agView->h, agView->depth);

#ifdef HAVE_OPENGL
	if (AG_Bool(agConfig, "view.opengl")) {
		screenflags |= SDL_OPENGL;
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		
		agView->opengl = 1;
	}
#endif

	if (agView->w < AG_Uint16(agConfig, "view.min-w") ||
	    agView->h < AG_Uint16(agConfig, "view.min-h")) {
		AG_SetError(_("The resolution is too small."));
		goto fail;
	}

	/* Set the video mode. */
	agView->v = SDL_SetVideoMode(agView->w, agView->h, 0, screenflags);
	if (agView->v == NULL) {
		AG_SetError("Setting %dx%dx%d mode: %s",
		    agView->w, agView->h, agView->depth, SDL_GetError());
		goto fail;
	}
	agView->stmpl = SDL_CreateRGBSurface(SDL_SWSURFACE, 1, 1, 32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
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
	if (agView->stmpl == NULL) {
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
	}
	agVideoFmt = agView->v->format;
	agSurfaceFmt = agView->stmpl->format;
	printf(_("Video display is %dbpp (%08x,%08x,%08x)\n"),
	     agVideoFmt->BitsPerPixel, agVideoFmt->Rmask, agVideoFmt->Gmask,
	     agVideoFmt->Bmask);
	printf(_("Reference surface is %dbpp (%08x,%08x,%08x,%08x)\n"),
	     agSurfaceFmt->BitsPerPixel,
	     agSurfaceFmt->Rmask,
	     agSurfaceFmt->Gmask,
	     agSurfaceFmt->Bmask,
	     agSurfaceFmt->Amask);

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		int red, blue, green, alpha, depth, bsize;
		Uint8 bR, bG, bB;

		SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &red);
		SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &green);
		SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &blue);
		SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &alpha);
		SDL_GL_GetAttribute(SDL_GL_BUFFER_SIZE, &bsize);
		SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depth);

		AG_SetInt(agConfig, "view.gl.depth", depth);
		AG_SetInt(agConfig, "view.gl.red_size", red);
		AG_SetInt(agConfig, "view.gl.green_size", green);
		AG_SetInt(agConfig, "view.gl.blue_size", blue);
		AG_SetInt(agConfig, "view.gl.alpha_size", alpha);
		AG_SetInt(agConfig, "view.gl.buffer_size", bsize);

		glViewport(0, 0, agView->w, agView->h);
		glOrtho(0, agView->w, agView->h, 0, -1.0, 1.0);

		SDL_GetRGB(AG_COLOR(BG_COLOR), agVideoFmt, &bR, &bG, &bB);
		glClearColor(bR/255.0, bG/255.0, bB/255.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glEnable(GL_TEXTURE_2D);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DITHER);
		glDisable(GL_BLEND);
	} else
#endif /* HAVE_OPENGL */
	{
		SDL_FillRect(agView->v, NULL, AG_COLOR(BG_COLOR));
		SDL_UpdateRect(agView->v, 0, 0, agView->v->w, agView->v->h);
	}

	AG_SetUint8(agConfig, "view.depth", agView->depth);
	AG_SetUint16(agConfig, "view.w", agView->w);
	AG_SetUint16(agConfig, "view.h", agView->h);
	return (0);
fail:
	pthread_mutex_destroy(&agView->lock);
	Free(agView, M_VIEW);
	agView = NULL;
	return (-1);
}

int
AG_ResizeDisplay(int w, int h)
{
	Uint32 flags = agView->v->flags & (SDL_SWSURFACE|SDL_FULLSCREEN|
	                                 SDL_HWSURFACE|SDL_ASYNCBLIT|
					 SDL_HWPALETTE|SDL_RESIZABLE|
					 SDL_OPENGL);
	AG_Window *win;
	SDL_Surface *su;
	int ow, oh;

	/*
	 * Set initial coordinates of windows that might have not been
	 * scaled yet.
	 */
	TAILQ_FOREACH(win, &agView->windows, windows) {
		pthread_mutex_lock(&win->lock);
		if (!win->visible) {
			AG_WINDOW_UPDATE(win);
		}
		pthread_mutex_unlock(&win->lock);
	}

	/* XXX set a minimum! */
	if ((su = SDL_SetVideoMode(w, h, 0, flags)) == NULL) {
		AG_SetError("resize to %ux%u: %s", w, h, SDL_GetError());
		return (-1);
	}
	ow = agView->w;
	oh = agView->h;

	agView->v = su;
	agView->w = w;
	agView->h = h;
	AG_SetUint16(agConfig, "view.w", w);
	AG_SetUint16(agConfig, "view.h", h);

	TAILQ_FOREACH(win, &agView->windows, windows) {
		pthread_mutex_lock(&win->lock);
		AGWIDGET(win)->x = AGWIDGET(win)->x*w/ow;
		AGWIDGET(win)->y = AGWIDGET(win)->y*h/oh;
		AGWIDGET(win)->w = AGWIDGET(win)->w*w/ow;
		AGWIDGET(win)->h = AGWIDGET(win)->h*h/oh;
		AG_WINDOW_UPDATE(win);
		pthread_mutex_unlock(&win->lock);
	}
	return (0);
}

void
AG_ViewVideoExpose(void)
{
	AG_Window *win;

	TAILQ_FOREACH(win, &agView->windows, windows) {
		pthread_mutex_lock(&win->lock);
		if (win->visible) {
			AG_WidgetDraw(win);
		}
		pthread_mutex_unlock(&win->lock);
	}

#ifdef HAVE_OPENGL
	if (agView->opengl)
		SDL_GL_SwapBuffers();
#endif
}

void
AG_ViewDestroy(void)
{
	AG_Window *win, *nwin;

	for (win = TAILQ_FIRST(&agView->windows);
	     win != TAILQ_END(&agView->windows);
	     win = nwin) {
		nwin = TAILQ_NEXT(win, windows);
		AG_ObjectDestroy(win);
		Free(win, M_OBJECT);
	}

	Free(agView->dirty, M_VIEW);
	pthread_mutex_destroy(&agView->lock);
	Free(agView, M_VIEW);
	agView = NULL;
}

/* Return the named window or NULL if there is no such window. */
AG_Window *
AG_FindWindow(const char *name)
{
	AG_Window *win;

	pthread_mutex_lock(&agView->lock);
	TAILQ_FOREACH(win, &agView->windows, windows) {
		if (strcmp(AGOBJECT(win)->name, name) == 0)
			break;
	}
	pthread_mutex_unlock(&agView->lock);
	return (win);
}

/* Attach a window to a view. */
void
AG_ViewAttach(void *child)
{
	AG_Window *win = child;
	
	pthread_mutex_lock(&agView->lock);
	agView->focus_win = NULL;
	TAILQ_INSERT_TAIL(&agView->windows, win, windows);
	pthread_mutex_unlock(&agView->lock);
}

static void
detach_window(AG_Window *win)
{
	AG_Window *subwin, *nsubwin;
	AG_Window *owin;

	for (subwin = TAILQ_FIRST(&win->subwins);
	     subwin != TAILQ_END(&win->subwins);
	     subwin = nsubwin) {
		nsubwin = TAILQ_NEXT(subwin, swins);
		detach_window(subwin);
	}
	TAILQ_INIT(&win->subwins);
	
	AG_WindowHide(win);
	AG_PostEvent(agView, win, "detached", NULL);
	TAILQ_REMOVE(&agView->windows, win, windows);
	TAILQ_INSERT_TAIL(&agView->detach, win, detach);

	TAILQ_FOREACH(owin, &agView->windows, windows) {
		TAILQ_FOREACH(subwin, &owin->subwins, swins) {
			if (subwin == win)
				break;
		}
		if (subwin != NULL)
			TAILQ_REMOVE(&owin->subwins, subwin, swins);
	}
}

/* Place a window and its children on the detachment queue. */
void
AG_ViewDetach(AG_Window *win)
{
	pthread_mutex_lock(&agView->lock);
	detach_window(win);
	pthread_mutex_unlock(&agView->lock);
}

/* Return the 32-bit form of the pixel at the given location. */
Uint32
AG_GetPixel(SDL_Surface *s, Uint8 *pSrc)
{
	switch (s->format->BytesPerPixel) {
	case 4:
		return (*(Uint32 *)pSrc);
	case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		return ((pSrc[0] << 16) +
		        (pSrc[1] << 8) +
		         pSrc[2]);
#else
		return  (pSrc[0] +
		        (pSrc[1] << 8) +
		        (pSrc[2] << 16));
#endif
	case 2:
		return (*(Uint16 *)pSrc);
	}
	return (*pSrc);
}

void
AG_PutPixel(SDL_Surface *s, Uint8 *pDst, Uint32 cDst)
{
	switch (s->format->BytesPerPixel) {
	case 4:
		*(Uint32 *)pDst = cDst;
		break;
	case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		pDst[0] = (cDst>>16) & 0xff;
		pDst[1] = (cDst>>8) & 0xff;
		pDst[2] = cDst & 0xff;
#else
		pDst[2] = (cDst>>16) & 0xff;
		pDst[1] = (cDst>>8) & 0xff;
		pDst[0] = cDst & 0xff;
#endif
		break;
	case 2:
		*(Uint16 *)pDst = cDst;
		break;
	default:
		*pDst = cDst;
		break;
	}
}

/* Release the windows on the detachment queue. */
void
AG_ViewDetachQueued(void)
{
	AG_Window *win, *nwin;

	for (win = TAILQ_FIRST(&agView->detach);
	     win != TAILQ_END(&agView->detach);
	     win = nwin) {
		nwin = TAILQ_NEXT(win, detach);
		AG_ObjectDestroy(win);
		Free(win, M_OBJECT);
	}
	TAILQ_INIT(&agView->detach);
}

/* Return a newly allocated surface containing a copy of ss. */
SDL_Surface *
AG_DupSurface(SDL_Surface *ss)
{
	SDL_Surface *rs;

	rs = SDL_ConvertSurface(ss, ss->format, SDL_SWSURFACE |
	    (ss->flags & (SDL_SRCCOLORKEY|SDL_SRCALPHA|SDL_RLEACCEL)));
	if (rs == NULL) {
		AG_SetError("SDL_ConvertSurface: %s", SDL_GetError());
		return (NULL);
	}
	rs->format->alpha = ss->format->alpha;
	rs->format->colorkey = ss->format->colorkey;
	return (rs);
}

int
AG_SamePixelFmt(SDL_Surface *s1, SDL_Surface *s2)
{
	return (s1->format->BytesPerPixel == s2->format->BytesPerPixel &&
	        s1->format->Rmask == s2->format->Rmask &&
		s1->format->Gmask == s2->format->Gmask &&
		s1->format->Bmask == s2->format->Bmask &&
		s1->format->Amask == s2->format->Amask &&
		s1->format->colorkey == s2->format->colorkey);
}

/*
 * Allocate a new surface containing a pixmap of ss scaled to wxh.
 * The source surface must not be locked by the calling thread.
 */
void
AG_ScaleSurface(SDL_Surface *ss, Uint16 w, Uint16 h, SDL_Surface **ds)
{
	Uint8 r1, g1, b1, a1;
	Uint8 *pDst;
	int x, y;
	int same_fmt;

	if (*ds == NULL) {
		*ds = SDL_CreateRGBSurface(
		    ss->flags & (SDL_SWSURFACE|SDL_SRCALPHA|SDL_SRCCOLORKEY),
		    w, h,
		    ss->format->BitsPerPixel,
		    ss->format->Rmask,
		    ss->format->Gmask,
		    ss->format->Bmask,
		    ss->format->Amask);
		if (*ds == NULL) {
			fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
		}
		(*ds)->format->alpha = ss->format->alpha;
		(*ds)->format->colorkey = ss->format->colorkey;
		same_fmt = 1;
	} else {
		//same_fmt = AG_SamePixelFmt(*ds, ss);
		same_fmt = 0;
	}

	if (ss->w == w && ss->h == h) {
		Uint32 saflags = ss->flags & (SDL_SRCALPHA|SDL_RLEACCEL);
		Uint8 salpha = ss->format->alpha;
		Uint32 sckflags = ss->flags & (SDL_SRCCOLORKEY|SDL_RLEACCEL);
		Uint32 sckey = ss->format->colorkey;

		SDL_SetAlpha(ss, 0, 0);
		SDL_SetColorKey(ss, 0, 0);
		SDL_BlitSurface(ss, NULL, *ds, NULL);
		SDL_SetAlpha(ss, saflags, salpha);
		SDL_SetColorKey(ss, sckflags, sckey);
		return;
	}

	if (SDL_MUSTLOCK(ss))
		SDL_LockSurface(ss);
	if (SDL_MUSTLOCK((*ds)))
		SDL_LockSurface(*ds);

	/* XXX only efficient when shrinking; inefficient when expanding */
	pDst = (Uint8 *)(*ds)->pixels;
	for (y = 0; y < (*ds)->h; y++) {
		for (x = 0; x < (*ds)->w; x++) {
			Uint8 *pSrc = (Uint8 *)ss->pixels +
			    (y*ss->h/(*ds)->h)*ss->pitch +
			    (x*ss->w/(*ds)->w)*ss->format->BytesPerPixel;
			Uint32 cSrc = AG_GET_PIXEL(ss, pSrc);
			Uint32 cDst;

			if (same_fmt) {
				cDst = cSrc;
			} else {
				SDL_GetRGBA(cSrc, ss->format,
				    &r1, &g1, &b1, &a1);
				cDst = SDL_MapRGBA((*ds)->format,
				    r1, g1, b1, a1);
			}
			AG_PUT_PIXEL((*ds), pDst, cDst);
			pDst += (*ds)->format->BytesPerPixel;
		}
	}
out:
	if (SDL_MUSTLOCK((*ds)))
		SDL_UnlockSurface(*ds);
	if (SDL_MUSTLOCK(ss))
		SDL_UnlockSurface(ss);
}

/* Set the alpha value of all pixels in a surface where a != 0. */
void
AG_SetAlphaPixels(SDL_Surface *su, Uint8 alpha)
{
	int x, y;

	if (SDL_MUSTLOCK(su))
		SDL_LockSurface(su);
	for (y = 0; y < su->h; y++) {
		for (x = 0; x < su->w; x++) {
			Uint8 *dst = (Uint8 *)su->pixels +
			    y*su->pitch +
			    x*su->format->BytesPerPixel;
			Uint8 r, g, b, a;

			SDL_GetRGBA(*(Uint32 *)dst, su->format, &r, &g, &b, &a);

			if (a != 0)
				a = alpha;

			AG_PUT_PIXEL(su, dst,
			    SDL_MapRGBA(su->format, r, g, b, a));
		}
	}
	if (SDL_MUSTLOCK(su))
		SDL_UnlockSurface(su);
}

int
AG_SetRefreshRate(int fps)
{
	if (fps < 1) {
		AG_SetError("FPS < 1");
		return (-1);
	}
	pthread_mutex_lock(&agView->lock);
	AG_SetUint(agConfig, "view.nominal-fps", fps);
	agView->rNom = 1000/fps;
	agView->rCur = 0;
	pthread_mutex_unlock(&agView->lock);
	dprintf("%u fps\n", fps);
	return (0);
}

#ifdef HAVE_OPENGL

void
AG_UpdateTexture(SDL_Surface *sourcesu, int texture)
{
	SDL_Surface *texsu;
	Uint32 saflags = sourcesu->flags & (SDL_SRCALPHA|SDL_RLEACCEL);
	Uint8 salpha = sourcesu->format->alpha;
	int w, h;

	w = powof2(sourcesu->w);
	h = powof2(sourcesu->h);

	/* Create a surface with the masks expected by OpenGL. */
	texsu = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
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
	if (texsu == NULL)
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
	
	/* Copy the source surface onto the GL texture surface. */
	SDL_SetAlpha(sourcesu, 0, 0);
	SDL_BlitSurface(sourcesu, NULL, texsu, NULL);
	SDL_SetAlpha(sourcesu, saflags, salpha);

	/* Create the OpenGL texture. */
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
	    GL_UNSIGNED_BYTE, texsu->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	SDL_FreeSurface(texsu);
}

GLuint
AG_SurfaceTexture(SDL_Surface *sourcesu, GLfloat *texcoord)
{
	SDL_Surface *texsu;
	GLuint texture;
	Uint32 saflags = sourcesu->flags & (SDL_SRCALPHA|SDL_RLEACCEL);
	Uint8 salpha = sourcesu->format->alpha;
	int w, h;
	
	w = powof2(sourcesu->w);
	h = powof2(sourcesu->h);

	/* The size of OpenGL surfaces must be a power of two. */
	if (texcoord != NULL) {
		texcoord[0] = 0.0f;
		texcoord[1] = 0.0f;
		texcoord[2] = (GLfloat)sourcesu->w / w;
		texcoord[3] = (GLfloat)sourcesu->h / h;
	}

	/* Create a surface with the masks expected by OpenGL. */
	texsu = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32,
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
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
	if (texsu == NULL)
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());

	/* Copy the source surface onto the GL texture surface. */
	SDL_SetAlpha(sourcesu, 0, 0);
	SDL_BlitSurface(sourcesu, NULL, texsu, NULL);
	SDL_SetAlpha(sourcesu, saflags, salpha);
	
	/* Create the OpenGL texture. */
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
	    GL_UNSIGNED_BYTE, texsu->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	SDL_FreeSurface(texsu);
	return (texture);
}

SDL_Surface *
AG_CaptureGLView(void)
{
	Uint8 *pixels, *tmp;
	SDL_Surface *su;

	pixels = Malloc(agView->w*agView->h*3, M_RG);
	glReadPixels(0, 0, agView->w, agView->h, GL_RGB, GL_UNSIGNED_BYTE,
	    pixels);
	AG_FlipSurface(pixels, agView->h, agView->w*3);
	su = SDL_CreateRGBSurfaceFrom(pixels, agView->w, agView->h, 24,
	    agView->w*3,
	    0x000000ff,
	    0x0000ff00,
	    0x00ff0000,
	    0);
	return (su);
}
#endif /* HAVE_OPENGL */

/*
 * Blend the specified components with the pixel at s:[x,y], using the
 * given alpha function.
 *
 * Clipping is not done; the destination surface must be locked.
 */
void
AG_BlendPixelRGBA(SDL_Surface *s, Uint8 *pDst, Uint8 sR, Uint8 sG, Uint8 sB,
    Uint8 sA, enum ag_blend_func func)
{
	Uint32 cDst;
	Uint8 dR, dG, dB, dA;
	int alpha = (int)dA;

	cDst = AG_GET_PIXEL(s, pDst);
	if ((s->flags & SDL_SRCCOLORKEY) && (cDst == s->format->colorkey)) {
	 	AG_PUT_PIXEL(s, pDst, SDL_MapRGBA(s->format, sR, sG, sB, sA));
	} else {
		SDL_GetRGBA(cDst, s->format, &dR, &dG, &dB, &dA);
		switch (func) {
		case AG_ALPHA_OVERLAY:
			alpha = dA+sA;
			break;
		case AG_ALPHA_SRC:
			alpha = sA;
			break;
		case AG_ALPHA_MEAN:
			alpha = (dA+sA)/2;
			break;
		case AG_ALPHA_SOURCE_MINUS_DST:
			alpha = sA-dA;
			break;
		case AG_ALPHA_DST_MINUS_SOURCE:
			alpha = dA-sA;
			break;
		case AG_ALPHA_PYTHAGOREAN:
			alpha = (int)sqrtf((dA*dA)+(sA*sA));
			break;
		default:
			break;
		}
		if (alpha < 0) {
			alpha = 0;
		} else if (alpha > 255) {
			alpha = 255;
		}
		AG_PUT_PIXEL(s, pDst, SDL_MapRGBA(s->format,
		    (((sR - dR) * sA) >> 8) + dR,
		    (((sG - dG) * sA) >> 8) + dG,
		    (((sB - dB) * sA) >> 8) + dB,
		    (Uint8)alpha));
	}
}

void
AG_ViewCapture(void)
{
	char path[MAXPATHLEN];
	
	AG_DumpSurface(agView->v, path);
	AG_TextTmsg(AG_MSG_INFO, 1000, _("Screenshot saved to %s."), path);
}

/* Dump a surface to a JPEG image. */
void
AG_DumpSurface(SDL_Surface *pSu, char *path_save)
{
#ifdef HAVE_JPEG
	SDL_Surface *su;
	char path[MAXPATHLEN];
	struct jpeg_error_mgr jerrmgr;
	struct jpeg_compress_struct jcomp;
	Uint8 *jcopybuf;
	FILE *fp;
	u_int seq = 0;
	int fd;
	JSAMPROW row[1];
	int x;

	if (AG_StringCopy(agConfig, "save-path", path, sizeof(path))
	    >= sizeof(path)) {
		goto toobig;
	}
	if (strlcat(path, "/screenshot", sizeof(path)) >= sizeof(path))
		goto toobig;
	if (Mkdir(path) == -1 && errno != EEXIST) {
		AG_TextMsg(AG_MSG_ERROR, "mkdir %s: %s", path, strerror(errno));
		return;
	}

	if (!agView->opengl || pSu != agView->v) {
		su = pSu;
	} else {
#ifdef HAVE_OPENGL
		su = AG_CaptureGLView();
#endif
	}

	for (;;) {
		char file[MAXPATHLEN];

		snprintf(file, sizeof(file), "%s/%s%u.jpg", path, agProgName,
		    seq++);
		if ((fd = open(file, O_WRONLY|O_CREAT|O_EXCL, 0600)) == -1) {
			if (errno == EEXIST) {
				continue;
			} else {
				AG_TextMsg(AG_MSG_ERROR, "open %s: %s", file,
				    strerror(errno));
				goto out;
			}
		}
		break;
	}
	if (path_save != NULL)
		strlcpy(path_save, path, MAXPATHLEN);

	if ((fp = fdopen(fd, "wb")) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, "fdopen: %s", strerror(errno));
		return;
	}

	jcomp.err = jpeg_std_error(&jerrmgr);

	jpeg_create_compress(&jcomp);

	jcomp.image_width = su->w;
	jcomp.image_height = su->h;
	jcomp.input_components = 3;
	jcomp.in_color_space = JCS_RGB;

	jpeg_set_defaults(&jcomp);
	jpeg_set_quality(&jcomp, agScreenshotQuality, TRUE);
	jpeg_stdio_dest(&jcomp, fp);

	jcopybuf = Malloc(su->w*3, M_VIEW);

	jpeg_start_compress(&jcomp, TRUE);
	while (jcomp.next_scanline < jcomp.image_height) {
		Uint8 *pSrc = (Uint8 *)su->pixels +
		    jcomp.next_scanline*su->pitch;
		Uint8 *pDst = jcopybuf;
		Uint8 r, g, b;

		for (x = agView->w; x > 0; x--) {
			SDL_GetRGB(AG_GET_PIXEL(su, pSrc), su->format,
			    &r, &g, &b);
			*pDst++ = r;
			*pDst++ = g;
			*pDst++ = b;
			pSrc += su->format->BytesPerPixel;
		}
		row[0] = jcopybuf;
		jpeg_write_scanlines(&jcomp, row, 1);
	}
	jpeg_finish_compress(&jcomp);
	jpeg_destroy_compress(&jcomp);

	fclose(fp);
	close(fd);
	Free(jcopybuf, M_VIEW);
out:
#ifdef HAVE_OPENGL
	if (agView->opengl && su != pSu)
		SDL_FreeSurface(su);
#endif
	return;
toobig:
	AG_TextMsg(AG_MSG_ERROR, _("Path is too big."));
#else
	AG_TextMsg(AG_MSG_ERROR, _("Screenshot feature requires libjpeg"));
#endif
}

/* Queue a video update. */
void
AG_UpdateRectQ(int x, int y, int w, int h)
{
#ifdef HAVE_OPENGL
	if (agView->opengl) {
		agView->ndirty = 1;
	} else
#endif
	{
		if (agView->ndirty+1 > agView->maxdirty) {
			agView->maxdirty *= 2;
			agView->dirty = Realloc(agView->dirty,
			    agView->maxdirty * sizeof(SDL_Rect));
		}
		agView->dirty[agView->ndirty].x = x;
		agView->dirty[agView->ndirty].y = y;
		agView->dirty[agView->ndirty].w = w;
		agView->dirty[agView->ndirty].h = h;
		agView->ndirty++;
	}
}

/* Flip the lines of a frame buffer; useful with glReadPixels(). */
void
AG_FlipSurface(Uint8 *src, int h, int pitch)
{
	Uint8 *tmp = Malloc(pitch, M_RG);
	int h2 = h >> 1;
	Uint8 *p1 = &src[0];
	Uint8 *p2 = &src[(h-1)*pitch];
	int i;

	for (i = 0; i < h2; i++) {
		memcpy(tmp, p1, pitch);
		memcpy(p1, p2, pitch);
		memcpy(p2, tmp, pitch);

		p1 += pitch;
		p2 -= pitch;
	}
	Free(tmp, M_RG);
}
