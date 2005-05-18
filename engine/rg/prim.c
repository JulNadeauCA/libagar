/*	$Csoft: prim.c,v 1.6 2005/04/10 09:09:02 vedge Exp $	*/

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

#include "tileset.h"

/* Obtain the hue/saturation/value of a given RGB triplet. */
void
prim_rgb2hsv(Uint8 r, Uint8 g, Uint8 b, float *h, float *s, float *v)
{
	float vR, vG, vB;
	float vMin, vMax, deltaMax;
	float deltaR, deltaG, deltaB;

	vR = (float)r/255.0;
	vG = (float)g/255.0;
	vB = (float)b/255.0;

	vMin = MIN3(vR, vG, vB);
	vMax = MAX3(vR, vG, vB);
	deltaMax = vMax - vMin;
	*v = vMax;
	
	if (deltaMax == 0.0) {
		/* This is a gray color (zero hue, no saturation). */
		*h = 0.0;
		*s = 0.0;
	} else {
		*s = deltaMax / vMax;
		deltaR = ((vMax - vR)/6.0 + deltaMax/2.0) / deltaMax;
		deltaG = ((vMax - vG)/6.0 + deltaMax/2.0) / deltaMax;
		deltaB = ((vMax - vB)/6.0 + deltaMax/2.0) / deltaMax;

		if (vR == vMax) {
			*h = (deltaB - deltaG)*360.0;
		} else if (vG == vMax) {
			*h = 120.0 + (deltaR - deltaB)*360.0;	/* 1/3 */
		} else if (vB == vMax) {
			*h = 240.0 + (deltaG - deltaR)*360.0;	/* 2/3 */
		}

		if (*h < 0.0)	(*h)++;
		if (*h > 360.0)	(*h)--;
	}
}

/* Convert hue/saturation/value to RGB. */
void
prim_hsv2rgb(float h, float s, float v, Uint8 *r, Uint8 *g, Uint8 *b)
{
	float var[3];
	float vR, vG, vB, hv;
	int iv;

	if (s == 0.0) {
		*r = (Uint8)v*255;
		*g = (Uint8)v*255;
		*b = (Uint8)v*255;
		return;
	}
	
	hv = h/60.0;
	iv = floorf(hv);
	var[0] = v * (1 - s);
	var[1] = v * (1 - s*(hv - iv));
	var[2] = v * (1 - s*(1 - (hv - iv)));

	switch (iv) {
	case 0:
		vR = v;
		vG = var[2];
		vB = var[0];
		break;
	case 1:
		vR = var[1];
		vG = v;
		vB = var[0];
		break;
	case 2:
		vR = var[0];
		vG = v;
		vB = var[2];
		break;
	case 3:
		vR = var[0];
		vG = var[1];
		vB = v;
		break;
	case 4:
		vR = var[2];
		vG = var[0];
		vB = v;
		break;
	default:
		vR = v;
		vG = var[0];
		vB = var[1];
		break;
	}
	
	*r = vR*255;
	*g = vG*255;
	*b = vB*255;
}

void
prim_color_rgb(struct tile *t, Uint8 r, Uint8 g, Uint8 b)
{
	t->c.r = r;
	t->c.g = g;
	t->c.b = b;
	t->pc = SDL_MapRGB(t->su->format, r, g, b);
}

void
prim_color_rgba(struct tile *t, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	t->c.r = r;
	t->c.g = g;
	t->c.b = b;
	t->pc = SDL_MapRGBA(t->su->format, r, g, b, a);
}

void
prim_color_hsv(struct tile *t, float h, float s, float v)
{
	prim_hsv2rgb(h, s, v, &t->c.r, &t->c.g, &t->c.b);
	t->pc = SDL_MapRGB(t->su->format, t->c.r, t->c.g, t->c.b);
}

void
prim_color_hsva(struct tile *t, float h, float s, float v, Uint8 a)
{
	prim_hsv2rgb(h, s, v, &t->c.r, &t->c.g, &t->c.b);
	t->pc = SDL_MapRGBA(t->su->format, t->c.r, t->c.g, t->c.b, a);
}

void
prim_color_u32(struct tile *t, Uint32 pc)
{
	SDL_GetRGB(pc, t->su->format,
	    &t->c.r,
	    &t->c.g,
	    &t->c.b);
	t->pc = pc;
}

void
prim_put_pixel(SDL_Surface *su, int x, int y, Uint32 pc)
{
	Uint8 *dst;
	    
	if (x < 0 || y < 0 || x >= su->w || y >= su->h)
		return;
#ifdef DEBUG
	if (su->format->BitsPerPixel != 32)
		fatal("surface != 32bpp");
#endif
	dst = (Uint8 *)su->pixels + y*su->pitch + (x << 2);
	*(Uint32 *)dst = pc;
}

