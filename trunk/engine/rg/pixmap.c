/*	$Csoft: pixmap.c,v 1.1 2005/02/11 04:51:04 vedge Exp $	*/

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
#include <engine/widget/mspinbutton.h>
#include <engine/widget/spinbutton.h>
#include <engine/widget/checkbox.h>

#include "tileset.h"
#include "tileview.h"

void
pixmap_init(struct pixmap *px, struct tileset *ts, int flags)
{
	unsigned int pixno = 0;
	struct pixmap *opx;

tryname:
	snprintf(px->name, sizeof(px->name), "pixmap #%d", pixno);
	TAILQ_FOREACH(opx, &ts->pixmaps, pixmaps) {
		if (strcmp(opx->name, px->name) == 0)
			break;
	}
	if (opx != NULL) {
		pixno++;
		goto tryname;
	}
	px->ts = ts;
	px->flags = flags;
	px->nrefs = 0;
	px->su = NULL;
}

void
pixmap_destroy(struct pixmap *px)
{
#ifdef DEBUG
	if (px->nrefs > 0)
		dprintf("%s is referenced\n", px->name);
#endif
}

void
pixmap_scale(struct pixmap *px, int w, int h)
{
	struct tileset *ts = px->ts;

	if (px->su != NULL) {
		SDL_FreeSurface(px->su);
	}
	px->su = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA,
	    w, h, ts->fmt->BitsPerPixel,
	    ts->fmt->Rmask,
	    ts->fmt->Gmask,
	    ts->fmt->Bmask,
	    ts->fmt->Amask);
	if (px->su == NULL)
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
}

static void
dimensions_changed(int argc, union evarg *argv)
{
	struct tileview *tv = argv[0].p;
	struct tileview_ctrl *ctrl = argv[1].p;
	int x = tileview_int(ctrl, 0);
	int y = tileview_int(ctrl, 1);
	u_int w = tileview_uint(ctrl, 2);
	u_int h = tileview_uint(ctrl, 3);

	dprintf("changed: %d,%d,%u,%u\n", x, y, w, h);
}

struct window *
pixmap_edit(struct tileview *tv, struct tile_element *tel)
{
	struct pixmap *px = tel->tel_pixmap.px;
	struct window *win;
	struct mspinbutton *msb;
	struct spinbutton *sb;
	struct checkbox *cb;

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
	
	return (win);
}

