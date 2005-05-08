/*	$Csoft: primitive.c,v 1.68 2005/03/09 06:39:20 vedge Exp $	    */

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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

/* Draw a 3D-style box. */
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

	if (widget_holds_focus(wid)) {
		bcol = (z < 0) ?
		    alter_color(color, -10, -10, -10) :
		    alter_color(color, 20, 20, 20);
	} else {
		bcol = (z < 0) ?
		    alter_color(color, -20, -20, -20) :
		    alter_color(color, 10, 10, 10);
	}

	primitives.rect_filled(wid, xoffs+1, yoffs, w-2, h-1, bcol);
	primitives.line(wid, xoffs, yoffs, xoffs+w-1, yoffs, lcol);
	primitives.line(wid, xoffs, yoffs, xoffs, yoffs+h-1, lcol);
	primitives.line(wid, xoffs, yoffs+h-1, xoffs+w-1, yoffs+h-1, rcol);
	primitives.line(wid, xoffs+w-1, yoffs, xoffs+w-1, yoffs+h-1, rcol);
}

/* Draw a 3D-style box with chamfered top edges. */
static void
box_chamfered(void *p, SDL_Rect *r, int z, int rad, Uint32 bcol)
{
	struct widget *wid = p;
	Uint32 lcol, rcol;
	int v, e, u;
	int x, y;

	lcol = (z < 0) ?
	    alter_color(bcol, -60, -60, -60) :
	    alter_color(bcol, 60, 60, 60);
	rcol = (z < 0) ?
	    alter_color(bcol, 60, 60, 60) :
	    alter_color(bcol, -60, -60, -60);

	/* Fill the background except the corners. */
	primitives.rect_filled(wid,			/* Body */
	    r->x + rad,
	    r->y + rad,
	    r->w - rad*2,
	    r->h - rad,
	    bcol);
	primitives.rect_filled(wid,			/* Top */
	    r->x + rad,
	    r->y,
	    r->w - rad*2,
	    r->h,
	    bcol);
	primitives.rect_filled(wid,			/* Left */
	    r->x,
	    r->y + rad,
	    rad,
	    r->h - rad,
	    bcol);
	primitives.rect_filled(wid,			/* Right */
	    r->x + r->w - rad,
	    r->y + rad,
	    rad,
	    r->h - rad,
	    bcol);

	/* Draw the three straight lines. */
	primitives.line(wid,				/* Top line */
	    r->x + rad,
	    r->y,
	    r->x + r->w - rad,
	    r->y,
	    bcol);
	primitives.line(wid,				/* Left line */
	    r->x,
	    r->y + rad,
	    r->x,
	    r->y + r->h,
	    lcol);
	primitives.line(wid,				/* Right line */
	    r->x + r->w - 1,
	    r->y + rad,
	    r->x + r->w - 1,
	    r->y + r->h,
	    rcol);

	/* Draw the two chamfered edges using a Bresenham generalization. */
	v = 2*rad - 1;
	e = 0;
	u = 0;
	x = 0;
	y = rad;
	SDL_LockSurface(view->v);
	while (x <= y) {
		int i;

		widget_put_pixel(wid,
		    r->x + rad - x,
		    r->y + rad - y,
		    lcol);
		widget_put_pixel(wid,
		    r->x + rad - y,
		    r->y + rad - x,
		    lcol);

		widget_put_pixel(wid,
		    r->x - rad + (r->w - 1) + x,
		    r->y + rad - y,
		    rcol);
		widget_put_pixel(wid,
		    r->x - rad + (r->w - 1) + y,
		    r->y + rad - x,
		    rcol);
		
		for (i = 0; i < x; i++) {
			widget_put_pixel(wid,
			    r->x + rad - i,
			    r->y + rad - y,
			    bcol);
			widget_put_pixel(wid,
			    r->x - rad + (r->w - 1) + i,
			    r->y + rad - y,
			    bcol);
		}
		for (i = 0; i < y; i++) {
			widget_put_pixel(wid,
			    r->x + rad - i,
			    r->y + rad - x,
			    bcol);
			widget_put_pixel(wid,
			    r->x - rad + (r->w - 1) + i,
			    r->y + rad - x,
			    bcol);
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
	SDL_UnlockSurface(view->v);
}

/* Draw a 3D-style frame. */
static void
frame(void *p, int xoffs, int yoffs, int w, int h, Uint32 color)
{
	struct widget *wid = p;
	Uint32 shade = alter_color(color, -40, -40, -40);

	primitives.line(wid, xoffs, yoffs, xoffs+w-1, yoffs, color);
	primitives.line(wid, xoffs, yoffs, xoffs, yoffs+h-1, color);
	primitives.line(wid, xoffs, yoffs+h-1, xoffs+w-1, yoffs+h-1, shade);
	primitives.line(wid, xoffs+w-1, yoffs, xoffs+w-1, yoffs+h-1, shade);
}

/* Render a circle using a modified Bresenham line algorithm. */
static void
circle_bresenham(void *p, int wx, int wy, int radius, Uint32 color)
{
	struct widget *wid = p;
	int v = 2*radius - 1;
	int e = 0, u = 1;
	int x = 0, y = radius;

	SDL_LockSurface(view->v);
	while (x < y) {
		widget_put_pixel(wid, wx+x, wy+y, color);
		widget_put_pixel(wid, wx+x, wy-y, color);
		widget_put_pixel(wid, wx-x, wy+y, color);
		widget_put_pixel(wid, wx-x, wy-y, color);

		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;
		
		widget_put_pixel(wid, wx+y, wy+x, color);
		widget_put_pixel(wid, wx+y, wy-x, color);
		widget_put_pixel(wid, wx-y, wy+x, color);
		widget_put_pixel(wid, wx-y, wy-x, color);
	}
	widget_put_pixel(wid, wx-radius, wy, color);
	widget_put_pixel(wid, wx+radius, wy, color);
	SDL_UnlockSurface(view->v);
}

static void
circle2_bresenham(void *p, int wx, int wy, int radius, Uint32 color)
{
	struct widget *wid = p;
	int v = 2*radius - 1;
	int e = 0, u = 1;
	int x = 0, y = radius;

	SDL_LockSurface(view->v);
	while (x < y) {
		widget_put_pixel(wid, wx+x, wy+y, color);
		widget_put_pixel(wid, wx+x+1, wy+y, color);
		widget_put_pixel(wid, wx+x, wy-y, color);
		widget_put_pixel(wid, wx+x+1, wy-y, color);
		widget_put_pixel(wid, wx-x, wy+y, color);
		widget_put_pixel(wid, wx-x-1, wy+y, color);
		widget_put_pixel(wid, wx-x, wy-y, color);
		widget_put_pixel(wid, wx-x-1, wy-y, color);


		e += u;
		u += 2;
		if (v < 2*e) {
			y--;
			e -= v;
			v -= 2;
		}
		x++;
		
		widget_put_pixel(wid, wx+y, wy+x, color);
		widget_put_pixel(wid, wx+y+1, wy+x, color);
		widget_put_pixel(wid, wx+y, wy-x, color);
		widget_put_pixel(wid, wx+y+1, wy-x, color);
		widget_put_pixel(wid, wx-y, wy+x, color);
		widget_put_pixel(wid, wx-y-1, wy+x, color);
		widget_put_pixel(wid, wx-y, wy-x, color);
		widget_put_pixel(wid, wx-y-1, wy-x, color);
	}
	widget_put_pixel(wid, wx-radius, wy, color);
	widget_put_pixel(wid, wx+radius, wy, color);
	SDL_UnlockSurface(view->v);
}

/* Render two lines with +50,50,50 RGB difference. */
static void
line2(void *wid, int x1, int y1, int x2, int y2, Uint32 color1)
{
	Uint32 color2 = alter_color(color1, 50, 50, 50);

	primitives.line(wid, x1, y1, x2, y2, color1);
	primitives.line(wid, x1+1, y1+1, x2+1, y2+1, color2);
}

/*
 * Draw a line segment between two points using the Bresenham algorithm
 * presented by Foley & Van Dam [1990].
 */
static void
line_bresenham(void *widget, int x1, int y1, int x2, int y2, Uint32 color)
{
	struct widget *wid = widget;
	int dx, dy;
	int inc1, inc2;
	int d, x, y;
	int xend, yend;
	int xdir, ydir;

	dx = abs(x2-x1);
	dy = abs(y2-y1);

	SDL_LockSurface(view->v);

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
		widget_put_pixel(wid, x, y, color);

		if (((y2-y1)*ydir) > 0) {
			while (x < xend) {
				x++;
				if (d < 0) {
					d += inc1;
				} else {
					y++;
					d += inc2;
				}
				widget_put_pixel(wid, x, y, color);
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
				widget_put_pixel(wid, x, y, color);
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
		widget_put_pixel(wid, x, y, color);

		if (((x2-x1)*xdir) > 0) {
			while (y < yend) {
				y++;
				if (d < 0) {
					d += inc1;
				} else {
					x++;
					d += inc2;
				}
				widget_put_pixel(wid, x, y, color);
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
				widget_put_pixel(wid, x, y, color);
			}
		}
	}
	SDL_UnlockSurface(view->v);

}

/* Render an outlined rectangle. */
static void
rect_outlined(void *p, int x, int y, int w, int h, Uint32 color)
{
	struct widget *wid = p;

	primitives.line(wid, x, y, x+w-1, y, color);
	primitives.line(wid, x, y+h-1, x+w-1, y+h-1, color);
	primitives.line(wid, x, y, x, y+h-1, color);
	primitives.line(wid, x+w-1, y, x+w-1, y+h-1, color);
}

/* Render a filled rectangle. */
static void
rect_filled(void *p, int x, int y, int w, int h, Uint32 color)
{
	struct widget *wid = p;
	SDL_Rect rd;

	rd.x = wid->cx+x;
	rd.y = wid->cy+y;
	rd.w = w;
	rd.h = h;
	SDL_FillRect(view->v, &rd, color);
}

/* Draw a gimp-style background tiling. */
static void
tiling(void *p, SDL_Rect rd, int tsz, int offs, Uint32 c1, Uint32 c2)
{
	struct widget *wid = p;
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
				primitives.rect_filled(wid, x, y, tsz, tsz,
				    c1);
				alt1 = 0;
			} else {
				primitives.rect_filled(wid, x, y, tsz, tsz,
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
plus(void *p, int x, int y, int w, int h, Uint32 color)
{
	int xcen = x + w/2;
	int ycen = y + h/2;

	primitives.line2(p, xcen, y, xcen, y+h, color);
	primitives.line2(p, x, ycen, x+w, ycen, color);
}

/* Draw a [-] sign. */
static void
minus(void *p, int x, int y, int w, int h, Uint32 color)
{
	struct widget *wid = p;
	int ycen = y+h/2;

	primitives.line2(wid, x, ycen, x+w, ycen, color);
}

#ifdef HAVE_OPENGL
static void
line_opengl(void *p, int px1, int py1, int px2, int py2, Uint32 color)
{
	struct widget *wid = p;
	int x1 = wid->cx + px1;
	int y1 = wid->cy + py1;
	int x2 = wid->cx + px2;
	int y2 = wid->cy + py2;
	Uint8 r, g, b;

	if (wid->flags & WIDGET_CLIPPING) {
		if (x1 > wid->cx+wid->w ||
		    y1 > wid->cy+wid->h ||
		    x2 > wid->cx+wid->w ||
		    y2 > wid->cy+wid->h ||
		    x1 < wid->cx ||
		    y1 < wid->cy) {
			return;
		}
	}

	SDL_GetRGB(color, vfmt, &r, &g, &b);
	glBegin(GL_LINES);
	glColor3ub(r, g, b);
	glVertex2s(x1, y1);
	glVertex2s(x2, y2);
	glEnd();
}

static void
circle_opengl(void *p, int x, int y, int radius, Uint32 color)
{
	struct widget *wid = p;
	int nedges = radius*2;
	int i;
	Uint8 r, g, b;
	
	SDL_GetRGB(color, vfmt, &r, &g, &b);

	glBegin(GL_LINE_LOOP);
	glColor3ub(r, g, b);
	for (i = 0; i < nedges; i++) {
		glVertex2f(wid->cx + x + radius*cos((2*M_PI*i)/nedges),
		           wid->cy + y + radius*sin((2*M_PI*i)/nedges));
	}
	glEnd();
}

static void
circle2_opengl(void *p, int x, int y, int radius, Uint32 color)
{
	struct widget *wid = p;
	int nedges = radius*2;
	int i;
	Uint8 r, g, b;
	
	SDL_GetRGB(color, vfmt, &r, &g, &b);

	glBegin(GL_LINE_LOOP);
	glColor3ub(r, g, b);
	for (i = 0; i < nedges; i++) {
		glVertex2f(wid->cx + x + radius*cos((2*M_PI*i)/nedges),
		           wid->cy + y + radius*sin((2*M_PI*i)/nedges));
		glVertex2f(wid->cx + x + (radius+1)*cos((2*M_PI*i)/nedges),
		           wid->cy + y + (radius+1)*sin((2*M_PI*i)/nedges));
	}
	glEnd();
}

static void
rect_opengl(void *p, int x, int y, int w, int h, Uint32 color)
{
	struct widget *wid = p;
	Uint8 r, g, b;
	int x1 = wid->cx+x;
	int y1 = wid->cy+y;
	int x2 = x1+w;
	int y2 = y1+h;

	if (wid->flags & WIDGET_CLIPPING) {
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

	SDL_GetRGB(color, vfmt, &r, &g, &b);

	glBegin(GL_POLYGON);
	glColor3ub(r, g, b);
	glVertex2i(x1, y1);
	glVertex2i(x2, y1);
	glVertex2i(x2, y2);
	glVertex2i(x1, y2);
	glEnd();
}

/* Draw a 3D-style box with chamfered top edges. */
static void
box_chamfered_gl(void *p, SDL_Rect *r, int z, int rad, Uint32 bcol)
{
	struct widget *wid = p;
	Uint32 lcol, rcol;
	int x, y;

	lcol = (z < 0) ?
	    alter_color(bcol, -60, -60, -60) :
	    alter_color(bcol, 60, 60, 60);
	rcol = (z < 0) ?
	    alter_color(bcol, 60, 60, 60) :
	    alter_color(bcol, -60, -60, -60);

	/* Fill the background except the corners. */
	primitives.rect_filled(wid,			/* Body */
	    r->x + rad,
	    r->y + rad,
	    r->w - rad*2,
	    r->h - rad,
	    bcol);
	primitives.rect_filled(wid,			/* Top */
	    r->x + rad,
	    r->y,
	    r->w - rad*2,
	    r->h,
	    bcol);
	primitives.rect_filled(wid,			/* Left */
	    r->x,
	    r->y + rad,
	    rad,
	    r->h - rad,
	    bcol);
	primitives.rect_filled(wid,			/* Right */
	    r->x + r->w - rad,
	    r->y + rad,
	    rad,
	    r->h - rad,
	    bcol);

	/* Draw the three straight lines. */
	primitives.line(wid,				/* Top line */
	    r->x + rad,
	    r->y,
	    r->x + r->w - rad,
	    r->y,
	    bcol);
	primitives.line(wid,				/* Left line */
	    r->x,
	    r->y + rad,
	    r->x,
	    r->y + r->h,
	    lcol);
	primitives.line(wid,				/* Right line */
	    r->x + r->w - 1,
	    r->y + rad,
	    r->x + r->w - 1,
	    r->y + r->h,
	    rcol);

	x = 0;
	y = rad;

	/* Draw the two chamfered edges. */
	glBegin(GL_LINE_LOOP);
	{
		Uint8 r, g, b;

		SDL_GetRGB(lcol, vfmt, &r, &g, &b);
		glColor3ub(r, g, b);
#if 0
		for (i = 0; i < 10; i++) {
			glVertex2f(wid->cx + x + rad*cos((2*M_PI*i)/10),
			           wid->cy + y + rad*sin((2*M_PI*i)/10));
		}
#endif
		glVertex2s(wid->cx, wid->cy+rad);
		glVertex2s(wid->cx+rad, wid->cy);
	}
	glEnd();
}
#endif /* HAVE_OPENGL */

void
primitives_init(void)
{
	primitives.box = box;
	primitives.frame = frame;
	primitives.rect_outlined = rect_outlined;
	primitives.plus = plus;
	primitives.minus = minus;
	primitives.line2 = line2;
	primitives.tiling = tiling;

#ifdef HAVE_OPENGL
	if (view->opengl) {
		primitives.line = line_opengl;
		primitives.rect_filled = rect_opengl;
		primitives.circle = circle_opengl;
		primitives.circle2 = circle2_opengl;
		primitives.box_chamfered = box_chamfered_gl;
	} else
#endif
	{
		primitives.line = line_bresenham;
		primitives.rect_filled = rect_filled;
		primitives.circle = circle_bresenham;
		primitives.circle2 = circle2_bresenham;
		primitives.box_chamfered = box_chamfered;
	}
}

