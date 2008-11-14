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
M_MatrixZero44v_FPU(M_Matrix44 *A)
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
M_MatrixIdentity44v_FPU(M_Matrix44 *A)
{
	A->m[0][0] = 1.0; A->m[0][1] = 0.0; A->m[0][2] = 0.0; A->m[0][3] = 0.0;
	A->m[1][0] = 0.0; A->m[1][1] = 1.0; A->m[1][2] = 0.0; A->m[1][3] = 0.0;
	A->m[2][0] = 0.0; A->m[2][1] = 0.0; A->m[2][2] = 1.0; A->m[2][3] = 0.0;
	A->m[3][0] = 0.0; A->m[3][1] = 0.0; A->m[3][2] = 0.0; A->m[3][3] = 1.0;
}

static __inline__ void
M_MatrixMult44pv_FPU(M_Matrix44 *C, const M_Matrix44 *A, const M_Matrix44 *B)
{
	int m, n;

#ifdef AG_DEBUG
	if (C == A) { AG_FatalError("use M_MatrixMultv() if C==A"); }
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
M_MatrixCopy44_FPU(M_Matrix44 *mDst, const M_Matrix44 *mSrc)
{
	/* XXX Optimize */
	memcpy(&mDst->m[0][0], &mSrc->m[0][0], 16*sizeof(M_Real));
}

static __inline__ void
M_MatrixMult44v_FPU(M_Matrix44 *A, const M_Matrix44 *B)
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

static __inline__ M_Vector3
M_MatrixMultVector344_FPU(M_Matrix44 A, M_Vector3 x)
{
	M_Vector3 Ax;

	Ax.x = A.m[0][0]*x.x + A.m[0][1]*x.y + A.m[0][2]*x.z + A.m[0][3];
	Ax.y = A.m[1][0]*x.x + A.m[1][1]*x.y + A.m[1][2]*x.z + A.m[1][3];
	Ax.z = A.m[2][0]*x.x + A.m[2][1]*x.y + A.m[2][2]*x.z + A.m[2][3];
	return (Ax);
}

static __inline__ M_Vector3
M_MatrixMultVector344p_FPU(const M_Matrix44 *A, const M_Vector3 *x)
{
	M_Vector3 Ax;

	Ax.x = A->m[0][0]*x->x + A->m[0][1]*x->y + A->m[0][2]*x->z + A->m[0][3];
	Ax.y = A->m[1][0]*x->x + A->m[1][1]*x->y + A->m[1][2]*x->z + A->m[1][3];
	Ax.z = A->m[2][0]*x->x + A->m[2][1]*x->y + A->m[2][2]*x->z + A->m[2][3];
	return (Ax);
}

static __inline__ void
M_MatrixMultVector344v_FPU(M_Vector3 *x, const M_Matrix44 *A)
{
	M_Real xx = x->x;
	M_Real xy = x->y;
	M_Real xz = x->z;
	
	x->x = A->m[0][0]*xx + A->m[0][1]*xy + A->m[0][2]*xz + A->m[0][3];
	x->y = A->m[1][0]*xx + A->m[1][1]*xy + A->m[1][2]*xz + A->m[1][3];
	x->z = A->m[2][0]*xx + A->m[2][1]*xy + A->m[2][2]*xz + A->m[2][3];
}

static __inline__ M_Vector4
M_MatrixMultVector444_FPU(M_Matrix44 A, M_Vector4 x)
{
	M_Vector4 Ax;

	Ax.x = A.m[0][0]*x.x + A.m[0][1]*x.y + A.m[0][2]*x.z + A.m[0][3]*x.w;
	Ax.y = A.m[1][0]*x.x + A.m[1][1]*x.y + A.m[1][2]*x.z + A.m[1][3]*x.w;
	Ax.z = A.m[2][0]*x.x + A.m[2][1]*x.y + A.m[2][2]*x.z + A.m[2][3]*x.w;
	Ax.w = A.m[3][0]*x.x + A.m[3][1]*x.y + A.m[3][2]*x.z + A.m[3][3]*x.w;
	return (Ax);
}

static __inline__ M_Vector4
M_MatrixMultVector444p_FPU(const M_Matrix44 *A, const M_Vector4 *x)
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
M_MatrixMultVector444v_FPU(M_Vector4 *x, const M_Matrix44 *A)
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

/* XXX Optimize */
static __inline__ void
M_MatrixToFloats44_FPU(float *fv, const M_Matrix44 *M)
{
#ifdef SINGLE_PRECISION
	memcpy(fv, &M->m[0][0], 16*sizeof(float));
#else
	int m, n;
	for (m = 0; m < 4; m++) {
		for (n = 0; n < 4; n++)
			fv[(m<<2)+n] = (float)M->m[m][n];
	}
#endif
}

/* XXX Optimize */
static __inline__ void
M_MatrixToDoubles44_FPU(double *dv, const M_Matrix44 *M)
{
#ifdef DOUBLE_PRECISION
	memcpy(dv, &M->m[0][0], 16*sizeof(double));
#else
	int m, n;
	for (m = 0; m < 4; m++) {
		for (n = 0; n < 4; n++)
			dv[(m<<2)+n] = (double)M->m[m][n];
	}
#endif
}

/* XXX Optimize */
static __inline__ void
M_MatrixFromFloats44_FPU(M_Matrix44 *M, const float *fv)
{
#ifdef SINGLE_PRECISION
	memcpy(&M->m[0][0], fv, 16*sizeof(float));
#else
	int m, n;
	for (m = 0; m < 4; m++) {
		for (n = 0; n < 4; n++)
			M->m[m][n] = (double)fv[(m<<2)+n];
	}
#endif
}

/* XXX Optimize */
static __inline__ void
M_MatrixFromDoubles44_FPU(M_Matrix44 *M, const double *fv)
{
#ifdef DOUBLE_PRECISION
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
M_MatrixGetDirection44_FPU(const M_Matrix44 *A, M_Vector3 *x, M_Vector3 *y,
    M_Vector3 *z)
{
	if (x != NULL) { x->x=A->m[0][0]; x->y=A->m[1][0]; x->z=A->m[2][0]; }
	if (y != NULL) { y->x=A->m[0][1]; y->y=A->m[1][1]; y->z=A->m[2][1]; }
	if (z != NULL) { z->x=A->m[0][2]; z->y=A->m[1][2]; z->z=A->m[2][2]; }
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
M_MatrixTranspose44p_FPU(const M_Matrix44 *M)
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
M_MatrixTranspose44v_FPU(M_Matrix44 *M)
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

static __inline__ void
M_MatrixRotateAxis44_FPU(M_Matrix44 *M, M_Real theta, M_Vector3 A)
{
	M_Real s = M_Sin(theta);
	M_Real c = M_Cos(theta);
	M_Real t = 1.0 - c;
	M_Matrix44 R;

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
	M_MatrixMult44v_FPU(M, &R);
}

static __inline__ void
M_MatrixTranslate44_FPU(M_Matrix44 *M, M_Vector3 v)
{
	M_Matrix44 T;

	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = v.x;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = v.y;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = v.z;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	M_MatrixMult44v_FPU(M, &T);
}

static __inline__ void
M_MatrixTranslate344_FPU(M_Matrix44 *M, M_Real x, M_Real y, M_Real z)
{
	M_Matrix44 T;

	/* XXX Optimize! */
	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = x;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = y;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = z;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	M_MatrixMult44v_FPU(M, &T);
}

static __inline__ void
M_MatrixOrbitAxis44_FPU(M_Matrix44 *M, M_Vector3 p, M_Vector3 A, M_Real theta)
{
	M_Matrix44 R;
	M_Real s = M_Sin(theta);
	M_Real c = M_Cos(theta);
	M_Real t = 1.0 - c;

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
	M_MatrixTranslate344_FPU(M, -p.x, -p.y, -p.z);
	M_MatrixMult44v_FPU(M, &R);
	M_MatrixTranslate44_FPU(M, p);
}

static __inline__ void
M_MatrixRotateEul44_FPU(M_Matrix44 *M, M_Real pitch, M_Real roll, M_Real yaw)
{
	M_Matrix44 R;

	R.m[0][0] =  M_Cos(yaw)*M_Cos(roll) +
	             M_Sin(yaw)*M_Sin(pitch)*M_Sin(roll);
	R.m[0][1] =  M_Sin(yaw)*M_Cos(roll) -
	             M_Cos(yaw)*M_Sin(pitch)*M_Sin(roll);
	R.m[0][2] =  M_Cos(pitch)*M_Sin(roll);
	R.m[0][3] =  0.0;
	R.m[1][0] = -M_Sin(yaw)*M_Cos(pitch);
	R.m[1][1] =  M_Cos(yaw)*M_Cos(pitch);
	R.m[1][2] =  M_Sin(pitch);
	R.m[1][3] =  0.0;
	R.m[2][0] =  M_Sin(yaw)*M_Sin(pitch)*M_Cos(roll) -
	             M_Cos(yaw)*M_Sin(roll);
	R.m[2][1] = -M_Cos(yaw)*M_Sin(pitch)*M_Cos(roll) -
	             M_Sin(yaw)*M_Sin(roll);
	R.m[2][2] =  M_Cos(pitch)*M_Cos(roll);
	R.m[2][3] =  0.0;
	R.m[3][0] =  0.0;
	R.m[3][1] =  0.0;
	R.m[3][2] =  0.0;
	R.m[3][3] =  1.0;
	M_MatrixMult44v_FPU(M, &R);
}

static __inline__ void
M_MatrixRotate44I_FPU(M_Matrix44 *M, M_Real theta)
{
	M_Real s = M_Sin(theta);
	M_Real c = M_Cos(theta);
	M_Matrix44 R;

	/* XXX Optimize! */
	R.m[0][0] = 1.0; R.m[0][1] = 0.0; R.m[0][2] = 0.0; R.m[0][3] = 0.0;
	R.m[1][0] = 0.0; R.m[1][1] = c;   R.m[1][2] = -s;  R.m[1][3] = 0.0;
	R.m[2][0] = 0.0; R.m[2][1] = s;   R.m[2][2] = c;   R.m[2][3] = 0.0;
	R.m[3][0] = 0.0; R.m[3][1] = 0.0; R.m[3][2] = 0.0; R.m[3][3] = 1.0;
	M_MatrixMult44v_FPU(M, &R);
}

static __inline__ void
M_MatrixRotate44J_FPU(M_Matrix44 *M, M_Real theta)
{
	M_Real s = M_Sin(theta);
	M_Real c = M_Cos(theta);
	M_Matrix44 R;

	/* XXX Optimize! */
	R.m[0][0] = c;   R.m[0][1] = 0.0; R.m[0][2] = s;   R.m[0][3] = 0.0;
	R.m[1][0] = 0.0; R.m[1][1] = 1.0; R.m[1][2] = 0.0; R.m[1][3] = 0.0;
	R.m[2][0] = -s;  R.m[2][1] = 0.0; R.m[2][2] = c;   R.m[2][3] = 0.0;
	R.m[3][0] = 0.0; R.m[3][1] = 0.0; R.m[3][2] = 0.0; R.m[3][3] = 1.0;
	M_MatrixMult44v_FPU(M, &R);
}

static __inline__ void
M_MatrixRotate44K_FPU(M_Matrix44 *M, M_Real theta)
{
	M_Real s = M_Sin(theta);
	M_Real c = M_Cos(theta);
	M_Matrix44 R;

	/* XXX Optimize! */
	R.m[0][0] = c;   R.m[0][1] = -s;  R.m[0][2] = 0.0; R.m[0][3] = 0.0;
	R.m[1][0] = s;   R.m[1][1] = c;   R.m[1][2] = 0.0; R.m[1][3] = 0.0;
	R.m[2][0] = 0.0; R.m[2][1] = 0.0; R.m[2][2] = 1.0; R.m[2][3] = 0.0;
	R.m[3][0] = 0.0; R.m[3][1] = 0.0; R.m[3][2] = 0.0; R.m[3][3] = 1.0;
	M_MatrixMult44v_FPU(M, &R);
}

static __inline__ void
M_MatrixTranslateX44_FPU(M_Matrix44 *M, M_Real x)
{
	M_Matrix44 T;

	/* XXX Optimize! */
	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = x;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = 0.0;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = 0.0;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	M_MatrixMult44v_FPU(M, &T);
}

static __inline__ void
M_MatrixTranslateY44_FPU(M_Matrix44 *M, M_Real y)
{
	M_Matrix44 T;

	/* XXX Optimize! */
	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = 0.0;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = y;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = 0.0;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	M_MatrixMult44v_FPU(M, &T);
}

static __inline__ void
M_MatrixTranslateZ44_FPU(M_Matrix44 *M, M_Real z)
{
	M_Matrix44 T;

	/* XXX Optimize! */
	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = 0.0;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = 0.0;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = z;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	M_MatrixMult44v_FPU(M, &T);
}

static __inline__ void
M_MatrixScale44_FPU(M_Matrix44 *M, M_Real x, M_Real y, M_Real z, M_Real w)
{
	M_Matrix44 S;

	S.m[0][0] = x;   S.m[0][1] = 0.0; S.m[0][2] = 0.0; S.m[0][3] = 0.0;
	S.m[1][0] = 0.0; S.m[1][1] = y;   S.m[1][2] = 0.0; S.m[1][3] = 0.0;
	S.m[2][0] = 0.0; S.m[2][1] = 0.0; S.m[2][2] = z;   S.m[2][3] = 0.0;
	S.m[3][0] = 0.0; S.m[3][1] = 0.0; S.m[3][2] = 0.0; S.m[3][3] = w;
	M_MatrixMult44v_FPU(M, &S);
}

static __inline__ void
M_MatrixUniScale44_FPU(M_Matrix44 *M, M_Real r)
{
	M_Matrix44 S;

	S.m[0][0] = r;   S.m[0][1] = 0.0; S.m[0][2] = 0.0; S.m[0][3] = 0.0;
	S.m[1][0] = 0.0; S.m[1][1] = r;   S.m[1][2] = 0.0; S.m[1][3] = 0.0;
	S.m[2][0] = 0.0; S.m[2][1] = 0.0; S.m[2][2] = r;   S.m[2][3] = 0.0;
	S.m[3][0] = 0.0; S.m[3][1] = 0.0; S.m[3][2] = 0.0; S.m[3][3] = 1.0;
	M_MatrixMult44v_FPU(M, &S);
}
__END_DECLS

__BEGIN_DECLS
extern const M_MatrixOps44 mMatOps44_FPU;

M_Matrix44 M_MatrixInvert44_FPU(M_Matrix44);
M_Matrix44 M_MatrixInvert44p_FPU(const M_Matrix44 *);
int        M_MatrixInvertGaussJordan44v_FPU(const M_Matrix44 *, M_Matrix44 *);
void       M_MatrixDiagonalSwap44v_FPU(M_Matrix44 *);
__END_DECLS
