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

/*
 * Low-level primitive graphics routines for use by GUI widgets.
 */

#include "opengl.h"

#include <core/core.h>
#include <gui/view.h>

#include "widget.h"
#include "window.h"
#include "primitive.h"
#include "gui_math.h"

AG_PrimitiveOps agPrim;

#ifdef HAVE_OPENGL
static Uint8 DisabledStipple[128];  /* Stiple pattern for "disabled" items */
#endif

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
ArrowUpFB(void *p, int x0, int y0, int h, Uint32 c1, Uint32 c2)
{
	AG_Widget *wid = p;
	int y1 = wid->rView.y1 + y0 - (h>>1);
	int y2 = y1 + h - 1;
	int xs = wid->rView.x1 + x0, xe = xs;
	int x, y;

	AG_LockView();
	for (y = y1; y < y2; y+=2) {
		for (x = xs; x <= xe; x++) {
			AG_VIEW_PUT_PIXEL2_CLIPPED(x, y,
			    (x == xs || x == xe) ? c2 : c1);
			AG_VIEW_PUT_PIXEL2_CLIPPED(x, y+1, c1);
		}
		xs--;
		xe++;
	}
	AG_UnlockView();
}

static void
ArrowDownFB(void *p, int x0, int y0, int h, Uint32 c1, Uint32 c2)
{
	AG_Widget *wid = p;
	int y1 = wid->rView.y1 + y0 - (h>>1);
	int y2 = y1+h-1;
	int xs = wid->rView.x1 + x0, xe = xs;
	int x, y;

	AG_LockView();
	for (y = y2; y > y1; y-=2) {
		for (x = xs; x <= xe; x++) {
			AG_VIEW_PUT_PIXEL2_CLIPPED(x, y,
			    (x == xs || x == xe) ? c2 : c1);
			AG_VIEW_PUT_PIXEL2_CLIPPED(x, y-1, c1);
		}
		xs--;
		xe++;
	}
	AG_UnlockView();
}

static void
ArrowLeftFB(void *p, int x0, int y0, int h, Uint32 c1, Uint32 c2)
{
	AG_Widget *wid = p;
	int x1 = wid->rView.x1 + x0 - (h>>1);
	int x2 = x1+h-1;
	int ys = wid->rView.y1 + y0, ye = ys;
	int x, y;

	AG_LockView();
	for (x = x1; x < x2; x+=2) {
		for (y = ys; y <= ye; y++) {
			AG_VIEW_PUT_PIXEL2_CLIPPED(x+1, y, c1);
			AG_VIEW_PUT_PIXEL2_CLIPPED(x, y,
			    (y == ys || y == ye) ? c2 : c1);
		}
		ys--;
		ye++;
	}
	AG_UnlockView();
}

static void
ArrowRightFB(void *p, int x0, int y0, int h, Uint32 c1, Uint32 c2)
{
	AG_Widget *wid = p;
	int x1 = wid->rView.x1 + x0 - (h>>1);
	int x2 = x1+h-1;
	int ys = wid->rView.y1 + y0, ye = ys;
	int x, y;

	AG_LockView();
	for (x = x2; x > x1; x-=2) {
		for (y = ys; y <= ye; y++) {
			AG_VIEW_PUT_PIXEL2_CLIPPED(x-1, y, c1);
			AG_VIEW_PUT_PIXEL2_CLIPPED(x, y,
			    (y == ys || y == ye) ? c2 : c1);
		}
		ys--;
		ye++;
	}
	AG_UnlockView();
}

/* Render a 3D-style box. */
static void
Box(void *p, AG_Rect r, int z, Uint32 c)
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
	AG_DrawRectFilled(wid, r, cBg);
	AG_DrawFrame(wid, r, z, c);
}

/* Render a 3D-style box with dithering. */
static void
BoxDisabledFB(void *p, AG_Rect r, int z, Uint32 c1, Uint32 c2)
{
	AG_Widget *wid = p;
	int x, y;
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
	AG_DrawBox(p, r, z, c1);

	AG_LockView();
	for (y = r.y; y < r.y+r.h-2; y++) {
		flag = !flag;
		for (x = r.x+1+flag; x < r.x+r.w-2; x+=2)
			AG_WidgetPutPixel(wid, x, y, cDither);
	}
	AG_UnlockView();
}

