/*	$Csoft: primitive.c,v 1.45 2003/05/22 05:45:46 vedge Exp $	    */

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

/* Add to the rgb components of a pixel. */
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
 * Write to a pixel in video memory, if it is inside the clipping rect.
 * The display surface must be locked.
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
 * Write to two pixels in video memory, if they are inside the clipping rect.
 * The display surface must be locked.
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
	SDL_Rect rd;

	lcol = (z < 0) ?
	    alter_color(color, -60, -60, -60) :
	    alter_color(color, 60, 60, 60);
	rcol = (z < 0) ?
	    alter_color(color, 60, 60, 60) :
	    alter_color(color, -60, -60, -60);
	bcol = (z < 0) ?
	    alter_color(color, -20, -20, -20) :
	    color;

	if (WIDGET_FOCUSED(wid)) {
		bcol = alter_color(bcol, 6, 6, 15);
	}
	/* Background */
	rd.x = xoffs;
	rd.y = yoffs;
	rd.w = w;
	rd.h = h;
	primitives.rect_filled(wid, &rd, bcol);

	primitives.line(wid,			/* Top */
	    xoffs, yoffs,
	    xoffs+w-1, yoffs, lcol);
	primitives.line(wid,			/* Left */
	    xoffs, yoffs,
	    xoffs, yoffs+h-1, lcol);
	primitives.line(wid,			/* Bottom */
	    xoffs, yoffs+h-1,
	    xoffs+w-1, yoffs+h-1, rcol);
	primitives.line(wid,			/* Right */
	    xoffs+w-1, yoffs,
	    xoffs+w-1, yoffs+h-1, rcol);
}

/* Render a 3D-style frame. */
static void
frame(void *p, int xoffs, int yoffs, int w, int h, Uint32 color)
{
	struct widget *wid = p;

	primitives.line(wid,			/* Top */
	    xoffs, yoffs,
	    xoffs+w-1, yoffs, color);
	primitives.line(wid,			/* Left */
	    xoffs, yoffs,
	    xoffs, yoffs+h-1, color);
	primitives.line(wid,			/* Bottom */
	    xoffs, yoffs+h-1,
	    xoffs+w-1, yoffs+h-1, color);
	primitives.line(wid,			/* Right */
	    xoffs+w-1, yoffs,
	    xoffs+w-1, yoffs+h-1, color);
}

/* Render a circle. */
static void
circle_bresenham(void *wid, int xoffs, int yoffs, int w, int h, int radius,
    Uint32 color)
{
	int x = 0, y, cx, cy, e = 0, u = 1, v;

	y = radius;
	cx = w/2 + xoffs;
	cy = h/2 + yoffs;
	v = 2*radius - 1;

	SDL_LockSurface(view->v);
	while (x <= y) {
		WIDGET_PUT_PIXEL(wid, cx + x, cy + y, color);	/* SE */
		WIDGET_PUT_PIXEL(wid, cx + x, cy - y, color);	/* NE */
		WIDGET_PUT_PIXEL(wid, cx - x, cy + y, color);	/* SW */
		WIDGET_PUT_PIXEL(wid, cx - x, cy - y, color);	/* NW */
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
		WIDGET_PUT_PIXEL(wid, cx + y, cy + x, color);	/* SE */
		WIDGET_PUT_PIXEL(wid, cx + y, cy - x, color);	/* NE */
		WIDGET_PUT_PIXEL(wid, cx - y, cy + x, color);	/* SW */
		WIDGET_PUT_PIXEL(wid, cx - y, cy - x, color);	/* NW */
	}
	WIDGET_PUT_PIXEL(wid, 2, cy, color);
	WIDGET_PUT_PIXEL(wid, w-2, cy, color);
	SDL_UnlockSurface(view->v);
}

static void
wline(void *wid, int width, int px1, int py1, int px2, int py2, Uint32 color)
{
	int i;

	for (i = 0; i < width; i++) {
		primitives.line(wid, px1+i, py1+i, px2+i, py2+i,
		    alter_color(color, i*90, i*90, i*90));
	}
}

