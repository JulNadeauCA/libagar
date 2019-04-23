/*
 * Copyright (c) 2011-2018 Julien Nadeau Carriere <vedge@csoft.net>
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
#include <agar/gui/gui.h>
#include <agar/gui/geometry.h>

AG_Rect
AG_ReadRect(AG_DataSource *_Nonnull ds)
{
	AG_Rect r;

	r.x = AG_ReadSint16(ds);
	r.y = AG_ReadSint16(ds);
	r.w = AG_ReadUint16(ds);
	r.h = AG_ReadUint16(ds);
	return (r);
}

AG_Rect2
AG_ReadRect2(AG_DataSource *_Nonnull ds)
{
	return AG_RectToRect2(AG_ReadRect(ds));
}

void
AG_WriteRect(AG_DataSource *_Nonnull ds, AG_Rect r)
{
	AG_WriteSint16(ds, r.x);
	AG_WriteSint16(ds, r.y);
	AG_WriteUint16(ds, r.w);
	AG_WriteUint16(ds, r.h);
}

void
AG_WriteRect2(AG_DataSource *_Nonnull ds, AG_Rect2 r)
{
	AG_WriteRect(ds, AG_Rect2ToRect(r));
}

/* Return a Point at x,y. */
AG_Pt
AG_POINT(int x, int y)
{
	AG_Pt pt;
	pt.x = x;
	pt.y = y;
	return (pt);
}

/* Return a Rect of dimensions w,h at position x,y. */
AG_Rect
AG_RECT(int x, int y, int w, int h)
{
	AG_Rect r;
	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;
	return (r);
}

/* Return a Rect2 of dimensions w,h at position x,y. */
AG_Rect2
AG_RECT2(int x, int y, int w, int h)
{
	AG_Rect2 r;
	r.x1 = x;
	r.y1 = y;
	r.w = w;
	r.h = h;
	r.x2 = x+w;
	r.y2 = y+h;
	return (r);
}

/* Convert a Rect2 to a Rect. */
AG_Rect
AG_Rect2ToRect(AG_Rect2 r2)
{
	AG_Rect r;
	r.x = r2.x1;
	r.y = r2.y1;
	r.w = r2.w;
	r.h = r2.h;
	return (r);
}

/* Resize a Rect. */
void
AG_RectSize(AG_Rect *_Nonnull r, int w, int h)
{
	r->w = w;
	r->h = h;
}

/* Resize a Rect2. */
void
AG_RectSize2(AG_Rect2 *_Nonnull r, int w, int h)
{
	r->w = w;
	r->x2 = r->x1+w;
	r->h = h;
	r->y2 = r->y1+h;
}

/* Translate a Rect. */
void
AG_RectTranslate(AG_Rect *_Nonnull r, int x, int y)
{
	r->x += x;
	r->y += y;
}

/* Translate a Rect2. */
void
AG_RectTranslate2(AG_Rect2 *_Nonnull r, int x, int y)
{
	r->x1 += x;
	r->y1 += y;
	r->x2 += x;
	r->y2 += y;
}

/* Convert a Rect to a Rect2. */
AG_Rect2
AG_RectToRect2(AG_Rect r)
{
	AG_Rect2 r2;
	r2.x1 = r.x;
	r2.y1 = r.y;
	r2.w = r.w;
	r2.h = r.h;
	r2.x2 = r.x+r.w;
	r2.y2 = r.y+r.h;
	return (r2);
}

/* Return the intersection of two Rect's. */
AG_Rect
AG_RectIntersect(AG_Rect a, AG_Rect b)
{
	AG_Rect x;
	x.x = AG_MAX(a.x, b.x);
	x.y = AG_MAX(a.y, b.y);
	x.w = AG_MIN((a.x + a.w), (b.x + b.w)) - x.x;
	x.h = AG_MIN((a.y + a.h), (b.y + b.h)) - x.y;
	if (x.w < 0) { x.w = 0; }
	if (x.h < 0) { x.h = 0; }
	return (x);
}

/* Return the intersection of two Rect2's. */
AG_Rect2
AG_RectIntersect2(const AG_Rect2 *a, const AG_Rect2 *b)
{
	AG_Rect2 rx;

	rx.x1 = AG_MAX(a->x1, b->x1);
	rx.y1 = AG_MAX(a->y1, b->y1);
	rx.w = AG_MIN(a->x2, b->x2) - rx.x1;
	if (rx.w < 0) { rx.w = 0; }
	rx.h = AG_MIN(a->y2, b->y2) - rx.y1;
	if (rx.h < 0) { rx.h = 0; }
	rx.x2 = rx.x1 + rx.w;
	rx.y2 = rx.y1 + rx.h;
	return (rx);
}

/* Test whether two Rect's are the same. */
int
AG_RectCompare(AG_Rect a, AG_Rect b)
{
	return !(a.x == b.x &&
	         a.y == b.y &&
		 a.w == b.w &&
		 a.h == b.h);
}

/* Test whether two (valid) Rect2's are the same. */
int
AG_RectCompare2(const AG_Rect2 *a, const AG_Rect2 *b)
{
	return !(a->x1 == b->x1 &&
	         a->y1 == b->y1 &&
		 a->w == b->w &&
		 a->h == b->h);
}

/* Test whether a point intersects a Rect. */
int
AG_RectInside(AG_Rect r, int x, int y)
{
	return (x >= r.x &&
	        y >= r.y &&
		x <  r.x + r.w &&
		y <  r.y + r.h); 
}

/* Test whether a point intersects a Rect2. */
int
AG_RectInside2(const AG_Rect2 *_Nonnull r, int x, int y)
{
	return (x >= r->x1 &&
	        y >= r->y1 &&
		x <  r->x2 &&
		y <  r->y2); 
}
