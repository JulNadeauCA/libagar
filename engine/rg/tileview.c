/*	$Csoft: tileview.c,v 1.1 2005/01/13 02:30:23 vedge Exp $	*/

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
	tileview_resize(tv, tv->zoom+4);
	return (ival);
}

static Uint32
zoomout_tick(void *obj, Uint32 ival, void *arg)
{
	struct tileview *tv = obj;

	if (tv->zoom < 10) {
		return (0);
	}
	tileview_resize(tv, tv->zoom-4);
	return (ival);
}

static void
tileview_keydown(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	int keysym = argv[1].i;

	switch (keysym) {
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
		tileview_resize(tv, 100);
		break;
	}
}

static void
tileview_keyup(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	int keysym = argv[1].i;

	switch (keysym) {
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
	case SDL_BUTTON_RIGHT:
		tv->scrolling++;
		break;
	case SDL_BUTTON_WHEELUP:
		if (tv->zoom < 400) {
			tileview_resize(tv, tv->zoom+10);
		}
		break;
	case SDL_BUTTON_WHEELDOWN:
		if (tv->zoom > 10) {
			tileview_resize(tv, tv->zoom-10);
		}
		break;
	}
}

static void
tileview_buttonup(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	int button = argv[1].i;

	if (button == SDL_BUTTON_RIGHT) {
		tv->scrolling = 0;
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
	}
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
	tileview_resize(tv, 100);
	
	event_new(tv, "window-keydown", tileview_keydown, NULL);
	event_new(tv, "window-keyup", tileview_keyup, NULL);
	event_new(tv, "window-mousebuttonup", tileview_buttonup, NULL);
	event_new(tv, "window-mousebuttondown", tileview_buttondown, NULL);
	event_new(tv, "window-mousemotion", tileview_mousemotion, NULL);
}

void
tileview_resize(struct tileview *tv, int zoom)
{
	struct tile *t = tv->tile;
	
	tv->zoom = zoom;

	tv->scaled = SDL_CreateRGBSurface(SDL_SWSURFACE,
	    t->su->w*zoom/100,
	    t->su->h*zoom/100,
	    t->su->format->BitsPerPixel,
	    t->su->format->Rmask, t->su->format->Gmask,
	    t->su->format->Bmask, t->su->format->Amask);
	tv->scaled->format->alpha = t->su->format->alpha;
	tv->scaled->format->colorkey = t->su->format->colorkey;

	if (tv->scaled == NULL) {
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
	}
	widget_replace_surface(tv, 0, tv->scaled);
}

void
tileview_scale(void *p, int rw, int rh)
{
	struct tileview *tv = p;

	if (rw == -1 && rh == -1) {
		WIDGET(tv)->w = tv->tile->su->w + 32;
		WIDGET(tv)->h = tv->tile->su->h + 32;
	}
}

void
tileview_draw(void *p)
{
	struct tileview *tv = p;
	struct tile *t = tv->tile;
	SDL_Rect rtiling;

	if (tv->flags & TILEVIEW_AUTOREGEN)
		tile_generate(tv->tile);

	rtiling.x = 0;
	rtiling.y = 0;
	rtiling.w = WIDGET(tv)->w;
	rtiling.h = WIDGET(tv)->h;
	primitives.tiling(tv, rtiling, 9, 0, TILE1_COLOR, TILE2_COLOR);

	view_scale_surface(t->su, tv->scaled->w, tv->scaled->h, &tv->scaled);
	widget_blit_surface(tv, 0, tv->xoffs, tv->yoffs);
}

void
tileview_destroy(void *p)
{
	struct tileview *tv = p;

	widget_destroy(tv);
}

