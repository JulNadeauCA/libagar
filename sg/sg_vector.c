/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * Basic linear algebra routines for vectors in R3.
 */

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>
#include "sg.h"

SG_Vector2
SG_GetVector2(SG_Real x, SG_Real y)
{
	SG_Vector2 v;

	v.x = x;
	v.y = y;
	return (v);
}

SG_Vector
SG_GetVector(SG_Real x, SG_Real y, SG_Real z)
{
	SG_Vector v;

	v.x = x;
	v.y = y;
	v.z = z;
	return (v);
}

SG_Vector4
SG_GetVector4(SG_Real x, SG_Real y, SG_Real z, SG_Real w)
{
	SG_Vector4 v;

	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return (v);
}

SG_Vector2
SG_Vector3to2(SG_Vector v)
{
	SG_Vector2 v2;
	v2.x = v.x;
	v2.y = v.y;
	return (v2);
}

SG_Vector
SG_Vector2to3(SG_Vector2 v)
{
	SG_Vector v3;
	v3.x = v.x;
	v3.y = v.y;
	v3.z = 0.0;
	return (v3);
}

SG_Vector4
SG_Vector3to4(SG_Vector v)
{
	SG_Vector4 v4;
	v4.x = v.x;
	v4.y = v.y;
	v4.z = v.z;
	v4.w = 0.0;
	return (v4);
}

SG_Vector2
SG_GetZeroVector2(void)
{
	SG_Vector2 v;

	v.x = 0.0;
	v.y = 0.0;
	return (v);
}

SG_Vector
SG_GetZeroVector(void)
{
	SG_Vector v;

	v.x = 0.0;
	v.y = 0.0;
	v.z = 0.0;
	return (v);
}

SG_Vector4
SG_GetZeroVector4(void)
{
	SG_Vector4 v;

	v.x = 0.0;
	v.y = 0.0;
	v.z = 0.0;
	v.w = 0.0;
	return (v);
}

void
SG_SetVector2(SG_Vector2 *v, SG_Real x, SG_Real y)
{
	v->x = x;
	v->y = y;
}

void
SG_SetVector(SG_Vector *v, SG_Real x, SG_Real y, SG_Real z)
{
	v->x = x;
	v->y = y;
	v->z = z;
}

void
SG_SetVector4(SG_Vector4 *v, SG_Real x, SG_Real y, SG_Real z, SG_Real w)
{
	v->x = x;
	v->y = y;
	v->z = z;
	v->w = w;
}

void
SG_VectorCopy2(SG_Vector2 *vDst, const SG_Vector2 *vSrc)
{
	vDst->x = vSrc->x;
	vDst->y = vSrc->y;
}

void
SG_VectorCopy(SG_Vector *vDst, const SG_Vector *vSrc)
{
	vDst->x = vSrc->x;
	vDst->y = vSrc->y;
	vDst->z = vSrc->z;
}

void
SG_VectorCopy4(SG_Vector4 *vDst, const SG_Vector4 *vSrc)
{
	vDst->x = vSrc->x;
	vDst->y = vSrc->y;
	vDst->z = vSrc->z;
	vDst->w = vSrc->w;
}

SG_Vector2
SG_VectorMirror2(SG_Vector2 a, int x, int y)
{
	SG_Vector2 b;

	b.x = x ? -a.x : a.x;
	b.y = y ? -a.y : a.y;
	return (b);
}

SG_Vector
SG_VectorMirror(SG_Vector a, int x, int y, int z)
{
	SG_Vector b;

	b.x = x ? -a.x : a.x;
	b.y = y ? -a.y : a.y;
	b.z = z ? -a.z : a.z;
	return (b);
}

SG_Vector4
SG_VectorMirror4(SG_Vector4 a, int x, int y, int z, int w)
{
	SG_Vector4 b;

	b.x = x ? -a.x : a.x;
	b.y = y ? -a.y : a.y;
	b.z = z ? -a.z : a.z;
	b.w = w ? -a.w : a.w;
	return (b);
}

SG_Vector2
SG_VectorMirror2p(const SG_Vector2 *a, int x, int y)
{
	SG_Vector2 b;

	b.x = x ? -(a->x) : a->x;
	b.y = y ? -(a->y) : a->y;
	return (b);
}

SG_Vector
SG_VectorMirrorp(const SG_Vector *a, int x, int y, int z)
{
	SG_Vector b;

	b.x = x ? -(a->x) : a->x;
	b.y = y ? -(a->y) : a->y;
	b.z = z ? -(a->z) : a->z;
	return (b);
}

