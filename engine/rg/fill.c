/*	$Csoft: fill.c,v 1.2 2005/01/26 02:43:03 vedge Exp $	*/

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

#include <engine/vg/vg.h>

#include <engine/widget/window.h>
#include <engine/widget/label.h>
#include <engine/widget/palette.h>
#include <engine/widget/radio.h>
#include <engine/widget/box.h>

#include "tileset.h"
#include "tileview.h"
#include "fill.h"

const struct version fill_ver = {
	"agar rg fill feature",
	0, 0
};

const struct feature_ops fill_ops = {
	"fill",
	sizeof(struct fill),
	N_("Fill tile with solid color/pattern."),
	FEATURE_AUTOREDRAW,
	fill_init,
	fill_load,
	fill_save,
	NULL,		/* destroy */
	fill_apply,
	fill_edit
};

void
fill_init(void *p, struct tileset *ts, int flags)
{
	struct fill *f = p;

	feature_init(f, ts, flags, &fill_ops);
	f->type = FILL_SOLID;
	f->f_gradient.c1 = SDL_MapRGB(ts->fmt, 0, 0, 0);
	f->f_gradient.c2 = SDL_MapRGB(ts->fmt, 0, 0, 0);
}

int
fill_load(void *p, struct netbuf *buf)
{
	struct fill *f = p;
	struct tileset *ts = FEATURE(f)->ts;

	if (version_read(buf, &fill_ver, NULL) == -1)
		return (-1);

	f->type = (enum fill_type)read_uint8(buf);
	switch (f->type) {
	case FILL_SOLID:
		f->f_solid.c = read_color(buf, ts->fmt);
		break;
	case FILL_HGRADIENT:
	case FILL_VGRADIENT:
	case FILL_CGRADIENT:
		f->f_gradient.c1 = read_color(buf, ts->fmt);
		f->f_gradient.c2 = read_color(buf, ts->fmt);
		break;
	case FILL_PATTERN:
		f->f_pattern.texid = (int)read_uint32(buf);
		f->f_pattern.tex_xoffs = (int)read_uint32(buf);
		f->f_pattern.tex_yoffs = (int)read_uint32(buf);
		break;
	}
	return (0);
}

void
fill_save(void *p, struct netbuf *buf)
{
	struct fill *f = p;
	struct tileset *ts = FEATURE(f)->ts;

	version_write(buf, &fill_ver);

	write_uint8(buf, (Uint8)f->type);
	switch (f->type) {
	case FILL_SOLID:
		write_color(buf, ts->fmt, f->f_solid.c);
		break;
	case FILL_HGRADIENT:
	case FILL_VGRADIENT:
	case FILL_CGRADIENT:
		write_color(buf, ts->fmt, f->f_gradient.c1);
		write_color(buf, ts->fmt, f->f_gradient.c2);
		break;
	case FILL_PATTERN:
		write_uint32(buf, (Uint32)f->f_pattern.texid);
		write_uint32(buf, (Uint32)f->f_pattern.tex_xoffs);
		write_uint32(buf, (Uint32)f->f_pattern.tex_yoffs);
		break;
	}
}

struct window *
fill_edit(void *p, struct tileview *tv)
{
	struct fill *f = p;
	struct window *win;
	struct palette *pal;
	struct radio *rad;
	static const char *modes[] = {
		N_("Solid"),
		N_("Horizontal gradient"),
		N_("Vertical gradient"),
		N_("Circular gradient"),
		N_("Pixmap pattern"),
		NULL
	};
	struct box *box;

	win = window_new(0, NULL);

	rad = radio_new(win, modes);
	widget_bind(rad, "value", WIDGET_INT, &f->type);

	box = box_new(win, BOX_VERT, BOX_WFILL|BOX_FRAME);
	{
		label_new(box, LABEL_STATIC, _("Solid color:"));
		pal = palette_new(box, PALETTE_RGBA, tv->tile->su->format);
		widget_bind(pal, "color", WIDGET_UINT32, &f->f_solid.c);
	}
	
	box = box_new(win, BOX_VERT, BOX_WFILL|BOX_FRAME);
	{
		label_new(box, LABEL_STATIC, _("Gradient colors:"));
		pal = palette_new(box, PALETTE_RGBA, tv->tile->su->format);
		widget_bind(pal, "color", WIDGET_UINT32, &f->f_gradient.c1);

		pal = palette_new(box, PALETTE_RGBA, tv->tile->su->format);
		widget_bind(pal, "color", WIDGET_UINT32, &f->f_gradient.c2);
	}

	return (win);
}

