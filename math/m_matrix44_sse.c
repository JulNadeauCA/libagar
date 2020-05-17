/*
 * Copyright (c) 2007-2018 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Operations on 4x4 matrices using Streaming SIMD Extensions.
 */

#include <agar/config/have_sse.h>
#ifdef HAVE_SSE

#include <agar/core/core.h>
#include <agar/math/m.h>

const M_MatrixOps44 mMatOps44_SSE = {
	"sse",
	M_MatrixZero44_SSE,			/* -34 clks */
	M_MatrixZero44v_SSE,			/* +25 clks */
	M_MatrixIdentity44_SSE,			/* -13 clks */
	M_MatrixIdentity44v_SSE,		/* -8 clks */
	M_MatrixTranspose44_SSE,		/* +4 clks */
	M_MatrixTranspose44p_SSE,		/* -22 clks */
	M_MatrixTranspose44v_SSE,		/* -28 clks */
	M_MatrixInvert44_SSE,			/* -295 clks */
	M_MatrixInvertElim44_FPU,
	M_MatrixMult44_SSE,			/* -61 clks */
	M_MatrixMult44v_SSE,			/* -59 clks */
	M_MatrixMultVector44_SSE,		/* -5 clks */
	M_MatrixMultVector44p_SSE,		/* -10 clks */
	M_MatrixMultVector44v_SSE,		/* -40 clks */
	M_MatrixCopy44_SSE,			/* -3 clks */
	M_MatrixToFloats44_FPU,
	M_MatrixToDoubles44_FPU,
	M_MatrixFromFloats44_FPU,
	M_MatrixFromDoubles44_FPU,
	M_MatrixRotateAxis44_SSE,		/* -1446 clks */
	M_MatrixOrbitAxis44_FPU,
	M_MatrixRotateEul44_FPU,
	M_MatrixRotate44I_SSE,			/* -1204 clks */
	M_MatrixRotate44J_SSE,			/* -1204 clks */
	M_MatrixRotate44K_SSE,			/* -1204 clks */
	M_MatrixTranslatev44_SSE,		/* -549 clks */
	M_MatrixTranslate44_SSE,
	M_MatrixTranslateX44_SSE,		/* -608 clks */
	M_MatrixTranslateY44_SSE,		/* -608 clks */
	M_MatrixTranslateZ44_SSE,		/* -608 clks */
	M_MatrixScale44_SSE,			/* -522 clks */
	M_MatrixUniScale44_SSE,			/* -595 clks */
};

M_Matrix44
M_MatrixInvert44_SSE(M_Matrix44 A)
{
	M_Matrix44 Ainv;
	float *src = &A.m[0][0];
	float *dst = &Ainv.m[0][0];
	__m128 minor0, minor1, minor2, minor3;
	__m128 row0, row1, row2, row3;
	__m128 det, tmp1;

	memset(&tmp1, 0, sizeof(tmp1));
	memset(&row1, 0, sizeof(row1));
	memset(&row3, 0, sizeof(row3));

	tmp1	= _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64 *)(src)),
	                                          (__m64 *)(src+4));
	row1	= _mm_loadh_pi(_mm_loadl_pi(row1, (__m64 *)(src+8)),
	                                          (__m64 *)(src+12));
	row0	= _mm_shuffle_ps(tmp1, row1, 0x88);
	row1	= _mm_shuffle_ps(row1, tmp1, 0xDD);
	tmp1	= _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64 *)(src+2)),
	                                          (__m64 *)(src+6));
	row3	= _mm_loadh_pi(_mm_loadl_pi(row3, (__m64 *)(src+10)),
	                                          (__m64 *)(src+14));
	row2	= _mm_shuffle_ps(tmp1, row3, 0x88);
	row3	= _mm_shuffle_ps(row3, tmp1, 0xDD);

	tmp1	= _mm_mul_ps(row2, row3);
	tmp1	= _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor0	= _mm_mul_ps(row1, tmp1);
	minor1	= _mm_mul_ps(row0, tmp1);
	tmp1	= _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0	= _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
	minor1	= _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
	minor1	= _mm_shuffle_ps(minor1, minor1, 0x4E);

	tmp1	= _mm_mul_ps(row1, row2);
	tmp1	= _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor0	= _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
	minor3	= _mm_mul_ps(row0, tmp1);
	tmp1	= _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0	= _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
	minor3	= _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
	minor3	= _mm_shuffle_ps(minor3, minor3, 0x4E);

	tmp1	= _mm_mul_ps(_mm_shuffle_ps(row1, row1, 0x4E), row3);
	tmp1	= _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	row2	= _mm_shuffle_ps(row2, row2, 0x4E);
	minor0	= _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
	minor2	= _mm_mul_ps(row0, tmp1);
	tmp1	= _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0	= _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
	minor2	= _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
	minor2	= _mm_shuffle_ps(minor2, minor2, 0x4E);

	tmp1	= _mm_mul_ps(row0, row1);
	tmp1	= _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor2	= _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
	minor3	= _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);
	tmp1	= _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor2	= _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
	minor3	= _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));

	tmp1	= _mm_mul_ps(row0, row3);
	tmp1	= _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor1	= _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
	minor2	= _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);
	tmp1	= _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor1	= _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
	minor2	= _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));

	tmp1	= _mm_mul_ps(row0, row2);
	tmp1	= _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor1	= _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
	minor3	= _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));
	tmp1	= _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor1	= _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
	minor3	= _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);

	det	= _mm_mul_ps(row0, minor0);
	det	= _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
	det	= _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);
	tmp1	= _mm_rcp_ss(det);
	det	= _mm_sub_ss(_mm_add_ss(tmp1, tmp1),
	                     _mm_mul_ss(det, _mm_mul_ss(tmp1,tmp1)));
	det	= _mm_shuffle_ps(det, det, 0x00);

	minor0	= _mm_mul_ps(det, minor0);
	_mm_storel_pi((__m64 *)(dst), minor0);
	_mm_storeh_pi((__m64 *)(dst+2), minor0);

	minor1	= _mm_mul_ps(det, minor1);
	_mm_storel_pi((__m64 *)(dst+4), minor1);
	_mm_storeh_pi((__m64 *)(dst+6), minor1);
	
	minor2	= _mm_mul_ps(det, minor2);
	_mm_storel_pi((__m64 *)(dst+8), minor2);
	_mm_storeh_pi((__m64 *)(dst+10), minor2);
	
	minor3	= _mm_mul_ps(det, minor3);
	_mm_storel_pi((__m64 *)(dst+12), minor3);
	_mm_storeh_pi((__m64 *)(dst+14), minor3);

	return (Ainv);
}

