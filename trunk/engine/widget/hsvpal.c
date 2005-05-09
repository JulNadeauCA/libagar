/*	$Csoft: hsvpal.c,v 1.14 2005/05/08 13:26:21 vedge Exp $	*/

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
hsvpal_new(void *parent, SDL_PixelFormat *fmt)
{
	struct hsvpal *pal;

	pal = Malloc(sizeof(struct hsvpal), M_OBJECT);
	hsvpal_init(pal, fmt);
	object_attach(parent, pal);
	return (pal);
}

static __inline__ void
update_pixel_from_hsv(struct hsvpal *pal)
{
	float h, s, v, a;
	Uint8 r, g, b;

	h = widget_get_float(pal, "hue");
	s = widget_get_float(pal, "saturation");
	v = widget_get_float(pal, "value");
	a = widget_get_float(pal, "alpha");

	prim_hsv2rgb(h, s, v, &r, &g, &b);
	widget_set_uint32(pal, "pixel",
	    SDL_MapRGBA(pal->format, r, g, b, (Uint8)(a*255.0)));
}

static __inline__ void
update_hsv_from_pixel(struct hsvpal *hsv, Uint32 pixel)
{
	Uint8 r, g, b, a;
	float h, s, v;

	SDL_GetRGBA(pixel, hsv->format, &r, &g, &b, &a);
	prim_rgb2hsv(r, g, b, &h, &s, &v);
	widget_set_float(hsv, "hue", h);
	widget_set_float(hsv, "saturation", s);
	widget_set_float(hsv, "value", v);
	widget_set_float(hsv, "alpha", ((float)a)/255.0);
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
	update_pixel_from_hsv(pal);

	/* XXX use a timer */
	render_palette(pal);
	widget_update_surface(pal, 0);
	
	event_post(NULL, pal, "h-changed", NULL);
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
	update_pixel_from_hsv(pal);

	event_post(NULL, pal, "sv-changed", NULL);
}

static void
mousebuttondown(int argc, union evarg *argv)
{
	struct hsvpal *pal = argv[0].p;
	int btn = argv[1].i;
	int x = argv[2].i - pal->circle.x;
	int y = argv[3].i - pal->circle.y;
	float r = hypot((float)x, (float)y);

	if (btn == SDL_BUTTON_LEFT) {
		if (r > (float)pal->circle.rin) {
			update_h(pal, x, y);
			pal->state = HSVPAL_SEL_H;
		} else {
			update_sv(pal, argv[2].i, argv[3].i);
			pal->state = HSVPAL_SEL_SV;
		}
		widget_focus(pal);
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
	}
}

static void
binding_changed(int argc, union evarg *argv)
{
	struct hsvpal *hsv = argv[0].p;
	struct widget_binding *bind = argv[1].p;

	if (bind->type == WIDGET_UINT32 &&
	    strcmp(bind->name, "pixel") == 0) {
		hsv->flags |= HSVPAL_PIXEL;
		update_hsv_from_pixel(hsv, *(Uint32 *)bind->p1);
	}
	
}

void
hsvpal_init(struct hsvpal *pal, SDL_PixelFormat *fmt)
{
	int i;

	widget_init(pal, "hsvpal", &hsvpal_ops, WIDGET_FOCUSABLE);
	widget_bind(pal, "hue", WIDGET_FLOAT, &pal->h);
	widget_bind(pal, "saturation", WIDGET_FLOAT, &pal->s);
	widget_bind(pal, "value", WIDGET_FLOAT, &pal->v);
	widget_bind(pal, "alpha", WIDGET_FLOAT, &pal->a);
	widget_bind(pal, "pixel", WIDGET_UINT32, &pal->pixel);

	pal->format = fmt;
	pal->h = 0.0;
	pal->s = 0.0;
	pal->v = 0.0;
	pal->a = 1.0;
	pal->pixel = SDL_MapRGBA(fmt, 0, 0, 0, 255);
	pal->circle.spacing = 10;
	pal->circle.width = 20;
	pal->state = HSVPAL_SEL_NONE;
	pal->surface = NULL;
	widget_map_surface(pal, NULL);

	event_new(pal, "window-mousebuttonup", mousebuttonup, NULL);
	event_new(pal, "window-mousebuttondown", mousebuttondown, NULL);
	event_new(pal, "window-mousemotion", mousemotion, NULL);
	event_new(pal, "widget-bound", binding_changed, NULL);
}

