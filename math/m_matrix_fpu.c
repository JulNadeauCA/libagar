/*
 * Public domain.
 * Operations on m*n matrices (FPU version).
 */

#include <agar/core/core.h>
#include <agar/math/m.h>

#undef SWAP
#define SWAP(a,b) { tmp=(a); (a)=(b); (b)=tmp; }

const M_MatrixOps mMatOps_FPU = {
	"scalar",
	/* Inline */
	M_GetElement_FPU,
	M_Get_FPU,
	M_MatrixResize_FPU,
	M_MatrixFree_FPU,
	M_MatrixNew_FPU,
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
	M_GaussJordan_FPU,
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
	if ((A = M_MatrixNew_FPU(m,n)) == NULL) {
		AG_FatalError(NULL);
	}
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
	memcpy(fv, A->v, MROWS(A)*MCOLS(A)*sizeof(float));
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
	memcpy(dv, A->v, MROWS(A)*MCOLS(A)*sizeof(double));
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
	memcpy(A->v, fv, MROWS(A)*MCOLS(A)*sizeof(float));
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
	memcpy(A->v, fv, MROWS(A)*MCOLS(A)*sizeof(double));
#else
	Uint i, j;
	for (i = 0; i < MROWS(A); i++) {
		for (j = 0; j < MCOLS(A); j++)
			A->v[i][j] = (float)fv[(i<<2)+j];
	}
#endif
}

/*
 * LU Factorization -
 * Decompose a square matrix A into a product of the upper-triangular
 * matrix U and the lower-triangular matrix L, following a row-wise
 * permutation. The partial pivoting information is recorded in ivec.
 */
int 
M_FactorizeLU_FPU(void *pA)
{
	M_MatrixFPU *Aorig=pA;
	M_Real big;
	M_Real sum, dum, a;
	int i, j, k, iMax = 0;
	M_Vector *vs;
	M_MatrixFPU *A = NULL;

	M_ASSERT_SQUARE_MATRIX(Aorig, -1);

	/* initialize LU structure if not previously used */
	if(Aorig->ivec == NULL)
		Aorig->ivec = M_VectorNewZ(MCOLS(Aorig));

	if(Aorig->LU == NULL)
		Aorig->LU = M_New(MROWS(Aorig), MCOLS(Aorig));

	A = Aorig->LU;	


	M_Copy(A, Aorig);

	vs = M_VecNew(MCOLS(A));

	/* Generate implicit scaling information. */
	for (i = 0; i < MCOLS(A); i++) {
		big = 0.0;
		for (j = 0; j < MROWS(A); j++) {
			a = Fabs(A->v[i][j]);
			if (a > big) { big = a; }
		}
		if (Fabs(big) <= M_MACHEP) {
#ifdef AG_ENABLE_STRING
			AG_Verbose("Singular matrix: %s\n", AG_Printf("%[M]", A));
#else
			AG_Verbose("Singular matrix\n");
#endif
			AG_SetError("Singular matrix (no pivot in column %i)", i);
			goto fail;
		}
		vs->v[i] = 1.0/big;
	}

	/* Solve the set of equations using Crout's algorithm. */
	for (j = 0; j < MCOLS(A); j++) {
		for (i = 0; i < j; i++) {
			sum = A->v[i][j];
			for (k = 0; k < i; k++) {
				sum -= A->v[i][k]*A->v[k][j];
			}
			A->v[i][j] = sum;
		}

		big = 0.0;
		for (i = j; i < MCOLS(A); i++) {
			sum = A->v[i][j];
			for (k = 0; k < j; k++) {
				sum -= A->v[i][k]*A->v[k][j];
			}
			A->v[i][j] = sum;
			dum = vs->v[i]*Fabs(sum);
			if (dum >= big) {
				big = dum;
				iMax = i;
			}
		}

		/* Interchange rows if necessary. */
		if (j != iMax) {
			for (k = 0; k < MCOLS(A); k++) {
				dum = A->v[iMax][k];
				A->v[iMax][k] = A->v[j][k];
				A->v[j][k] = dum;
			}
			vs->v[iMax] = vs->v[j];
		}
		Aorig->ivec->v[j] = iMax;

		if (Fabs(A->v[j][j]) <= M_MACHEP)
			A->v[j][j] = M_TINYVAL;
		
		/* Divide by the pivot element. */
		if (j != MCOLS(A)-1) {
			dum = 1.0/A->v[j][j];
			for (i = j+1; i < MCOLS(A); i++)
				A->v[i][j] *= dum;
		}
	}
	M_VecFree(vs);
	return 0;
fail:
	M_VecFree(vs);
	return -1;
}

/*
 * Solve a (LU-factorized) system Ax=b by backsubstitution.
 */
