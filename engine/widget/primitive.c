/*	$Csoft	    */

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
	SDL_LockSurface(s);		/* XXX not needed */
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {	/* XXX waste */
			if (y < z || x < z) {
				VIEW_PUT_PIXEL(s, x, y, lcol);
			} else if (y >= (h - z) || x >= (w - z)) {
				VIEW_PUT_PIXEL(s, x, y, rcol);
			}
		}
	}
	SDL_UnlockSurface(s);

	return (s);
}