static void
circle2(struct tile *t, int wx, int wy, int radius, Uint32 color)
{
	int v = 2*radius - 1;
	int e = 0, u = 1;
	int x = 0, y = radius;

	while (x < y) {
		tile_put_pixel(t, wx+x, wy+y, color);
		tile_put_pixel(t, wx+x+1, wy+y, color);
		tile_put_pixel(t, wx+x, wy-y, color);
		tile_put_pixel(t, wx+x+1, wy-y, color);
		tile_put_pixel(t, wx-x, wy+y, color);
		tile_put_pixel(t, wx-x-1, wy+y, color);
		tile_put_pixel(t, wx-x, wy-y, color);
		tile_put_pixel(t, wx-x-1, wy-y, color);

		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;
		
		tile_put_pixel(t, wx+y, wy+x, color);
		tile_put_pixel(t, wx+y+1, wy+x, color);
		tile_put_pixel(t, wx+y, wy-x, color);
		tile_put_pixel(t, wx+y+1, wy-x, color);
		tile_put_pixel(t, wx-y, wy+x, color);
		tile_put_pixel(t, wx-y-1, wy+x, color);
		tile_put_pixel(t, wx-y, wy-x, color);
		tile_put_pixel(t, wx-y-1, wy-x, color);
	}
	tile_put_pixel(t, wx-radius, wy, color);
	tile_put_pixel(t, wx+radius, wy, color);
}

void
fill_apply(void *p, struct tile *t, int x, int y)
{
	struct fill *fi = p;
	SDL_Surface *su = t->su;
	Uint8 r1, g1, b1, a1;
	Uint8 r2, g2, b2, a2;
	
	SDL_GetRGBA(fi->f_gradient.c1, su->format, &r1, &g1, &b1, &a1);
	SDL_GetRGBA(fi->f_gradient.c2, su->format, &r2, &g2, &b2, &a2);

	switch (fi->type) {
	case FILL_SOLID:
		SDL_FillRect(su, NULL, fi->f_solid.c);
		break;
	case FILL_HGRADIENT:
		{
			int x, y;
		
			SDL_LockSurface(su);
			for (y = 0; y < su->h; y++) {
				Uint32 c;
				Uint8 a = (su->h-y)*255/su->h;

				c = SDL_MapRGBA(su->format,
				    (((r1 - r2) * a) >> 8) + r2,
				    (((g1 - g2) * a) >> 8) + g2,
				    (((b1 - b2) * a) >> 8) + b2,
				    (((a1 - a2) * a) >> 8) + a2);

				for (x = 0; x < su->w; x++) {
					tile_blend_rgb(t, x, y,
					    TILE_BLEND_SRCALPHA,
					    (((r1 - r2) * a) >> 8) + r2,
					    (((g1 - g2) * a) >> 8) + g2,
					    (((b1 - b2) * a) >> 8) + b2,
					    (((a1 - a2) * a) >> 8) + a2);
				}
			}
			SDL_UnlockSurface(su);
		}
		break;
	case FILL_VGRADIENT:
		{
			int x, y;
		
			SDL_LockSurface(su);
			for (y = 0; y < su->h; y++) {
				for (x = 0; x < su->w; x++) {
					Uint8 a = (su->h-x)*255/su->h;
				
					tile_blend_rgb(t, x, y,
					    TILE_BLEND_SRCALPHA,
					    (((r1 - r2) * a) >> 8) + r2,
					    (((g1 - g2) * a) >> 8) + g2,
					    (((b1 - b2) * a) >> 8) + b2,
					    (((a1 - a2) * a) >> 8) + a2);
				}
			}
			SDL_UnlockSurface(su);
		}
		break;
	case FILL_CGRADIENT:
		{
			int i, r = MAX(su->w,su->h);
			int x = su->w/2;
			int y = su->h/2;
		
			SDL_LockSurface(su);
			for (i = 0; i < r; i++) {
				Uint8 a = (r-i)*255/r;
				Uint32 c;
			
				c = SDL_MapRGBA(su->format,
				    (((r1 - r2) * a) >> 8) + r2,
				    (((g1 - g2) * a) >> 8) + g2,
				    (((b1 - b2) * a) >> 8) + b2,
				    (((a1 - a2) * a) >> 8) + a2);
				circle2(t, x, y, i, c);
			}
			SDL_UnlockSurface(su);
		}
		break;
	default:
		break;
	}
}