SG_Vector4
SG_VectorMirror4p(const SG_Vector4 *a, int x, int y, int z, int w)
{
	SG_Vector4 b;

	b.x = x ? -(a->x) : a->x;
	b.y = y ? -(a->y) : a->y;
	b.z = z ? -(a->z) : a->z;
	b.w = w ? -(a->w) : a->w;
	return (b);
}

SG_Real
SG_VectorLen2(SG_Vector2 v)
{
	return Hypot2(v.x, v.y);
}

SG_Real
SG_VectorLen(SG_Vector v)
{
	return Sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

SG_Real
SG_VectorLen4(SG_Vector4 v)
{
	return Sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);
}

SG_Real
SG_VectorLen2p(const SG_Vector2 *v)
{
	return Hypot2(v->x, v->y);
}

SG_Real
SG_VectorLenp(const SG_Vector *v)
{
	return Sqrt(v->x*v->x + v->y*v->y + v->z*v->z);
}

SG_Real
SG_VectorLen4p(const SG_Vector4 *v)
{
	return Sqrt(v->x*v->x + v->y*v->y + v->z*v->z + v->w*v->w);
}

SG_Real
SG_VectorDot2(SG_Vector2 v1, SG_Vector2 v2)
{
	return (v1.x*v2.x + v1.y*v2.y);
}

SG_Real
SG_VectorDot(SG_Vector v1, SG_Vector v2)
{
	return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}

SG_Real
SG_VectorDot4(SG_Vector4 v1, SG_Vector4 v2)
{
	return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w);
}

SG_Real
SG_VectorDot2p(const SG_Vector2 *v1, const SG_Vector2 *v2)
{
	return (v1->x*v2->x + v1->y*v2->y);
}

SG_Real
SG_VectorDotp(const SG_Vector *v1, const SG_Vector *v2)
{
	return (v1->x*v2->x + v1->y*v2->y + v1->z*v2->z);
}

SG_Real
SG_VectorDot4p(const SG_Vector4 *v1, const SG_Vector4 *v2)
{
	return (v1->x*v2->x + v1->y*v2->y + v1->z*v2->z + v1->w*v2->w);
}

SG_Real
SG_VectorPerpDot2(SG_Vector2 v1, SG_Vector2 v2)
{
	return (v1.x*v2.y - v1.y*v2.x);
}

SG_Real
SG_VectorPerpDot2p(const SG_Vector2 *v1, const SG_Vector2 *v2)
{
	return (v1->x*v2->y - v1->y*v2->x);
}

SG_Real
SG_VectorDistance2(SG_Vector2 a, SG_Vector2 b)
{
	return SG_VectorLen2( SG_VectorSub2(a,b) );
}

SG_Real
SG_VectorDistance(SG_Vector a, SG_Vector b)
{
	return SG_VectorLen( SG_VectorSub(a,b) );
}

SG_Real
SG_VectorDistance4(SG_Vector4 a, SG_Vector4 b)
{
	return SG_VectorLen4( SG_VectorSub4(a,b) );
}

SG_Real
SG_VectorDistance2p(const SG_Vector2 *a, const SG_Vector2 *b)
{
	return SG_VectorLen2( SG_VectorAdd2(*b, SG_VectorMirror2p(a,1,1)) );
}

SG_Real
SG_VectorDistancep(const SG_Vector *a, const SG_Vector *b)
{
	return SG_VectorLen(SG_VectorAdd(*b, SG_VectorMirrorp(a,1,1,1)));
}

SG_Real
SG_VectorDistance4p(const SG_Vector4 *a, const SG_Vector4 *b)
{
	return SG_VectorLen4( SG_VectorAdd4(*b, SG_VectorMirror4p(a,1,1,1,1)) );
}

SG_Vector2
SG_VectorNorm2(SG_Vector2 v)
{
	SG_Vector2 n;
	SG_Real len;

	if ((len = SG_VectorLen2p(&v)) == 0.0) {
		return (v);
	}
	n.x = v.x/len;
	n.y = v.y/len;
	return (n);
}

SG_Vector
SG_VectorNorm(SG_Vector v)
{
	SG_Vector n;
	SG_Real len;

	if ((len = SG_VectorLenp(&v)) == 0.0) {
		return (v);
	}
	n.x = v.x/len;
	n.y = v.y/len;
	n.z = v.z/len;
	return (n);
}

SG_Vector4
SG_VectorNorm4(SG_Vector4 v)
{
	SG_Vector4 n;
	SG_Real len;

	if ((len = SG_VectorLen4p(&v)) == 0.0) {
		return (v);
	}
	n.x = v.x/len;
	n.y = v.y/len;
	n.z = v.z/len;
	n.w = v.w/len;
	return (n);
}

SG_Vector2
SG_VectorNorm2p(const SG_Vector2 *v)
{
	SG_Vector2 n;
	SG_Real len;

	if ((len = SG_VectorLen2p(v)) == 0.0) {
		return (*v);
	}
	n.x = v->x/len;
	n.y = v->y/len;
	return (n);
}

