/*
 * Copyright (c) 2006-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Operations on 4x4 matrices (FPU version).
 */

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL
#include <core/core.h>
#include "sg.h"

const SG_MatrixOps44 sgMatOps44_FPU = {
	"scalar",
	SG_MatrixZero44_FPU,
	SG_MatrixZero44v_FPU,
	SG_MatrixIdentity44_FPU,
	SG_MatrixIdentity44v_FPU,
	SG_MatrixTranspose44_FPU,
	SG_MatrixTranspose44p_FPU,
	SG_MatrixTranspose44v_FPU,
	SG_MatrixInvert44_FPU,
	SG_MatrixInvert44p_FPU,
	SG_MatrixInvertGaussJordan44v_FPU,
	SG_MatrixMult44_FPU,
	SG_MatrixMult44v_FPU,
	SG_MatrixMult44pv_FPU,
	SG_MatrixMultVector44_FPU,
	SG_MatrixMultVector44p_FPU,
	SG_MatrixMultVector44v_FPU,
	SG_MatrixMultVector444_FPU,
	SG_MatrixMultVector444p_FPU,
	SG_MatrixMultVector444v_FPU,
	SG_MatrixCopy44_FPU,
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
SG_MatrixInvert44_FPU(SG_Matrix A)
{
	SG_Matrix Ainv;
	SG_Real tmp[12];
	SG_Real src[16];
	SG_Real dei;
	int i, j;

	/* Transpose matrix into flat array */
	for (i = 0; i < 4; i++) {
		src[i]	  = A.m[i][0];
		src[i+4]  = A.m[i][1];
		src[i+8]  = A.m[i][2];
		src[i+12] = A.m[i][3];
	}

	/* Compute pairs for first 8 elements (cofactors) */
	tmp[0]	= src[10] * src[15];
	tmp[1]	= src[11] * src[14];
	tmp[2]	= src[ 9] * src[15];
	tmp[3]	= src[11] * src[13];
	tmp[4]	= src[ 9] * src[14];
	tmp[5]	= src[10] * src[13];
	tmp[6]	= src[ 8] * src[15];
	tmp[7]	= src[11] * src[12];
	tmp[8]	= src[ 8] * src[14];
	tmp[9]	= src[10] * src[12];
	tmp[10]	= src[ 8] * src[13];
	tmp[11]	= src[ 9] * src[12];

	/* Compute first 8 elements (cofactors) */
	Ainv.m[0][0]  = tmp[ 0]*src[ 5] + tmp[ 3]*src[ 6] + tmp[ 4]*src[ 7];
	Ainv.m[0][0] -= tmp[ 1]*src[ 5] + tmp[ 2]*src[ 6] + tmp[ 5]*src[ 7];
	Ainv.m[0][1]  = tmp[ 1]*src[ 4] + tmp[ 6]*src[ 6] + tmp[ 9]*src[ 7];
	Ainv.m[0][1] -= tmp[ 0]*src[ 4] + tmp[ 7]*src[ 6] + tmp[ 8]*src[ 7];
	Ainv.m[0][2]  = tmp[ 2]*src[ 4] + tmp[ 7]*src[ 5] + tmp[10]*src[ 7];
	Ainv.m[0][2] -= tmp[ 3]*src[ 4] + tmp[ 6]*src[ 5] + tmp[11]*src[ 7];
	Ainv.m[0][3]  = tmp[ 5]*src[ 4] + tmp[ 8]*src[ 5] + tmp[11]*src[ 6];
	Ainv.m[0][3] -= tmp[ 4]*src[ 4] + tmp[ 9]*src[ 5] + tmp[10]*src[ 6];
	Ainv.m[1][0]  = tmp[ 1]*src[ 1] + tmp[ 2]*src[ 2] + tmp[ 5]*src[ 3];
	Ainv.m[1][0] -= tmp[ 0]*src[ 1] + tmp[ 3]*src[ 2] + tmp[ 4]*src[ 3];
	Ainv.m[1][1]  = tmp[ 0]*src[ 0] + tmp[ 7]*src[ 2] + tmp[ 8]*src[ 3];
	Ainv.m[1][1] -= tmp[ 1]*src[ 0] + tmp[ 6]*src[ 2] + tmp[ 9]*src[ 3];
	Ainv.m[1][2]  = tmp[ 3]*src[ 0] + tmp[ 6]*src[ 1] + tmp[11]*src[ 3];
	Ainv.m[1][2] -= tmp[ 2]*src[ 0] + tmp[ 7]*src[ 1] + tmp[10]*src[ 3];
	Ainv.m[1][3]  = tmp[ 4]*src[ 0] + tmp[ 9]*src[ 1] + tmp[10]*src[ 2];
	Ainv.m[1][3] -= tmp[ 5]*src[ 0] + tmp[ 8]*src[ 1] + tmp[11]*src[ 2];

	/* Compute pairs for second 8 elements (cofactors) */
	tmp[ 0] = src[2]*src[7];
	tmp[ 1] = src[3]*src[6];
	tmp[ 2] = src[1]*src[7];
	tmp[ 3] = src[3]*src[5];
	tmp[ 4] = src[1]*src[6];
	tmp[ 5] = src[2]*src[5];
	tmp[ 6] = src[0]*src[7];
	tmp[ 7] = src[3]*src[4];
	tmp[ 8] = src[0]*src[6];
	tmp[ 9] = src[2]*src[4];
	tmp[10] = src[0]*src[5];
	tmp[11] = src[1]*src[4];

	/* Compute second 8 elements (cofactors) */
	Ainv.m[2][0]  = tmp[ 0]*src[13] + tmp[ 3]*src[14] + tmp[ 4]*src[15];
	Ainv.m[2][0] -= tmp[ 1]*src[13] + tmp[ 2]*src[14] + tmp[ 5]*src[15];
	Ainv.m[2][1]  = tmp[ 1]*src[12] + tmp[ 6]*src[14] + tmp[ 9]*src[15];
	Ainv.m[2][1] -= tmp[ 0]*src[12] + tmp[ 7]*src[14] + tmp[ 8]*src[15];
	Ainv.m[2][2]  = tmp[ 2]*src[12] + tmp[ 7]*src[13] + tmp[10]*src[15];
	Ainv.m[2][2] -= tmp[ 3]*src[12] + tmp[ 6]*src[13] + tmp[11]*src[15];
	Ainv.m[2][3]  = tmp[ 5]*src[12] + tmp[ 8]*src[13] + tmp[11]*src[14];
	Ainv.m[2][3] -= tmp[ 4]*src[12] + tmp[ 9]*src[13] + tmp[10]*src[14];
	Ainv.m[3][0]  = tmp[ 2]*src[10] + tmp[ 5]*src[11] + tmp[ 1]*src[ 9];
	Ainv.m[3][0] -= tmp[ 4]*src[11] + tmp[ 0]*src[ 9] + tmp[ 3]*src[10];
	Ainv.m[3][1]  = tmp[ 8]*src[11] + tmp[ 0]*src[ 8] + tmp[ 7]*src[10];
	Ainv.m[3][1] -= tmp[ 6]*src[10] + tmp[ 9]*src[11] + tmp[ 1]*src[ 8];
	Ainv.m[3][2]  = tmp[ 6]*src[ 9] + tmp[11]*src[11] + tmp[ 3]*src[ 8];
	Ainv.m[3][2] -= tmp[10]*src[11] + tmp[ 2]*src[ 8] + tmp[ 7]*src[ 9];
	Ainv.m[3][3]  = tmp[10]*src[10] + tmp[ 4]*src[ 8] + tmp[ 9]*src[ 9];
	Ainv.m[3][3] -= tmp[ 8]*src[ 9] + tmp[11]*src[10] + tmp[ 5]*src[ 8];

	dei = 1.0 / (src[0]*Ainv.m[0][0] +
	             src[1]*Ainv.m[0][1] +
		     src[2]*Ainv.m[0][2] +
	             src[3]*Ainv.m[0][3]);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++)
			Ainv.m[i][j] *= dei;
	}
	return (Ainv);
}

