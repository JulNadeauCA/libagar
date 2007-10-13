/*
 * Copyright (c) 2002-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
#include <core/view.h>

#include "widget.h"
#include "window.h"
#include "primitive.h"

AG_PrimitiveOps agPrim;

/* Add to individual RGB components of a pixel. */
/* TODO use SIMD to compute the components in parallel */
static __inline__ Uint32
ColorShift(Uint32 pixel, Sint8 *shift)
{
	Sint8 r = shift[0];
	Sint8 g = shift[1];
	Sint8 b = shift[2];
	Uint32 rv = 0;
	int v1, v2;

	v1 = ((pixel & agVideoFmt->Rmask) >> agVideoFmt->Rshift);
	v2 = ((v1 << agVideoFmt->Rloss) + (v1 >> (8 - agVideoFmt->Rloss))) + r;
	if (v2 < 0) {
		v2 = 0;
	} else if (v2 > 255) {
		v2 = 255;
	}
	rv |= (v2 >> agVideoFmt->Rloss) << agVideoFmt->Rshift;

	v1 = ((pixel & agVideoFmt->Gmask) >> agVideoFmt->Gshift);
	v2 = ((v1 << agVideoFmt->Gloss) + (v1 >> (8 - agVideoFmt->Gloss))) + g;
	if (v2 < 0) {
		v2 = 0;
	} else if (v2 > 255) {
		v2 = 255;
	}
	rv |= (v2 >> agVideoFmt->Gloss) << agVideoFmt->Gshift;

	v1 = ((pixel & agVideoFmt->Bmask) >> agVideoFmt->Bshift);
	v2 = ((v1 << agVideoFmt->Bloss) + (v1 >> (8 - agVideoFmt->Bloss))) + b;
	if (v2 < 0) {
		v2 = 0;
	} else if (v2 > 255) {
		v2 = 255;
	}
	rv |= (v2 >> agVideoFmt->Bloss) << agVideoFmt->Bshift;

	rv |= agVideoFmt->Amask;
	return (rv);
}

static void
arrow_up(void *p, int x0, int y0, int h, Uint32 c1, Uint32 c2)
{
	AG_Widget *wid = p;
	int y1 = wid->cy+y0 - (h>>1);
	int y2 = y1+h-1;
	int xs = wid->cx+x0, xe = xs;
	int x, y;

	SDL_LockSurface(agView->v);
	for (y = y1; y < y2; y+=2) {
		for (x = xs; x <= xe; x++) {
			AG_VIEW_PUT_PIXEL2_CLIPPED(x, y,
			    (x == xs || x == xe) ? c2 : c1);
			AG_VIEW_PUT_PIXEL2_CLIPPED(x, y+1, c1);
		}
		xs--;
		xe++;
	}
	SDL_UnlockSurface(agView->v);
}

static void
arrow_down(void *p, int x0, int y0, int h, Uint32 c1, Uint32 c2)
{
	AG_Widget *wid = p;
	int y1 = wid->cy+y0 - (h>>1);
	int y2 = y1+h-1;
	int xs = wid->cx+x0, xe = xs;
	int x, y;

	SDL_LockSurface(agView->v);
	for (y = y2; y > y1; y-=2) {
		for (x = xs; x <= xe; x++) {
			AG_VIEW_PUT_PIXEL2_CLIPPED(x, y,
			    (x == xs || x == xe) ? c2 : c1);
			AG_VIEW_PUT_PIXEL2_CLIPPED(x, y-1, c1);
		}
		xs--;
		xe++;
	}
	SDL_UnlockSurface(agView->v);
}

static void
arrow_left(void *p, int x0, int y0, int h, Uint32 c1, Uint32 c2)
{
	AG_Widget *wid = p;
	int x1 = wid->cx+x0 - (h>>1);
	int x2 = x1+h-1;
	int ys = wid->cy+y0, ye = ys;
	int x, y;

	SDL_LockSurface(agView->v);
	for (x = x1; x < x2; x+=2) {
		for (y = ys; y <= ye; y++) {
			AG_VIEW_PUT_PIXEL2_CLIPPED(x+1, y, c1);
			AG_VIEW_PUT_PIXEL2_CLIPPED(x, y,
			    (y == ys || y == ye) ? c2 : c1);
		}
		ys--;
		ye++;
	}
	SDL_UnlockSurface(agView->v);
}

static void
arrow_right(void *p, int x0, int y0, int h, Uint32 c1, Uint32 c2)
{
	AG_Widget *wid = p;
	int x1 = wid->cx+x0 - (h>>1);
	int x2 = x1+h-1;
	int ys = wid->cy+y0, ye = ys;
	int x, y;

	SDL_LockSurface(agView->v);
	for (x = x2; x > x1; x-=2) {
		for (y = ys; y <= ye; y++) {
			AG_VIEW_PUT_PIXEL2_CLIPPED(x-1, y, c1);
			AG_VIEW_PUT_PIXEL2_CLIPPED(x, y,
			    (y == ys || y == ye) ? c2 : c1);
		}
		ys--;
		ye++;
	}
	SDL_UnlockSurface(agView->v);
}