/* Blend the pixel at t:[x,y] with the given RGBA value. */
void
prim_blend_rgb(SDL_Surface *su, int x, int y, enum prim_blend_mode mode,
    Uint8 sR, Uint8 sG, Uint8 sB, Uint8 sA)
{
	Uint8 dR, dG, dB, dA;
	Uint8 *pDst;
	int alpha;

	if (x < 0 || y < 0 ||
	    x >= su->w || y >= su->h)
		return;
#ifdef DEBUG
	if (su->format->BitsPerPixel != 32)
		fatal("surface != 32bpp");
#endif
	pDst = (Uint8 *)su->pixels + y*su->pitch + (x << 2);
	if (*(Uint32 *)pDst != su->format->colorkey) {
		SDL_GetRGBA(*(Uint32 *)pDst, su->format, &dR, &dG, &dB, &dA);

		switch (mode) {
		case PRIM_OVERLAY_ALPHA:
			alpha = dA + sA;
			if (alpha > 255) {
				alpha = 255;
			}
			*(Uint32 *)pDst = SDL_MapRGBA(su->format,
			    (((sR - dR) * sA) >> 8) + dR,
			    (((sG - dG) * sA) >> 8) + dG,
			    (((sB - dB) * sA) >> 8) + dB,
			    (Uint8)alpha);
			break;
		case PRIM_AVERAGE_ALPHA:
			*(Uint32 *)pDst = SDL_MapRGBA(su->format,
			    (((sR - dR) * sA) >> 8) + dR,
			    (((sG - dG) * sA) >> 8) + dG,
			    (((sB - dB) * sA) >> 8) + dB,
			    (Uint8)((dA*sA)/2));
			break;
		case PRIM_SRC_ALPHA:
			*(Uint32 *)pDst = SDL_MapRGBA(su->format,
			    (((sR - dR) * sA) >> 8) + dR,
			    (((sG - dG) * sA) >> 8) + dG,
			    (((sB - dB) * sA) >> 8) + dB,
			    (Uint8)(sA));
			break;
		case PRIM_DST_ALPHA:
			*(Uint32 *)pDst = SDL_MapRGBA(su->format,
			    (((sR - dR) * sA) >> 8) + dR,
			    (((sG - dG) * sA) >> 8) + dG,
			    (((sB - dB) * sA) >> 8) + dB,
			    dA);
			break;
		}
	} else {
		*(Uint32 *)pDst = SDL_MapRGBA(su->format, sR, sG, sB, sA);
	}
}

/*
 * Draw a line segment between two points using the Bresenham algorithm
 * as presented by Foley & Van Dam [1990].
 */
