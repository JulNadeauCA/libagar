/*
 * Copyright (c) 2007-2012 Hypertriton, Inc. <http://hypertriton.com/>
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

#ifdef HAVE_SSE

__BEGIN_DECLS
static __inline__ M_Vector3
M_VectorZero3_SSE(void)
{
	M_Vector3 v;

	v.m128 = _mm_setzero_ps();
	return (v);
}

static __inline__ M_Vector3
M_VectorGet3_SSE(M_Real x, M_Real y, M_Real z)
{
	M_Vector3 v;

	v.m128 = _mm_set_ps(0.0f, z, y, x);
	return (v);
}

static __inline__ void
M_VectorSet3_SSE(M_Vector3 *_Nonnull v, M_Real x, M_Real y, M_Real z)
{
	v->m128 = _mm_set_ps(0.0f, z, y, x);
}

static __inline__ void
M_VectorCopy3_SSE(M_Vector3 *_Nonnull vDst, const M_Vector3 *_Nonnull vSrc)
{
	vDst->m128 = vSrc->m128;
}

static __inline__ M_Vector3
M_VectorFlip3_SSE(M_Vector3 a)
{
	M_Vector3 b;

	b.m128 = _mm_mul_ps(a.m128, _mm_set1_ps(-1.0f));
	return (b);
}

static __inline__ M_Real
M_VectorLen3_SSE(M_Vector3 v)
{
	__m128 r1, r2, r3;
	float len;
	
	r1 = _mm_mul_ps(v.m128, v.m128);
	r2 = _mm_add_ss(
	    _mm_shuffle_ps(r1,r1,_MM_SHUFFLE(0,0,0,0)),
	    _mm_add_ss(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(1,1,1,1)),
	               _mm_shuffle_ps(r1,r1,_MM_SHUFFLE(2,2,2,2))));
	r3 = _mm_sqrt_ss(r2);
	_mm_store_ss(&len, r3);
	return (M_Real)len;
}

static __inline__ M_Real
M_VectorLen3p_SSE(const M_Vector3 *_Nonnull v)
{
	__m128 r1, r2, r3;
	float len;
	
	r1 = _mm_mul_ps(v->m128, v->m128);
	r2 = _mm_add_ss(
	    _mm_shuffle_ps(r1,r1,_MM_SHUFFLE(0,0,0,0)),
	    _mm_add_ss(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(1,1,1,1)),
	               _mm_shuffle_ps(r1,r1,_MM_SHUFFLE(2,2,2,2))));
	r3 = _mm_sqrt_ss(r2);
	_mm_store_ss(&len, r3);
	return (M_Real)len;
}

static __inline__ M_Real
M_VectorDot3_SSE(M_Vector3 a, M_Vector3 b)
{
#ifdef HAVE_SSE3
	float dot;
	__m128 r;

	r = _mm_mul_ps(a.m128, b.m128);		/* [ aXbX; aYbY; aZbZ; 0] */
	r = _mm_hadd_ps(r, r);			/* [ aXbX+aYbY; aZbZ+0; ...] */
	r = _mm_hadd_ps(r, r);			/* [ aXbX+aYbY+aZbZ; ...] */
	_mm_store_ss(&dot, r);
	return (M_Real)dot;
#else
	return (a.x*b.x + a.y*b.y + a.z*b.z);
#endif
}
static __inline__ M_Real
M_VectorDot3p_SSE(const M_Vector3 *_Nonnull a, const M_Vector3 *_Nonnull b)
{
#ifdef HAVE_SSE3
	float dot;
	__m128 r;

	r = _mm_mul_ps(a->m128, b->m128);	/* [ aXbX; aYbY; aZbZ; 0] */
	r = _mm_hadd_ps(r, r);			/* [ aXbX+aYbY; aZbZ+0; ...] */
	r = _mm_hadd_ps(r, r);			/* [ aXbX+aYbY+aZbZ; ...] */
	_mm_store_ss(&dot, r);
	return (M_Real)dot;
#else
	return (a->x*b->x + a->y*b->y + a->z*b->z);
#endif
}

