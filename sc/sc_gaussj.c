/*	Public domain	*/

#include <core/core.h>

#include "sc.h"

#undef SWAP
#define SWAP(a,b) { tmp=(a); (a)=(b); (b)=tmp; }

/*
 * Find the inverse of matrix A using Gauss-Jordan elimination, where B
 * contains the right-hand side vectors. The original contents of A are
 * destroyed, as it is replaced by the matrix inverse. The solution vectors
 * are returned in B.
 */
void
SC_InvertGaussJordan(SC_Matrix *A, SC_Matrix *B)
{
	SC_Ivector *vicol, *virow, *vipiv;
	int icol = 0, irow = 0;
	int i, j, k, l, m;
	SC_Real big, dum, pivinv, tmp;

	if (!SC_MatrixIsSquare(A))
		fatal("not a square matrix");

	virow = SC_IvectorNew(A->n);
	vicol = SC_IvectorNew(A->n);
	vipiv = SC_IvectorNew(A->n);
	SC_IvectorSetZero(vipiv);

	for (i = 1; i <= A->n; i++) {
		big = 0.0;

		/* Search for a pivot element. */
		for (j = 1; j <= A->n; j++) {
			if (vipiv->vec[j] != 1) {
				for (k = 1; k <= A->n; k++) {
					if (vipiv->vec[k] == 0) {
						if (SC_Fabs(A->mat[j][k]) >=
						    big) {
							big = SC_Fabs(
							    A->mat[j][k]);
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

		for (l = 1; l <= A->n; l++) { A->mat[icol][l] *= pivinv; }
		for (l = 1; l <= B->n; l++) { B->mat[icol][l] *= pivinv; }

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

	SC_IvectorFree(virow);
	SC_IvectorFree(vicol);
	SC_IvectorFree(vipiv);
}
