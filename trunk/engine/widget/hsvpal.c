/*	$Csoft: hsvpal.c,v 1.27 2005/01/25 01:18:57 vedge Exp $	*/

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

enum {
	CIRCLE_COLOR,
	CUR_COLOR
};

struct hsvpal *
hsvpal_new(void *parent, SDL_PixelFormat *fmt)
{
	struct hsvpal *pal;

	pal = Malloc(sizeof(struct hsvpal), M_OBJECT);
	hsvpal_init(pal, fmt);
	object_attach(parent, pal);
	return (pal);
}

void
hsvpal_init(struct hsvpal *pal, SDL_PixelFormat *fmt)
{
	int i;

	widget_init(pal, "hsvpal", &hsvpal_ops, WIDGET_FOCUSABLE);
	widget_bind(pal, "hue", WIDGET_FLOAT, &pal->h);
	widget_bind(pal, "saturation", WIDGET_FLOAT, &pal->s);
	widget_bind(pal, "value", WIDGET_FLOAT, &pal->v);

	widget_map_color(pal, CIRCLE_COLOR, "circle", 0, 0, 0, 255);
	widget_map_color(pal, CUR_COLOR, "_current", 255, 255, 255, 255);

	pal->format = fmt;
	pal->h = 0.0;
	pal->s = 0.0;
	pal->v = 0.0;
	pal->circle.spacing = 10;
	pal->circle.width = 20;
}

void
hsvpal_scale(void *p, int w, int h)
{
	struct hsvpal *pal = p;
	int i, y = 0;

	if (w == -1 && h == -1) {
		WIDGET(pal)->w = 128;
		WIDGET(pal)->h = 160;
	}
	pal->rpreview.x = 0;
	pal->rpreview.y = WIDGET(pal)->h - 32;
	pal->rpreview.w = WIDGET(pal)->w;
	pal->rpreview.h = 32;
	
	pal->circle.rout = MIN(WIDGET(pal)->w,
	    WIDGET(pal)->h - pal->rpreview.h - pal->circle.spacing)/2;
	pal->circle.rin = pal->circle.rout - pal->circle.width;
	pal->circle.dh = (float)(1.0/(pal->circle.rout*M_PI));
	pal->circle.x = WIDGET(pal)->w/2;
	pal->circle.y = (WIDGET(pal)->h - pal->rpreview.h)/2;

	pal->triangle.x = WIDGET(pal)->w/2;
	pal->triangle.y = pal->circle.y+pal->circle.width-pal->circle.rout;
	pal->triangle.h = pal->circle.rin*sin((37.0/360.0)*(2*M_PI)) -
			  pal->circle.rin*sin((270.0/360.0)*(2*M_PI));
}

static __inline__ void
car2pol(double x, double y, double *rho, double *theta)
{
	*rho = hypot(x, y);
	*theta = atan2(y, x);
}

static __inline__ void
pol2car(double r, double theta, double *x, double *y)
{
	*x = r*cos(theta);
	*y = r*sin(theta);
}

void
hsvpal_draw(void *p)
{
	struct hsvpal *pal = p;
	float hv = widget_get_float(pal, "hue")/360.0 * 2*M_PI;
	float h;
	Uint32 pc;
	Uint8 r, g, b;
	int x, y;
	int i;

	SDL_LockSurface(view->v);
	
	/* Render the circle of hues. */
	for (h = 0.0; h < 2*M_PI; h += pal->circle.dh) {
		prim_hsv2rgb(h/(2*M_PI), 1.0, 1.0, &r, &g, &b);
		pc = SDL_MapRGB(vfmt, r, g, b);

		for (i = 0; i < pal->circle.width; i++) {
			x = (pal->circle.rout - i)*cos(h);
			y = (pal->circle.rout - i)*sin(h);

			widget_put_pixel(pal,
			    pal->circle.x+x,
			    pal->circle.y+y,
			    pc);
		}
	}

	/* Render the triangle of saturation and value. */
	for (y = 0; y < pal->triangle.h; y += 2) {
		float sat = (float)(pal->triangle.h - y) /
		            (float)(pal->triangle.h);
		int w = y;

		for (x = 0; x < w; x++) {
			prim_hsv2rgb(hv/(2*M_PI), sat,
			    1.0-((float)x/(float)pal->triangle.h),
			    &r, &g, &b);
			pc = SDL_MapRGB(vfmt, r, g, b);
			widget_put_pixel(pal,
			    pal->triangle.x + x - y/2,
			    pal->triangle.y + y,
			    pc);
			widget_put_pixel(pal,
			    pal->triangle.x + x - y/2,
			    pal->triangle.y + y + 1,
			    pc);
		}
	}

	SDL_UnlockSurface(view->v);

	primitives.circle(pal,
	    pal->circle.x + (pal->circle.rin + pal->circle.width/2)*cos(hv),
	    pal->circle.y + (pal->circle.rin + pal->circle.width/2)*sin(hv),
	    pal->circle.width/2 - 4,
	    CIRCLE_COLOR);

	/* Draw the preview rectangle. */
	prim_hsv2rgb(hv/(2*M_PI), widget_get_float(pal, "saturation"),
	    widget_get_float(pal, "value"), &r, &g, &b);
	WIDGET_COLOR(pal,CUR_COLOR) = SDL_MapRGB(vfmt, r, g, b);
	primitives.rect_filled(pal,
	    pal->rpreview.x, pal->rpreview.y,
	    pal->rpreview.w, pal->rpreview.h - pal->circle.spacing,
	    CUR_COLOR);
}

