/*	$Csoft: view.c,v 1.176 2005/05/08 09:22:42 vedge Exp $	*/

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
#include <engine/map/rootmap.h>
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
struct viewport *view = NULL;
SDL_PixelFormat *vfmt = NULL;
const SDL_VideoInfo *vinfo;
int view_screenshot_quality = 75;

int
view_init(enum gfx_engine ge)
{
	int screenflags = SDL_SWSURFACE;
	int depth;

	if (view != NULL) {
		error_set(_("The viewport is already initialized."));
		return (-1);
	}

	view = Malloc(sizeof(struct viewport), M_VIEW);
	view->gfx_engine = ge;
	view->rootmap = NULL;
	view->winop = VIEW_WINOP_NONE;
	view->ndirty = 0;
	view->maxdirty = 4;
	view->dirty = Malloc(view->maxdirty * sizeof(SDL_Rect), M_VIEW);
	view->opengl = 0;
	view->modal_win = NULL;
	view->wop_win = NULL;
	view->focus_win = NULL;
	TAILQ_INIT(&view->windows);
	TAILQ_INIT(&view->detach);
	pthread_mutex_init(&view->lock, &recursive_mutexattr);

	depth = prop_get_uint8(config, "view.depth");
	view->w = prop_get_uint16(config, "view.w");
	view->h = prop_get_uint16(config, "view.h");
	dprintf("setting mode %ux%ux%u\n", view->w, view->h, depth);

	if (prop_get_bool(config, "view.full-screen"))
		screenflags |= SDL_FULLSCREEN;
	if (prop_get_bool(config, "view.async-blits"))
		screenflags |= SDL_HWSURFACE|SDL_ASYNCBLIT;

	view->depth = SDL_VideoModeOK(view->w, view->h, depth, screenflags);
	if (view->depth == 8)
		screenflags |= SDL_HWPALETTE;

	switch (view->gfx_engine) {
	case GFX_ENGINE_TILEBASED:
		dprintf("direct video / tile-based\n");
		view->w -= view->w % TILESZ;
		view->h -= view->h % TILESZ;
		view->rootmap = Malloc(sizeof(struct viewmap), M_VIEW);
		rootmap_init(view->rootmap, view->w/TILESZ, view->h/TILESZ);
		break;
	case GFX_ENGINE_GUI:
		dprintf("direct video / gui\n");
		screenflags |= SDL_RESIZABLE;		/* XXX thread unsafe? */
		break;
	default:
		error_set(_("Unsupported graphic mode."));
		goto fail;
	}

#ifdef HAVE_OPENGL
	if (prop_get_bool(config, "view.opengl")) {
		screenflags |= SDL_OPENGL;
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		
		view->opengl = 1;
	}
#endif

	if (view->w < prop_get_uint16(config, "view.min-w") ||
	    view->h < prop_get_uint16(config, "view.min-h")) {
		error_set(_("The resolution is too small."));
		goto fail;
	}

	/* Set the video mode. */
	view->v = SDL_SetVideoMode(view->w, view->h, 0, screenflags);
	if (view->v == NULL) {
		error_set(_("Setting %dx%dx%d mode: %s"), view->w, view->h,
		    view->depth, SDL_GetError());
		goto fail;
	}
	vfmt = view->v->format;
	printf(_("Video display is %dbpp "
	         "(ckey=0x%x, alpha=0x%04x)\n"),
	    vfmt->BitsPerPixel, vfmt->colorkey,
	    vfmt->alpha);

#ifdef HAVE_OPENGL
	if (view->opengl) {
		int red, blue, green, alpha, depth, bsize;
		Uint8 bR, bG, bB;

		SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &red);
		SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE, &green);
		SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE, &blue);
		SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &alpha);
		SDL_GL_GetAttribute(SDL_GL_BUFFER_SIZE, &bsize);
		SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &depth);

		prop_set_int(config, "view.gl.depth", depth);
		prop_set_int(config, "view.gl.red_size", red);
		prop_set_int(config, "view.gl.green_size", green);
		prop_set_int(config, "view.gl.blue_size", blue);
		prop_set_int(config, "view.gl.alpha_size", alpha);
		prop_set_int(config, "view.gl.buffer_size", bsize);

		dprintf("gl depth=%d size=(%d,%d,%d,%d) bsize=%d\n", depth,
		    red, green, blue, alpha, bsize);
	
		glViewport(0, 0, view->w, view->h);
		glOrtho(0, view->w, view->h, 0, -1.0, 1.0);

		SDL_GetRGB(COLOR(BG_COLOR), vfmt, &bR, &bG, &bB);
		glClearColor(bR/255.0, bG/255.0, bB/255.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DITHER);
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);
	} else
