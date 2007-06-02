/*
 * Copyright (c) 2006-2007 Hypertriton, Inc.
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
#include "sg_bitstring.h"

#include <string.h>

#undef MSWAP
#define MSWAP(M,a1,a2,b1,b2) { tmp=(M)->m[a1][a2]; (M)->m[a1][a2]=(M)->m[b1][b2]; (M)->m[b1][b2]=tmp; }

void
SG_MatrixPrint(SG_Matrix *A)
{
	int m, n;

	for (n = 0; n < 4; n++) {
		for (m = 0; m < 4; m++) { printf("%f ", A->m[m][n]); }
		printf("\n");
	}
}

/* Return the identity matrix I. */
SG_Matrix
SG_MatrixIdentity(void)
{
	SG_Matrix A;

	A.m[0][0] = 1.0; A.m[0][1] = 0.0; A.m[0][2] = 0.0; A.m[0][3] = 0.0;
	A.m[1][0] = 0.0; A.m[1][1] = 1.0; A.m[1][2] = 0.0; A.m[1][3] = 0.0;
	A.m[2][0] = 0.0; A.m[2][1] = 0.0; A.m[2][2] = 1.0; A.m[2][3] = 0.0;
	A.m[3][0] = 0.0; A.m[3][1] = 0.0; A.m[3][2] = 0.0; A.m[3][3] = 1.0;
	return (A);
}

/* Return the identity matrix in A. */
void
SG_MatrixIdentityv(SG_Matrix *A)
{
	A->m[0][0] = 1.0; A->m[0][1] = 0.0; A->m[0][2] = 0.0; A->m[0][3] = 0.0;
	A->m[1][0] = 0.0; A->m[1][1] = 1.0; A->m[1][2] = 0.0; A->m[1][3] = 0.0;
	A->m[2][0] = 0.0; A->m[2][1] = 0.0; A->m[2][2] = 1.0; A->m[2][3] = 0.0;
	A->m[3][0] = 0.0; A->m[3][1] = 0.0; A->m[3][2] = 0.0; A->m[3][3] = 1.0;
}

/* Return the product of A and B into C. */
void
SG_MatrixMultpv(SG_Matrix *C, const SG_Matrix *A, const SG_Matrix *B)
{
	int m, n;

#ifdef DEBUG
	if (C == A) { fatal("use SG_MatrixMultv() if C==A"); }
#endif
	for (m = 0; m < 4; m++) {
		for (n = 0; n < 4; n++)
			C->m[m][n] = A->m[m][0] * B->m[0][n] +
			             A->m[m][1] * B->m[1][n] +
			             A->m[m][2] * B->m[2][n] +
			             A->m[m][3] * B->m[3][n];
	}
}

/* Return the product of A and B into A. */
void
SG_MatrixMultv(SG_Matrix *A, const SG_Matrix *B)
{
	SG_Matrix Atmp;
	int m, n;

	for (m = 0; m < 4; m++) {
		for (n = 0; n < 4; n++)
			Atmp.m[m][n] = A->m[m][0] * B->m[0][n] +
			               A->m[m][1] * B->m[1][n] +
			               A->m[m][2] * B->m[2][n] +
			               A->m[m][3] * B->m[3][n];
	}
	SG_MatrixCopy(A, &Atmp);
}

/* Return the product of A and B. */
SG_Matrix
SG_MatrixMult(SG_Matrix A, SG_Matrix B)
{
	SG_Matrix R;
	int m, n;

	for (m = 0; m < 4; m++) {
		for (n = 0; n < 4; n++)
			R.m[m][n] = A.m[m][0] * B.m[0][n] +
			            A.m[m][1] * B.m[1][n] +
			            A.m[m][2] * B.m[2][n] +
			            A.m[m][3] * B.m[3][n];
	}
	return (R);
}

/* Return the product A and x. */
SG_Vector
SG_MatrixMultVector(SG_Matrix A, SG_Vector x)
{
	SG_Vector Ax;

	Ax.x = A.m[0][0]*x.x + A.m[0][1]*x.y + A.m[0][2]*x.z + A.m[0][3];
	Ax.y = A.m[1][0]*x.x + A.m[1][1]*x.y + A.m[1][2]*x.z + A.m[1][3];
	Ax.z = A.m[2][0]*x.x + A.m[2][1]*x.y + A.m[2][2]*x.z + A.m[2][3];
	return (Ax);
}

