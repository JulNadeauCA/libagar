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

/*
 * Operations on 4x4 matrices with SSE1 operations.
 */

#include <config/have_opengl.h>
#include <config/have_sse.h>
#if defined(HAVE_OPENGL) && defined(HAVE_SSE)
#include <core/core.h>
#include "sg.h"

const SG_MatrixOps44 sgMatOps44_SSE = {
	"sse",
	SG_MatrixZero44_FPU,
	SG_MatrixZero44v_FPU,
	SG_MatrixIdentity44_FPU,
	SG_MatrixIdentity44v_FPU,
	SG_MatrixTranspose44_FPU,
	SG_MatrixTranspose44p_FPU,
	SG_MatrixTranspose44v_FPU,
	SG_MatrixInvert44_SSE,			/* To test */
	SG_MatrixInvert44p_SSE,			/* To test */
	SG_MatrixInvertGaussJordan44v_FPU,
	SG_MatrixMult44_SSE,			/* To test */
	SG_MatrixMult44v_SSE,			/* To test */
	SG_MatrixMult44pv_SSE,			/* To test */
	SG_MatrixMultVector44_FPU,
	SG_MatrixMultVector44p_FPU,
	SG_MatrixMultVector44v_FPU,
	SG_MatrixMultVector444_FPU,
	SG_MatrixMultVector444p_FPU,
	SG_MatrixMultVector444v_FPU,
	SG_MatrixCopy44_SSE,
	SG_MatrixToFloats44_FPU,
	SG_MatrixToDoubles44_FPU,
	SG_MatrixFromFloats44_FPU,
	SG_MatrixFromDoubles44_FPU,
	SG_MatrixGetDirection44_FPU,
	SG_MatrixDiagonalSwap44v_FPU,
	SG_MatrixRotateAxis44_FPU,
	SG_MatrixOrbitAxis44_FPU,
	SG_MatrixRotateEul44_FPU,
	SG_MatrixRotateI44_FPU,
	SG_MatrixRotateJ44_FPU,
	SG_MatrixRotateK44_FPU,
	SG_MatrixTranslate44_FPU,
	SG_MatrixTranslate344_FPU,
	SG_MatrixTranslateX44_FPU,
	SG_MatrixTranslateY44_FPU,
	SG_MatrixTranslateZ44_FPU,
	SG_MatrixScale44_FPU,
	SG_MatrixUniScale44_FPU,
};

SG_Matrix
SG_MatrixInvert44_SSE(SG_Matrix A)
{
	SG_Matrix Ainv;
	float *src = &A.m[0][0];
	float *dst = &Ainv.m[0][0];
	__m128 minor0, minor1, minor2, minor3;
	__m128 row0, row1, row2, row3;
	__m128 det, tmp1;

	/* XXX */
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

SG_Matrix
SG_MatrixInvert44p_SSE(const SG_Matrix *A)
{
	SG_Matrix Ainv;
	const float *src = &A->m[0][0];
	float *dst = &Ainv.m[0][0];
	__m128 minor0, minor1, minor2, minor3;
	__m128 row0, row1, row2, row3;
	__m128 det, tmp1;

	/* XXX */
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

#endif /* HAVE_OPENGL and HAVE_SSE */
