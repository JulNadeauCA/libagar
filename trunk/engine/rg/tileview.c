/*	$Csoft: tileview.c,v 1.4 2005/02/05 03:23:32 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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

#include <engine/engine.h>
#include <engine/view.h>

#include <engine/widget/primitive.h>

#include "tileview.h"

const struct widget_ops tileview_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		tileview_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	tileview_draw,
	tileview_scale
};

enum {
	FRAME_COLOR,
	TILE1_COLOR,
	TILE2_COLOR
};

struct tileview *
tileview_new(void *parent, struct tileset *ts, struct tile *t, int flags)
{
	struct tileview *tv;

	tv = Malloc(sizeof(struct tileview), M_OBJECT);
	tileview_init(tv, ts, t, flags);
	object_attach(parent, tv);
	return (tv);
}

static Uint32
zoomin_tick(void *obj, Uint32 ival, void *arg)
{
	struct tileview *tv = obj;

	if (tv->zoom > 800) {
		return (0);
	}
	tileview_set_zoom(tv, tv->zoom<100 ? tv->zoom+5 : tv->zoom+20, 1);
	return (ival);
}

static Uint32
zoomout_tick(void *obj, Uint32 ival, void *arg)
{
	struct tileview *tv = obj;

	if (tv->zoom < 10) {
		return (0);
	}
	tileview_set_zoom(tv, tv->zoom<100 ? tv->zoom-5 : tv->zoom-20, 1);
	return (ival);
}

static void
tileview_keydown(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	int keysym = argv[1].i;

	switch (keysym) {
	case SDLK_LCTRL:
	case SDLK_RCTRL:
		tv->flags |= TILEVIEW_PRESEL;
		break;
	case SDLK_EQUALS:
		timeout_set(&tv->zoom_to, zoomin_tick, NULL, 0);
		timeout_del(tv, &tv->zoom_to);
		timeout_add(tv, &tv->zoom_to, 1);
		zoomin_tick(tv, 0, NULL);
		break;
	case SDLK_MINUS:
		timeout_set(&tv->zoom_to, zoomout_tick, NULL, 0);
		timeout_del(tv, &tv->zoom_to);
		timeout_add(tv, &tv->zoom_to, 1);
		zoomout_tick(tv, 0, NULL);
		break;
	case SDLK_0:
	case SDLK_1:
		tileview_set_zoom(tv, 100, 1);
		break;
	case SDLK_r:
		tv->tile->flags |= TILE_DIRTY;
		break;
	}
}

static void
tileview_keyup(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	int keysym = argv[1].i;

	switch (keysym) {
	case SDLK_LCTRL:
	case SDLK_RCTRL:
		tv->flags &= ~TILEVIEW_PRESEL;
		break;
	case SDLK_EQUALS:
	case SDLK_MINUS:
		timeout_del(tv, &tv->zoom_to);
		break;
	}
}

static void
tileview_buttondown(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	int button = argv[1].i;

	widget_focus(tv);
	switch (button) {
	case SDL_BUTTON_LEFT:
		break;
	case SDL_BUTTON_MIDDLE:
	case SDL_BUTTON_RIGHT:
		tv->scrolling++;
		break;
	case SDL_BUTTON_WHEELUP:
		tileview_set_zoom(tv,
		    tv->zoom<100 ? tv->zoom+5 : tv->zoom+20, 1);
		break;
	case SDL_BUTTON_WHEELDOWN:
		tileview_set_zoom(tv,
		    tv->zoom<100 ? tv->zoom-5 : tv->zoom-20, 1);
		tileview_set_zoom(tv, tv->zoom-10, 1);
		break;
	}
}

static void
tileview_buttonup(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	int button = argv[1].i;

	if (button == SDL_BUTTON_RIGHT ||
	    button == SDL_BUTTON_MIDDLE) {
		tv->scrolling = 0;
	}
}

static __inline__ void
clamp_offsets(struct tileview *tv)
{
	int lim;

	if (tv->xoffs >
	   (lim = (WIDGET(tv)->w - TILEVIEW_MIN_W))) {
		tv->xoffs = lim;
	} else if (tv->xoffs <
	   (lim = (-tv->scaled->w + TILEVIEW_MIN_W))) {
		tv->xoffs = lim;
	}

	if (tv->yoffs >
	    (lim = (WIDGET(tv)->h - TILEVIEW_MIN_H))) {
		tv->yoffs = lim;
	} else if (tv->yoffs <
	    (lim = (-tv->scaled->h + TILEVIEW_MIN_H))) {
		tv->yoffs = lim;
	}
}

static void
tileview_mousemotion(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	int x = argv[1].i;
	int y = argv[2].i;
	int xrel = argv[3].i;
	int yrel = argv[4].i;
	int state = argv[5].i;

	if (tv->scrolling) {
		tv->xoffs += xrel;
		tv->yoffs += yrel;
		clamp_offsets(tv);
	}
	if (tv->flags |= TILEVIEW_PRESEL) {
		tv->xms = x - tv->xoffs;
		tv->yms = y - tv->yoffs;
		tv->xsub = tv->xms%tv->pxsz;
		tv->ysub = tv->yms%tv->pxsz;
		tv->xms -= tv->xsub;
		tv->yms -= tv->ysub;
	}
}

static Uint32
autoredraw(void *obj, Uint32 ival, void *arg)
{
	struct tileview *tv = obj;

	tile_generate(tv->tile);
	view_scale_surface(tv->tile->su, tv->scaled->w, tv->scaled->h,
	    &tv->scaled);
	tv->tile->flags &= ~TILE_DIRTY;
	return (ival);
}

void
tileview_init(struct tileview *tv, struct tileset *ts, struct tile *tile,
    int flags)
{
	widget_init(tv, "tileview", &tileview_ops, WIDGET_WFILL|WIDGET_HFILL|
	                                           WIDGET_FOCUSABLE|
						   WIDGET_CLIPPING);
	widget_map_color(tv, FRAME_COLOR, "frame", 40, 40, 60, 255);
	widget_map_color(tv, TILE1_COLOR, "tile1", 140, 140, 140, 255);
	widget_map_color(tv, TILE2_COLOR, "tile2", 80, 80, 80, 255);

	tv->ts = ts;
	tv->tile = tile;
	tv->scaled = NULL;
	tv->xoffs = 0;
	tv->yoffs = 0;
	tv->scrolling = 0;
	tv->flags = flags;
	tv->state = TILEVIEW_TILE_EDIT;
	tv->edit_mode = 0;
	widget_map_surface(tv, NULL);
	tileview_set_zoom(tv, 100, 0);

	timeout_set(&tv->redraw_to, autoredraw, NULL, 0);
	
	event_new(tv, "window-keydown", tileview_keydown, NULL);
	event_new(tv, "window-keyup", tileview_keyup, NULL);
	event_new(tv, "window-mousebuttonup", tileview_buttonup, NULL);
	event_new(tv, "window-mousebuttondown", tileview_buttondown, NULL);
	event_new(tv, "window-mousemotion", tileview_mousemotion, NULL);
}

void
tileview_set_autoredraw(struct tileview *tv, int ena, int rate)
{
	if (ena) {
		timeout_add(tv, &tv->redraw_to, rate);
		dprintf("enabled autoredraw\n");
	} else {
		timeout_del(tv, &tv->redraw_to);
		dprintf("disabled autoredraw\n");
	}
}

void
tileview_set_zoom(struct tileview *tv, int z2, int adj_offs)
{
	struct tile *t = tv->tile;
	int z1 = tv->zoom;

	tv->zoom = z2;
	tv->pxsz = z2/100;
	if (tv->pxsz < 1)
		tv->pxsz = 1;

	tv->scaled = SDL_CreateRGBSurface(SDL_SWSURFACE,
	    t->su->w*z2/100,
	    t->su->h*z2/100,
	    t->su->format->BitsPerPixel,
	    t->su->format->Rmask, t->su->format->Gmask,
	    t->su->format->Bmask, t->su->format->Amask);
	tv->scaled->format->alpha = t->su->format->alpha;
	tv->scaled->format->colorkey = t->su->format->colorkey;

	if (tv->scaled == NULL) {
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
	}
	widget_replace_surface(tv, 0, tv->scaled);

	if (adj_offs) {
		tv->xoffs += (t->su->w*z1 - t->su->w*z2)/100/2;
		tv->yoffs += (t->su->h*z1 - t->su->h*z2)/100/2;
	}

	t->flags |= TILE_DIRTY;
}

void
tileview_scale(void *p, int rw, int rh)
{
	struct tileview *tv = p;
	int lim;

	if (rw == -1 && rh == -1) {
		WIDGET(tv)->w = tv->tile->su->w + TILEVIEW_MIN_W;
		WIDGET(tv)->h = tv->tile->su->h + TILEVIEW_MIN_H;
	} else {
		if (tv->xoffs >
		   (lim = (WIDGET(tv)->w - TILEVIEW_MIN_W))) {
			tv->xoffs = lim;
		}
		if (tv->yoffs >
		   (lim	= (WIDGET(tv)->h - TILEVIEW_MIN_H))) {
			tv->yoffs = lim;
		}
	}
}

void
tileview_draw(void *p)
{
	struct tileview *tv = p;
	struct tile *t = tv->tile;
	SDL_Rect rsrc, rdst;
	int dxoffs, dyoffs;
	int drawbg = 2;

	if (t->flags & TILE_DIRTY) {
		t->flags &= ~TILE_DIRTY;
		tile_generate(t);
		view_scale_surface(t->su, tv->scaled->w, tv->scaled->h,
		    &tv->scaled);
	}

	rsrc.x = 0;
	rsrc.y = 0;
	rsrc.w = tv->scaled->w;
	rsrc.h = tv->scaled->h;
	rdst.x = tv->xoffs;
	rdst.y = tv->yoffs;

	if (tv->xoffs > 0 &&
	    tv->xoffs + tv->scaled->w > WIDGET(tv)->w) {
		rsrc.w = WIDGET(tv)->w - tv->xoffs;
	} else if (tv->xoffs < 0 &&
	          -tv->xoffs < tv->scaled->w) {
		rdst.x = 0;
		rsrc.x = -tv->xoffs;
		rsrc.w = tv->scaled->w - (-tv->xoffs);
		if (rsrc.w > WIDGET(tv)->w) {
			rsrc.w = WIDGET(tv)->w;
			drawbg--;
		}
	}

	if (tv->yoffs > 0 &&
	    tv->yoffs + tv->scaled->h > WIDGET(tv)->h) {
		rsrc.h = WIDGET(tv)->h - tv->yoffs;
	} else if (tv->yoffs < 0 &&
	          -tv->yoffs < tv->scaled->h) {
		rdst.y = 0;
		rsrc.y = -tv->yoffs;
		rsrc.h = tv->scaled->h - (-tv->yoffs);
		if (rsrc.h > WIDGET(tv)->h) {
			rsrc.h = WIDGET(tv)->h;
			drawbg--;
		}
	}
	
	if (drawbg) {
		SDL_Rect rtiling;
	
		rtiling.x = 0;
		rtiling.y = 0;
		rtiling.w = WIDGET(tv)->w;
		rtiling.h = WIDGET(tv)->h;
		primitives.tiling(tv, rtiling, 9, 0, TILE1_COLOR, TILE2_COLOR);
	}

	widget_blit_from(tv, tv, 0, &rsrc, rdst.x, rdst.y);

	if (tv->flags & TILEVIEW_PRESEL) {
		int x1 = WIDGET(tv)->cx + tv->xoffs + tv->xms;
		int y1 = WIDGET(tv)->cy + tv->yoffs + tv->yms;
		int dx, dy;

		for (dy = 0; dy < tv->pxsz; dy++) {
			for (dx = 0; dx < tv->pxsz; dx++) {
				if (x1+dx < WIDGET(tv)->cx ||
				    y1+dy < WIDGET(tv)->cy ||
				    x1+dx > WIDGET(tv)->cx+WIDGET(tv)->w ||
				    y1+dy > WIDGET(tv)->cy+WIDGET(tv)->h)
					break;

				view_alpha_blend(view->v,
				    x1+dx, y1+dy,
				    255, 255, 255, 128);
			}
		}
	}
}

void
tileview_destroy(void *p)
{
	struct tileview *tv = p;

	widget_destroy(tv);
}