/* Draw a 3D-style box. */
static void
box(void *p, int xoffs, int yoffs, int w, int h, int z, Uint32 c)
{
	AG_Widget *wid = p;
	Uint32 cBg;

	if (AG_WidgetFocused(wid)) {
		cBg = ColorShift(c, (z<0) ? agFocusSunkColorShift :
		                            agFocusRaisedColorShift);
	} else {
		cBg = ColorShift(c, (z<0) ? agNofocusSunkColorShift :
		                            agNofocusRaisedColorShift);
	}
	agPrim.rect_filled(wid, xoffs, yoffs, w, h, cBg);
	agPrim.frame(wid, xoffs, yoffs, w, h, z, c);
}

/* Draw a 3D-style box with dithering. */
static void
box_dithered(void *p, int xoffs, int yoffs, int w, int h, int z,
    Uint32 c1, Uint32 c2)
{
	AG_Widget *wid = p;
	Uint x, y;
	int flag = 0;
	Uint32 cDither;
	
	if (AG_WidgetFocused(wid)) {
		cDither = ColorShift(c2, (z<0) ? agFocusSunkColorShift :
		                                 agFocusRaisedColorShift);
	} else {
		cDither = ColorShift(c2, (z<0) ? agNofocusSunkColorShift :
		                                 agNofocusRaisedColorShift);
	}

	/* XXX inefficient */
	agPrim.box(p, xoffs, yoffs, w, h, z, c1);
	for (y = yoffs; y < yoffs+h-2; y++) {
		flag = !flag;
		for (x = xoffs+1+flag; x < xoffs+w-2; x+=2)
			AG_WidgetPutPixel(wid, x, y, cDither);
	}
}

/* Draw a 3D-style box with chamfered top edges. */
static void
box_chamfered(void *p, SDL_Rect *r, int z, int rad, Uint32 cBg)
{
	AG_Widget *wid = p;
	Uint32 cLeft, cRight;
	int v, e, u;
	int x, y;
	
	cLeft = ColorShift(cBg, (z<0) ? agLowColorShift:agHighColorShift);
	cRight = ColorShift(cBg, (z<0) ? agHighColorShift:agLowColorShift);

	/* Fill the background except the corners. */
	agPrim.rect_filled(wid,			/* Body */
	    r->x + rad,			r->y + rad,
	    r->w - rad*2,		r->h - rad,
	    cBg);
	agPrim.rect_filled(wid,			/* Top */
	    r->x + rad,			r->y,
	    r->w - rad*2,		r->h,
	    cBg);
	agPrim.rect_filled(wid,			/* Left */
	    r->x,			r->y + rad,
	    rad,			r->h - rad,
	    cBg);
	agPrim.rect_filled(wid,			/* Right */
	    r->x + r->w - rad,		r->y + rad,
	    rad,			r->h - rad,
	    cBg);

	/* Draw the three straight lines. */
	agPrim.hline(wid,				/* Top line */
	    r->x + rad,			r->x + r->w - rad,
	    r->y,
	    cBg);
	agPrim.vline(wid,				/* Left line */
	    r->x,
	    r->y + rad,			r->y + r->h,
	    cLeft);
	agPrim.vline(wid,				/* Right line */
	    r->x + r->w - 1,
	    r->y + rad,			r->y + r->h,
	    cRight);

	/* Draw the two chamfered edges using a Bresenham generalization. */
	v = 2*rad - 1;
	e = 0;
	u = 0;
	x = 0;
	y = rad;
	SDL_LockSurface(agView->v);
	while (x <= y) {
		int i;

		AG_WidgetPutPixel(wid,
		    r->x + rad - x,
		    r->y + rad - y,
		    cLeft);
		AG_WidgetPutPixel(wid,
		    r->x + rad - y,
		    r->y + rad - x,
		    cLeft);

		AG_WidgetPutPixel(wid,
		    r->x - rad + (r->w - 1) + x,
		    r->y + rad - y,
		    cRight);
		AG_WidgetPutPixel(wid,
		    r->x - rad + (r->w - 1) + y,
		    r->y + rad - x,
		    cRight);
		
		for (i = 0; i < x; i++) {
			AG_WidgetPutPixel(wid,
			    r->x + rad - i,
			    r->y + rad - y,
			    cBg);
			AG_WidgetPutPixel(wid,
			    r->x - rad + (r->w - 1) + i,
			    r->y + rad - y,
			    cBg);
		}
		for (i = 0; i < y; i++) {
			AG_WidgetPutPixel(wid,
			    r->x + rad - i,
			    r->y + rad - x,
			    cBg);
			AG_WidgetPutPixel(wid,
			    r->x - rad + (r->w - 1) + i,
			    r->y + rad - x,
			    cBg);
		}

		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;
	}
	SDL_UnlockSurface(agView->v);
}

