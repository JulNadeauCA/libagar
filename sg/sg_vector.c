/*
 * Copyright (c) 2005-2007 Hypertriton, Inc.
 * <http://www.hypertriton.com/>
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

#include <agar/config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <agar/core/core.h>
#include "sg.h"

SG_Vector
SG_GetVector(SG_Real x, SG_Real y, SG_Real z)
{
	SG_Vector v;

	v.x = x;
	v.y = y;
	v.z = z;
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

void
SG_SetVector(SG_Vector *v, SG_Real x, SG_Real y, SG_Real z)
{
	v->x = x;
	v->y = y;
	v->z = z;
}

void
SG_CopyVector(SG_Vector *vDst, const SG_Vector *vSrc)
{
	vDst->x = vSrc->x;
	vDst->y = vSrc->y;
	vDst->z = vSrc->z;
}

SG_Real
SG_VectorLen(SG_Vector v)
{
	return (SG_Sqrt(v.x*v.x + v.y*v.y + v.z*v.z));
}

SG_Real
SG_VectorLenp(const SG_Vector *v)
{
	return (SG_Sqrt(v->x*v->x + v->y*v->y + v->z*v->z));
}

SG_Real
SG_VectorDistance(SG_Vector a, SG_Vector b)
{
	return (SG_VectorLen(SG_VectorSub(a, b)));
}

SG_Real
SG_VectorDistancep(const SG_Vector *a, const SG_Vector *b)
{
	return (SG_VectorLen(SG_VectorAdd(*b, SG_VectorMirrorp(a, 1,1,1))));
}

SG_Real
SG_VectorVectorAngle(SG_Vector v1, SG_Vector v2)
{
	return (SG_Acos(SG_VectorDot(SG_VectorNormp(&v1),
	                             SG_VectorNormp(&v2))));
}

SG_Real
SG_VectorDotp(const SG_Vector *v1, const SG_Vector *v2)
{
	return (v1->x*v2->x + v1->y*v2->y + v1->z*v2->z);
}

SG_Real
SG_VectorDot(SG_Vector v1, SG_Vector v2)
{
	return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}

SG_Vector
SG_VectorNormp(const SG_Vector *v)
{
	SG_Vector rv;
	SG_Real len;

	if ((len = SG_VectorLenp(v)) == 0.0) {
		rv = *v;
		return (rv);
	}
	rv.x = v->x/len;
	rv.y = v->y/len;
	rv.z = v->z/len;
	return (rv);
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

SG_Vector
SG_VectorNorm(SG_Vector v)
{
	return (SG_VectorNormp(&v));
}

SG_Vector
SG_VectorCross(SG_Vector a, SG_Vector b)
{
	return (SG_VectorCrossp(&a, &b));
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

void
SG_VectorCrossv(SG_Vector *r, const SG_Vector *a, const SG_Vector *b)
{
	if (r == a) {
		SG_Real ax = a->x;
		SG_Real ay = a->y;

		r->x = ay*b->z - b->y*a->z;
		r->y = a->z*b->x - b->z*ax;
		r->z = ax*b->y - b->x*ay;
	} else {
		r->x = a->y*b->z - b->y*a->z;
		r->y = a->z*b->x - b->z*a->x;
		r->z = a->x*b->y - b->x*a->y;
	}
}

SG_Vector
SG_VectorNCross(SG_Vector a, SG_Vector b)
{
	SG_Vector c;

	c.x = a.y*b.z - b.y*a.z;
	c.y = a.z*b.x - b.z*a.x;
	c.z = a.x*b.y - b.x*a.y;
	SG_VectorNormp(&c);
	return (c);
}

SG_Vector
SG_VectorNCrossp(const SG_Vector *a, const SG_Vector *b)
{
	SG_Vector c;

	c.x = a->y*b->z - b->y*a->z;
	c.y = a->z*b->x - b->z*a->x;
	c.z = a->x*b->y - b->x*a->y;
	SG_VectorNormp(&c);
	return (c);
}

void
SG_VectorNCrossv(SG_Vector *r, const SG_Vector *a, const SG_Vector *b)
{
	if (r == a) {
		SG_Real ax = a->x;
		SG_Real ay = a->y;

		r->x = a->y*b->z - b->y*a->z;
		r->y = a->z*b->x - b->z*ax;
		r->z = ax*b->y - b->x*ay;
	} else {
		r->x = a->y*b->z - b->y*a->z;
		r->y = a->z*b->x - b->z*a->x;
		r->z = a->x*b->y - b->x*a->y;
	}
	SG_VectorNormp(r);
}

SG_Vector
SG_VectorScale(SG_Vector a, SG_Real c)
{
	return (SG_VectorScalep(&a, c));
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

void
SG_VectorScalev(SG_Vector *a, SG_Real c)
{
	a->x *= c;
	a->y *= c;
	a->z *= c;
}

SG_Vector
SG_VectorAdd(SG_Vector a, SG_Vector b)
{
	return (SG_VectorAddp(&a, &b));
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

void
SG_VectorAddv(SG_Vector *r, const SG_Vector *a)
{
	r->x += a->x;
	r->y += a->y;
	r->z += a->z;
}

void
SG_VectorAddv3(SG_Vector *r, SG_Real x, SG_Real y, SG_Real z)
{
	r->x += x;
	r->y += y;
	r->z += z;
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

SG_Vector
SG_VectorSub(SG_Vector a, SG_Vector b)
{
	return (SG_VectorSubp(&a, &b));
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

void
SG_VectorSubv(SG_Vector *r, const SG_Vector *a, const SG_Vector *b)
{
	r->x = a->x - b->x;
	r->y = a->y - b->y;
	r->z = a->z - b->z;
}

SG_Vector
SG_VectorSubn(int nvecs, ...)
{
	SG_Vector c, *v;
	va_list ap;
	int i;
#ifdef DEBUG
	if (nvecs < 1) { fatal("<1 vectors specified"); }
#endif
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

SG_Vector
SG_VectorAvg2(SG_Vector a, SG_Vector b)
{
	SG_Vector c;
	SG_VectorAvg2v(&c, &a, &b);
	return (c);
}

SG_Vector
SG_VectorAvg2p(const SG_Vector *a, const SG_Vector *b)
{
	SG_Vector c;
	SG_VectorAvg2v(&c, a, b);
	return (c);
}

void
SG_VectorAvg2v(SG_Vector *c, const SG_Vector *a, const SG_Vector *b)
{
	c->x = (a->x + b->x)/2.0;
	c->y = (a->y + b->y)/2.0;
	c->z = (a->z + b->z)/2.0;
}

SG_Vector
SG_VectorTranslate(SG_Vector a, SG_Real x, SG_Real y, SG_Real z)
{
	SG_Vector b;

	b.x = a.x+x;
	b.y = a.y+y;
	b.z = a.z+z;
	return (b);
}

SG_Vector
SG_VectorTranslatep(const SG_Vector *a, SG_Real x, SG_Real y, SG_Real z)
{
	SG_Vector b;

	b.x += x;
	b.y += y;
	b.z += z;
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

SG_Vector
SG_VectorMirrorp(const SG_Vector *a, int x, int y, int z)
{
	SG_Vector b;

	b.x = x ? -(a->x) : a->x;
	b.y = y ? -(a->y) : a->y;
	b.z = z ? -(a->z) : a->z;
	return (b);
}

SG_Vector
SG_VectorRotateEul(SG_Vector a, SG_Real x, SG_Real y, SG_Real z)
{
	SG_Vector b;

	b = SG_VectorRotateX(a, x);
	b = SG_VectorRotateY(b, y);
	b = SG_VectorRotateZ(b, z);
	return (b);
}

SG_Vector
SG_VectorRotateX(SG_Vector a, SG_Real theta)
{
	SG_Vector b;

	b.x = a.x;
	b.y = (a.y * SG_Cos(theta)) + (a.z * -SG_Sin(theta));
	b.z = (a.y * SG_Sin(theta)) + (a.z * SG_Cos(theta));
	return (b);
}

SG_Vector
SG_VectorRotateY(SG_Vector a, SG_Real theta)
{
	SG_Vector b;

	b.x = (a.x * SG_Cos(theta)) + (a.z * SG_Sin(theta));
	b.y = a.y;
	b.z = (a.x * -SG_Sin(theta)) + (a.z * SG_Cos(theta));
	return (b);
}

SG_Vector
SG_VectorRotateZ(SG_Vector a, SG_Real theta)
{
	SG_Vector b;

	b.x = (a.x * SG_Cos(theta)) + (a.y * -SG_Sin(theta));
	b.y = (a.x * SG_Sin(theta)) + (a.y * SG_Cos(theta));
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

SG_Vector
SG_VectorRotate(SG_Vector V, SG_Real theta, SG_Vector A)
{
	SG_Vector Vr = V;
	SG_VectorRotatev(&Vr, theta, A);
	return (Vr);
}

void
SG_VectorRotatev(SG_Vector *V, SG_Real theta, SG_Vector A)
{
	SG_Real s = SG_Sin(theta);
	SG_Real c = SG_Cos(theta);
	SG_Real t = 1.0 - c;
	SG_Matrix R;

	R.m[0][0] = t*A.x*A.x + c;
	R.m[0][1] = t*A.x*A.y + s*A.z;
	R.m[0][2] = t*A.x*A.z - s*A.y;
	R.m[0][3] = 0.0;
	R.m[1][0] = t*A.x*A.y - s*A.z;
	R.m[1][1] = t*A.y*A.y + c;
	R.m[1][2] = t*A.y*A.z + s*A.x;
	R.m[1][3] = 0.0;
	R.m[2][0] = t*A.x*A.z + s*A.y;
	R.m[2][1] = t*A.y*A.z - s*A.x;
	R.m[2][2] = t*A.z*A.z + c;
	R.m[2][3] = 0.0;
	R.m[3][0] = 0.0;
	R.m[3][1] = 0.0;
	R.m[3][2] = 0.0;
	R.m[3][3] = 1.0;
	SG_MatrixMultVectorv(V, &R);
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

void
SG_ReadVectorv(AG_Netbuf *buf, SG_Vector *v)
{
	v->x = (SG_Real)AG_ReadDouble(buf);
	v->y = (SG_Real)AG_ReadDouble(buf);
	v->z = (SG_Real)AG_ReadDouble(buf);
}

void
SG_WriteVector(AG_Netbuf *buf, SG_Vector *v)
{
	AG_WriteDouble(buf, (double)v->x);
	AG_WriteDouble(buf, (double)v->y);
	AG_WriteDouble(buf, (double)v->z);
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

void
SG_ReadVectorfv(AG_Netbuf *buf, SG_Vector *v)
{
	v->x = (SG_Real)AG_ReadFloat(buf);
	v->y = (SG_Real)AG_ReadFloat(buf);
	v->z = (SG_Real)AG_ReadFloat(buf);
}

void
SG_WriteVectorf(AG_Netbuf *buf, SG_Vector *v)
{
	AG_WriteFloat(buf, (float)v->x);
	AG_WriteFloat(buf, (float)v->y);
	AG_WriteFloat(buf, (float)v->z);
}

#endif /* HAVE_OPENGL */