void
M_MatrixRotateAxis44_SSE(M_Matrix44 *M, M_Real theta, M_Vector3 A)
{
	float s = sinf((float)theta);
	float c = cosf((float)theta);
	float t = 1.0f - c;
	M_Matrix44 R;
	__m128 a = A.m128, r1;
#ifdef HAVE_SSE3
	__m128 rC1 = _mm_set_ps(-c,    s*A.z, 0.0f,  +c);	/* 1,3 1,2 3 2 */
	__m128 rC2 = _mm_set_ps(0.0f, -s*A.y, s*A.y, -s*A.x);	/* 1,2 3 1 2,3 */
#endif
	
	/* m1: [t*AxAx + c,    t*AxAy + sAz,    t*AxAz - sAy,    0] */
	r1 = _mm_mul_ps(_mm_set1_ps(t), a);
	r1 = _mm_mul_ps(r1, _mm_shuffle_ps(a,a,_MM_SHUFFLE(0,0,0,0)));
#ifdef HAVE_SSE3
	R.m1 = _mm_addsub_ps(r1, _mm_shuffle_ps(rC1,rC2,_MM_SHUFFLE(3,1,2,3)));
#else
	R.m1 = _mm_add_ps(r1, _mm_set_ps(0.0f, -s*A.y, s*A.z, c));
#endif

	/* m2: [t*AxAy - sAz,    t*AyAy + c,    t*AyAz + sAx,    0] */
	r1 = _mm_mul_ps(_mm_set1_ps(t), _mm_shuffle_ps(a,a,_MM_SHUFFLE(3,1,1,0)));
	r1 = _mm_mul_ps(r1, _mm_shuffle_ps(a,a,_MM_SHUFFLE(3,2,1,1)));
#ifdef HAVE_SSE3
	R.m2 = _mm_addsub_ps(r1, _mm_shuffle_ps(rC1,rC2,_MM_SHUFFLE(3,0,0,2)));
#else
	R.m2 = _mm_add_ps(r1, _mm_set_ps(0.0f, +s*A.x, c, -s*A.z));
#endif

	/* m3: [t*AxAz + sAy,    t*AyAz - sAx,    t*AzAz + c,    0] */
	r1 = _mm_mul_ps(_mm_set1_ps(t), a);
	r1 = _mm_mul_ps(r1, _mm_shuffle_ps(a,a,_MM_SHUFFLE(0,2,2,2)));
#ifdef HAVE_SSE3
	R.m3 = _mm_addsub_ps(r1, _mm_shuffle_ps(rC2,rC1,_MM_SHUFFLE(1,3,0,2)));
#else
	R.m3 = _mm_add_ps(r1, _mm_set_ps(0.0f, c, -s*A.x, +s*A.y));
#endif
	/* m4: [0,    0,    0,    1] */
	R.m4 = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);

	M_MatrixMult44v_SSE(M, &R);
}