/* Draw a 3D-style frame. */
static void
frame(void *p, int xoffs, int yoffs, int w, int h, int z, Uint32 color)
{
	AG_Widget *wid = p;
	Uint32 cLeft = ColorShift(color, (z<0) ? agLowColorShift :
	                                         agHighColorShift);
	Uint32 cRight = ColorShift(color, (z<0) ? agHighColorShift :
	                                          agLowColorShift);
	
	agPrim.hline(wid, xoffs,	xoffs+w-1,	yoffs,		cLeft);
	agPrim.hline(wid, xoffs,	xoffs+w,	yoffs+h-1,	cRight);
	agPrim.vline(wid, xoffs,	yoffs,		yoffs+h-1,	cLeft);
	agPrim.vline(wid, xoffs+w-1,	yoffs,		yoffs+h-1,	cRight);
}

/* Draw a blended 3D-style frame. */
static void
frame_blended(void *p, int xoffs, int yoffs, int w, int h, Uint8 c[4],
    enum ag_blend_func func)
{
	AG_Widget *wid = p;

	agPrim.line_blended(wid, xoffs, yoffs, xoffs+w-1, yoffs,
	    c, func);
	agPrim.line_blended(wid, xoffs, yoffs, xoffs, yoffs+h-1,
	    c, func);
	agPrim.line_blended(wid, xoffs, yoffs+h-1, xoffs+w-1, yoffs+h-1,
	    c, func);
	agPrim.line_blended(wid, xoffs+w-1, yoffs, xoffs+w-1, yoffs+h-1,
	    c, func);
}

/* Render a circle using a modified Bresenham line algorithm. */
static void
circle_bresenham(void *p, int wx, int wy, int radius, Uint32 color)
{
	AG_Widget *wid = p;
	int v = 2*radius - 1;
	int e = 0, u = 1;
	int x = 0, y = radius;

	SDL_LockSurface(agView->v);
	while (x < y) {
		AG_WidgetPutPixel(wid, wx+x, wy+y, color);
		AG_WidgetPutPixel(wid, wx+x, wy-y, color);
		AG_WidgetPutPixel(wid, wx-x, wy+y, color);
		AG_WidgetPutPixel(wid, wx-x, wy-y, color);

		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;
		
		AG_WidgetPutPixel(wid, wx+y, wy+x, color);
		AG_WidgetPutPixel(wid, wx+y, wy-x, color);
		AG_WidgetPutPixel(wid, wx-y, wy+x, color);
		AG_WidgetPutPixel(wid, wx-y, wy-x, color);
	}
	AG_WidgetPutPixel(wid, wx-radius, wy, color);
	AG_WidgetPutPixel(wid, wx+radius, wy, color);
	SDL_UnlockSurface(agView->v);
}

static void
circle2_bresenham(void *p, int wx, int wy, int radius, Uint32 color)
{
	AG_Widget *wid = p;
	int v = 2*radius - 1;
	int e = 0, u = 1;
	int x = 0, y = radius;

	SDL_LockSurface(agView->v);
	while (x < y) {
		AG_WidgetPutPixel(wid, wx+x, wy+y, color);
		AG_WidgetPutPixel(wid, wx+x+1, wy+y, color);
		AG_WidgetPutPixel(wid, wx+x, wy-y, color);
		AG_WidgetPutPixel(wid, wx+x+1, wy-y, color);
		AG_WidgetPutPixel(wid, wx-x, wy+y, color);
		AG_WidgetPutPixel(wid, wx-x-1, wy+y, color);
		AG_WidgetPutPixel(wid, wx-x, wy-y, color);
		AG_WidgetPutPixel(wid, wx-x-1, wy-y, color);


		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;
		
		AG_WidgetPutPixel(wid, wx+y, wy+x, color);
		AG_WidgetPutPixel(wid, wx+y+1, wy+x, color);
		AG_WidgetPutPixel(wid, wx+y, wy-x, color);
		AG_WidgetPutPixel(wid, wx+y+1, wy-x, color);
		AG_WidgetPutPixel(wid, wx-y, wy+x, color);
		AG_WidgetPutPixel(wid, wx-y-1, wy+x, color);
		AG_WidgetPutPixel(wid, wx-y, wy-x, color);
		AG_WidgetPutPixel(wid, wx-y-1, wy-x, color);
	}
	AG_WidgetPutPixel(wid, wx-radius, wy, color);
	AG_WidgetPutPixel(wid, wx+radius, wy, color);
	SDL_UnlockSurface(agView->v);
}

/* Render a 3D-style line. */
static void
line2(void *wid, int x1, int y1, int x2, int y2, Uint32 color)
{
	agPrim.line(wid, x1, y1, x2, y2,
	    ColorShift(color, agHighColorShift));
	agPrim.line(wid, x1+1, y1+1, x2+1, y2+1,
	    ColorShift(color, agLowColorShift));
}

/*
 * Draw a line segment between two points using the Bresenham algorithm
 * presented by Foley & Van Dam [1990].
 */
