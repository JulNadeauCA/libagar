/*	$Csoft: vg_primitive.c,v 1.12 2005/06/07 02:42:30 vedge Exp $	*/

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

#include <core/core.h>
#include <core/util.h>

#include <gui/view.h>

#include "vg.h"
#include "vg_math.h"
#include "vg_primitive.h"

#include <string.h>

/* Render a circle using a modified Bresenham line algorithm. */
void
VG_CirclePrimitive(VG *vg, int px, int py, int radius, Uint32 color)
{
	int rx = vg->rDst.x+px;
	int ry = vg->rDst.y+py;
	int v = 2*radius - 1;
	int e = 0, u = 1;
	int x = 0, y = radius;

	while (x < y) {
		VG_PutPixel(vg, rx+x, ry+y, color);
		VG_PutPixel(vg, rx+x, ry-y, color);
		VG_PutPixel(vg, rx-x, ry+y, color);
		VG_PutPixel(vg, rx-x, ry-y, color);

		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;
		
		VG_PutPixel(vg, rx+y, ry+x, color);
		VG_PutPixel(vg, rx+y, ry-x, color);
		VG_PutPixel(vg, rx-y, ry+x, color);
		VG_PutPixel(vg, rx-y, ry-x, color);
	}
	VG_PutPixel(vg, rx-radius, ry, color);
	VG_PutPixel(vg, rx+radius, ry, color);
}

/*
 * Draw a partial ellipse centered around [cx,cy]. Angles a1, a2 are
 * given in degrees. It is assumed that a2 > a1.
 */
void
VG_ArcPrimitive(VG *vg, int cx, int cy, int w, int h, int a1, int a2,
    Uint32 color)
{
	extern int vg_cos_tbl[];
	extern int vg_sin_tbl[];
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
			VG_LinePrimitive(vg, px, py, x, y, color);
		}
		px = x;
		py = y;
	}
}

/* Draw a horizontal line segment. */
void
VG_HLinePrimitive(VG *vg, int px1, int px2, int py, Uint32 c)
{
	SDL_Surface *su = vg->su;
	Uint8 *pDst, *pEnd;
	int x1 = vg->rDst.x+px1;
	int x2 = vg->rDst.x+px2;
	int y = vg->rDst.y+py;
	int dx;

	if (y >= su->clip_rect.y+su->clip_rect.h || y < su->clip_rect.y) {
		return;
	}
	if (x1 > x2) {
		int xTmp = x2;
		x2 = x1;
		x1 = xTmp;
	}
	if (x1 >= su->clip_rect.x+su->clip_rect.w) {
		x1 = su->clip_rect.x + su->clip_rect.w - 1;
	} else if (x1 <= su->clip_rect.x) {
		x1 = su->clip_rect.x + 1;
	}
	if (x2 >= su->clip_rect.x+su->clip_rect.w) {
		x2 = su->clip_rect.x + su->clip_rect.w - 1;
	} else if (x2 <= su->clip_rect.x) {
		x2 = su->clip_rect.x + 1;
	}
	dx = x2 - x1;

	SDL_LockSurface(su);
	switch (su->format->BytesPerPixel) {
	case 4:
		pDst = (Uint8 *)su->pixels + y*su->pitch + (x1<<2);
		pEnd = pDst + (dx<<2);
		while (pDst < pEnd) {
			*(Uint32 *)pDst = c;
			pDst += 4;
		}
		break;
	case 3:
		pDst = (Uint8 *)su->pixels + y*su->pitch + x1*3;
		pEnd = pDst + dx*3;
		while (pDst < pEnd) {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			pDst[0] = (c >>16) & 0xff;
			pDst[1] = (c >>8) & 0xff;
			pDst[2] = c & 0xff;
#else
			pDst[2] = (c>>16) & 0xff;
			pDst[1] = (c>>8) & 0xff;
			pDst[0] = c & 0xff;
#endif
			pDst += 3;
		}
		break;
	case 2:
		pDst = (Uint8 *)su->pixels + y*su->pitch + (x1<<1);
		pEnd = pDst + (dx<<1);
		while (pDst < pEnd) {
			*(Uint16 *)pDst = c;
			pDst += 2;
		}
		break;
	case 1:
		memset(((Uint8 *)su->pixels + y*su->pitch + x1), c, dx);
		break;
	}
	SDL_UnlockSurface(su);
}

/*
 * Draw a line segment between two points using the Bresenham algorithm
 * presented by Foley & Van Dam [1990].
 */
