/*	$Csoft: view.c,v 1.104 2003/01/03 23:29:56 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003 CubeSoft Communications, Inc.
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

#include "engine.h"

#include "rootmap.h"
#include "map.h"
#include "physics.h"
#include "config.h"
#include "view.h"
#include "world.h"

#include "widget/widget.h"
#include "widget/window.h"
#include "widget/primitive.h"

static const struct object_ops viewport_ops = {
	view_destroy,
	NULL,		/* load */
	NULL		/* save */
};

/* Read-only as long as the engine is running. */
struct viewport *view = NULL;

#ifdef DEBUG
int	view_debug = 1;
#define engine_debug view_debug
#endif

/* Initialize the graphic engine. */
int
view_init(enum gfx_engine ge)
{
	struct viewport *v;
	int screenflags = SDL_SWSURFACE;
	int depth;

	if (view != NULL) {
		error_set("viewport is already initialized");
		return (-1);
	}

	v = emalloc(sizeof(struct viewport));
	object_init(&v->obj, "view-port", "view", NULL, OBJECT_SYSTEM,
	    &viewport_ops);
	v->gfx_engine = ge;
	v->rootmap = NULL;
	v->winop = VIEW_WINOP_NONE;
	v->ndirty = 0;
	v->maxdirty = 4096;
	v->dirty = emalloc(v->maxdirty * sizeof(SDL_Rect *));
	v->opengl = 0;
	TAILQ_INIT(&v->windows);
	TAILQ_INIT(&v->detach);
	pthread_mutexattr_init(&v->lockattr);
	pthread_mutexattr_settype(&v->lockattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&v->lock, &v->lockattr);

	/* Obtain the display preferences. */
	depth = prop_get_uint8(config, "view.depth");
	v->w = prop_get_uint16(config, "view.w");
	v->h = prop_get_uint16(config, "view.h");
	if (prop_get_bool(config, "view.full-screen"))
		screenflags |= SDL_FULLSCREEN;
	if (prop_get_bool(config, "view.async-blits"))
		screenflags |= SDL_ASYNCBLIT;

	/* Negotiate the depth. */
	v->depth = SDL_VideoModeOK(v->w, v->h, depth, screenflags);
	if (v->depth == 8)
		screenflags |= SDL_HWPALETTE;

	switch (v->gfx_engine) {
	case GFX_ENGINE_TILEBASED:
		dprintf("direct video / tile-based\n");
	
		/* Adapt resolution to tile geometry. */
		v->w -= v->w % TILEW;
		v->h -= v->h % TILEH;
		dprintf("rounded resolution to %dx%d\n", v->w, v->h);

		/* Initialize the map display. */
		v->rootmap = emalloc(sizeof(struct viewmap));
		rootmap_init(v->rootmap, v->w / TILEW, v->h / TILEH);
		break;
	case GFX_ENGINE_GUI:
		dprintf("direct video / gui\n");
		screenflags |= SDL_RESIZABLE;		/* XXX thread unsafe? */
		break;
	default:
		error_set("unsupported graphic mode");
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
		
		v->opengl = 1;
	}
#endif

	if (v->w < 320 || v->h < 240) {
		error_set("minimum resolution is 320x240");
		goto fail;
	}

	/* Set the video mode. */
	v->v = SDL_SetVideoMode(v->w, v->h, 0, screenflags);
	if (v->v == NULL) {
		error_set("setting %dx%dx%d mode: %s", v->w, v->h, v->depth,
		    SDL_GetError());
		goto fail;
	}

#ifdef HAVE_OPENGL
	if (v->opengl) {
		int red, blue, green, alpha, depth, bsize;

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

		glViewport(0, 0, v->w, v->h);
		glOrtho(0, v->w, v->h, 0, -1.0, 1.0);
		glClearColor(0, 0, 0, 0);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
#endif /* HAVE_OPENGL */

	prop_set_uint16(config, "view.w", v->w);
	prop_set_uint16(config, "view.h", v->h);
	view = v;

	primitives_init();
	return (0);
fail:
	pthread_mutex_destroy(&v->lock);
	pthread_mutexattr_destroy(&v->lockattr);
	free(v);
	return (-1);
}

/*
 * Process all windows on the detach queue. This is executed after
 * window list traversal by the event loop.
 *
 * The view must be locked, the detach queue must not be empty.
 */
void
view_detach_queued(void)
{
	struct window *win, *nwin;

	for (win = TAILQ_FIRST(&view->detach);
	     win != TAILQ_END(&view->detach);
	     win = nwin) {
		nwin = TAILQ_NEXT(win, detach);

		TAILQ_REMOVE(&view->windows, win, windows);
		window_hide(win);
		event_post(win, "detached", "%p", view);
		object_destroy(win);
	}
	TAILQ_INIT(&view->detach);
}

void
view_destroy(void *p)
{
	struct viewport *v = p;
	struct window *win;
	
	pthread_mutex_lock(&v->lock);
	
	if (v->rootmap != NULL) {
		rootmap_free_maprects(v);
		free(v->rootmap);
		v->rootmap = NULL;
	}

	TAILQ_FOREACH(win, &v->windows, windows) {
		win->flags &= ~(WINDOW_MATERIALIZE|WINDOW_DEMATERIALIZE);
		view_detach(win);
	}
	if (!TAILQ_EMPTY(&view->detach)) {
		view_detach_queued();
	}

	free(v->dirty);

	pthread_mutex_unlock(&v->lock);

	pthread_mutex_destroy(&v->lock);
	pthread_mutexattr_destroy(&v->lockattr);
}

/* Attach a window to a view. */
void
view_attach(void *child)
{
	struct window *win = child;
	
	pthread_mutex_lock(&view->lock);

	view->focus_win = NULL;

	OBJECT_ASSERT(child, "window");
	event_post(child, "attached", "%p", view);
	TAILQ_INSERT_TAIL(&view->windows, win, windows);
	
	pthread_mutex_unlock(&view->lock);
}

/* Detach a window from a view. */
void
view_detach(void *child)
{
	struct window *win = child;
	
	OBJECT_ASSERT(child, "window");

	/*
	 * Allow windows to detach themselves only after event processing
	 * is complete.
	 */
	pthread_mutex_lock(&view->lock);
	TAILQ_INSERT_HEAD(&view->detach, win, detach);
	pthread_mutex_unlock(&view->lock);
}

SDL_Surface *
view_surface(int flags, int w, int h)
{
	SDL_Surface *s;

	s = SDL_CreateRGBSurface(flags, w, h, 32,
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
	if (s == NULL) {
		fatal("SDL_CreateRGBSurface: %s\n", SDL_GetError());
	}
	return (s);
}

/*
 * Allocate a new surface containing a bitmap of ss scaled to wxh.
 * The source surface must not be locked by the caller thread.
 */
SDL_Surface *
view_scale_surface(SDL_Surface *ss, Uint16 w, Uint16 h)
{
	SDL_Surface *ds;
	Uint32 col =0;
	Uint8 *src, *dst, r1, g1, b1, a1;
	int x, y;

	ds = view_surface(SDL_SWSURFACE, w, h);
	
	if (ss->w == w && ss->h == h) {			/* Just copy */
		SDL_SetAlpha(ss, 0, 0);
		SDL_BlitSurface(ss, NULL, ds, NULL);
		SDL_SetAlpha(ss, SDL_SRCALPHA, SDL_ALPHA_TRANSPARENT);
		return (ds);
	}

	if (SDL_MUSTLOCK(ss))
		SDL_LockSurface(ss);
	if (SDL_MUSTLOCK(ds))
		SDL_LockSurface(ds);

	for (y = 0; y < ds->h; y++) {
		for (x = 0; x < ds->w; x++) {
			src = (Uint8 *)ss->pixels +
			    (y * ss->h / ds->h) * ss->pitch +
			    (x * ss->w / ds->w) * ss->format->BytesPerPixel;
			dst = (Uint8 *)ds->pixels +
			    y * ds->pitch +
			    x * ds->format->BytesPerPixel;

			switch (ss->format->BytesPerPixel) {
			case 1:
				SDL_GetRGBA(*src, ss->format,
				    &r1, &g1, &b1, &a1);
				break;
			case 2:
				SDL_GetRGBA(*(Uint16 *)src, ss->format,
				    &r1, &g1, &b1, &a1);
				break;
			case 3:
				SDL_GetRGBA(*(Uint16 *)src, ss->format,
				    &r1, &g1, &b1, &a1);
				break;
			case 4:
				SDL_GetRGBA(*(Uint32 *)src, ss->format,
				    &r1, &g1, &b1, &a1);
				break;
			}

			/* Transparency hack for text surfaces. */
			if (r1 == 15 && g1 == 15 && b1 == 15) {
				a1 = 0;
			}

			col = SDL_MapRGBA(ds->format, r1, g1, b1, a1);
			switch (ds->format->BytesPerPixel) {
				_VIEW_PUTPIXEL_8(dst, col)
				_VIEW_PUTPIXEL_16(dst, col)
				_VIEW_PUTPIXEL_24(dst, col)
				_VIEW_PUTPIXEL_32(dst, col)
			}
		}
	}

	if (SDL_MUSTLOCK(ds))
		SDL_UnlockSurface(ds);
	if (SDL_MUSTLOCK(ss))
		SDL_UnlockSurface(ss);
	return (ds);
}

int
view_set_refresh(int min_delay, int max_delay)
{
	if (min_delay < 0 || min_delay > 300 ||
	    max_delay < 0 || max_delay > 300) {
		error_set("out of range");
		return (-1);
	}

	pthread_mutex_lock(&view->lock);
	view->refresh.current = 0;
	view->refresh.delay = max_delay;
	view->refresh.max_delay = max_delay;
	view->refresh.min_delay = min_delay;
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

GLuint
view_surface_texture(SDL_Surface *sourcesu, GLfloat *texcoord)
{
	SDL_Surface *texsu;
	GLuint texture;
	Uint32 sflags;
	Uint8 salpha;
	int w, h;

	w = powof2(sourcesu->w);
	h = powof2(sourcesu->h);
	texcoord[0] = 0.0f;
	texcoord[1] = 0.0f;
	texcoord[2] = (GLfloat)sourcesu->w / w;
	texcoord[3] = (GLfloat)sourcesu->h / h;

	/* Create a surface with the OpenGL RGBA masks. */
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
	if (texsu == NULL) {
		fatal("SDL_CreateRGBSurface: %s\n", SDL_GetError());
	}

	/* Disable alpha blending state of the source surface. */
	sflags = sourcesu->flags & (SDL_SRCALPHA|SDL_RLEACCELOK);
	salpha = sourcesu->format->alpha;
	if (sflags & SDL_SRCALPHA) {
		SDL_SetAlpha(sourcesu, 0, 0);
	}

	/* Copy the source surface into the GL texture surface. */
	if (SDL_BlitSurface(sourcesu, NULL, texsu, NULL) == -1) {
		fatal("SDL_BlitSurface: %s\n", SDL_GetError());
	}

	/* Restore the alpha blending state of the source surface. */
	if (sflags & SDL_SRCALPHA) {
		SDL_SetAlpha(sourcesu, sflags, salpha);
	}

	/* Create the OpenGL texture. */
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
	    GL_UNSIGNED_BYTE, texsu->pixels);
	SDL_FreeSurface(texsu);

	return (texture);
}

#endif /* HAVE_OPENGL */

/* Alpha-blend two pixels in software. */
__inline__ void
view_alpha_blend(SDL_Surface *s, Sint16 x, Sint16 y, Uint8 r, Uint8 g,
    Uint8 b, Uint8 a)
{
	Uint32 color, dstcolor;
	Uint8 dr, dg, db;
	Uint8 *dst;

	if (!VIEW_INSIDE_CLIP_RECT(s, x, y)) {
		return;
	}

	dst = (Uint8 *)s->pixels + y*s->pitch + x*s->format->BytesPerPixel;
	switch (s->format->BytesPerPixel) {
	case 1:
		dstcolor = *dst;
		break;
	case 2:
		dstcolor = *(Uint16 *)dst;
		break;
	case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		dstcolor = (dst[0] << 16) +
		           (dst[1] << 8) +
			    dst[2];
#else
		dstcolor =  dst[0] +
		           (dst[1] << 8) +
			   (dst[2] << 16);
#endif
		break;
	case 4:
		dstcolor = *(Uint32 *)dst;
		break;
	default:
		fatal("bad bpp\n");
	}
	SDL_GetRGB(dstcolor, s->format, &dr, &dg, &db);

	dr = (((r - dr) * a) >> 8) + dr;
	dg = (((g - dg) * a) >> 8) + dg;
	db = (((b - db) * a) >> 8) + db;
	color = SDL_MapRGB((s)->format, dr, dg, db);

	switch (s->format->BytesPerPixel) {
		_VIEW_PUTPIXEL_8(dst,  color)
		_VIEW_PUTPIXEL_16(dst, color)
		_VIEW_PUTPIXEL_24(dst, color)
		_VIEW_PUTPIXEL_32(dst, color)
	}
}
