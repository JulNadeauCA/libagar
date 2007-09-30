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
 * Operations on vectors in R^4 using standard FPU instructions.
 */

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>
#include "sg.h"

const SG_VectorOps4 sgVecOps4_FPU = {
	"fpu",
	SG_VectorZero4_FPU,
	SG_VectorGet4_FPU,
	SG_VectorSet4_FPU,
	SG_VectorCopy4_FPU,
	SG_VectorMirror4_FPU,
	SG_VectorMirror4p_FPU,
	SG_VectorLen4_FPU,
	SG_VectorLen4p_FPU,
	SG_VectorDot4_FPU,
	SG_VectorDot4p_FPU,
	SG_VectorDistance4_FPU,
	SG_VectorDistance4p_FPU,
	SG_VectorNorm4_FPU,
	SG_VectorNorm4p_FPU,
	SG_VectorNorm4v_FPU,
	SG_VectorCross4_FPU,
	SG_VectorCross4p_FPU,
	SG_VectorNormCross4_FPU,
	SG_VectorNormCross4p_FPU,
	SG_VectorScale4_FPU,
	SG_VectorScale4p_FPU,
	SG_VectorScale4v_FPU,
	SG_VectorAdd4_FPU,
	SG_VectorAdd4p_FPU,
	SG_VectorAdd4v_FPU,
	SG_VectorAdd4n_FPU,
	SG_VectorSub4_FPU,
	SG_VectorSub4p_FPU,
	SG_VectorSub4v_FPU,
	SG_VectorSub4n_FPU,
	SG_VectorAvg4_FPU,
	SG_VectorAvg4p_FPU,
	SG_VectorLERP4_FPU,
	SG_VectorLERP4p_FPU,
	SG_VectorElemPow4_FPU,
	SG_VectorVecAngle4_FPU,
	SG_VectorRotate4_FPU,
	SG_VectorRotate4v_FPU
};

SG_Vector4
SG_VectorZero4_FPU(void)
{
	SG_Vector4 v;

	v.x = 0.0;
	v.y = 0.0;
	v.z = 0.0;
	v.w = 0.0;
	return (v);
}

SG_Vector4
SG_VectorGet4_FPU(SG_Real x, SG_Real y, SG_Real z, SG_Real w)
{
	SG_Vector4 v;

	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return (v);
}

void
SG_VectorSet4_FPU(SG_Vector4 *v, SG_Real x, SG_Real y, SG_Real z, SG_Real w)
{
	v->x = x;
	v->y = y;
	v->z = z;
	v->w = w;
}

void
SG_VectorCopy4_FPU(SG_Vector4 *vDst, const SG_Vector4 *vSrc)
{
	vDst->x = vSrc->x;
	vDst->y = vSrc->y;
	vDst->z = vSrc->z;
	vDst->w = vSrc->w;
}

SG_Vector4
SG_VectorMirror4_FPU(SG_Vector4 a, int x, int y, int z, int w)
{
	SG_Vector4 b;

	b.x = x ? -a.x : a.x;
	b.y = y ? -a.y : a.y;
	b.z = z ? -a.z : a.z;
	b.w = w ? -a.w : a.w;
	return (b);
}

SG_Vector4
SG_VectorMirror4p_FPU(const SG_Vector4 *a, int x, int y, int z, int w)
{
	SG_Vector4 b;

	b.x = x ? -(a->x) : a->x;
	b.y = y ? -(a->y) : a->y;
	b.z = z ? -(a->z) : a->z;
	b.w = w ? -(a->w) : a->w;
	return (b);
}

SG_Real
SG_VectorLen4_FPU(SG_Vector4 v)
{
	return Sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);
}

SG_Real
SG_VectorLen4p_FPU(const SG_Vector4 *v)
{
	return Sqrt(v->x*v->x + v->y*v->y + v->z*v->z + v->w*v->w);
}

SG_Real
SG_VectorDot4_FPU(SG_Vector4 v1, SG_Vector4 v2)
{
	return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w);
}

SG_Real
SG_VectorDot4p_FPU(const SG_Vector4 *v1, const SG_Vector4 *v2)
{
	return (v1->x*v2->x + v1->y*v2->y + v1->z*v2->z + v1->w*v2->w);
}

