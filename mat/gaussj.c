/*	$Csoft: gaussj.c,v 1.1 2004/11/23 02:32:39 vedge Exp $	*/
/*	Public domain	*/

#include <engine/engine.h>

#include "mat.h"

#undef SWAP
#define SWAP(a,b) { tmp=(a); (a)=(b); (b)=tmp; }

/*
 * Find the inverse of matrix A using Gauss-Jordan elimination, where B
 * contains the right-hand side vectors. The original contents of A are
 * destroyed, as it is replaced by the matrix inverse. The solution vectors
 * are returned in B.
 */
void
mat_gaussj(mat_t *A, mat_t *B)
{
	veci_t *vicol, *virow, *vipiv;
	int icol = 0, irow = 0;
	int i, j, k, l, m;
	double big, dum, pivinv, tmp;

	if (!mat_is_square(A))
		fatal("not a square matrix");

	virow = veci_new(A->n);
	vicol = veci_new(A->n);
	vipiv = veci_new(A->n);
	veci_set(vipiv, 0);

	for (i = 1; i <= A->n; i++) {
		big = 0.0;

		/* Search for a pivot element. */
		for (j = 1; j <= A->n; j++) {
			if (vipiv->vec[j] != 1) {
				for (k = 1; k <= A->n; k++) {
					if (vipiv->vec[k] == 0) {
						if (fabs(A->mat[j][k]) >= big) {
							big = fabs(A->mat[j]
							                 [k]);
							irow = j;
							icol = k;
						}
					} else if (vipiv->vec[k] > 1)
						fatal("singular matrix");
				}
			}
		}
		vipiv->vec[icol] += 1;

		if (irow != icol) {
			for (l = 1; l <= A->n; l++)
				SWAP(A->mat[irow][l], A->mat[icol][l]);
			for (l = 1; l <= B->n; l++)
				SWAP(B->mat[irow][l], B->mat[icol][l]);
		}
		virow->vec[i] = irow;
		vicol->vec[i] = icol;

		if (A->mat[icol][icol] == 0.0)
			fatal("singular matrix");

		pivinv = 1.0/A->mat[icol][icol];
		A->mat[icol][icol] = 1.0;

		for (l = 1; l <= A->n; l++)
			A->mat[icol][l] *= pivinv;
		for (l = 1; l <= B->n; l++)
			B->mat[icol][l] *= pivinv;

		/* Reduce the rows except for the pivot one. */
		for (m = 1; m <= A->n; m++) {
			if (m == icol) {
				continue;
			}
			dum = A->mat[m][icol];
			A->mat[m][icol] = 0.0;

			for (l = 1; l <= A->n; l++)
				A->mat[m][l] -= A->mat[icol][l]*dum;
			for (l = 1; l <= B->n; l++)
				B->mat[m][l] -= B->mat[icol][l]*dum;
		}
	}

	for (l = A->n; l >= 1; l--) {
		if (virow->vec[l] != vicol->vec[l]) {
			for (k = 1; k <= A->n; k++)
				SWAP(A->mat[k][virow->vec[l]],
				     A->mat[k][vicol->vec[l]]);
		}
	}

	veci_free(virow);
	veci_free(vicol);
	veci_free(vipiv);
}
