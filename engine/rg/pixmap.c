/*	$Csoft: pixmap.c,v 1.4 2005/02/16 03:30:31 vedge Exp $	*/

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
#include <engine/widget/spinbutton.h>
#include <engine/widget/fspinbutton.h>
#include <engine/widget/mspinbutton.h>
#include <engine/widget/checkbox.h>
#include <engine/widget/hsvpal.h>

#include "tileset.h"
#include "tileview.h"

void
pixmap_init(struct pixmap *px, struct tileset *ts, int flags)
{
	px->name[0] = '\0';
	px->ts = ts;
	px->flags = flags;
	px->nrefs = 0;
	px->su = NULL;
	px->bg = NULL;
	px->h = 0.0;
	px->s = 1.0;
	px->v = 1.0;
	px->a = 1.0;
}

void
pixmap_destroy(struct pixmap *px)
{
#ifdef DEBUG
	if (px->nrefs > 0)
		dprintf("%s is referenced\n", px->name);
#endif
	if (px->su != NULL)
		SDL_FreeSurface(px->su);
	if (px->bg != NULL)
		SDL_FreeSurface(px->bg);
}

/* Resize a pixmap and copy the previous surface at the given offset. */
void
pixmap_scale(struct pixmap *px, int w, int h, int xoffs, int yoffs)
{
	struct tileset *ts = px->ts;
	SDL_Surface *nsu;

	/* Create the new surface. */
	nsu = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA,
	    w, h, ts->fmt->BitsPerPixel,
	    ts->fmt->Rmask,
	    ts->fmt->Gmask,
	    ts->fmt->Bmask,
	    ts->fmt->Amask);
	if (nsu == NULL)
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());

	/* Copy the old surface over. */
	if (px->su != NULL) {
		SDL_Rect rd;

		SDL_SetAlpha(nsu, px->su->flags &
		    (SDL_SRCALPHA|SDL_RLEACCEL),
		    px->su->format->alpha);
		SDL_SetColorKey(nsu, px->su->flags &
		    (SDL_SRCCOLORKEY|SDL_RLEACCEL),
		    px->su->format->colorkey);
		SDL_SetAlpha(px->su, 0, 0);
		SDL_SetColorKey(px->su, 0, 0);

		rd.x = xoffs;
		rd.y = yoffs;
		SDL_BlitSurface(px->su, NULL, nsu, &rd);
		SDL_FreeSurface(px->su);
	}
	px->su = nsu;

	/* Resize the background save surface. */
	if (px->bg != NULL) {
		SDL_FreeSurface(px->bg);
	}
	px->bg = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h,
	    ts->fmt->BitsPerPixel,
	    ts->fmt->Rmask,
	    ts->fmt->Gmask,
	    ts->fmt->Bmask,
	    0);
}

struct window *
pixmap_edit(struct tileview *tv, struct tile_element *tel)
{
	struct pixmap *px = tel->tel_pixmap.px;
	struct window *win;
	struct mspinbutton *msb;
	struct spinbutton *sb;
	struct checkbox *cb;
	struct box *bo;

	win = window_new(0, NULL);
	window_set_caption(win, _("Pixmap %s"), px->name);
	window_set_position(win, WINDOW_MIDDLE_LEFT, 0);

	cb = checkbox_new(win, _("Visible"));
	widget_bind(cb, "state", WIDGET_INT, &tel->visible);

	msb = mspinbutton_new(win, ",", _("Coordinates: "));
	widget_bind(msb, "xvalue", WIDGET_INT, &tel->tel_pixmap.x);
	widget_bind(msb, "yvalue", WIDGET_INT, &tel->tel_pixmap.y);
	mspinbutton_set_range(msb, 0, TILE_SIZE_MAX-1);

	sb = spinbutton_new(win, _("Transparency: "));
	widget_bind(sb, "value", WIDGET_INT, &tel->tel_pixmap.alpha);
	spinbutton_set_range(sb, 0, 255);

	bo = box_new(win, BOX_VERT, BOX_WFILL|BOX_HFILL);
	{
		struct hsvpal *pal;
		struct fspinbutton *fsb;

		pal = hsvpal_new(bo, px->su->format);
		WIDGET(pal)->flags |= WIDGET_WFILL|WIDGET_HFILL;
		widget_bind(pal, "hue", WIDGET_FLOAT, &px->h);
		widget_bind(pal, "saturation", WIDGET_FLOAT, &px->s);
		widget_bind(pal, "value", WIDGET_FLOAT, &px->v);

		fsb = fspinbutton_new(win, NULL, _("Hue: "));
		widget_bind(fsb, "value", WIDGET_FLOAT, &px->h);
		fspinbutton_set_range(fsb, 0.0, 359.0);
		fspinbutton_set_increment(fsb, 1);
		
		fsb = fspinbutton_new(win, NULL, _("Saturation: "));
		widget_bind(fsb, "value", WIDGET_FLOAT, &px->s);
		fspinbutton_set_range(fsb, 0.0, 1.0);
		fspinbutton_set_increment(fsb, 0.01);
		
		fsb = fspinbutton_new(win, NULL, _("Value: "));
		widget_bind(fsb, "value", WIDGET_FLOAT, &px->v);
		fspinbutton_set_range(fsb, 0.0, 1.0);
		fspinbutton_set_increment(fsb, 0.01);
		
		fsb = fspinbutton_new(win, NULL, _("Source alpha: "));
		widget_bind(fsb, "value", WIDGET_FLOAT, &px->a);
		fspinbutton_set_range(fsb, 0.0, 1.0);
		fspinbutton_set_increment(fsb, 0.01);
	}
	return (win);
}

void
pixmap_mousebuttondown(struct tileview *tv, struct tile_element *tel,
    int x, int y, int button)
{
	struct pixmap *px = tel->tel_pixmap.px;
	Uint8 r, g, b;
	Uint32 pc;

	prim_hsv2rgb(px->h/360.0, px->s, px->v, &r, &g, &b);

	pc = SDL_MapRGB(px->su->format, r, g, b);
	prim_put_pixel(px->su, x, y, pc);
	tileview_scaled_pixel(tv, tel->tel_pixmap.x+x, tel->tel_pixmap.y+y, pc);
}

void
pixmap_mousebuttonup(struct tileview *tv, struct tile_element *tel, int x,
    int y, int button)
{
	dprintf("%d,%d,%d\n", x, y, button);
}

void
pixmap_mousemotion(struct tileview *tv, struct tile_element *tel, int x, int y,
    int xrel, int yrel, int state)
{
	struct pixmap *px = tel->tel_pixmap.px;

	if (state & SDL_BUTTON_LEFT) {
		Uint8 r, g, b;
		Uint32 pc;

		dprintf("%d,%d %d,%d %d\n", x, y, xrel, yrel, state); 
		prim_hsv2rgb(px->h/360.0, px->s, px->v, &r, &g, &b);

		pc = SDL_MapRGB(px->su->format, r, g, b);
		prim_put_pixel(px->su, x, y, pc);
		tileview_scaled_pixel(tv,
		    tel->tel_pixmap.x + x,
		    tel->tel_pixmap.y + y,
		    pc);
	}
}