SG_Vector
SG_VectorNormp(const SG_Vector *v)
{
	SG_Vector n;
	SG_Real len;

	if ((len = SG_VectorLenp(v)) == 0.0) {
		return (*v);
	}
	n.x = v->x/len;
	n.y = v->y/len;
	n.z = v->z/len;
	return (n);
}

SG_Vector4
SG_VectorNorm4p(const SG_Vector4 *v)
{
	SG_Vector4 n;
	SG_Real len;

	if ((len = SG_VectorLen4p(v)) == 0.0) {
		return (*v);
	}
	n.x = v->x/len;
	n.y = v->y/len;
	n.z = v->z/len;
	n.w = v->w/len;
	return (n);
}

void
SG_VectorNorm2v(SG_Vector2 *v)
{
	SG_Real len;

	if ((len = SG_VectorLen2p(v)) == 0.0) {
		return;
	}
	v->x /= len;
	v->y /= len;
}

void
SG_VectorNormv(SG_Vector *v)
{
	SG_Real len;

	if ((len = SG_VectorLenp(v)) == 0.0) {
		return;
	}
	v->x /= len;
	v->y /= len;
	v->z /= len;
}

void
SG_VectorNorm4v(SG_Vector4 *v)
{
	SG_Real len;

	if ((len = SG_VectorLen4p(v)) == 0.0) {
		return;
	}
	v->x /= len;
	v->y /= len;
	v->z /= len;
	v->w /= len;
}

SG_Vector
SG_VectorCross(SG_Vector a, SG_Vector b)
{
	return (SG_VectorCrossp(&a, &b));
}

SG_Vector4
SG_VectorCross4(SG_Vector4 a, SG_Vector4 b, SG_Vector4 c)
{
	return (SG_VectorCross4p(&a, &b, &c));
}

SG_Vector
SG_VectorCrossp(const SG_Vector *a, const SG_Vector *b)
{
	SG_Vector c;

	c.x = a->y*b->z - b->y*a->z;
	c.y = a->z*b->x - b->z*a->x;
	c.z = a->x*b->y - b->x*a->y;
	return (c);
}

SG_Vector4
SG_VectorCross4p(const SG_Vector4 *u, const SG_Vector4 *v, const SG_Vector4 *w)
{
	SG_Real a, b, c, d, e, f;
	SG_Vector4 r;

	a = v->x*w->y - v->y*w->x;
	b = v->x*w->z - v->z*w->x;
	c = v->x*w->w - v->w*w->x;
	d = v->y*w->z - v->z*w->y;
	e = v->y*w->w - v->w*w->y;
	f = v->z*w->w - v->w*w->z;
	r.x =  u->y*f - u->z*e + u->w*d;
	r.y = -u->x*f + u->z*c - u->w*b;
	r.z =  u->x*e + u->y*c - u->w*a;
	r.w = -u->x*d + u->y*b - u->z*a;
	return (r);
}

SG_Vector
SG_VectorNormCross(SG_Vector a, SG_Vector b)
{
	SG_Vector c;

	c.x = a.y*b.z - b.y*a.z;
	c.y = a.z*b.x - b.z*a.x;
	c.z = a.x*b.y - b.x*a.y;
	SG_VectorNormp(&c);
	return (c);
}

SG_Vector4
SG_VectorNormCross4(SG_Vector4 u, SG_Vector4 v, SG_Vector4 w)
{
	SG_Vector4 c;

	c = SG_VectorCross4p(&u, &v, &w);
	SG_VectorNorm4p(&c);
	return (c);
}

SG_Vector
SG_VectorNormCrossp(const SG_Vector *a, const SG_Vector *b)
{
	SG_Vector c;

	c.x = a->y*b->z - b->y*a->z;
	c.y = a->z*b->x - b->z*a->x;
	c.z = a->x*b->y - b->x*a->y;
	SG_VectorNormp(&c);
	return (c);
}

SG_Vector4
SG_VectorNormCross4p(const SG_Vector4 *u, const SG_Vector4 *v,
    const SG_Vector4 *w)
{
	SG_Vector4 c;

	c = SG_VectorCross4p(u, v, w);
	SG_VectorNorm4p(&c);
	return (c);
}

SG_Vector2
SG_VectorScale2(SG_Vector2 a, SG_Real c)
{
	SG_Vector2 b;

	b.x = a.x*c;
	b.y = a.y*c;
	return (b);
}

SG_Vector
SG_VectorScale(SG_Vector a, SG_Real c)
{
	SG_Vector b;

	b.x = a.x*c;
	b.y = a.y*c;
	b.z = a.z*c;
	return (b);
}

