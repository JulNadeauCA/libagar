/*	$Csoft: primitive.c,v 1.20 2002/11/14 00:44:11 vedge Exp $	    */

/*
 * Copyright (c) 2002 CubeSoft Communications <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include <stdlib.h>

#include <engine/engine.h>

#include "widget.h"
#include "window.h"
#include "label.h"
#include "tlist.h"
#include "primitive.h"

static void	apply(int, union evarg *);

static __inline__ void	put_pixel1(Uint8, Uint8 *, Uint32);
static __inline__ void	put_pixel2(Uint8, Uint8 *, Uint8 *, Uint32);

/* Types of primitives */
enum {
	BOX,
	FRAME,
	CIRCLE,
	LINE,
	SQUARE,
	TRIANGLE
};

static __inline__ Uint32
alter_color(Uint32 col, Sint8 r, Sint8 g, Sint8 b)
{
	Uint8 nr, ng, nb;

	SDL_GetRGB(col, view->v->format, &nr, &ng, &nb);

	if (nr+r > 255) {
		nr = 255;
	} else if (nr+r < 0) {
		nr = 0;
	} else {
		nr += r;
	}
	
	if (ng+g > 255) {
		ng = 255;
	} else if (ng+g < 0) {
		ng = 0;
	} else {
		ng += g;
	}
	
	if (nb+b > 255) {
		nb = 255;
	} else if (nb+b < 0) {
		nb = 0;
	} else {
		nb += b;
	}

	return (SDL_MapRGB(view->v->format, nr, ng, nb));
}

