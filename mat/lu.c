/*	$Csoft: lu.c,v 1.4 2004/10/29 02:03:55 vedge Exp $	*/
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
int
mat_lu_decompose(struct mat *A, struct veci *ivec, double *d)
{
	double big;
	double sum, dum;
	int i, j, k;
	int imax = 0;
	struct vec *vs;

	if (!mat_is_square(A)) {
		error_set("not a square matrix");
		return (-1);
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
		vs->vec[i] = 1.0/big;
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
			dum = vs->vec[i]*fabs(sum);
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
			vs->vec[imax] = vs->vec[j];
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
	return (0);
fail:
	vec_free(vs);
	return (-1);
}

/*
 * Use backsubstitution to solve the matrix equation A*x=b, where A is the
 * LU decomposition of a row-wise permutation of the original matrix, and
 * vector ivec is the permutation information returned by mat_lu_decompose().
 */
void
mat_lu_backsubst(const struct mat *A, const struct veci *ivec, struct vec *b)
{
	double sum;
	int i, ip, j;
	int ii = 0;

	for (i = 1; i <= A->n; i++) {
		ip = ivec->vec[i];
		sum = b->vec[ip];
		b->vec[ip] = b->vec[i];
		if (ii) {
			for (j = ii; j <= i-1; j++) {
				sum -= A->mat[i][j]*b->vec[j];
			}
		} else if (sum) {
			ii = i;
		}
		b->vec[i] = sum;
	}

	for (i = A->n; i >= 1; i--) {
		sum = b->vec[i];
		for (j = i+1; j <= A->n; j++) {
			sum -= A->mat[i][j]*b->vec[j];
		}
		b->vec[i] = sum/A->mat[i][i];
	}
}

