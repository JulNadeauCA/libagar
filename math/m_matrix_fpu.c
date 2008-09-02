/*
 * Public domain.
 * Operations on m*n matrices (FPU version).
 */

#include <core/core.h>
#include "m.h"

const M_MatrixOps mMatOps_FPU = {
	"scalar",
	/* Inline */
	M_GetElement_FPU,
	M_Get_FPU,
	M_MatrixResize_FPU,
	M_MatrixFree_FPU,
	M_MatrixNew_FPU,
	M_MatrixPrint_FPU,
	M_MatrixSetIdentity_FPU,
	M_MatrixSetZero_FPU,
	M_MatrixTranspose_FPU,
	M_MatrixCopy_FPU,
	M_MatrixDup_FPU,
	M_MatrixAdd_FPU,
	M_MatrixAddv_FPU,
	M_MatrixDirectSum_FPU,
	M_MatrixMul_FPU,
	M_MatrixMulv_FPU,
	M_MatrixEntMul_FPU,
	M_MatrixEntMulv_FPU,
	/* Not inline */
	M_MatrixCompare_FPU,
	M_MatrixTrace_FPU,
	M_MatrixRead_FPU,
	M_MatrixWrite_FPU,
	M_MatrixToFloats_FPU,
	M_MatrixToDoubles_FPU,
	M_MatrixFromFloats_FPU,
	M_MatrixFromDoubles_FPU,

	M_InvertGaussJordanv_FPU,
	M_InvertGaussJordan_FPU,
	M_FactorizeLU_FPU,
	M_BacksubstLU_FPU,
	M_MNAPreorder_FPU,
	M_AddToDiag_FPU
};

void *
M_MatrixRead_FPU(AG_DataSource *buf)
{
	M_MatrixFPU *A;
	Uint m,n, i,j;

	m = (Uint)AG_ReadUint32(buf);
	n = (Uint)AG_ReadUint32(buf);
	A = M_MatrixNew_FPU(m,n);
	for (i = 0; i < m; i++) {
		for (j = 0; j < n; j++)
			A->v[i][j] = M_ReadReal(buf);
	}
	return (A);
}

void
M_MatrixWrite_FPU(AG_DataSource *buf, const void *pA)
{
	const M_MatrixFPU *A=pA;
	Uint i, j;

	AG_WriteUint32(buf, (Uint32)MROWS(A));
	AG_WriteUint32(buf, (Uint32)MCOLS(A));
	for (i = 0; i < MROWS(A); i++) {
		for (j = 0; j < MCOLS(A); j++)
			M_WriteReal(buf, A->v[i][j]);
	}
}

/* Compare two matrices entrywise and return the largest difference. */
int
M_MatrixCompare_FPU(const void *pA, const void *pB, M_Real *diff)
{
	const M_MatrixFPU *A=pA, *B=pB;
	M_Real d;
	Uint n, m;

	M_ASSERT_COMPAT_MATRICES(A,B, -1);
	*diff = 0.0;
	for (m = 0; m < MROWS(A); m++) {
		for (n = 0; n < MCOLS(A); n++) {
			d = M_Fabs(A->v[m][n] - B->v[m][n]);
			if (d > *diff) { *diff = d; }
		}
	}
	return (0);
}

/* Return the trace of matrix A. */
int
M_MatrixTrace_FPU(M_Real *sum, const void *pA)
{
	const M_MatrixFPU *A=pA;
	Uint n;

	M_ASSERT_SQUARE_MATRIX(A, -1);
	*sum = 0.0;
	for (n = 0; n < MCOLS(A); n++) {
		(*sum) += A->v[n][n];
	}
	return (0);
}

/* Convert matrix A to an array of floats. */
void
M_MatrixToFloats_FPU(float *fv, const void *pA)
{
	const M_MatrixFPU *A=pA;
#ifdef SINGLE_PRECISION
	memcpy(fv, &A->v[0][0], MROWS(A)*MCOLS(A)*sizeof(float));
#else
	Uint i, j;
	for (i = 0; i < MROWS(A); i++) {
		for (j = 0; j < MCOLS(A); j++)
			fv[(i<<2)+j] = (float)A->v[i][j];
	}
#endif
}

/* Convert matrix A to an array of doubles. */
void
M_MatrixToDoubles_FPU(double *dv, const void *pA)
{
	const M_MatrixFPU *A=pA;

#ifdef DOUBLE_PRECISION
	memcpy(dv, &A->v[0][0], MROWS(A)*MCOLS(A)*sizeof(double));
#else
	Uint i, j;
	for (i = 0; i < MROWS(A); i++) {
		for (j = 0; j < MCOLS(A); j++)
			dv[(i<<2)+j] = (double)A->v[i][j];
	}
#endif
}

/* Load matrix A from an array of floats. */
void
M_MatrixFromFloats_FPU(void *pA, const float *fv)
{
	M_MatrixFPU *A=pA;

#ifdef SINGLE_PRECISION
	memcpy(&A->v[0][0], fv, MROWS(A)*MCOLS(A)*sizeof(float));
#else
	Uint i, j;
	for (i = 0; i < MROWS(A); i++) {
		for (j = 0; j < MCOLS(A); j++)
			A->v[i][j] = (double)fv[(i<<2)+j];
	}
#endif
}

/* Load matrix A from an array of doubles. */
void
M_MatrixFromDoubles_FPU(void *pA, const double *fv)
{
	M_MatrixFPU *A=pA;

#ifdef DOUBLE_PRECISION
	memcpy(&A->v[0][0], fv, MROWS(A)*MCOLS(A)*sizeof(double));
#else
	Uint i, j;
	for (i = 0; i < MROWS(A); i++) {
		for (j = 0; j < MCOLS(A); j++)
			A->v[i][j] = (float)fv[(i<<2)+j];
	}
#endif
}

void
M_MatrixPrint_FPU(void *pM)
{
	/* Do not cast pM into M_MatrixFPU since the print function
	 * is generic and may be used by other backends */
	M_Matrix *M = pM;
	Uint m, n;
	M_Real e;

	fputs(" -\n", stdout);
	for (m = 0; m < MROWS(M); m++) {
		fputs("| ", stdout);
		for (n = 0; n < MCOLS(M); n++) {
			e = M_Get(M, m, n);
			if (e >= 0) {
				fputc(' ', stdout);
			}
			printf("%.02f ", e);
		}
		fputs("|\n", stdout);
	}
	fputs(" -\n", stdout);
}
