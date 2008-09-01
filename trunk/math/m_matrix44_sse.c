/*
 * Public domain.
 * Operations on 4x4 matrices with SSE1 operations.
 */

#include <config/have_sse.h>
#ifdef HAVE_SSE

#include <core/core.h>
#include "m.h"

const M_MatrixOps44 mMatOps44_SSE = {
	"sse",
	M_MatrixZero44_FPU,
	M_MatrixZero44v_FPU,
	M_MatrixIdentity44_FPU,
	M_MatrixIdentity44v_FPU,
	M_MatrixTranspose44_FPU,
	M_MatrixTranspose44p_FPU,
	M_MatrixTranspose44v_FPU,
	M_MatrixInvert44_SSE,			/* To test */
	M_MatrixInvert44p_SSE,			/* To test */
	M_MatrixInvertGaussJordan44v_FPU,
	M_MatrixMult44_SSE,			/* To test */
	M_MatrixMult44v_SSE,			/* To test */
	M_MatrixMult44pv_SSE,			/* To test */
	M_MatrixMultVector44_FPU,
	M_MatrixMultVector44p_FPU,
	M_MatrixMultVector44v_FPU,
	M_MatrixMultVector444_FPU,
	M_MatrixMultVector444p_FPU,
	M_MatrixMultVector444v_FPU,
	M_MatrixCopy44_SSE,
	M_MatrixToFloats44_FPU,
	M_MatrixToDoubles44_FPU,
	M_MatrixFromFloats44_FPU,
	M_MatrixFromDoubles44_FPU,
	M_MatrixGetDirection44_FPU,
	M_MatrixDiagonalSwap44v_FPU,
	M_MatrixRotateAxis44_FPU,
	M_MatrixOrbitAxis44_FPU,
	M_MatrixRotateEul44_FPU,
	M_MatrixRotate44I_FPU,
	M_MatrixRotate44J_FPU,
	M_MatrixRotate44K_FPU,
	M_MatrixTranslate44_FPU,
	M_MatrixTranslate344_FPU,
	M_MatrixTranslateX44_FPU,
	M_MatrixTranslateY44_FPU,
	M_MatrixTranslateZ44_FPU,
	M_MatrixScale44_FPU,
	M_MatrixUniScale44_FPU,
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

M_Matrix44
M_MatrixInvert44p_SSE(const M_Matrix44 *A)
{
	M_Matrix44 Ainv;
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

#endif /* HAVE_SSE */
