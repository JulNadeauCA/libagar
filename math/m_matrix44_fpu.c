/*
 * Public domain.
 * Operations on 4x4 matrices (FPU version).
 */

#include <agar/core/core.h>
#include <agar/math/m.h>

/* Invert a 4x4 matrix using cofactors and Cramer's rule. */
M_Matrix44
M_MatrixInvert44_FPU(M_Matrix44 A)
{
	M_Matrix44 Ainv;
	M_Real tmp[12];
	M_Real src[16];
	M_Real dei;
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

/* Invert a 4x4 matrix by systematic elimination. */
int
M_MatrixInvertElim44_FPU(M_Matrix44 Ain, M_Matrix44 *Ainv)
{
	M_Matrix44 A = Ain;
	int m, n, o, swap;
	M_Real t;

	M_MatrixIdentity44v_FPU(Ainv);
	
	for (n = 0; n < 4; n++) {
		for (m = n+1, swap = n;
		     m < 4;
		     m++) {
			if (Fabs(A.m[m][n]) > Fabs(A.m[n][n]))
				swap = m;
		}
		if (swap != n) {
			for (o = 0; o < 4; o++) {
				t = A.m[n][o];
				A.m[n][o] = A.m[swap][o];
				Ainv->m[swap][o] = t;

				t = Ainv->m[n][o];
				Ainv->m[n][o] = Ainv->m[swap][o];
				Ainv->m[swap][o] = t;
			}
		}
		if (Fabs(A.m[n][n] - 0.0) < M_MACHEP) {
			AG_SetError("Matrix singular to machine precision");
			return (-1);
		}

		t = A.m[n][n];
		for (o = 0; o < 4; o++) {
			A.m[n][o] /= t;
			Ainv->m[n][o] /= t;
		}
		for (m = 0; m < 4; m++) {
			if (m != n) {
				t = A.m[m][n];
				for (o = 0; o < 4; o++) {
					A.m[m][o] -= A.m[n][o]*t;
					Ainv->m[m][o] -= Ainv->m[n][o]*t;
				}
			}
		}
	}
	return (0);
}

void
M_MatrixCopy44_FPU(M_Matrix44 *_Nonnull mDst, const M_Matrix44 *_Nonnull mSrc)
{
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
	memcpy(mDst->m, mSrc->m, 16*sizeof(float));
#else
	memcpy(mDst->m, mSrc->m, 16*sizeof(M_Real));
#endif
}

void
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

void
M_MatrixTranslatev44_FPU(M_Matrix44 *M, M_Vector3 v)
{
	M_Matrix44 T;

	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = v.x;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = v.y;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = v.z;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	M_MatrixMult44v_FPU(M, &T);
}

void
M_MatrixTranslate44_FPU(M_Matrix44 *M, M_Real x, M_Real y, M_Real z)
{
	M_Matrix44 T;

	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = x;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = y;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = z;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	M_MatrixMult44v_FPU(M, &T);
}

void
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

	M_MatrixTranslate44_FPU(M, -p.x, -p.y, -p.z);
	M_MatrixMult44v_FPU(M, &R);
	M_MatrixTranslatev44_FPU(M, p);
}

void
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

void
M_MatrixRotate44I_FPU(M_Matrix44 *M, M_Real theta)
{
	M_Real s = M_Sin(theta);
	M_Real c = M_Cos(theta);
	M_Matrix44 R;

	R.m[0][0] = 1.0; R.m[0][1] = 0.0; R.m[0][2] = 0.0; R.m[0][3] = 0.0;
	R.m[1][0] = 0.0; R.m[1][1] = c;   R.m[1][2] = -s;  R.m[1][3] = 0.0;
	R.m[2][0] = 0.0; R.m[2][1] = s;   R.m[2][2] = c;   R.m[2][3] = 0.0;
	R.m[3][0] = 0.0; R.m[3][1] = 0.0; R.m[3][2] = 0.0; R.m[3][3] = 1.0;
	M_MatrixMult44v_FPU(M, &R);
}
void
M_MatrixRotate44J_FPU(M_Matrix44 *M, M_Real theta)
{
	M_Real s = M_Sin(theta);
	M_Real c = M_Cos(theta);
	M_Matrix44 R;

	R.m[0][0] = c;   R.m[0][1] = 0.0; R.m[0][2] = s;   R.m[0][3] = 0.0;
	R.m[1][0] = 0.0; R.m[1][1] = 1.0; R.m[1][2] = 0.0; R.m[1][3] = 0.0;
	R.m[2][0] = -s;  R.m[2][1] = 0.0; R.m[2][2] = c;   R.m[2][3] = 0.0;
	R.m[3][0] = 0.0; R.m[3][1] = 0.0; R.m[3][2] = 0.0; R.m[3][3] = 1.0;
	M_MatrixMult44v_FPU(M, &R);
}
void
M_MatrixRotate44K_FPU(M_Matrix44 *M, M_Real theta)
{
	M_Real s = M_Sin(theta);
	M_Real c = M_Cos(theta);
	M_Matrix44 R;

	R.m[0][0] = c;   R.m[0][1] = -s;  R.m[0][2] = 0.0; R.m[0][3] = 0.0;
	R.m[1][0] = s;   R.m[1][1] = c;   R.m[1][2] = 0.0; R.m[1][3] = 0.0;
	R.m[2][0] = 0.0; R.m[2][1] = 0.0; R.m[2][2] = 1.0; R.m[2][3] = 0.0;
	R.m[3][0] = 0.0; R.m[3][1] = 0.0; R.m[3][2] = 0.0; R.m[3][3] = 1.0;
	M_MatrixMult44v_FPU(M, &R);
}