static void
line_bresenham(void *widget, int x1, int y1, int x2, int y2, Uint32 color)
{
	AG_Widget *wid = widget;
	int dx, dy;
	int inc1, inc2;
	int d, x, y;
	int xend, yend;
	int xdir, ydir;

	dx = abs(x2-x1);
	dy = abs(y2-y1);

	SDL_LockSurface(agView->v);

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
		AG_WidgetPutPixel(wid, x, y, color);

		if (((y2-y1)*ydir) > 0) {
			while (x < xend) {
				x++;
				if (d < 0) {
					d += inc1;
				} else {
					y++;
					d += inc2;
				}
				AG_WidgetPutPixel(wid, x, y, color);
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
				AG_WidgetPutPixel(wid, x, y, color);
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
		AG_WidgetPutPixel(wid, x, y, color);

		if (((x2-x1)*xdir) > 0) {
			while (y < yend) {
				y++;
				if (d < 0) {
					d += inc1;
				} else {
					x++;
					d += inc2;
				}
				AG_WidgetPutPixel(wid, x, y, color);
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
				AG_WidgetPutPixel(wid, x, y, color);
			}
		}
	}
	SDL_UnlockSurface(agView->v);
}

static __inline__ int
hline_clip(AG_Widget *wid, int *x1, int *x2, int *y, int *dx)
{
	SDL_Rect *rd = &agView->v->clip_rect;

	if ((wid->cy + *y) <  rd->y ||
	    (wid->cy + *y) >= rd->y+rd->h) {
		return (1);
	}
	if (*x1 > *x2) {
		*dx = *x2;
		*x2 = *x1;
		*x1 = *dx;
	}
	if (wid->cx + *x1 < rd->x)
		*x1 = rd->x - wid->cx;
	if (wid->cx + *x2 >= rd->x+rd->w)
		*x2 = rd->x + rd->w - wid->cx - 1;

	*dx = *x2 - *x1;
	return (*dx <= 0);
}

static void
hline32(void *widget, int x1, int x2, int y, Uint32 c)
{
	AG_Widget *wid = widget;
	Uint8 *pDst, *pEnd;
	int dx;

	if (hline_clip(wid, &x1, &x2, &y, &dx)) {
		return;
	}
	SDL_LockSurface(agView->v);
	pDst = (Uint8 *)agView->v->pixels + (wid->cy+y)*agView->v->pitch +
	    ((wid->cx+x1)<<2);
	pEnd = pDst + (dx<<2);
	while (pDst < pEnd) {
		*(Uint32 *)pDst = c;
		pDst += 4;
	}
	SDL_UnlockSurface(agView->v);
}

static void
hline24(void *widget, int x1, int x2, int y, Uint32 c)
{
	AG_Widget *wid = widget;
	Uint8 *pDst, *pEnd;
	int dx;
	
	if (hline_clip(wid, &x1, &x2, &y, &dx)) {
		return;
	}
	SDL_LockSurface(agView->v);
	pDst = (Uint8 *)agView->v->pixels + (wid->cy+y)*agView->v->pitch +
	    (wid->cx+x1)*3;
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
	SDL_UnlockSurface(agView->v);
}

static void
hline16(void *widget, int x1, int x2, int y, Uint32 c)
{
	AG_Widget *wid = widget;
	Uint8 *pDst, *pEnd;
	int dx;

	if (hline_clip(wid, &x1, &x2, &y, &dx)) {
		return;
	}
	SDL_LockSurface(agView->v);
	pDst = (Uint8 *)agView->v->pixels + (wid->cy+y)*agView->v->pitch +
	    ((wid->cx+x1)<<1);
	pEnd = pDst + (dx<<1);
	while (pDst < pEnd) {
		*(Uint16 *)pDst = c;
		pDst += 2;
	}
	SDL_UnlockSurface(agView->v);
}
		
static void
hline8(void *widget, int x1, int x2, int y, Uint32 c)
{
	AG_Widget *wid = widget;
	Uint8 *pDst, *pEnd;
	int dx;

	if (hline_clip(wid, &x1, &x2, &y, &dx)) {
		return;
	}
	SDL_LockSurface(agView->v);
	pDst = (Uint8 *)agView->v->pixels + (wid->cy+y)*agView->v->pitch +
	    (wid->cx+x1);
	pEnd = pDst + dx;
	memset(pDst, c, pEnd-pDst);
	SDL_UnlockSurface(agView->v);
}

static __inline__ int
vline_clip(AG_Widget *wid, int *x, int *y1, int *y2, int *dy)
{
	SDL_Rect *rd = &agView->v->clip_rect;

	if ((wid->cx + *x) <  rd->x ||
	    (wid->cx + *x) >= rd->x+rd->w) {
		return (1);
	}
	if (*y1 > *y2) {
		*dy = *y2;
		*y2 = *y1;
		*y1 = *dy;
	}
	if (wid->cy + *y1 < rd->y)
		*y1 = rd->y - wid->cy;
	if (wid->cy + *y2 >= rd->y+rd->h)
		*y2 = rd->y + rd->h - wid->cy - 1;

	*dy = *y2 - *y1;
	return (*dy <= 0);
}

static void
vline32(void *widget, int x, int y1, int y2, Uint32 c)
{
	AG_Widget *wid = widget;
	Uint8 *pDst, *pEnd;
	int dy;

	if (vline_clip(wid, &x, &y1, &y2, &dy)) {
		return;
	}
	SDL_LockSurface(agView->v);
	pDst = (Uint8 *)agView->v->pixels + (wid->cy+y1)*agView->v->pitch +
	    ((wid->cx+x)<<2);
	pEnd = pDst + dy*agView->v->pitch;
	while (pDst < pEnd) {
		*(Uint32 *)pDst = c;
		pDst += agView->v->pitch;
	}
	SDL_UnlockSurface(agView->v);
}

static void
vline24(void *widget, int x, int y1, int y2, Uint32 c)
{
	AG_Widget *wid = widget;
	Uint8 *pDst, *pEnd;
	int dy;

	if (vline_clip(wid, &x, &y1, &y2, &dy)) {
		return;
	}
	SDL_LockSurface(agView->v);
	pDst = (Uint8 *)agView->v->pixels + (wid->cy+y1)*agView->v->pitch +
	    (wid->cx+x)*3;
	pEnd = pDst + dy*agView->v->pitch;
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
		pDst += agView->v->pitch;
	}
	SDL_UnlockSurface(agView->v);
}