SG_Matrix
SG_MatrixInvert44p_FPU(const SG_Matrix *A)
{
	SG_Matrix Ainv;
	SG_Real tmp[12];
	SG_Real src[16];
	SG_Real dei;
	int i, j;

	/* Transpose matrix into flat array */
	for (i = 0; i < 4; i++) {
		src[i]	  = A->m[i][0];
		src[i+4]  = A->m[i][1];
		src[i+8]  = A->m[i][2];
		src[i+12] = A->m[i][3];
	}

	/* Compute pairs for first 8 elements (cofactors) */
	tmp[0]	= src[10] * src[15];
	tmp[1]	= src[11] * src[14];
	tmp[2]	= src[ 9] * src[15];
	tmp[3]	= src[11] * src[13];
	tmp[4]	= src[ 9] * src[14];
	tmp[5]	= src[10] * src[13];
	tmp[6]	= src[ 8] * src[15];
	tmp[7]	= src[11] * src[12];
	tmp[8]	= src[ 8] * src[14];
	tmp[9]	= src[10] * src[12];
	tmp[10]	= src[ 8] * src[13];
	tmp[11]	= src[ 9] * src[12];

	/* Compute first 8 elements (cofactors) */
	Ainv.m[0][0]  = tmp[ 0]*src[ 5] + tmp[ 3]*src[ 6] + tmp[ 4]*src[ 7];
	Ainv.m[0][0] -= tmp[ 1]*src[ 5] + tmp[ 2]*src[ 6] + tmp[ 5]*src[ 7];
	Ainv.m[0][1]  = tmp[ 1]*src[ 4] + tmp[ 6]*src[ 6] + tmp[ 9]*src[ 7];
	Ainv.m[0][1] -= tmp[ 0]*src[ 4] + tmp[ 7]*src[ 6] + tmp[ 8]*src[ 7];
	Ainv.m[0][2]  = tmp[ 2]*src[ 4] + tmp[ 7]*src[ 5] + tmp[10]*src[ 7];
	Ainv.m[0][2] -= tmp[ 3]*src[ 4] + tmp[ 6]*src[ 5] + tmp[11]*src[ 7];
	Ainv.m[0][3]  = tmp[ 5]*src[ 4] + tmp[ 8]*src[ 5] + tmp[11]*src[ 6];
	Ainv.m[0][3] -= tmp[ 4]*src[ 4] + tmp[ 9]*src[ 5] + tmp[10]*src[ 6];
	Ainv.m[1][0]  = tmp[ 1]*src[ 1] + tmp[ 2]*src[ 2] + tmp[ 5]*src[ 3];
	Ainv.m[1][0] -= tmp[ 0]*src[ 1] + tmp[ 3]*src[ 2] + tmp[ 4]*src[ 3];
	Ainv.m[1][1]  = tmp[ 0]*src[ 0] + tmp[ 7]*src[ 2] + tmp[ 8]*src[ 3];
	Ainv.m[1][1] -= tmp[ 1]*src[ 0] + tmp[ 6]*src[ 2] + tmp[ 9]*src[ 3];
	Ainv.m[1][2]  = tmp[ 3]*src[ 0] + tmp[ 6]*src[ 1] + tmp[11]*src[ 3];
	Ainv.m[1][2] -= tmp[ 2]*src[ 0] + tmp[ 7]*src[ 1] + tmp[10]*src[ 3];
	Ainv.m[1][3]  = tmp[ 4]*src[ 0] + tmp[ 9]*src[ 1] + tmp[10]*src[ 2];
	Ainv.m[1][3] -= tmp[ 5]*src[ 0] + tmp[ 8]*src[ 1] + tmp[11]*src[ 2];

	/* Compute pairs for second 8 elements (cofactors) */
	tmp[ 0] = src[2]*src[7];
	tmp[ 1] = src[3]*src[6];
	tmp[ 2] = src[1]*src[7];
	tmp[ 3] = src[3]*src[5];
	tmp[ 4] = src[1]*src[6];
	tmp[ 5] = src[2]*src[5];
	tmp[ 6] = src[0]*src[7];
	tmp[ 7] = src[3]*src[4];
	tmp[ 8] = src[0]*src[6];
	tmp[ 9] = src[2]*src[4];
	tmp[10] = src[0]*src[5];
	tmp[11] = src[1]*src[4];

	/* Compute second 8 elements (cofactors) */
	Ainv.m[2][0]  = tmp[ 0]*src[13] + tmp[ 3]*src[14] + tmp[ 4]*src[15];
	Ainv.m[2][0] -= tmp[ 1]*src[13] + tmp[ 2]*src[14] + tmp[ 5]*src[15];
	Ainv.m[2][1]  = tmp[ 1]*src[12] + tmp[ 6]*src[14] + tmp[ 9]*src[15];
	Ainv.m[2][1] -= tmp[ 0]*src[12] + tmp[ 7]*src[14] + tmp[ 8]*src[15];
	Ainv.m[2][2]  = tmp[ 2]*src[12] + tmp[ 7]*src[13] + tmp[10]*src[15];
	Ainv.m[2][2] -= tmp[ 3]*src[12] + tmp[ 6]*src[13] + tmp[11]*src[15];
	Ainv.m[2][3]  = tmp[ 5]*src[12] + tmp[ 8]*src[13] + tmp[11]*src[14];
	Ainv.m[2][3] -= tmp[ 4]*src[12] + tmp[ 9]*src[13] + tmp[10]*src[14];
	Ainv.m[3][0]  = tmp[ 2]*src[10] + tmp[ 5]*src[11] + tmp[ 1]*src[ 9];
	Ainv.m[3][0] -= tmp[ 4]*src[11] + tmp[ 0]*src[ 9] + tmp[ 3]*src[10];
	Ainv.m[3][1]  = tmp[ 8]*src[11] + tmp[ 0]*src[ 8] + tmp[ 7]*src[10];
	Ainv.m[3][1] -= tmp[ 6]*src[10] + tmp[ 9]*src[11] + tmp[ 1]*src[ 8];
	Ainv.m[3][2]  = tmp[ 6]*src[ 9] + tmp[11]*src[11] + tmp[ 3]*src[ 8];
	Ainv.m[3][2] -= tmp[10]*src[11] + tmp[ 2]*src[ 8] + tmp[ 7]*src[ 9];
	Ainv.m[3][3]  = tmp[10]*src[10] + tmp[ 4]*src[ 8] + tmp[ 9]*src[ 9];
	Ainv.m[3][3] -= tmp[ 8]*src[ 9] + tmp[11]*src[10] + tmp[ 5]*src[ 8];

	dei = 1.0 / (src[0]*Ainv.m[0][0] +
	             src[1]*Ainv.m[0][1] +
		     src[2]*Ainv.m[0][2] +
	             src[3]*Ainv.m[0][3]);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++)
			Ainv.m[i][j] *= dei;
	}
	return (Ainv);
}

