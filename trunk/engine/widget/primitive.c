/*	$Csoft: primitive.c,v 1.5 2002/06/20 16:36:39 vedge Exp $	    */

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

SDL_Surface *
primitive_box(void *p, int w, int h, int z)
{
	struct widget *wid = p;
	SDL_Surface *s;
	int x, y;
	Uint32 lcol, rcol, bcol;

	s = view_surface(SDL_SWSURFACE, w, h);

	if (z < 0) {
		z = abs(z);
		lcol = SDL_MapRGB(s->format, 20, 20, 20);
		rcol = SDL_MapRGB(s->format, 140, 140, 140);
		if (WIDGET_FOCUSED(wid)) {
			bcol = SDL_MapRGB(s->format, 100, 100, 100);
		} else {
			bcol = SDL_MapRGB(s->format, 60, 60, 60);
		}
	} else {
		lcol = SDL_MapRGB(s->format, 140, 140, 140);
		rcol = SDL_MapRGB(s->format, 20, 20, 20);
		if (WIDGET_FOCUSED(wid)) {
			bcol = SDL_MapRGB(s->format, 110, 110, 110);
		} else {
			bcol = SDL_MapRGB(s->format, 90, 90, 90);
		}
	}

	/* Background */
	SDL_FillRect(s, NULL, bcol);

	/* Border */
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {	/* XXX waste */
			if (y < z || x < z) {
				VIEW_PUT_PIXEL(s, x, y, lcol);
			} else if (y >= (h - z) || x >= (w - z)) {
				VIEW_PUT_PIXEL(s, x, y, rcol);
			}
		}
	}

	return (s);
}

SDL_Surface *
primitive_frame(void *p, int w, int h, int col)
{
	struct widget *wid = p;
	SDL_Surface *s;
	Uint32 fcol;
	int i;

	s = view_surface(SDL_SWSURFACE, w, h);
	if (col) {
		fcol = SDL_MapRGB(s->format, 200, 200, 200);
	} else {
		fcol = SDL_MapRGB(s->format, 150, 150, 150);
	}

	for (i = 0; i < h; i++) {
		VIEW_PUT_PIXEL(s, 0, i, fcol);
		VIEW_PUT_PIXEL(s, w - 1, i, fcol);
	}
	for (i = 0; i < w; i++) {
		VIEW_PUT_PIXEL(s, i, 0, fcol);
		VIEW_PUT_PIXEL(s, i, h - 1, fcol);
	}

	return (s);
}

SDL_Surface *
primitive_circle(void *p, int w, int h, int radius, int z)
{
	SDL_Surface *s;
	int x = 0, y, cx, cy, e = 0, u = 1, v;
	Uint32 fcol;

	y = radius;
	cx = w / 2;
	cy = h / 2;
	v = 2*radius - 1;

	s = view_surface(SDL_SWSURFACE, w, h);
	if (z) {
		fcol = SDL_MapRGB(s->format, 200, 200, 200);
	} else {
		fcol = SDL_MapRGB(s->format, 150, 150, 150);
	}

	while (x < y) {
		VIEW_PUT_PIXEL(s, cx + x, cy + y, fcol);
		VIEW_PUT_PIXEL(s, cx + x, cy - y, fcol);
		VIEW_PUT_PIXEL(s, cx - x, cy + y, fcol);
		VIEW_PUT_PIXEL(s, cx - x, cy - y, fcol);
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
		VIEW_PUT_PIXEL(s, cx + y, cy + x, fcol);
		VIEW_PUT_PIXEL(s, cx + y, cy - x, fcol);
		VIEW_PUT_PIXEL(s, cx - y, cy + x, fcol);
		VIEW_PUT_PIXEL(s, cx - y, cy - x, fcol);
	}
	return (s);
}