#endif /* HAVE_OPENGL */
	{
		SDL_FillRect(view->v, NULL, COLOR(BG_COLOR));
		SDL_UpdateRect(view->v, 0, 0, view->v->w, view->v->h);
	}

	prop_set_uint16(config, "view.w", view->w);
	prop_set_uint16(config, "view.h", view->h);

	if (view_set_refresh(prop_get_uint8(config, "view.fps")) == -1) {
		fprintf(stderr, "%s\n", error_get());
		goto fail;
	}
	return (0);
fail:
	pthread_mutex_destroy(&view->lock);
	Free(view, M_VIEW);
	view = NULL;
	return (-1);
}

int
view_resize(int w, int h)
{
	Uint32 flags = view->v->flags & (SDL_SWSURFACE|SDL_FULLSCREEN|
	                                 SDL_HWSURFACE|SDL_ASYNCBLIT|
					 SDL_HWPALETTE|SDL_RESIZABLE|
					 SDL_OPENGL);
	struct window *win;
	SDL_Surface *su;
	int ow, oh;

	/*
	 * Set initial coordinates of windows that might have not been
	 * scaled yet.
	 */
	TAILQ_FOREACH(win, &view->windows, windows) {
		pthread_mutex_lock(&win->lock);
		if (!win->visible) {
			WINDOW_UPDATE(win);
		}
		pthread_mutex_unlock(&win->lock);
	}

	/* XXX set a minimum! */
	if ((su = SDL_SetVideoMode(w, h, 0, flags)) == NULL) {
		error_set("resize to %ux%u: %s", w, h, SDL_GetError());
		return (-1);
	}
	ow = view->w;
	oh = view->h;

	view->v = su;
	view->w = w;
	view->h = h;
	prop_set_uint16(config, "view.w", w);
	prop_set_uint16(config, "view.h", h);

	TAILQ_FOREACH(win, &view->windows, windows) {
		pthread_mutex_lock(&win->lock);
		WIDGET(win)->x = WIDGET(win)->x*w/ow;
		WIDGET(win)->y = WIDGET(win)->y*h/oh;
		WIDGET(win)->w = WIDGET(win)->w*w/ow;
		WIDGET(win)->h = WIDGET(win)->h*h/oh;
		WINDOW_UPDATE(win);
		pthread_mutex_unlock(&win->lock);
	}
	return (0);
}

void
view_videoexpose(void)
{
	struct window *win;

	if (view->gfx_engine == GFX_ENGINE_TILEBASED)
		rootmap_redraw();

	TAILQ_FOREACH(win, &view->windows, windows) {
		pthread_mutex_lock(&win->lock);
		if (win->visible) {
			widget_draw(win);
		}
		pthread_mutex_unlock(&win->lock);
	}

#ifdef HAVE_OPENGL
	if (view->opengl) {
		dprintf("swapping gl buffers\n");
		SDL_GL_SwapBuffers();
	}
#endif
}

void
view_destroy(void)
{
	struct window *win, *nwin;

	for (win = TAILQ_FIRST(&view->windows);
	     win != TAILQ_END(&view->windows);
	     win = nwin) {
		nwin = TAILQ_NEXT(win, windows);
		object_destroy(win);
		Free(win, M_OBJECT);
	}

	Free(view->rootmap, M_VIEW);
	Free(view->dirty, M_VIEW);
	pthread_mutex_destroy(&view->lock);
	Free(view, M_VIEW);
	view = NULL;
}

/* Return the named window or NULL if there is no such window. */
struct window *
view_window_exists(const char *name)
{
	struct window *win;

	pthread_mutex_lock(&view->lock);
	TAILQ_FOREACH(win, &view->windows, windows) {
		if (strcmp(OBJECT(win)->name, name) == 0)
			break;
	}
	pthread_mutex_unlock(&view->lock);
	return (win);
}

/* Attach a window to a view. */
void
view_attach(void *child)
{
	struct window *win = child;
	
	pthread_mutex_lock(&view->lock);
	view->focus_win = NULL;
	TAILQ_INSERT_TAIL(&view->windows, win, windows);
	pthread_mutex_unlock(&view->lock);
}