static __inline__ M_Vector3
M_VectorNorm3_SSE(M_Vector3 v)
{
	__m128 r1, r2, r3;
	M_Vector3 out;

	r1 = _mm_mul_ps(v.m128, v.m128);
	r2 = _mm_add_ss(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(0,0,0,0)),
	                _mm_add_ss(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(1,1,1,1)),
	                           _mm_shuffle_ps(r1,r1,_MM_SHUFFLE(2,2,2,2))));
	r3 = _mm_sqrt_ss(r2);
	r3 = _mm_shuffle_ps(r3,r3,_MM_SHUFFLE(0,0,0,0));
	out.m128 = _mm_div_ps(v.m128, r3);
	return (out);
}
static __inline__ M_Vector3
M_VectorNorm3p_SSE(const M_Vector3 *_Nonnull v)
{
	__m128 r1, r2, r3;
	M_Vector3 out;

	r1 = _mm_mul_ps(v->m128, v->m128);
	r2 = _mm_add_ss(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(0,0,0,0)),
	                _mm_add_ss(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(1,1,1,1)),
	                           _mm_shuffle_ps(r1,r1,_MM_SHUFFLE(2,2,2,2))));
	r3 = _mm_sqrt_ss(r2);
	r3 = _mm_shuffle_ps(r3,r3,_MM_SHUFFLE(0,0,0,0));
	out.m128 = _mm_div_ps(v->m128, r3);
	return (out);
}
static __inline__ void
M_VectorNorm3v_SSE(M_Vector3 *_Nonnull v)
{
	__m128 r1, r2, r3;
	
	r1 = _mm_mul_ps(v->m128, v->m128);
	r2 = _mm_add_ss(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(0,0,0,0)),
	                _mm_add_ss(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(1,1,1,1)),
	                           _mm_shuffle_ps(r1,r1,_MM_SHUFFLE(2,2,2,2))));
	r3 = _mm_sqrt_ss(r2);
	r3 = _mm_shuffle_ps(r3,r3,_MM_SHUFFLE(0,0,0,0));
	v->m128 = _mm_div_ps(v->m128, r3);
}

static __inline__ M_Vector3
M_VectorCross3_SSE(M_Vector3 a, M_Vector3 b)
{
	__m128 rA, rB, r1, r2;
	M_Vector3 out;

	rA = a.m128;
	rB = b.m128;
	r1 = _mm_mul_ps(_mm_shuffle_ps(rA,rA,_MM_SHUFFLE(3,0,2,1)),
	                _mm_shuffle_ps(rB,rB,_MM_SHUFFLE(3,1,0,2)));
	r2 = _mm_mul_ps(_mm_shuffle_ps(rA,rA,_MM_SHUFFLE(3,1,0,2)),
	                _mm_shuffle_ps(rB,rB,_MM_SHUFFLE(3,0,2,1)));
	out.m128 = _mm_sub_ps(r1, r2);
	return (out);
}
static __inline__ M_Vector3
M_VectorCross3p_SSE(const M_Vector3 *_Nonnull a, const M_Vector3 *_Nonnull b)
{
	__m128 rA, rB, r1, r2;
	M_Vector3 out;

	rA = a->m128;
	rB = b->m128;
	r1 = _mm_mul_ps(_mm_shuffle_ps(rA,rA,_MM_SHUFFLE(3,0,2,1)),
	                _mm_shuffle_ps(rB,rB,_MM_SHUFFLE(3,1,0,2)));
	r2 = _mm_mul_ps(_mm_shuffle_ps(rA,rA,_MM_SHUFFLE(3,1,0,2)),
	                _mm_shuffle_ps(rB,rB,_MM_SHUFFLE(3,0,2,1)));
	out.m128 = _mm_sub_ps(r1, r2);
	return (out);
}

