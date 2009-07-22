/*	Public domain	*/

#include <core/core.h>
#include "m.h"

/*
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
			AG_SetError("Singular matrix (no pivot in column %i)",
			    i);
			fprintf(stderr, "Singular matrix:\n");
			M_MatrixPrint(MMATRIX(A));
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
