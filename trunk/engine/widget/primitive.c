/*	$Csoft: primitive.c,v 1.54 2004/04/09 08:05:43 vedge Exp $	    */

/*
 * Copyright (c) 2002, 2003, 2004 CubeSoft Communications, Inc.
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

static __inline__ void
clip_pixel(SDL_Surface *su, Uint8 **p)
{
	Uint8 *end = (Uint8 *)su->pixels + su->h*su->pitch - su->format->BytesPerPixel;

	if (*p > end)
		*p = end;
}

/*
 * Write to a pixel to absolute view coordinates x,y.
 * The display surface must be locked; clipping is done.
 */
static __inline__ void
put_pixel1(Uint8 *dst, Uint32 color)
{
	Uint8 *d = dst;

	clip_pixel(view->v, &d);

	switch (vfmt->BytesPerPixel) {
	case 4:
		*(Uint32 *)d = color;
		break;
	case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		d[0] = (color>>16) & 0xff;
		d[1] = (color>>8) & 0xff;
		d[2] = color & 0xff;
#else
		d[0] = color & 0xff;
		d[1] = (color>>8) & 0xff;
		d[2] = (color>>16) & 0xff;
#endif
		break;
	case 2:
		*(Uint16 *)d = color;
		break;
	case 1:
		*d = color;
		break;
	}
}

/*
 * Write two pixels to absolute view coordinates x1,y1 and x2,y2.
 * The display surface must be locked; clipping is done.
 */
static __inline__ void
put_pixel2(Uint8 *dst1, Uint8 *dst2, Uint32 color)
{
	Uint8 *d1 = dst1;
	Uint8 *d2 = dst2;

	clip_pixel(view->v, &d1);
	clip_pixel(view->v, &d2);

	switch (vfmt->BytesPerPixel) {
	case 4:
		*(Uint32 *)d1 = color;
		*(Uint32 *)d2 = color;
		break;
	case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		d2[0] = d1[0] = (color>>16) & 0xff;
		d2[1] = d1[1] = (color>>8) & 0xff;
		d2[2] = d1[2] = color & 0xff;
#else
		d2[0] = d1[0] = color & 0xff;
		d2[1] = d1[1] = (color>>8) & 0xff;
		d2[2] = d1[2] = (color>>16) & 0xff;
#endif
		break;
	case 2:
		*(Uint16 *)d1 = color;
		*(Uint16 *)d2 = color;
		break;
	case 1:
		*d1 = color;
		*d2 = color;
		break;
	}
}

/*
 * Render a 3D-style box with a RGB difference of -60,-60,-60 and 60,60,60
 * between the two border lines. Offset the background color and swap the
 * border colors depending on the z value.
 */
static void
box(void *p, int xoffs, int yoffs, int w, int h, int z, int ncolor)
{
	struct widget *wid = p;
	Uint32 color = WIDGET_COLOR(wid, ncolor);
	int lcol, rcol, bcol;

	if (ncolor >= WIDGET_COLORS_MAX)
		fatal("color stack oflow");

	lcol = widget_push_color(wid, (z < 0) ?
	    alter_color(color, -60, -60, -60) :
	    alter_color(color, 60, 60, 60));
	rcol = widget_push_color(wid, (z < 0) ?
	    alter_color(color, 60, 60, 60) :
	    alter_color(color, -60, -60, -60));

	if (widget_holds_focus(wid)) {
		bcol = widget_push_color(wid, (z < 0) ?
		    alter_color(color, -14, -13, -4) :
		    alter_color(color, 6, 7, 16));
	} else {
		bcol = widget_push_color(wid, (z < 0) ?
		    alter_color(color, -20, -20, -20) :
		    alter_color(color, 10, 10, 10));
	}

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

	widget_pop_color(wid);
	widget_pop_color(wid);
	widget_pop_color(wid);
}

/*
 * Render a 3D-style frame with a RGB difference of -80,-80,-80 and 80,80,80
 * between the two border lines.
 */