static __inline__ M_Vector3
M_VectorNormCross3_SSE(M_Vector3 a, M_Vector3 b)
{
	__m128 rA, rB, r1, r2, r3;
	M_Vector3 out;

	rA = a.m128;
	rB = b.m128;
	r1 = _mm_mul_ps(_mm_shuffle_ps(rA,rA,_MM_SHUFFLE(3,0,2,1)),
	                _mm_shuffle_ps(rB,rB,_MM_SHUFFLE(3,1,0,2)));
	r2 = _mm_mul_ps(_mm_shuffle_ps(rA,rA,_MM_SHUFFLE(3,1,0,2)),
	                _mm_shuffle_ps(rB,rB,_MM_SHUFFLE(3,0,2,1)));
	r3 = _mm_sub_ps(r1, r2);
	r1 = _mm_mul_ps(r3, r3);
	r2 = _mm_add_ss(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(0,0,0,0)),
	                _mm_add_ss(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(1,1,1,1)),
	                           _mm_shuffle_ps(r1,r1,_MM_SHUFFLE(2,2,2,2))));
	r1 = _mm_sqrt_ss(r2);
	r1 = _mm_shuffle_ps(r1,r1,_MM_SHUFFLE(0,0,0,0));
	out.m128 = _mm_div_ps(r3, r1);
	return (out);
}
static __inline__ M_Vector3
M_VectorNormCross3p_SSE(const M_Vector3 *_Nonnull a, const M_Vector3 *_Nonnull b)
{
	__m128 rA, rB, r1, r2, r3;
	M_Vector3 out;

	rA = a->m128;
	rB = b->m128;
	r1 = _mm_mul_ps(_mm_shuffle_ps(rA,rA,_MM_SHUFFLE(3,0,2,1)),
	                _mm_shuffle_ps(rB,rB,_MM_SHUFFLE(3,1,0,2)));
	r2 = _mm_mul_ps(_mm_shuffle_ps(rA,rA,_MM_SHUFFLE(3,1,0,2)),
	                _mm_shuffle_ps(rB,rB,_MM_SHUFFLE(3,0,2,1)));
	r3 = _mm_sub_ps(r1, r2);
	r1 = _mm_mul_ps(r3, r3);
	r2 = _mm_add_ss(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(0,0,0,0)),
	                _mm_add_ss(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(1,1,1,1)),
	                           _mm_shuffle_ps(r1,r1,_MM_SHUFFLE(2,2,2,2))));
	r1 = _mm_sqrt_ss(r2);
	r1 = _mm_shuffle_ps(r1,r1,_MM_SHUFFLE(0,0,0,0));
	out.m128 = _mm_div_ps(r3, r1);
	return (out);
}

static __inline__ M_Vector3
M_VectorScale3_SSE(M_Vector3 a, M_Real c)
{
	M_Vector3 out;
	out.m128 = _mm_mul_ps(a.m128, _mm_set1_ps(c));
	return (out);
}
static __inline__ M_Vector3
M_VectorScale3p_SSE(const M_Vector3 *_Nonnull a, M_Real c)
{
	M_Vector3 out;
	out.m128 = _mm_mul_ps(a->m128, _mm_set1_ps(c));
	return (out);
}
static __inline__ void
M_VectorScale3v_SSE(M_Vector3 *_Nonnull a, M_Real c)
{
	a->m128 = _mm_mul_ps(a->m128, _mm_set1_ps(c));
}

static __inline__ M_Vector3
M_VectorAdd3_SSE(M_Vector3 a, M_Vector3 b)
{
	M_Vector3 out;
	out.m128 = _mm_add_ps(a.m128, b.m128);
	return (out);
}
static __inline__ M_Vector3
M_VectorAdd3p_SSE(const M_Vector3 *_Nonnull a, const M_Vector3 *_Nonnull b)
{
	M_Vector3 out;
	out.m128 = _mm_add_ps(a->m128, b->m128);
	return (out);
}
static __inline__ void
M_VectorAdd3v_SSE(M_Vector3 *_Nonnull r, const M_Vector3 *_Nonnull a)
{
	r->m128 = _mm_add_ps(r->m128, a->m128);
}

static __inline__ M_Vector3
M_VectorSub3_SSE(M_Vector3 a, M_Vector3 b)
{
	M_Vector3 out;
	out.m128 = _mm_sub_ps(a.m128, b.m128);
	return (out);
}
static __inline__ M_Vector3
M_VectorSub3p_SSE(const M_Vector3 *_Nonnull a, const M_Vector3 *_Nonnull b)
{
	M_Vector3 out;
	out.m128 = _mm_sub_ps(a->m128, b->m128);
	return (out);
}
static __inline__ void
M_VectorSub3v_SSE(M_Vector3 *_Nonnull r, const M_Vector3 *_Nonnull a)
{
	r->m128 = _mm_sub_ps(r->m128, a->m128);
}

