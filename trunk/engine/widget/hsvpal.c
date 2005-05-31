/*	$Csoft: hsvpal.c,v 1.19 2005/05/31 04:03:13 vedge Exp $	*/

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
#include <engine/config.h>
#include <engine/view.h>

#include <engine/rg/prim.h>

#include "hsvpal.h"

#include <engine/widget/primitive.h>
#include <engine/widget/window.h>
#include <engine/widget/fspinbutton.h>

const struct widget_ops hsvpal_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		widget_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	hsvpal_draw,
	hsvpal_scale
};

static void render_palette(struct hsvpal *);

struct hsvpal *
hsvpal_new(void *parent)
{
	struct hsvpal *pal;

	pal = Malloc(sizeof(struct hsvpal), M_OBJECT);
	hsvpal_init(pal);
	object_attach(parent, pal);
	return (pal);
}

static __inline__ Uint8
get_alpha8(struct hsvpal *pal)
{
	struct widget_binding *bAlpha;
	void *pAlpha;
	Uint8 a = 255;

	bAlpha = widget_get_binding(pal, "alpha", &pAlpha);
	switch (bAlpha->vtype) {
	case WIDGET_FLOAT:
		a = (Uint8)((*(float *)pAlpha)*255.0);
		break;
	case WIDGET_DOUBLE:
		a = (Uint8)((*(double *)pAlpha)*255.0);
		break;
	case WIDGET_INT:
		a = (int)((*(int *)pAlpha));
		break;
	case WIDGET_UINT8:
		a = (int)((*(Uint8 *)pAlpha));
		break;
	}
	widget_binding_unlock(bAlpha);
	return (a);
}
	
static __inline__ void
set_alpha8(struct hsvpal *pal, Uint8 a)
{
	struct widget_binding *bAlpha;
	void *pAlpha;

	bAlpha = widget_get_binding(pal, "alpha", &pAlpha);
	switch (bAlpha->vtype) {
	case WIDGET_FLOAT:
		*(float *)pAlpha = (float)(((float)a)/255.0);
		break;
	case WIDGET_DOUBLE:
		*(double *)pAlpha = (double)(((double)a)/255.0);
		break;
	case WIDGET_INT:
		*(int *)pAlpha = (int)a;
		break;
	case WIDGET_UINT8:
		*(Uint8 *)pAlpha = (Uint8)a;
		break;
	}
	widget_binding_unlock(bAlpha);
}

static __inline__ void
update_pixel_from_hsva(struct hsvpal *pal)
{
	float h, s, v;
	Uint8 r, g, b, a;
	struct widget_binding *bFormat;
	SDL_PixelFormat **pFormat;

	h = widget_get_float(pal, "hue");
	s = widget_get_float(pal, "saturation");
	v = widget_get_float(pal, "value");
	bFormat = widget_get_binding(pal, "pixel-format", &pFormat);
	prim_hsv2rgb(h, s, v, &r, &g, &b);
	widget_set_uint32(pal, "pixel", SDL_MapRGBA(*pFormat, r, g, b,
	    get_alpha8(pal)));
	widget_binding_unlock(bFormat);
}

static __inline__ void
update_hsv_from_pixel(struct hsvpal *hsv, Uint32 pixel)
{
	Uint8 r, g, b, a;
	float h, s, v;
	struct widget_binding *bFormat;
	SDL_PixelFormat **pFormat;
	
	bFormat = widget_get_binding(hsv, "pixel-format", &pFormat);
	SDL_GetRGBA(pixel, *pFormat, &r, &g, &b, &a);
	prim_rgb2hsv(r, g, b, &h, &s, &v);
	widget_set_float(hsv, "hue", h);
	widget_set_float(hsv, "saturation", s);
	widget_set_float(hsv, "value", v);
	set_alpha8(hsv, a);
	widget_binding_unlock(bFormat);
}

