/*	$Csoft: vg_primitive.c,v 1.4 2004/05/12 04:53:13 vedge Exp $	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
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

#include "vg.h"
#include "vg_math.h"
#include "vg_primitive.h"

void
vg_put_pixel(struct vg *vg, int x, int y, Uint32 c)
{
	Uint8 *d;

	if (!VIEW_INSIDE_CLIP_RECT(vg->su, x, y))
		return;

	d = (Uint8 *)vg->su->pixels +
	    y*vg->su->pitch + x*vg->su->format->BytesPerPixel;

	switch (vg->su->format->BytesPerPixel) {
	case 4:
		*(Uint32 *)d = c;
		break;
	case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		d[0] = (c>>16) & 0xff;
		d[1] = (c>>8) & 0xff;
		d[2] = c & 0xff;
#else
		d[0] = c & 0xff;
		d[1] = (c>>8) & 0xff;
		d[2] = (c>>16) & 0xff;
#endif
		break;
	case 2:
		*(Uint16 *)d = c;
		break;
	case 1:
		*d = c;
		break;
	}
}

/* Render a circle using a modified Bresenham line algorithm. */
void
vg_circle_primitive(struct vg *vg, int rx, int ry, int radius, Uint32 color)
{
	SDL_Surface *su = vg->su;
	int v = 2*radius - 1;
	int e = 0, u = 1;
	int x = 0, y = radius;

	while (x < y) {
		vg_put_pixel(vg, rx+x, ry+y, color);
		vg_put_pixel(vg, rx+x, ry-y, color);
		vg_put_pixel(vg, rx-x, ry+y, color);
		vg_put_pixel(vg, rx-x, ry-y, color);

		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;
		
		vg_put_pixel(vg, rx+y, ry+x, color);
		vg_put_pixel(vg, rx+y, ry-x, color);
		vg_put_pixel(vg, rx-y, ry+x, color);
		vg_put_pixel(vg, rx-y, ry-x, color);
	}
	vg_put_pixel(vg, rx-radius, ry, color);
	vg_put_pixel(vg, rx+radius, ry, color);
}

/*
 * Draw a partial ellipse centered around [cx,cy]. Angles a1, a2 are
 * given in degrees. It is assumed that a2 > a1.
 * TODO generalize Bresenham
 */
void
vg_arc_primitive(struct vg *vg, int cx, int cy, int w, int h, int a1, int a2,
    Uint32 color)
{
	extern int vg_cos_tbl[];
	extern int vg_sin_tbl[];
	SDL_Surface *su = vg->su;
	int px = 0, py = 0;
	int w2 = w/2, h2 = h/2;
	int a;

	while (a2 < a1)
		a2 += 360;

	for (a = a1; a <= a2; a++) {
		int x, y;

		x = ((long)vg_cos_tbl[a % 360]*(long)w2/1024) + cx; 
		y = ((long)vg_sin_tbl[a % 360]*(long)h2/1024) + cy;
		if (a != a1) {
			vg_line_primitive(vg, px, py, x, y, color);
		}
		px = x;
		py = y;
	}
}

/*
 * Draw a line segment between two points using the Bresenham algorithm
 * presented by Foley & Van Dam [1990].
 */
void
vg_line_primitive(struct vg *vg, int x1, int y1, int x2, int y2, Uint32 color)
{
	SDL_Surface *su = vg->su;
	int dx, dy;
	int inc1, inc2;
	int d, x, y;
	int xend, yend;
	int xdir, ydir;

	dx = abs(x2-x1);
	dy = abs(y2-y1);

	if (dy <= dx) {
		d = dy*2 - dx;
		inc1 = dy*2;
		inc2 = (dy-dx)*2;
		if (x1 > x2) {
			x = x2;
			y = y2;
			ydir = -1;
			xend = x1;
		} else {
			x = x1;
			y = y1;
			ydir = 1;
			xend = x2;
		}
		vg_put_pixel(vg, x, y, color);

		if (((y2-y1)*ydir) > 0) {
			while (x < xend) {
				x++;
				if (d < 0) {
					d += inc1;
				} else {
					y++;
					d += inc2;
				}
				vg_put_pixel(vg, x, y, color);
			}
		} else {
			while (x < xend) {
				x++;
				if (d < 0) {
					d += inc1;
				} else {
					y--;
					d += inc2;
				}
				vg_put_pixel(vg, x, y, color);
			}
		}		
	} else {
		d = dx*2 - dy;
		inc1 = dx*2;
		inc2 = (dx-dy)*2;
		if (y1 > y2) {
			y = y2;
			x = x2;
			yend = y1;
			xdir = -1;
		} else {
			y = y1;
			x = x1;
			yend = y2;
			xdir = 1;
		}
		vg_put_pixel(vg, x, y, color);

		if (((x2-x1)*xdir) > 0) {
			while (y < yend) {
				y++;
				if (d < 0) {
					d += inc1;
				} else {
					x++;
					d += inc2;
				}
				vg_put_pixel(vg, x, y, color);
			}
		} else {
			while (y < yend) {
				y++;
				if (d < 0) {
					d += inc1;
				} else {
					x--;
					d += inc2;
				}
				vg_put_pixel(vg, x, y, color);
			}
		}
	}
}