SG_Vector4
SG_VectorScale4(SG_Vector4 a, SG_Real c)
{
	SG_Vector4 b;

	b.x = a.x*c;
	b.y = a.y*c;
	b.z = a.z*c;
	b.w = a.w*c;
	return (b);
}

SG_Vector2
SG_VectorScale2p(const SG_Vector2 *a, SG_Real c)
{
	SG_Vector2 b;

	b.x = a->x*c;
	b.y = a->y*c;
	return (b);
}

SG_Vector
SG_VectorScalep(const SG_Vector *a, SG_Real c)
{
	SG_Vector b;

	b.x = a->x*c;
	b.y = a->y*c;
	b.z = a->z*c;
	return (b);
}

SG_Vector4
SG_VectorScale4p(const SG_Vector4 *a, SG_Real c)
{
	SG_Vector4 b;

	b.x = a->x*c;
	b.y = a->y*c;
	b.z = a->z*c;
	b.w = a->w*c;
	return (b);
}

void
SG_VectorScale2v(SG_Vector2 *a, SG_Real c)
{
	a->x *= c;
	a->y *= c;
}

void
SG_VectorScalev(SG_Vector *a, SG_Real c)
{
	a->x *= c;
	a->y *= c;
	a->z *= c;
}

void
SG_VectorScale4v(SG_Vector4 *a, SG_Real c)
{
	a->x *= c;
	a->y *= c;
	a->z *= c;
	a->w *= c;
}

SG_Vector2
SG_VectorAdd2(SG_Vector2 a, SG_Vector2 b)
{
	SG_Vector2 c;

	c.x = a.x + b.x;
	c.y = a.y + b.y;
	return (c);
}

SG_Vector
SG_VectorAdd(SG_Vector a, SG_Vector b)
{
	SG_Vector c;

	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;
	return (c);
}

SG_Vector4
SG_VectorAdd4(SG_Vector4 a, SG_Vector4 b)
{
	SG_Vector4 c;

	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;
	c.w = a.w + b.w;
	return (c);
}

SG_Vector2
SG_VectorAdd2p(const SG_Vector2 *a, const SG_Vector2 *b)
{
	SG_Vector2 c;

	c.x = a->x + b->x;
	c.y = a->y + b->y;
	return (c);
}

SG_Vector
SG_VectorAddp(const SG_Vector *a, const SG_Vector *b)
{
	SG_Vector c;

	c.x = a->x + b->x;
	c.y = a->y + b->y;
	c.z = a->z + b->z;
	return (c);
}

SG_Vector4
SG_VectorAdd4p(const SG_Vector4 *a, const SG_Vector4 *b)
{
	SG_Vector4 c;

	c.x = a->x + b->x;
	c.y = a->y + b->y;
	c.z = a->z + b->z;
	c.w = a->w + b->w;
	return (c);
}

void
SG_VectorAddv(SG_Vector *r, const SG_Vector *a)
{
	r->x += a->x;
	r->y += a->y;
	r->z += a->z;
}

SG_Vector2
SG_VectorAdd2n(int nvecs, ...)
{
	SG_Vector2 c, *v;
	int i;
	va_list ap;

	va_start(ap, nvecs);
	v = va_arg(ap, void *);
	c.x = v->x;
	c.y = v->y;
	for (i = 0; i < nvecs; i++) {
		v = va_arg(ap, void *);
		c.x += v->x;
		c.y += v->y;
	}
	va_end(ap);
	return (c);
}

SG_Vector
SG_VectorAddn(int nvecs, ...)
{
	SG_Vector c, *v;
	int i;
	va_list ap;

	va_start(ap, nvecs);
	v = va_arg(ap, void *);
	c.x = v->x;
	c.y = v->y;
	c.z = v->z;
	for (i = 0; i < nvecs; i++) {
		v = va_arg(ap, void *);
		c.x += v->x;
		c.y += v->y;
		c.z += v->z;
	}
	va_end(ap);
	return (c);
}

SG_Vector4
SG_VectorAdd4n(int nvecs, ...)
{
	SG_Vector4 c, *v;
	int i;
	va_list ap;

	va_start(ap, nvecs);
	v = va_arg(ap, void *);
	c.x = v->x;
	c.y = v->y;
	c.z = v->z;
	c.w = v->w;
	for (i = 0; i < nvecs; i++) {
		v = va_arg(ap, void *);
		c.x += v->x;
		c.y += v->y;
		c.z += v->z;
		c.w += v->w;
	}
	va_end(ap);
	return (c);
}

SG_Vector2
SG_VectorSub2(SG_Vector2 a, SG_Vector2 b)
{
	SG_Vector2 c;

	c.x = a.x - b.x;
	c.y = a.y - b.y;
	return (c);
}

