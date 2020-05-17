/*
 * Copyright (c) 2011-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

void
AG_ReadRect(AG_Rect *r, AG_DataSource *_Nonnull ds)
{
	r->x = AG_ReadSint16(ds);
	r->y = AG_ReadSint16(ds);
	r->w = AG_ReadUint16(ds);
	r->h = AG_ReadUint16(ds);
}

void
AG_ReadRect2(AG_Rect2 *r, AG_DataSource *_Nonnull ds)
{
	r->x1 = AG_ReadSint16(ds);
	r->y1 = AG_ReadSint16(ds);
	r->w  = AG_ReadUint16(ds);
	r->h  = AG_ReadUint16(ds);
	r->x2 = r->x1 + r->w;
	r->y2 = r->y1 + r->h;
}

void
AG_WriteRect(AG_DataSource *_Nonnull ds, const AG_Rect *_Nonnull r)
{
	AG_WriteSint16(ds, r->x);
	AG_WriteSint16(ds, r->y);
	AG_WriteUint16(ds, r->w);
	AG_WriteUint16(ds, r->h);
}

void
AG_WriteRect2(AG_DataSource *_Nonnull ds, const AG_Rect2 *_Nonnull r)
{
	AG_WriteSint16(ds, r->x1);
	AG_WriteSint16(ds, r->y1);
	AG_WriteUint16(ds, r->w);
	AG_WriteUint16(ds, r->h);
}

/* Return a Point at x,y. */
void
AG_PtInit(AG_Pt *pt, int x, int y)
{
	pt->x = x;
	pt->y = y;
}

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

/* Return a Rect of dimensions w,h at position x,y. */
void
AG_RectInit(AG_Rect *rd, int x, int y, int w, int h)
{
	rd->x = x;
	rd->y = y;
	rd->w = w;
	rd->h = h;
}

/* Return a Rect2 of dimensions w,h at position x,y. */
void
AG_Rect2Init(AG_Rect2 *rd, int x, int y, int w, int h)
{
	rd->x1 = x;
	rd->y1 = y;
	rd->w = w;
	rd->h = h;
	rd->x2 = x + w;
	rd->y2 = y + h;
}

/* Convert a Rect2 to a Rect. */
void
AG_Rect2ToRect(AG_Rect *rd, const AG_Rect2 *r2)
{
	rd->x = r2->x1;
	rd->y = r2->y1;
	rd->w = r2->w;
	rd->h = r2->h;
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
void
AG_RectToRect2(AG_Rect2 *rd, const AG_Rect *r)
{
	int x = r->x;
	int y = r->y;
	int w = r->w;
	int h = r->h;

	rd->x1 = x;
	rd->y1 = y;
	rd->w = w;
	rd->h = h;
	rd->x2 = x+w;
	rd->y2 = y+h;
}

/* Return the intersection of two Rect's. */
int
AG_RectIntersect(AG_Rect *rd, const AG_Rect *a, const AG_Rect *b)
{
	int ax = a->x, ay = a->y;
	int bx = b->x, by = b->y;
	AG_Rect r;

	r.x = AG_MAX(ax, bx);
	r.y = AG_MAX(ay, by);

	r.w = AG_MIN((ax + a->w), (bx + b->w)) - r.x;
	if (r.w < 0) { r.w = 0; }

	r.h = AG_MIN((ay + a->h), (by + b->h)) - r.y;
	if (r.h < 0) { r.h = 0; }

	*rd = r;
	return (r.w > 0 && r.h > 0);
}

/* Return the intersection of two Rect2's. */
int
AG_RectIntersect2(AG_Rect2 *rd, const AG_Rect2 *a, const AG_Rect2 *b)
{
	int ax1 = a->x1, ay1 = a->y1;
	int ax2 = a->x2, ay2 = a->y2;
	int bx1 = b->x1, bx2 = b->x2;
	int by1 = b->y1, by2 = b->y2;
	AG_Rect2 r;

	r.x1 = AG_MAX(ax1, bx1);
	r.y1 = AG_MAX(ay1, by1);

	r.w = AG_MIN(ax2, bx2) - r.x1;
	if (r.w < 0) { r.w = 0; }
	r.x2 = r.x1 + r.w;

	r.h = AG_MIN(ay2, by2) - r.y1;
	if (r.h < 0) { r.h = 0; }
	r.y2 = r.y1 + r.h;

	*rd = r;
	return (r.w > 0 && r.h > 0);
}

/* Test whether two Rect's are the same. */
int
AG_RectCompare(const AG_Rect *a, const AG_Rect *b)
{
	return !(a->x == b->x &&
	         a->y == b->y &&
		 a->w == b->w &&
		 a->h == b->h);
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
AG_RectInside(const AG_Rect *r, int x, int y)
{
	int rx = r->x;
	int ry = r->y;

	return (x >= rx &&
	        y >= ry &&
		x <  rx + r->w &&
		y <  ry + r->h); 
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