static void
vline16(void *widget, int x, int y1, int y2, Uint32 c)
{
	AG_Widget *wid = widget;
	Uint8 *pDst, *pEnd;
	int  dy;

	if (vline_clip(wid, &x, &y1, &y2, &dy)) {
		return;
	}
	SDL_LockSurface(agView->v);
	pDst = (Uint8 *)agView->v->pixels + (wid->cy+y1)*agView->v->pitch +
	    ((wid->cx+x)<<1);
	pEnd = pDst + dy*agView->v->pitch;
	while (pDst < pEnd) {
		*(Uint16 *)pDst = c;
		pDst += agView->v->pitch;
	}
	SDL_UnlockSurface(agView->v);
}

static void
vline8(void *widget, int x, int y1, int y2, Uint32 c)
{
	AG_Widget *wid = widget;
	Uint8 *pDst, *pEnd;
	int  dy;

	if (vline_clip(wid, &x, &y1, &y2, &dy)) {
		return;
	}
	SDL_LockSurface(agView->v);
	pDst = (Uint8 *)agView->v->pixels + (wid->cy+y1)*agView->v->pitch +
	    (wid->cx+x);
	pEnd = pDst + dy*agView->v->pitch;
	while (pDst < pEnd) {
		*(Uint8 *)pDst = c;
		pDst += agView->v->pitch;
	}
	SDL_UnlockSurface(agView->v);
}

static void
line_blended_bresenham(void *widget, int x1, int y1, int x2, int y2,
    Uint8 c[4], enum ag_blend_func func)
{
	AG_Widget *wid = widget;
	int dx, dy;
	int inc1, inc2;
	int d, x, y;
	int xend, yend;
	int xdir, ydir;

	dx = abs(x2-x1);
	dy = abs(y2-y1);

	SDL_LockSurface(agView->v);

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
		AG_WidgetBlendPixel(wid, x, y, c, func);

		if (((y2-y1)*ydir) > 0) {
			while (x < xend) {
				x++;
				if (d < 0) {
					d += inc1;
				} else {
					y++;
					d += inc2;
				}
				AG_WidgetBlendPixel(wid, x, y, c, func);
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
				AG_WidgetBlendPixel(wid, x, y, c, func);
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
		AG_WidgetBlendPixel(wid, x, y, c, func);

		if (((x2-x1)*xdir) > 0) {
			while (y < yend) {
				y++;
				if (d < 0) {
					d += inc1;
				} else {
					x++;
					d += inc2;
				}
				AG_WidgetBlendPixel(wid, x, y, c, func);
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
				AG_WidgetBlendPixel(wid, x, y, c, func);
			}
		}
	}
	SDL_UnlockSurface(agView->v);
}

/* Render an outlined rectangle. */
static void
rect_outlined(void *p, int x1, int y1, int w, int h, Uint32 color)
{
	AG_Widget *wid = p;
	int x2 = x1+w-1;
	int y2 = y1+h-1;

	agPrim.hline(wid, x1, x2, y1, color);
	agPrim.hline(wid, x1, x2, y2, color);
	agPrim.vline(wid, x1, y1, y2, color);
	agPrim.vline(wid, x2, y1, y2, color);
}

/* Render a filled rectangle. */
static void
rect_filled(void *p, int x, int y, int w, int h, Uint32 color)
{
	AG_Widget *wid = p;
	SDL_Rect rd;

	rd.x = wid->cx+x;
	rd.y = wid->cy+y;
	rd.w = w;
	rd.h = h;
	SDL_FillRect(agView->v, &rd, color);
}

/* Render an alpha blended rectangle. */
static void
rect_blended(void *p, int x1, int y1, int pw, int ph, Uint8 c[4],
    enum ag_blend_func func)
{
	AG_Widget *wid = p;
	Uint8 *pView;
	int x, y, yinc, d;
	int w = pw;
	int h = ph;

	if (x1 < 0) {
		if (x1+w >= 0) {
			w += x1;
			x1 = 0;
		} else {
			return;
		}
	}
	if (y1 < 0) {
		if (y1+h >= 0) {
			h += y1;
			y1 = 0;
		} else {
			return;
		}
	}
	if ((d = (wid->cx+x1+w - wid->cx2)) > 0) { w -= d; }
	if ((d = (wid->cy+y1+h - wid->cy2)) > 0) { h -= d; }

	pView = (Uint8 *)agView->v->pixels +
	    (wid->cy+y1)*agView->v->pitch +
	    (wid->cx+x1)*agVideoFmt->BytesPerPixel;
	yinc = agView->v->pitch - w*agVideoFmt->BytesPerPixel;

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			AG_BLEND_RGBA(agView->v, pView, c[0], c[1], c[2], c[3],
			    func);
			pView += agVideoFmt->BytesPerPixel;
		}
		pView += yinc;
	}
}