void
M_MatrixTranslateX44_FPU(M_Matrix44 *M, M_Real x)
{
	M_Matrix44 T;

	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = x;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = 0.0;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = 0.0;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	M_MatrixMult44v_FPU(M, &T);
}
void
M_MatrixTranslateY44_FPU(M_Matrix44 *M, M_Real y)
{
	M_Matrix44 T;

	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = 0.0;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = y;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = 0.0;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	M_MatrixMult44v_FPU(M, &T);
}
void
M_MatrixTranslateZ44_FPU(M_Matrix44 *M, M_Real z)
{
	M_Matrix44 T;

	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0; T.m[0][3] = 0.0;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0; T.m[1][3] = 0.0;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0; T.m[2][3] = z;
	T.m[3][0] = 0.0; T.m[3][1] = 0.0; T.m[3][2] = 0.0; T.m[3][3] = 1.0;
	M_MatrixMult44v_FPU(M, &T);
}

void
M_MatrixScale44_FPU(M_Matrix44 *M, M_Real x, M_Real y, M_Real z, M_Real w)
{
	M_Matrix44 S;

	S.m[0][0] = x;   S.m[0][1] = 0.0; S.m[0][2] = 0.0; S.m[0][3] = 0.0;
	S.m[1][0] = 0.0; S.m[1][1] = y;   S.m[1][2] = 0.0; S.m[1][3] = 0.0;
	S.m[2][0] = 0.0; S.m[2][1] = 0.0; S.m[2][2] = z;   S.m[2][3] = 0.0;
	S.m[3][0] = 0.0; S.m[3][1] = 0.0; S.m[3][2] = 0.0; S.m[3][3] = w;
	M_MatrixMult44v_FPU(M, &S);
}
void
M_MatrixUniScale44_FPU(M_Matrix44 *M, M_Real r)
{
	M_Matrix44 S;

	S.m[0][0] = r;   S.m[0][1] = 0.0; S.m[0][2] = 0.0; S.m[0][3] = 0.0;
	S.m[1][0] = 0.0; S.m[1][1] = r;   S.m[1][2] = 0.0; S.m[1][3] = 0.0;
	S.m[2][0] = 0.0; S.m[2][1] = 0.0; S.m[2][2] = r;   S.m[2][3] = 0.0;
	S.m[3][0] = 0.0; S.m[3][1] = 0.0; S.m[3][2] = 0.0; S.m[3][3] = 1.0;
	M_MatrixMult44v_FPU(M, &S);
}

const M_MatrixOps44 mMatOps44_FPU = {
	"scalar",
	M_MatrixZero44_FPU,
	M_MatrixZero44v_FPU,
	M_MatrixIdentity44_FPU,
	M_MatrixIdentity44v_FPU,
	M_MatrixTranspose44_FPU,
	M_MatrixTranspose44p_FPU,
	M_MatrixTranspose44v_FPU,
	M_MatrixInvert44_FPU,
	M_MatrixInvertElim44_FPU,
	M_MatrixMult44_FPU,
	M_MatrixMult44v_FPU,
	M_MatrixMultVector44_FPU,
	M_MatrixMultVector44p_FPU,
	M_MatrixMultVector44v_FPU,
	M_MatrixCopy44_FPU,
	M_MatrixToFloats44_FPU,
	M_MatrixToDoubles44_FPU,
	M_MatrixFromFloats44_FPU,
	M_MatrixFromDoubles44_FPU,
	M_MatrixRotateAxis44_FPU,
	M_MatrixOrbitAxis44_FPU,
	M_MatrixRotateEul44_FPU,
	M_MatrixRotate44I_FPU,
	M_MatrixRotate44J_FPU,
	M_MatrixRotate44K_FPU,
	M_MatrixTranslatev44_FPU,
	M_MatrixTranslate44_FPU,
	M_MatrixTranslateX44_FPU,
	M_MatrixTranslateY44_FPU,
	M_MatrixTranslateZ44_FPU,
	M_MatrixScale44_FPU,
	M_MatrixUniScale44_FPU,
};