static void
detach_window(struct window *win)
{
	struct window *subwin, *nsubwin;
	struct window *owin;

	for (subwin = TAILQ_FIRST(&win->subwins);
	     subwin != TAILQ_END(&win->subwins);
	     subwin = nsubwin) {
		nsubwin = TAILQ_NEXT(subwin, swins);
		detach_window(subwin);
	}
	TAILQ_INIT(&win->subwins);
	
	window_hide(win);
	event_post(view, win, "detached", NULL);
	TAILQ_REMOVE(&view->windows, win, windows);
	TAILQ_INSERT_TAIL(&view->detach, win, detach);

	TAILQ_FOREACH(owin, &view->windows, windows) {
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
view_detach(struct window *win)
{
	pthread_mutex_lock(&view->lock);
	detach_window(win);
	pthread_mutex_unlock(&view->lock);
}

/* Return the 32-bit form of the pixel at the given location. */
Uint32
view_get_pixel(SDL_Surface *s, Uint8 *pSrc)
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
view_put_pixel(SDL_Surface *s, Uint8 *pDst, Uint32 cDst)
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
view_detach_queued(void)
{
	struct window *win, *nwin;

	for (win = TAILQ_FIRST(&view->detach);
	     win != TAILQ_END(&view->detach);
	     win = nwin) {
		nwin = TAILQ_NEXT(win, detach);
		object_destroy(win);
		Free(win, M_OBJECT);
	}
	TAILQ_INIT(&view->detach);
}

/* Return a newly allocated surface containing a copy of ss. */
SDL_Surface *
view_copy_surface(SDL_Surface *ss)
{
	SDL_Surface *rs;

	rs = SDL_ConvertSurface(ss, ss->format, SDL_SWSURFACE |
	    (ss->flags & (SDL_SRCCOLORKEY|SDL_SRCALPHA|SDL_RLEACCEL)));
	if (rs == NULL) {
		error_set("SDL_ConvertSurface: %s", SDL_GetError());
		return (NULL);
	}
	rs->format->alpha = ss->format->alpha;
	rs->format->colorkey = ss->format->colorkey;
	return (rs);
}

int
view_same_pixel_fmt(SDL_Surface *s1, SDL_Surface *s2)
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
view_scale_surface(SDL_Surface *ss, Uint16 w, Uint16 h, SDL_Surface **ds)
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
		//same_fmt = view_same_pixel_fmt(*ds, ss);
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
			Uint32 cSrc = GET_PIXEL(ss, pSrc);
			Uint32 cDst;

			if (same_fmt) {
				cDst = cSrc;
			} else {
				SDL_GetRGBA(cSrc, ss->format,
				    &r1, &g1, &b1, &a1);
				cDst = SDL_MapRGBA((*ds)->format,
				    r1, g1, b1, a1);
			}
			PUT_PIXEL((*ds), pDst, cDst);
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
view_pixels_alpha(SDL_Surface *su, Uint8 alpha)
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

			PUT_PIXEL(su, dst,
			    SDL_MapRGBA(su->format, r, g, b, a));
		}
	}
	if (SDL_MUSTLOCK(su))
		SDL_UnlockSurface(su);
}

int
view_set_refresh(int ms)
{
	int fps = 1000/ms;

	dprintf("%d fps\n", fps);
	if (fps < 4 || fps > 240) {
		error_set(_("The refresh rate is out of range."));
		return (-1);
	}
	pthread_mutex_lock(&view->lock);
	view->refresh.r = 0;
	view->refresh.rnom = fps;
	pthread_mutex_unlock(&view->lock);
	return (0);
}

#ifdef HAVE_OPENGL

static __inline__ int
powof2(int i)
{
	int val = 1;

	while (val < i) {
		val <<= 1;
	}
	return (val);
}

void
view_update_texture(SDL_Surface *sourcesu, int texture)
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
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
	    GL_UNSIGNED_BYTE, texsu->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	SDL_FreeSurface(texsu);
}

GLuint
view_surface_texture(SDL_Surface *sourcesu, GLfloat *texcoord)
{
	SDL_Surface *texsu;
	GLuint texture;
	Uint32 saflags = sourcesu->flags & (SDL_SRCALPHA|SDL_RLEACCEL);
	Uint8 salpha = sourcesu->format->alpha;
	int w, h;

	/* The size of OpenGL surfaces must be a power of two. */
	w = powof2(sourcesu->w);
	h = powof2(sourcesu->h);
	texcoord[0] = 0.0f;
	texcoord[1] = 0.0f;
	texcoord[2] = (GLfloat)sourcesu->w / w;
	texcoord[3] = (GLfloat)sourcesu->h / h;

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
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
	    GL_UNSIGNED_BYTE, texsu->pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	SDL_FreeSurface(texsu);
	return (texture);
}

#endif /* HAVE_OPENGL */

/*
 * Blend the specified components with the pixel at s:[x,y], using the
 * given source alpha value.
 *
 * Clipping is not done; the destination surface must be locked.
 */
