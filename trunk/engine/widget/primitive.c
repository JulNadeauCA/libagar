/*	$Csoft: primitive.c,v 1.6 2002/07/06 05:31:29 vedge Exp $	    */

/*
 * Copyright (c) 2002 CubeSoft Communications <http://www.csoft.org>
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

#include <stdlib.h>

#include <engine/engine.h>

#include "window.h"
#include "widget.h"
#include "primitive.h"

static void	box_3d(void *, int, int, int, int, int);
static void	frame_2d(void *, int, int, int);
static void	bresenham_circle(void *, int, int, int, int);

struct primitive_ops primitives = {
	box_3d,			/* box */
	frame_2d,		/* frame */
	bresenham_circle	/* circle */
};

static void
box_3d(void *p, int xoffs, int yoffs, int w, int h, int z)
{
	struct widget *wid = p;
	Uint32 lcol, rcol, bcol;
	int x, y;

	OBJECT_ASSERT(wid, "widget");

	if (z < 0) {
		z = abs(z);
		lcol = SDL_MapRGB(view->v->format, 20, 20, 20);
		rcol = SDL_MapRGB(view->v->format, 140, 140, 140);
		if (WIDGET_FOCUSED(wid)) {
			bcol = SDL_MapRGB(view->v->format, 100, 100, 100);
		} else {
			bcol = SDL_MapRGB(view->v->format, 60, 60, 60);
		}
	} else {
		lcol = SDL_MapRGB(view->v->format, 140, 140, 140);
		rcol = SDL_MapRGB(view->v->format, 20, 20, 20);
		if (WIDGET_FOCUSED(wid)) {
			bcol = SDL_MapRGB(view->v->format, 110, 110, 110);
		} else {
			bcol = SDL_MapRGB(view->v->format, 90, 90, 90);
		}
	}

	/* Background */
	WIDGET_FILL(wid, xoffs, yoffs, w, h, bcol);

#if 0
	/* Border */
	for (y = yoffs; y < yoffs+h; y++) {
		for (x = xoffs; x < xoffs+w; x++) {	/* XXX waste */
			if (y < z || x < z) {
				WIDGET_PUT_PIXEL(wid, x, y, lcol);
			} else if (y >= (h - z) || x >= (w - z)) {
				WIDGET_PUT_PIXEL(wid, x, y, rcol);
			}
		}
	}
#endif
}

static void
frame_2d(void *p, int w, int h, int col)
{
	struct widget *wid = p;
	Uint32 fcol;
	int i;

	OBJECT_ASSERT(wid, "widget");

	if (col) {
		fcol = SDL_MapRGB(view->v->format, 200, 200, 200);
	} else {
		fcol = SDL_MapRGB(view->v->format, 150, 150, 150);
	}

	/* XXX unoptimized */
	for (i = 0; i < h; i++) {
		WIDGET_PUT_PIXEL(wid, 0, i, fcol);
		WIDGET_PUT_PIXEL(wid, w - 1, i, fcol);
	}
	for (i = 0; i < w; i++) {
		WIDGET_PUT_PIXEL(wid, i, 0, fcol);
		WIDGET_PUT_PIXEL(wid, i, h - 1, fcol);
	}
}

static void
bresenham_circle(void *wid, int w, int h, int radius, int z)
{
	int x = 0, y, cx, cy, e = 0, u = 1, v;
	Uint32 fcol;

	OBJECT_ASSERT(wid, "widget");

	y = radius;
	cx = w / 2;
	cy = h / 2;
	v = 2*radius - 1;

	if (z) {
		fcol = SDL_MapRGB(view->v->format, 200, 200, 200);
	} else {
		fcol = SDL_MapRGB(view->v->format, 150, 150, 150);
	}

	while (x < y) {
		WIDGET_PUT_PIXEL(wid, cx + x, cy + y, fcol);
		WIDGET_PUT_PIXEL(wid, cx + x, cy - y, fcol);
		WIDGET_PUT_PIXEL(wid, cx - x, cy + y, fcol);
		WIDGET_PUT_PIXEL(wid, cx - x, cy - y, fcol);
		x++;
		e += u;
		u += 2;
		if (v < 2 * e) {
			y--;
			e -= v;
			v -= 2;
		}
		if (x > y) {
			break;
		}
		WIDGET_PUT_PIXEL(wid, cx + y, cy + x, fcol);
		WIDGET_PUT_PIXEL(wid, cx + y, cy - x, fcol);
		WIDGET_PUT_PIXEL(wid, cx - y, cy + x, fcol);
		WIDGET_PUT_PIXEL(wid, cx - y, cy - x, fcol);
	}
}

