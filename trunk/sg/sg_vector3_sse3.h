/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#ifdef HAVE_SSE3

static __inline__ SG_Vector
SG_VectorZero3_SSE3(void)
{
	SG_Vector v;
	v.m128 = _mm_setzero_ps();
	return (v);
}

static __inline__ SG_Vector
SG_VectorGet3_SSE3(SG_Real x, SG_Real y, SG_Real z)
{
	SG_Vector v;

	v.m128 = _mm_set_ps(0.0, z, y, x);
	return (v);
}

#if 0
static __inline__ void
SG_VectorSet3_SSE3(SG_Vector *v, SG_Real x, SG_Real y, SG_Real z)
{
	v->m128 = _mm_set_ps(0.0, z, y, x);
}

static __inline__ void
SG_VectorCopy3_SSE3(SG_Vector *vDst, const SG_Vector *vSrc)
{
	vDst->m128 = vSrc->m128;
}
#endif

/* TODO */
static __inline__ SG_Vector
SG_VectorMirror3_SSE3(SG_Vector a, int x, int y, int z)
{
	SG_Vector b;

	b.x = x ? -a.x : a.x;
	b.y = y ? -a.y : a.y;
	b.z = z ? -a.z : a.z;
	return (b);
}

/* TODO */
static __inline__ SG_Vector
SG_VectorMirror3p_SSE3(const SG_Vector *a, int x, int y, int z)
{
	SG_Vector b;

	b.x = x ? -(a->x) : a->x;
	b.y = y ? -(a->y) : a->y;
	b.z = z ? -(a->z) : a->z;
	return (b);
}

#if 0
static __inline__ SG_Real
SG_VectorLen3_SSE3(SG_Vector v)
{
	__m128 a, b, r;
	SG_Real len;
	
	a = _mm_mul_ps(v.m128, v.m128);
	b = _mm_add_ss(
	    _mm_shuffle_ps(a,a,_MM_SHUFFLE(0,0,0,0)),
	    _mm_add_ss(_mm_shuffle_ps(a,a,_MM_SHUFFLE(1,1,1,1)),
		       _mm_shuffle_ps(a,a,_MM_SHUFFLE(2,2,2,2))));
	_mm_store_ss(&len, _mm_sqrt_ss(b));
	return (len);
}

static __inline__ SG_Real
SG_VectorLen3p_SSE3(const SG_Vector *v)
{
	__m128 a, b, r;
	SG_Real len;

	a = _mm_mul_ps(v->m128, v->m128);
	b = _mm_add_ss(
	    _mm_shuffle_ps(a,a,_MM_SHUFFLE(0,0,0,0)),
	    _mm_add_ss(_mm_shuffle_ps(a,a,_MM_SHUFFLE(1,1,1,1)),
		       _mm_shuffle_ps(a,a,_MM_SHUFFLE(2,2,2,2))));
	_mm_store_ss(&len, _mm_sqrt_ss(b));
	return (len);
}
#endif

static __inline__ SG_Real
SG_VectorDot3_SSE3(SG_Vector v1, SG_Vector v2)
{
	SG_Real rv;
	__m128 a;

	a = _mm_mul_ps(v1.m128, v2.m128);
	a = _mm_hadd_ps(a, a);
	a = _mm_hadd_ps(a, a);
	_mm_store_ss(&rv, a);
	return (rv);
}

static __inline__ SG_Real
SG_VectorDot3p_SSE3(const SG_Vector *v1, const SG_Vector *v2)
{
	SG_Real rv;
	__m128 a;

	a = _mm_mul_ps(v1->m128, v2->m128);
	a = _mm_hadd_ps(v1->m128, v1->m128);
	a = _mm_hadd_ps(v1->m128, v1->m128);
	_mm_store_ss(&rv, v1->m128);
	return (rv);
}

static __inline__ SG_Vector
SG_VectorNorm3_SSE3(SG_Vector v)
{
	__m128 a, b, rsqrt, lenr;
	SG_Vector n;

	a = _mm_mul_ps(v.m128, v.m128);
	b = _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0,0,0,0)),
	               _mm_add_ss(_mm_shuffle_ps(a,a,_MM_SHUFFLE(1,1,1,1)),
	               _mm_shuffle_ps(a,a,_MM_SHUFFLE(2,2,2,2))));
	rsqrt	= _mm_rsqrt_ss(b);
	lenr	= _mm_shuffle_ps(rsqrt, rsqrt, _MM_SHUFFLE(0,0,0,0));
	n.m128	= _mm_mul_ps(n.m128, lenr);
	return (n);
}