static void
frame(void *p, int xoffs, int yoffs, int w, int h, int ncolor)
{
	struct widget *wid = p;
	Uint32 color = WIDGET_COLOR(wid, ncolor);
	int lcol, rcol;

	lcol = widget_push_color(wid, alter_color(color, 80, 80, 80));
	rcol = widget_push_color(wid, alter_color(color, -80, -80, -80));

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

	widget_pop_color(wid);
	widget_pop_color(wid);
}

/* Render a circle using a modified Bresenham line algorithm. */
static void
circle_bresenham(void *p, int xoffs, int yoffs, int radius, int ncolor)
{
	struct widget *wid = p;
	Uint32 color = WIDGET_COLOR(wid, ncolor);
	int cx = wid->cx + xoffs;
	int cy = wid->cy + yoffs;
	int v = 2*radius - 1;
	int e = 0, u = 1;
	int x = 0, y = radius;
	Uint8 *pixel1, *pixel2, *pixel3, *pixel4;

	SDL_LockSurface(view->v);
	while (x < y) {
		pixel1 = (Uint8 *)view->v->pixels + (cy+y)*view->v->pitch +
		    (cx+x)*vfmt->BytesPerPixel;
		pixel2 = (Uint8 *)view->v->pixels + (cy-y)*view->v->pitch +
		    (cx+x)*vfmt->BytesPerPixel;
		pixel3 = (Uint8 *)view->v->pixels + (cy+y)*view->v->pitch +
		    (cx-x)*vfmt->BytesPerPixel;
		pixel4 = (Uint8 *)view->v->pixels + (cy-y)*view->v->pitch +
		    (cx-x)*vfmt->BytesPerPixel;

		clip_pixel(view->v, &pixel1);
		clip_pixel(view->v, &pixel2);
		clip_pixel(view->v, &pixel3);
		clip_pixel(view->v, &pixel4);

		switch (vfmt->BytesPerPixel) {
		case 4:
			*(Uint32 *)pixel1 = color;
			*(Uint32 *)pixel2 = color;
			*(Uint32 *)pixel3 = color;
			*(Uint32 *)pixel4 = color;
			break;
		case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			pixel1[0] = pixel2[0] = (color>>16)&0xff;
			pixel3[0] = pixel4[0] = pixel1[0];
			pixel2[1] = pixel1[1] = (color>>8)&0xff;
			pixel3[1] = pixel4[1] = pixel1[1];
			pixel1[2] = pixel2[2] = color&0xff;
			pixel3[2] = pixel4[2] = pixel1[2];
#else
			pixel1[0] = pixel2[0] = color&0xff;
			pixel3[0] = pixel4[0] = pixel1[0];
			pixel2[1] = pixel1[1] = (color>>8)&0xff;
			pixel3[1] = pixel4[1] = pixel1[1];
			pixel1[2] = pixel2[2] = (color>>16)&0xff;
			pixel3[2] = pixel4[2] = pixel1[2];
#endif /* SDL_BYTEORDER */
			break;
		case 2:
			*(Uint16 *)pixel1 = color;
			*(Uint16 *)pixel2 = color;
			*(Uint16 *)pixel3 = color;
			*(Uint16 *)pixel4 = color;
			break;
		case 1:
			*pixel1 = color;
			*pixel2 = color;
			*pixel3 = color;
			*pixel4 = color;
			break;
		}

		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;

		pixel1 = (Uint8 *)view->v->pixels + (cy+x)*view->v->pitch +
		    (cx+y)*vfmt->BytesPerPixel;
		pixel2 = (Uint8 *)view->v->pixels + (cy-x)*view->v->pitch +
		    (cx+y)*vfmt->BytesPerPixel;
		pixel3 = (Uint8 *)view->v->pixels + (cy+x)*view->v->pitch +
		    (cx-y)*vfmt->BytesPerPixel;
		pixel4 = (Uint8 *)view->v->pixels + (cy-x)*view->v->pitch +
		    (cx-y)*vfmt->BytesPerPixel;
		clip_pixel(view->v, &pixel1);
		clip_pixel(view->v, &pixel2);
		clip_pixel(view->v, &pixel3);
		clip_pixel(view->v, &pixel4);

		switch (vfmt->BytesPerPixel) {
		case 4:
			*(Uint32 *)pixel1 = color;
			*(Uint32 *)pixel2 = color;
			*(Uint32 *)pixel3 = color;
			*(Uint32 *)pixel4 = color;
			break;
		case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			pixel1[0] = pixel2[0] = (color>>16)&0xff;
			pixel3[0] = pixel4[0] = pixel1[0];
			pixel2[1] = pixel1[1] = (color>>8)&0xff;
			pixel3[1] = pixel4[1] = pixel1[1];
			pixel1[2] = pixel2[2] = color&0xff;
			pixel3[2] = pixel4[2] = pixel1[2];
#else
			pixel1[0] = pixel2[0] = color&0xff;
			pixel3[0] = pixel4[0] = pixel1[0];
			pixel2[1] = pixel1[1] = (color>>8)&0xff;
			pixel3[1] = pixel4[1] = pixel1[1];
			pixel1[2] = pixel2[2] = (color>>16)&0xff;
			pixel3[2] = pixel4[2] = pixel1[2];
#endif /* SDL_BYTEORDER */
			break;
		case 2:
			*(Uint16 *)pixel1 = color;
			*(Uint16 *)pixel2 = color;
			*(Uint16 *)pixel3 = color;
			*(Uint16 *)pixel4 = color;
			break;
		case 1:
			*pixel1 = color;
			*pixel2 = color;
			*pixel3 = color;
			*pixel4 = color;
			break;
		}
	}
	pixel1 = (Uint8 *)view->v->pixels + cy*view->v->pitch +
	    (cx-radius)*vfmt->BytesPerPixel;
	pixel2 = (Uint8 *)view->v->pixels + cy*view->v->pitch +
	    (cx+radius)*vfmt->BytesPerPixel;
	clip_pixel(view->v, &pixel1);
	clip_pixel(view->v, &pixel2);
	
	switch (vfmt->BytesPerPixel) {
	case 4:
		*(Uint32 *)pixel1 = color;
		*(Uint32 *)pixel2 = color;
		break;
	case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		pixel2[0] = pixel1[0] = color&0xff;
		pixel2[1] = pixel1[1] = (color>>8)&0xff;
		pixel2[2] = pixel1[2] = (color>>16)&0xff;
#else
		pixel2[0] = pixel1[0] = color&0xff;
		pixel2[1] = pixel1[1] = (color>>8)&0xff;
		pixel2[2] = pixel1[2] = (color>>16)&0xff;
#endif
		break;
	case 2:
		*(Uint16 *)pixel1 = color;
		*(Uint16 *)pixel2 = color;
		break;
	case 1:
		*pixel1 = color;
		*pixel2 = color;
		break;
	}
	SDL_UnlockSurface(view->v);
}

