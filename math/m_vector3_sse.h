/*
 * Copyright (c) 2007-2008 Hypertriton, Inc. <http://hypertriton.com/>
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

	v.m128 = _mm_set_ps(0.0, z, y, x);
	return (v);
}

/* TODO */
static __inline__ M_Vector3
M_VectorMirror3_SSE(M_Vector3 a, int x, int y, int z)
{
	M_Vector3 b;

	b.x = x ? -a.x : a.x;
	b.y = y ? -a.y : a.y;
	b.z = z ? -a.z : a.z;
	return (b);
}

/* TODO */
static __inline__ M_Vector3
M_VectorMirror3p_SSE(const M_Vector3 *a, int x, int y, int z)
{
	M_Vector3 b;

	b.x = x ? -(a->x) : a->x;
	b.y = y ? -(a->y) : a->y;
	b.z = z ? -(a->z) : a->z;
	return (b);
}

static __inline__ M_Vector3
M_VectorNorm3_SSE(M_Vector3 v)
{
	__m128 a, b, rsqrt, lenr;
	M_Vector3 n;

	a = _mm_mul_ps(v.m128, v.m128);
	b = _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0,0,0,0)),
	               _mm_add_ss(_mm_shuffle_ps(a,a,_MM_SHUFFLE(1,1,1,1)),
	               _mm_shuffle_ps(a,a,_MM_SHUFFLE(2,2,2,2))));
	rsqrt	= _mm_rsqrt_ss(b);
	lenr	= _mm_shuffle_ps(rsqrt, rsqrt, _MM_SHUFFLE(0,0,0,0));
	n.m128	= _mm_mul_ps(n.m128, lenr);
	return (n);
}

static __inline__ M_Vector3
M_VectorNorm3p_SSE(const M_Vector3 *v)
{
	__m128 a, b, rsqrt, lenr;
	M_Vector3 n;

	a = _mm_mul_ps(v->m128, v->m128);
	b = _mm_add_ss(_mm_shuffle_ps(a, a, _MM_SHUFFLE(0,0,0,0)),
	               _mm_add_ss(_mm_shuffle_ps(a,a,_MM_SHUFFLE(1,1,1,1)),
	               _mm_shuffle_ps(a,a,_MM_SHUFFLE(2,2,2,2))));
	rsqrt	= _mm_rsqrt_ss(b);
	lenr	= _mm_shuffle_ps(rsqrt, rsqrt, _MM_SHUFFLE(0,0,0,0));
	n.m128	= _mm_mul_ps(n.m128, lenr);
	return (n);
}

static __inline__ M_Vector3
M_VectorScale3_SSE(M_Vector3 a, M_Real c)
{
	M_Vector3 b;
	__m128 vs;

	vs = _mm_set1_ps(c);
	b.m128 = _mm_mul_ps(a.m128, vs);
	return (b);
}

static __inline__ M_Vector3
M_VectorScale3p_SSE(const M_Vector3 *a, M_Real c)
{
	M_Vector3 b;

	b.m128 = _mm_mul_ps(a->m128, _mm_set1_ps(c));
	return (b);
}

static __inline__ M_Vector3
M_VectorAdd3_SSE(M_Vector3 a, M_Vector3 b)
{
	M_Vector3 c;

	c.m128 = _mm_add_ps(a.m128, b.m128);
	return (c);
}

static __inline__ M_Vector3
M_VectorAdd3p_SSE(const M_Vector3 *a, const M_Vector3 *b)
{
	M_Vector3 c;

	c.m128 = _mm_add_ps(a->m128, b->m128);
	return (c);
}

static __inline__ M_Vector3
M_VectorSub3_SSE(M_Vector3 a, M_Vector3 b)
{
	M_Vector3 c;

	c.m128 = _mm_sub_ps(a.m128, b.m128);
	return (c);
}

static __inline__ M_Vector3
M_VectorSub3p_SSE(const M_Vector3 *a, const M_Vector3 *b)
{
	M_Vector3 c;

	c.m128 = _mm_sub_ps(a->m128, b->m128);
	return (c);
}

/* TODO */
static __inline__ M_Real
M_VectorDistance3_SSE(M_Vector3 a, M_Vector3 b)
{
	return M_VectorLen3_FPU(M_VectorSub3_SSE(a,b));
}

/* TODO */
static __inline__ M_Real
M_VectorDistance3p_SSE(const M_Vector3 *a, const M_Vector3 *b)
{
	return M_VectorLen3_FPU(M_VectorAdd3_SSE(*b,
	                        M_VectorMirror3p_SSE(a,1,1,1)));
}

/* TODO */
static __inline__ M_Vector3
M_VectorAvg3_SSE(M_Vector3 a, M_Vector3 b)
{
	M_Vector3 c;
	
	c.x = (a.x + b.x)/2.0;
	c.y = (a.y + b.y)/2.0;
	c.z = (a.z + b.z)/2.0;
	return (c);
}

/* TODO */
static __inline__ M_Vector3
M_VectorAvg3p_SSE(const M_Vector3 *a, const M_Vector3 *b)
{
	M_Vector3 c;
	
	c.x = (a->x + b->x)/2.0;
	c.y = (a->y + b->y)/2.0;
	c.z = (a->z + b->z)/2.0;
	return (c);
}

/* TODO */
static __inline__ void
M_VectorVecAngle3_SSE(M_Vector3 vOrig, M_Vector3 vOther, M_Real *theta,
    M_Real *phi)
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
M_VectorRotateI3_SSE(M_Vector3 a, M_Real theta)
{
	M_Vector3 b;

	b.x = a.x;
	b.y = (a.y * M_Cos(theta)) + (a.z * -M_Sin(theta));
	b.z = (a.y * M_Sin(theta)) + (a.z *  M_Cos(theta));
	return (b);
}

/* TODO */
static __inline__ M_Vector3
M_VectorRotateJ3_SSE(M_Vector3 a, M_Real theta)
{
	M_Vector3 b;

	b.x = (a.x *  M_Cos(theta)) + (a.z * M_Sin(theta));
	b.y = a.y;
	b.z = (a.x * -M_Sin(theta)) + (a.z * M_Cos(theta));
	return (b);
}

/* TODO */
static __inline__ M_Vector3
M_VectorRotateK3_SSE(M_Vector3 a, M_Real theta)
{
	M_Vector3 b;

	b.x = (a.x * M_Cos(theta)) + (a.y * -M_Sin(theta));
	b.y = (a.x * M_Sin(theta)) + (a.y *  M_Cos(theta));
	b.z = a.z;
	return (b);
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
M_VectorLERP3p_SSE(M_Vector3 *v1, M_Vector3 *v2, M_Real t)
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
__END_DECLS

__BEGIN_DECLS
extern const M_VectorOps3 mVecOps3_SSE;

M_Vector3	M_VectorRotate3_SSE(M_Vector3, M_Real, M_Vector3);
void		M_VectorRotate3v_SSE(M_Vector3 *, M_Real, M_Vector3);
M_Vector3	M_VectorRotateQuat3_SSE(M_Vector3, M_Quaternion);
M_Vector3	M_VectorAdd3n_SSE(int, ...);
M_Vector3	M_VectorSub3n_SSE(int, ...);
__END_DECLS

#endif /* HAVE_SSE */