SG_Vector
SG_VectorSub(SG_Vector a, SG_Vector b)
{
	SG_Vector c;

	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
	return (c);
}

SG_Vector4
SG_VectorSub4(SG_Vector4 a, SG_Vector4 b)
{
	SG_Vector4 c;

	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
	c.w = a.w - b.w;
	return (c);
}

SG_Vector2
SG_VectorSub2p(const SG_Vector2 *a, const SG_Vector2 *b)
{
	SG_Vector2 c;

	c.x = a->x - b->x;
	c.y = a->y - b->y;
	return (c);
}

SG_Vector
SG_VectorSubp(const SG_Vector *a, const SG_Vector *b)
{
	SG_Vector c;

	c.x = a->x - b->x;
	c.y = a->y - b->y;
	c.z = a->z - b->z;
	return (c);
}

SG_Vector4
SG_VectorSub4p(const SG_Vector4 *a, const SG_Vector4 *b)
{
	SG_Vector4 c;

	c.x = a->x - b->x;
	c.y = a->y - b->y;
	c.z = a->z - b->z;
	c.w = a->w - b->w;
	return (c);
}

void
SG_VectorSubv(SG_Vector *r, const SG_Vector *a)
{
	r->x -= a->x;
	r->y -= a->y;
	r->z -= a->z;
}

SG_Vector2
SG_VectorSub2n(int nvecs, ...)
{
	SG_Vector2 c, *v;
	int i;
	va_list ap;

	va_start(ap, nvecs);
	v = va_arg(ap, void *);
	c.x = v->x;
	c.y = v->y;
	for (i = 0; i < nvecs; i++) {
		v = va_arg(ap, void *);
		c.x -= v->x;
		c.y -= v->y;
	}
	va_end(ap);
	return (c);
}

SG_Vector
SG_VectorSubn(int nvecs, ...)
{
	SG_Vector c, *v;
	int i;
	va_list ap;

	va_start(ap, nvecs);
	v = va_arg(ap, void *);
	c.x = v->x;
	c.y = v->y;
	c.z = v->z;
	for (i = 0; i < nvecs; i++) {
		v = va_arg(ap, void *);
		c.x -= v->x;
		c.y -= v->y;
		c.z -= v->z;
	}
	va_end(ap);
	return (c);
}

SG_Vector4
SG_VectorSub4n(int nvecs, ...)
{
	SG_Vector4 c, *v;
	int i;
	va_list ap;

	va_start(ap, nvecs);
	v = va_arg(ap, void *);
	c.x = v->x;
	c.y = v->y;
	c.z = v->z;
	c.w = v->w;
	for (i = 0; i < nvecs; i++) {
		v = va_arg(ap, void *);
		c.x -= v->x;
		c.y -= v->y;
		c.z -= v->z;
		c.w -= v->w;
	}
	va_end(ap);
	return (c);
}

SG_Vector2
SG_VectorAvg2(SG_Vector2 a, SG_Vector2 b)
{
	SG_Vector2 c;
	
	c.x = (a.x + b.x)/2.0;
	c.y = (a.y + b.y)/2.0;
	return (c);
}

SG_Vector
SG_VectorAvg(SG_Vector a, SG_Vector b)
{
	SG_Vector c;
	
	c.x = (a.x + b.x)/2.0;
	c.y = (a.y + b.y)/2.0;
	c.z = (a.z + b.z)/2.0;
	return (c);
}

SG_Vector4
SG_VectorAvg4(SG_Vector4 a, SG_Vector4 b)
{
	SG_Vector4 c;
	
	c.x = (a.x + b.x)/2.0;
	c.y = (a.y + b.y)/2.0;
	c.z = (a.z + b.z)/2.0;
	c.w = (a.w + b.w)/2.0;
	return (c);
}

SG_Vector2
SG_VectorAvg2p(const SG_Vector2 *a, const SG_Vector2 *b)
{
	SG_Vector2 c;
	
	c.x = (a->x + b->x)/2.0;
	c.y = (a->y + b->y)/2.0;
	return (c);
}

SG_Vector
SG_VectorAvgp(const SG_Vector *a, const SG_Vector *b)
{
	SG_Vector c;
	
	c.x = (a->x + b->x)/2.0;
	c.y = (a->y + b->y)/2.0;
	c.z = (a->z + b->z)/2.0;
	return (c);
}

SG_Vector4
SG_VectorAvg4p(const SG_Vector4 *a, const SG_Vector4 *b)
{
	SG_Vector4 c;
	
	c.x = (a->x + b->x)/2.0;
	c.y = (a->y + b->y)/2.0;
	c.z = (a->z + b->z)/2.0;
	c.w = (a->w + b->w)/2.0;
	return (c);
}

