/*	$Csoft: primitive.c,v 1.47 2003/05/26 03:03:33 vedge Exp $	    */

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
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

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/label.h>
#include <engine/widget/tlist.h>
#include <engine/widget/primitive.h>

struct primitive_ops primitives;

/* Add to individual RGB components of a pixel. */
static __inline__ Uint32
alter_color(Uint32 pixel, Sint8 r, Sint8 g, Sint8 b)
{
	Uint32 rv = 0;
	int v1, v2;

	v1 = ((pixel & vfmt->Rmask) >> vfmt->Rshift);
	v2 = ((v1 << vfmt->Rloss) + (v1 >> (8 - vfmt->Rloss))) + r;
	if (v2 < 0) {
		v2 = 0;
	} else if (v2 > 255) {
		v2 = 255;
	}
	rv |= (v2 >> vfmt->Rloss) << vfmt->Rshift;

	v1 = ((pixel & vfmt->Gmask) >> vfmt->Gshift);
	v2 = ((v1 << vfmt->Gloss) + (v1 >> (8 - vfmt->Gloss))) + g;
	if (v2 < 0) {
		v2 = 0;
	} else if (v2 > 255) {
		v2 = 255;
	}
	rv |= (v2 >> vfmt->Gloss) << vfmt->Gshift;

	v1 = ((pixel & vfmt->Bmask) >> vfmt->Bshift);
	v2 = ((v1 << vfmt->Bloss) + (v1 >> (8 - vfmt->Bloss))) + b;
	if (v2 < 0) {
		v2 = 0;
	} else if (v2 > 255) {
		v2 = 255;
	}
	rv |= (v2 >> vfmt->Bloss) << vfmt->Bshift;

	rv |= vfmt->Amask;
	return (rv);
}

/*
 * Write to a pixel to absolute view coordinates x,y.
 * Clipping is done, the display surface must be locked.
 */
static __inline__ void
put_pixel1(Uint8 *dst, Uint32 color, int x, int y)
{
	if (!VIEW_INSIDE_CLIP_RECT(view->v, x, y))
		return;

	switch (vfmt->BytesPerPixel) {
		_VIEW_PUTPIXEL_32(dst, color);
		_VIEW_PUTPIXEL_24(dst, color);
		_VIEW_PUTPIXEL_16(dst, color);
		_VIEW_PUTPIXEL_8(dst, color);
	}
}

/*
 * Write two pixels to absolute view coordinates x1,y1 and x2,y2.
 * Clipping is done, the display surface must be locked.
 */
static __inline__ void
put_pixel2(Uint8 *dst1, int x1, int y1, Uint8 *dst2, int x2, int y2,
    Uint32 color)
{
	if (VIEW_INSIDE_CLIP_RECT(view->v, x1, y1)) {
		switch (vfmt->BytesPerPixel) {
			_VIEW_PUTPIXEL_32(dst1, color);
			_VIEW_PUTPIXEL_24(dst1, color);
			_VIEW_PUTPIXEL_16(dst1, color);
			_VIEW_PUTPIXEL_8(dst1, color);
		}
	}

	if (VIEW_INSIDE_CLIP_RECT(view->v, x2, y2)) {
		switch (vfmt->BytesPerPixel) {
			_VIEW_PUTPIXEL_32(dst2, color);
			_VIEW_PUTPIXEL_24(dst2, color);
			_VIEW_PUTPIXEL_16(dst2, color);
			_VIEW_PUTPIXEL_8(dst2, color);
		}
	}
}

/* Render a 3D-style box. */
static void
box(void *p, int xoffs, int yoffs, int w, int h, int z, Uint32 color)
{
	struct widget *wid = p;
	Uint32 lcol, rcol, bcol;

	lcol = (z < 0) ?
	    alter_color(color, -60, -60, -60) :
	    alter_color(color, 60, 60, 60);
	rcol = (z < 0) ?
	    alter_color(color, 60, 60, 60) :
	    alter_color(color, -60, -60, -60);
	bcol = (z < 0) ?
	    alter_color(color, -20, -20, -20) :
	    color;

	if (widget_holds_focus(wid))
		bcol = alter_color(bcol, 6, 6, 15);

	primitives.rect_filled(wid,
	    xoffs,
	    yoffs,
	    w,
	    h,
	    bcol);

	primitives.line(wid,			/* Top */
	    xoffs,
	    yoffs,
	    xoffs + w - 1,
	    yoffs,
	    lcol);
	primitives.line(wid,			/* Left */
	    xoffs,
	    yoffs,
	    xoffs,
	    yoffs + h - 1,
	    lcol);
	primitives.line(wid,			/* Bottom */
	    xoffs,
	    yoffs + h - 1,
	    xoffs + w - 1,
	    yoffs + h - 1,
	    rcol);
	primitives.line(wid,			/* Right */
	    xoffs + w - 1,
	    yoffs,
	    xoffs + w - 1,
	    yoffs + h - 1,
	    rcol);
}

