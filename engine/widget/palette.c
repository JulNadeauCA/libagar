/*	$Csoft: palette.c,v 1.1 2002/12/21 10:25:57 vedge Exp $	*/

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
	FRAME_COLOR
};

struct palette *
palette_new(struct region *reg, int flags, int w, int h)
{
	struct palette *palette;
	va_list args;
	char *buf;

	palette = emalloc(sizeof(struct palette));
	palette_init(palette, 0, w, h);

	region_attach(reg, palette);

	return (palette);
}

void
palette_init(struct palette *pal, int flags, int rw, int rh)
{
	widget_init(&pal->wid, "palette", &palette_ops, rw, rh);
	widget_map_color(pal, FRAME_COLOR, "frame", 196, 196, 196);

	pal->flags = flags;

	scrollbar_init(&pal->r_sb, -1, -1, 0);
	scrollbar_init(&pal->g_sb, -1, -1, 0);
	scrollbar_init(&pal->b_sb, -1, -1, 0);
	scrollbar_init(&pal->a_sb, -1, -1, 0);

	event_new(pal, "widget-scaled", palette_scaled, NULL);
}

void
palette_scaled(int argc, union evarg *argv)
{
	struct palette *pal = argv[0].p;
	int w = argv[1].i;
	int h = argv[2].i;
	int sbh = h / 4;

	widget_set_position(&pal->r_sb, 0, 0);
	widget_set_position(&pal->g_sb, 0, sbh);
	widget_set_position(&pal->b_sb, 0, sbh*2);
	widget_set_position(&pal->a_sb, 0, sbh*3);

	widget_set_geometry(&pal->r_sb, w-h, sbh);
	widget_set_geometry(&pal->g_sb, w-h, sbh);
	widget_set_geometry(&pal->b_sb, w-h, sbh);
	widget_set_geometry(&pal->a_sb, w-h, sbh);

	pal->rpreview.x = w - sbh;
	pal->rpreview.y = 0;
	pal->rpreview.w = sbh;
	pal->rpreview.h = WIDGET(pal)->h;
}

void
palette_draw(void *p)
{
	struct palette *pal = p;
	Uint32 color;
	int r, g, b;

	scrollbar_draw(&pal->r_sb);
	scrollbar_draw(&pal->g_sb);
	scrollbar_draw(&pal->b_sb);
	scrollbar_draw(&pal->a_sb);

	r = widget_get_int(&pal->r_sb, "value");
	g = widget_get_int(&pal->g_sb, "value");
	b = widget_get_int(&pal->b_sb, "value");
	color = SDL_MapRGB(view->v->format, (Uint8)r, (Uint8)g, (Uint8)b);

	WIDGET_FILL_RECT(pal, &pal->rpreview, color);
	primitives.frame_rect(pal, &pal->rpreview,
	    WIDGET_COLOR(pal, FRAME_COLOR));
}

void
palette_set_color(struct palette *pal, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	widget_set_int(&pal->r_sb, "value", (int)r);
	widget_set_int(&pal->g_sb, "value", (int)g);
	widget_set_int(&pal->b_sb, "value", (int)b);
	widget_set_int(&pal->a_sb, "value", (int)a);
}

void
palette_get_color(struct palette *pal, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a)
{
	*r = (Uint8)widget_get_int(&pal->r_sb, "value");
	*g = (Uint8)widget_get_int(&pal->g_sb, "value");
	*b = (Uint8)widget_get_int(&pal->b_sb, "value");
	*a = (Uint8)widget_get_int(&pal->a_sb, "value");
}

void
palette_destroy(void *p)
{
	struct palette *pal = p;

	widget_destroy(&pal->r_sb);
	widget_destroy(&pal->g_sb);
	widget_destroy(&pal->b_sb);
	widget_destroy(&pal->a_sb);

	widget_destroy(pal);
}
