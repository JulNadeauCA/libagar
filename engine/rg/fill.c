/*	$Csoft: fill.c,v 1.10 2005/03/05 12:13:49 vedge Exp $	*/

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
#include <engine/widget/hsvpal.h>
#include <engine/widget/radio.h>
#include <engine/widget/box.h>
#include <engine/widget/fspinbutton.h>

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
	fill_menu,
	fill_toolbar,
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

	label_new(win, LABEL_STATIC, _("Filling/gradient type:"));
	rad = radio_new(win, modes);
	widget_bind(rad, "value", WIDGET_INT, &f->type);

	box = box_new(win, BOX_VERT, BOX_WFILL|BOX_HFILL);
	{
		struct hsvpal *hsv1, *hsv2;
		struct fspinbutton *fsb;

		label_new(box, LABEL_STATIC, _("Color 1: "));
		hsv1 = hsvpal_new(box, tv->ts->fmt);
		widget_bind(hsv1, "pixel", WIDGET_UINT32, &f->f_gradient.c1);

		label_new(box, LABEL_STATIC, _("Color 2: "));
		hsv2 = hsvpal_new(box, tv->ts->fmt);
		widget_bind(hsv2, "pixel", WIDGET_UINT32, &f->f_gradient.c2);
		
		fsb = fspinbutton_new(box, NULL, _("Alpha 1: "));
		fspinbutton_prescale(fsb, "0.000");
		widget_bind(fsb, "value", WIDGET_FLOAT, &hsv1->a);
		fspinbutton_set_range(fsb, 0.0, 1.0);
		fspinbutton_set_increment(fsb, 0.010);
		fspinbutton_set_precision(fsb, "f", 3);
		
		fsb = fspinbutton_new(box, NULL, _("Alpha 2: "));
		fspinbutton_prescale(fsb, "0.000");
		widget_bind(fsb, "value", WIDGET_FLOAT, &hsv2->a);
		fspinbutton_set_range(fsb, 0.0, 1.0);
		fspinbutton_set_increment(fsb, 0.010);
		fspinbutton_set_precision(fsb, "f", 3);
	}
	return (win);
}

void
fill_apply(void *p, struct tile *t, int x, int y)
{
	struct fill *fi = p;
	SDL_Surface *su = t->su;
	Uint8 r1, g1, b1, a1;
	Uint8 r2, g2, b2, a2;
	
	SDL_GetRGBA(fi->f_gradient.c1, t->ts->fmt, &r1, &g1, &b1, &a1);
	SDL_GetRGBA(fi->f_gradient.c2, t->ts->fmt, &r2, &g2, &b2, &a2);

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

				c = SDL_MapRGBA(t->ts->fmt,
				    (((r1 - r2) * a) >> 8) + r2,
				    (((g1 - g2) * a) >> 8) + g2,
				    (((b1 - b2) * a) >> 8) + b2,
				    (((a1 - a2) * a) >> 8) + a2);

				for (x = 0; x < su->w; x++) {
					prim_blend_rgb(t->su, x, y,
					    PRIM_BLEND_SRCALPHA,
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
				
					prim_blend_rgb(t->su, x, y,
					    PRIM_BLEND_SRCALPHA,
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
			
				prim_color_rgba(t,
				    (((r1 - r2) * a) >> 8) + r2,
				    (((g1 - g2) * a) >> 8) + g2,
				    (((b1 - b2) * a) >> 8) + b2,
				    (((a1 - a2) * a) >> 8) + a2);
				prim_circle2(t, x, y, i);
			}
			SDL_UnlockSurface(su);
		}
		break;
	default:
		break;
	}
}

static void
invert_colors(int argc, union evarg *argv)
{
	struct fill *fi = argv[1].p;
	struct tileset *ts = FEATURE(fi)->ts;
	Uint8 r, g, b, a;

	switch (fi->type) {
	case FILL_SOLID:
		SDL_GetRGBA(fi->f_solid.c, ts->fmt, &r, &g, &b, &a);
		r = 255 - r;
		g = 255 - g;
		b = 255 - b;
		fi->f_solid.c = SDL_MapRGBA(ts->fmt, r, g, b, a);
		break;
	case FILL_HGRADIENT:
	case FILL_VGRADIENT:
	case FILL_CGRADIENT:
		SDL_GetRGBA(fi->f_gradient.c1, ts->fmt, &r, &g, &b, &a);
		r = 255 - r;
		g = 255 - g;
		b = 255 - b;
		fi->f_gradient.c1 = SDL_MapRGBA(ts->fmt, r, g, b, a);
		SDL_GetRGBA(fi->f_gradient.c2, ts->fmt, &r, &g, &b, &a);
		r = 255 - r;
		g = 255 - g;
		b = 255 - b;
		fi->f_gradient.c2 = SDL_MapRGBA(ts->fmt, r, g, b, a);
		break;
	default:
		break;
	}
}

static void
swap_gradient(int argc, union evarg *argv)
{
	struct fill *fi = argv[1].p;
	Uint32 cSave;

	switch (fi->type) {
	case FILL_HGRADIENT:
	case FILL_VGRADIENT:
	case FILL_CGRADIENT:
		cSave = fi->f_gradient.c2;
		fi->f_gradient.c2 = fi->f_gradient.c1;
		fi->f_gradient.c1 = cSave;
		break;
	default:
		break;
	}
}

static void
set_type(int argc, union evarg *argv)
{
	struct fill *fi = argv[1].p;
	int type = argv[2].i;

	fi->type = type;
}

void
fill_menu(void *p, struct AGMenuItem *mi)
{
	struct fill *fi = p;
	struct AGMenuItem *mi_fill;

	mi_fill = menu_action(mi, _("Fill type"), RG_FILL_ICON, NULL, NULL);
	{
		menu_action(mi_fill, _("Solid fill"), RG_FILL_ICON,
		    set_type, "%p, %i", fi, FILL_SOLID);
		menu_action(mi_fill, _("Horizontal gradient"),RG_HGRADIENT_ICON,
		    set_type, "%p, %i", fi, FILL_HGRADIENT);
		menu_action(mi_fill, _("Vertical gradient"), RG_VGRADIENT_ICON,
		    set_type, "%p, %i", fi, FILL_VGRADIENT);
		menu_action(mi_fill, _("Circular gradient"), RG_CGRADIENT_ICON,
		    set_type, "%p, %i", fi, FILL_CGRADIENT);
		menu_action(mi_fill, _("Pattern"), RG_TILING_ICON,
		    set_type, "%p, %i", fi, FILL_PATTERN);
	}
	
	menu_separator(mi);

	menu_action(mi, _("Swap gradient colors"), RG_SWAP_ICON,
	    swap_gradient, "%p", fi);
	menu_action(mi, _("Invert colors"), RG_INVERT_ICON,
	    invert_colors, "%p", fi);
}

struct toolbar *
fill_toolbar(void *p, struct tileview *tv)
{
	struct fill *fi = p;
	struct toolbar *tbar;

	tbar = toolbar_new(tv->tel_box, TOOLBAR_VERT, 1, 0);
	toolbar_add_button(tbar, 0, ICON(RG_SWAP_ICON), 0, 0,
	    swap_gradient, "%p", fi);
	toolbar_add_button(tbar, 0, ICON(RG_INVERT_ICON), 0, 0,
	    invert_colors, "%p", fi);
	return (tbar);
}

