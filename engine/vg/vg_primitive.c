/*	$Csoft: vg_primitive.c,v 1.11 2005/06/01 09:06:56 vedge Exp $	*/

/*
 * Copyright (c) 2004, 2005 CubeSoft Communications, Inc.
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

	if (CLIPPED_PIXEL(vg->su, x, y))
		return;

	d = (Uint8 *)vg->su->pixels + y*vg->su->pitch + (x<<2);
	*(Uint32 *)d = c;
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

/* Draw a horizontal line segment. */
void
vg_hline_primitive(struct vg *vg, int x1, int x2, int y, Uint32 c)
{
	SDL_Surface *su = vg->su;
	int x, xtmp;
	Uint8 *pDst, *pEnd;
	int dx;

	if (y >= su->h || y < 0) { return; }
	if (x1 >= su->w) { x1 = su->w - 1; } else if (x1 < 0) { x1 = 0; }
	if (x2 >= su->w) { x2 = su->w - 1; } else if (x2 < 0) { x2 = 0; }
	if (x1 > x2) { xtmp = x2; x2 = x1; x1 = xtmp; }
	dx = x2 - x1;

	pDst = (Uint8 *)su->pixels + y*su->pitch + ((x1)<<2);
	pEnd = pDst + (dx<<2);
	while (pDst < pEnd) {
		*(Uint32 *)pDst = c;
		pDst += 4;
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

/* Render an anti-aliased line. */
void
vg_wuline_primitive(struct vg *vg, double x1p, double y1p, double x2p,
    double y2p, int thick, Uint32 color)
{
	double x1 = x1p, y1 = y1p, x2 = x2p, y2 = y2p;
	double grad, xd, yd, length, xm, ym, xgap, ygap, xend, yend, xf, yf,
	    lum1, lum2, ipart;
	int x, y, ix1, ix2, iy1, iy2;
	Uint32 c1, c2;
	Uint8 r, g, b, a;
	int yoffs;
	float focus;

	xd = x2 - x1;
	yd = y2 - y1;

	SDL_GetRGBA(color, vg->fmt, &r, &g, &b, &a);

	if (fabs(xd) > fabs(yd)) {			/* Horizontal */
		if (x1 > x2) {
			double tmp;
			
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			xd = (x2-x1);
			yd = (y2-y1);
		}

		grad = yd/xd;

		/* End point 1 */
		xend = ftrunc(x1+0.5f);
		yend = y1 + grad*(xend-x1);

		xgap = finvfrac(x1+0.5f);

		ix1 = (int)xend;
		iy1 = (int)(yend+0.5f);

		lum1 = finvfrac(yend)*xgap;
		lum2 = ffrac(yend)*xgap;
		BLEND_RGBA2_CLIPPED(vg->su, ix1, iy1, r, g, b,
		    (Uint8)(lum1*255), ALPHA_OVERLAY);
		BLEND_RGBA2_CLIPPED(vg->su, ix1, iy1+1, r, g, b,
		    (Uint8)(lum2*255), ALPHA_OVERLAY);

		yf = yend+grad;

		/* End point 2 */
		xend = ftrunc(x2+0.5f);
		yend = y2 + grad*(xend-x2);

		xgap = finvfrac(x2-0.5f);

		ix2 = (int)xend;
		iy2 = (int)yend;

		lum1 = finvfrac(yend)*xgap;
		lum2 = ffrac(yend)*xgap;

		BLEND_RGBA2_CLIPPED(vg->su, ix2, iy2, r, g, b,
		    (Uint8)(lum1*255), ALPHA_OVERLAY);
		BLEND_RGBA2_CLIPPED(vg->su, ix2, iy2+1, r, g, b,
		    (Uint8)(lum2*255), ALPHA_OVERLAY);

		/* Main loop */
		for (x = (ix1+1); x < ix2; x++) {
			lum1 = finvfrac(yf);
			lum2 = ffrac(yf);

			if (thick == 1) {
				focus = (1.0 - fabs((lum1 - lum2)));
				lum1 += 0.3*focus;
				lum2 += 0.3*focus;
				BLEND_RGBA2_CLIPPED(vg->su, x, (int)yf,
				    r, g, b, (Uint8)(lum1*255), ALPHA_OVERLAY);
				BLEND_RGBA2_CLIPPED(vg->su, x, (int)yf+1,
				    r, g, b, (Uint8)(lum2*255), ALPHA_OVERLAY);
			} else {
				BLEND_RGBA2_CLIPPED(vg->su, x, (int)yf-thick+1,
				    r, g, b, (Uint8)(lum1*255), ALPHA_OVERLAY);
				BLEND_RGBA2_CLIPPED(vg->su, x, (int)yf+thick-1,
				    r, g, b, (Uint8)(lum2*255), ALPHA_OVERLAY);
				for (yoffs = -thick+2; yoffs < thick-1;
				     yoffs++) {
					PUT_PIXEL2_CLIPPED(vg->su, x,
					    (int)yf+yoffs, color);
				}
			}
			yf = yf + grad;
		}
	} else {					/* Vertical */
		if (y1 > y2) {
			double tmp;

			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			xd = (x2-x1);
			yd = (y2-y1);
		}
		grad = xd/yd;

		/* End point 1 */
		yend = ftrunc(y1+0.5f);
		xend = x1 + grad*(yend-y1);

		ygap = finvfrac(y1+0.5f);

		iy1 = (int)yend;
		ix1 = (int)(xend+0.5f);

		lum1 = finvfrac(yend)*ygap;
		lum2 = ffrac(yend)*ygap;
		BLEND_RGBA2_CLIPPED(vg->su, ix1, iy1, r, g, b,
		    (Uint8)(lum1*255), ALPHA_OVERLAY);
		BLEND_RGBA2_CLIPPED(vg->su, ix1, iy1+1, r, g, b,
		    (Uint8)(lum2*255), ALPHA_OVERLAY);
		
		xf = xend + grad;

		/* End point 2 */
		yend = ftrunc(y2+0.5f);
		xend = x2 + grad*(yend-y2);

		xgap = finvfrac(y2-0.5f);

		iy2 = (int)yend;
		ix2 = (int)xend;

		lum1 = finvfrac(yend)*xgap;
		lum2 = ffrac(yend)*xgap;
		BLEND_RGBA2_CLIPPED(vg->su, ix2, iy2, r, g, b,
		    (Uint8)(lum1*255), ALPHA_OVERLAY);
		BLEND_RGBA2_CLIPPED(vg->su, ix2, iy2+1, r, g, b,
		    (Uint8)(lum2*255), ALPHA_OVERLAY);

		/* Main loop */
		for (y = (iy1+1); y < iy2; y++) {
			lum1 = finvfrac(xf);
			lum2 = ffrac(xf);

			if (thick == 1) {
				focus = (1.0 - fabs((lum1 - lum2)));
				lum1 += 0.3*focus;
				lum2 += 0.3*focus;
				BLEND_RGBA2_CLIPPED(vg->su, (int)xf, y,
				    r, g, b, (Uint8)(lum1*255), ALPHA_OVERLAY);
				BLEND_RGBA2_CLIPPED(vg->su, (int)xf+1, y,
				    r, g, b, (Uint8)(lum2*255), ALPHA_OVERLAY);
			} else {
				BLEND_RGBA2_CLIPPED(vg->su, (int)xf-thick+1, y,
				    r, g, b, (Uint8)(lum1*255), ALPHA_OVERLAY);
				BLEND_RGBA2_CLIPPED(vg->su, (int)xf+thick-1, y,
				    r, g, b, (Uint8)(lum2*255), ALPHA_OVERLAY);
				for (yoffs = -thick+2; yoffs < thick-1;
				     yoffs++) {
					PUT_PIXEL2_CLIPPED(vg->su,
					    (int)xf+yoffs, y, color);
				}
			}

			xf = xf + grad;
		}
	}
}

void
vg_rect_primitive(struct vg *vg, int x, int y, int w, int h, Uint32 color)
{
	vg_line_primitive(vg, x, y, x+w, y, color);
	vg_line_primitive(vg, x, y, x, y+h, color);
	vg_line_primitive(vg, x, y+h, x+w, y+h, color);
	vg_line_primitive(vg, x+w, y, x+w, y+h, color);
}