/* Render a 3D-style box with rounded top edges. */
static void
BoxRoundedTopFB(void *p, AG_Rect r, int z, int rad, Uint32 cBg)
{
	AG_Widget *wid = p;
	AG_Rect rd;
	Uint32 cLeft, cRight;
	int v, e, u;
	int x, y, i;
	
	cLeft = ColorShift(cBg, (z<0) ? agLowColorShift:agHighColorShift);
	cRight = ColorShift(cBg, (z<0) ? agHighColorShift:agLowColorShift);

	/* Center */
	rd.x = r.x+rad;
	rd.y = r.y+rad;
	rd.w = r.w - rad*2;
	rd.h = r.h - rad;
	AG_DrawRectFilled(wid, rd, cBg);

	/* Top */
	rd.y = r.y;
	rd.h = r.h;
	AG_DrawRectFilled(wid, rd, cBg);

	/* Left */
	rd.x = r.x;
	rd.y = r.y+rad;
	rd.w = rad;
	rd.h = r.h-rad;
	AG_DrawRectFilled(wid, rd, cBg);

	/* Right */
	rd.x = r.x+r.w-rad;
	rd.y = r.y+rad;
	rd.w = rad;
	rd.h = r.h-rad;
	AG_DrawRectFilled(wid, rd, cBg);

	/* Top, left and right lines */
	AG_DrawLineH(wid, r.x+rad,   r.x+r.w-rad, r.y,     cBg);
	AG_DrawLineV(wid, r.x,       r.y+rad,     r.y+r.h, cLeft);
	AG_DrawLineV(wid, r.x+r.w-1, r.y+rad,     r.y+r.h, cRight);

	/* Top left and top right rounded edges */
	v = 2*rad - 1;
	e = 0;
	u = 0;
	x = 0;
	y = rad;
	AG_LockView();
	while (x <= y) {
		AG_WidgetPutPixel(wid, r.x+rad-x, r.y+rad-y, cLeft);
		AG_WidgetPutPixel(wid, r.x+rad-y, r.y+rad-x, cLeft);
		AG_WidgetPutPixel(wid, r.x-rad+(r.w-1)+x, r.y+rad-y, cRight);
		AG_WidgetPutPixel(wid, r.x-rad+(r.w-1)+y, r.y+rad-x, cRight);
		for (i = 0; i < x; i++) {
			AG_WidgetPutPixel(wid, r.x+rad-i, r.y+rad-y, cBg);
			AG_WidgetPutPixel(wid, r.x-rad+(r.w-1)+i, r.y+rad-y,
			    cBg);
		}
		for (i = 0; i < y; i++) {
			AG_WidgetPutPixel(wid, r.x+rad-i, r.y+rad-x, cBg);
			AG_WidgetPutPixel(wid, r.x-rad+(r.w-1)+i, r.y+rad-x,
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
	AG_UnlockView();
}

/* Render a 3D-style box with rounded edges. */
static void
BoxRoundedFB(void *p, AG_Rect r, int z, int pRad, Uint32 c)
{
	AG_Widget *wid = p;
	int rad = pRad;
	AG_Rect rd;
	Uint32 cL, cR, cBg;
	int v, e, u;
	int x, y, i;
	int w1 = r.w - 1;
	
	if (rad*2 > r.w || rad*2 > r.h) {
		rad = MIN(r.w/2, r.h/2);
	}
	if (r.w < 4 || r.h < 4)
		return;

	if (AG_WidgetFocused(wid)) {
		cBg = ColorShift(c, (z<0) ? agFocusSunkColorShift :
		                            agFocusRaisedColorShift);
	} else {
		cBg = ColorShift(c, (z<0) ? agNofocusSunkColorShift :
		                            agNofocusRaisedColorShift);
	}
	cL = ColorShift(cBg, (z<0) ? agLowColorShift:agHighColorShift);
	cR = ColorShift(cBg, (z<0) ? agHighColorShift:agLowColorShift);
	
	/* Center */
	rd.x = r.x+rad;
	rd.y = r.y+rad;
	rd.w = r.w - rad*2;
	rd.h = r.h - rad*2;
	AG_DrawRectFilled(wid, rd, cBg);
	
	/* Top */
	rd.y = r.y;
	rd.h = rad;
	AG_DrawRectFilled(wid, rd, cBg);
	
	/* Bottom */
	rd.y = r.h-rad;
	rd.h = rad;
	AG_DrawRectFilled(wid, rd, cBg);

	/* Left */
	rd.x = r.x;
	rd.y = r.y+rad;
	rd.w = rad;
	rd.h = r.h-rad*2;
	AG_DrawRectFilled(wid, rd, cBg);

	/* Right */
	rd.x = r.x+r.w-rad;
	rd.y = r.y+rad;
	rd.w = rad;
	rd.h = r.h-rad*2;
	AG_DrawRectFilled(wid, rd, cBg);

	/* Rounded edges */
	v = 2*rad - 1;
	e = 0;
	u = 0;
	x = 0;
	y = rad;
	AG_LockView();
	while (x <= y) {
		AG_WidgetPutPixel(wid, r.x+rad-x,    r.y+rad-y,     cL);
		AG_WidgetPutPixel(wid, r.x+rad-y,    r.y+rad-x,     cL);
		AG_WidgetPutPixel(wid, r.x-rad+w1+x, r.y+rad-y,     cR);
		AG_WidgetPutPixel(wid, r.x-rad+w1+y, r.y+rad-x,     cR);

		AG_WidgetPutPixel(wid, r.x+rad-x,    r.y+r.h-rad+y, cL);
		AG_WidgetPutPixel(wid, r.x+rad-y,    r.y+r.h-rad+x, cL);
		AG_WidgetPutPixel(wid, r.x-rad+w1+x, r.y+r.h-rad+y, cR);
		AG_WidgetPutPixel(wid, r.x-rad+w1+y, r.y+r.h-rad+x, cR);

		for (i = 0; i < x; i++) {
			AG_WidgetPutPixel(wid, r.x+rad-i,    r.y+rad-y, cBg);
			AG_WidgetPutPixel(wid, r.x-rad+w1+i, r.y+rad-y, cBg);
			AG_WidgetPutPixel(wid, r.x+rad-i,    r.y+r.h-rad+y,cBg);
			AG_WidgetPutPixel(wid, r.x-rad+w1+i, r.y+r.h-rad+y,cBg);
		}
		for (i = 0; i < y; i++) {
			AG_WidgetPutPixel(wid,
			    r.x + rad - i,
			    r.y + rad - x,
			    cBg);
			AG_WidgetPutPixel(wid,
			    r.x - rad + w1 + i,
			    r.y + rad - x,
			    cBg);
			AG_WidgetPutPixel(wid,
			    r.x + rad - i,
			    r.y + r.h - rad + x,
			    cBg);
			AG_WidgetPutPixel(wid,
			    r.x - rad + w1 + i,
			    r.y + r.h - rad + x,
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
	
	/* Contour lines */
	AG_DrawLineH(wid, r.x+rad,   r.x+r.w-rad,   r.y,         cBg);
	AG_DrawLineH(wid, r.x+rad/2, r.x+r.w-rad/2, r.y,         cL);
	AG_DrawLineH(wid, r.x+rad/2, r.x+r.w-rad/2, r.y+r.h,     cR);
	AG_DrawLineV(wid, r.x,       r.y+rad,       r.y+r.h-rad, cL);
	AG_DrawLineV(wid, r.x+w1,    r.y+rad,       r.y+r.h-rad, cR);

	AG_UnlockView();
}

/* Render a 3D-style frame. */
static void
Frame(void *p, AG_Rect r, int z, Uint32 color)
{
	AG_Widget *wid = p;
	Uint32 cLeft = ColorShift(color, (z<0) ? agLowColorShift :
	                                         agHighColorShift);
	Uint32 cRight = ColorShift(color, (z<0) ? agHighColorShift :
	                                          agLowColorShift);
	
	AG_DrawLineH(wid, r.x,		r.x+r.w-1,	r.y,		cLeft);
	AG_DrawLineH(wid, r.x,		r.x+r.w,	r.y+r.h-1,	cRight);
	AG_DrawLineV(wid, r.x,		r.y,		r.y+r.h-1,	cLeft);
	AG_DrawLineV(wid, r.x+r.w-1,	r.y,		r.y+r.h-1,	cRight);
}

/* Render a blended 3D-style frame. */
static void
FrameBlended(void *p, AG_Rect r, Uint8 c[4], AG_BlendFn func)
{
	AG_Widget *wid = p;

	AG_DrawLineBlended(wid, r.x, r.y, r.x+r.w-1, r.y, c, func);
	AG_DrawLineBlended(wid, r.x, r.y, r.x, r.y+r.h-1, c, func);
	AG_DrawLineBlended(wid, r.x, r.y+r.h-1, r.x+r.w-1, r.y+r.h-1, c, func);
	AG_DrawLineBlended(wid, r.x+r.w-1, r.y, r.x+r.w-1, r.y+r.h-1, c, func);
}

/* Render a circle using a modified Bresenham line algorithm. */
static void
CircleFB(void *p, int px, int py, int radius, Uint32 color)
{
	AG_Widget *wid = p;
	int v = 2*radius - 1;
	int e = 0, u = 1;
	int x = 0, y = radius;

	AG_LockView();
	while (x < y) {
		AG_WidgetPutPixel(wid, px+x, py+y, color);
		AG_WidgetPutPixel(wid, px+x, py-y, color);
		AG_WidgetPutPixel(wid, px-x, py+y, color);
		AG_WidgetPutPixel(wid, px-x, py-y, color);
		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;
		AG_WidgetPutPixel(wid, px+y, py+x, color);
		AG_WidgetPutPixel(wid, px+y, py-x, color);
		AG_WidgetPutPixel(wid, px-y, py+x, color);
		AG_WidgetPutPixel(wid, px-y, py-x, color);
	}
	AG_WidgetPutPixel(wid, px-radius, py, color);
	AG_WidgetPutPixel(wid, px+radius, py, color);
	AG_UnlockView();
}

static void
Circle2FB(void *p, int px, int py, int radius, Uint32 color)
{
	AG_Widget *wid = p;
	int v = 2*radius - 1;
	int e = 0, u = 1;
	int x = 0, y = radius;

	AG_LockView();
	while (x < y) {
		AG_WidgetPutPixel(wid, px+x,   py+y, color);
		AG_WidgetPutPixel(wid, px+x+1, py+y, color);
		AG_WidgetPutPixel(wid, px+x,   py-y, color);
		AG_WidgetPutPixel(wid, px+x+1, py-y, color);
		AG_WidgetPutPixel(wid, px-x,   py+y, color);
		AG_WidgetPutPixel(wid, px-x-1, py+y, color);
		AG_WidgetPutPixel(wid, px-x,   py-y, color);
		AG_WidgetPutPixel(wid, px-x-1, py-y, color);
		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;
		AG_WidgetPutPixel(wid, px+y,   py+x, color);
		AG_WidgetPutPixel(wid, px+y+1, py+x, color);
		AG_WidgetPutPixel(wid, px+y,   py-x, color);
		AG_WidgetPutPixel(wid, px+y+1, py-x, color);
		AG_WidgetPutPixel(wid, px-y,   py+x, color);
		AG_WidgetPutPixel(wid, px-y-1, py+x, color);
		AG_WidgetPutPixel(wid, px-y,   py-x, color);
		AG_WidgetPutPixel(wid, px-y-1, py-x, color);
	}
	AG_WidgetPutPixel(wid, px-radius, py, color);
	AG_WidgetPutPixel(wid, px+radius, py, color);
	AG_UnlockView();
}

/* Render a 3D-style line. */
static void
Line2(void *wid, int x1, int y1, int x2, int y2, Uint32 color)
{
	AG_DrawLine(wid, x1, y1, x2, y2,
	    ColorShift(color, agHighColorShift));
	AG_DrawLine(wid, x1+1, y1+1, x2+1, y2+1,
	    ColorShift(color, agLowColorShift));
}

/*
 * Render a line segment between two points using the Bresenham algorithm
 * as presented by Foley & Van Dam [1990].
 */
static void
LineFB(void *widget, int x1, int y1, int x2, int y2, Uint32 color)
{
	AG_Widget *wid = widget;
	int dx, dy;
	int inc1, inc2;
	int x, y, d;
	int xend, yend;
	int xdir, ydir;

	dx = abs(x2 - x1);
	dy = abs(y2 - y1);

	AG_LockView();

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
	AG_UnlockView();
}

static __inline__ int
ClipHorizLine(AG_Widget *wid, int *x1, int *x2, int *y, int *dx)
{
	SDL_Rect *rd = &agView->v->clip_rect;

	if ((wid->rView.y1 + *y) <  rd->y ||
	    (wid->rView.y1 + *y) >= rd->y+rd->h) {
		return (1);
	}
	if (*x1 > *x2) {
		*dx = *x2;
		*x2 = *x1;
		*x1 = *dx;
	}
	if (wid->rView.x1 + *x1 < rd->x)
		*x1 = rd->x - wid->rView.x1;
	if (wid->rView.x1 + *x2 >= rd->x+rd->w)
		*x2 = rd->x + rd->w - wid->rView.x1 - 1;

	*dx = *x2 - *x1;
	return (*dx <= 0);
}

static void
LineH32(void *widget, int x1, int x2, int y, Uint32 c)
{
	AG_Widget *wid = widget;
	Uint8 *pDst, *pEnd;
	int dx;

	if (ClipHorizLine(wid, &x1, &x2, &y, &dx)) {
		return;
	}
	AG_LockView();
	pDst = (Uint8 *)agView->v->pixels +
	       (wid->rView.y1 + y)*agView->v->pitch +
	       ((wid->rView.x1 + x1) << 2);
	pEnd = pDst + (dx<<2);
	while (pDst < pEnd) {
		*(Uint32 *)pDst = c;
		pDst += 4;
	}
	AG_UnlockView();
}

static void
LineH24(void *widget, int x1, int x2, int y, Uint32 c)
{
	AG_Widget *wid = widget;
	Uint8 *pDst, *pEnd;
	int dx;
	
	if (ClipHorizLine(wid, &x1, &x2, &y, &dx)) {
		return;
	}
	AG_LockView();
	pDst = (Uint8 *)agView->v->pixels +
	    (wid->rView.y1 + y)*agView->v->pitch +
	    (wid->rView.x1 + x1)*3;
	pEnd = pDst + dx*3;
	while (pDst < pEnd) {
#if AG_BYTEORDER == AG_BIG_ENDIAN
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
	AG_UnlockView();
}

static void
LineH16(void *widget, int x1, int x2, int y, Uint32 c)
{
	AG_Widget *wid = widget;
	Uint8 *pDst, *pEnd;
	int dx;

	if (ClipHorizLine(wid, &x1, &x2, &y, &dx)) {
		return;
	}
	AG_LockView();
	pDst = (Uint8 *)agView->v->pixels +
	       (wid->rView.y1 + y)*agView->v->pitch +
	       ((wid->rView.x1 + x1)<<1);
	pEnd = pDst + (dx<<1);
	while (pDst < pEnd) {
		*(Uint16 *)pDst = c;
		pDst += 2;
	}
	AG_UnlockView();
}
		
static void
LineH8(void *widget, int x1, int x2, int y, Uint32 c)
{
	AG_Widget *wid = widget;
	Uint8 *pDst, *pEnd;
	int dx;

	if (ClipHorizLine(wid, &x1, &x2, &y, &dx)) {
		return;
	}
	AG_LockView();
	pDst = (Uint8 *)agView->v->pixels +
	       (wid->rView.y1 + y)*agView->v->pitch +
	       (wid->rView.x1 + x1);
	pEnd = pDst + dx;
	memset(pDst, c, pEnd-pDst);
	AG_UnlockView();
}

static __inline__ int
ClipVertLine(AG_Widget *wid, int *x, int *y1, int *y2, int *dy)
{
	SDL_Rect *rd = &agView->v->clip_rect;

	if ((wid->rView.x1 + *x) <  rd->x ||
	    (wid->rView.x1 + *x) >= rd->x+rd->w) {
		return (1);
	}
	if (*y1 > *y2) {
		*dy = *y2;
		*y2 = *y1;
		*y1 = *dy;
	}
	if (wid->rView.y1 + *y1 < rd->y)
		*y1 = rd->y - wid->rView.y1;
	if (wid->rView.y1 + *y2 >= rd->y+rd->h)
		*y2 = rd->y + rd->h - wid->rView.y1 - 1;

	*dy = *y2 - *y1;
	return (*dy <= 0);
}

static void
LineV32(void *widget, int x, int y1, int y2, Uint32 c)
{
	AG_Widget *wid = widget;
	Uint8 *pDst, *pEnd;
	int dy;

	if (ClipVertLine(wid, &x, &y1, &y2, &dy)) {
		return;
	}
	AG_LockView();
	pDst = (Uint8 *)agView->v->pixels +
	       (wid->rView.y1 + y1)*agView->v->pitch +
	       ((wid->rView.x1 + x)<<2);
	pEnd = pDst + dy*agView->v->pitch;
	while (pDst < pEnd) {
		*(Uint32 *)pDst = c;
		pDst += agView->v->pitch;
	}
	AG_UnlockView();
}

static void
LineV24(void *widget, int x, int y1, int y2, Uint32 c)
{
	AG_Widget *wid = widget;
	Uint8 *pDst, *pEnd;
	int dy;

	if (ClipVertLine(wid, &x, &y1, &y2, &dy)) {
		return;
	}
	AG_LockView();
	pDst = (Uint8 *)agView->v->pixels +
	       (wid->rView.y1 + y1)*agView->v->pitch +
	       (wid->rView.x1 + x)*3;
	pEnd = pDst + dy*agView->v->pitch;
	while (pDst < pEnd) {
#if AG_BYTEORDER == AG_BIG_ENDIAN
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
	AG_UnlockView();
}

static void
LineV16(void *widget, int x, int y1, int y2, Uint32 c)
{
	AG_Widget *wid = widget;
	Uint8 *pDst, *pEnd;
	int dy;

	if (ClipVertLine(wid, &x, &y1, &y2, &dy)) {
		return;
	}
	AG_LockView();
	pDst = (Uint8 *)agView->v->pixels +
	       (wid->rView.y1 + y1)*agView->v->pitch +
	       ((wid->rView.x1 + x)<<1);
	pEnd = pDst + dy*agView->v->pitch;
	while (pDst < pEnd) {
		*(Uint16 *)pDst = c;
		pDst += agView->v->pitch;
	}
	AG_UnlockView();
}

static void
LineV8(void *widget, int x, int y1, int y2, Uint32 c)
{
	AG_Widget *wid = widget;
	Uint8 *pDst, *pEnd;
	int dy;

	if (ClipVertLine(wid, &x, &y1, &y2, &dy)) {
		return;
	}
	AG_LockView();
	pDst = (Uint8 *)agView->v->pixels +
	       (wid->rView.y1 + y1)*agView->v->pitch +
	       (wid->rView.x1 + x);
	pEnd = pDst + dy*agView->v->pitch;
	while (pDst < pEnd) {
		*(Uint8 *)pDst = c;
		pDst += agView->v->pitch;
	}
	AG_UnlockView();
}

static void
LineBlendedFB(void *widget, int x1, int y1, int x2, int y2, Uint8 c[4],
    AG_BlendFn func)
{
	AG_Widget *wid = widget;
	int dx, dy;
	int inc1, inc2;
	int d, x, y;
	int xend, yend;
	int xdir, ydir;

	dx = abs(x2-x1);
	dy = abs(y2-y1);

	AG_LockView();

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
	AG_UnlockView();
}

/* Render a rectangle outline. */
static void
RectOutline(void *p, AG_Rect r, Uint32 color)
{
	AG_Widget *wid = p;
	int x2 = r.x+r.w-1;
	int y2 = r.y+r.h-1;

	AG_DrawLineH(wid, r.x, x2, r.y, color);
	AG_DrawLineH(wid, r.x, x2, y2, color);
	AG_DrawLineV(wid, r.x, r.y, y2, color);
	AG_DrawLineV(wid, x2, r.y, y2, color);
}

/* Render a filled rectangle. */
static void
RectFilledFB(void *p, AG_Rect r, Uint32 color)
{
	AG_Widget *wid = p;
	AG_Rect rd;

	rd.x = wid->rView.x1 + r.x;
	rd.y = wid->rView.y1 + r.y;
	rd.w = r.w;
	rd.h = r.h;
	AG_FillRect(agView->v, &rd, color);
}

/* Render an alpha blended rectangle. */
static void
RectBlendedFB(void *p, AG_Rect r, Uint8 c[4], AG_BlendFn func)
{
	AG_Widget *wid = p;
	int x, y;

	AG_LockView();
	for (y = 0; y < r.h; y++) {
		for (x = 0; x < r.w; x++) {
			AG_BLEND_RGBA2_CLIPPED(agView->v,
			    wid->rView.x1 + r.x + x,
			    wid->rView.y1 + r.y + y,
			    c[0], c[1], c[2], c[3],
			    func);
		}
	}
	AG_UnlockView();
}

/* Render a gimp-style background tiling. */
static void
Tiling(void *p, AG_Rect r, int tsz, int offs, Uint32 c1, Uint32 c2)
{
	AG_Widget *wid = p;
	int alt1 = 0, alt2 = 0;
	AG_Rect rt;

	rt.w = tsz;
	rt.h = tsz;

	/* XXX inelegant */
	for (rt.y = r.y-tsz+offs;
	     rt.y < r.y+r.h;
	     rt.y += tsz) {
		for (rt.x = r.x-tsz+offs;
		     rt.x < r.x+r.w;
		     rt.x += tsz) {
			if (alt1++ == 1) {
				AG_DrawRectFilled(wid, rt, c1);
				alt1 = 0;
			} else {
				AG_DrawRectFilled(wid, rt, c2);
			}
		}
		if (alt2++ == 1) {
			alt2 = 0;
		}
		alt1 = alt2;
	}
}

/* Render a [+] sign. */
static void
Plus(void *p, AG_Rect r, Uint8 c[4], AG_BlendFn func)
{
	int xcen = r.x + r.w/2;
	int ycen = r.y + r.h/2;

	AG_DrawLineBlended(p, xcen, r.y, xcen, r.y+r.h, c, func);
	AG_DrawLineBlended(p, r.x, ycen, r.x+r.w, ycen, c, func);
}

/* Render a [-] sign. */
static void
Minus(void *p, AG_Rect r, Uint8 c[4], AG_BlendFn func)
{
	int ycen = r.y+r.h/2;

	AG_DrawLineBlended(p, r.x, ycen, r.x+r.w, ycen, c, func);
}

#ifdef HAVE_OPENGL
/*
 * OpenGL versions of the primitives.
 */

static void
LineGL(void *p, int px1, int py1, int px2, int py2, Uint32 color)
{
	AG_Widget *wid = p;
	int x1 = wid->rView.x1 + px1;
	int y1 = wid->rView.y1 + py1;
	int x2 = wid->rView.x1 + px2;
	int y2 = wid->rView.y1 + py2;
	Uint8 r, g, b;

	AG_GetRGB(color, agVideoFmt, &r,&g,&b);
	glBegin(GL_LINES);
	glColor3ub(r, g, b);
	glVertex2s(x1, y1);
	glVertex2s(x2, y2);
	glEnd();
}

static void
LineHGL(void *p, int x1, int x2, int py, Uint32 color)
{
	AG_Widget *wid = p;
	int y = wid->rView.y1 + py;
	Uint8 r, g, b;

	AG_GetRGB(color, agVideoFmt, &r,&g,&b);
	glBegin(GL_LINES);
	glColor3ub(r, g, b);
	glVertex2s(wid->rView.x1 + x1, y);
	glVertex2s(wid->rView.x1 + x2, y);
	glEnd();
}

static void
LineVGL(void *p, int px, int y1, int y2, Uint32 color)
{
	AG_Widget *wid = p;
	int x = wid->rView.x1 + px;
	Uint8 r, g, b;

	AG_GetRGB(color, agVideoFmt, &r,&g,&b);
	glBegin(GL_LINES);
	glColor3ub(r, g, b);
	glVertex2s(x, wid->rView.y1 + y1);
	glVertex2s(x, wid->rView.y1 + y2);
	glEnd();
}

static void
LineBlendedGL(void *p, int px1, int py1, int px2, int py2, Uint8 c[4],
    AG_BlendFn func)
{
	AG_Widget *wid = p;
	int x1 = wid->rView.x1 + px1;
	int y1 = wid->rView.y1 + py1;
	int x2 = wid->rView.x1 + px2;
	int y2 = wid->rView.y1 + py2;
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
CircleGL(void *p, int x, int y, int radius, Uint32 color)
{
	AG_Widget *wid = p;
	int nedges = radius*2;
	int i;
	Uint8 r, g, b;
	
	AG_GetRGB(color, agVideoFmt, &r,&g,&b);

	glBegin(GL_LINE_LOOP);
	glColor3ub(r, g, b);
	for (i = 0; i < nedges; i++) {
		glVertex2f(wid->rView.x1 + x + radius*Cos((2*AG_PI*i)/nedges),
		           wid->rView.y1 + y + radius*Sin((2*AG_PI*i)/nedges));
	}
	glEnd();
}

static void
Circle2GL(void *p, int x, int y, int radius, Uint32 color)
{
	AG_Widget *wid = p;
	int nedges = radius*2;
	Uint8 r, g, b;
	int x1 = wid->rView.x1 + x;
	int y1 = wid->rView.y1 + y;
	int i;
	
	AG_GetRGB(color, agVideoFmt, &r,&g,&b);

	glBegin(GL_LINE_LOOP);
	glColor3ub(r, g, b);
	for (i = 0; i < nedges; i++) {
		glVertex2f(x1 + radius*Cos((2*AG_PI*i)/nedges),
		           y1 + radius*Sin((2*AG_PI*i)/nedges));
		glVertex2f(x1 + (radius+1)*Cos((2*AG_PI*i)/nedges),
		           y1 + (radius+1)*Sin((2*AG_PI*i)/nedges));
	}
	glEnd();
}

static void
RectGL(void *p, AG_Rect r, Uint32 color)
{
	AG_Widget *wid = p;
	Uint8 red, green, blue;
	int x1 = wid->rView.x1 + r.x;
	int y1 = wid->rView.y1 + r.y;
	int x2 = x1 + r.w - 1;
	int y2 = y1 + r.h - 1;

#if 0
	if (wid->flags & AG_WIDGET_CLIPPING) {
		if (x1 > wid->rView.x2 ||
		    y1 > wid->rView.y2) {
			return;
		}
		if (x1 < wid->rView.x1) { x1 = wid->rView.x1; }
		if (y1 < wid->rView.y1) { y1 = wid->rView.y1; }
		if (x2 > wid->rView.x2) { x2 = wid->rView.x2; }
		if (y2 > wid->rView.y2) { y2 = wid->rView.y2; }
	}
#endif
	AG_GetRGB(color, agVideoFmt, &red,&green,&blue);
	glBegin(GL_POLYGON);
	glColor3ub(red, green, blue);
	glVertex2i(x1, y1);
	glVertex2i(x2, y1);
	glVertex2i(x2, y2);
	glVertex2i(x1, y2);
	glEnd();
}

static void
RectBlendedGL(void *p, AG_Rect r, Uint8 c[4], AG_BlendFn func)
{
	AG_Widget *wid = p;
	int x1 = wid->rView.x1 + r.x;
	int y1 = wid->rView.y1 + r.y;
	int x2 = x1+r.w;
	int y2 = y1+r.h;
	GLboolean blend_save;
	GLint sfac_save, dfac_save;

#if 0
	if (wid->flags & AG_WIDGET_CLIPPING) {
		if (x1 > rView.x2 || y1 > rView.y2) {
			return;
		}
		if (x1 < wid->rView.x1) { x1 = wid->rView.x1; }
		if (y1 < wid->rView.y1) { y1 = wid->rView.y1; }
		if (x2 > wid->rView.x2) { x2 = wid->rView.x2; }
		if (y2 > wid->rView.y2) { y2 = wid->rView.y2; }
	}
#endif
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
BoxDisabledGL(void *p, AG_Rect r, int z, Uint32 c1, Uint32 c2)
{
	AG_Widget *wid = p;
	int stippleEnabled = glIsEnabled(GL_POLYGON_STIPPLE);
	Uint32 cBg;

	glEnable(GL_POLYGON_STIPPLE);
	glPushAttrib(GL_POLYGON_STIPPLE_BIT);
	glPolygonStipple(DisabledStipple);

	if (AG_WidgetFocused(wid)) {
		cBg = ColorShift(c2, (z<0) ? agFocusSunkColorShift :
		                             agFocusRaisedColorShift);
	} else {
		cBg = ColorShift(c2, (z<0) ? agNofocusSunkColorShift :
		                             agNofocusRaisedColorShift);
	}
	AG_DrawRectFilled(wid, r, c1);
	AG_DrawFrame(wid, r, z, c1);

	glPopAttrib();
	if (!stippleEnabled) { glDisable(GL_POLYGON_STIPPLE); }
}

/* Render a 3D-style box with rounded edges. */
static void
BoxRoundedGL(void *p, AG_Rect r, int z, int rad, Uint32 cBg)
{
	AG_Widget *wid = p;
	Uint8 red, green, blue;

	/* TODO */

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(wid->rView.x1 + r.x,
	             wid->rView.y1 + r.y, 0);
	glBegin(GL_POLYGON);
	{
		AG_GetRGB(cBg, agVideoFmt, &red,&green,&blue);
		glColor3ub(red, green, blue);
		glVertex2i(0, r.h);
		glVertex2i(0, rad);
		glVertex2i(rad, 0);
		glVertex2i(r.w-rad, 0);
		glVertex2i(r.w, rad);
		glVertex2i(r.w, r.h);
	}
	glEnd();
	if (z >= 0) {
		glBegin(GL_LINE_STRIP);
		{
			AG_GetRGB(ColorShift(cBg, agHighColorShift),
			                      agVideoFmt, &red,&green,&blue);
			glColor3ub(red, green, blue);
			glVertex2i(0, r.h);
			glVertex2i(0, rad);
			glVertex2i(rad, 0);
		}
		glEnd();
		glBegin(GL_LINES);
		{
			AG_GetRGB(ColorShift(cBg, agLowColorShift),
			                      agVideoFmt, &red,&green,&blue);
			glColor3ub(red, green, blue);
			glVertex2i(r.w-1, r.h);
			glVertex2i(r.w-1, rad);
		}
		glEnd();
	}
	glPopMatrix();
}

/* Render a 3D-style box with rounded top edges. */
static void
BoxRoundedTopGL(void *p, AG_Rect r, int z, int rad, Uint32 cBg)
{
	AG_Widget *wid = p;
	Uint8 red, green, blue;

	/* TODO */

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(wid->rView.x1 + r.x,
	             wid->rView.y1 + r.y, 0);
	glBegin(GL_POLYGON);
	{
		AG_GetRGB(cBg, agVideoFmt, &red,&green,&blue);
		glColor3ub(red, green, blue);
		glVertex2i(0, r.h);
		glVertex2i(0, rad);
		glVertex2i(rad, 0);
		glVertex2i(r.w-rad, 0);
		glVertex2i(r.w, rad);
		glVertex2i(r.w, r.h);
	}
	glEnd();
	if (z >= 0) {
		glBegin(GL_LINE_STRIP);
		{
			AG_GetRGB(ColorShift(cBg, agHighColorShift),
			                     agVideoFmt, &red,&green,&blue);
			glColor3ub(red, green, blue);
			glVertex2i(0, r.h);
			glVertex2i(0, rad);
			glVertex2i(rad, 0);
		}
		glEnd();
		glBegin(GL_LINES);
		{
			AG_GetRGB(ColorShift(cBg, agLowColorShift),
			                     agVideoFmt, &red,&green,&blue);
			glColor3ub(red, green, blue);
			glVertex2i(r.w-1, r.h);
			glVertex2i(r.w-1, rad);
		}
		glEnd();
	}
	glPopMatrix();
}

static void
ArrowUpGL(void *p, int x, int y, int h, Uint32 c1, Uint32 c2)
{
	AG_Widget *wid = p;
	Uint8 r, g, b;
	int h2 = h>>1;

	AG_GetRGB(c1, agVideoFmt, &r,&g,&b);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(wid->rView.x1, wid->rView.y1, 0);
	glBegin(GL_POLYGON);
	{
		glColor3ub(r, g, b);
		glVertex2i(x,		y - h2);
		glVertex2i(x - h2,	y + h2);
		glVertex2i(x + h2,	y + h2);
	}
	glEnd();
	glPopMatrix();
}

static void
ArrowDownGL(void *p, int x, int y, int h, Uint32 c1, Uint32 c2)
{
	AG_Widget *wid = p;
	Uint8 r, g, b;
	int h2 = h>>1;

	AG_GetRGB(c1, agVideoFmt, &r,&g,&b);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(wid->rView.x1, wid->rView.y1, 0);
	glBegin(GL_POLYGON);
	{
		glColor3ub(r, g, b);
		glVertex2i(x - h2,	y - h2);
		glVertex2i(x + h2,	y - h2);
		glVertex2i(x,		y + h2);
	}
	glEnd();
	glPopMatrix();
}

static void
ArrowLeftGL(void *p, int x, int y, int h, Uint32 c1, Uint32 c2)
{
	AG_Widget *wid = p;
	Uint8 r, g, b;
	int h2 = h>>1;

	AG_GetRGB(c1, agVideoFmt, &r,&g,&b);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(wid->rView.x1, wid->rView.y1, 0);
	glBegin(GL_POLYGON);
	{
		glColor3ub(r, g, b);
		glVertex2i(x - h2,	y);
		glVertex2i(x + h2,	y + h2);
		glVertex2i(x + h2,	y - h2);
	}
	glEnd();
	glPopMatrix();
}

static void
ArrowRightGL(void *p, int x, int y, int h, Uint32 c1, Uint32 c2)
{
	AG_Widget *wid = p;
	Uint8 r, g, b;
	int h2 = h>>1;

	AG_GetRGB(c1, agVideoFmt, &r,&g,&b);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(wid->rView.x1, wid->rView.y1, 0);
	glBegin(GL_POLYGON);
	{
		glColor3ub(r, g, b);
		glVertex2i(x + h2,	y);
		glVertex2i(x - h2,	y + h2);
		glVertex2i(x - h2,	y - h2);
	}
	glEnd();
	glPopMatrix();
}

#endif /* HAVE_OPENGL */

void
AG_InitPrimitives(void)
{
#ifdef HAVE_OPENGL
	memset(DisabledStipple, 0xaa, sizeof(DisabledStipple));
#endif
	agPrim.Box = Box;
	agPrim.Frame = Frame;
	agPrim.FrameBlended = FrameBlended;
	agPrim.RectOutline = RectOutline;
	agPrim.Plus = Plus;
	agPrim.Minus = Minus;
	agPrim.Line2 = Line2;
	agPrim.Tiling = Tiling;

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		agPrim.Line = LineGL;
		agPrim.LineH = LineHGL;
		agPrim.LineV = LineVGL;
		agPrim.LineBlended = LineBlendedGL;
		agPrim.RectFilled = RectGL;
		agPrim.RectBlended = RectBlendedGL;
		agPrim.Circle = CircleGL;
		agPrim.Circle2 = Circle2GL;
		agPrim.BoxRounded = BoxRoundedGL;
		agPrim.BoxRoundedTop = BoxRoundedTopGL;
		agPrim.BoxDisabled = BoxDisabledGL;
		agPrim.ArrowUp = ArrowUpGL;
		agPrim.ArrowDown = ArrowDownGL;
		agPrim.ArrowLeft = ArrowLeftGL;
		agPrim.ArrowRight = ArrowRightGL;
	} else
#endif
	{
		agPrim.Line = LineFB;
		agPrim.LineBlended = LineBlendedFB;
		agPrim.RectFilled = RectFilledFB;
		agPrim.RectBlended = RectBlendedFB;
		agPrim.Circle = CircleFB;
		agPrim.Circle2 = Circle2FB;
		agPrim.BoxRounded = BoxRoundedFB;
		agPrim.BoxRoundedTop = BoxRoundedTopFB;
		agPrim.BoxDisabled = BoxDisabledFB;
		agPrim.ArrowUp = ArrowUpFB;
		agPrim.ArrowDown = ArrowDownFB;
		agPrim.ArrowLeft = ArrowLeftFB;
		agPrim.ArrowRight = ArrowRightFB;

		switch (agVideoFmt->BytesPerPixel) {
		case 4:
			agPrim.LineH = LineH32;
			agPrim.LineV = LineV32;
			break;
		case 3:
			agPrim.LineH = LineH24;
			agPrim.LineV = LineV24;
			break;
		case 2:
			agPrim.LineH = LineH16;
			agPrim.LineV = LineV16;
			break;
		case 1:
			agPrim.LineH = LineH8;
			agPrim.LineV = LineV8;
			break;
		}
	}
}
