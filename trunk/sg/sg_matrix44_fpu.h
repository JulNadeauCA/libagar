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

static __inline__ SG_Matrix
SG_MatrixZero44_FPU(void)
{
	SG_Matrix A;

	A.m[0][0] = 0.0; A.m[0][1] = 0.0; A.m[0][2] = 0.0; A.m[0][3] = 0.0;
	A.m[1][0] = 0.0; A.m[1][1] = 0.0; A.m[1][2] = 0.0; A.m[1][3] = 0.0;
	A.m[2][0] = 0.0; A.m[2][1] = 0.0; A.m[2][2] = 0.0; A.m[2][3] = 0.0;
	A.m[3][0] = 0.0; A.m[3][1] = 0.0; A.m[3][2] = 0.0; A.m[3][3] = 0.0;
	return (A);
}

static __inline__ void
SG_MatrixZero44v_FPU(SG_Matrix *A)
{
	A->m[0][0] = 0.0; A->m[0][1] = 0.0; A->m[0][2] = 0.0; A->m[0][3] = 0.0;
	A->m[1][0] = 0.0; A->m[1][1] = 0.0; A->m[1][2] = 0.0; A->m[1][3] = 0.0;
	A->m[2][0] = 0.0; A->m[2][1] = 0.0; A->m[2][2] = 0.0; A->m[2][3] = 0.0;
	A->m[3][0] = 0.0; A->m[3][1] = 0.0; A->m[3][2] = 0.0; A->m[3][3] = 0.0;
}

static __inline__ SG_Matrix
SG_MatrixIdentity44_FPU(void)
{
	SG_Matrix A;

	A.m[0][0] = 1.0; A.m[0][1] = 0.0; A.m[0][2] = 0.0; A.m[0][3] = 0.0;
	A.m[1][0] = 0.0; A.m[1][1] = 1.0; A.m[1][2] = 0.0; A.m[1][3] = 0.0;
	A.m[2][0] = 0.0; A.m[2][1] = 0.0; A.m[2][2] = 1.0; A.m[2][3] = 0.0;
	A.m[3][0] = 0.0; A.m[3][1] = 0.0; A.m[3][2] = 0.0; A.m[3][3] = 1.0;
	return (A);
}

static __inline__ void
SG_MatrixIdentity44v_FPU(SG_Matrix *A)
{
	A->m[0][0] = 1.0; A->m[0][1] = 0.0; A->m[0][2] = 0.0; A->m[0][3] = 0.0;
	A->m[1][0] = 0.0; A->m[1][1] = 1.0; A->m[1][2] = 0.0; A->m[1][3] = 0.0;
	A->m[2][0] = 0.0; A->m[2][1] = 0.0; A->m[2][2] = 1.0; A->m[2][3] = 0.0;
	A->m[3][0] = 0.0; A->m[3][1] = 0.0; A->m[3][2] = 0.0; A->m[3][3] = 1.0;
}

static __inline__ void
SG_MatrixMult44pv_FPU(SG_Matrix *C, const SG_Matrix *A, const SG_Matrix *B)
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

static __inline__ void
SG_MatrixCopy44_FPU(SG_Matrix *mDst, const SG_Matrix *mSrc)
{
	/* XXX Optimize */
	memcpy(&mDst->m[0][0], &mSrc->m[0][0], 16*sizeof(SG_Real));
}

static __inline__ void
SG_MatrixMult44v_FPU(SG_Matrix *A, const SG_Matrix *B)
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
	SG_MatrixCopy44_FPU(A, &Atmp);
}

static __inline__ SG_Matrix
SG_MatrixMult44_FPU(SG_Matrix A, SG_Matrix B)
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

