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

#include "fill.h"

void
tile_init(struct tile *t, const char *name)
{
	strlcpy(t->name, name, sizeof(t->name));
	t->flags = 0;
	t->used = 0;
	t->features = NULL;
	t->nfeatures = 0;
	t->su = NULL;
}

void
tile_scale(struct tile *t, Uint16 w, Uint16 h, Uint8 flags)
{
	Uint32 sflags = SDL_SWSURFACE;

	if (flags & TILE_CKEYING)	sflags |= SDL_SRCCOLORKEY;
	if (flags & TILE_BLENDING)	sflags |= SDL_SRCALPHA;

	if (t->su != NULL) {
		SDL_FreeSurface(t->su);
	}
	t->flags = flags;
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
}

struct tile_feature *
tile_add_feature(struct tile *t, void *ft)
{
	struct tile_feature *tft;

	t->features = Realloc(t->features,
	    sizeof(struct tile_feature)*(t->nfeatures+1));
	tft = &t->features[t->nfeatures++];
	tft->ft = ft;
	tft->x = 0;
	tft->y = 0;
	tft->visible = 1;
	return (tft);
}

void
tile_remove_feature(struct tile *t, void *ftp)
{
	int i;

	for (i = 0; i < t->nfeatures; i++) {
		struct tile_feature *tft = &t->features[i];

		if (tft->ft == ftp)
			break;
	}
	/* TODO */
}

void
tile_save(struct tile *t, struct netbuf *buf)
{
	u_int i;

	write_string(buf, t->name);
	write_uint8(buf, t->flags);
	write_uint16(buf, t->su->w);
	write_uint16(buf, t->su->h);

	dprintf("%s: saving %u features\n", t->name, t->nfeatures);

	write_uint32(buf, (Uint32)t->nfeatures);
	for (i = 0; i < t->nfeatures; i++) {
		struct tile_feature *tft = &t->features[i];

		dprintf("%s: saving %s feature\n", t->name, tft->ft->name);
		write_string(buf, tft->ft->name);
		write_sint32(buf, (Sint32)tft->x);
		write_sint32(buf, (Sint32)tft->y);
		write_uint8(buf, (Uint8)tft->visible);
	}
}

int
tile_load(struct tileset *ts, struct tile *t, struct netbuf *buf)
{
	Uint32 i, nfeatures;
	Uint16 w, h;
	Uint8 flags;
	
	flags = read_uint8(buf);
	w = read_uint16(buf);
	h = read_uint16(buf);
	tile_scale(t, w, h, flags);

	nfeatures = read_uint32(buf);
	dprintf("%s: %ux%u, %u features\n", t->name, w, h, nfeatures);
	for (i = 0; i < nfeatures; i++) {
		char feat_name[FEATURE_NAME_MAX];
		struct feature *ft;
		Sint32 x, y;
		Uint8 visible;

		copy_string(feat_name, buf, sizeof(feat_name));
		x = read_sint32(buf);
		y = read_sint32(buf);
		visible = read_uint8(buf);
		dprintf("%s: feat %s at %d,%d\n", t->name, feat_name, x, y);

		TAILQ_FOREACH(ft, &ts->features, features) {
			if (strcmp(ft->name, feat_name) == 0)
				break;
		}
		if (ft == NULL) {
			error_set("Nonexistent feature: `%s'", feat_name);
			return (-1);
		}
		tile_add_feature(t, ft);
	}
	return (0);
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

static void
insert_fill(int argc, union evarg *argv)
{
	struct tileset *ts = argv[1].p;
	struct tile *t = argv[2].p;
	struct fill *fill;

	fill = Malloc(sizeof(struct fill), M_RG);
	fill_init(fill, "Fill #0", 0);
	TAILQ_INSERT_TAIL(&ts->features, FEATURE(fill), features);
	tile_add_feature(t, fill);

	if (FEATURE(fill)->ops->edit != NULL)
		FEATURE(fill)->ops->edit(fill);
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
		    SDLK_f, KMOD_CTRL, insert_fill, "%p,%p", ts, t);
		    
		ag_menu_action(item, _("Sketch projection"), NULL,
		    SDLK_s, KMOD_CTRL, NULL, "%p,%p", ts, t);
		
		ag_menu_action(item, _("Polygon"), NULL,
		    SDLK_p, KMOD_CTRL, NULL, "%p,%p", ts, t);
		
		ag_menu_action(item, _("Extruded base"), NULL,
		    SDLK_e, KMOD_CTRL, NULL, "%p,%p", ts, t);
		
		ag_menu_action(item, _("Revolved base"), NULL,
		    SDLK_r, KMOD_CTRL, NULL, "%p,%p", ts, t);
	}

	item = ag_menu_add_item(menu, _("Edit"));
	{
	}

	box = box_new(win, BOX_VERT, BOX_WFILL|BOX_HFILL|BOX_FRAME);
	{
		tv = tileview_new(box, ts, t);
	}

	t->used++;
	return (win);
}