/* Render two lines with +50,50,50 RGB difference. */
static void
line2(void *wid, int x1, int y1, int x2, int y2, int ncolor)
{
	Uint32 color = WIDGET_COLOR(wid, ncolor);
	int ncolor2;

	primitives.line(wid, x1, y1, x2, y2, ncolor);
	
	ncolor2 = widget_push_color(wid, alter_color(color, 50, 50, 50));
	primitives.line(wid, x1+1, y1+1, x2+1, y2+1, ncolor2);
	widget_pop_color(wid);
}

/*
 * Render a straight line between x1,y1 to x2,y2 using a derivate of the
 * line algorithm developed by Jack E. Bresenham.
 */
static void
line_bresenham(void *widget, int px1, int py1, int px2, int py2, int ncolor)
{
	struct widget *wid = widget;
	int dx, dy, dpr, dpru, p;
	int yinc = view->v->pitch;
	int xyinc = vfmt->BytesPerPixel + yinc;
	Uint8 *fb1, *fb2;
	Uint32 color = WIDGET_COLOR(wid, ncolor);
	int x1 = wid->cx + px1, y1 = wid->cy + py1;	/* Left endpoint */
	int x2 = wid->cx + px2, y2 = wid->cy + py2;	/* Right endpoint */

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
	put_pixel2(fb1, fb2, color);

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
	put_pixel1(fb1, color);
	if ((dx & 1) == 0) {
		goto done;
	}
	put_pixel1(fb2, color);
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
	put_pixel1(fb1, color);
	if ((dx & 1) == 0) {
		goto done;
	}
	put_pixel1(fb2, color);
	goto done;

indep_y:
	dpr = dx+dx;
	p = -dy;
	dpru = p+p;
	dx = dy>>1;

yloop:
	put_pixel2(fb1, fb2, color);

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
	put_pixel1(fb2, color);
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
	put_pixel1(fb1, color);
	if ((dy & 1) == 0) {
		goto done;
	}
	put_pixel1(fb2, color);
done:
	SDL_UnlockSurface(view->v);
}

