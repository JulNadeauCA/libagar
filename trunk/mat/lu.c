/*	$Csoft: lu.c,v 1.2 2005/09/10 05:06:06 vedge Exp $	*/
/*	Public domain	*/

#include <engine/engine.h>

#include "mat.h"

/*
 * Decompose a square matrix A into a product of the upper-triangular
 * matrix U and the lower-triangular matrix L, following a row-wise
 * permutation. The partial pivoting information is recorded in ivec,
 * and d is set to 1 or -1 depending on whether the number of interchanges
 * turned out to be even or odd.
 */
mat_t *
mat_lu_decompose(const mat_t *pA, mat_t *dA, veci_t *ivec, double *d)
{
	double big;
	double sum, dum;
	int i, j, k;
	int imax = 0;
	vec_t *vs;
	mat_t *A, *Adup = NULL;

	if (!mat_is_square(pA)) {
		error_set("not a square matrix");
		return (NULL);
	}
	if (dA != NULL) {
		mat_copy(pA, dA);
		A = dA;
		Adup = NULL;
	} else {
		A = mat_dup(pA);
		Adup = A;
	}
	vs = vec_new(A->n);
	*d = 1.0;

	/* Generate implicit scaling information. */
	for (i = 1; i <= A->n; i++) {
		big = 0.0;
		for (j = 1; j <= A->n; j++) {
			double a = fabs(A->mat[i][j]);

			if (a > big)
				big = a;
		}
		if (big == 0.0) {
			error_set("singular matrix");
			goto fail;
		}
		vs->mat[i][0] = 1.0/big;
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
			dum = vs->mat[i][0]*fabs(sum);
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
			vs->mat[imax][0] = vs->mat[j][0];
		}
		ivec->vec[j] = imax;
		if (A->mat[j][j] == 0.0)
			A->mat[j][j] = TINY_VAL;
		
		/* Divide by the pivot element. */
		if (j != A->n) {
			dum = 1.0/A->mat[j][j];
			for (i = j+1; i <= A->n; i++)
				A->mat[i][j] *= dum;
		}
	}
	vec_free(vs);
	return (A);
fail:
	if (Adup != NULL) {
		mat_free(Adup);
	}
	vec_free(vs);
	return (NULL);
}

/*
 * Use backsubstitution to solve the matrix equation A*x=b, where A is the
 * LU decomposition of a row-wise permutation of the original matrix, and
 * vector ivec is the permutation information returned by mat_lu_decompose().
 */
void
mat_lu_backsubst(const mat_t *A, const veci_t *ivec, vec_t *b)
{
	double sum;
	int i, ip, j;
	int ii = 0;

	for (i = 1; i <= A->n; i++) {
		ip = ivec->vec[i];
		sum = b->mat[ip][0];
		b->mat[ip][0] = b->mat[i][0];
		if (ii) {
			for (j = ii; j <= i-1; j++) {
				sum -= A->mat[i][j] * b->mat[j][0];
			}
		} else if (sum) {
			ii = i;
		}
		b->mat[i][0] = sum;
	}

	for (i = A->n; i >= 1; i--) {
		sum = b->mat[i][0];
		for (j = i+1; j <= A->n; j++) {
			sum -= A->mat[i][j] * b->mat[j][0];
		}
		b->mat[i][0] = sum/A->mat[i][i];
	}
}

