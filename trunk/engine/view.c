/*	$Csoft: view.c,v 1.127 2003/06/17 23:30:43 vedge Exp $	*/

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

#include <config/have_jpeg.h>

#include <engine/engine.h>
#include <engine/rootmap.h>
#include <engine/map.h>
#include <engine/physics.h>
#include <engine/config.h>
#include <engine/view.h>
#include <engine/prop.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

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

#ifdef DEBUG
int	view_debug = 1;
#define engine_debug view_debug
#endif

/* Initialize the graphic engine. */
int
view_init(enum gfx_engine ge)
{
	int screenflags = SDL_SWSURFACE;
	int depth;

	if (view != NULL) {
		error_set("viewport is already initialized");
		return (-1);
	}

	view = Malloc(sizeof(struct viewport));
	view->gfx_engine = ge;
	view->rootmap = NULL;
	view->winop = VIEW_WINOP_NONE;
	view->ndirty = 0;
	view->maxdirty = 4;
	view->dirty = Malloc(view->maxdirty * sizeof(SDL_Rect));
	view->opengl = 0;
	TAILQ_INIT(&view->windows);
	TAILQ_INIT(&view->detach);
	pthread_mutex_init(&view->lock, &recursive_mutexattr);

	/* Obtain the display preferences. */
	depth = prop_get_uint8(config, "view.depth");
	view->w = prop_get_uint16(config, "view.w");
	view->h = prop_get_uint16(config, "view.h");
	if (prop_get_bool(config, "view.full-screen"))
		screenflags |= SDL_FULLSCREEN;
	if (prop_get_bool(config, "view.async-blits"))
		screenflags |= SDL_HWSURFACE|SDL_ASYNCBLIT;

	/* Negotiate the depth. */
	view->depth = SDL_VideoModeOK(view->w, view->h, depth, screenflags);
	if (view->depth == 8)
		screenflags |= SDL_HWPALETTE;

	switch (view->gfx_engine) {
	case GFX_ENGINE_TILEBASED:
		dprintf("direct video / tile-based\n");
	
		/* Adapt resolution to tile geometry. */
		view->w -= view->w % TILEW;
		view->h -= view->h % TILEH;
		dprintf("rounded resolution to %dx%d\n", view->w, view->h);

		/* Initialize the map display. */
		view->rootmap = Malloc(sizeof(struct viewmap));
		rootmap_init(view->rootmap, view->w / TILEW, view->h / TILEH);
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
		
		view->opengl = 1;
	}
#endif

	if (view->w < prop_get_uint16(config, "view.min-w") ||
	    view->h < prop_get_uint16(config, "view.min-h")) {
		error_set("resolution is too small");
		goto fail;
	}

	/* Set the video mode. */
	view->v = SDL_SetVideoMode(view->w, view->h, 0, screenflags);
	if (view->v == NULL) {
		error_set("setting %dx%dx%d mode: %s", view->w, view->h,
		    view->depth, SDL_GetError());
		goto fail;
	}

#ifdef HAVE_OPENGL
	if (view->opengl) {
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

		glViewport(0, 0, view->w, view->h);
		glOrtho(0, view->w, view->h, 0, -1.0, 1.0);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DITHER);
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
#endif /* HAVE_OPENGL */

	prop_set_uint16(config, "view.w", view->w);
	prop_set_uint16(config, "view.h", view->h);
	vfmt = view->v->format;

	primitives_init();
	return (0);
fail:
	pthread_mutex_destroy(&view->lock);
	free(view);
	view = NULL;
	return (-1);
}