static __inline__ SG_Vector
SG_MatrixMultVector44_FPU(SG_Matrix A, SG_Vector x)
{
	SG_Vector Ax;

	Ax.x = A.m[0][0]*x.x + A.m[0][1]*x.y + A.m[0][2]*x.z + A.m[0][3];
	Ax.y = A.m[1][0]*x.x + A.m[1][1]*x.y + A.m[1][2]*x.z + A.m[1][3];
	Ax.z = A.m[2][0]*x.x + A.m[2][1]*x.y + A.m[2][2]*x.z + A.m[2][3];
	return (Ax);
}

static __inline__ SG_Vector
SG_MatrixMultVector44p_FPU(const SG_Matrix *A, const SG_Vector *x)
{
	SG_Vector Ax;

	Ax.x = A->m[0][0]*x->x + A->m[0][1]*x->y + A->m[0][2]*x->z + A->m[0][3];
	Ax.y = A->m[1][0]*x->x + A->m[1][1]*x->y + A->m[1][2]*x->z + A->m[1][3];
	Ax.z = A->m[2][0]*x->x + A->m[2][1]*x->y + A->m[2][2]*x->z + A->m[2][3];
	return (Ax);
}

static __inline__ void
SG_MatrixMultVector44v_FPU(SG_Vector *x, const SG_Matrix *A)
{
	SG_Real xx = x->x;
	SG_Real xy = x->y;
	SG_Real xz = x->z;
	
	x->x = A->m[0][0]*xx + A->m[0][1]*xy + A->m[0][2]*xz + A->m[0][3];
	x->y = A->m[1][0]*xx + A->m[1][1]*xy + A->m[1][2]*xz + A->m[1][3];
	x->z = A->m[2][0]*xx + A->m[2][1]*xy + A->m[2][2]*xz + A->m[2][3];
}

static __inline__ SG_Vector4
SG_MatrixMultVector444_FPU(SG_Matrix A, SG_Vector4 x)
{
	SG_Vector4 Ax;

	Ax.x = A.m[0][0]*x.x + A.m[0][1]*x.y + A.m[0][2]*x.z + A.m[0][3]*x.w;
	Ax.y = A.m[1][0]*x.x + A.m[1][1]*x.y + A.m[1][2]*x.z + A.m[1][3]*x.w;
	Ax.z = A.m[2][0]*x.x + A.m[2][1]*x.y + A.m[2][2]*x.z + A.m[2][3]*x.w;
	Ax.w = A.m[3][0]*x.x + A.m[3][1]*x.y + A.m[3][2]*x.z + A.m[3][3]*x.w;
	return (Ax);
}

static __inline__ SG_Vector4
SG_MatrixMultVector444p_FPU(const SG_Matrix *A, const SG_Vector4 *x)
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

static __inline__ void
SG_MatrixMultVector444v_FPU(SG_Vector4 *x, const SG_Matrix *A)
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