void
view_blend_rgba(SDL_Surface *s, Uint8 *pDst, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	Uint32 cDst;
	Uint8 dR, dG, dB;

	cDst = GET_PIXEL(s, pDst);
	SDL_GetRGB(cDst, s->format, &dR, &dG, &dB);
	dR = (((r - dR) * a) >> 8) + dR;
	dG = (((g - dG) * a) >> 8) + dG;
	dB = (((b - dB) * a) >> 8) + dB;
	PUT_PIXEL(s, pDst, SDL_MapRGB(s->format, dR, dG, dB));
}

/* Dump a surface to a JPEG image. */
void
view_capture(SDL_Surface *su)
{
#ifdef HAVE_JPEG
	char path[MAXPATHLEN];
	struct jpeg_error_mgr jerrmgr;
	struct jpeg_compress_struct jcomp;
	Uint8 *jcopybuf;
	FILE *fp;
	u_int seq = 0;
	int fd;
	JSAMPROW row[1];
	int x;

	if (prop_copy_string(config, "save-path", path, sizeof(path)) >=
	    sizeof(path))
		goto toobig;
	if (strlcat(path, "/screenshot", sizeof(path)) >= sizeof(path))
		goto toobig;
	if (Mkdir(path) == -1 && errno != EEXIST) {
		text_msg(MSG_ERROR, "mkdir %s: %s", path, strerror(errno));
		return;
	}

	for (;;) {
		char file[MAXPATHLEN];

		snprintf(file, sizeof(file), "%s/%s%u.jpg", path, progname,
		    seq++);
		if ((fd = open(file, O_WRONLY|O_CREAT|O_EXCL, 0600)) == -1) {
			if (errno == EEXIST) {
				continue;
			} else {
				text_msg(MSG_ERROR, "open %s: %s", file,
				    strerror(errno));
				return;
			}
		}
		break;
	}

	if ((fp = fdopen(fd, "wb")) == NULL) {
		text_msg(MSG_ERROR, "fdopen: %s", strerror(errno));
		return;
	}

	jcomp.err = jpeg_std_error(&jerrmgr);

	jpeg_create_compress(&jcomp);

	jcomp.image_width = su->w;
	jcomp.image_height = su->h;
	jcomp.input_components = 3;
	jcomp.in_color_space = JCS_RGB;

	jpeg_set_defaults(&jcomp);
	jpeg_set_quality(&jcomp, view_screenshot_quality, TRUE);
	jpeg_stdio_dest(&jcomp, fp);

	jcopybuf = Malloc(su->w*3, M_VIEW);

	jpeg_start_compress(&jcomp, TRUE);
	while (jcomp.next_scanline < jcomp.image_height) {
		Uint8 *src = (Uint8 *)su->pixels +
		    jcomp.next_scanline*su->pitch;
		Uint8 *dst = jcopybuf;
		Uint8 r, g, b;

		for (x = view->w; x > 0; x--) {
			switch (su->format->BytesPerPixel) {
			case 4:
				SDL_GetRGB(*(Uint32 *)src, su->format,
				    &r, &g, &b);
				break;
			case 3:
			case 2:
				SDL_GetRGB(*(Uint16 *)src, su->format,
				    &r, &g, &b);
				break;
			case 1:
				SDL_GetRGB(*src, su->format,
				    &r, &g, &b);
				break;
			}
			*dst++ = r;
			*dst++ = g;
			*dst++ = b;
			src += su->format->BytesPerPixel;
		}
		row[0] = jcopybuf;
		jpeg_write_scanlines(&jcomp, row, 1);
	}
	jpeg_finish_compress(&jcomp);
	jpeg_destroy_compress(&jcomp);
	fclose(fp);
	close(fd);
	Free(jcopybuf, M_VIEW);
	return;
toobig:
	text_msg(MSG_ERROR, "Screenshot path is too big");
#else
	text_msg(MSG_ERROR, _("Screenshot feature requires libjpeg"));
#endif
}

/* Queue a video update. */
void
view_update(int x, int y, int w, int h)
{
#ifdef HAVE_OPENGL
	if (view->opengl) {
		view->ndirty = 1;
	} else
#endif
	{
		if (view->ndirty+1 > view->maxdirty) {
			view->maxdirty *= 2;
			view->dirty = Realloc(view->dirty, view->maxdirty *
			                                   sizeof(SDL_Rect));
		}
		view->dirty[view->ndirty].x = x;
		view->dirty[view->ndirty].y = y;
		view->dirty[view->ndirty].w = w;
		view->dirty[view->ndirty].h = h;
		view->ndirty++;
	}
}

/* Parse a command-line refresh rate specification. */
void
view_parse_fpsspec(const char *fpsspec)
{
	int fps;

	fps = atoi(fpsspec);
	if (fps < 4 || fps > 240) {
		fprintf(stderr, _("Unreasonable refresh rate; ignoring\n"));
		return;
	}
	prop_set_uint8(config, "view.fps", fps);
}