SG_Real
SG_VectorVectorAngle2(SG_Vector2 vOrig, SG_Vector2 vOther)
{
	SG_Vector2 vd;
	
	vd = SG_VectorSub2p(&vOther, &vOrig);
	return (Atan2(vd.y, vd.x));
}

void
SG_VectorVectorAngle(SG_Vector vOrig, SG_Vector vOther, SG_Real *theta,
    SG_Real *phi)
{
	SG_Spherical sph;
	SG_Vector vd;

	vd = SG_VectorSubp(&vOther, &vOrig);
	if (theta != NULL) { *theta = Atan2(vd.y, vd.x); }
	if (phi != NULL) { *phi = Atan2(Sqrt(vd.x*vd.x + vd.y*vd.y), vd.z); }
}

void
SG_VectorVectorAngle4(SG_Vector4 vOrig, SG_Vector4 vOther, SG_Real *phi1,
    SG_Real *phi2, SG_Real *phi3)
{
	SG_Spherical sph;
	SG_Vector4 vd;

	vd = SG_VectorSub4p(&vOther, &vOrig);
	if (phi1 != NULL)
		*phi1 = Atan2(Sqrt(vd.w*vd.w + vd.z*vd.z + vd.y*vd.y),	vd.x);
	if (phi2 != NULL)
		*phi2 = Atan2(Sqrt(vd.w*vd.w + vd.z*vd.z),		vd.y);
	if (phi3 != NULL)
		*phi3 = Atan2(vd.w,					vd.z);
}

SG_Vector2
SG_VectorRotate2(SG_Vector2 v, SG_Real theta)
{
	SG_Vector2 r;
	SG_Real s = Sin(theta);
	SG_Real c = Cos(theta);
	
	r.x = c*v.x - s*v.y;
	r.y = s*v.x + c*v.y;
	return (r);
}

SG_Vector
SG_VectorRotate(SG_Vector v, SG_Real theta, SG_Vector n)
{
	SG_Vector r = v;

	SG_VectorRotatev(&r, theta, n);
	return (r);
}

SG_Vector4
SG_VectorRotate4(SG_Vector4 v, SG_Real theta, SG_Vector4 n)
{
	SG_Vector4 r = v;

	SG_VectorRotate4v(&r, theta, n);
	return (r);
}

void
SG_VectorRotate2v(SG_Vector2 *v, SG_Real theta)
{
	SG_Vector2 r;
	SG_Real s = Sin(theta);
	SG_Real c = Cos(theta);

	r.x = c*v->x - s*v->y;
	r.y = s*v->x + c*v->y;
	SG_VectorCopy2(v, &r);
}

void
SG_VectorRotatev(SG_Vector *v, SG_Real theta, SG_Vector n)
{
	SG_Real s = Sin(theta);
	SG_Real c = Cos(theta);
	SG_Real t = 1.0 - c;
	SG_Matrix R;

	R.m[0][0] = t*n.x*n.x + c;
	R.m[0][1] = t*n.x*n.y + s*n.z;
	R.m[0][2] = t*n.x*n.z - s*n.y;
	R.m[0][3] = 0.0;
	R.m[1][0] = t*n.x*n.y - s*n.z;
	R.m[1][1] = t*n.y*n.y + c;
	R.m[1][2] = t*n.y*n.z + s*n.x;
	R.m[1][3] = 0.0;
	R.m[2][0] = t*n.x*n.z + s*n.y;
	R.m[2][1] = t*n.y*n.z - s*n.x;
	R.m[2][2] = t*n.z*n.z + c;
	R.m[2][3] = 0.0;
	R.m[3][0] = 0.0;
	R.m[3][1] = 0.0;
	R.m[3][2] = 0.0;
	R.m[3][3] = 1.0;
	SG_MatrixMultVectorv(v, &R);
}

void
SG_VectorRotate4v(SG_Vector4 *v, SG_Real theta, SG_Vector4 n)
{
	SG_Real s = Sin(theta);
	SG_Real c = Cos(theta);
	SG_Real t = 1.0 - c;
	SG_Matrix R;

	R.m[0][0] = 1.0 - (n.x*n.x)*t;
	R.m[0][1] = -t*n.x*n.y;
	R.m[0][2] = -t*n.x*n.z;
	R.m[0][3] =  s*n.x;
	R.m[1][0] = -t*n.x*n.y;
	R.m[1][1] = 1.0 - (n.y*n.y)*t;
	R.m[1][2] = -t*n.y*n.z;
	R.m[1][3] =  s*n.y;
	R.m[2][0] = -t*n.x*n.z;
	R.m[2][1] = -t*n.y*n.z;
	R.m[2][2] = 1.0 - (n.z*n.z)*t;
	R.m[2][3] =  s*n.z;
	R.m[3][0] = -s*n.x;
	R.m[3][1] = -s*n.y;
	R.m[3][2] = -s*n.z;
	R.m[3][3] = c;
	SG_MatrixMultVector4v(v, &R);
}