/* Render a 3D-style frame. */
static void
frame(void *p, int xoffs, int yoffs, int w, int h, Uint32 color)
{
	struct widget *wid = p;
	Uint32 lcol, rcol;

	lcol = alter_color(color, 80, 80, 80);
	rcol = alter_color(color, -80, -80, -80);

	primitives.line(wid,			/* Top */
	    xoffs,
	    yoffs,
	    xoffs + w - 1,
	    yoffs,
	    lcol);
	primitives.line(wid,			/* Left */
	    xoffs,
	    yoffs,
	    xoffs,
	    yoffs + h - 1,
	    lcol);
	primitives.line(wid,			/* Bottom */
	    xoffs,
	    yoffs + h - 1,
	    xoffs + w - 1,
	    yoffs + h - 1,
	    rcol);
	primitives.line(wid,			/* Right */
	    xoffs + w - 1,
	    yoffs,
	    xoffs + w - 1,
	    yoffs + h - 1,
	    rcol);
}

/* Render a circle. */
static void
circle_bresenham(void *p, int xoffs, int yoffs, int w, int h, int radius,
    Uint32 color)
{
	struct widget *wid = p;
	int x = 0;
	int y = radius;
	int cx = w/2 + xoffs + wid->cx;
	int cy = h/2 + yoffs + wid->cy;
	int v = 2*radius - 1;
	int e = 0, u = 1;

	SDL_LockSurface(view->v);
	while (x < y) {
		widget_put_pixel(p, cx + x, cy + y, color);	/* SE */
		widget_put_pixel(p, cx + x, cy - y, color);	/* NE */
		widget_put_pixel(p, cx - x, cy + y, color);	/* SW */
		widget_put_pixel(p, cx - x, cy - y, color);	/* NW */

		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		if (++x > y)
			break;

		widget_put_pixel(p, cx + y, cy + x, color);	/* SE */
		widget_put_pixel(p, cx + y, cy - x, color);	/* NE */
		widget_put_pixel(p, cx - y, cy + x, color);	/* SW */
		widget_put_pixel(p, cx - y, cy - x, color);	/* NW */
	}
	SDL_UnlockSurface(view->v);
}

/* Render two lines with +50,50,50 RGB difference. */
static void
line2(void *wid, int x1, int y1, int x2, int y2, Uint32 color)
{
	primitives.line(wid, x1, y1, x2, y2, color);
	primitives.line(wid, x1+1, y1+1, x2+1, y2+1,
	    alter_color(color, 50, 50, 50));
}

/* Render a segment from x1,y1 to x2,y2. */
static void
line_bresenham(void *widget, int px1, int py1, int px2, int py2, Uint32 color)
{
	struct widget *wid = widget;
	int dx, dy, dpr, dpru, p;
	int yinc = view->v->pitch;
	int xyinc = vfmt->BytesPerPixel + yinc;
	Uint8 *fb1, *fb2;
	int x1 = wid->cx + px1;
	int y1 = wid->cy + py1;
	int x2 = wid->cx + px2;
	int y2 = wid->cy + py2;

	if (x1 > x2) {		/* Swap inverse X coords */
		dx = x1;
		x1 = x2;
		x2 = dx;
	}
	if (y1 > y2) {		/* Swap inverse Y coords */
		dx = y1;
		y1 = y2;
		y2 = dx;
	}
	dx = x2 - x1;
	dy = y2 - y1;

	fb1 = (Uint8 *)view->v->pixels +
	    y1*view->v->pitch +
	    x1*view->v->format->BytesPerPixel;
	fb2 = (Uint8 *)view->v->pixels +
	    y2*view->v->pitch +
	    x2*view->v->format->BytesPerPixel;

	SDL_LockSurface(view->v);

	if (dy > dx) {
		goto indep_y;
	}

/* indep_x: */
	dpr = dy + dy;
	p = -dx;
	dpru = p + p;
	dy = dx >> 1;

xloop:
	put_pixel2(fb1, x1, y1, fb2, x2, y2, color);

	if ((p += dpr) > 0) {
		goto right_and_up;
	}

/* up: */
	x1++;
	x2--;
	fb1 += vfmt->BytesPerPixel;
	fb2 -= vfmt->BytesPerPixel;
	if ((dy = dy - 1) >= 0) {
		goto xloop;
	}
	put_pixel1(fb1, color, x1, y1);
	if ((dx & 1) == 0) {
		goto done;
	}
	put_pixel1(fb2, color, x2, y2);
	goto done;

right_and_up:
	x1++;
	y1++;
	x2--;
	y2--;
	fb1 += xyinc;
	fb2 -= xyinc;
	p += dpru;
	if (--dy >= 0) {
		goto xloop;
	}
	put_pixel1(fb1, color, x1, y1);
	if ((dx & 1) == 0) {
		goto done;
	}
	put_pixel1(fb2, color, x2, y2);
	goto done;

indep_y:
	dpr = dx+dx;
	p = -dy;
	dpru = p+p;
	dx = dy>>1;

yloop:
	put_pixel2(fb1, x1, y1, fb2, x2, y2, color);

	if ((p += dpr) > 0) {
		goto right_and_up_2;
	}
/* up: */
	y1++;
	y2--;
	fb1 += yinc;
	fb2 -= yinc;
	if ((dx = dx - 1) >= 0) {
		goto yloop;
	}
	put_pixel1(fb2, color, x2, y2);
	goto done;

right_and_up_2:
	x1++;
	y1++;
	x2--;
	y2--;
	fb1 += xyinc;
	fb2 -= xyinc;
	p += dpru;
	if ((dx = dx - 1) >= 0) {
		goto yloop;
	}
	put_pixel1(fb1, color, x1, y1);
	if ((dy & 1) == 0) {
		goto done;
	}
	put_pixel1(fb2, color, x2, y2);
done:
	SDL_UnlockSurface(view->v);
}

