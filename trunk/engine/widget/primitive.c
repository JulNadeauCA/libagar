/*	$Csoft: primitive.c,v 1.61 2005/01/05 04:44:05 vedge Exp $	    */

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
box(void *p, int xoffs, int yoffs, int w, int h, int z, int ncolor)
{
	struct widget *wid = p;
	Uint32 color = WIDGET_COLOR(wid, ncolor);
	int lcol, rcol, bcol;

#ifdef DEBUG
	if (ncolor >= WIDGET_COLORS_MAX)
		fatal("color stack oflow");
#endif
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

	primitives.rect_filled(wid, xoffs+1, yoffs, w-2, h-1, bcol);
	primitives.line(wid, xoffs, yoffs, xoffs+w-1, yoffs, lcol);
	primitives.line(wid, xoffs, yoffs, xoffs, yoffs+h-1, lcol);
	primitives.line(wid, xoffs, yoffs+h-1, xoffs+w-1, yoffs+h-1, rcol);
	primitives.line(wid, xoffs+w-1, yoffs, xoffs+w-1, yoffs+h-1, rcol);

	widget_pop_color(wid);
	widget_pop_color(wid);
	widget_pop_color(wid);
}

/* Draw a 3D-style frame. */
static void
frame(void *p, int xoffs, int yoffs, int w, int h, int ncolor)
{
	struct widget *wid = p;
	Uint32 color = WIDGET_COLOR(wid, ncolor);
	int lcol, rcol;

	lcol = widget_push_color(wid, alter_color(color, 80, 80, 80));
	rcol = widget_push_color(wid, alter_color(color, -80, -80, -80));

	primitives.line(wid, xoffs, yoffs, xoffs+w-1, yoffs, lcol);
	primitives.line(wid, xoffs, yoffs, xoffs, yoffs+h-1, lcol);
	primitives.line(wid, xoffs, yoffs+h-1, xoffs+w-1, yoffs+h-1, rcol);
	primitives.line(wid, xoffs+w-1, yoffs, xoffs+w-1, yoffs+h-1, rcol);

	widget_pop_color(wid);
	widget_pop_color(wid);
}

/* Render a circle using a modified Bresenham line algorithm. */
static void
circle_bresenham(void *p, int wx, int wy, int radius, int ncolor)
{
	struct widget *wid = p;
	Uint32 color = WIDGET_COLOR(wid, ncolor);
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
 * Draw a line segment between two points using the Bresenham algorithm
 * presented by Foley & Van Dam [1990].
 */
static void
line_bresenham(void *widget, int x1, int y1, int x2, int y2, int ncolor)
{
	struct widget *wid = widget;
	Uint32 color = WIDGET_COLOR(wid, ncolor);
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
rect_outlined(void *p, int x, int y, int w, int h, int ncolor)
{
	struct widget *wid = p;

	primitives.line(wid, x, y, x+w-1, y, ncolor);
	primitives.line(wid, x, y+h-1, x+w-1, y+h-1, ncolor);
	primitives.line(wid, x, y, x, y+h-1, ncolor);
	primitives.line(wid, x+w-1, y, x+w-1, y+h-1, ncolor);
}

/* Render a filled rectangle. */
static void
rect_filled(void *p, int x, int y, int w, int h, int ncolor)
{
	struct widget *wid = p;
	Uint32 color = WIDGET_COLOR(wid, ncolor);
	SDL_Rect rd;

	rd.x = wid->cx+x;
	rd.y = wid->cy+y;
	rd.w = w;
	rd.h = h;
	SDL_FillRect(view->v, &rd, color);
}

/* Draw a gimp-style background tiling. */
static void
tiling(void *p, SDL_Rect rd, int tsz, int offs, int c1, int c2)
{
	struct widget *wid = p;
	int alt1 = 0, alt2 = 0;
	int x, y;

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
plus(void *p, int x, int y, int w, int h, int ncolor)
{
	int xcen = x + w/2;
	int ycen = y + h/2;

	primitives.line2(p, xcen, y, xcen, y+h, ncolor);
	primitives.line2(p, x, ycen, x+w, ycen, ncolor);
}

/* Draw a [-] sign. */
static void
minus(void *p, int x, int y, int w, int h, int ncolor)
{
	struct widget *wid = p;
	int ycen = y+h/2;

	primitives.line2(wid, x, ycen, x+w, ycen, ncolor);
}

#ifdef HAVE_OPENGL
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
circle_opengl(void *p, int x, int y, int radius, int ncolor)
{
	struct widget *wid = p;
	int nedges = radius*2;
	int i;
	Uint8 r, g, b;
	
	SDL_GetRGB(WIDGET_COLOR(p, ncolor), vfmt, &r, &g, &b);

	glBegin(GL_LINE_LOOP);
	glColor3ub(r, g, b);
	for (i = 0; i < nedges; i++) {
		glVertex2f(wid->cx + x + radius*cos((2*M_PI*i)/nedges),
		           wid->cy + y + radius*sin((2*M_PI*i)/nedges));
	}
	glEnd();
}

static void
rect_opengl(void *p, int x, int y, int w, int h, int ncolor)
{
	struct widget *wid = p;
	Uint32 color = WIDGET_COLOR(wid, ncolor);
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
	} else
#endif
	{
		primitives.line = line_bresenham;
		primitives.rect_filled = rect_filled;
		primitives.circle = circle_bresenham;
	}
}

