/*
 * Copyright (c) 2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>
#include <agar/micro/gui.h>
#include <agar/micro/geometry.h>

#ifdef AG_SERIALIZATION
void
MA_ReadRect(MA_Rect *r, AG_DataSource *_Nonnull ds)
{
	r->x = AG_ReadUint8(ds);
	r->y = AG_ReadUint8(ds);
	r->w = AG_ReadUint8(ds);
	r->h = AG_ReadUint8(ds);
}

void
MA_ReadRect2(MA_Rect2 *r, AG_DataSource *_Nonnull ds)
{
	r->x1 = AG_ReadUint8(ds);
	r->y1 = AG_ReadUint8(ds);
	r->w  = AG_ReadUint8(ds);
	r->h  = AG_ReadUint8(ds);
	r->x2 = r->x1 + r->w;
	r->y2 = r->y1 + r->h;
}

void
AG_WriteRect(AG_DataSource *_Nonnull ds, const MA_Rect *_Nonnull r)
{
	AG_WriteUint8(ds, r->x);
	AG_WriteUint8(ds, r->y);
	AG_WriteUint8(ds, r->w);
	AG_WriteUint8(ds, r->h);
}

void
AG_WriteRect2(AG_DataSource *_Nonnull ds, const MA_Rect2 *_Nonnull r)
{
	AG_WriteUint8(ds, r->x1);
	AG_WriteUint8(ds, r->y1);
	AG_WriteUint8(ds, r->w);
	AG_WriteUint8(ds, r->h);
}
#endif /* AG_SERIALIZATION */

/* Compute the intersection of two Rect's. */
Uint8
MA_RectIntersect(MA_Rect *rd, const MA_Rect *a, const MA_Rect *b)
{
	Uint8 ax = a->x, ay = a->y;
	Uint8 bx = b->x, by = b->y;
	Sint8  rx, ry;
	Sint16 rw, rh;

	rx = AG_MAX(ax, bx);
	ry = AG_MAX(ay, by);

	rw = AG_MIN((ax + a->w), (bx + b->w)) - rx;
	if (rw < 0) { rw = 0; }
	rh = AG_MIN((ay + a->h), (by + b->h)) - ry;
	if (rh < 0) { rh = 0; }

	rd->x = (Uint8)rx;
	rd->y = (Uint8)ry;
	rd->w = (Uint8)rw;
	rd->h = (Uint8)rh;

	return (rd->w > 0 && rd->h > 0);
}

/* Return the intersection of two Rect2's. */
Uint8
MA_RectIntersect2(MA_Rect2 *rd, const MA_Rect2 *a, const MA_Rect2 *b)
{
	Uint8 ax1 = a->x1, ay1 = a->y1;
	Uint8 ax2 = a->x2, ay2 = a->y2;
	Uint8 bx1 = b->x1, bx2 = b->x2;
	Uint8 by1 = b->y1, by2 = b->y2;
	Sint16 rx1, ry1;
	Sint16 rx2, ry2;
	Sint16 rw, rh;

	rx1 = AG_MAX(ax1, bx1);
	ry1 = AG_MAX(ay1, by1);

	rw = AG_MIN(ax2, bx2) - rx1;
	if (rw < 0) { rw = 0; }
	rx2 = rx1 + rw;

	rh = AG_MIN(ay2, by2) - ry1;
	if (rh < 0) { rh = 0; }
	ry2 = ry1 + rh;
	
	rd->x1 = (Uint8)rx1;
	rd->y1 = (Uint8)ry1;
	rd->w =  (Uint8)rw;
	rd->h =  (Uint8)rh;
	rd->x2 = (Uint8)rx2;
	rd->y2 = (Uint8)ry2;

	return (rd->w > 0 && rd->h > 0);
}

/* Test whether two Rect's are the same. */
Uint8
MA_RectCompare(const MA_Rect *a, const MA_Rect *b)
{
	return !(a->x == b->x &&
	         a->y == b->y &&
		 a->w == b->w &&
		 a->h == b->h);
}

/* Test whether two (valid) Rect2's are the same. */
Uint8
MA_RectCompare2(const MA_Rect2 *a, const MA_Rect2 *b)
{
	return !(a->x1 == b->x1 &&
	         a->y1 == b->y1 &&
		 a->w == b->w &&
		 a->h == b->h);
}

/* Test whether a point intersects a Rect. */
Uint8
MA_RectInside(const MA_Rect *r, Uint8 x, Uint8 y)
{
	Uint8 rx = r->x;
	Uint8 ry = r->y;

	return (x >= rx &&
	        y >= ry &&
		x <  rx + r->w &&
		y <  ry + r->h); 
}

/* Test whether a point intersects a Rect2. */
Uint8
MA_RectInside2(const MA_Rect2 *_Nonnull r, Uint8 x, Uint8 y)
{
	return (x >= r->x1 &&
	        y >= r->y1 &&
		x <  r->x2 &&
		y <  r->y2); 
}
