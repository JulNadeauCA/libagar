/*
 * Copyright (c) 2006-2008 Hypertriton, Inc. <http://hypertriton.com/>
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

__BEGIN_DECLS
extern const M_MatrixOps44 mMatOps44_FPU;

M_Matrix44 M_MatrixInvert44_FPU(M_Matrix44);
int        M_MatrixInvertElim44_FPU(M_Matrix44, M_Matrix44 *_Nonnull);
void       M_MatrixCopy44_FPU(M_Matrix44 *_Nonnull, const M_Matrix44 *_Nonnull);

void M_MatrixRotateAxis44_FPU(M_Matrix44 *_Nonnull, M_Real, M_Vector3);
void M_MatrixTranslatev44_FPU(M_Matrix44 *_Nonnull, M_Vector3);
void M_MatrixTranslate44_FPU(M_Matrix44 *_Nonnull, M_Real, M_Real, M_Real);
void M_MatrixOrbitAxis44_FPU(M_Matrix44 *_Nonnull, M_Vector3, M_Vector3, M_Real);
void M_MatrixRotateEul44_FPU(M_Matrix44 *_Nonnull, M_Real, M_Real, M_Real);
void M_MatrixRotate44I_FPU(M_Matrix44 *_Nonnull, M_Real);
void M_MatrixRotate44J_FPU(M_Matrix44 *_Nonnull, M_Real);
void M_MatrixRotate44K_FPU(M_Matrix44 *_Nonnull, M_Real);
void M_MatrixTranslateX44_FPU(M_Matrix44 *_Nonnull, M_Real);
void M_MatrixTranslateY44_FPU(M_Matrix44 *_Nonnull, M_Real);
void M_MatrixTranslateZ44_FPU(M_Matrix44 *_Nonnull, M_Real);
void M_MatrixScale44_FPU(M_Matrix44 *_Nonnull, M_Real, M_Real, M_Real, M_Real);
void M_MatrixUniScale44_FPU(M_Matrix44 *_Nonnull, M_Real);

static __inline__ M_Matrix44
M_MatrixZero44_FPU(void)
{
	M_Matrix44 A;

	A.m[0][0] = 0.0; A.m[0][1] = 0.0; A.m[0][2] = 0.0; A.m[0][3] = 0.0;
	A.m[1][0] = 0.0; A.m[1][1] = 0.0; A.m[1][2] = 0.0; A.m[1][3] = 0.0;
	A.m[2][0] = 0.0; A.m[2][1] = 0.0; A.m[2][2] = 0.0; A.m[2][3] = 0.0;
	A.m[3][0] = 0.0; A.m[3][1] = 0.0; A.m[3][2] = 0.0; A.m[3][3] = 0.0;
	return (A);
}

static __inline__ void
M_MatrixZero44v_FPU(M_Matrix44 *_Nonnull A)
{
	A->m[0][0] = 0.0; A->m[0][1] = 0.0; A->m[0][2] = 0.0; A->m[0][3] = 0.0;
	A->m[1][0] = 0.0; A->m[1][1] = 0.0; A->m[1][2] = 0.0; A->m[1][3] = 0.0;
	A->m[2][0] = 0.0; A->m[2][1] = 0.0; A->m[2][2] = 0.0; A->m[2][3] = 0.0;
	A->m[3][0] = 0.0; A->m[3][1] = 0.0; A->m[3][2] = 0.0; A->m[3][3] = 0.0;
}

static __inline__ M_Matrix44
M_MatrixIdentity44_FPU(void)
{
	M_Matrix44 A;

	A.m[0][0] = 1.0; A.m[0][1] = 0.0; A.m[0][2] = 0.0; A.m[0][3] = 0.0;
	A.m[1][0] = 0.0; A.m[1][1] = 1.0; A.m[1][2] = 0.0; A.m[1][3] = 0.0;
	A.m[2][0] = 0.0; A.m[2][1] = 0.0; A.m[2][2] = 1.0; A.m[2][3] = 0.0;
	A.m[3][0] = 0.0; A.m[3][1] = 0.0; A.m[3][2] = 0.0; A.m[3][3] = 1.0;
	return (A);
}

static __inline__ void
M_MatrixIdentity44v_FPU(M_Matrix44 *_Nonnull A)
{
	A->m[0][0] = 1.0; A->m[0][1] = 0.0; A->m[0][2] = 0.0; A->m[0][3] = 0.0;
	A->m[1][0] = 0.0; A->m[1][1] = 1.0; A->m[1][2] = 0.0; A->m[1][3] = 0.0;
	A->m[2][0] = 0.0; A->m[2][1] = 0.0; A->m[2][2] = 1.0; A->m[2][3] = 0.0;
	A->m[3][0] = 0.0; A->m[3][1] = 0.0; A->m[3][2] = 0.0; A->m[3][3] = 1.0;
}

static __inline__ void
M_MatrixMult44v_FPU(M_Matrix44 *_Nonnull A, const M_Matrix44 *_Nonnull B)
{
	M_Matrix44 Atmp;
	int m, n;

	for (m = 0; m < 4; m++) {
		for (n = 0; n < 4; n++)
			Atmp.m[m][n] = A->m[m][0] * B->m[0][n] +
			               A->m[m][1] * B->m[1][n] +
			               A->m[m][2] * B->m[2][n] +
			               A->m[m][3] * B->m[3][n];
	}
	M_MatrixCopy44_FPU(A, &Atmp);
}

static __inline__ M_Matrix44
M_MatrixMult44_FPU(M_Matrix44 A, M_Matrix44 B)
{
	M_Matrix44 R;
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

static __inline__ M_Vector4
M_MatrixMultVector44_FPU(M_Matrix44 A, M_Vector4 x)
{
	M_Vector4 Ax;

	Ax.x = A.m[0][0]*x.x + A.m[0][1]*x.y + A.m[0][2]*x.z + A.m[0][3]*x.w;
	Ax.y = A.m[1][0]*x.x + A.m[1][1]*x.y + A.m[1][2]*x.z + A.m[1][3]*x.w;
	Ax.z = A.m[2][0]*x.x + A.m[2][1]*x.y + A.m[2][2]*x.z + A.m[2][3]*x.w;
	Ax.w = A.m[3][0]*x.x + A.m[3][1]*x.y + A.m[3][2]*x.z + A.m[3][3]*x.w;
	return (Ax);
}

static __inline__ M_Vector4
M_MatrixMultVector44p_FPU(const M_Matrix44 *_Nonnull A, const M_Vector4 *_Nonnull x)
{
	M_Vector4 Ax;
	
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

static __inline__ void
M_MatrixMultVector44v_FPU(M_Vector4 *_Nonnull x, const M_Matrix44 *_Nonnull A)
{
	M_Real xx = x->x;
	M_Real xy = x->y;
	M_Real xz = x->z;
	M_Real xw = x->w;

	x->x = A->m[0][0]*xx + A->m[0][1]*xy + A->m[0][2]*xz + A->m[0][3]*xw;
	x->y = A->m[1][0]*xx + A->m[1][1]*xy + A->m[1][2]*xz + A->m[1][3]*xw;
	x->z = A->m[2][0]*xx + A->m[2][1]*xy + A->m[2][2]*xz + A->m[2][3]*xw;
	x->w = A->m[3][0]*xx + A->m[3][1]*xy + A->m[3][2]*xz + A->m[3][3]*xw;
}

static __inline__ void
M_MatrixToFloats44_FPU(float *_Nonnull fv, const M_Matrix44 *_Nonnull M)
{
	int m, n;

	for (m = 0; m < 4; m++) {
		for (n = 0; n < 4; n++)
			fv[(m<<2)+n] = (float)M->m[m][n];
	}
}

static __inline__ void
M_MatrixToDoubles44_FPU(double *_Nonnull dv, const M_Matrix44 *_Nonnull M)
{
	int m, n;

	for (m = 0; m < 4; m++) {
		for (n = 0; n < 4; n++)
			dv[(m<<2)+n] = (double)M->m[m][n];
	}
}

static __inline__ void
M_MatrixFromFloats44_FPU(M_Matrix44 *_Nonnull M, const float *_Nonnull fv)
{
	int m, n;

	for (m = 0; m < 4; m++) {
		for (n = 0; n < 4; n++)
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
			M->m[m][n] = fv[(m<<2)+n];
#elif defined(DOUBLE_PRECISION)
			M->m[m][n] = (double)fv[(m<<2)+n];
#endif
	}
}

static __inline__ void
M_MatrixFromDoubles44_FPU(M_Matrix44 *_Nonnull M, const double *_Nonnull fv)
{
	int m, n;

	for (m = 0; m < 4; m++) {
		for (n = 0; n < 4; n++)
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
			M->m[m][n] = (double)fv[(m<<2)+n];
#elif defined(DOUBLE_PRECISION)
			M->m[m][n] = fv[(m<<2)+n];
#endif
	}
}

static __inline__ M_Matrix44
M_MatrixTranspose44_FPU(M_Matrix44 M)
{
	M_Matrix44 T;

	T.m[0][0] = M.m[0][0]; T.m[1][0] = M.m[0][1]; T.m[2][0] = M.m[0][2];
	T.m[3][0] = M.m[0][3];
	T.m[0][1] = M.m[1][0]; T.m[1][1] = M.m[1][1]; T.m[2][1] = M.m[1][2];
	T.m[3][1] = M.m[1][3];
	T.m[0][2] = M.m[2][0]; T.m[1][2] = M.m[2][1]; T.m[2][2] = M.m[2][2];
	T.m[3][2] = M.m[2][3];
	T.m[0][3] = M.m[3][0]; T.m[1][3] = M.m[3][1]; T.m[2][3] = M.m[3][2];
	T.m[3][3] = M.m[3][3];
	return (T);
}

static __inline__ M_Matrix44
M_MatrixTranspose44p_FPU(const M_Matrix44 *_Nonnull M)
{
	M_Matrix44 T;

	T.m[0][0] = M->m[0][0]; T.m[1][0] = M->m[0][1]; T.m[2][0] = M->m[0][2];
	T.m[3][0] = M->m[0][3];
	T.m[0][1] = M->m[1][0]; T.m[1][1] = M->m[1][1]; T.m[2][1] = M->m[1][2];
	T.m[3][1] = M->m[1][3];
	T.m[0][2] = M->m[2][0]; T.m[1][2] = M->m[2][1]; T.m[2][2] = M->m[2][2];
	T.m[3][2] = M->m[2][3];
	T.m[0][3] = M->m[3][0]; T.m[1][3] = M->m[3][1]; T.m[2][3] = M->m[3][2];
	T.m[3][3] = M->m[3][3];
	return (T);
}

static __inline__ void
M_MatrixTranspose44v_FPU(M_Matrix44 *_Nonnull M)
{
	M_Matrix44 T;

	T.m[0][0] = M->m[0][0]; T.m[1][0] = M->m[0][1]; T.m[2][0] = M->m[0][2];
	T.m[3][0] = M->m[0][3];
	T.m[0][1] = M->m[1][0]; T.m[1][1] = M->m[1][1]; T.m[2][1] = M->m[1][2];
	T.m[3][1] = M->m[1][3];
	T.m[0][2] = M->m[2][0]; T.m[1][2] = M->m[2][1]; T.m[2][2] = M->m[2][2];
	T.m[3][2] = M->m[2][3];
	T.m[0][3] = M->m[3][0]; T.m[1][3] = M->m[3][1]; T.m[2][3] = M->m[3][2];
	T.m[3][3] = M->m[3][3];
	M_MatrixCopy44_FPU(M, &T);
}

__END_DECLS