int
SG_MatrixInvertGaussJordan44v_FPU(const SG_Matrix *A, SG_Matrix *Ainv)
{
	SG_Matrix Tmp;
	int m, n, o, swap;
	SG_Real t;

	SG_MatrixCopy44_FPU(&Tmp, A);
	SG_MatrixIdentity44v_FPU(Ainv);
	
	for (n = 0; n < 4; n++) {
		for (m = n+1, swap = n;
		     m < 4;
		     m++) {
			if (Fabs(Tmp.m[m][n]) > Fabs(Tmp.m[n][n]))
				swap = m;
		}
		if (swap != n) {
			for (o = 0; o < 4; o++) {
				t = Tmp.m[n][o];
				Tmp.m[n][o] = Tmp.m[swap][o];
				Ainv->m[swap][o] = t;

				t = Ainv->m[n][o];
				Ainv->m[n][o] = Ainv->m[swap][o];
				Ainv->m[swap][o] = t;
			}
		}
		if (Fabs(Tmp.m[n][n] - 0.0) < SG_MATRIX_TINYVAL) {
			AG_SetError("Matrix singular to %e", SG_MATRIX_TINYVAL);
			return (-1);
		}

		t = Tmp.m[n][n];
		for (o = 0; o < 4; o++) {
			Tmp.m[n][o] /= t;
			Ainv->m[n][o] /= t;
		}
		for (m = 0; m < 4; m++) {
			if (m != n) {
				t = Tmp.m[m][n];
				for (o = 0; o < 4; o++) {
					Tmp.m[m][o] -= Tmp.m[n][o]*t;
					Ainv->m[m][o] -= Ainv->m[n][o]*t;
				}
			}
		}
	}
	return (0);
}

#undef MSWAP
#define MSWAP(M,a1,a2,b1,b2) { \
	tmp=(M)->m[a1][a2]; \
	(M)->m[a1][a2]=(M)->m[b1][b2]; \
	(M)->m[b1][b2]=tmp; \
}
void
SG_MatrixDiagonalSwap44v_FPU(SG_Matrix *M)
{
	SG_Real tmp;

	MSWAP(M, 1,0, 0,1);	MSWAP(M, 2,0, 0,2);	MSWAP(M, 3,0, 0,3);
	MSWAP(M, 0,1, 1,0);	MSWAP(M, 2,1, 1,2);	MSWAP(M, 3,1, 1,3);
	MSWAP(M, 0,2, 2,0);	MSWAP(M, 1,2, 2,1);	MSWAP(M, 3,2, 2,3);
	MSWAP(M, 0,3, 3,0);	MSWAP(M, 1,3, 3,1);	MSWAP(M, 2,3, 3,2);
}
#undef MSWAP

#endif /* HAVE_OPENGL */