/* Draw a gimp-style background tiling. */
static void
tiling(void *p, SDL_Rect rd, int tsz, int offs, Uint32 c1, Uint32 c2)
{
	AG_Widget *wid = p;
	int alt1 = 0, alt2 = 0;
	int x, y;

	/* XXX inelegant */
	for (y = rd.y-tsz+offs;
	     y < rd.y+rd.h;
	     y += tsz) {
		for (x = rd.x-tsz+offs;
		     x < rd.x+rd.w;
		     x += tsz) {
			if (alt1++ == 1) {
				agPrim.rect_filled(wid, x, y, tsz, tsz,
				    c1);
				alt1 = 0;
			} else {
				agPrim.rect_filled(wid, x, y, tsz, tsz,
				    c2);
			}
		}
		if (alt2++ == 1) {
			alt2 = 0;
		}
		alt1 = alt2;
	}
}

/* Draw a [+] sign. */
static void
plus(void *p, int x, int y, int w, int h, Uint8 c[4], enum ag_blend_func func)
{
	int xcen = x + w/2;
	int ycen = y + h/2;

	agPrim.line_blended(p, xcen, y, xcen, y+h, c, func);
	agPrim.line_blended(p, x, ycen, x+w, ycen, c, func);
}

/* Draw a [-] sign. */
static void
minus(void *p, int x, int y, int w, int h, Uint8 c[4],
    enum ag_blend_func func)
{
	AG_Widget *wid = p;
	int ycen = y+h/2;

	agPrim.line_blended(wid, x, ycen, x+w, ycen, c, func);
}

/*
 * OpenGL versions of the primitives. Note that we do not bother using
 * LockGL(), so the primitives are not safe to invoke anywhere outside
 * of widget draw functions.
 */
#ifdef HAVE_OPENGL
static void
line_opengl(void *p, int px1, int py1, int px2, int py2, Uint32 color)
{
	AG_Widget *wid = p;
	int x1 = wid->cx + px1;
	int y1 = wid->cy + py1;
	int x2 = wid->cx + px2;
	int y2 = wid->cy + py2;
	Uint8 r, g, b;

	SDL_GetRGB(color, agVideoFmt, &r, &g, &b);
	glBegin(GL_LINES);
	glColor3ub(r, g, b);
	glVertex2s(x1, y1);
	glVertex2s(x2, y2);
	glEnd();
}

static void
hline_opengl(void *p, int x1, int x2, int py, Uint32 color)
{
	AG_Widget *wid = p;
	int y = wid->cy + py;
	Uint8 r, g, b;

	SDL_GetRGB(color, agVideoFmt, &r, &g, &b);
	glBegin(GL_LINES);
	glColor3ub(r, g, b);
	glVertex2s(wid->cx + x1, y);
	glVertex2s(wid->cx + x2, y);
	glEnd();
}

static void
vline_opengl(void *p, int px, int y1, int y2, Uint32 color)
{
	AG_Widget *wid = p;
	int x = wid->cx + px;
	Uint8 r, g, b;

	SDL_GetRGB(color, agVideoFmt, &r, &g, &b);
	glBegin(GL_LINES);
	glColor3ub(r, g, b);
	glVertex2s(x, wid->cy + y1);
	glVertex2s(x, wid->cy + y2);
	glEnd();
}

static void
line_blended_opengl(void *p, int px1, int py1, int px2, int py2, Uint8 c[4],
    enum ag_blend_func func)
{
	AG_Widget *wid = p;
	int x1 = wid->cx + px1;
	int y1 = wid->cy + py1;
	int x2 = wid->cx + px2;
	int y2 = wid->cy + py2;
	GLboolean blend_save;
	GLint sfac_save, dfac_save;
	
