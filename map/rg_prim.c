/*
 * Copyright (c) 2005-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>
#include <agar/gui/widget.h>

#include <agar/map/rg_tileset.h>
#include <agar/map/rg_math.h>

#include <math.h>
#include <stdlib.h>

void
RG_ColorRGB(RG_Tile *t, const AG_Color *c)
{
	t->c.r = c->r;
	t->c.g = c->g;
	t->c.b = c->b;
	t->pc = AG_MapPixel32(&t->su->format, c);
}

void
RG_ColorRGBA(RG_Tile *t, const AG_Color *c)
{
	t->c.r = c->r;
	t->c.g = c->g;
	t->c.b = c->b;
	t->pc = AG_MapPixel32(&t->su->format, c);
}

void
RG_ColorHSV(RG_Tile *t, float h, float s, float v)
{
	AG_MapHSVf_RGB(h, s, v,
	   &t->c.r,
	   &t->c.g,
	   &t->c.b);

	t->pc = AG_MapPixel32_RGB8(&t->su->format,
	   t->c.r,
	   t->c.g,
	   t->c.b);
}

void
RG_ColorHSVA(RG_Tile *t, float h, float s, float v, Uint8 a)
{
	AG_MapHSVf_RGB(h, s, v,
	   &t->c.r,
	   &t->c.g,
	   &t->c.b);

	t->pc = AG_MapPixel32_RGBA8(&t->su->format,
	    t->c.r,
	    t->c.g,
	    t->c.b, a);
}

void
RG_ColorUint32(RG_Tile *t, Uint32 pc)
{
	AG_GetColor32(&t->c, pc, &t->su->format);
	t->c.a = AG_OPAQUE;
	t->pc = pc;
}

/* Blend the pixel at t:[x,y] with the given RGBA value. */
void
RG_BlendRGB(AG_Surface *S, int x, int y, enum rg_prim_blend_mode mode,
    const AG_Color *c)
{
	AG_Color Cdst;
	AG_Component r,g,b,a;
	Uint8 *pDst;
	int alpha;

	if (x < 0 || y < 0 || x >= S->w || y >= S->h) {
		return;
	}
	r = c->r;
	g = c->g;
	b = c->b;
	a = c->a;
	pDst = S->pixels + y*S->pitch + (x << 2);
	if (*(Uint32 *)pDst != S->colorkey) {
		AG_GetColor32(&Cdst, *(Uint32 *)pDst, &S->format);

		switch (mode) {
		case RG_PRIM_OVERLAY_ALPHA:
			alpha = Cdst.a + a;
			if (alpha > 255) {
				alpha = 255;
			}
			*(Uint32 *)pDst = AG_MapPixel32_RGBA8(&S->format,
			    (((r - Cdst.r) * a) >> 8) + Cdst.r,
			    (((g - Cdst.g) * a) >> 8) + Cdst.g,
			    (((b - Cdst.b) * a) >> 8) + Cdst.b,
			    (Uint8)alpha);
			break;
		case RG_PRIM_AVERAGE_ALPHA:
			*(Uint32 *)pDst = AG_MapPixel32_RGBA8(&S->format,
			    (((r - Cdst.r) * a) >> 8) + Cdst.r,
			    (((g - Cdst.g) * a) >> 8) + Cdst.g,
			    (((b - Cdst.b) * a) >> 8) + Cdst.b,
			    (Uint8)((Cdst.a * a) >> 1));
			break;
		case RG_PRIM_SRC_ALPHA:
			*(Uint32 *)pDst = AG_MapPixel32_RGBA8(&S->format,
			    (((r - Cdst.r) * a) >> 8) + Cdst.r,
			    (((g - Cdst.g) * a) >> 8) + Cdst.g,
			    (((b - Cdst.b) * a) >> 8) + Cdst.b,
			    (Uint8)(a));
			break;
		case RG_PRIM_DST_ALPHA:
			*(Uint32 *)pDst = AG_MapPixel32_RGBA8(&S->format,
			    (((r - Cdst.r) * a) >> 8) + Cdst.r,
			    (((g - Cdst.g) * a) >> 8) + Cdst.g,
			    (((b - Cdst.b) * a) >> 8) + Cdst.b,
			    Cdst.a);
			break;
		}
	} else {
		*(Uint32 *)pDst = AG_MapPixel32(&S->format, c);
	}
}