static __inline__ void
update_h(struct hsvpal *pal, int x, int y)
{
	float h;

	h = atan2((float)y, (float)x);
	if (h < 0) {
		h += 2*M_PI;
	}
	widget_set_float(pal, "hue", h/(2*M_PI)*360.0);

	update_pixel_from_hsva(pal);
	event_post(NULL, pal, "h-changed", NULL);
	pal->flags |= HSVPAL_DIRTY;
}

static void
update_sv(struct hsvpal *pal, int ax, int ay)
{
	float s, v;
	int x = ax - pal->triangle.x;
	int y = ay - pal->triangle.y;

	if (x < -y/2) { x = -y/2; }
	if (x > y/2) { x = y/2; }
	if (y > pal->triangle.h-1) { y = pal->triangle.h-1; }

	s = 1.0 - (float)y/(float)pal->triangle.h;
	v = 1.0 - (float)(x + y/2)/(float)pal->triangle.h;

	if (s < 0.0) { s = 0.00001; }
	else if (s > 1.0) { s = 1.0; }

	if (v < 0.0) { v = 0.0001; }
	else if (v > 1.0) { v = 1.0; }

	widget_set_float(pal, "saturation", s);
	widget_set_float(pal, "value", v);

	update_pixel_from_hsva(pal);
	event_post(NULL, pal, "sv-changed", NULL);
	pal->flags |= HSVPAL_DIRTY;
}

static void
update_a(struct hsvpal *pal, int x)
{
	struct widget_binding *bAlpha;
	void *pAlpha;

	bAlpha = widget_get_binding(pal, "alpha", &pAlpha);
	switch (bAlpha->vtype) {
	case WIDGET_FLOAT:
		*(float *)pAlpha = ((float)x)/((float)pal->rAlpha.w);
		if (*(float *)pAlpha > 1.0) {
			*(float *)pAlpha = 1.0;
		} else if (*(float *)pAlpha < 0.0) {
			*(float *)pAlpha = 0.0;
		}
		break;
	case WIDGET_DOUBLE:
		*(double *)pAlpha = ((double)x)/((double)pal->rAlpha.w);
		if (*(double *)pAlpha > 1.0) {
			*(double *)pAlpha = 1.0;
		} else if (*(double *)pAlpha < 0.0) {
			*(double *)pAlpha = 0.0;
		}
		break;
	case WIDGET_INT:
		*(int *)pAlpha = x/pal->rAlpha.w;
		if (*(int *)pAlpha > 255) {
			*(int *)pAlpha = 255;
		} else if (*(int *)pAlpha < 0) {
			*(int *)pAlpha = 0;
		}
		break;
	case WIDGET_UINT8:
		*(Uint8 *)pAlpha = (Uint8)(x/pal->rAlpha.w);
		break;
	}
	widget_binding_unlock(bAlpha);

	update_pixel_from_hsva(pal);
	event_post(NULL, pal, "a-changed", NULL);
}

static void
close_menu(struct hsvpal *pal)
{
	menu_collapse(pal->menu, pal->menu_item);
	object_destroy(pal->menu);
	Free(pal->menu, M_OBJECT);

	pal->menu = NULL;
	pal->menu_item = NULL;
	pal->menu_win = NULL;
}