/* Return the product of *A and *x. */
SG_Vector
SG_MatrixMultVectorp(const SG_Matrix *A, const SG_Vector *x)
{
	SG_Vector Ax;

	Ax.x = A->m[0][0]*x->x + A->m[0][1]*x->y + A->m[0][2]*x->z + A->m[0][3];
	Ax.y = A->m[1][0]*x->x + A->m[1][1]*x->y + A->m[1][2]*x->z + A->m[1][3];
	Ax.z = A->m[2][0]*x->x + A->m[2][1]*x->y + A->m[2][2]*x->z + A->m[2][3];
	return (Ax);
}

/* Return the product of *A and *x into x. */
void
SG_MatrixMultVectorv(SG_Vector *x, const SG_Matrix *A)
{
	SG_Real xx = x->x;
	SG_Real xy = x->y;
	SG_Real xz = x->z;

	x->x = A->m[0][0]*xx + A->m[0][1]*xy + A->m[0][2]*xz + A->m[0][3];
	x->y = A->m[1][0]*xx + A->m[1][1]*xy + A->m[1][2]*xz + A->m[1][3];
	x->z = A->m[2][0]*xx + A->m[2][1]*xy + A->m[2][2]*xz + A->m[2][3];
}

/* Return the product of A and x. */
SG_Vector4
SG_MatrixMultVector4(SG_Matrix A, SG_Vector4 x)
{
	SG_Vector4 Ax;

	Ax.x = A.m[0][0]*x.x + A.m[0][1]*x.y + A.m[0][2]*x.z + A.m[0][3]*x.w;
	Ax.y = A.m[1][0]*x.x + A.m[1][1]*x.y + A.m[1][2]*x.z + A.m[1][3]*x.w;
	Ax.z = A.m[2][0]*x.x + A.m[2][1]*x.y + A.m[2][2]*x.z + A.m[2][3]*x.w;
	Ax.w = A.m[3][0]*x.x + A.m[3][1]*x.y + A.m[3][2]*x.z + A.m[3][3]*x.w;
	return (Ax);
}

/* Return the product of *A and *x. */
SG_Vector4
SG_MatrixMultVector4p(const SG_Matrix *A, const SG_Vector4 *x)
{
	SG_Vector4 Ax;

	Ax.x = A->m[0][0]*x->x + A->m[0][1]*x->y + A->m[0][2]*x->z +
	       A->m[0][3]*x->w;
	Ax.y = A->m[1][0]*x->x + A->m[1][1]*x->y + A->m[1][2]*x->z +
	       A->m[1][3]*x->w;
	Ax.z = A->m[2][0]*x->x + A->m[2][1]*x->y + A->m[2][2]*x->z +
	       A->m[2][3]*x->w;
	Ax.w = A->m[3][0]*x->x + A->m[3][1]*x->y + A->m[3][2]*x->z +
	       A->m[3][3]*x->w;
	return (Ax);
}

/* Return the product of *A and *x into x. */
void
SG_MatrixMultVector4v(SG_Vector4 *x, const SG_Matrix *A)
{
	SG_Real xx = x->x;
	SG_Real xy = x->y;
	SG_Real xz = x->z;
	SG_Real xw = x->w;

	x->x = A->m[0][0]*xx + A->m[0][1]*xy + A->m[0][2]*xz + A->m[0][3]*xw;
	x->y = A->m[1][0]*xx + A->m[1][1]*xy + A->m[1][2]*xz + A->m[1][3]*xw;
	x->z = A->m[2][0]*xx + A->m[2][1]*xy + A->m[2][2]*xz + A->m[2][3]*xw;
	x->w = A->m[3][0]*xx + A->m[3][1]*xy + A->m[3][2]*xz + A->m[3][3]*xw;
}

/* Return the product of *A and *x into x[4]. */
void
SG_MatrixMultVector4fv(float *x, const SG_Matrix *A)
{
	SG_Real xx = x[0];
	SG_Real xy = x[1];
	SG_Real xz = x[2];
	SG_Real xw = x[3];

	x[0] = A->m[0][0]*xx + A->m[0][1]*xy + A->m[0][2]*xz + A->m[0][3]*xw;
	x[1] = A->m[1][0]*xx + A->m[1][1]*xy + A->m[1][2]*xz + A->m[1][3]*xw;
	x[2] = A->m[2][0]*xx + A->m[2][1]*xy + A->m[2][2]*xz + A->m[2][3]*xw;
	x[3] = A->m[3][0]*xx + A->m[3][1]*xy + A->m[3][2]*xz + A->m[3][3]*xw;
}