	if (c[3] < 255) {
		glGetBooleanv(GL_BLEND, &blend_save);
		glGetIntegerv(GL_BLEND_SRC, &sfac_save);
		glGetIntegerv(GL_BLEND_DST, &dfac_save);
			
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glBegin(GL_LINES);
	glColor4ub(c[0], c[1], c[2], c[3]);
	glVertex2s(x1, y1);
	glVertex2s(x2, y2);
	glEnd();
	
	if (c[3] < 255) {
		if (blend_save) {
			glEnable(GL_BLEND);
		} else {
			glDisable(GL_BLEND);
		}
		glBlendFunc(sfac_save, dfac_save);
	}
}

static void
circle_opengl(void *p, int x, int y, int radius, Uint32 color)
{
	AG_Widget *wid = p;
	int nedges = radius*2;
	int i;
	Uint8 r, g, b;
	
	SDL_GetRGB(color, agVideoFmt, &r, &g, &b);

	glBegin(GL_LINE_LOOP);
	glColor3ub(r, g, b);
	for (i = 0; i < nedges; i++) {
		glVertex2f(wid->cx + x + radius*cos((2*AG_PI*i)/nedges),
		           wid->cy + y + radius*sin((2*AG_PI*i)/nedges));
	}
	glEnd();
}

static void
circle2_opengl(void *p, int x, int y, int radius, Uint32 color)
{
	AG_Widget *wid = p;
	int nedges = radius*2;
	int i;
	Uint8 r, g, b;
	
	SDL_GetRGB(color, agVideoFmt, &r, &g, &b);

	glBegin(GL_LINE_LOOP);
	glColor3ub(r, g, b);
	for (i = 0; i < nedges; i++) {
		glVertex2f(wid->cx + x + radius*cos((2*AG_PI*i)/nedges),
		           wid->cy + y + radius*sin((2*AG_PI*i)/nedges));
		glVertex2f(wid->cx + x + (radius+1)*cos((2*AG_PI*i)/nedges),
		           wid->cy + y + (radius+1)*sin((2*AG_PI*i)/nedges));
	}
	glEnd();
}

static void
rect_opengl(void *p, int x, int y, int w, int h, Uint32 color)
{
	AG_Widget *wid = p;
	Uint8 r, g, b;
	int x1 = wid->cx+x;
	int y1 = wid->cy+y;
	int x2 = x1+w-1;
	int y2 = y1+h-1;

	if (wid->flags & AG_WIDGET_CLIPPING) {
		if (x1 > wid->cx+wid->w ||
		    y1 > wid->cy+wid->h) {
			return;
		}
		if (x1 < wid->cx)
			x1 = wid->cx;
		if (y1 < wid->cy)
			y1 = wid->cy;
		if (x2 > wid->cx+wid->w)
			x2 = wid->cx+wid->w;
		if (y2 > wid->cy+wid->h)
			y2 = wid->cy+wid->h;
	}

	SDL_GetRGB(color, agVideoFmt, &r, &g, &b);

	glBegin(GL_POLYGON);
	glColor3ub(r, g, b);
	glVertex2i(x1, y1);
	glVertex2i(x2, y1);
	glVertex2i(x2, y2);
	glVertex2i(x1, y2);
	glEnd();
}

static void
rect_blended_opengl(void *p, int x, int y, int w, int h, Uint8 c[4],
    enum ag_blend_func func)
{
	AG_Widget *wid = p;
	int x1 = wid->cx+x;
	int y1 = wid->cy+y;
	int x2 = x1+w;
	int y2 = y1+h;
	GLboolean blend_save;
	GLint sfac_save, dfac_save;

	if (wid->flags & AG_WIDGET_CLIPPING) {
		if (x1 > wid->cx+wid->w ||
		    y1 > wid->cy+wid->h) {
			return;
		}
		if (x1 < wid->cx)
			x1 = wid->cx;
		if (y1 < wid->cy)
			y1 = wid->cy;
		if (x2 > wid->cx+wid->w)
			x2 = wid->cx+wid->w;
		if (y2 > wid->cy+wid->h)
			y2 = wid->cy+wid->h;
	}