static __inline__ SG_Vector
SG_VectorNorm3p_SSE3(const SG_Vector *v)
{
	__m128 a, b, rsqrt, lenr;
	SG_Vector n;

	a = _mm_mul_ps(v->m128, v->m128);
	b = _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0,0,0,0)),
	               _mm_add_ss(_mm_shuffle_ps(a,a,_MM_SHUFFLE(1,1,1,1)),
	               _mm_shuffle_ps(a,a,_MM_SHUFFLE(2,2,2,2))));
	rsqrt	= _mm_rsqrt_ss(b);
	lenr	= _mm_shuffle_ps(rsqrt, rsqrt, _MM_SHUFFLE(0,0,0,0));
	n.m128	= _mm_mul_ps(n.m128, lenr);
	return (n);
}

#if 0
static __inline__ void
SG_VectorNorm3v_SSE3(SG_Vector *v)
{
	__m128 a, b, rsqrt, lenr;
	SG_Vector n;

	a = _mm_mul_ps(v->m128, v->m128);
	b = _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0,0,0,0)),
	               _mm_add_ss(_mm_shuffle_ps(a,a,_MM_SHUFFLE(1,1,1,1)),
	               _mm_shuffle_ps(a,a,_MM_SHUFFLE(2,2,2,2))));
	rsqrt	= _mm_rsqrt_ss(b);
	lenr	= _mm_shuffle_ps(rsqrt, rsqrt, _MM_SHUFFLE(0,0,0,0));
	v->m128	= _mm_mul_ps(n.m128, lenr);
}
#endif

static __inline__ SG_Vector
SG_VectorScale3_SSE3(SG_Vector a, SG_Real c)
{
	SG_Vector b;

	b.m128 = _mm_mul_ps(a.m128, _mm_set1_ps(c));
	return (b);
}

static __inline__ SG_Vector
SG_VectorScale3p_SSE3(const SG_Vector *a, SG_Real c)
{
	SG_Vector b;

	b.m128 = _mm_mul_ps(a->m128, _mm_set1_ps(c));
	return (b);
}

#if 0
static __inline__ void
SG_VectorScale3v_SSE3(SG_Vector *a, SG_Real c)
{
	a->m128 = _mm_mul_ps(a->m128, _mm_set1_ps(c));
}
#endif

static __inline__ SG_Vector
SG_VectorAdd3_SSE3(SG_Vector a, SG_Vector b)
{
	SG_Vector c;

	c.m128 = _mm_add_ps(a.m128, b.m128);
	return (c);
}

static __inline__ SG_Vector
SG_VectorAdd3p_SSE3(const SG_Vector *a, const SG_Vector *b)
{
	SG_Vector c;

	c.m128 = _mm_add_ps(a->m128, b->m128);
	return (c);
}

#if 0
static __inline__ void
SG_VectorAdd3v_SSE3(SG_Vector *r, const SG_Vector *a)
{
	r->m128 = _mm_add_ps(r->m128, a->m128);
}
#endif

static __inline__ SG_Vector
SG_VectorSub3_SSE3(SG_Vector a, SG_Vector b)
{
	SG_Vector c;

	c.m128 = _mm_sub_ps(a.m128, b.m128);
	return (c);
}

static __inline__ SG_Vector
SG_VectorSub3p_SSE3(const SG_Vector *a, const SG_Vector *b)
{
	SG_Vector c;

	c.m128 = _mm_sub_ps(a->m128, b->m128);
	return (c);
}

#if 0
static __inline__ void
SG_VectorSub3v_SSE3(SG_Vector *r, const SG_Vector *a)
{
	r->m128 = _mm_sub_ps(r->m128, a->m128);
}
#endif

/* TODO */
static __inline__ SG_Real
SG_VectorDistance3_SSE3(SG_Vector a, SG_Vector b)
{
	return SG_VectorLen3_FPU(SG_VectorSub3_SSE3(a,b));
}

/* TODO */
static __inline__ SG_Real
SG_VectorDistance3p_SSE3(const SG_Vector *a, const SG_Vector *b)
{
	return SG_VectorLen3_FPU(SG_VectorAdd3_SSE3(*b,
	                         SG_VectorMirror3p_SSE3(a,1,1,1)));
}