static void
edit_values(int argc, union evarg *argv)
{
	struct hsvpal *pal = argv[1].p;
	struct window *pwin;
	struct window *win;
	struct fspinbutton *fsb;
	struct widget_binding *b1, *b2;
	float v;

	if ((pwin = widget_parent_window(pal)) == NULL)
		return;

	if ((win = window_new(WINDOW_NO_MAXIMIZE|WINDOW_DETACH,
	    "hsvpal-%p-numedit", pal)) == NULL) {
		return;
	}
	window_set_caption(win, _("Color values"));
	window_set_position(win, WINDOW_LOWER_LEFT, 0);
	{
		struct widget_binding *bAlpha;
		void *pAlpha;

		fsb = fspinbutton_new(win, NULL, _("Hue: "));
		fspinbutton_prescale(fsb, "000");
		widget_copy_binding(fsb, "value", pal, "hue");
		fspinbutton_set_range(fsb, 0.0, 359.0);
		fspinbutton_set_increment(fsb, 1);
		fspinbutton_set_precision(fsb, "f", 0);
		
		fsb = fspinbutton_new(win, NULL, _("Saturation: "));
		fspinbutton_prescale(fsb, "00.00");
		widget_copy_binding(fsb, "value", pal, "saturation");
		fspinbutton_set_range(fsb, 0.0, 1.0);
		fspinbutton_set_increment(fsb, 0.01);
		fspinbutton_set_precision(fsb, "f", 2);

		fsb = fspinbutton_new(win, NULL, _("Value: "));
		fspinbutton_prescale(fsb, "00.00");
		widget_copy_binding(fsb, "value", pal, "value");
		fspinbutton_set_range(fsb, 0.0, 1.0);
		fspinbutton_set_increment(fsb, 0.01);
		fspinbutton_set_precision(fsb, "f", 2);

		fsb = fspinbutton_new(win, NULL, _("Alpha: "));
		fspinbutton_prescale(fsb, "0.000");
		widget_copy_binding(fsb, "value", pal, "alpha");
		bAlpha = widget_get_binding(pal, "alpha", &pAlpha);
		switch (bAlpha->vtype) {
		case WIDGET_FLOAT:
		case WIDGET_DOUBLE:
			fspinbutton_set_range(fsb, 0.0, 1.0);
			fspinbutton_set_increment(fsb, 0.005);
			fspinbutton_set_precision(fsb, "f", 3);
			break;
		case WIDGET_INT:
		case WIDGET_UINT:
		case WIDGET_UINT8:
			fspinbutton_set_range(fsb, 0.0, 255.0);
			fspinbutton_set_increment(fsb, 1.0);
			fspinbutton_set_precision(fsb, "f", 0);
			break;
		}
		widget_binding_unlock(bAlpha);
	}

	window_attach(pwin, win);
	window_show(win);
}

static void
complementary(int argc, union evarg *argv)
{
	struct hsvpal *pal = argv[1].p;
	float hue = widget_get_float(pal, "hue");

	widget_set_float(pal, "hue", ((int)hue+180) % 359);
	pal->flags |= HSVPAL_DIRTY;
}

static void
invert_saturation(int argc, union evarg *argv)
{
	struct hsvpal *pal = argv[1].p;

	widget_set_float(pal, "saturation",
	    1.0 - widget_get_float(pal, "saturation"));
	pal->flags |= HSVPAL_DIRTY;
}

static void
invert_value(int argc, union evarg *argv)
{
	struct hsvpal *pal = argv[1].p;

	widget_set_float(pal, "value",
	    1.0 - widget_get_float(pal, "value"));
	pal->flags |= HSVPAL_DIRTY;
}

static void
open_menu(struct hsvpal *pal)
{
	int x, y;

	if (pal->menu != NULL)
		close_menu(pal);

	pal->menu = Malloc(sizeof(struct AGMenu), M_OBJECT);
	menu_init(pal->menu);

	pal->menu_item = menu_add_item(pal->menu, NULL);
	{
		menu_action(pal->menu_item, _("Edit numerically"), -1,
		    edit_values, "%p", pal);

		menu_separator(pal->menu_item);

		menu_action(pal->menu_item, _("Complementary color"), -1,
		    complementary, "%p", pal);
		menu_action(pal->menu_item, _("Invert saturation"), -1,
		    invert_saturation, "%p", pal);
		menu_action(pal->menu_item, _("Invert value"), -1,
		    invert_value, "%p", pal);
	}
	pal->menu->sel_item = pal->menu_item;
	
	SDL_GetMouseState(&x, &y);
	pal->menu_win = menu_expand(pal->menu, pal->menu_item, x, y);
}