void
VG_LinePrimitive(VG *vg, int px1, int py1, int px2, int py2, Uint32 color)
{
	int x1 = vg->rDst.x+px1;
	int y1 = vg->rDst.y+py1;
	int x2 = vg->rDst.x+px2;
	int y2 = vg->rDst.y+py2;
	int dx, dy;
	int inc1, inc2;
	int d, x, y;
	int xEnd, yEnd;
	int xDir, yDir;

	dx = abs(x2-x1);
	dy = abs(y2-y1);

	if (dy <= dx) {
		d = dy*2 - dx;
		inc1 = dy*2;
		inc2 = (dy-dx)*2;
		if (x1 > x2) {
			x = x2;
			y = y2;
			yDir = -1;
			xEnd = x1;
		} else {
			x = x1;
			y = y1;
			yDir = 1;
			xEnd = x2;
		}
		VG_PutPixel(vg, x, y, color);

		if (((y2-y1)*yDir) > 0) {
			while (x < xEnd) {
				x++;
				if (d < 0) {
					d += inc1;
				} else {
					y++;
					d += inc2;
				}
				VG_PutPixel(vg, x, y, color);
			}
		} else {
			while (x < xEnd) {
				x++;
				if (d < 0) {
					d += inc1;
				} else {
					y--;
					d += inc2;
				}
				VG_PutPixel(vg, x, y, color);
			}
		}		
	} else {
		d = dx*2 - dy;
		inc1 = dx*2;
		inc2 = (dx-dy)*2;
		if (y1 > y2) {
			y = y2;
			x = x2;
			yEnd = y1;
			xDir = -1;
		} else {
			y = y1;
			x = x1;
			yEnd = y2;
			xDir = 1;
		}
		VG_PutPixel(vg, x, y, color);

		if (((x2-x1)*xDir) > 0) {
			while (y < yEnd) {
				y++;
				if (d < 0) {
					d += inc1;
				} else {
					x++;
					d += inc2;
				}
				VG_PutPixel(vg, x, y, color);
			}
		} else {
			while (y < yEnd) {
				y++;
				if (d < 0) {
					d += inc1;
				} else {
					x--;
					d += inc2;
				}
				VG_PutPixel(vg, x, y, color);
			}
		}
	}
}