/*
 * Draw a line segment between two points using the Bresenham algorithm
 * as presented by Foley & Van Dam [1990].
 */
void
RG_Line(RG_Tile *t, int x1, int y1, int x2, int y2)
{
	AG_Surface *S = t->su;
	Uint32 pc = t->pc;
	int dx, dy;
	int inc1, inc2;
	int d, x,y;
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
		RG_PutPixel(S, x,y, pc);

		if (((y2-y1)*ydir) > 0) {
			while (x < xend) {
				x++;
				if (d < 0) {
					d += inc1;
				} else {
					y++;
					d += inc2;
				}
				RG_PutPixel(S, x,y, pc);
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
				RG_PutPixel(S, x,y, pc);
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
		RG_PutPixel(S, x,y, pc);

		if (((x2-x1)*xdir) > 0) {
			while (y < yend) {
				y++;
				if (d < 0) {
					d += inc1;
				} else {
					x++;
					d += inc2;
				}
				RG_PutPixel(S, x,y, pc);
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
				RG_PutPixel(S, x,y, pc);
			}
		}
	}
}

/* Draw a horizontal line segment. */
void
RG_HLine(RG_Tile *t, int x1, int x2, int y, Uint32 c)
{
	AG_Surface *S = t->su;
	int xTmp;
	Uint8 *pDst, *pEnd;
	int dx;

	if (y >= S->h || y < 0) { return; }
	if (x1 >= S->w) { x1 = S->w - 1; } else if (x1 < 0) { x1 = 0; }
	if (x2 >= S->w) { x2 = S->w - 1; } else if (x2 < 0) { x2 = 0; }
	if (x1 > x2) { xTmp = x2; x2 = x1; x1 = xTmp; }
	dx = x2 - x1;

	pDst = S->pixels + y*S->pitch + (x1 << 2);
	pEnd = pDst + (dx << 2);
	while (pDst < pEnd) {
		*(Uint32 *)pDst = c;
		pDst += 4;
	}
}

/* Draw an antialiased line using the Wu-line algorithm. */
void
RG_WuLine(RG_Tile *t, float x1p, float y1p, float x2p, float y2p)
{
	AG_Surface *S = t->su;
	float x1 = x1p, y1 = y1p, x2 = x2p, y2 = y2p;
	float grad, xd, yd, xgap, xend, yend, xf, yf, lum1, lum2;
	int x, y, ix1, ix2, iy1, iy2;
	AG_Color c;

	xd = x2 - x1;
	yd = y2 - y1;

	AG_GetColor32(&c, t->pc, t->ts->fmt);

	if (Fabs(xd) > Fabs(yd)) {			/* Horizontal */
		if (x1 > x2) {
			float tmp;
			
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			xd = (x2-x1);
			yd = (y2-y1);
		}

		grad = yd/xd;

		/* End point 1 */
		xend = Truncf(x1+0.5);
		yend = y1 + grad*(xend-x1);

		xgap = FracInvf(x1+0.5);

		ix1 = (int)xend;
		iy1 = (int)yend;

		lum1 = FracInvf(yend)*xgap;
		lum2 = Fracf(yend)*xgap;
		c.a = (Uint8)(lum1*255);
		RG_BlendRGB(S, ix1, iy1, RG_PRIM_OVERLAY_ALPHA, &c);
		c.a = (Uint8)(lum2*255);
		RG_BlendRGB(S, ix1, iy1+1, RG_PRIM_OVERLAY_ALPHA, &c);

		yf = yend+grad;

		/* End point 2 */
		xend = Truncf(x2+0.5);
		yend = y2 + grad*(xend-x2);

		xgap = FracInvf(x2-0.5);

		ix2 = (int)xend;
		iy2 = (int)yend;

		lum1 = FracInvf(yend)*xgap;
		lum2 = Fracf(yend)*xgap;
		c.a = (Uint8)(lum1*255);
		RG_BlendRGB(S, ix2, iy2, RG_PRIM_OVERLAY_ALPHA, &c);
		c.a = (Uint8)(lum2*255);
		RG_BlendRGB(S, ix2, iy2+1, RG_PRIM_OVERLAY_ALPHA, &c);

		/* Main loop */
		for (x = (ix1+1); x < ix2; x++) {
			float focus;

			lum1 = FracInvf(yf);
			lum2 = Fracf(yf);
			focus = (1.0 - Fabs(lum1-lum2));
			lum1 += 0.3*focus;
			lum2 += 0.3*focus;

			c.a = (Uint8)(lum1*255);
			RG_BlendRGB(S, x, (int)yf, RG_PRIM_OVERLAY_ALPHA, &c);
			c.a = (Uint8)(lum2*255);
			RG_BlendRGB(S, x, (int)yf+1, RG_PRIM_OVERLAY_ALPHA, &c);

			yf = yf + grad;
		}
	} else {					/* Vertical */
		if (x1 > x2) {
			float tmp;

			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			xd = (x2 - x1);
			yd = (y2 - y1);
		}
		grad = xd/yd;

		/* End point 1 */
		xend = Truncf(x1+0.5);
		yend = y1 + grad*(xend-x1);

		xgap = Fracf(x1+0.5);

		ix1 = (int)xend;
		iy1 = (int)yend;

		lum1 = FracInvf(yend)*xgap;
		lum2 = Fracf(yend)*xgap;

		c.a = (Uint8)(lum1*255);
		RG_BlendRGB(S, ix1, iy1, RG_PRIM_OVERLAY_ALPHA, &c);
		c.a = (Uint8)(lum2*255);
		RG_BlendRGB(S, ix1, iy1+1, RG_PRIM_OVERLAY_ALPHA, &c);
		
		xf = xend + grad;

		/* End point 2 */
		xend = Truncf(x2+0.5);
		yend = y2 + grad*(xend-x2);

		xgap = FracInvf(x2-0.5);

		ix2 = (int)xend;
		iy2 = (int)yend;

		lum1 = FracInvf(yend)*xgap;
		lum2 = Fracf(yend)*xgap;
		c.a = (Uint8)(lum1*255);
		RG_BlendRGB(S, ix2, iy2, RG_PRIM_OVERLAY_ALPHA, &c);
		c.a = (Uint8)(lum2*255);
		RG_BlendRGB(S, ix2, iy2+1, RG_PRIM_OVERLAY_ALPHA, &c);

		/* Main loop */
		for (y = (iy1+1); y < iy2; y++) {
			float focus;

			lum1 = FracInvf(xf);
			lum2 = Fracf(xf);
			focus = (1.0 - Fabs(lum1-lum2));
			lum1 += 0.3*focus;
			lum2 += 0.3*focus;
			c.a = (Uint8)(lum1*255);
			RG_BlendRGB(S, (int)xf, y, RG_PRIM_OVERLAY_ALPHA, &c);
			c.a = (Uint8)(lum2*255);
			RG_BlendRGB(S, (int)xf+1, y, RG_PRIM_OVERLAY_ALPHA, &c);

			xf = xf + grad;
		}
	}
}

/* Draw a circle. */
void
RG_Circle2(RG_Tile *t, int wx, int wy, int radius)
{
	AG_Surface *S = t->su;
	Uint32 pc = t->pc;
	int v = (radius << 1) - 1;
	int e = 0, u = 1;
	int x = 0, y = radius;

	while (x < y) {
		RG_PutPixel(S, wx+x,   wy+y, pc);
		RG_PutPixel(S, wx+x+1, wy+y, pc);
		RG_PutPixel(S, wx+x,   wy-y, pc);
		RG_PutPixel(S, wx+x+1, wy-y, pc);
		RG_PutPixel(S, wx-x,   wy+y, pc);
		RG_PutPixel(S, wx-x-1, wy+y, pc);
		RG_PutPixel(S, wx-x,   wy-y, pc);
		RG_PutPixel(S, wx-x-1, wy-y, pc);

		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;
		
		RG_PutPixel(S, wx+y,   wy+x, pc);
		RG_PutPixel(S, wx+y+1, wy+x, pc);
		RG_PutPixel(S, wx+y,   wy-x, pc);
		RG_PutPixel(S, wx+y+1, wy-x, pc);
		RG_PutPixel(S, wx-y,   wy+x, pc);
		RG_PutPixel(S, wx-y-1, wy+x, pc);
		RG_PutPixel(S, wx-y,   wy-x, pc);
		RG_PutPixel(S, wx-y-1, wy-x, pc);
	}
	RG_PutPixel(S, wx-radius, wy, pc);
	RG_PutPixel(S, wx+radius, wy, pc);
}