static void
render_palette(struct hsvpal *pal)
{
	float h, cur_h;
	Uint32 pc;
	Uint8 r, g, b;
	int x, y, i;
	
	cur_h = widget_get_float(pal, "hue");
	cur_h /= 360.0;
	cur_h *= 2*M_PI;

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
	pal->rpreview.x = 0;
	pal->rpreview.h = 32;
	pal->rpreview.y = WIDGET(pal)->h - pal->rpreview.h;
	pal->rpreview.w = WIDGET(pal)->w;

	pal->circle.rout = MIN(WIDGET(pal)->w,
	    WIDGET(pal)->h - pal->rpreview.h)/2;
	pal->circle.rin = pal->circle.rout - pal->circle.width;
	pal->circle.dh = (float)(1.0/(pal->circle.rout*M_PI));
	pal->circle.x = WIDGET(pal)->w/2;
	pal->circle.y = (WIDGET(pal)->h - pal->rpreview.h)/2;

	pal->triangle.x = WIDGET(pal)->w/2;
	pal->triangle.y = pal->circle.y+pal->circle.width-pal->circle.rout;
	pal->triangle.h = pal->circle.rin*sin((37.0/360.0)*(2*M_PI)) -
			  pal->circle.rin*sin((270.0/360.0)*(2*M_PI));
	
	pal->selcircle_r = pal->circle.width/2 - 4;

	pal->surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
	    WIDGET(pal)->w, WIDGET(pal)->h - pal->rpreview.h, 32,
	    vfmt->Rmask, vfmt->Gmask, vfmt->Bmask, 0);
	if (pal->surface == NULL) {
		fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
	}
	render_palette(pal);
	widget_replace_surface(pal, 0, pal->surface);
}

void
hsvpal_draw(void *p)
{
	struct hsvpal *pal = p;
	float cur_h, cur_s, cur_v;
	Uint32 pc;
	int x, y;
	int i;
	Uint8 r, g, b, a;

#if 0
	/* numerically instable */
	if (pal->flags & HSVPAL_PIXEL) {
		Uint32 pixel = widget_get_uint32(pal, "pixel");

		SDL_GetRGBA(pixel, pal->format, &r, &g, &b, &a);
		prim_rgb2hsv(r, g, b, &cur_h, &cur_s, &cur_v);
	} else
#else
	{
		cur_h = widget_get_float(pal, "hue");
		cur_s = widget_get_float(pal, "saturation");
		cur_v = widget_get_float(pal, "value");
		a = (Uint8)(widget_get_float(pal, "alpha")*255);
	}
#endif
	cur_h /= 360.0;
	cur_h *= 2*M_PI;

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

	/* Draw the preview rectangle. */
	prim_hsv2rgb((cur_h/(2*M_PI))*360.0, cur_s, cur_v, &r, &g, &b);
	pc = SDL_MapRGB(vfmt, r, g, b);
	if (a < 255) {
		/* 
		 * TODO optimize blending on the basis that the background is
		 * predictable.
		 */
		primitives.tiling(pal, pal->rpreview, 8, 8,
		    COLOR(HSVPAL_TILE1_COLOR),
		    COLOR(HSVPAL_TILE2_COLOR));
		for (y = pal->rpreview.y + 5;
		     y < pal->rpreview.y + pal->rpreview.h - 5;
		     y++) {
			for (x = pal->rpreview.x + 20;
			     x < pal->rpreview.x + pal->rpreview.w - 20;
			     x++) {
				BLEND_RGBA2_CLIPPED(view->v,
				    WIDGET(pal)->cx+x,
				    WIDGET(pal)->cy+y,
				    r, g, b, a);
			}
		}
	} else {
		primitives.rect_filled(pal,
		    pal->rpreview.x, pal->rpreview.y,
		    pal->rpreview.w, pal->rpreview.h,
		    pc);
	}

	{
		char text[LABEL_MAX];
		Uint8 lr, lg, lb;
		Uint32 cText;
		SDL_Surface *sText;

		if (cur_v < 0.50 ||
		   (cur_s > 0.6 && (cur_h > 3.665 || cur_h < 0.6453))) {
			lr = 255;
			lg = 255;
			lb = 255;
		} else {
			lr = 0;
			lg = 0;
			lb = 0;
		}

		cText = SDL_MapRGB(vfmt, lr, lg, lb);
		snprintf(text, sizeof(text), "%u,%u,%u,%u", r, g, b, a);
		sText = text_render(NULL, -1, cText, text);
		widget_blit(pal, sText,
		    WIDGET(pal)->w/2 - sText->w/2,
		    pal->rpreview.y + pal->rpreview.h/2 - sText->h/2);
		SDL_FreeSurface(sText);
	}
}