/* TODO */
static __inline__ SG_Vector
SG_VectorAvg3_SSE3(SG_Vector a, SG_Vector b)
{
	SG_Vector c;
	
	c.x = (a.x + b.x)/2.0;
	c.y = (a.y + b.y)/2.0;
	c.z = (a.z + b.z)/2.0;
	return (c);
}

/* TODO */
static __inline__ SG_Vector
SG_VectorAvg3p_SSE3(const SG_Vector *a, const SG_Vector *b)
{
	SG_Vector c;
	
	c.x = (a->x + b->x)/2.0;
	c.y = (a->y + b->y)/2.0;
	c.z = (a->z + b->z)/2.0;
	return (c);
}

/* TODO */
static __inline__ void
SG_VectorVecAngle3_SSE3(SG_Vector vOrig, SG_Vector vOther, SG_Real *theta,
    SG_Real *phi)
{
	SG_Vector vd;

	vd = SG_VectorSub3p_SSE3(&vOther, &vOrig);
	if (theta != NULL) {
		*theta = SG_Atan2(vd.y, vd.x);
	}
	if (phi != NULL) {
		*phi = SG_Atan2(SG_Sqrt(vd.x*vd.x + vd.y*vd.y), vd.z);
	}
}

/* TODO */
static __inline__ SG_Vector
SG_VectorRotateI3_SSE3(SG_Vector a, SG_Real theta)
{
	SG_Vector b;

	b.x = a.x;
	b.y = (a.y * SG_Cos(theta)) + (a.z * -SG_Sin(theta));
	b.z = (a.y * SG_Sin(theta)) + (a.z *  SG_Cos(theta));
	return (b);
}

/* TODO */
static __inline__ SG_Vector
SG_VectorRotateJ3_SSE3(SG_Vector a, SG_Real theta)
{
	SG_Vector b;

	b.x = (a.x *  SG_Cos(theta)) + (a.z * SG_Sin(theta));
	b.y = a.y;
	b.z = (a.x * -SG_Sin(theta)) + (a.z * SG_Cos(theta));
	return (b);
}

/* TODO */
static __inline__ SG_Vector
SG_VectorRotateK3_SSE3(SG_Vector a, SG_Real theta)
{
	SG_Vector b;

	b.x = (a.x * SG_Cos(theta)) + (a.y * -SG_Sin(theta));
	b.y = (a.x * SG_Sin(theta)) + (a.y *  SG_Cos(theta));
	b.z = a.z;
	return (b);
}

/* TODO */
static __inline__ SG_Vector
SG_VectorLERP3_SSE3(SG_Vector v1, SG_Vector v2, SG_Real t)
{
	SG_Vector v;

	v.x = v1.x + (v2.x - v1.x)*t;
	v.y = v1.y + (v2.y - v1.y)*t;
	v.z = v1.z + (v2.z - v1.z)*t;
	return (v);
}

/* TODO */
static __inline__ SG_Vector
SG_VectorLERP3p_SSE3(SG_Vector *v1, SG_Vector *v2, SG_Real t)
{
	SG_Vector v;

	v.x = v1->x + (v2->x - v1->x)*t;
	v.y = v1->y + (v2->y - v1->y)*t;
	v.z = v1->z + (v2->z - v1->z)*t;
	return (v);
}

/* TODO */
static __inline__ SG_Vector
SG_VectorElemPow3_SSE3(SG_Vector v, SG_Real pow)
{
	SG_Vector r;

	r.x = SG_Pow(v.x, pow);
	r.y = SG_Pow(v.y, pow);
	r.z = SG_Pow(v.z, pow);
	return (r);
}

__BEGIN_DECLS
extern const SG_VectorOps3 sgVecOps3_SSE3;

SG_Vector	SG_VectorRotate3_SSE3(SG_Vector, SG_Real, SG_Vector);
void		SG_VectorRotate3v_SSE3(SG_Vector *, SG_Real, SG_Vector);
SG_Vector	SG_VectorRotateQuat3_SSE3(SG_Vector, SG_Quat);
SG_Vector	SG_VectorAdd3n_SSE3(int, ...);
SG_Vector	SG_VectorSub3n_SSE3(int, ...);
__END_DECLS

#endif /* HAVE_SSE3 */