/* Copy matrix mSrc to mDst. */
void
SG_MatrixCopy(SG_Matrix *mDst, const SG_Matrix *mSrc)
{
	memcpy(&mDst->m[0][0], &mSrc->m[0][0], 16*sizeof(SG_Real));
}

/* Return 16-float form of matrix M into fv. */
void
SG_MatrixToFloats(float *fv, const SG_Matrix *M)
{
#ifdef SG_DOUBLE_PRECISION
	int m, n;
	for (m = 0; m < 4; m++) {
		for (n = 0; n < 4; n++)
			fv[(m<<2)+n] = (float)M->m[m][n];
	}
#else
	memcpy(fv, &M->m[0][0], 16*sizeof(float));
#endif
}

/* Return 16-double form of matrix M into fv. */
void
SG_MatrixToDoubles(double *dv, const SG_Matrix *M)
{
#ifdef SG_DOUBLE_PRECISION
	memcpy(dv, &M->m[0][0], 16*sizeof(double));
#else
	int m, n;
	for (m = 0; m < 4; m++) {
		for (n = 0; n < 4; n++)
			dv[(m<<2)+n] = (double)M->m[m][n];
	}
#endif
}

/*
 * Return the direction vectors for rotation/translation/shearing matrix
 * M into x,y,z.
 */
void
SG_MatrixDirection(const SG_Matrix *M, SG_Vector *x, SG_Vector *y, SG_Vector *z)
{
	if (x != NULL) {
		x->x = M->m[0][0];
		x->y = M->m[1][0];
		x->z = M->m[2][0];
	}
	if (y != NULL) {
		y->x = M->m[0][1];
		y->y = M->m[1][1];
		y->z = M->m[2][1];
	}
	if (z != NULL) {
		z->x = M->m[0][2];
		z->y = M->m[1][2];
		z->z = M->m[2][2];
	}
}

#if 0
int
SG_MatrixInvertCramerSIMD(const SG_Matrix *A, SG_Matrix *Ainv)
{
	__m128 minor0, minor1, minor2, minor3;
	__m128 row0, row1, row2, row3;
	__m128 det, tmp1;

	tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src)),
	                                       (__m64*)(src+4));
	row1 = _mm_loadh_pi(_mm_loadl_pi(row1, (__m64*)(src+8)),
	                                       (__m64*)(src+12));
	row0 = _mm_shuffle_ps(tmp1, row1, 0x88);
	row1 = _mm_shuffle_ps(row1, tmp1, 0xdd);
	
	tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src+2)),
	                                       (__m64*)(src+6));
	row3 = _mm_loadh_pi(_mm_loadl_pi(row3, (__m64*)(src+10)),
	                                       (__m64*)(src+14));
	
	row2 = _mm_shuffle_ps(tmp1, row3, 0x88);
	row3 = _mm_shuffle_ps(row3, tmp1, 0xdd);

	tmp1 = _mm_mul_ps(row2, row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xb1);

	...
}
#endif

/*
 * Invert a 4x4 matrix using Cramer's rule. Assume that the matrix
 * is non-singular up to machine precision, or division by zero will
 * occur.
 */
SG_Matrix
SG_MatrixInvertCramerp(const SG_Matrix *A)
{
	SG_Matrix Ainv;
	SG_Real tmp[12];
	SG_Real src[16];
	SG_Real det;
	int i, j;

	/* Transpose matrix into flat array */
	for (i = 0; i < 4; i++) {
		src[i]		= A->m[i][0];
		src[i+4]	= A->m[i][1];
		src[i+8]	= A->m[i][2];
		src[i+12]	= A->m[i][3];
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

	/* Compute determinant */
	det = src[0]*Ainv.m[0][0] +
	      src[1]*Ainv.m[0][1] +
	      src[2]*Ainv.m[0][2] +
	      src[3]*Ainv.m[0][3];

	/* Compute inverse */
	det = 1.0/det;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++)
			Ainv.m[i][j] *= det;
	}
	return (Ainv);
}