	if (c[3] < 255) {
		glGetBooleanv(GL_BLEND, &blend_save);
		glGetIntegerv(GL_BLEND_SRC, &sfac_save);
		glGetIntegerv(GL_BLEND_DST, &dfac_save);
			
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glBegin(GL_POLYGON);
	glColor4ub(c[0], c[1], c[2], c[3]);
	glVertex2i(x1, y1);
	glVertex2i(x2, y1);
	glVertex2i(x2, y2);
	glVertex2i(x1, y2);
	glEnd();
	
	if (c[3] < 255) {
		if (blend_save) {
			glEnable(GL_BLEND);
		} else {
			glDisable(GL_BLEND);
		}
		glBlendFunc(sfac_save, dfac_save);
	}
}

static void
box_dithered_gl(void *p, int xoffs, int yoffs, int w, int h, int z, Uint32 c1,
    Uint32 c2)
{
	/* TODO */
}

/* Draw a 3D-style box with chamfered top edges. */
static void
box_chamfered_gl(void *p, SDL_Rect *rd, int z, int rad, Uint32 cBg)
{
	AG_Widget *wid = p;
	Uint8 r, g, b;
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(wid->cx+rd->x, wid->cy+rd->y, 0);
	glBegin(GL_POLYGON);
	{
		SDL_GetRGB(cBg, agVideoFmt, &r, &g, &b);
		glColor3ub(r, g, b);
		glVertex2i(0,			rd->h);
		glVertex2i(0,			rad);
		glVertex2i(rad,			0);
		glVertex2i((rd->w - rad),	0);
		glVertex2i(rd->w,		rad);
		glVertex2i(rd->w,		rd->h);
	}
	glEnd();
	if (z >= 0) {
		glBegin(GL_LINE_STRIP);
		{
			SDL_GetRGB(ColorShift(cBg, agHighColorShift),
			                      agVideoFmt, &r, &g, &b);
			glColor3ub(r, g, b);
			glVertex2i(0, rd->h);
			glVertex2i(0, rad);
			glVertex2i(rad, 0);
		}
		glEnd();
		glBegin(GL_LINES);
		{
			SDL_GetRGB(ColorShift(cBg, agLowColorShift),
			                      agVideoFmt, &r, &g, &b);
			glColor3ub(r, g, b);
			glVertex2i((rd->w - 1), rd->h);
			glVertex2i((rd->w - 1), rad);
		}
		glEnd();
	}
	glPopMatrix();
}

static void
arrow_up_gl(void *p, int x, int y, int h, Uint32 c1, Uint32 c2)
{
	AG_Widget *wid = p;
	Uint8 r, g, b;

	SDL_GetRGB(c1, agVideoFmt, &r, &g, &b);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(wid->cx, wid->cy, 0);
	glBegin(GL_POLYGON);
	{
		glColor3ub(r, g, b);
		glVertex2i(x,		y - (h>>1));
		glVertex2i(x - (h>>1),	y + (h>>1));
		glVertex2i(x + (h>>1),	y + (h>>1));
	}
	glEnd();
	glPopMatrix();
}

static void
arrow_down_gl(void *p, int x, int y, int h, Uint32 c1, Uint32 c2)
{
	AG_Widget *wid = p;
	Uint8 r, g, b;

	SDL_GetRGB(c1, agVideoFmt, &r, &g, &b);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(wid->cx, wid->cy, 0);
	glBegin(GL_POLYGON);
	{
		glColor3ub(r, g, b);
		glVertex2i(x - (h>>1),	y - (h>>1));
		glVertex2i(x + (h>>1),	y - (h>>1));
		glVertex2i(x,		y + (h>>1));
	}
	glEnd();
	glPopMatrix();
}

static void
arrow_left_gl(void *p, int x, int y, int h, Uint32 c1, Uint32 c2)
{
}

static void
arrow_right_gl(void *p, int x, int y, int h, Uint32 c1, Uint32 c2)
{
}
#endif /* HAVE_OPENGL */

void
AG_InitPrimitives(void)
{
	agPrim.box = box;
	agPrim.frame = frame;
	agPrim.frame_blended = frame_blended;
	agPrim.rect_outlined = rect_outlined;
	agPrim.plus = plus;
	agPrim.minus = minus;
	agPrim.line2 = line2;
	agPrim.tiling = tiling;

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		agPrim.line = line_opengl;
		agPrim.hline = hline_opengl;
		agPrim.vline = vline_opengl;
		agPrim.line_blended = line_blended_opengl;
		agPrim.rect_filled = rect_opengl;
		agPrim.rect_blended = rect_blended_opengl;
		agPrim.circle = circle_opengl;
		agPrim.circle2 = circle2_opengl;
		agPrim.box_chamfered = box_chamfered_gl;
		agPrim.box_dithered = box_dithered_gl;
		agPrim.arrow_up = arrow_up_gl;
		agPrim.arrow_down = arrow_down_gl;
		agPrim.arrow_left = arrow_left_gl;
		agPrim.arrow_right = arrow_right_gl;
	} else
#endif
	{
		agPrim.line = line_bresenham;
		agPrim.line_blended = line_blended_bresenham;
		agPrim.rect_filled = rect_filled;
		agPrim.rect_blended = rect_blended;
		agPrim.circle = circle_bresenham;
		agPrim.circle2 = circle2_bresenham;
		agPrim.box_chamfered = box_chamfered;
		agPrim.box_dithered = box_dithered;
		agPrim.arrow_up = arrow_up;
		agPrim.arrow_down = arrow_down;
		agPrim.arrow_left = arrow_left;
		agPrim.arrow_right = arrow_right;
		
		switch (agVideoFmt->BytesPerPixel) {
		case 4:
			agPrim.hline = hline32;
			agPrim.vline = vline32;
			break;
		case 3:
			agPrim.hline = hline24;
			agPrim.vline = vline24;
			break;
		case 2:
			agPrim.hline = hline16;
			agPrim.vline = vline16;
			break;
		case 1:
			agPrim.hline = hline8;
			agPrim.vline = vline8;
			break;
		}
	}
}

