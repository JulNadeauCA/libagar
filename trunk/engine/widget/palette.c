/*	$Csoft: palette.c,v 1.5 2002/12/31 02:15:46 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
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

#include <engine/engine.h>

#include <engine/view.h>

#include "widget.h"
#include "window.h"
#include "palette.h"
#include "primitive.h"

static const struct widget_ops palette_ops = {
	{
		palette_destroy,
		NULL,		/* load */
		NULL		/* save */
	},
	palette_draw,
	NULL			/* update */
};

enum {
	BG_COLOR
};

static void	palette_mousebuttondown(int, union evarg *);
static void	palette_mousebuttonup(int, union evarg *);
static void	palette_mousemotion(int, union evarg *);
static void	palette_attached(int, union evarg *);
static void	palette_changed(int, union evarg *);

struct palette *
palette_new(struct region *reg, int w, int h, int nbars)
{
	struct palette *pal;

	pal = emalloc(sizeof(struct palette));
	palette_init(pal, w, h, nbars);
	region_attach(reg, pal);

	return (pal);
}

void
palette_init(struct palette *pal, int rw, int rh, int nbars)
{
	int i;

	widget_init(&pal->wid, "palette", &palette_ops, rw, rh);
	widget_map_color(pal, BG_COLOR, "frame", 196, 196, 196);

	widget_bind(pal, "color", WIDGET_UINT32, NULL, &pal->def.color);
	pal->def.color = 0;
	
	pal->cur_sb = NULL;
	pal->nbars = nbars;
	pal->bars = emalloc(sizeof(struct scrollbar *) * pal->nbars);

	for (i = 0; i < pal->nbars; i++) {
		pal->bars[i] = emalloc(sizeof(struct scrollbar));
		scrollbar_init(pal->bars[i], -1, -1, SCROLLBAR_HORIZ);
		WIDGET(pal->bars[i])->flags |= WIDGET_NO_FOCUS;
		widget_set_int(pal->bars[i], "max", 255);
		event_new(pal->bars[i], "scrollbar-changed",
		    palette_changed, "%p, %i", pal, i);
	}

	event_new(pal, "attached", palette_attached, NULL);
	event_new(pal, "widget-scaled", palette_scaled, NULL);
	event_new(pal, "window-mousebuttondown", palette_mousebuttondown, NULL);
	event_new(pal, "window-mousebuttonup", palette_mousebuttonup, NULL);
	event_new(pal, "window-mousemotion", palette_mousemotion, NULL);
}

static void
palette_changed(int argc, union evarg *argv)
{
	struct palette *pal = argv[1].p;
	int nbar = argv[2].i;
	struct widget_binding *colorb;
	Uint32 *color;
	Uint8 r, g, b;

	colorb = widget_binding_get_locked(pal, "color", &color);
	if (colorb == NULL) {
		fatal("%s\n", error_get());
	}
	
	SDL_GetRGB(*color, view->v->format, &r, &g, &b);
	switch (nbar) {
	case PALETTE_RED:
		r = widget_get_int(pal->bars[nbar], "value");
		break;
	case PALETTE_GREEN:
		g = widget_get_int(pal->bars[nbar], "value");
		break;
	case PALETTE_BLUE:
		b = widget_get_int(pal->bars[nbar], "value");
		break;
	}
	*color = SDL_MapRGB(view->v->format, r, g, b);
	widget_binding_unlock(colorb);
}

static void
palette_attached(int argc, union evarg *argv)
{
	struct palette *pal = argv[0].p;
	struct region *reg = argv[1].p;
	int i;

	for (i = 0; i < pal->nbars; i++) {
		widget_set_parent(pal->bars[i], reg);
	}
}

static struct scrollbar *
palette_which_scrollbar(struct palette *pal, int y)
{
	int sbh = WIDGET(pal)->h/pal->nbars;

	if (y < sbh) {
		return (pal->bars[0]);
	} else if (y > sbh && y < sbh*2) {
		return (pal->bars[1]);
	} else if (y > sbh*2 && y < sbh*3) {
		return (pal->bars[2]);
	} else if (y > sbh*3 && y < sbh*4) {
		return (pal->bars[3]);
	}
	return (NULL);
}

static void
palette_mousebuttondown(int argc, union evarg *argv)
{
	struct palette *pal = argv[0].p;
	int button = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;
	struct scrollbar *sb;

	sb = palette_which_scrollbar(pal, y);
	if (sb != NULL) {
		WIDGET_FOCUS(pal);
		pal->cur_sb = sb;
		event_forward(sb, "window-mousebuttondown", argc, argv);
	}
}

static void
palette_mousebuttonup(int argc, union evarg *argv)
{
	struct palette *pal = argv[0].p;

	if (pal->cur_sb != NULL) {
		event_forward(pal->cur_sb, "window-mousebuttonup", argc, argv);
		pal->cur_sb = NULL;
	}
}

static void
palette_mousemotion(int argc, union evarg *argv)
{
	struct palette *pal = argv[0].p;

	if (pal->cur_sb != NULL) {
		event_forward(pal->cur_sb, "window-mousemotion", argc, argv);
	}
}

void
palette_scaled(int argc, union evarg *argv)
{
	struct palette *pal = argv[0].p;
	int w = argv[1].i;
	int h = argv[2].i/2;
	int i, sbh = h / pal->nbars;

	for (i = 0; i < pal->nbars; i++) {
		widget_set_position(pal->bars[i], WIDGET(pal)->x,
		    WIDGET(pal)->y + i*sbh);
		widget_set_geometry(pal->bars[i], w - h - 2, sbh);
		event_forward(pal->bars[i], "widget-scaled", argc, argv);
	}

	pal->rpreview.x = w - h;
	pal->rpreview.y = 0;
	pal->rpreview.w = h;
	pal->rpreview.h = WIDGET(pal)->h;
}

void
palette_draw(void *p)
{
	struct palette *pal = p;
	int i;
	Uint32 color;
	Uint8 r, g, b;

	color = widget_get_uint32(pal, "color");
	primitives.rect_filled(pal, &pal->rpreview, color);
	primitives.frame_rect(pal, &pal->rpreview, WIDGET_COLOR(pal, BG_COLOR));
	
	SDL_GetRGB(color, view->v->format, &r, &g, &b);
	widget_set_int(pal->bars[0], "value", (int)r);
	widget_set_int(pal->bars[1], "value", (int)g);
	widget_set_int(pal->bars[2], "value", (int)b);

	scrollbar_draw(pal->bars[0]);
	scrollbar_draw(pal->bars[1]);
	scrollbar_draw(pal->bars[2]);
}

void
palette_destroy(void *p)
{
	struct palette *pal = p;
	int i;

	for (i = 0; i < pal->nbars; i++) {
		widget_destroy(pal->bars[i]);
	}
	free(pal->bars);

	widget_destroy(pal);
}