/* Return the inverse of matrix A into Ainv. */
int
SG_MatrixInvert(const SG_Matrix *A, SG_Matrix *Ainv)
{
	SG_Matrix Tmp;
	int m, n, o, swap;
	SG_Real t;

	SG_MatrixCopy(&Tmp, A);
	SG_MatrixIdentityv(Ainv);
	
	for (n = 0; n < 4; n++) {
		for (m = n+1, swap = n;
		     m < 4;
		     m++) {
			if (SG_Fabs(Tmp.m[m][n]) > SG_Fabs(Tmp.m[n][n]))
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
#if 1
		if (SG_Fabs(Tmp.m[n][n] - 0.0) < 1e-12) { /* XXX arbitrary */
			AG_SetError("Matrix singular to 1e-12");
			return (-1);
		}
#endif

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

SG_Matrix
SG_MatrixTranspose(SG_Matrix M)
{
	SG_Matrix T;

	T.m[0][0] = M.m[0][0];
	T.m[1][0] = M.m[0][1];
	T.m[2][0] = M.m[0][2];
	T.m[3][0] = M.m[0][3];
	T.m[0][1] = M.m[1][0];
	T.m[1][1] = M.m[1][1];
	T.m[2][1] = M.m[1][2];
	T.m[3][1] = M.m[1][3];
	T.m[0][2] = M.m[2][0];
	T.m[1][2] = M.m[2][1];
	T.m[2][2] = M.m[2][2];
	T.m[3][2] = M.m[2][3];
	T.m[0][3] = M.m[3][0];
	T.m[1][3] = M.m[3][1];
	T.m[2][3] = M.m[3][2];
	T.m[3][3] = M.m[3][3];
	return (T);
}

SG_Matrix
SG_MatrixTransposep(const SG_Matrix *M)
{
	SG_Matrix T;

	T.m[0][0] = M->m[0][0];
	T.m[1][0] = M->m[0][1];
	T.m[2][0] = M->m[0][2];
	T.m[3][0] = M->m[0][3];
	T.m[0][1] = M->m[1][0];
	T.m[1][1] = M->m[1][1];
	T.m[2][1] = M->m[1][2];
	T.m[3][1] = M->m[1][3];
	T.m[0][2] = M->m[2][0];
	T.m[1][2] = M->m[2][1];
	T.m[2][2] = M->m[2][2];
	T.m[3][2] = M->m[2][3];
	T.m[0][3] = M->m[3][0];
	T.m[1][3] = M->m[3][1];
	T.m[2][3] = M->m[3][2];
	T.m[3][3] = M->m[3][3];
	return (T);
}

void
SG_MatrixTransposev(SG_Matrix *M)
{
	SG_Matrix T;

	T.m[0][0] = M->m[0][0];
	T.m[1][0] = M->m[0][1];
	T.m[2][0] = M->m[0][2];
	T.m[3][0] = M->m[0][3];
	T.m[0][1] = M->m[1][0];
	T.m[1][1] = M->m[1][1];
	T.m[2][1] = M->m[1][2];
	T.m[3][1] = M->m[1][3];
	T.m[0][2] = M->m[2][0];
	T.m[1][2] = M->m[2][1];
	T.m[2][2] = M->m[2][2];
	T.m[3][2] = M->m[2][3];
	T.m[0][3] = M->m[3][0];
	T.m[1][3] = M->m[3][1];
	T.m[2][3] = M->m[3][2];
	T.m[3][3] = M->m[3][3];
	SG_MatrixCopy(M, &T);
}

void
SG_MatrixDiagonalSwap(SG_Matrix *M)
{
	SG_Real tmp;

//	MSWAP(M, 0,0, 0,0);
	MSWAP(M, 1,0, 0,1);
	MSWAP(M, 2,0, 0,2);
	MSWAP(M, 3,0, 0,3);
	
	MSWAP(M, 0,1, 1,0);
//	MSWAP(M, 1,1, 1,1);
	MSWAP(M, 2,1, 1,2);
	MSWAP(M, 3,1, 1,3);

	MSWAP(M, 0,2, 2,0);
	MSWAP(M, 1,2, 2,1);
//	MSWAP(M, 2,2, 2,2);
	MSWAP(M, 3,2, 2,3);

	MSWAP(M, 0,3, 3,0);
	MSWAP(M, 1,3, 3,1);
	MSWAP(M, 2,3, 3,2);
//	MSWAP(M, 3,3, 3,3);
}

/* Extract the roll, pitch and yaw angles from a rotation matrix R. */
void
SG_MatrixGetRotationXYZ(const SG_Matrix *R, SG_Real *pitch, SG_Real *yaw,
    SG_Real *roll)
{
	*roll = SG_Atan2(R->m[1][0], R->m[0][0]);
	*pitch = -SG_Asin(R->m[2][0]);
	*yaw = SG_Atan2(R->m[2][1], R->m[2][2]);
}

/* Extract a 3-dimensional translation vector t from translation matrix T. */
void
SG_MatrixGetTranslation(const SG_Matrix *T, SG_Vector *t)
{
	t->x = T->m[0][3];
	t->y = T->m[1][3];
	t->z = T->m[2][3];
}

/* Generate a rotation of theta radians around arbitrary axis A. */
void
SG_MatrixRotatev(SG_Matrix *M, SG_Real theta, SG_Vector A)
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
	SG_MatrixMultv(M, &R);
}

void
SG_MatrixRotateEul(SG_Matrix *M, SG_Real pitch, SG_Real roll, SG_Real yaw)
{
	SG_Matrix4 R;

	R.m[0][0] = SG_Cos(yaw)*SG_Cos(roll) +
	            SG_Sin(yaw)*SG_Sin(pitch)*SG_Sin(roll);
	R.m[0][1] = SG_Sin(yaw)*SG_Cos(roll) -
	            SG_Cos(yaw)*SG_Sin(pitch)*SG_Sin(roll);
	R.m[0][2] = SG_Cos(pitch)*SG_Sin(roll);
	R.m[0][3] = 0.0;
	R.m[1][0] = -SG_Sin(yaw)*SG_Cos(pitch);
	R.m[1][1] = SG_Cos(yaw)*SG_Cos(pitch);
	R.m[1][2] = SG_Sin(pitch);
	R.m[1][3] = 0.0;
	R.m[2][0] = SG_Sin(yaw)*SG_Sin(pitch)*SG_Cos(roll) -
	            SG_Cos(yaw)*SG_Sin(roll);
	R.m[2][1] = -SG_Cos(yaw)*SG_Sin(pitch)*SG_Cos(roll) -
	            SG_Sin(yaw)*SG_Sin(roll);
	R.m[2][2] = SG_Cos(pitch)*SG_Cos(roll);
	R.m[2][3] = 0.0;
	R.m[3][0] = 0.0;
	R.m[3][1] = 0.0;
	R.m[3][2] = 0.0;
	R.m[3][3] = 1.0;
	SG_MatrixMultv(M, &R);
}

/* Generate a rotation of (theta) radians around i (pitch). */
void
SG_MatrixRotateXv(SG_Matrix *M, SG_Real theta)
{
	SG_Real s = SG_Sin(theta);
	SG_Real c = SG_Cos(theta);
	SG_Matrix R;

	R.m[0][0] = 1.0; R.m[0][1] = 0.0; R.m[0][2] = 0.0; R.m[0][3] = 0.0;
	R.m[1][0] = 0.0; R.m[1][1] = c;   R.m[1][2] = -s;  R.m[1][3] = 0.0;
	R.m[2][0] = 0.0; R.m[2][1] = s;   R.m[2][2] = c;   R.m[2][3] = 0.0;
	R.m[3][0] = 0.0; R.m[3][1] = 0.0; R.m[3][2] = 0.0; R.m[3][3] = 1.0;
	SG_MatrixMultv(M, &R);
}

/* Generate a rotation of (theta) radians around j (yaw). */
void
SG_MatrixRotateYv(SG_Matrix *M, SG_Real theta)
{
	SG_Real s = SG_Sin(theta);
	SG_Real c = SG_Cos(theta);
	SG_Matrix R;

	R.m[0][0] = c;   R.m[0][1] = 0.0; R.m[0][2] = s;   R.m[0][3] = 0.0;
	R.m[1][0] = 0.0; R.m[1][1] = 1.0; R.m[1][2] = 0.0; R.m[1][3] = 0.0;
	R.m[2][0] = -s;  R.m[2][1] = 0.0; R.m[2][2] = c;   R.m[2][3] = 0.0;
	R.m[3][0] = 0.0; R.m[3][1] = 0.0; R.m[3][2] = 0.0; R.m[3][3] = 1.0;
	SG_MatrixMultv(M, &R);
}

/* Generate a rotation of (theta) radians around k (roll). */
void
SG_MatrixRotateZv(SG_Matrix *M, SG_Real theta)
{
	SG_Real s = SG_Sin(theta);
	SG_Real c = SG_Cos(theta);
	SG_Matrix R;

	R.m[0][0] = c;   R.m[0][1] = -s;  R.m[0][2] = 0.0; R.m[0][3] = 0.0;
	R.m[1][0] = s;   R.m[1][1] = c;   R.m[1][2] = 0.0; R.m[1][3] = 0.0;
	R.m[2][0] = 0.0; R.m[2][1] = 0.0; R.m[2][2] = 1.0; R.m[2][3] = 0.0;
	R.m[3][0] = 0.0; R.m[3][1] = 0.0; R.m[3][2] = 0.0; R.m[3][3] = 1.0;
	SG_MatrixMultv(M, &R);
}

void
SG_MatrixTranslatev(SG_Matrix *M, SG_Vector v)
{
	SG_Matrix T;

	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = v.x;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = v.y;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = v.z;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	SG_MatrixMultv(M, &T);
}

void
SG_MatrixTranslate2(SG_Matrix *M, SG_Real x, SG_Real y)
{
	SG_Matrix T;

	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = x;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = y;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = 0.0;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	SG_MatrixMultv(M, &T);
}

void
SG_MatrixTranslate3(SG_Matrix *M, SG_Real x, SG_Real y, SG_Real z)
{
	SG_Matrix T;

	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = x;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = y;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = z;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	SG_MatrixMultv(M, &T);
}

void
SG_MatrixTranslateX(SG_Matrix *M, SG_Real x)
{
	SG_Matrix T;

	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = x;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = 0.0;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = 0.0;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	SG_MatrixMultv(M, &T);
}

void
SG_MatrixTranslateY(SG_Matrix *M, SG_Real y)
{
	SG_Matrix T;

	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = 0.0;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = y;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = 0.0;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	SG_MatrixMultv(M, &T);
}

void
SG_MatrixTranslateZ(SG_Matrix *M, SG_Real z)
{
	SG_Matrix T;

	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = 0.0;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = 0.0;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = z;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	SG_MatrixMultv(M, &T);
}

void
SG_MatrixScalev(SG_Matrix *M, SG_Vector v)
{
	SG_Matrix S;

	S.m[0][0] = v.x; S.m[0][1] = 0.0; S.m[0][2] = 0.0; S.m[0][3] = 0.0;
	S.m[1][0] = 0.0; S.m[1][1] = v.y; S.m[1][2] = 0.0; S.m[1][3] = 0.0;
	S.m[2][0] = 0.0; S.m[2][1] = 0.0; S.m[2][2] = v.z; S.m[2][3] = 0.0;
	S.m[3][0] = 0.0; S.m[3][1] = 0.0; S.m[3][2] = 0.0; S.m[3][3] = 1.0;
	SG_MatrixMultv(M, &S);
}

void
SG_MatrixScale2(SG_Matrix *M, SG_Real x, SG_Real y)
{
	SG_Matrix S;

	S.m[0][0] = x;   S.m[0][1] = 0.0; S.m[0][2] = 0.0; S.m[0][3] = 0.0;
	S.m[1][0] = 0.0; S.m[1][1] = y;   S.m[1][2] = 0.0; S.m[1][3] = 0.0;
	S.m[2][0] = 0.0; S.m[2][1] = 0.0; S.m[2][2] = 0.0; S.m[2][3] = 0.0;
	S.m[3][0] = 0.0; S.m[3][1] = 0.0; S.m[3][2] = 0.0; S.m[3][3] = 1.0;
	SG_MatrixMultv(M, &S);
}

void
SG_MatrixUniScale(SG_Matrix *M, SG_Real r)
{
	SG_Matrix S;

	S.m[0][0] = r;   S.m[0][1] = 0.0; S.m[0][2] = 0.0; S.m[0][3] = 0.0;
	S.m[1][0] = 0.0; S.m[1][1] = r;   S.m[1][2] = 0.0; S.m[1][3] = 0.0;
	S.m[2][0] = 0.0; S.m[2][1] = 0.0; S.m[2][2] = r;   S.m[2][3] = 0.0;
	S.m[3][0] = 0.0; S.m[3][1] = 0.0; S.m[3][2] = 0.0; S.m[3][3] = 1.0;
	SG_MatrixMultv(M, &S);
}

SG_Matrix
SG_ReadMatrix(AG_Netbuf *buf)
{
	SG_Matrix A;
	SG_ReadMatrixv(buf, &A);
	return (A);
}

void
SG_ReadMatrixv(AG_Netbuf *buf, SG_Matrix *A)
{
	SG_Real *pm = &A->m[0][0];
	int i;
#if 0
	bitstr_t bit_decl(map0, 16);
	bitstr_t bit_decl(map1, 16);

	AG_NetbufRead(&map0, sizeof(map0), 1, buf);
	AG_NetbufRead(&map1, sizeof(map1), 1, buf);

	for (i = 0; i < 16; i++) {
		if (bit_test(map0, i)) {
			*pm = 0.0;
		} else if (bit_test(map1, i)) {
			*pm = 1.0;
		} else {
			*pm = (SG_Real)AG_ReadDouble(buf);
		}
		pm++;
	}
#else
	for (i = 0; i < 16; i++) {
		*pm = (SG_Real)AG_ReadDouble(buf);
		pm++;
	}
#endif
}

void
SG_WriteMatrix(AG_Netbuf *buf, SG_Matrix *A)
{
	SG_Real *pm = &A->m[0][0];
	int i;
#if 0
	bitstr_t bit_decl(map0, 16);
	bitstr_t bit_decl(map1, 16);
	off_t offs;

	offs = AG_NetbufTell(buf);
	AG_NetbufSeek(buf, 4, SEEK_CUR);
	for (i = 0; i < 16; i++) {
		if (*pm == 0.0) {
			bit_set(map0, i);
			bit_clear(map1, i);
		} else if (*pm == 1.0) {
			bit_clear(map0, i);
			bit_set(map1, i);
		} else {
			bit_clear(map0, i);
			bit_clear(map1, i);
			AG_WriteDouble(buf, (SG_Real)(*pm));
		}
		pm++;
	}
	AG_NetbufPwrite(&map0, sizeof(map0), 1, offs, buf);
	AG_NetbufPwrite(&map1, sizeof(map1), 1, offs+2, buf);
#else
	for (i = 0; i < 16; i++) {
		AG_WriteDouble(buf, (double)(*pm));
		pm++;
	}
#endif
}

void
SG_LoadMatrixGL(const SG_Matrix *M)
{
	float f[4][4];

	f[0][0] = M->m[0][0];
	f[1][0] = M->m[0][1];
	f[2][0] = M->m[0][2];
	f[3][0] = M->m[0][3];
	f[0][1] = M->m[1][0];
	f[1][1] = M->m[1][1];
	f[2][1] = M->m[1][2];
	f[3][1] = M->m[1][3];
	f[0][2] = M->m[2][0];
	f[1][2] = M->m[2][1];
	f[2][2] = M->m[2][2];
	f[3][2] = M->m[2][3];
	f[0][3] = M->m[3][0];
	f[1][3] = M->m[3][1];
	f[2][3] = M->m[3][2];
	f[3][3] = M->m[3][3];
	glLoadMatrixf(&f[0][0]);
}

void
SG_GetMatrixGL(int which, SG_Matrix *M)
{
	float f[4][4];

	glGetFloatv((GLenum)which, &f[0][0]);
	M->m[0][0] = f[0][0];
	M->m[1][0] = f[0][1];
	M->m[2][0] = f[0][2];
	M->m[3][0] = f[0][3];
	M->m[0][1] = f[1][0];
	M->m[1][1] = f[1][1];
	M->m[2][1] = f[1][2];
	M->m[3][1] = f[1][3];
	M->m[0][2] = f[2][0];
	M->m[1][2] = f[2][1];
	M->m[2][2] = f[2][2];
	M->m[3][2] = f[2][3];
	M->m[0][3] = f[3][0];
	M->m[1][3] = f[3][1];
	M->m[2][3] = f[3][2];
	M->m[3][3] = f[3][3];
}

#endif /* HAVE_OPENGL */