/* Render an outlined rectangle. */
static void
rect_outlined(void *p, int x, int y, int w, int h, int ncolor)
{
	struct widget *wid = p;

	primitives.line(wid, 		/* Top */
	    x,
	    y,
	    x + w - 1,
	    y,
	    ncolor);
	primitives.line(wid, 		/* Bottom */
	    x,
	    y + h - 1,
	    x + w - 1,
	    y + h - 1,
	    ncolor);
	primitives.line(wid, 		/* Left */
	    x,
	    y,
	    x,
	    y + h - 1,
	    ncolor);
	primitives.line(wid, 		/* Right */
	    x + w - 1,
	    y,
	    x + w - 1,
	    y + h - 1,
	    ncolor);
}

/* Render a filled rectangle. */
static void
rect_filled(void *p, int x, int y, int w, int h, int ncolor)
{
	struct widget *wid = p;
	Uint32 color = WIDGET_COLOR(wid, ncolor);
	SDL_Rect rd;

	rd.x = wid->cx + x;
	rd.y = wid->cy + y;
	rd.w = w;
	rd.h = h;
	SDL_FillRect(view->v, &rd, color);
}

/* Render a [+] sign scaled to a w,h box (for trees). */
static void
plus(void *p, int x, int y, int w, int h, int ncolor)
{
	int xcenter = x + w/2;
	int ycenter = y + h/2;

	primitives.line2(p,
	    xcenter,
	    y,
	    xcenter,
	    y + h,
	    ncolor);
	primitives.line2(p,
	    x,
	    ycenter,
	    x + w,
	    ycenter,
	    ncolor);
}

/* Render a [-] sign scaled to a w,h box (for trees). */
static void
minus(void *p, int x, int y, int w, int h, int ncolor)
{
	struct widget *wid = p;
	int ycenter = y+h/2;

	primitives.line2(wid,
	    x,
	    ycenter,
	    x + w,
	    ycenter,
	    ncolor);
}

#ifdef HAVE_OPENGL

/* Render a line using OpenGL. */
static void
line_opengl(void *p, int px1, int py1, int px2, int py2, int ncolor)
{
	struct widget *wid = p;
	Uint32 color = WIDGET_COLOR(wid, ncolor);
	int x1 = wid->cx + px1;
	int y1 = wid->cy + py1;
	int x2 = wid->cx + px2;
	int y2 = wid->cy + py2;
	Uint8 r, g, b;
	
	SDL_GetRGB(color, vfmt, &r, &g, &b);
	glBegin(GL_LINES);
	glColor3ub(r, g, b);
	glVertex2s(x1, y1);
	glVertex2s(x2, y2);
	glEnd();
}

/* Render a filled rectangle using OpenGL. */
static void
rect_opengl(void *p, int x, int y, int w, int h, int ncolor)
{
	struct widget *wid = p;
	Uint32 color = WIDGET_COLOR(wid, ncolor);
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