static __inline__ M_Real
M_VectorDistance3_SSE(M_Vector3 a, M_Vector3 b)
{
	__m128 r1, r2, r3;
	float dist;

	r1 = _mm_sub_ps(a.m128, b.m128);
	r1 = _mm_mul_ps(r1, r1);
	r2 = _mm_add_ss(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(0,0,0,0)),
	                _mm_add_ss(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(1,1,1,1)),
	                           _mm_shuffle_ps(r1,r1,_MM_SHUFFLE(2,2,2,2))));
	r3 = _mm_sqrt_ss(r2);
	_mm_store_ss(&dist, r3);
	return (M_Real)dist;
}

static __inline__ M_Real
M_VectorDistance3p_SSE(const M_Vector3 *_Nonnull a, const M_Vector3 *_Nonnull b)
{
	__m128 r1, r2, r3;
	float dist;

	r1 = _mm_sub_ps(a->m128, b->m128);
	r1 = _mm_mul_ps(r1, r1);
	r2 = _mm_add_ss(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(0,0,0,0)),
	                _mm_add_ss(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(1,1,1,1)),
	                           _mm_shuffle_ps(r1,r1,_MM_SHUFFLE(2,2,2,2))));
	r3 = _mm_sqrt_ss(r2);
	_mm_store_ss(&dist, r3);
	return (M_Real)dist;
}

static __inline__ M_Vector3
M_VectorAvg3_SSE(M_Vector3 a, M_Vector3 b)
{
	__m128 r1;
	M_Vector3 out;
	
	r1 = _mm_add_ps(a.m128, b.m128);
	out.m128 = _mm_div_ps(r1, _mm_set1_ps(2.0f));
	return (out);
}

static __inline__ M_Vector3
M_VectorAvg3p_SSE(const M_Vector3 *_Nonnull a, const M_Vector3 *_Nonnull b)
{
	__m128 r1;
	M_Vector3 out;
	
	r1 = _mm_add_ps(a->m128, b->m128);
	out.m128 = _mm_div_ps(r1, _mm_set1_ps(2.0f));
	return (out);
}

/* TODO */
static __inline__ void
M_VectorVecAngle3_SSE(M_Vector3 vOrig, M_Vector3 vOther,
    M_Real *_Nullable theta, M_Real *_Nullable phi)
{
	M_Vector3 vd;

	vd = M_VectorSub3p_SSE(&vOther, &vOrig);
	if (theta != NULL) {
		*theta = M_Atan2(vd.y, vd.x);
	}
	if (phi != NULL) {
		*phi = M_Atan2(M_Sqrt(vd.x*vd.x + vd.y*vd.y), vd.z);
	}
}

/* TODO */
static __inline__ M_Vector3
M_VectorLERP3_SSE(M_Vector3 v1, M_Vector3 v2, M_Real t)
{
	M_Vector3 v;

	v.x = v1.x + (v2.x - v1.x)*t;
	v.y = v1.y + (v2.y - v1.y)*t;
	v.z = v1.z + (v2.z - v1.z)*t;
	return (v);
}

/* TODO */
static __inline__ M_Vector3
M_VectorLERP3p_SSE(M_Vector3 *_Nonnull v1, M_Vector3 *_Nonnull v2, M_Real t)
{
	M_Vector3 v;

	v.x = v1->x + (v2->x - v1->x)*t;
	v.y = v1->y + (v2->y - v1->y)*t;
	v.z = v1->z + (v2->z - v1->z)*t;
	return (v);
}

/* TODO */
static __inline__ M_Vector3
M_VectorElemPow3_SSE(M_Vector3 v, M_Real p)
{
	M_Vector3 r;

	r.x = M_Pow(v.x, p);
	r.y = M_Pow(v.y, p);
	r.z = M_Pow(v.z, p);
	return (r);
}

static __inline__ M_Vector3
M_VectorSum3_SSE(const M_Vector3 *_Nonnull va, Uint count)
{
	__m128 r1;
	M_Vector3 out;
	Uint i;

	r1 = _mm_setzero_ps();
	for (i = 0; i < count; i++) {
		r1 = _mm_add_ps(r1, va[i].m128);
	}
	out.m128 = r1;
	return (out);
}
__END_DECLS

__BEGIN_DECLS
extern const M_VectorOps3 mVecOps3_SSE;
__END_DECLS

#endif /* HAVE_SSE */