SG_Real
SG_VectorDistance4_FPU(SG_Vector4 a, SG_Vector4 b)
{
	return SG_VectorLen4_FPU( SG_VectorSub4_FPU(a,b) );
}

SG_Real
SG_VectorDistance4p_FPU(const SG_Vector4 *a, const SG_Vector4 *b)
{
	return SG_VectorLen4_FPU( SG_VectorAdd4_FPU(*b,
	                          SG_VectorMirror4p_FPU(a,1,1,1,1)) );
}

SG_Vector4
SG_VectorNorm4_FPU(SG_Vector4 v)
{
	SG_Vector4 n;
	SG_Real len;

	if ((len = SG_VectorLen4p_FPU(&v)) == 0.0) {
		return (v);
	}
	n.x = v.x/len;
	n.y = v.y/len;
	n.z = v.z/len;
	n.w = v.w/len;
	return (n);
}

SG_Vector4
SG_VectorNorm4p_FPU(const SG_Vector4 *v)
{
	SG_Vector4 n;
	SG_Real len;

	if ((len = SG_VectorLen4p_FPU(v)) == 0.0) {
		return (*v);
	}
	n.x = v->x/len;
	n.y = v->y/len;
	n.z = v->z/len;
	n.w = v->w/len;
	return (n);
}

void
SG_VectorNorm4v_FPU(SG_Vector4 *v)
{
	SG_Real len;

	if ((len = SG_VectorLen4p_FPU(v)) == 0.0) {
		return;
	}
	v->x /= len;
	v->y /= len;
	v->z /= len;
	v->w /= len;
}

SG_Vector4
SG_VectorCross4_FPU(SG_Vector4 a, SG_Vector4 b, SG_Vector4 c)
{
	return (SG_VectorCross4p_FPU(&a, &b, &c));
}

SG_Vector4
SG_VectorCross4p_FPU(const SG_Vector4 *u, const SG_Vector4 *v,
    const SG_Vector4 *w)
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

SG_Vector4
SG_VectorNormCross4_FPU(SG_Vector4 u, SG_Vector4 v, SG_Vector4 w)
{
	SG_Vector4 c;

	c = SG_VectorCross4p_FPU(&u, &v, &w);
	SG_VectorNorm4v_FPU(&c);
	return (c);
}

SG_Vector4
SG_VectorNormCross4p_FPU(const SG_Vector4 *u, const SG_Vector4 *v,
    const SG_Vector4 *w)
{
	SG_Vector4 c;

	c = SG_VectorCross4p_FPU(u, v, w);
	SG_VectorNorm4v_FPU(&c);
	return (c);
}

SG_Vector4
SG_VectorScale4_FPU(SG_Vector4 a, SG_Real c)
{
	SG_Vector4 b;

	b.x = a.x*c;
	b.y = a.y*c;
	b.z = a.z*c;
	b.w = a.w*c;
	return (b);
}

SG_Vector4
SG_VectorScale4p_FPU(const SG_Vector4 *a, SG_Real c)
{
	SG_Vector4 b;

	b.x = a->x*c;
	b.y = a->y*c;
	b.z = a->z*c;
	b.w = a->w*c;
	return (b);
}

void
SG_VectorScale4v_FPU(SG_Vector4 *a, SG_Real c)
{
	a->x *= c;
	a->y *= c;
	a->z *= c;
	a->w *= c;
}

SG_Vector4
SG_VectorAdd4_FPU(SG_Vector4 a, SG_Vector4 b)
{
	SG_Vector4 c;

	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;
	c.w = a.w + b.w;
	return (c);
}

SG_Vector4
SG_VectorAdd4p_FPU(const SG_Vector4 *a, const SG_Vector4 *b)
{
	SG_Vector4 c;

	c.x = a->x + b->x;
	c.y = a->y + b->y;
	c.z = a->z + b->z;
	c.w = a->w + b->w;
	return (c);
}

void
SG_VectorAdd4v_FPU(SG_Vector4 *r, const SG_Vector4 *a)
{
	r->x += a->x;
	r->y += a->y;
	r->z += a->z;
	r->w += a->w;
}