static void
box_2d(void *p, int xoffs, int yoffs, int w, int h, int z,
    Uint32 color)
{
	struct widget *wid = p;
	Uint32 bgcolor;

	bgcolor = (z < 0) ?
	    alter_color(color, -20, -20, -20) :
	    color;
	if (WIDGET_FOCUSED(wid)) {
		bgcolor = alter_color(bgcolor, 6, 6, 15);
	}

	/* Background */
	WIDGET_FILL(wid, xoffs, yoffs, w, h, bgcolor);

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

static void
box_3d(void *p, int xoffs, int yoffs, int w, int h, int z,
    Uint32 color)
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

	if (WIDGET_FOCUSED(wid)) {
		bcol = alter_color(bcol, 6, 6, 15);
	}

	/* Background */
	WIDGET_FILL(wid, xoffs, yoffs, w, h, bcol);

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

static void
box_3d_dark(void *p, int xoffs, int yoffs, int w, int h, int z,
    Uint32 color, int ra, int ga, int ba)
{
	struct widget *wid = p;
	Uint32 lcol, rcol, bcol;
	
	color = alter_color(color, ra, ga, ba);

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
	WIDGET_FILL(wid, xoffs, yoffs, w, h, bcol);

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

static void
box_3d_dark1(void *p, int xoffs, int yoffs, int w, int h, int z, Uint32 color)
{
	box_3d_dark(p, xoffs, yoffs, w, h, z, color, -20, -20, -20);
}

static void
box_3d_dark2(void *p, int xoffs, int yoffs, int w, int h, int z, Uint32 color)
{
	box_3d_dark(p, xoffs, yoffs, w, h, z, color, -40, -40, -40);
}

static void
box_3d_dark3(void *p, int xoffs, int yoffs, int w, int h, int z, Uint32 color)
{
	box_3d_dark(p, xoffs, yoffs, w, h, z, color, -60, -60, -60);
}

static void
box_3d_dark4(void *p, int xoffs, int yoffs, int w, int h, int z, Uint32 color)
{
	box_3d_dark(p, xoffs, yoffs, w, h, z, color, -100, -100, -100);
}

static void
box_3d_dark5(void *p, int xoffs, int yoffs, int w, int h, int z, Uint32 color)
{
	box_3d_dark(p, xoffs, yoffs, w, h, z, color, -120, -120, -120);
}

static void
frame_3d(void *p, int xoffs, int yoffs, int w, int h, Uint32 color)
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

/* Surface must be locked. */
static __inline__ void
put_pixel1(Uint8 bpp, Uint8 *dst, Uint32 color)
{
	switch (bpp) {
#ifdef VIEW_8BPP
	case 1:
		*dst = color;
		break;
#endif

#ifdef VIEW_16BPP
	case 2:
		*(Uint16 *)dst = color;
		break;
#endif

#ifdef VIEW_24BPP
	case 3:
# if SDL_BYTEORDER == SDL_BIG_ENDIAN
		dst[0] = (color >> 16)	& 0xff;
		dst[1] = (color >> 8)	& 0xff;
		dst[2] = color		& 0xff;
# else
		dst[0] = color		& 0xff;
		dst[1] = (color >> 8)	& 0xff;
		dst[2] = (color >> 16)	& 0xff;
# endif
		break;
#endif

#ifdef VIEW_32BPP
	case 4:
		*(Uint32 *)dst = color;
		break;
#endif
	}
}

/* Surface must be locked. */
static __inline__ void
put_pixel2(Uint8 bpp, Uint8 *dst1, Uint8 *dst2, Uint32 color)
{
	switch (bpp) {
#ifdef VIEW_8BPP
	case 1:
		*dst1 = color;
		*dst2 = color;
		break;
#endif

#ifdef VIEW_16BPP
	case 2:
		*(Uint16 *)dst1 = color;
		*(Uint16 *)dst2 = color;
		break;
#endif

#ifdef VIEW_24BPP
	case 3:
# if SDL_BYTEORDER == SDL_BIG_ENDIAN
		dst1[0] = (color >> 16)	& 0xff;
		dst1[1] = (color >> 8)	& 0xff;
		dst1[2] = color		& 0xff;
# else
		dst1[0] = color		& 0xff;
		dst1[1] = (color >> 8)	& 0xff;
		dst1[2] = (color >> 16)	& 0xff;
# endif
		break;
#endif

#ifdef VIEW_32BPP
	case 4:
		*(Uint32 *)dst1 = color;
		*(Uint32 *)dst2 = color;
		break;
#endif
	}
}

static void
line_bresenham(void *wid, int x1, int y1, int x2, int y2, Uint32 color)
{
	int dx, dy, xinc, yinc, xyinc, dpr, dpru, p;
	Uint8 *fb1, *fb2;
	
	x1 += WIDGET(wid)->win->rd.x + WIDGET(wid)->x;
	y1 += WIDGET(wid)->win->rd.y + WIDGET(wid)->y;
	x2 += WIDGET(wid)->win->rd.x + WIDGET(wid)->x;
	y2 += WIDGET(wid)->win->rd.y + WIDGET(wid)->y;

	fb1 = (Uint8 *)view->v->pixels +
	    y1*view->v->pitch +
	    x1*view->v->format->BytesPerPixel;

	fb2 = (Uint8 *)view->v->pixels +
	    y2*view->v->pitch +
	    x2*view->v->format->BytesPerPixel;
	
	xinc = view->v->format->BytesPerPixel;
	dx = x2 - x1;
	if (dx < 0) {
		dx = -dx;
		xinc = -view->v->format->BytesPerPixel;
	}

	yinc = view->v->pitch;
	dy = y2 - y1;
	if (dy < 0) {
		yinc = -view->v->pitch;
		dy = -dy;
	}

	xyinc = xinc+yinc;

	SDL_LockSurface(view->v);

	if (dy > dx) {
		goto y_is_independent;
	}

/* x_is_independent: */
	dpr = dy+dy;
	p = -dx;
	dpru = p+p;
	dy = dx>>1;

xloop:
	put_pixel2(xinc, fb1, fb2, color);

	if ((p += dpr) > 0) {
		goto right_and_up;
	}

/* up: */
	fb1 += xinc;
	fb2 -= xinc;
	if ((dy=dy-1) >= 0) {
		goto xloop;
	}
	put_pixel1(xinc, fb1, color);
	if ((dx & 1) == 0) {
		goto done;
	}
	put_pixel1(xinc, fb2, color);
	goto done;

right_and_up:
	fb1 += xyinc;
	fb2 -= xyinc;
	p += dpru;
	if (--dy >= 0) {
		goto xloop;
	}
	put_pixel1(xinc, fb1, color);
	if ((dx & 1) == 0) {
		goto done;
	}
	put_pixel1(xinc, fb2, color);
	goto done;

y_is_independent:
	dpr = dx+dx;
	p = -dy;
	dpru = p+p;
	dx = dy>>1;

yloop:
	put_pixel2(xinc, fb1, fb2, color);

	if ((p += dpr) > 0) {
		goto right_and_up_2;
	}
/* up: */
	fb1 += yinc;
	fb2 -= yinc;
	if ((dx=dx-1) >= 0) {
		goto yloop;
	}
	put_pixel1(xinc, fb2, color);
	goto done;

right_and_up_2:
	fb1 += xyinc;
	fb2 -= xyinc;
	p += dpru;
	if ((dx=dx-1) >= 0) {
		goto yloop;
	}
	put_pixel1(xinc, fb1, color);
	if ((dy & 1) == 0) {
		goto done;
	}
	put_pixel1(xinc, fb2, color);
done:
	SDL_UnlockSurface(view->v);
}

static void
line_bresenham_dashed(void *wid, int x1, int y1, int x2, int y2, Uint32 color,
    int dash)
{
	int dx, dy, xinc, yinc, xyinc, dpr, dpru, p, alt = 0;
	Uint8 *fb1, *fb2;

	x1 += WIDGET(wid)->win->rd.x + WIDGET(wid)->x;
	y1 += WIDGET(wid)->win->rd.y + WIDGET(wid)->y;
	x2 += WIDGET(wid)->win->rd.x + WIDGET(wid)->x;
	y2 += WIDGET(wid)->win->rd.y + WIDGET(wid)->y;

	fb1 = (Uint8 *)view->v->pixels +
	    y1*view->v->pitch +
	    x1*view->v->format->BytesPerPixel;

	fb2 = (Uint8 *)view->v->pixels +
	    y2*view->v->pitch +
	    x2*view->v->format->BytesPerPixel;
	
	xinc = view->v->format->BytesPerPixel;
	dx = x2 - x1;
	if (dx < 0) {
		dx = -dx;
		xinc = -view->v->format->BytesPerPixel;
	}

	yinc = view->v->pitch;
	dy = y2 - y1;
	if (dy < 0) {
		yinc = -view->v->pitch;
		dy = -dy;
	}

	xyinc = xinc+yinc;

	SDL_LockSurface(view->v);

	if (dy > dx) {
		goto y_is_independent;
	}

/* x_is_independent: */
	dpr = dy+dy;
	p = -dx;
	dpru = p+p;
	dy = dx>>1;

xloop:
	if (++alt > dash) {
		alt = 0;
		put_pixel2(xinc, fb1, fb2, color);
	}

	if ((p += dpr) > 0) {
		goto right_and_up;
	}

/* up: */
	fb1 += xinc;
	fb2 -= xinc;
	if ((dy=dy-1) >= 0) {
		goto xloop;
	}
	if (++alt > dash) {
		alt = 0;
		put_pixel1(xinc, fb1, color);
	}
	if ((dx & 1) == 0) {
		goto done;
	}
	if (++alt > dash) {
		alt = 0;
		put_pixel1(xinc, fb2, color);
	}
	goto done;

right_and_up:
	fb1 += xyinc;
	fb2 -= xyinc;
	p += dpru;
	if (--dy >= 0) {
		goto xloop;
	}
	if (++alt > dash) {
		alt = 0;
		put_pixel1(xinc, fb1, color);
	}
	if ((dx & 1) == 0) {
		goto done;
	}
	if (++alt > dash) {
		alt = 0;
		put_pixel1(xinc, fb2, color);
	}
	goto done;

y_is_independent:
	dpr = dx+dx;
	p = -dy;
	dpru = p+p;
	dx = dy>>1;

yloop:
	if (++alt > dash) {
		alt = 0;
		put_pixel2(xinc, fb1, fb2, color);
	}

	if ((p += dpr) > 0) {
		goto right_and_up_2;
	}
/* up: */
	fb1 += yinc;
	fb2 -= yinc;
	if ((dx=dx-1) >= 0) {
		goto yloop;
	}
	if (++alt > dash) {
		alt = 0;
		put_pixel1(xinc, fb2, color);
	}
	goto done;

right_and_up_2:
	fb1 += xyinc;
	fb2 -= xyinc;
	p += dpru;
	if ((dx=dx-1) >= 0) {
		goto yloop;
	}
	if (++alt > dash) {
		alt = 0;
		put_pixel1(xinc, fb1, color);
	}
	if ((dy & 1) == 0) {
		goto done;
	}
	if (++alt > dash) {
		alt = 0;
		put_pixel1(xinc, fb2, color);
	}
done:
	SDL_UnlockSurface(view->v);
}

static void
line_bresenham_dashed1(void *wid, int x1, int y1, int x2, int y2, Uint32 color)
{
	line_bresenham_dashed(wid, x1, y1, x2, y2, color, 1);
}

static void
line_bresenham_dashed2(void *wid, int x1, int y1, int x2, int y2, Uint32 color)
{
	line_bresenham_dashed(wid, x1, y1, x2, y2, color, 2);
}

static void
line_bresenham_dashed3(void *wid, int x1, int y1, int x2, int y2, Uint32 color)
{
	line_bresenham_dashed(wid, x1, y1, x2, y2, color, 3);
}

static void
line_bresenham_dashed4(void *wid, int x1, int y1, int x2, int y2, Uint32 color)
{
	line_bresenham_dashed(wid, x1, y1, x2, y2, color, 4);
}

static void
line_bresenham_dashed5(void *wid, int x1, int y1, int x2, int y2, Uint32 color)
{
	line_bresenham_dashed(wid, x1, y1, x2, y2, color, 5);
}

static void
square_composite(void *p, int x, int y, int w, int h, Uint32 color)
{
	struct widget *wid = p;

	if (x > wid->w || y > wid->h)
		return;
	if (x+w > wid->w)
		w = wid->w-x;
	if (y+h > wid->h)
		h = wid->h-y;

	primitives.line(wid,		/* Top */
	    x, y,
	    x + w - 1, y, color);
	primitives.line(wid,		/* Bottom */
	    x, y + h - 1,
	    x + w - 1, y + h - 1, color);
	primitives.line(wid,		/* Left */
	    x, y,
	    x, y + h - 1, color);
	primitives.line(wid,		/* Right */
	    x+w - 1, y,
	    x+w - 1, y + h - 1, color);
}

static void
triangle_composite(void *p, int p1[2], int p2[2], int p3[2], Uint32 color)
{
	struct widget *wid = p;

	primitives.line(wid,
	    p1[0], p1[1],
	    p2[0], p2[1], color);
	primitives.line(wid,
	    p2[0], p2[1],
	    p3[0], p3[1], color);
	primitives.line(wid,
	    p3[0], p3[1],
	    p1[0], p1[1], color);
}

/* Default primitives ops */
struct primitive_ops primitives = {
	box_3d,			/* box */
	frame_3d,		/* frame */
	circle_bresenham,	/* circle */
	line_bresenham,		/* line */
	square_composite,	/* square */
	triangle_composite	/* triangle */
};

struct window *
primitive_config_window(void)
{
	struct window *win;
	struct region *reg;
	struct label *lab;
	struct tlist *tl;
	const int ydiv = 25;

	/* Primitive drawing algorithm switch */
	win = window_new("config-primitive-algorithm-sw",
	    "Primitive algorithm switch", WINDOW_CENTER,
	    0, 0,
	    503, 440,
	    424, 267);

	reg = region_new(win, REGION_VALIGN, 0, 0, 50, 93);
	
	lab = label_new(reg, "Box:", 100, 5);
	tl = tlist_new(reg, 100, 28, 0);
	tlist_insert_item(tl, NULL,
	    "2d-style", box_2d);
	tlist_insert_item_selected(tl, NULL,
	    "3d-style", box_3d);
	tlist_insert_item(tl, NULL,
	    "Dark 3d-style #1", box_3d_dark1);
	tlist_insert_item(tl, NULL,
	    "Dark 3d-style #2", box_3d_dark2);
	tlist_insert_item(tl, NULL,
	    "Dark 3d-style #3", box_3d_dark3);
	tlist_insert_item(tl, NULL,
	    "Dark 3d-style #4", box_3d_dark4);
	tlist_insert_item(tl, NULL,
	    "Dark 3d-style #5", box_3d_dark5);
	event_new(tl, "tlist-changed", 0, apply, "%i", BOX);

	lab = label_new(reg, "Frame:", 100, 5);
	tl = tlist_new(reg, 100, 28, 0);
	tlist_insert_item_selected(tl, NULL,
	    "3d-style", frame_3d);
	event_new(tl, "tlist-changed", 0, apply, "%i", FRAME);

	lab = label_new(reg, "Circle:", 100, 5);
	tl = tlist_new(reg, 100, 28, 0);
	tlist_insert_item_selected(tl, NULL,
	    "Bresenham #1", circle_bresenham);
	event_new(tl, "tlist-changed", 0, apply, "%i", CIRCLE);
	
	reg = region_new(win, REGION_VALIGN, 50, 0, 50, 93);
		
	lab = label_new(reg, "Line:", 100, 5);
	tl = tlist_new(reg, 100, 28, 0);
	tlist_insert_item_selected(tl, NULL,
	    "Bresenham plain #1", line_bresenham);
	tlist_insert_item(tl, NULL,
	    "Bresenham dashed #1", line_bresenham_dashed1);
	tlist_insert_item(tl, NULL,
	    "Bresenham dashed #2", line_bresenham_dashed2);
	tlist_insert_item(tl, NULL,
	    "Bresenham dashed #3", line_bresenham_dashed3);
	tlist_insert_item(tl, NULL,
	    "Bresenham dashed #4", line_bresenham_dashed4);
	tlist_insert_item(tl, NULL,
	    "Bresenham dashed #5", line_bresenham_dashed5);
	event_new(tl, "tlist-changed", 0, apply, "%i", LINE);
		
	lab = label_new(reg, "Square:", 100, 5);
	tl = tlist_new(reg, 100, 28, 0);
	tlist_insert_item_selected(tl, NULL,
	    "Composite", square_composite);
	event_new(tl, "tlist-changed", 0, apply, "%i", SQUARE);
		
	lab = label_new(reg, "Triangle:", 100, 5);
	tl = tlist_new(reg, 100, 28, 0);
	tlist_insert_item_selected(tl, NULL,
	    "Composite", triangle_composite);
	event_new(tl, "tlist-changed", 0, apply, "%i", TRIANGLE);

	return (win);
}

static void
apply(int argc, union evarg *argv)
{
	struct tlist *tl = argv[0].p;
	int prim = argv[1].i;
	struct tlist_item *sel = argv[2].p;

	switch (prim) {
	case BOX:
		primitives.box = sel->p1;
		break;
	case FRAME:
		primitives.frame = sel->p1;
		break;
	case CIRCLE:
		primitives.circle = sel->p1;
		break;
	case LINE:
		primitives.line = sel->p1;
		break;
	case SQUARE:
		primitives.square = sel->p1;
		break;
	case TRIANGLE:
		primitives.triangle = sel->p1;
		break;
	}
}

void
primitive_sequence(struct window *win, enum primitive_seq seq)
{
	void (*old_line)(void *, int, int, int, int, Uint32);
	int i;
#define REDRAW() {						\
	window_draw(win);					\
	SDL_UpdateRects(view->v, view->ndirty, view->dirty);	\
	view->ndirty = 0;					\
}

	switch (seq) {
	case PRIMITIVE_SEQ_MATERIALIZE:
		old_line = primitives.line;
		primitives.line = line_bresenham_dashed5;
		REDRAW();
		SDL_Delay(50);
		primitives.line = line_bresenham_dashed4;
		REDRAW();
		SDL_Delay(40);
		primitives.line = line_bresenham_dashed3;
		REDRAW();
		SDL_Delay(30);
		primitives.line = line_bresenham_dashed2;
		REDRAW();
		SDL_Delay(20);
		primitives.line = line_bresenham_dashed1;
		REDRAW();
		SDL_Delay(15);
		primitives.line = old_line;
		break;
	case PRIMITIVE_SEQ_DEMATERIALIZE:
		old_line = primitives.line;
		primitives.line = line_bresenham_dashed1;
		REDRAW();
		SDL_Delay(15);
		primitives.line = line_bresenham_dashed2;
		REDRAW();
		SDL_Delay(20);
		primitives.line = line_bresenham_dashed3;
		REDRAW();
		SDL_Delay(25);
		primitives.line = line_bresenham_dashed4;
		REDRAW();
		SDL_Delay(30);
		primitives.line = line_bresenham_dashed5;
		REDRAW();
		SDL_Delay(35);
		primitives.line = old_line;
		break;
	}
#undef REDRAW()
}