static void
mousebuttondown(int argc, union evarg *argv)
{
	struct hsvpal *pal = argv[0].p;
	int btn = argv[1].i;
	int x = argv[2].i;
	int y = argv[3].i;
	float r;

	switch (btn) {
	case SDL_BUTTON_LEFT:
		if (y > pal->rAlpha.y) {
			update_a(pal, x);
			pal->state = HSVPAL_SEL_A;
		} else {
			x -= pal->circle.x;
			y -= pal->circle.y;
			r = hypot((float)x, (float)y);

			if (r > (float)pal->circle.rin) {
				update_h(pal, x, y);
				pal->state = HSVPAL_SEL_H;
			} else {
				update_sv(pal, argv[2].i, argv[3].i);
				pal->state = HSVPAL_SEL_SV;
			}
		}
		widget_focus(pal);
		break;
	case SDL_BUTTON_MIDDLE:
	case SDL_BUTTON_RIGHT:
		open_menu(pal);
		break;
	}
}

static void
mousebuttonup(int argc, union evarg *argv)
{
	struct hsvpal *pal = argv[0].p;

	pal->state = HSVPAL_SEL_NONE;
}

static void
mousemotion(int argc, union evarg *argv)
{
	struct hsvpal *pal = argv[0].p;
	int x = argv[1].i;
	int y = argv[2].i;

	switch (pal->state) {
	case HSVPAL_SEL_NONE:
		break;
	case HSVPAL_SEL_H:
		update_h(pal,
		    x - pal->circle.x,
		    y - pal->circle.y);
		break;
	case HSVPAL_SEL_SV:
		update_sv(pal, x, y);
		break;
	case HSVPAL_SEL_A:
		update_a(pal, x);
		break;
	}
}

static void
binding_changed(int argc, union evarg *argv)
{
	struct hsvpal *hsv = argv[0].p;
	struct widget_binding *bind = argv[1].p;

	if (bind->type == WIDGET_UINT32 &&
	    strcmp(bind->name, "pixel") == 0) {
#if 0
		hsv->flags |= HSVPAL_PIXEL;
#endif
		update_hsv_from_pixel(hsv, *(Uint32 *)bind->p1);
	}
	
}

void
hsvpal_init(struct hsvpal *pal)
{
	int i;

	widget_init(pal, "hsvpal", &hsvpal_ops, WIDGET_FOCUSABLE);
	widget_bind(pal, "hue", WIDGET_FLOAT, &pal->h);
	widget_bind(pal, "saturation", WIDGET_FLOAT, &pal->s);
	widget_bind(pal, "value", WIDGET_FLOAT, &pal->v);
	widget_bind(pal, "alpha", WIDGET_FLOAT, &pal->a);
	widget_bind(pal, "pixel", WIDGET_UINT32, &pal->pixel);
	widget_bind(pal, "pixel-format", WIDGET_POINTER, &vfmt);

	pal->flags = 0;
	pal->h = 0.0;
	pal->s = 0.0;
	pal->v = 0.0;
	pal->a = 1.0;
	pal->pixel = SDL_MapRGBA(vfmt, 0, 0, 0, 255);
	pal->circle.spacing = 10;
	pal->circle.width = 20;
	pal->state = HSVPAL_SEL_NONE;
	pal->surface = NULL;
	pal->menu = NULL;
	pal->menu_item = NULL;
	pal->menu_win = NULL;
	widget_map_surface(pal, NULL);

	event_new(pal, "window-mousebuttonup", mousebuttonup, NULL);
	event_new(pal, "window-mousebuttondown", mousebuttondown, NULL);
	event_new(pal, "window-mousemotion", mousemotion, NULL);
	event_new(pal, "widget-bound", binding_changed, NULL);
}