/* Render an anti-aliased line. */
void
VG_WuLinePrimitive(VG *vg, float x1p, float y1p, float x2p, float y2p,
    int thick, Uint32 color)
{
	float x1 = x1p, y1 = y1p, x2 = x2p, y2 = y2p;
	float grad, xd, yd, xgap, ygap, xEnd, yEnd, xf, yf, lum1, lum2;
	int x, y, ix1, ix2, iy1, iy2;
	Uint8 r, g, b, a;
	int yoffs;
	float focus;

	xd = x2 - x1;
	yd = y2 - y1;

	SDL_GetRGBA(color, vg->fmt, &r, &g, &b, &a);

	if (fabsf(xd) > fabsf(yd)) {			/* Horizontal */
		if (x1 > x2) {
			float tmp;
			
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			xd = (x2-x1);
			yd = (y2-y1);
		}

		grad = yd/xd;

		/* End point 1 */
		xEnd = AG_Truncf(x1+0.5f);
		yEnd = y1 + grad*(xEnd-x1);

		xgap = AG_FracInvf(x1+0.5f);

		ix1 = (int)xEnd;
		iy1 = (int)(yEnd+0.5f);

		lum1 = AG_FracInvf(yEnd)*xgap;
		lum2 = AG_Fracf(yEnd)*xgap;
		AG_BLEND_RGBA2_CLIPPED(vg->su, ix1, iy1, r, g, b,
		    (Uint8)(lum1*255), AG_ALPHA_OVERLAY);
		AG_BLEND_RGBA2_CLIPPED(vg->su, ix1, iy1+1, r, g, b,
		    (Uint8)(lum2*255), AG_ALPHA_OVERLAY);

		yf = yEnd+grad;

		/* End point 2 */
		xEnd = AG_Truncf(x2+0.5f);
		yEnd = y2 + grad*(xEnd-x2);

		xgap = AG_FracInvf(x2-0.5f);

		ix2 = (int)xEnd;
		iy2 = (int)yEnd;

		lum1 = AG_FracInvf(yEnd)*xgap;
		lum2 = AG_Fracf(yEnd)*xgap;

		AG_BLEND_RGBA2_CLIPPED(vg->su, ix2, iy2, r, g, b,
		    (Uint8)(lum1*255), AG_ALPHA_OVERLAY);
		AG_BLEND_RGBA2_CLIPPED(vg->su, ix2, iy2+1, r, g, b,
		    (Uint8)(lum2*255), AG_ALPHA_OVERLAY);

		/* Main loop */
		for (x = (ix1+1); x < ix2; x++) {
			lum1 = AG_FracInvf(yf);
			lum2 = AG_Fracf(yf);

			if (thick == 1) {
				focus = (1.0 - fabsf((lum1 - lum2)));
				lum1 += 0.3*focus;
				lum2 += 0.3*focus;
				AG_BLEND_RGBA2_CLIPPED(vg->su, x, (int)yf,
				    r, g, b, (Uint8)(lum1*255),
				    AG_ALPHA_OVERLAY);
				AG_BLEND_RGBA2_CLIPPED(vg->su, x, (int)yf+1,
				    r, g, b, (Uint8)(lum2*255),
				    AG_ALPHA_OVERLAY);
			} else {
				AG_BLEND_RGBA2_CLIPPED(vg->su, x,
				    (int)yf-thick+1,
				    r, g, b, (Uint8)(lum1*255),
				    AG_ALPHA_OVERLAY);
				AG_BLEND_RGBA2_CLIPPED(vg->su, x,
				    (int)yf+thick-1,
				    r, g, b, (Uint8)(lum2*255),
				    AG_ALPHA_OVERLAY);
				for (yoffs = -thick+2; yoffs < thick-1;
				     yoffs++) {
					AG_PUT_PIXEL2_CLIPPED(vg->su, x,
					    (int)yf+yoffs, color);
				}
			}
			yf = yf + grad;
		}
	} else {					/* Vertical */
		if (y1 > y2) {
			float tmp;

			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			xd = (x2-x1);
			yd = (y2-y1);
		}
		grad = xd/yd;

		/* End point 1 */
		yEnd = AG_Truncf(y1+0.5f);
		xEnd = x1 + grad*(yEnd-y1);

		ygap = AG_FracInvf(y1+0.5f);

		iy1 = (int)yEnd;
		ix1 = (int)(xEnd+0.5f);

		lum1 = AG_FracInvf(yEnd)*ygap;
		lum2 = AG_Fracf(yEnd)*ygap;
		AG_BLEND_RGBA2_CLIPPED(vg->su, ix1, iy1, r, g, b,
		    (Uint8)(lum1*255), AG_ALPHA_OVERLAY);
		AG_BLEND_RGBA2_CLIPPED(vg->su, ix1, iy1+1, r, g, b,
		    (Uint8)(lum2*255), AG_ALPHA_OVERLAY);
		
		xf = xEnd + grad;

		/* End point 2 */
		yEnd = AG_Truncf(y2+0.5f);
		xEnd = x2 + grad*(yEnd-y2);

		xgap = AG_FracInvf(y2-0.5f);

		iy2 = (int)yEnd;
		ix2 = (int)xEnd;

		lum1 = AG_FracInvf(yEnd)*xgap;
		lum2 = AG_Fracf(yEnd)*xgap;
		AG_BLEND_RGBA2_CLIPPED(vg->su, ix2, iy2, r, g, b,
		    (Uint8)(lum1*255), AG_ALPHA_OVERLAY);
		AG_BLEND_RGBA2_CLIPPED(vg->su, ix2, iy2+1, r, g, b,
		    (Uint8)(lum2*255), AG_ALPHA_OVERLAY);

		/* Main loop */
		for (y = (iy1+1); y < iy2; y++) {
			lum1 = AG_FracInvf(xf);
			lum2 = AG_Fracf(xf);

			if (thick == 1) {
				focus = (1.0 - fabsf((lum1 - lum2)));
				lum1 += 0.3*focus;
				lum2 += 0.3*focus;
				AG_BLEND_RGBA2_CLIPPED(vg->su, (int)xf, y,
				    r, g, b, (Uint8)(lum1*255),
				    AG_ALPHA_OVERLAY);
				AG_BLEND_RGBA2_CLIPPED(vg->su, (int)xf+1, y,
				    r, g, b, (Uint8)(lum2*255),
				    AG_ALPHA_OVERLAY);
			} else {
				AG_BLEND_RGBA2_CLIPPED(vg->su,
				    (int)xf-thick+1, y,
				    r, g, b, (Uint8)(lum1*255),
				    AG_ALPHA_OVERLAY);
				AG_BLEND_RGBA2_CLIPPED(vg->su,
				    (int)xf+thick-1, y,
				    r, g, b, (Uint8)(lum2*255),
				    AG_ALPHA_OVERLAY);
				for (yoffs = -thick+2; yoffs < thick-1;
				     yoffs++) {
					AG_PUT_PIXEL2_CLIPPED(vg->su,
					    (int)xf+yoffs, y, color);
				}
			}

			xf = xf + grad;
		}
	}
}

void
VG_RectPrimitive(VG *vg, int x, int y, int w, int h, Uint32 color)
{
	VG_HLinePrimitive(vg, x, x+w, y, color);
	VG_LinePrimitive(vg, x, y, x, y+h, color);
	VG_HLinePrimitive(vg, x, x+w, y+h, color);
	VG_LinePrimitive(vg, x+w, y, x+w, y+h, color);
}