SG_Vector
SG_VectorRotateI(SG_Vector a, SG_Real theta)
{
	SG_Vector b;

	b.x = a.x;
	b.y = (a.y * Cos(theta)) + (a.z * -Sin(theta));
	b.z = (a.y * Sin(theta)) + (a.z *  Cos(theta));
	return (b);
}

SG_Vector
SG_VectorRotateJ(SG_Vector a, SG_Real theta)
{
	SG_Vector b;

	b.x = (a.x *  Cos(theta)) + (a.z * Sin(theta));
	b.y = a.y;
	b.z = (a.x * -Sin(theta)) + (a.z * Cos(theta));
	return (b);
}

SG_Vector
SG_VectorRotateK(SG_Vector a, SG_Real theta)
{
	SG_Vector b;

	b.x = (a.x * Cos(theta)) + (a.y * -Sin(theta));
	b.y = (a.x * Sin(theta)) + (a.y *  Cos(theta));
	b.z = a.z;
	return (b);
}

SG_Vector
SG_VectorRotateQuat(SG_Vector V, SG_Quat Q)
{
	SG_Matrix R;

	SG_QuatToMatrix(&R, &Q);
	return (SG_MatrixMultVectorp(&R, &V));
}

SG_Vector2
SG_VectorLERP2(SG_Vector2 v1, SG_Vector2 v2, SG_Real t)
{
	SG_Vector2 v;

	v.x = v1.x + (v2.x - v1.x)*t;
	v.y = v1.y + (v2.y - v1.y)*t;
	return (v);
}

SG_Vector
SG_VectorLERP(SG_Vector v1, SG_Vector v2, SG_Real t)
{
	SG_Vector v;

	v.x = v1.x + (v2.x - v1.x)*t;
	v.y = v1.y + (v2.y - v1.y)*t;
	v.z = v1.z + (v2.z - v1.z)*t;
	return (v);
}

SG_Vector4
SG_VectorLERP4(SG_Vector4 v1, SG_Vector4 v2, SG_Real t)
{
	SG_Vector4 v;

	v.x = v1.x + (v2.x - v1.x)*t;
	v.y = v1.y + (v2.y - v1.y)*t;
	v.z = v1.z + (v2.z - v1.z)*t;
	v.w = v1.w + (v2.w - v1.w)*t;
	return (v);
}

SG_Vector2
SG_VectorLERP2p(SG_Vector2 *v1, SG_Vector2 *v2, SG_Real t)
{
	SG_Vector2 v;

	v.x = v1->x + (v2->x - v1->x)*t;
	v.y = v1->y + (v2->y - v1->y)*t;
	return (v);
}

SG_Vector
SG_VectorLERPp(SG_Vector *v1, SG_Vector *v2, SG_Real t)
{
	SG_Vector v;

	v.x = v1->x + (v2->x - v1->x)*t;
	v.y = v1->y + (v2->y - v1->y)*t;
	v.z = v1->z + (v2->z - v1->z)*t;
	return (v);
}

SG_Vector4
SG_VectorLERP4p(SG_Vector4 *v1, SG_Vector4 *v2, SG_Real t)
{
	SG_Vector4 v;

	v.x = v1->x + (v2->x - v1->x)*t;
	v.y = v1->y + (v2->y - v1->y)*t;
	v.z = v1->z + (v2->z - v1->z)*t;
	v.w = v1->w + (v2->w - v1->w)*t;
	return (v);
}

SG_Vector
SG_VectorElemPow(SG_Vector v, SG_Real pow)
{
	SG_Vector r;

	r.x = Pow(v.x, pow);
	r.y = Pow(v.y, pow);
	r.z = Pow(v.z, pow);
	return (r);
}

SG_Vector2
SG_ReadVector2(AG_Netbuf *buf)
{
	SG_Vector2 v;

	v.x = (SG_Real)AG_ReadDouble(buf);
	v.y = (SG_Real)AG_ReadDouble(buf);
	return (v);
}

SG_Vector
SG_ReadVector(AG_Netbuf *buf)
{
	SG_Vector v;

	v.x = (SG_Real)AG_ReadDouble(buf);
	v.y = (SG_Real)AG_ReadDouble(buf);
	v.z = (SG_Real)AG_ReadDouble(buf);
	return (v);
}