static void
render_palette(struct hsvpal *pal)
{
	float h, cur_h, cur_s, cur_v;
	Uint32 pc;
	Uint8 r, g, b, a, da;
	Uint8 *pDst;
	int x, y, i;
	SDL_Rect rd;

	cur_h = (widget_get_float(pal, "hue")/360) * 2*M_PI;
	cur_s = widget_get_float(pal, "saturation");
	cur_v = widget_get_float(pal, "value");

	SDL_LockSurface(pal->surface);

	/* Render the circle of hues. */
	for (h = 0.0; h < 2*M_PI; h += pal->circle.dh) {
		prim_hsv2rgb((h/(2*M_PI)*360.0), 1.0, 1.0, &r, &g, &b);
		pc = SDL_MapRGB(vfmt, r, g, b);

		for (i = 0; i < pal->circle.width; i++) {
			x = (pal->circle.rout - i)*cos(h);
			y = (pal->circle.rout - i)*sin(h);

			PUT_PIXEL2(pal->surface,
			    pal->circle.x+x,
			    pal->circle.y+y,
			    pc);
		}
	}

	/* Render the triangle of saturation and value. */
	for (y = 0; y < pal->triangle.h; y += 2) {
		float sat = (float)(pal->triangle.h - y) /
		            (float)(pal->triangle.h);

		for (x = 0; x < y; x++) {
			prim_hsv2rgb((cur_h/(2*M_PI))*360.0, sat,
			    1.0 - ((float)x/(float)pal->triangle.h),
			    &r, &g, &b);
			pc = SDL_MapRGB(vfmt, r, g, b);
			PUT_PIXEL2(pal->surface,
			    pal->triangle.x + x - y/2,
			    pal->triangle.y + y,
			    pc);
			PUT_PIXEL2(pal->surface,
			    pal->triangle.x + x - y/2,
			    pal->triangle.y + y + 1,
			    pc);
		}
	}

	/* Render the alpha selector. */
	/* XXX overblending */
	for (y = 8; y < pal->rAlpha.h+16; y+=8) {
		for (x = 0; x < pal->rAlpha.w; x+=16) {
			rd.w = 8;
			rd.h = 8;
			rd.x = pal->rAlpha.x+x;
			rd.y = pal->rAlpha.y+y;
			SDL_FillRect(pal->surface, &rd, pal->cTile);
		}
		y += 8;
		for (x = 8; x < pal->rAlpha.w; x+=16) {
			rd.w = 8;
			rd.h = 8;
			rd.x = pal->rAlpha.x+x;
			rd.y = pal->rAlpha.y+y;
			SDL_FillRect(pal->surface, &rd, pal->cTile);
		}
	}
	prim_hsv2rgb((cur_h/(2*M_PI))*360.0, cur_s, cur_v, &r, &g, &b);
	da = MIN(1, pal->surface->w/255);
	for (y = pal->rAlpha.y+8; y < pal->surface->h; y++) {
		for (x = 0, a = 0; x < pal->surface->w; x++) {
			BLEND_RGBA2(pal->surface, x, y,
			    r, g, b, a, ALPHA_SRC);
			a = x*255/pal->surface->w;
		}
	}
	SDL_UnlockSurface(pal->surface);
}

void
hsvpal_scale(void *p, int w, int h)
{
	struct hsvpal *pal = p;
	int i, y = 0;

	if (w == -1 && h == -1) {
		WIDGET(pal)->w = view->w/5;
		WIDGET(pal)->h = view->h/3;
	}
	
	pal->rAlpha.x = 0;
	pal->rAlpha.h = 32;
	pal->rAlpha.y = WIDGET(pal)->h - 32;
	pal->rAlpha.w = WIDGET(pal)->w;
	
	pal->circle.rout = MIN(WIDGET(pal)->w,
	    WIDGET(pal)->h - pal->rAlpha.h)/2;
	pal->circle.rin = pal->circle.rout - pal->circle.width;
	pal->circle.dh = (float)(1.0/(pal->circle.rout*M_PI));
	pal->circle.x = WIDGET(pal)->w/2;
	pal->circle.y = (WIDGET(pal)->h - pal->rAlpha.h)/2;

	pal->triangle.x = WIDGET(pal)->w/2;
	pal->triangle.y = pal->circle.y+pal->circle.width-pal->circle.rout;
	pal->triangle.h = pal->circle.rin*sin((37.0/360.0)*(2*M_PI)) -
			  pal->circle.rin*sin((270.0/360.0)*(2*M_PI));
	
	pal->selcircle_r = pal->circle.width/2 - 4;

	pal->flags |= HSVPAL_DIRTY;
}

