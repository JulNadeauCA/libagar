/*	$Csoft: primitive.c,v 1.38 2003/03/09 23:34:03 vedge Exp $	    */

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

#include <config/view_8bpp.h>
#include <config/view_16bpp.h>
#include <config/view_24bpp.h>
#include <config/view_32bpp.h>

#include <engine/view.h>

#include "widget.h"
#include "window.h"
#include "label.h"
#include "tlist.h"
#include "primitive.h"

static void	apply(int, union evarg *);

enum {
	BOX,
	FRAME,
	CIRCLE,
	LINE
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

/* Surface must be locked. */
static __inline__ void
put_pixel1(Uint8 *dst, Uint32 color, int x, int y)
{
	if (VIEW_INSIDE_CLIP_RECT(view->v, x, y)) {
		switch (view->v->format->BytesPerPixel) {
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
}

/* Surface must be locked. */
static __inline__ void
put_pixel2(Uint8 *dst1, int x1, int y1, Uint8 *dst2, int x2, int y2,
    Uint32 color)
{
	if (VIEW_INSIDE_CLIP_RECT(view->v, x1, y1)) {
		switch (view->v->format->BytesPerPixel) {
#ifdef VIEW_8BPP
		case 1:
			*dst1 = color;
			break;
#endif
#ifdef VIEW_16BPP
		case 2:
			*(Uint16 *)dst1 = color;
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
			break;
#endif
		}
	}

	if (VIEW_INSIDE_CLIP_RECT(view->v, x2, y2)) {
		switch (view->v->format->BytesPerPixel) {
#ifdef VIEW_8BPP
		case 1:
			*dst2 = color;
			break;
#endif
#ifdef VIEW_16BPP
		case 2:
			*(Uint16 *)dst2 = color;
			break;
#endif
#ifdef VIEW_24BPP
		case 3:
# if SDL_BYTEORDER == SDL_BIG_ENDIAN
			dst2[0] = (color >> 16)	& 0xff;
			dst2[1] = (color >> 8)	& 0xff;
			dst2[2] = color		& 0xff;
# else
			dst2[0] = color		& 0xff;
			dst2[1] = (color >> 8)	& 0xff;
			dst2[2] = (color >> 16)	& 0xff;
# endif
			break;
#endif
#ifdef VIEW_32BPP
		case 4:
			*(Uint32 *)dst2 = color;
			break;
#endif
		}
	}
}

static void
box_rect(void *p, SDL_Rect *rd, int z, Uint32 color)
{
	primitives.box(p, rd->x, rd->y, rd->w, rd->h, z, color);
}
static void
box_2d(void *p, int xoffs, int yoffs, int w, int h, int z,
    Uint32 color)
{
	struct widget *wid = p;
	Uint32 bgcolor;
	SDL_Rect rd;

	bgcolor = (z < 0) ? alter_color(color, -20, -20, -20) : color;
	if (WIDGET_FOCUSED(wid)) {
		bgcolor = alter_color(bgcolor, 6, 6, 15);
	}

	/* Background */
	rd.x = xoffs;
	rd.y = yoffs;
	rd.w = w;
	rd.h = h;
	primitives.rect_filled(wid, &rd, bgcolor);

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

static void
frame_rect(void *p, SDL_Rect *rd, Uint32 color)
{
	primitives.frame(p, rd->x, rd->y, rd->w, rd->h, color);
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

static void
line_bresenham(void *wid, int px1, int py1, int px2, int py2, Uint32 color)
{
	int dx, dy, dpr, dpru, p;
	int x1, x2, y1, y2;
	int xinc = view->v->format->BytesPerPixel;
	int yinc = view->v->pitch;
	int xyinc = xinc + yinc;
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
	fb1 += xinc;
	fb2 -= xinc;
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

#ifdef HAVE_OPENGL
static void
line_opengl(void *wid, int x1, int y1, int x2, int y2, Uint32 color)
{
	Uint8 r, g, b;
	
	SDL_GetRGB(color, view->v->format, &r, &g, &b);

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
#endif /* HAVE_OPENGL */

static void
rect_outlined(void *p, int x, int y, int w, int h, Uint32 color)
{
	struct widget *wid = p;

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
rect_filled(void *p, SDL_Rect *rd, Uint32 color)
{
	SDL_Rect nrd = *rd;

	nrd.x += WIDGET(p)->win->rd.x + WIDGET(p)->x;
	nrd.y += WIDGET(p)->win->rd.y + WIDGET(p)->y;

	SDL_FillRect(view->v, &nrd, color);
}

#ifdef HAVE_OPENGL
static void
rect_opengl(void *p, SDL_Rect *rd, Uint32 color)
{
	SDL_Rect nrd = *rd;
	Uint8 r, g, b;

	nrd.x += WIDGET(p)->win->rd.x + WIDGET(p)->x;
	nrd.y += WIDGET(p)->win->rd.y + WIDGET(p)->y;

	SDL_GetRGB(color, view->v->format, &r, &g, &b);
	glColor3ub(r, g, b);
	glRecti(nrd.x, nrd.y, nrd.x+nrd.w, nrd.y+nrd.h);
}
#endif /* HAVE_OPENGL */

/* Default primitives ops */
struct primitive_ops primitives = {
	box_3d,			/* box */
	box_rect,		/* box (SDL_Rect) */
	frame_3d,		/* frame */
	frame_rect,		/* frame (SDL_Rect) */
	circle_bresenham,	/* circle */
	line_bresenham,		/* line */
	rect_outlined,		/* outlined rectangle */
	rect_filled		/* filled rectangle */
};

void
primitives_init(void)
{
#ifdef HAVE_OPENGL
	if (view->opengl) {
		primitives.line = line_opengl;
		primitives.rect_filled = rect_opengl;
	}
#endif
}

struct window *
primitive_config_window(void)
{
	struct window *win;
	struct region *reg;
	struct label *lab;
	struct tlist *tl;

	win = window_new("widget-primitive-sw", WINDOW_CENTER, -1, -1,
	    303, 190, 303, 190);
	window_set_caption(win, "Widget primitives");

	reg = region_new(win, REGION_VALIGN, 0, 0, 50, 100);
	{
		lab = label_new(reg, 100, 10, "Box:");
		tl = tlist_new(reg, 100, 35, 0);
		tlist_insert_item(tl, NULL,
		    "2d-style", box_2d);
		tlist_insert_item_selected(tl, NULL,
		    "3d-style", box_3d);
		event_new(tl, "tlist-changed", apply, "%i", BOX);

		lab = label_new(reg, 100, 10, "Frame:");
		tl = tlist_new(reg, 100, 35, 0);
		tlist_insert_item_selected(tl, NULL,
		    "3d-style", frame_3d);
		event_new(tl, "tlist-changed", apply, "%i", FRAME);
	}

	reg = region_new(win, REGION_VALIGN, 50, 0, 50, 100);
	{	
		struct tlist_item *it_bres, *it;
	
		lab = label_new(reg, 100, 10, "Line:");
		tl = tlist_new(reg, 100, 35, 0);
		it_bres = tlist_insert_item_selected(tl, NULL,
		    "Bresenham", line_bresenham);
#ifdef HAVE_OPENGL
		it = tlist_insert_item(tl, NULL,
		    "OpenGL", line_opengl);
		if (view->opengl) {
			tlist_select(it);
			tlist_unselect(it_bres);
		}
#endif
		event_new(tl, "tlist-changed", apply, "%i", LINE);
		
		lab = label_new(reg, 100, 10, "Circle:");
		tl = tlist_new(reg, 100, 35, 0);
		tlist_insert_item_selected(tl, NULL,
		    "Bresenham", circle_bresenham);
		event_new(tl, "tlist-changed", apply, "%i", CIRCLE);
	}
	return (win);
}

static void
apply(int argc, union evarg *argv)
{
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
	}
}