void
prim_line(struct tile *t, int x1, int y1, int x2, int y2)
{
	SDL_Surface *su = t->su;
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
		prim_put_pixel(t->su, x, y, t->pc);

		if (((y2-y1)*ydir) > 0) {
			while (x < xend) {
				x++;
				if (d < 0) {
					d += inc1;
				} else {
					y++;
					d += inc2;
				}
				prim_put_pixel(t->su, x, y, t->pc);
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
				prim_put_pixel(t->su, x, y, t->pc);
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
		prim_put_pixel(t->su, x, y, t->pc);

		if (((x2-x1)*xdir) > 0) {
			while (y < yend) {
				y++;
				if (d < 0) {
					d += inc1;
				} else {
					x++;
					d += inc2;
				}
				prim_put_pixel(t->su, x, y, t->pc);
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
				prim_put_pixel(t->su, x, y, t->pc);
			}
		}
	}
}

static __inline__ int
ftrunc(double d)
{
	return ((int)floor(d));
}

static __inline__ double
ffrac(double d)
{
	return (d - floor(d));
}

static __inline__ double
finvfrac(double d)
{
	return (1 - (d - floor(d)));
}

void
prim_wuline(struct tile *t, double x1, double y1, double x2, double y2)
{
	double grad, xd, yd, length, xm, ym, xgap, ygap, xend, yend, xf, yf,
	    lum1, lum2, ipart;
	int x, y, ix1, ix2, iy1, iy2;
	Uint32 c1, c2;
	Uint8 r, g, b, a;

	xd = x2 - x1;
	yd = y2 - y1;

	SDL_GetRGBA(t->pc, t->ts->fmt, &r, &g, &b, &a);

	if (abs(xd) > abs(yd)) {			/* Horizontal */
		if (x1 > x2) {
			fix6 tmp;
			
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			xd = (x2-x1);
			yd = (y2-y1);
		}

		grad = yd/xd;

		/* End point 1 */
		xend = trunc(x1+0.5);
		yend = y1 + grad*(xend-x1);

		xgap = finvfrac(x1+0.5);

		ix1 = (int)xend;
		iy1 = (int)yend;

		lum1 = finvfrac(yend)*xgap;
		lum2 = ffrac(yend)*xgap;
		prim_blend_rgb(t->su, ix1, iy1, PRIM_OVERLAY_ALPHA, r, g, b,
		    (Uint8)(lum1*255));
		prim_blend_rgb(t->su, ix1, iy1+1, PRIM_OVERLAY_ALPHA, r, g, b,
		    (Uint8)(lum2*255));

		yf = yend+grad;

		/* End point 2 */
		xend = trunc(x2+0.5);
		yend = y2 + grad*(xend-x2);

		xgap = finvfrac(x2-0.5);

		ix2 = (int)xend;
		iy2 = (int)yend;

		lum1 = finvfrac(yend)*xgap;
		lum2 = ffrac(yend)*xgap;
		prim_blend_rgb(t->su, ix2, iy2, PRIM_OVERLAY_ALPHA, r, g, b,
		    (Uint8)(lum1*255));
		prim_blend_rgb(t->su, ix2, iy2+1, PRIM_OVERLAY_ALPHA, r, g, b,
		    (Uint8)(lum2*255));

		/* Main loop */
		for (x = (ix1+1); x < ix2; x++) {
			lum1 = finvfrac(yf);
			lum2 = ffrac(yf);
			prim_blend_rgb(t->su, x, (int)yf, PRIM_OVERLAY_ALPHA,
			    r, g, b, (Uint8)(lum1*255));
			prim_blend_rgb(t->su, x, (int)yf+1, PRIM_OVERLAY_ALPHA,
			    r, g, b, (Uint8)(lum2*255));

			yf = yf + grad;
		}
	} else {					/* Vertical */
		if (x1 > x2) {
			double tmp;

			tmp = x1; x1 = x2; x2 = x1;
			tmp = y1; y1 = y2; y2 = y1;
			xd = (x2 - x1);
			yd = (y2 - y1);
		}
		grad = xd/yd;

		/* End point 1 */
		xend = trunc(x1+0.5);
		yend = y1 + grad*(xend-x1);

		xgap = ffrac(x1+0.5);

		ix1 = (int)xend;
		iy1 = (int)yend;

		lum1 = finvfrac(yend)*xgap;
		lum2 = ffrac(yend)*xgap;
		prim_blend_rgb(t->su, ix1, iy1, PRIM_OVERLAY_ALPHA,
		    r, g, b, (Uint8)(lum1*255));
		prim_blend_rgb(t->su, ix1, iy1+1, PRIM_OVERLAY_ALPHA,
		    r, g, b, (Uint8)(lum2*255));
		
		xf = xend + grad;

		/* End point 2 */
		xend = trunc(x2+0.5);
		yend = y2 + grad*(xend-x2);

		xgap = finvfrac(x2-0.5);

		ix2 = (int)xend;
		iy2 = (int)yend;

		lum1 = finvfrac(yend)*xgap;
		lum2 = ffrac(yend)*xgap;
		prim_blend_rgb(t->su, ix2, iy2, PRIM_OVERLAY_ALPHA,
		    r, g, b, (Uint8)(lum1*255));
		prim_blend_rgb(t->su, ix2, iy2+1, PRIM_OVERLAY_ALPHA,
		    r, g, b, (Uint8)(lum2*255));

		/* Main loop */
		for (y = (iy1+1); y < iy2; y++) {
			lum1 = finvfrac(xf);
			lum2 = ffrac(xf);
			prim_blend_rgb(t->su, (int)xf, y, PRIM_OVERLAY_ALPHA,
			    r, g, b, (Uint8)(lum1*255));
			prim_blend_rgb(t->su, (int)xf+1, y, PRIM_OVERLAY_ALPHA,
			    r, g, b, (Uint8)(lum2*255));

			xf = xf + grad;
		}
	}
}

void
prim_circle2(struct tile *t, int wx, int wy, int radius)
{
	int v = 2*radius - 1;
	int e = 0, u = 1;
	int x = 0, y = radius;

	while (x < y) {
		prim_put_pixel(t->su, wx+x, wy+y, t->pc);
		prim_put_pixel(t->su, wx+x+1, wy+y, t->pc);
		prim_put_pixel(t->su, wx+x, wy-y, t->pc);
		prim_put_pixel(t->su, wx+x+1, wy-y, t->pc);
		prim_put_pixel(t->su, wx-x, wy+y, t->pc);
		prim_put_pixel(t->su, wx-x-1, wy+y, t->pc);
		prim_put_pixel(t->su, wx-x, wy-y, t->pc);
		prim_put_pixel(t->su, wx-x-1, wy-y, t->pc);

		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;
		
		prim_put_pixel(t->su, wx+y, wy+x, t->pc);
		prim_put_pixel(t->su, wx+y+1, wy+x, t->pc);
		prim_put_pixel(t->su, wx+y, wy-x, t->pc);
		prim_put_pixel(t->su, wx+y+1, wy-x, t->pc);
		prim_put_pixel(t->su, wx-y, wy+x, t->pc);
		prim_put_pixel(t->su, wx-y-1, wy+x, t->pc);
		prim_put_pixel(t->su, wx-y, wy-x, t->pc);
		prim_put_pixel(t->su, wx-y-1, wy-x, t->pc);
	}
	prim_put_pixel(t->su, wx-radius, wy, t->pc);
	prim_put_pixel(t->su, wx+radius, wy, t->pc);
}