void
hsvpal_draw(void *p)
{
	struct hsvpal *pal = p;
	float cur_h, cur_s, cur_v;
	Uint8 r, g, b, a;
	int x, y;
	int i;
	
	if (pal->flags & HSVPAL_DIRTY) {
		pal->flags &= ~(HSVPAL_DIRTY);
		pal->surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
		    WIDGET(pal)->w, WIDGET(pal)->h, 32,
		    vfmt->Rmask, vfmt->Gmask, vfmt->Bmask, 0);
		if (pal->surface == NULL) {
			fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
		}
		pal->cTile = SDL_MapRGB(pal->surface->format, 140, 140, 140);
		render_palette(pal);
		widget_replace_surface(pal, 0, pal->surface);
	}

	cur_h = (widget_get_float(pal, "hue") / 360.0) * 2*M_PI;
	cur_s = widget_get_float(pal, "saturation");
	cur_v = widget_get_float(pal, "value");
	a = (Uint8)(widget_get_float(pal, "alpha")*255);

	widget_blit_from(pal, pal, 0, NULL, 0, 0);

	/* Indicate the current selection. */
	primitives.circle(pal,
	    pal->circle.x + (pal->circle.rin + pal->circle.width/2)*cos(cur_h),
	    pal->circle.y + (pal->circle.rin + pal->circle.width/2)*sin(cur_h),
	    pal->selcircle_r,
	    COLOR(HSVPAL_CIRCLE_COLOR));
	
	/* The rendering routine uses (v = 1 - x/h), so (x = -v*h + h). */
	y = (int)((1.0 - cur_s) * (float)pal->triangle.h);
	x = (int)(-(cur_v*(float)pal->triangle.h - (float)pal->triangle.h));
	if (x < 0) { x = 0; }
	if (x > y) { x = y; }
	primitives.circle(pal,
	    pal->triangle.x + x - y/2,
	    pal->triangle.y + y,
	    pal->selcircle_r,
	    COLOR(HSVPAL_CIRCLE_COLOR));

	x = a*pal->rAlpha.w/255;
	if (x > pal->rAlpha.w-3) { x = pal->rAlpha.w-3; }

	/* Draw the color preview. */
	prim_hsv2rgb((cur_h*360.0)/(2*M_PI), cur_s, cur_v, &r, &g, &b);
	primitives.rect_filled(pal,
	    pal->rAlpha.x, pal->rAlpha.y,
	    pal->rAlpha.w, 8,
	    SDL_MapRGB(vfmt, r, g, b));

	/* Draw the alpha bar. */
	primitives.vline(pal,
	    pal->rAlpha.x + x,
	    pal->rAlpha.y + 1,
	    pal->rAlpha.y + pal->rAlpha.h,
	    COLOR(HSVPAL_BAR1_COLOR));
	primitives.vline(pal,
	    pal->rAlpha.x + x + 1,
	    pal->rAlpha.y + 1,
	    pal->rAlpha.y + pal->rAlpha.h,
	    COLOR(HSVPAL_BAR2_COLOR));
	primitives.vline(pal,
	    pal->rAlpha.x + x + 2,
	    pal->rAlpha.y + 1,
	    pal->rAlpha.y + pal->rAlpha.h,
	    COLOR(HSVPAL_BAR1_COLOR));
}

