/*	$Csoft: primitive.c,v 1.10 2002/07/24 04:54:28 vedge Exp $	    */

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
#include "primitive.h"

static void	box_3d(void *, int, int, int, int, int, Uint32);
static void	frame_3d(void *, int, int, int, int, Uint32);
static void	bresenham_circle(void *, int, int, int, int, int, Uint32);
static void	bresenham_line(void *, int, int, int, int, Uint32);
static void	composite_square(void *, int, int, int, int, Uint32);

static __inline__ void	put_pixel1(Uint8, Uint8 *, Uint32);
static __inline__ void	put_pixel2(Uint8, Uint8 *, Uint8 *, Uint32);

struct primitive_ops primitives = {
	box_3d,			/* box */
	frame_3d,		/* frame */
	bresenham_circle,	/* circle */
	bresenham_line,		/* line */
	composite_square	/* square */
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
box_3d(void *p, int xoffs, int yoffs, int w, int h, int z,
    Uint32 color)
{
	struct widget *wid = p;
	Uint32 lcol, rcol, bcol;
	int x, y;

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
frame_3d(void *p, int xoffs, int yoffs, int w, int h, Uint32 color)
{
	struct widget *wid = p;
	int i;

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
bresenham_circle(void *wid, int xoffs, int yoffs, int w, int h,
    int radius, Uint32 color)
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
	case 1:
		*dst = color;
		break;
	case 2:
		*(Uint16 *)dst = color;
		break;
	case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		dst[0] = (color >> 16)	& 0xff;
		dst[1] = (color >> 8)	& 0xff;
		dst[2] = color		& 0xff;
#else
		dst[0] = color		& 0xff;
		dst[1] = (color >> 8)	& 0xff;
		dst[2] = (color >> 16)	& 0xff;
#endif
		break;
	case 4:
		*(Uint32 *)dst = color;
		break;
	}
}

/* Surface must be locked. */
static __inline__ void
put_pixel2(Uint8 bpp, Uint8 *dst1, Uint8 *dst2, Uint32 color)
{
	switch (bpp) {
	case 1:
		*dst1 = color;
		*dst2 = color;
		break;
	case 2:
		*(Uint16 *)dst1 = color;
		*(Uint16 *)dst2 = color;
		break;
	case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		dst1[0] = (color >> 16)	& 0xff;
		dst1[1] = (color >> 8)	& 0xff;
		dst1[2] = color		& 0xff;
#else
		dst1[0] = color		& 0xff;
		dst1[1] = (color >> 8)	& 0xff;
		dst1[2] = (color >> 16)	& 0xff;
#endif
		break;
	case 4:
		*(Uint32 *)dst1 = color;
		*(Uint32 *)dst2 = color;
		break;
	}
}

static void
bresenham_line(void *wid, int x1, int y1, int x2, int y2, Uint32 color)
{
	int dx, dy, xinc, yinc, xyinc, dpr, dpru, p;
	Uint8 *fb1, *fb2;
	
	x1 += WIDGET(wid)->win->x + WIDGET(wid)->x;
	y1 += WIDGET(wid)->win->y + WIDGET(wid)->y;
	x2 += WIDGET(wid)->win->x + WIDGET(wid)->x;
	y2 += WIDGET(wid)->win->y + WIDGET(wid)->y;

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
composite_square(void *wid, int x, int y, int w, int h, Uint32 color)
{
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