SG_Vector4
SG_ReadVector4(AG_Netbuf *buf)
{
	SG_Vector4 v;

	v.x = (SG_Real)AG_ReadDouble(buf);
	v.y = (SG_Real)AG_ReadDouble(buf);
	v.z = (SG_Real)AG_ReadDouble(buf);
	v.w = (SG_Real)AG_ReadDouble(buf);
	return (v);
}

void
SG_ReadVector2v(AG_Netbuf *buf, SG_Vector2 *v)
{
	v->x = (SG_Real)AG_ReadDouble(buf);
	v->y = (SG_Real)AG_ReadDouble(buf);
}

void
SG_ReadVectorv(AG_Netbuf *buf, SG_Vector *v)
{
	v->x = (SG_Real)AG_ReadDouble(buf);
	v->y = (SG_Real)AG_ReadDouble(buf);
	v->z = (SG_Real)AG_ReadDouble(buf);
}

void
SG_ReadVector4v(AG_Netbuf *buf, SG_Vector4 *v)
{
	v->x = (SG_Real)AG_ReadDouble(buf);
	v->y = (SG_Real)AG_ReadDouble(buf);
	v->z = (SG_Real)AG_ReadDouble(buf);
	v->w = (SG_Real)AG_ReadDouble(buf);
}

void
SG_WriteVector2(AG_Netbuf *buf, SG_Vector2 *v)
{
	AG_WriteDouble(buf, (double)v->x);
	AG_WriteDouble(buf, (double)v->y);
}

void
SG_WriteVector(AG_Netbuf *buf, SG_Vector *v)
{
	AG_WriteDouble(buf, (double)v->x);
	AG_WriteDouble(buf, (double)v->y);
	AG_WriteDouble(buf, (double)v->z);
}

void
SG_WriteVector4(AG_Netbuf *buf, SG_Vector4 *v)
{
	AG_WriteDouble(buf, (double)v->x);
	AG_WriteDouble(buf, (double)v->y);
	AG_WriteDouble(buf, (double)v->z);
	AG_WriteDouble(buf, (double)v->w);
}

SG_Vector2
SG_ReadVectorf2(AG_Netbuf *buf)
{
	SG_Vector2 v;

	v.x = (SG_Real)AG_ReadFloat(buf);
	v.y = (SG_Real)AG_ReadFloat(buf);
	return (v);
}

SG_Vector
SG_ReadVectorf(AG_Netbuf *buf)
{
	SG_Vector v;

	v.x = (SG_Real)AG_ReadFloat(buf);
	v.y = (SG_Real)AG_ReadFloat(buf);
	v.z = (SG_Real)AG_ReadFloat(buf);
	return (v);
}

SG_Vector4
SG_ReadVectorf4(AG_Netbuf *buf)
{
	SG_Vector4 v;

	v.x = (SG_Real)AG_ReadFloat(buf);
	v.y = (SG_Real)AG_ReadFloat(buf);
	v.z = (SG_Real)AG_ReadFloat(buf);
	v.w = (SG_Real)AG_ReadFloat(buf);
	return (v);
}

void
SG_ReadVectorf2v(AG_Netbuf *buf, SG_Vector2 *v)
{
	v->x = (SG_Real)AG_ReadFloat(buf);
	v->y = (SG_Real)AG_ReadFloat(buf);
}

void
SG_ReadVectorfv(AG_Netbuf *buf, SG_Vector *v)
{
	v->x = (SG_Real)AG_ReadFloat(buf);
	v->y = (SG_Real)AG_ReadFloat(buf);
	v->z = (SG_Real)AG_ReadFloat(buf);
}

void
SG_ReadVectorf4v(AG_Netbuf *buf, SG_Vector4 *v)
{
	v->x = (SG_Real)AG_ReadFloat(buf);
	v->y = (SG_Real)AG_ReadFloat(buf);
	v->z = (SG_Real)AG_ReadFloat(buf);
	v->w = (SG_Real)AG_ReadFloat(buf);
}


void
SG_WriteVectorf2(AG_Netbuf *buf, SG_Vector2 *v)
{
	AG_WriteFloat(buf, (float)v->x);
	AG_WriteFloat(buf, (float)v->y);
}

void
SG_WriteVectorf(AG_Netbuf *buf, SG_Vector *v)
{
	AG_WriteFloat(buf, (float)v->x);
	AG_WriteFloat(buf, (float)v->y);
	AG_WriteFloat(buf, (float)v->z);
}

void
SG_WriteVectorf4(AG_Netbuf *buf, SG_Vector4 *v)
{
	AG_WriteFloat(buf, (float)v->x);
	AG_WriteFloat(buf, (float)v->y);
	AG_WriteFloat(buf, (float)v->z);
	AG_WriteFloat(buf, (float)v->w);
}

#endif /* HAVE_OPENGL */
