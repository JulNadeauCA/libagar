/*	$Csoft: view.c,v 1.61 2002/08/24 03:06:15 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "engine.h"
#include "rootmap.h"
#include "map.h"
#include "physics.h"
#include "config.h"

#include "widget/widget.h"
#include "widget/window.h"

static const struct object_ops viewport_ops = {
	view_destroy,
	NULL,		/* load */
	NULL		/* save */
};

/* Read-only as long as the engine is running. */
struct viewport *view;

/* Initialize the graphic engine. */
void
view_init(gfx_engine_t ge)
{
	struct viewport *v;
	int screenflags = SDL_HWSURFACE;
	int bpp = config->view.bpp;
	int w = config->view.w, mw = w/TILEW;
	int h = config->view.h, mh = h/TILEH;

	if (config->flags & CONFIG_FULLSCREEN) {
		screenflags |= SDL_FULLSCREEN;
	}
	if (config->flags & CONFIG_ASYNCBLIT) {
		screenflags |= SDL_ASYNCBLIT;
	}

	v = emalloc(sizeof(struct viewport));
	object_init(&v->obj, "viewport", "main-view", NULL, 0, &viewport_ops);
	v->gfx_engine = ge;
	v->bpp = SDL_VideoModeOK(w, h, bpp, screenflags);
	v->w = config->view.w;
	v->h = config->view.h;
	v->rootmap = NULL;
	v->winop = VIEW_WINOP_NONE;
	TAILQ_INIT(&v->windowsh);
	pthread_mutex_init(&v->lock, NULL);

	switch (v->gfx_engine) {
	case GFX_ENGINE_TILEBASED:
		v->w -= v->w % TILEW;
		v->h -= v->h % TILEH;
		v->margin.w = TILEW;
		v->margin.h = TILEH;
		break;
	case GFX_ENGINE_GUI:
		v->margin.w = 16;
		v->margin.h = 16;
		break;
	}

	if (v->w < 4*TILEW || v->h < 4*TILEH) {
		fatal("resolution is minimum %dx%d\n",
		    4*TILEW, 4*TILEH);
	}

	switch (v->bpp) {
	case 8:
		screenflags |= SDL_HWPALETTE;
		break;
	}

#if 0
	switch (v->gfx_engine) {
	case GFX_ENGINE_GUI:
		screenflags |= SDL_RESIZABLE;
		break;
	default:
	}
#endif

	v->v = SDL_SetVideoMode(v->w, v->h, v->bpp, screenflags);
	if (v->v == NULL) {
		fatal("SDL: %dx%dx%d: %s\n", v->w, v->h, v->bpp,
		    SDL_GetError());
	}

	switch (v->gfx_engine) {
	case GFX_ENGINE_TILEBASED:
		v->rootmap = emalloc(sizeof(struct viewmap));
		v->rootmap->w = mw - 1;
		v->rootmap->h = mh - 1;

		v->rootmap->map = NULL;
		v->rootmap->x = 0;
		v->rootmap->y = 0;
		
		dprintf("GFX_ENGINE_TILEBASED: %dx%d map view\n", mw, mh);
	
		/*
		 * Allocate view masks, precalculate node rectangles, and
		 * preallocate an array able to hold all possible rectangles
		 * in a view, for optimization purposes.
		 */
		v->rootmap->mask = rootmap_alloc_mask(mw, mh);
		v->rootmap->maprects = rootmap_alloc_maprects(mw, mh);
		v->rootmap->rects = rootmap_alloc_rects(mw, mh);
	
		SDL_WM_SetCaption("AGAR (tile-based)", "AGAR");
		break;
	case GFX_ENGINE_GUI:
		SDL_WM_SetCaption("AGAR (GUI)", "AGAR");
		SDL_ShowCursor(SDL_ENABLE);
		break;
	}

	view = v;
}

void
view_destroy(void *p)
{
	struct viewport *v = p;

	if (v->rootmap != NULL) {
		rootmap_free_mask(v);
		rootmap_free_maprects(v);
		free(v->rootmap->rects);
	}
}

/*
 * Attach a window to a view.
 * View must be locked.
 */
void
view_attach(void *child)
{
	struct window *win = child;

	view->focus_win = NULL;

	OBJECT_ASSERT(child, "window");
	event_post(child, "attached", "%p", view);
	TAILQ_INSERT_TAIL(&view->windowsh, win, windows);
}

/*
 * Detach a window from a view.
 * View must be locked.
 */
void
view_detach(void *child)
{
	struct window *win = child;

	OBJECT_ASSERT(child, "window");
	event_post(child, "detached", "%p", view);
	TAILQ_REMOVE(&view->windowsh, win, windows);
}

SDL_Surface *
view_surface(int flags, int w, int h)
{
	SDL_Surface *s;
	Uint32 rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif

	s = SDL_CreateRGBSurface(flags, w, h, 32, rmask, gmask, bmask, amask);
	if (s == NULL) {
		fatal("SDL_CreateRGBSurface: %s\n", SDL_GetError());
	}
	return (s);
}

/*
 * Focus on a window.
 * View and window must be locked.
 */
void
view_focus(struct window *win)
{
	TAILQ_REMOVE(&view->windowsh, win, windows);
	TAILQ_INSERT_TAIL(&view->windowsh, win, windows);
	
	win->redraw++;
}