void
M_BacksubstLU_FPU(void *pA, void *pb)
{
	const M_MatrixFPU *A=pA;
	M_MatrixFPU *LU = A->LU;
	M_Vector *b=pb;
	M_VectorZ *ivec = A->ivec;
	M_Real sum;
	int i, ip, j;
	int ii = 0;

	for (i = 0; i < MCOLS(LU); i++) {
		ip = ivec->v[i];
		sum = b->v[ip];
		b->v[ip] = b->v[i];
		if (ii) {
			for (j = ii; j <= i-1; j++) {
				sum -= LU->v[i][j] * b->v[j];
			}
		} else if (sum) {
			ii = i;
		}
		b->v[i] = sum;
	}

	for (i = MCOLS(LU)-1; i >= 0; i--) {
		sum = b->v[i];
		for (j = i+1; j < MCOLS(LU); j++) {
			sum -= LU->v[i][j] * b->v[j];
		}
		b->v[i] = sum/LU->v[i][i];
	}
}

/*
 * Perform Gauss-Jordan elimination on a matrix A and a right-hand side b.
 * The original contents of A are destroyed, as it is replaced by the matrix
 * inverse. The solution vectors are returned in b.
 */
static int
M_GaussJordanv_FPU(void *_Nonnull pA, void *_Nonnull pb)
{
	M_MatrixFPU *A=pA, *b=pb;
	M_VectorZ *iCol, *iRow, *iPivot;
	int col = 0, row = 0;
	int i, j, k, l, m;
	M_Real big, dum, pivinv, tmp;

	M_ASSERT_SQUARE_MATRIX(A, -1);
	iRow = M_VectorNewZ(MCOLS(A));
	iCol = M_VectorNewZ(MCOLS(A));
	iPivot = M_VectorNewZ(MCOLS(A));
	M_VectorSetZ(iPivot, 0);

	for (i = 0; i < MCOLS(A); i++) {
		big = 0.0;

		/* Search for the pivot element of this column. */
		for (j = 0; j < MCOLS(A); j++) {
			if (iPivot->v[j] != 1) {
				for (k = 0; k < MCOLS(A); k++) {
					if (iPivot->v[k] == 0) {
						if (Fabs(A->v[j][k]) >= big) {
							big = Fabs(A->v[j][k]);
							row = j;
							col = k;
						}
					} else if (iPivot->v[k] > 1) {
						AG_SetError("Singular matrix");
						goto fail;
					}
				}
			}
		}
		iPivot->v[col]++;

		/* Move the pivot to the diagonal and record the interchange. */
		if (row != col) {
			for (l = 0; l < MCOLS(A); l++)
				SWAP(A->v[row][l], A->v[col][l]);
			for (l = 0; l < MCOLS(b); l++)
				SWAP(b->v[row][l], b->v[col][l]);
		}
		iRow->v[i] = row;
		iCol->v[i] = col;

		if (Fabs(A->v[col][col]) < M_MACHEP) {
			AG_SetError("Matrix singular to machine precision");
			goto fail;
		}
		pivinv = 1.0/A->v[col][col];
		A->v[col][col] = 1.0;

		for (l = 0; l < MCOLS(A); l++) { A->v[col][l] *= pivinv; }
		for (l = 0; l < MCOLS(b); l++) { b->v[col][l] *= pivinv; }

		/* Reduce the rows except for the pivot one. */
		for (m = 0; m < MCOLS(A); m++) {
			if (m == col) {
				continue;
			}
			dum = A->v[m][col];
			A->v[m][col] = 0.0;

			for (l = 0; l < MCOLS(A); l++)
				A->v[m][l] -= A->v[col][l]*dum;
			for (l = 0; l < MCOLS(b); l++)
				b->v[m][l] -= b->v[col][l]*dum;
		}
	}

	for (l = MCOLS(A)-1; l >= 0; l--) {
		if (iRow->v[l] != iCol->v[l]) {
			for (k = 0; k < MCOLS(A); k++)
				SWAP(A->v[k][iRow->v[l]],
				     A->v[k][iCol->v[l]]);
		}
	}

	M_VectorFreeZ(iRow);
	M_VectorFreeZ(iCol);
	M_VectorFreeZ(iPivot);
	return (0);
fail:
	M_VectorFreeZ(iRow);
	M_VectorFreeZ(iCol);
	M_VectorFreeZ(iPivot);
	return (-1);
}
void *
M_GaussJordan_FPU(const void *pA, void *pb)
{
	const M_MatrixFPU *A=pA;
	M_MatrixFPU *b=pb, *Ainv;

	if ((Ainv = M_Dup(A)) == NULL) {
		return (NULL);
	}
	if (M_GaussJordanv_FPU(Ainv, b) == -1) {
		M_Free(Ainv);
		return (NULL);
	}
	return (Ainv);
}
