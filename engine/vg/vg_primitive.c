/*	$Csoft: vg.c,v 1.2 2004/03/18 21:27:48 vedge Exp $	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
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

#include "vg.h"
#include "vg_math.h"
#include "vg_primitive.h"

static __inline__ void
put_pixel1(SDL_Surface *su, Uint8 *dst, Uint32 c)
{
	switch (su->format->BytesPerPixel) {
	case 4:
		*(Uint32 *)dst = c;
		break;
	case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		dst[0] = (c>>16) & 0xff;
		dst[1] = (c>>8) & 0xff;
		dst[2] = c & 0xff;
#else
		dst[0] = c & 0xff;
		dst[1] = (c>>8) & 0xff;
		dst[2] = (c>>16) & 0xff;
#endif
		break;
	case 2:
		*(Uint16 *)dst = c;
		break;
	case 1:
		*dst = c;
		break;
	}
}

static __inline__ void
put_pixel2(SDL_Surface *su, Uint8 *dst1, Uint8 *dst2, Uint32 c)
{
	switch (su->format->BytesPerPixel) {
	case 4:
		*(Uint32 *)dst1 = c;
		*(Uint32 *)dst2 = c;
		break;
	case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		dst2[0] = dst1[0] = (c>>16) & 0xff;
		dst2[1] = dst1[1] = (c>>8) & 0xff;
		dst2[2] = dst1[2] = c & 0xff;
#else
		dst2[0] = dst1[0] = c & 0xff;
		dst2[1] = dst1[1] = (c>>8) & 0xff;
		dst2[2] = dst1[2] = (c>>16) & 0xff;
#endif
		break;
	case 2:
		*(Uint16 *)dst1 = c;
		*(Uint16 *)dst2 = c;
		break;
	case 1:
		*dst1 = c;
		*dst2 = c;
		break;
	}
}

void
vg_putpixel(struct vg *vg, int x, int y, Uint32 c)
{
	Uint8 *dst = (Uint8 *)vg->su->pixels +
	    y*vg->su->pitch + x*vg->su->format->BytesPerPixel;

	put_pixel1(vg->su, dst, c);
}

/* Render a circle using a modified Bresenham line algorithm. */
void
vg_circle_primitive(struct vg *vg, int px, int py, int radius, Uint32 c)
{
	SDL_Surface *su = vg->su;
	int v = 2*radius - 1;
	int e = 0, u = 1;
	int x = 0, y = radius;
	Uint8 *pixel1, *pixel2, *pixel3, *pixel4;

	while (x < y) {
		pixel1 = (Uint8 *)su->pixels + (py+y)*su->pitch +
		    (px+x)*su->format->BytesPerPixel;
		pixel2 = (Uint8 *)su->pixels + (py-y)*su->pitch +
		    (px+x)*su->format->BytesPerPixel;
		pixel3 = (Uint8 *)su->pixels + (py+y)*su->pitch +
		    (px-x)*su->format->BytesPerPixel;
		pixel4 = (Uint8 *)su->pixels + (py-y)*su->pitch +
		    (px-x)*su->format->BytesPerPixel;
	
		switch (su->format->BytesPerPixel) {
		case 4:
			*(Uint32 *)pixel1 = c;
			*(Uint32 *)pixel2 = c;
			*(Uint32 *)pixel3 = c;
			*(Uint32 *)pixel4 = c;
			break;
		case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			pixel1[0] = (c >> 16) & 0xff;
			pixel1[1] = (c >> 8) & 0xff;
			pixel1[2] = c & 0xff;
			pixel2[0] = (c >> 16) & 0xff;
			pixel2[1] = (c >> 8) & 0xff;
			pixel2[2] = c & 0xff;
			pixel3[0] = (c >> 16) & 0xff;
			pixel3[1] = (c >> 8) & 0xff;
			pixel3[2] = c & 0xff;
			pixel4[0] = (c >> 16) & 0xff;
			pixel4[1] = (c >> 8) & 0xff;
			pixel4[2] = c & 0xff;
#else
			pixel1[0] = c & 0xff;
			pixel1[1] = (c >> 8) & 0xff;
			pixel1[2] = (c >> 16) & 0xff;
			pixel2[0] = c & 0xff;
			pixel2[1] = (c >> 8) & 0xff;
			pixel2[2] = (c >> 16) & 0xff;
			pixel3[0] = c & 0xff;
			pixel3[1] = (c >> 8) & 0xff;
			pixel3[2] = (c >> 16) & 0xff;
			pixel4[0] = c & 0xff;
			pixel4[1] = (c >> 8) & 0xff;
			pixel4[2] = (c >> 16) & 0xff;
#endif /* SDL_BYTEORDER */
			break;
		case 2:
			*(Uint16 *)pixel1 = c;
			*(Uint16 *)pixel2 = c;
			*(Uint16 *)pixel3 = c;
			*(Uint16 *)pixel4 = c;
			break;
		case 1:
			*pixel1 = c;
			*pixel2 = c;
			*pixel3 = c;
			*pixel4 = c;
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

		pixel1 = (Uint8 *)su->pixels + (py+x)*su->pitch +
		    (px+y)*su->format->BytesPerPixel;
		pixel2 = (Uint8 *)su->pixels + (py-x)*su->pitch +
		    (px+y)*su->format->BytesPerPixel;
		pixel3 = (Uint8 *)su->pixels + (py+x)*su->pitch +
		    (px-y)*su->format->BytesPerPixel;
		pixel4 = (Uint8 *)su->pixels + (py-x)*su->pitch +
		    (px-y)*su->format->BytesPerPixel;

		switch (su->format->BytesPerPixel) {
		case 4:
			*(Uint32 *)pixel1 = c;
			*(Uint32 *)pixel2 = c;
			*(Uint32 *)pixel3 = c;
			*(Uint32 *)pixel4 = c;
			break;
		case 3:
# if SDL_BYTEORDER == SDL_BIG_ENDIAN
			pixel1[0] = (c >> 16) & 0xff;
			pixel1[1] = (c >> 8) & 0xff;
			pixel1[2] = c & 0xff;
			pixel2[0] = (c >> 16) & 0xff;
			pixel2[1] = (c >> 8) & 0xff;
			pixel2[2] = c & 0xff;
			pixel3[0] = (c >> 16) & 0xff;
			pixel3[1] = (c >> 8) & 0xff;
			pixel3[2] = c & 0xff;
			pixel4[0] = (c >> 16) & 0xff;
			pixel4[1] = (c >> 8) & 0xff;
			pixel4[2] = c & 0xff;
# else
			pixel1[0] = c & 0xff;
			pixel1[1] = (c >> 8) & 0xff;
			pixel1[2] = (c >> 16) & 0xff;
			pixel2[0] = c & 0xff;
			pixel2[1] = (c >> 8) & 0xff;
			pixel2[2] = (c >> 16) & 0xff;
			pixel3[0] = c & 0xff;
			pixel3[1] = (c >> 8) & 0xff;
			pixel3[2] = (c >> 16) & 0xff;
			pixel4[0] = c & 0xff;
			pixel4[1] = (c >> 8) & 0xff;
			pixel4[2] = (c >> 16) & 0xff;
# endif /* SDL_BYTEORDER */
			break;
		case 2:
			*(Uint16 *)pixel1 = c;
			*(Uint16 *)pixel2 = c;
			*(Uint16 *)pixel3 = c;
			*(Uint16 *)pixel4 = c;
			break;
		case 1:
			*pixel1 = c;
			*pixel2 = c;
			*pixel3 = c;
			*pixel4 = c;
			break;
		}
	}
	pixel1 = (Uint8 *)view->v->pixels + py*view->v->pitch +
	    (px-radius)*su->format->BytesPerPixel;
	pixel2 = (Uint8 *)view->v->pixels + py*view->v->pitch +
	    (px+radius)*su->format->BytesPerPixel;
	
	switch (su->format->BytesPerPixel) {
	case 4:
		*(Uint32 *)pixel1 = c;
		*(Uint32 *)pixel2 = c;
		break;
	case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		pixel1[0] = c & 0xff;
		pixel1[1] = (c >> 8) & 0xff;
		pixel1[2] = (c >> 16) & 0xff;
		pixel2[0] = c & 0xff;
		pixel2[1] = (c >> 8) & 0xff;
		pixel2[2] = (c >> 16) & 0xff;
#else
		pixel1[0] = c & 0xff;
		pixel1[1] = (c >> 8) & 0xff;
		pixel1[2] = (c >> 16) & 0xff;
		pixel2[0] = c & 0xff;
		pixel2[1] = (c >> 8) & 0xff;
		pixel2[2] = (c >> 16) & 0xff;
#endif
		break;
	case 2:
		*(Uint16 *)pixel1 = c;
		*(Uint16 *)pixel2 = c;
		break;
	case 1:
		*pixel1 = c;
		*pixel2 = c;
		break;
	}
}

/*
 * Render a straight line between x1,y1 to x2,y2 using a derivate of the
 * line algorithm developed by Jack E. Bresenham.
 */
void
vg_line_primitive(struct vg *vg, int px1, int py1, int px2, int py2,
    Uint32 c)
{
	SDL_Surface *su = vg->su;
	int dx, dy, dpr, dpru, p;
	int yinc = su->pitch;
	int xyinc = su->format->BytesPerPixel + yinc;
	Uint8 *fb1, *fb2;
	int x1 = px1, y1 = py1;				/* Left endpoint */
	int x2 = px2, y2 = py2;				/* Right endpoint */

	if (x1 > x2) {
		dx = x1;
		x1 = x2;
		x2 = dx;
	}
	if (y1 > y2) {
		dx = y1;
		y1 = y2;
		y2 = dx;
	}
	dx = x2 - x1;
	dy = y2 - y1;

	fb1 = (Uint8 *)su->pixels + y1*su->pitch + x1*su->format->BytesPerPixel;
	fb2 = (Uint8 *)su->pixels + y2*su->pitch + x2*su->format->BytesPerPixel;

	if (dy > dx)
		goto indep_y;
/* indep_x: */
	dpr = dy + dy;
	p = -dx;
	dpru = p + p;
	dy = dx >> 1;
xloop:
	put_pixel2(su, fb1, fb2, c);
	if ((p += dpr) > 0)
		goto right_and_up;
/* up: */
	x1++;
	x2--;
	fb1 += su->format->BytesPerPixel;
	fb2 -= su->format->BytesPerPixel;
	if ((dy = dy - 1) >= 0) {
		goto xloop;
	}
	put_pixel1(su, fb1, c);
	if ((dx & 1) == 0) {
		return;
	}
	put_pixel1(su, fb2, c);
	return;
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
	put_pixel1(su, fb1, c);
	if ((dx & 1) == 0) {
		return;
	}
	put_pixel1(su, fb2, c);
	return;
indep_y:
	dpr = dx+dx;
	p = -dy;
	dpru = p+p;
	dx = dy>>1;
yloop:
	put_pixel2(su, fb1, fb2, c);
	if ((p += dpr) > 0)
		goto right_and_up_2;
/* up: */
	y1++;
	y2--;
	fb1 += yinc;
	fb2 -= yinc;
	if ((dx = dx - 1) >= 0) {
		goto yloop;
	}
	put_pixel1(su, fb2, c);
	return;
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
	put_pixel1(su, fb1, c);
	if ((dy & 1) == 0) {
		return;
	}
	put_pixel1(su, fb2, c);
}