/* Release the resources allocated by the graphic engine. */
void
view_destroy(void)
{
	struct window *win, *nwin;

	for (win = TAILQ_FIRST(&view->windows);
	     win != TAILQ_END(&view->windows);
	     win = nwin) {
		nwin = TAILQ_NEXT(win, windows);
		dprintf("freeing: %s (attached)\n", OBJECT(win)->name);
		window_hide(win);
		event_post(win, "detached", "%p", view);
#if 0
		/* XXX lock */
		object_destroy(win);
#endif
		free(win);
	}
	TAILQ_INIT(&view->windows);

	if (view->rootmap != NULL) {
		rootmap_free_maprects(view);
		free(view->rootmap);
	}
	free(view->dirty);
	pthread_mutex_destroy(&view->lock);

	free(view);
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
	event_post(child, "attached", "%p", view);
	TAILQ_INSERT_TAIL(&view->windows, win, windows);

	pthread_mutex_unlock(&view->lock);
}

/* Detach a window from a view. */
void
view_detach(void *child)
{
	struct window *win = child;
	
	/*
	 * Allow windows to detach themselves only after event processing
	 * is complete.
	 */
	pthread_mutex_lock(&view->lock);
	TAILQ_INSERT_HEAD(&view->detach, win, detach);
	pthread_mutex_unlock(&view->lock);
}

/* Allocate a 32bpp surface with native masks. */
SDL_Surface *
view_surface(Uint32 flags, int w, int h)
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
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
	}
	return (s);
}

/* Return a newly allocated surface containing a copy of ss. */
SDL_Surface *
view_copy_surface(SDL_Surface *ss)
{
	SDL_Surface *rs;

	rs = SDL_ConvertSurface(ss, ss->format,
	    SDL_SWSURFACE |
	    (ss->flags & (SDL_SRCCOLORKEY|SDL_SRCALPHA|SDL_RLEACCEL)));
	if (rs == NULL) {
		error_set("SDL_ConvertSurface: %s", SDL_GetError());
		return (NULL);
	}
	return (rs);
}

/*
 * Allocate a new surface containing a bitmap of ss scaled to wxh.
 * The source surface must not be locked by the caller thread.
 */
