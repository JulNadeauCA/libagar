/*	$Csoft$	*/

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
#include <engine/map.h>
#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/box.h>
#include <engine/widget/tlist.h>
#include <engine/widget/button.h>
#include <engine/widget/textbox.h>
#include <engine/widget/menu.h>

#include "tileset.h"
#include "tileview.h"

struct tile *
tile_insert(struct tileset *ts, const char *name, Uint16 w, Uint16 h,
    Uint8 flags)
{
	struct tile *t;
	Uint32 sflags = SDL_SWSURFACE;

	if (flags & TILE_CKEYING)	sflags |= SDL_SRCCOLORKEY;
	if (flags & TILE_BLENDING)	sflags |= SDL_SRCALPHA;

	t = Malloc(sizeof(struct tile), M_OBJECT);
	strlcpy(t->name, name, sizeof(t->name));
	t->flags = flags;
	t->used = 0;
	t->features = NULL;
	t->nfeatures = 0;
	t->su = SDL_CreateRGBSurface(sflags, w, h, 32,
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
	if (t->su == NULL) {
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
	}
	TAILQ_INSERT_TAIL(&ts->tiles, t, tiles);
	return (t);
}

void
tile_remove(struct tileset *ts, struct tile *t)
{
	TAILQ_REMOVE(&ts->tiles, t, tiles);
	tile_destroy(t);
}

void
tile_destroy(struct tile *t)
{

}

static void
close_tile(int argc, union evarg *argv)
{
	struct window *win = argv[0].p;
	struct tileset *ts = argv[1].p;
	struct tile *t = argv[2].p;
	
	pthread_mutex_lock(&ts->lock);
	t->used--;
	pthread_mutex_unlock(&ts->lock);

	view_detach(win);
}

struct window *
tile_edit(struct tileset *ts, struct tile *t)
{
	struct window *win;
	struct box *box;
	struct tileview *tv;
	struct AGMenu *menu;
	struct AGMenuItem *item;

	win = window_new(WINDOW_DETACH, NULL);
	window_set_caption(win, "%s <%s>", t->name, OBJECT(ts)->name);
	event_new(win, "window-close", close_tile, "%p,%p", ts, t);

	menu = ag_menu_new(win);
	item = ag_menu_add_item(menu, _("Features"));
	{
		ag_menu_action(item, _("Fill"), NULL,
		    SDLK_f, KMOD_CTRL, NULL, "%p,%p", ts, t);
		    
		ag_menu_action(item, _("Sketch projection"), NULL,
		    SDLK_s, KMOD_CTRL, NULL, "%p,%p", ts, t);
		
		ag_menu_action(item, _("Polygon"), NULL,
		    SDLK_p, KMOD_CTRL, NULL, "%p,%p", ts, t);
		
		ag_menu_action(item, _("Extruded base"), NULL,
		    SDLK_e, KMOD_CTRL, NULL, "%p,%p", ts, t);
		
		ag_menu_action(item, _("Revolved base"), NULL,
		    SDLK_r, KMOD_CTRL, NULL, "%p,%p", ts, t);
	}

	box = box_new(win, BOX_VERT, BOX_WFILL|BOX_HFILL|BOX_FRAME);
	{
		tv = tileview_new(box, ts, t);
	}

	t->used++;
	return (win);
}
