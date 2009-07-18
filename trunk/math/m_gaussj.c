/*	Public domain	*/

#include <core/core.h>
#include "m.h"

#undef SWAP
#define SWAP(a,b) { tmp=(a); (a)=(b); (b)=tmp; }

/*
 * Apply Gauss-Jordan elimination to a matrix A and a right-hand side b.
 * The original contents of A are destroyed, as it is replaced by the matrix
 * inverse. The solution vectors are returned in b.
 */
int
M_InvertGaussJordanv_FPU(void *pA, void *pb)
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
M_InvertGaussJordan_FPU(const void *pA, void *pb)
{
	const M_MatrixFPU *A=pA;
	M_MatrixFPU *b=pb, *Ainv;

	if ((Ainv = M_Dup(A)) == NULL) {
		return (NULL);
	}
	if (M_InvertGaussJordanv(Ainv, b) == -1) {
		M_Free(Ainv);
		return (NULL);
	}
	return (Ainv);
}
