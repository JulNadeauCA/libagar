/*	Public domain	*/

#include <core/core.h>

#include "sc.h"

/*
 * Decompose a square matrix A into a product of the upper-triangular
 * matrix U and the lower-triangular matrix L, following a row-wise
 * permutation. The partial pivoting information is recorded in ivec,
 * and d is set to 1 or -1 depending on whether the number of interchanges
 * turned out to be even or odd.
 */
SC_Matrix *
SC_FactorizeLU(const SC_Matrix *pA, SC_Matrix *dA, SC_Ivector *ivec, SC_Real *d)
{
	SC_Real big;
	SC_Real sum, dum, a;
	int i, j, k;
	int imax = 0;
	SC_Vector *vs;
	SC_Matrix *A, *Adup = NULL;

	if (!SC_MatrixIsSquare(pA)) {
		AG_SetError("not a square matrix");
		return (NULL);
	}
	if (dA != NULL) {
		SC_MatrixCopy(pA, dA);
		A = dA;
		Adup = NULL;
	} else {
		A = SC_MatrixDup(pA);
		Adup = A;
	}
	vs = SC_VectorNew(A->n);
	*d = 1.0;

	/* Generate implicit scaling information. */
	for (i = 1; i <= A->n; i++) {
		big = 0.0;
		for (j = 1; j <= A->n; j++) {
			a = SC_Fabs(A->mat[i][j]);
			if (a > big) { big = a; }
		}
		if (big == 0.0) {
			AG_SetError("singular matrix");
			goto fail;
		}
		vs->mat[i][1] = 1.0/big;
	}

	/* Solve the set of equations using Crout's algorithm. */
	for (j = 1; j <= A->n; j++) {
		for (i = 1; i < j; i++) {
			sum = A->mat[i][j];
			for (k = 1; k < i; k++) {
				sum -= A->mat[i][k]*A->mat[k][j];
			}
			A->mat[i][j] = sum;
		}

		big = 0.0;
		for (i = j; i <= A->n; i++) {
			sum = A->mat[i][j];
			for (k = 1; k < j; k++) {
				sum -= A->mat[i][k]*A->mat[k][j];
			}
			A->mat[i][j] = sum;
			dum = vs->mat[i][1]*SC_Fabs(sum);
			if (dum >= big) {
				big = dum;
				imax = i;
			}
		}

		/* Interchange rows if necessary. */
		if (j != imax) {
			for (k = 1; k <= A->n; k++) {
				dum = A->mat[imax][k];
				A->mat[imax][k] = A->mat[j][k];
				A->mat[j][k] = dum;
			}
			*d = -(*d);
			vs->mat[imax][1] = vs->mat[j][1];
		}
		ivec->vec[j] = imax;
		if (A->mat[j][j] == 0.0) { A->mat[j][j] = SC_TINY_VAL; }
		
		/* Divide by the pivot element. */
		if (j != A->n) {
			dum = 1.0/A->mat[j][j];
			for (i = j+1; i <= A->n; i++)
				A->mat[i][j] *= dum;
		}
	}
	SC_VectorFree(vs);
	return (A);
fail:
	if (Adup != NULL) {
		SC_MatrixFree(Adup);
	}
	SC_VectorFree(vs);
	return (NULL);
}

/*
 * Use backsubstitution to solve the matrix equation A*x=b, where A is the
 * LU decomposition of a row-wise permutation of the original matrix, and
 * vector ivec is the permutation information returned by SC_FactorizeLU().
 */
void
SC_BacksubstLU(const SC_Matrix *A, const SC_Ivector *ivec, SC_Vector *b)
{
	SC_Real sum;
	int i, ip, j;
	int ii = 0;

	for (i = 1; i <= A->n; i++) {
		ip = ivec->vec[i];
		sum = b->mat[ip][1];
		b->mat[ip][1] = b->mat[i][1];
		if (ii) {
			for (j = ii; j <= i-1; j++) {
				sum -= A->mat[i][j] * b->mat[j][1];
			}
		} else if (sum) {
			ii = i;
		}
		b->mat[i][1] = sum;
	}

	for (i = A->n; i >= 1; i--) {
		sum = b->mat[i][1];
		for (j = i+1; j <= A->n; j++) {
			sum -= A->mat[i][j] * b->mat[j][1];
		}
		b->mat[i][1] = sum/A->mat[i][i];
	}
}