/* Render a segment from px1,py1 to px2,py2. */
static void
line_bresenham(void *wid, int px1, int py1, int px2, int py2, Uint32 color)
{
	int dx, dy, dpr, dpru, p;
	int x1, x2, y1, y2;
	int yinc = view->v->pitch;
	int xyinc = vfmt->BytesPerPixel + yinc;
	Uint8 *fb1, *fb2;

	x1 = px1 + WIDGET(wid)->win->rd.x + WIDGET(wid)->x;
	y1 = py1 + WIDGET(wid)->win->rd.y + WIDGET(wid)->y;
	x2 = px2 + WIDGET(wid)->win->rd.x + WIDGET(wid)->x;
	y2 = py2 + WIDGET(wid)->win->rd.y + WIDGET(wid)->y;

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

	primitives.wline(wid, 2,		/* Top */
	    x, y,
	    x + w - 1, y, color);
	primitives.wline(wid, 2,		/* Bottom */
	    x, y + h - 1,
	    x + w - 1, y + h - 1, color);
	primitives.wline(wid, 2,		/* Left */
	    x, y,
	    x, y + h - 1, color);
	primitives.wline(wid, 2,		/* Right */
	    x+w - 1, y,
	    x+w - 1, y + h - 1, color);
}

/* Render a filled rectangle. */
static void
rect_filled(void *p, SDL_Rect *rd, Uint32 color)
{
	SDL_Rect nrd = *rd;

	nrd.x += WIDGET(p)->win->rd.x + WIDGET(p)->x;
	nrd.y += WIDGET(p)->win->rd.y + WIDGET(p)->y;

	SDL_FillRect(view->v, &nrd, color);
}

static void
plus(void *p, int x, int y, int w, int h, Uint32 color)
{
	struct widget *wid = p;
	int xcenter = x+w/2;
	int ycenter = y+h/2;

	primitives.wline(wid, 2,
	    xcenter,	y,
	    xcenter,	y + h,
	    color);
	primitives.wline(wid, 2,
	    x,		ycenter,
	    x + w,	ycenter,
	    color);
}

static void
minus(void *p, int x, int y, int w, int h, Uint32 color)
{
	struct widget *wid = p;
	int ycenter = y+h/2;

	primitives.wline(wid, 2,
	    x,		ycenter,
	    x + w,	ycenter,
	    color);
}

#ifdef HAVE_OPENGL

/* Render a line using OpenGL. */
static void
line_opengl(void *wid, int x1, int y1, int x2, int y2, Uint32 color)
{
	Uint8 r, g, b;
	
	SDL_GetRGB(color, vfmt, &r, &g, &b);

	x1 += WIDGET(wid)->win->rd.x + WIDGET(wid)->x;
	y1 += WIDGET(wid)->win->rd.y + WIDGET(wid)->y;
	x2 += WIDGET(wid)->win->rd.x + WIDGET(wid)->x;
	y2 += WIDGET(wid)->win->rd.y + WIDGET(wid)->y;

	glBegin(GL_LINES);
	{
		glColor3ub(r, g, b);
		glVertex2s(x1, y1);
		glVertex2s(x2, y2);
	}
	glEnd();
}

/* Render a filled rectangle using OpenGL. */
static void
rect_opengl(void *p, SDL_Rect *rd, Uint32 color)
{
	SDL_Rect nrd = *rd;
	Uint8 r, g, b;

	nrd.x += WIDGET(p)->win->rd.x + WIDGET(p)->x;
	nrd.y += WIDGET(p)->win->rd.y + WIDGET(p)->y;

	SDL_GetRGB(color, vfmt, &r, &g, &b);
	glColor3ub(r, g, b);
	glRecti(nrd.x, nrd.y, nrd.x+nrd.w, nrd.y+nrd.h);
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
	primitives.wline = wline;

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