SG_Vector4
SG_VectorAdd4n_FPU(int nvecs, ...)
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

SG_Vector4
SG_VectorSub4_FPU(SG_Vector4 a, SG_Vector4 b)
{
	SG_Vector4 c;

	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
	c.w = a.w - b.w;
	return (c);
}

SG_Vector4
SG_VectorSub4p_FPU(const SG_Vector4 *a, const SG_Vector4 *b)
{
	SG_Vector4 c;

	c.x = a->x - b->x;
	c.y = a->y - b->y;
	c.z = a->z - b->z;
	c.w = a->w - b->w;
	return (c);
}

void
SG_VectorSub4v_FPU(SG_Vector4 *r, const SG_Vector4 *a)
{
	r->x -= a->x;
	r->y -= a->y;
	r->z -= a->z;
	r->w -= a->w;
}

SG_Vector4
SG_VectorSub4n_FPU(int nvecs, ...)
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

SG_Vector4
SG_VectorAvg4_FPU(SG_Vector4 a, SG_Vector4 b)
{
	SG_Vector4 c;
	
	c.x = (a.x + b.x)/2.0;
	c.y = (a.y + b.y)/2.0;
	c.z = (a.z + b.z)/2.0;
	c.w = (a.w + b.w)/2.0;
	return (c);
}

SG_Vector4
SG_VectorAvg4p_FPU(const SG_Vector4 *a, const SG_Vector4 *b)
{
	SG_Vector4 c;
	
	c.x = (a->x + b->x)/2.0;
	c.y = (a->y + b->y)/2.0;
	c.z = (a->z + b->z)/2.0;
	c.w = (a->w + b->w)/2.0;
	return (c);
}

void
SG_VectorVecAngle4_FPU(SG_Vector4 vOrig, SG_Vector4 vOther, SG_Real *phi1,
    SG_Real *phi2, SG_Real *phi3)
{
	SG_Spherical sph;
	SG_Vector4 vd;

	vd = SG_VectorSub4p_FPU(&vOther, &vOrig);
	if (phi1 != NULL)
		*phi1 = Atan2(Sqrt(vd.w*vd.w + vd.z*vd.z + vd.y*vd.y),	vd.x);
	if (phi2 != NULL)
		*phi2 = Atan2(Sqrt(vd.w*vd.w + vd.z*vd.z),		vd.y);
	if (phi3 != NULL)
		*phi3 = Atan2(vd.w,					vd.z);
}

SG_Vector4
SG_VectorRotate4_FPU(SG_Vector4 v, SG_Real theta, SG_Vector4 n)
{
	SG_Vector4 r = v;

	SG_VectorRotate4v_FPU(&r, theta, n);
	return (r);
}

void
SG_VectorRotate4v_FPU(SG_Vector4 *v, SG_Real theta, SG_Vector4 n)
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
	MatMultVector4v(v, &R);
}

SG_Vector4
SG_VectorLERP4_FPU(SG_Vector4 v1, SG_Vector4 v2, SG_Real t)
{
	SG_Vector4 v;

	v.x = v1.x + (v2.x - v1.x)*t;
	v.y = v1.y + (v2.y - v1.y)*t;
	v.z = v1.z + (v2.z - v1.z)*t;
	v.w = v1.w + (v2.w - v1.w)*t;
	return (v);
}

SG_Vector4
SG_VectorLERP4p_FPU(SG_Vector4 *v1, SG_Vector4 *v2, SG_Real t)
{
	SG_Vector4 v;

	v.x = v1->x + (v2->x - v1->x)*t;
	v.y = v1->y + (v2->y - v1->y)*t;
	v.z = v1->z + (v2->z - v1->z)*t;
	v.w = v1->w + (v2->w - v1->w)*t;
	return (v);
}

SG_Vector4
SG_VectorElemPow4_FPU(SG_Vector4 v, SG_Real pow)
{
	SG_Vector4 r;

	r.x = Pow(v.x, pow);
	r.y = Pow(v.y, pow);
	r.z = Pow(v.z, pow);
	r.w = Pow(v.w, pow);
	return (r);
}

#endif /* HAVE_OPENGL */