void
M_MatrixRotate44I_SSE(M_Matrix44 *M, M_Real theta)
{
	float s = sinf((float)theta);
	float c = cosf((float)theta);
	M_Matrix44 R;

	R.m1 = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);
	R.m2 = _mm_set_ps(0.0f, -s,   c,    0.0f);
	R.m3 = _mm_set_ps(0.0f, c,    s,    0.0f);
	R.m4 = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
	M_MatrixMult44v_SSE(M, &R);
}
void
M_MatrixRotate44J_SSE(M_Matrix44 *M, M_Real theta)
{
	float s = sinf((float)theta);
	float c = cosf((float)theta);
	M_Matrix44 R;

	R.m1 = _mm_set_ps(0.0f, s,    0.0f, c);
	R.m2 = _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f);
	R.m3 = _mm_set_ps(0.0f, c,    0.0f, -s);
	R.m4 = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
	M_MatrixMult44v_SSE(M, &R);
}
void
M_MatrixRotate44K_SSE(M_Matrix44 *M, M_Real theta)
{
	float s = sinf((float)theta);
	float c = cosf((float)theta);
	M_Matrix44 R;

	R.m1 = _mm_set_ps(0.0f, 0.0f, -s,   c);
	R.m2 = _mm_set_ps(0.0f, 0.0f, c,    s);
	R.m3 = _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f);
	R.m4 = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
	M_MatrixMult44v_SSE(M, &R);
}

void
M_MatrixTranslate44_SSE(M_Matrix44 *M, M_Real x, M_Real y, M_Real z)
{
	M_Matrix44 T;
	float xf = (float)x;
	float yf = (float)y;
	float zf = (float)z;

	T.m1 = _mm_set_ps(xf,   0.0f, 0.0f, 1.0f);
	T.m2 = _mm_set_ps(yf,   0.0f, 1.0f, 0.0f);
	T.m3 = _mm_set_ps(zf,   1.0f, 0.0f, 0.0f);
	T.m4 = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
	M_MatrixMult44v_SSE(M, &T);
}

void
M_MatrixTranslatev44_SSE(M_Matrix44 *M, M_Vector3 v)
{
	M_Matrix44 T;

	T.m1 = _mm_set_ps(v.x,  0.0f, 0.0f, 1.0f);
	T.m2 = _mm_set_ps(v.y,  0.0f, 1.0f, 0.0f);
	T.m3 = _mm_set_ps(v.z,  1.0f, 0.0f, 0.0f);
	T.m4 = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
	M_MatrixMult44v_SSE(M, &T);
}
void
M_MatrixTranslateX44_SSE(M_Matrix44 *M, M_Real x)
{
	float xf = (float)x;
	M_Matrix44 T;

	T.m1 = _mm_set_ps(xf,   0.0f, 0.0f, 1.0f);
	T.m2 = _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f);
	T.m3 = _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f);
	T.m4 = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
	M_MatrixMult44v_SSE(M, &T);
}
void
M_MatrixTranslateY44_SSE(M_Matrix44 *M, M_Real y)
{
	float yf = (float)y;
	M_Matrix44 T;

	T.m1 = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);
	T.m2 = _mm_set_ps(yf,   0.0f, 1.0f, 0.0f);
	T.m3 = _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f);
	T.m4 = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
	M_MatrixMult44v_SSE(M, &T);
}
void
M_MatrixTranslateZ44_SSE(M_Matrix44 *M, M_Real z)
{
	float zf = (float)z;
	M_Matrix44 T;

	T.m1 = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);
	T.m2 = _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f);
	T.m3 = _mm_set_ps(zf,   1.0f, 0.0f, 0.0f);
	T.m4 = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
	M_MatrixMult44v_SSE(M, &T);
}

void
M_MatrixScale44_SSE(M_Matrix44 *M, M_Real x, M_Real y, M_Real z, M_Real w)
{
	float xf = (float)x;
	float yf = (float)y;
	float zf = (float)z;
	float wf = (float)w;
	M_Matrix44 S;

	S.m1 = _mm_set_ps(0.0f, 0.0f, 0.0f, xf);
	S.m2 = _mm_set_ps(0.0f, 0.0f, yf,   0.0f);
	S.m3 = _mm_set_ps(0.0f, zf,   0.0f, 0.0f);
	S.m4 = _mm_set_ps(wf,   0.0f, 0.0f, 0.0f);
	M_MatrixMult44v_SSE(M, &S);
}
void
M_MatrixUniScale44_SSE(M_Matrix44 *M, M_Real r)
{
	M_Matrix44 S;
	float f = (float)r;
	
	S.m1 = _mm_set_ps(0.0f, 0.0f, 0.0f, f);
	S.m2 = _mm_shuffle_ps(S.m1,S.m1,_MM_SHUFFLE(3,3,0,3));
	S.m3 = _mm_shuffle_ps(S.m1,S.m1,_MM_SHUFFLE(3,0,3,3));
	S.m4 = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
	M_MatrixMult44v_SSE(M, &S);
}

#endif /* HAVE_SSE */