static __inline__ void
SG_MatrixToFloats44_FPU(float *fv, const SG_Matrix *M)
{
	/* XXX Optimize */
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

static __inline__ void
SG_MatrixToDoubles44_FPU(double *dv, const SG_Matrix *M)
{
	/* XXX Optimize */
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

static __inline__ void
SG_MatrixFromFloats44_FPU(SG_Matrix *M, const float *fv)
{
	/* XXX Optimize */
#ifdef SG_DOUBLE_PRECISION
	int m, n;
	for (m = 0; m < 4; m++) {
		for (n = 0; n < 4; n++)
			M->m[m][n] = (double)fv[(m<<2)+n];
	}
#else
	memcpy(&M->m[0][0], fv, 16*sizeof(float));
#endif
}

static __inline__ void
SG_MatrixFromDoubles44_FPU(SG_Matrix *M, const double *fv)
{
	/* XXX Optimize */
#ifdef SG_DOUBLE_PRECISION
	memcpy(&M->m[0][0], fv, 16*sizeof(double));
#else
	int m, n;
	for (m = 0; m < 4; m++) {
		for (n = 0; n < 4; n++)
			M->m[m][n] = (float)fv[(m<<2)+n];
	}
#endif
}

static __inline__ void
SG_MatrixGetDirection44_FPU(const SG_Matrix *A, SG_Vector *x, SG_Vector *y,
    SG_Vector *z)
{
	if (x != NULL) { x->x = A->m[0][0]; x->y = A->m[1][0]; x->z = A->m[2][0]; }
	if (y != NULL) { y->x = A->m[0][1]; y->y = A->m[1][1]; y->z = A->m[2][1]; }
	if (z != NULL) { z->x = A->m[0][2]; z->y = A->m[1][2]; z->z = A->m[2][2]; }
}

static __inline__ SG_Matrix
SG_MatrixTranspose44_FPU(SG_Matrix M)
{
	SG_Matrix T;

	T.m[0][0] = M.m[0][0]; T.m[1][0] = M.m[0][1]; T.m[2][0] = M.m[0][2]; T.m[3][0] = M.m[0][3];
	T.m[0][1] = M.m[1][0]; T.m[1][1] = M.m[1][1]; T.m[2][1] = M.m[1][2]; T.m[3][1] = M.m[1][3];
	T.m[0][2] = M.m[2][0]; T.m[1][2] = M.m[2][1]; T.m[2][2] = M.m[2][2]; T.m[3][2] = M.m[2][3];
	T.m[0][3] = M.m[3][0]; T.m[1][3] = M.m[3][1]; T.m[2][3] = M.m[3][2]; T.m[3][3] = M.m[3][3];
	return (T);
}

static __inline__ SG_Matrix
SG_MatrixTranspose44p_FPU(const SG_Matrix *M)
{
	SG_Matrix T;

	T.m[0][0] = M->m[0][0]; T.m[1][0] = M->m[0][1]; T.m[2][0] = M->m[0][2]; T.m[3][0] = M->m[0][3];
	T.m[0][1] = M->m[1][0]; T.m[1][1] = M->m[1][1]; T.m[2][1] = M->m[1][2]; T.m[3][1] = M->m[1][3];
	T.m[0][2] = M->m[2][0]; T.m[1][2] = M->m[2][1]; T.m[2][2] = M->m[2][2]; T.m[3][2] = M->m[2][3];
	T.m[0][3] = M->m[3][0]; T.m[1][3] = M->m[3][1]; T.m[2][3] = M->m[3][2]; T.m[3][3] = M->m[3][3];
	return (T);
}

static __inline__ void
SG_MatrixTranspose44v_FPU(SG_Matrix *M)
{
	SG_Matrix T;

	T.m[0][0] = M->m[0][0]; T.m[1][0] = M->m[0][1]; T.m[2][0] = M->m[0][2]; T.m[3][0] = M->m[0][3];
	T.m[0][1] = M->m[1][0]; T.m[1][1] = M->m[1][1]; T.m[2][1] = M->m[1][2]; T.m[3][1] = M->m[1][3];
	T.m[0][2] = M->m[2][0]; T.m[1][2] = M->m[2][1]; T.m[2][2] = M->m[2][2]; T.m[3][2] = M->m[2][3];
	T.m[0][3] = M->m[3][0]; T.m[1][3] = M->m[3][1]; T.m[2][3] = M->m[3][2]; T.m[3][3] = M->m[3][3];
	SG_MatrixCopy44_FPU(M, &T);
}

static __inline__ void
SG_MatrixRotateAxis44_FPU(SG_Matrix *M, SG_Real theta, SG_Vector A)
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
	SG_MatrixMult44v_FPU(M, &R);
}

static __inline__ void
SG_MatrixTranslate44_FPU(SG_Matrix *M, SG_Vector v)
{
	SG_Matrix T;

	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = v.x;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = v.y;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = v.z;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	SG_MatrixMult44v_FPU(M, &T);
}

static __inline__ void
SG_MatrixTranslate344_FPU(SG_Matrix *M, SG_Real x, SG_Real y, SG_Real z)
{
	SG_Matrix T;

	/* XXX Optimize! */
	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = x;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = y;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = z;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	SG_MatrixMult44v_FPU(M, &T);
}

static __inline__ void
SG_MatrixOrbitAxis44_FPU(SG_Matrix *M, SG_Vector p, SG_Vector A, SG_Real theta)
{
	SG_Matrix R;
	SG_Real s = SG_Sin(theta);
	SG_Real c = SG_Cos(theta);
	SG_Real t = 1.0 - c;

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

	/* XXX Optimize! */
	SG_MatrixTranslate44_FPU(M, p);
	SG_MatrixMult44v_FPU(M, &R);
	SG_MatrixTranslate344_FPU(M, -p.x, -p.y, -p.z);
}

static __inline__ void
SG_MatrixRotateEul44_FPU(SG_Matrix *M, SG_Real pitch, SG_Real roll, SG_Real yaw)
{
	SG_Matrix R;

	R.m[0][0] =  SG_Cos(yaw)*SG_Cos(roll) +
	         SG_Sin(yaw)*SG_Sin(pitch)*SG_Sin(roll);
	R.m[0][1] =  SG_Sin(yaw)*SG_Cos(roll) -
	         SG_Cos(yaw)*SG_Sin(pitch)*SG_Sin(roll);
	R.m[0][2] =  SG_Cos(pitch)*SG_Sin(roll);
	R.m[0][3] =  0.0;
	R.m[1][0] = -SG_Sin(yaw)*SG_Cos(pitch);
	R.m[1][1] =  SG_Cos(yaw)*SG_Cos(pitch);
	R.m[1][2] =  SG_Sin(pitch);
	R.m[1][3] =  0.0;
	R.m[2][0] =  SG_Sin(yaw)*SG_Sin(pitch)*SG_Cos(roll) -
	         SG_Cos(yaw)*SG_Sin(roll);
	R.m[2][1] = -SG_Cos(yaw)*SG_Sin(pitch)*SG_Cos(roll) -
	         SG_Sin(yaw)*SG_Sin(roll);
	R.m[2][2] =  SG_Cos(pitch)*SG_Cos(roll);
	R.m[2][3] =  0.0;
	R.m[3][0] =  0.0;
	R.m[3][1] =  0.0;
	R.m[3][2] =  0.0;
	R.m[3][3] =  1.0;
	SG_MatrixMult44v_FPU(M, &R);
}

static __inline__ void
SG_MatrixRotateI44_FPU(SG_Matrix *M, SG_Real theta)
{
	SG_Real s = SG_Sin(theta);
	SG_Real c = SG_Cos(theta);
	SG_Matrix R;

	/* XXX Optimize! */
	R.m[0][0] = 1.0; R.m[0][1] = 0.0; R.m[0][2] = 0.0; R.m[0][3] = 0.0;
	R.m[1][0] = 0.0; R.m[1][1] = c;   R.m[1][2] = -s;  R.m[1][3] = 0.0;
	R.m[2][0] = 0.0; R.m[2][1] = s;   R.m[2][2] = c;   R.m[2][3] = 0.0;
	R.m[3][0] = 0.0; R.m[3][1] = 0.0; R.m[3][2] = 0.0; R.m[3][3] = 1.0;
	SG_MatrixMult44v_FPU(M, &R);
}

static __inline__ void
SG_MatrixRotateJ44_FPU(SG_Matrix *M, SG_Real theta)
{
	SG_Real s = SG_Sin(theta);
	SG_Real c = SG_Cos(theta);
	SG_Matrix R;

	/* XXX Optimize! */
	R.m[0][0] = c;   R.m[0][1] = 0.0; R.m[0][2] = s;   R.m[0][3] = 0.0;
	R.m[1][0] = 0.0; R.m[1][1] = 1.0; R.m[1][2] = 0.0; R.m[1][3] = 0.0;
	R.m[2][0] = -s;  R.m[2][1] = 0.0; R.m[2][2] = c;   R.m[2][3] = 0.0;
	R.m[3][0] = 0.0; R.m[3][1] = 0.0; R.m[3][2] = 0.0; R.m[3][3] = 1.0;
	SG_MatrixMult44v_FPU(M, &R);
}

static __inline__ void
SG_MatrixRotateK44_FPU(SG_Matrix *M, SG_Real theta)
{
	SG_Real s = SG_Sin(theta);
	SG_Real c = SG_Cos(theta);
	SG_Matrix R;

	/* XXX Optimize! */
	R.m[0][0] = c;   R.m[0][1] = -s;  R.m[0][2] = 0.0; R.m[0][3] = 0.0;
	R.m[1][0] = s;   R.m[1][1] = c;   R.m[1][2] = 0.0; R.m[1][3] = 0.0;
	R.m[2][0] = 0.0; R.m[2][1] = 0.0; R.m[2][2] = 1.0; R.m[2][3] = 0.0;
	R.m[3][0] = 0.0; R.m[3][1] = 0.0; R.m[3][2] = 0.0; R.m[3][3] = 1.0;
	SG_MatrixMult44v_FPU(M, &R);
}

static __inline__ void
SG_MatrixTranslateX44_FPU(SG_Matrix *M, SG_Real x)
{
	SG_Matrix T;

	/* XXX Optimize! */
	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = x;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = 0.0;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = 0.0;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	SG_MatrixMult44v_FPU(M, &T);
}

static __inline__ void
SG_MatrixTranslateY44_FPU(SG_Matrix *M, SG_Real y)
{
	SG_Matrix T;

	/* XXX Optimize! */
	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = 0.0;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = y;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = 0.0;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	SG_MatrixMult44v_FPU(M, &T);
}

static __inline__ void
SG_MatrixTranslateZ44_FPU(SG_Matrix *M, SG_Real z)
{
	SG_Matrix T;

	/* XXX Optimize! */
	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = 0.0;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = 0.0;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = z;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	SG_MatrixMult44v_FPU(M, &T);
}

static __inline__ void
SG_MatrixScale44_FPU(SG_Matrix *M, SG_Real x, SG_Real y, SG_Real z, SG_Real w)
{
	SG_Matrix S;

	S.m[0][0] = x;   S.m[0][1] = 0.0; S.m[0][2] = 0.0; S.m[0][3] = 0.0;
	S.m[1][0] = 0.0; S.m[1][1] = y;   S.m[1][2] = 0.0; S.m[1][3] = 0.0;
	S.m[2][0] = 0.0; S.m[2][1] = 0.0; S.m[2][2] = z;   S.m[2][3] = 0.0;
	S.m[3][0] = 0.0; S.m[3][1] = 0.0; S.m[3][2] = 0.0; S.m[3][3] = w;
	SG_MatrixMult44v_FPU(M, &S);
}

static __inline__ void
SG_MatrixUniScale44_FPU(SG_Matrix *M, SG_Real r)
{
	SG_Matrix S;

	S.m[0][0] = r;   S.m[0][1] = 0.0; S.m[0][2] = 0.0; S.m[0][3] = 0.0;
	S.m[1][0] = 0.0; S.m[1][1] = r;   S.m[1][2] = 0.0; S.m[1][3] = 0.0;
	S.m[2][0] = 0.0; S.m[2][1] = 0.0; S.m[2][2] = r;   S.m[2][3] = 0.0;
	S.m[3][0] = 0.0; S.m[3][1] = 0.0; S.m[3][2] = 0.0; S.m[3][3] = 1.0;
	SG_MatrixMult44v_FPU(M, &S);
}

__BEGIN_DECLS
extern const SG_MatrixOps44 sgMatOps44_FPU;

SG_Matrix SG_MatrixInvert44_FPU(SG_Matrix);
SG_Matrix SG_MatrixInvert44p_FPU(const SG_Matrix *);
int       SG_MatrixInvertGaussJordan44v_FPU(const SG_Matrix *, SG_Matrix *);
void      SG_MatrixDiagonalSwap44v_FPU(SG_Matrix *);
__END_DECLS