/* Render an outlined rectangle. */
static void
rect_outlined(void *p, int x, int y, int w, int h, Uint32 color)
{
	struct widget *wid = p;

	primitives.line(wid, 		/* Top */
	    x,
	    y,
	    x + w - 1,
	    y,
	    color);
	primitives.line(wid, 		/* Bottom */
	    x,
	    y + h - 1,
	    x + w - 1,
	    y + h - 1,
	    color);
	primitives.line(wid, 		/* Left */
	    x,
	    y,
	    x,
	    y + h - 1,
	    color);
	primitives.line(wid, 		/* Right */
	    x + w - 1,
	    y,
	    x + w - 1,
	    y + h - 1,
	    color);
}

/* Render a filled rectangle. */
static void
rect_filled(void *p, int x, int y, int w, int h, Uint32 color)
{
	struct widget *wid = p;
	SDL_Rect rd;

	rd.x = wid->cx + x;
	rd.y = wid->cy + y;
	rd.w = w;
	rd.h = h;
	SDL_FillRect(view->v, &rd, color);
}

/* Render a [+] sign scaled to a w,h box (for trees). */
static void
plus(void *p, int x, int y, int w, int h, Uint32 color)
{
	int xcenter = x + w/2;
	int ycenter = y + h/2;

	primitives.line2(p,
	    xcenter,
	    y,
	    xcenter,
	    y + h,
	    color);
	primitives.line2(p,
	    x,
	    ycenter,
	    x + w,
	    ycenter,
	    color);
}

/* Render a [-] sign scaled to a w,h box (for trees). */
static void
minus(void *p, int x, int y, int w, int h, Uint32 color)
{
	struct widget *wid = p;
	int ycenter = y+h/2;

	primitives.line2(wid,
	    x,
	    ycenter,
	    x + w,
	    ycenter,
	    color);
}

#ifdef HAVE_OPENGL

/* Render a line using OpenGL. */
static void
line_opengl(void *p, int x1, int y1, int x2, int y2, Uint32 color)
{
	struct widget *wid = p;
	Uint8 r, g, b;
	
	x1 += wid->cx;
	y1 += wid->cy;
	x2 += wid->cx;
	y2 += wid->cy;

	SDL_GetRGB(color, vfmt, &r, &g, &b);
	glBegin(GL_LINES);
	glColor3ub(r, g, b);
	glVertex2s(x1, y1);
	glVertex2s(x2, y2);
	glEnd();
}

/* Render a filled rectangle using OpenGL. */
static void
rect_opengl(void *p, int x, int y, int w, int h, Uint32 color)
{
	struct widget *wid = p;
	Uint8 r, g, b;

	SDL_GetRGB(color, vfmt, &r, &g, &b);
	glColor3ub(r, g, b);
	glRecti(
	    wid->cx + x,
	    wid->cy + y,
	    wid->cx + x + w,
	    wid->cy + y + h);
}

#endif /* HAVE_OPENGL */

void
primitives_init(void)
{
	primitives.box = box;
	primitives.frame = frame;
	primitives.circle = circle_bresenham;
	primitives.rect_outlined = rect_outlined;
	primitives.plus = plus;
	primitives.minus = minus;
	primitives.line2 = line2;

#ifdef HAVE_OPENGL
	if (view->opengl) {
		primitives.line = line_opengl;
		primitives.rect_filled = rect_opengl;
	} else
#endif
	{
		primitives.line = line_bresenham;
		primitives.rect_filled = rect_filled;
	}
}

