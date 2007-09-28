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
 * Operations on vectors in R^2 using standard FPU instructions.
 */

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>
#include "sg.h"

const SG_VectorOps2 sgVecOps2_FPU = {
	"fpu",
	SG_VectorZero2_FPU,
	SG_VectorGet2_FPU,
	SG_VectorSet2_FPU,
	SG_VectorCopy2_FPU,
	SG_VectorMirror2_FPU,
	SG_VectorMirror2p_FPU,
	SG_VectorLen2_FPU,
	SG_VectorLen2p_FPU,
	SG_VectorDot2_FPU,
	SG_VectorDot2p_FPU,
	SG_VectorPerpDot2_FPU,
	SG_VectorPerpDot2p_FPU,
	SG_VectorDistance2_FPU,
	SG_VectorDistance2p_FPU,
	SG_VectorNorm2_FPU,
	SG_VectorNorm2p_FPU,
	SG_VectorNorm2v_FPU,
	SG_VectorScale2_FPU,
	SG_VectorScale2p_FPU,
	SG_VectorScale2v_FPU,
	SG_VectorAdd2_FPU,
	SG_VectorAdd2p_FPU,
	SG_VectorAdd2v_FPU,
	SG_VectorAdd2n_FPU,
	SG_VectorSub2_FPU,
	SG_VectorSub2p_FPU,
	SG_VectorSub2v_FPU,
	SG_VectorSub2n_FPU,
	SG_VectorAvg2_FPU,
	SG_VectorAvg2p_FPU,
	SG_VectorLERP2_FPU,
	SG_VectorLERP2p_FPU,
	SG_VectorElemPow2_FPU,
	SG_VectorVecAngle2_FPU,
	SG_VectorRotate2_FPU,
	SG_VectorRotate2v_FPU
};

SG_Vector2
SG_VectorZero2_FPU(void)
{
	SG_Vector2 v;

	v.x = 0.0;
	v.y = 0.0;
	return (v);
}

SG_Vector2
SG_VectorGet2_FPU(SG_Real x, SG_Real y)
{
	SG_Vector2 v;

	v.x = x;
	v.y = y;
	return (v);
}

void
SG_VectorSet2_FPU(SG_Vector2 *v, SG_Real x, SG_Real y)
{
	v->x = x;
	v->y = y;
}

void
SG_VectorCopy2_FPU(SG_Vector2 *vDst, const SG_Vector2 *vSrc)
{
	vDst->x = vSrc->x;
	vDst->y = vSrc->y;
}

SG_Vector2
SG_VectorMirror2_FPU(SG_Vector2 a, int x, int y)
{
	SG_Vector2 b;

	b.x = x ? -a.x : a.x;
	b.y = y ? -a.y : a.y;
	return (b);
}

SG_Vector2
SG_VectorMirror2p_FPU(const SG_Vector2 *a, int x, int y)
{
	SG_Vector2 b;

	b.x = x ? -(a->x) : a->x;
	b.y = y ? -(a->y) : a->y;
	return (b);
}

SG_Real
SG_VectorLen2_FPU(SG_Vector2 v)
{
	return Hypot2(v.x, v.y);
}

SG_Real
SG_VectorLen2p_FPU(const SG_Vector2 *v)
{
	return Hypot2(v->x, v->y);
}

SG_Real
SG_VectorDot2_FPU(SG_Vector2 v1, SG_Vector2 v2)
{
	return (v1.x*v2.x + v1.y*v2.y);
}

SG_Real
SG_VectorDot2p_FPU(const SG_Vector2 *v1, const SG_Vector2 *v2)
{
	return (v1->x*v2->x + v1->y*v2->y);
}

SG_Real
SG_VectorPerpDot2_FPU(SG_Vector2 v1, SG_Vector2 v2)
{
	return (v1.x*v2.y - v1.y*v2.x);
}

SG_Real
SG_VectorPerpDot2p_FPU(const SG_Vector2 *v1, const SG_Vector2 *v2)
{
	return (v1->x*v2->y - v1->y*v2->x);
}

SG_Real
SG_VectorDistance2_FPU(SG_Vector2 a, SG_Vector2 b)
{
	return SG_VectorLen2_FPU( SG_VectorSub2_FPU(a,b) );
}

SG_Real
SG_VectorDistance2p_FPU(const SG_Vector2 *a, const SG_Vector2 *b)
{
	return SG_VectorLen2_FPU( SG_VectorAdd2_FPU(*b,
	                          SG_VectorMirror2p_FPU(a,1,1)) );
}

SG_Vector2
SG_VectorNorm2_FPU(SG_Vector2 v)
{
	SG_Vector2 n;
	SG_Real len;

	if ((len = SG_VectorLen2p_FPU(&v)) == 0.0) {
		return (v);
	}
	n.x = v.x/len;
	n.y = v.y/len;
	return (n);
}