SDL_Surface *
view_scale_surface(SDL_Surface *ss, Uint16 w, Uint16 h)
{
	SDL_Surface *ds;
	Uint32 col = 0;
	Uint8 *src, *dst, r1, g1, b1, a1;
	int x, y;

	ds = view_surface(ss->flags, w, h);
	ds->format->alpha = ss->format->alpha;
	ds->format->colorkey = ss->format->colorkey;

	if (ss->w == w && ss->h == h) {
		Uint32 saflags = ss->flags & (SDL_SRCALPHA|SDL_RLEACCEL);
		Uint8 salpha = ss->format->alpha;
		Uint32 sckflags = ss->flags & (SDL_SRCCOLORKEY|SDL_RLEACCEL);
		Uint32 sckey = ss->format->colorkey;

		/* Return a copy of the original surface. */
		SDL_SetAlpha(ss, 0, 0);
		SDL_SetColorKey(ss, 0, 0);
		SDL_BlitSurface(ss, NULL, ds, NULL);
		SDL_SetAlpha(ss, saflags, salpha);
		SDL_SetColorKey(ss, sckflags, sckey);
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
			case 4:
				SDL_GetRGBA(*(Uint32 *)src, ss->format,
				    &r1, &g1, &b1, &a1);
				break;
			case 3:
				SDL_GetRGBA(*(Uint16 *)src, ss->format,
				    &r1, &g1, &b1, &a1);
				break;
			case 2:
				SDL_GetRGBA(*(Uint16 *)src, ss->format,
				    &r1, &g1, &b1, &a1);
				break;
			case 1:
				SDL_GetRGBA(*src, ss->format,
				    &r1, &g1, &b1, &a1);
				break;
			}
			/* XXX transparency hack for text surfaces. */
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

/* Set the alpha value of all pixels in a surface where a != 0. */
void
view_set_trans(SDL_Surface *su, Uint8 alpha)
{
	int x, y;

	if (SDL_MUSTLOCK(su))
		SDL_LockSurface(su);
	for (y = 0; y < su->h; y++) {
		for (x = 0; x < su->w; x++) {
			Uint8 *dst = (Uint8 *)su->pixels +
			    y*su->pitch +
			    x*su->format->BytesPerPixel;
			Uint32 npixel;
			Uint8 r, g, b, a;

			SDL_GetRGBA(*(Uint32 *)dst, su->format, &r, &g, &b, &a);

			if (r == 15 && g == 15 && b == 15) {
				/* XXX transparency hack for text surfaces. */
				a = 0;
			} else {
				if (a != 0)
					a = alpha;
			}

			npixel = SDL_MapRGBA(su->format, r, g, b, a);
			switch (su->format->BytesPerPixel) {
				_VIEW_PUTPIXEL_8(dst, npixel)
				_VIEW_PUTPIXEL_16(dst, npixel)
				_VIEW_PUTPIXEL_24(dst, npixel)
				_VIEW_PUTPIXEL_32(dst, npixel)
			}
		}
	}
	if (SDL_MUSTLOCK(su))
		SDL_UnlockSurface(su);
}

int
view_set_refresh(int min_delay, int max_delay)
{
	if (min_delay < 0 || min_delay > 300 ||
	    max_delay < 0 || max_delay > 300) {
		error_set(_("The refresh rate is out of range"));
		return (-1);
	}
	dprintf("%d fps (%dms)\n", 1000/max_delay, max_delay);
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
	Uint32 saflags = sourcesu->flags & (SDL_SRCALPHA|SDL_RLEACCEL);
	Uint8 salpha = sourcesu->format->alpha;
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
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
	}

	/* Copy the source surface onto the GL texture surface. */
	SDL_SetAlpha(sourcesu, 0, 0);
	if (SDL_BlitSurface(sourcesu, NULL, texsu, NULL) == -1) {
		fatal("SDL_BlitSurface: %s", SDL_GetError());
	}
	SDL_SetAlpha(sourcesu, saflags, salpha);

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

/*
 * Alpha-blend two pixels in software.
 * Clipping is done; the surface must be locked.
 */
__inline__ void
view_alpha_blend(SDL_Surface *s, Sint16 x, Sint16 y, Uint8 r, Uint8 g,
    Uint8 b, Uint8 a)
{
	Uint32 color, dstcolor = 0;
	Uint8 dr, dg, db;
	Uint8 *dst;

	if (!VIEW_INSIDE_CLIP_RECT(s, x, y))
		return;

	dst = (Uint8 *)s->pixels + y*s->pitch + x*s->format->BytesPerPixel;
	switch (s->format->BytesPerPixel) {
	case 4:
		dstcolor = *(Uint32 *)dst;
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
	case 2:
		dstcolor = *(Uint16 *)dst;
		break;
	case 1:
		dstcolor = *dst;
		break;
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
	unsigned int seq = 0;
	int fd;
	JSAMPROW row[1];
	int x;

	if (prop_copy_string(config, "save-path", path, sizeof(path)) >=
	    sizeof(path))
		goto toobig;
	if (strlcat(path, "/screenshot", sizeof(path)) >= sizeof(path))
		goto toobig;
	if (mkdir(path, 0755) == -1 && errno != EEXIST) {
		text_msg(MSG_ERROR, "mkdir %s: %s", path, strerror(errno));
		return;
	}

	for (;;) {
		char file[MAXPATHLEN];

		snprintf(file, sizeof(file), "%s/%s%u.jpg", path,
		    proginfo->progname, seq++);
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
	jpeg_set_quality(&jcomp, 75, TRUE);
	jpeg_stdio_dest(&jcomp, fp);

	jcopybuf = Malloc(su->w * 3);

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
	return;
toobig:
	text_msg(MSG_ERROR, "Screenshot path is too big");
#else
	text_msg(MSG_ERROR, _("Screenshot feature requires libjpeg"));
#endif
}

/* Queue a video update. */
__inline__ void
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
			dprintf("%d maxdirty\n", view->maxdirty);
		}
		view->dirty[view->ndirty].x = x;
		view->dirty[view->ndirty].y = y;
		view->dirty[view->ndirty].w = w;
		view->dirty[view->ndirty].h = h;
		view->ndirty++;
	}
}