SG_Vector2
SG_VectorNorm2p_FPU(const SG_Vector2 *v)
{
	SG_Vector2 n;
	SG_Real len;

	if ((len = SG_VectorLen2p_FPU(v)) == 0.0) {
		return (*v);
	}
	n.x = v->x/len;
	n.y = v->y/len;
	return (n);
}

void
SG_VectorNorm2v_FPU(SG_Vector2 *v)
{
	SG_Real len;

	if ((len = SG_VectorLen2p_FPU(v)) == 0.0) {
		return;
	}
	v->x /= len;
	v->y /= len;
}

SG_Vector2
SG_VectorScale2_FPU(SG_Vector2 a, SG_Real c)
{
	SG_Vector2 b;

	b.x = a.x*c;
	b.y = a.y*c;
	return (b);
}

SG_Vector2
SG_VectorScale2p_FPU(const SG_Vector2 *a, SG_Real c)
{
	SG_Vector2 b;

	b.x = a->x*c;
	b.y = a->y*c;
	return (b);
}

void
SG_VectorScale2v_FPU(SG_Vector2 *a, SG_Real c)
{
	a->x *= c;
	a->y *= c;
}

SG_Vector2
SG_VectorAdd2_FPU(SG_Vector2 a, SG_Vector2 b)
{
	SG_Vector2 c;

	c.x = a.x + b.x;
	c.y = a.y + b.y;
	return (c);
}

SG_Vector2
SG_VectorAdd2p_FPU(const SG_Vector2 *a, const SG_Vector2 *b)
{
	SG_Vector2 c;

	c.x = a->x + b->x;
	c.y = a->y + b->y;
	return (c);
}

void
SG_VectorAdd2v_FPU(SG_Vector2 *r, const SG_Vector2 *a)
{
	r->x += a->x;
	r->y += a->y;
}

SG_Vector2
SG_VectorAdd2n_FPU(int nvecs, ...)
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

SG_Vector2
SG_VectorSub2_FPU(SG_Vector2 a, SG_Vector2 b)
{
	SG_Vector2 c;

	c.x = a.x - b.x;
	c.y = a.y - b.y;
	return (c);
}

SG_Vector2
SG_VectorSub2p_FPU(const SG_Vector2 *a, const SG_Vector2 *b)
{
	SG_Vector2 c;

	c.x = a->x - b->x;
	c.y = a->y - b->y;
	return (c);
}

void
SG_VectorSub2v_FPU(SG_Vector2 *r, const SG_Vector2 *a)
{
	r->x -= a->x;
	r->y -= a->y;
}

SG_Vector2
SG_VectorSub2n_FPU(int nvecs, ...)
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

SG_Vector2
SG_VectorAvg2_FPU(SG_Vector2 a, SG_Vector2 b)
{
	SG_Vector2 c;
	
	c.x = (a.x + b.x)/2.0;
	c.y = (a.y + b.y)/2.0;
	return (c);
}

SG_Vector2
SG_VectorAvg2p_FPU(const SG_Vector2 *a, const SG_Vector2 *b)
{
	SG_Vector2 c;
	
	c.x = (a->x + b->x)/2.0;
	c.y = (a->y + b->y)/2.0;
	return (c);
}

SG_Vector2
SG_VectorLERP2_FPU(SG_Vector2 v1, SG_Vector2 v2, SG_Real t)
{
	SG_Vector2 v;

	v.x = v1.x + (v2.x - v1.x)*t;
	v.y = v1.y + (v2.y - v1.y)*t;
	return (v);
}

SG_Vector2
SG_VectorLERP2p_FPU(SG_Vector2 *v1, SG_Vector2 *v2, SG_Real t)
{
	SG_Vector2 v;

	v.x = v1->x + (v2->x - v1->x)*t;
	v.y = v1->y + (v2->y - v1->y)*t;
	return (v);
}

SG_Vector2
SG_VectorElemPow2_FPU(SG_Vector2 v, SG_Real pow)
{
	SG_Vector2 r;

	r.x = Pow(v.x, pow);
	r.y = Pow(v.y, pow);
	return (r);
}

SG_Real
SG_VectorVecAngle2_FPU(SG_Vector2 vOrig, SG_Vector2 vOther)
{
	SG_Vector2 vd;
	
	vd = SG_VectorSub2p_FPU(&vOther, &vOrig);
	return (Atan2(vd.y, vd.x));
}

SG_Vector2
SG_VectorRotate2_FPU(SG_Vector2 v, SG_Real theta)
{
	SG_Vector2 r;
	SG_Real s = Sin(theta);
	SG_Real c = Cos(theta);
	
	r.x = c*v.x - s*v.y;
	r.y = s*v.x + c*v.y;
	return (r);
}

void
SG_VectorRotate2v_FPU(SG_Vector2 *v, SG_Real theta)
{
	SG_Vector2 r;
	SG_Real s = Sin(theta);
	SG_Real c = Cos(theta);

	r.x = c*v->x - s*v->y;
	r.y = s*v->x + c*v->y;
	SG_VectorCopy2_FPU(v, &r);
}

#endif /* HAVE_OPENGL */
