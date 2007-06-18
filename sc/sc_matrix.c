/*
 * Copyright (c) 2004-2007 Hypertriton, Inc. <http://hypertriton.com/>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <core/core.h>

#include "sc.h"

#define assert_same_dimensions(A, B) \
	if ((A)->m != (B)->m || (A)->n != (B)->n) \
	    fatal("matrix A = %dx%d, B = %dx%d", (A)->m, (A)->n, (B)->m, (B)->n)

/* Allocate a new, uninitialized matrix of m rows by n columns. */
SC_Matrix *
SC_MatrixNew(Uint m, Uint n)
{
	SC_Matrix *M;

	M = Malloc(sizeof(SC_Matrix), M_MATH);
	SC_MatrixAlloc(M, m, n);
	return (M);
}

void
SC_MatrixAlloc(SC_Matrix *M, Uint m, Uint n)
{
	Uint i;

	if (m != 0 && n != 0) {
		M->mat = Malloc((m+1)*sizeof(SC_Real), M_MATH);
		for (i = 1; i <= m; i++) {
			M->mat[i] = Malloc((n+1)*sizeof(SC_Real), M_MATH);
		}
	} else {
		M->mat = NULL;
	}
	M->m = m;
	M->n = n;
}

/* Resize a matrix, leaving any new element uninitialized. */
void
SC_MatrixResize(SC_Matrix *M, Uint m, Uint n)
{
	SC_MatrixFreeElements(M);
	SC_MatrixAlloc(M, m, n);
}

SC_Real
SC_MatrixGetEntry(const SC_Matrix *A, Uint i, Uint j)
{
	SC_AssertEntry(A,i,j);
	return (A->mat[i][j]);
}

SC_Real *
SC_MatrixGetEntryp(const SC_Matrix *A, Uint i, Uint j)
{
	SC_AssertEntry(A,i,j);
	return (&A->mat[i][j]);
}

int
SC_MatrixEntryExists(const SC_Matrix *A, Uint i, Uint j)
{
	return (i > 0 && i <= A->m &&
	        j > 0 && j <= A->n);
}

/* Assign the identity matrix. */
void
SC_MatrixSetIdentity(SC_Matrix *M)
{
	Uint m, n;

	if (!SC_MatrixIsSquare(M))
		fatal("not a square matrix");

	for (m = 1; m <= M->m; m++)
		for (n = 1; n <= M->n; n++)
			M->mat[m][n] = (n == m) ? 1.0 : 0.0;
}

/* Initialize all elements of a matrix to 0. */
void
SC_MatrixSetZero(SC_Matrix *M)
{
	Uint m, n;

	for (m = 1; m <= M->m; m++)
		for (n = 1; n <= M->n; n++)
			M->mat[m][n] = 0.0;
}

/* Compose supermatrix A from submatrices [G;B]. */
void
SC_MatrixCompose21(SC_Matrix *A, const SC_Matrix *G, const SC_Matrix *B)
{
	Uint m, n;

	for (m = 1; m <= G->m; m++) {
		for (n = 1; n <= G->n; n++)
			A->mat[m][n] = G->mat[m][n];
	}
	for (m = 1; m <= B->m; m++) {
		for (n = 1; n <= B->n; n++)
			A->mat[G->m+m][n] = B->mat[m][n];
	}
}

/* Compose supermatrix A from submatrices [G,B]. */
void
SC_MatrixCompose12(SC_Matrix *A, const SC_Matrix *G, const SC_Matrix *B)
{
	Uint m, n;

	for (m = 1; m <= G->m; m++) {
		for (n = 1; n <= G->n; n++)
			A->mat[m][n] = G->mat[m][n];
	}
	for (m = 1; m <= B->m; m++) {
		for (n = 1; n <= B->n; n++)
			A->mat[m][G->n+n] = B->mat[m][n];
	}
}

/* Compose supermatrix A from submatrices [G,B;C,D]. */
void
SC_MatrixCompose22(SC_Matrix *A, const SC_Matrix *G, const SC_Matrix *B, const SC_Matrix *C,
    const SC_Matrix *D)
{
	Uint m, n;

	for (m = 1; m <= G->m; m++) {
		for (n = 1; n <= G->n; n++)
			A->mat[m][n] = G->mat[m][n];
	}
	for (m = 1; m <= B->m; m++) {
		for (n = 1; n <= B->n; n++)
			A->mat[m][G->n+n] = B->mat[m][n];
	}
	for (m = 1; m <= C->m; m++) {
		for (n = 1; n <= C->n; n++)
			A->mat[G->m+m][n] = C->mat[m][n];
	}
	for (m = 1; m <= D->m; m++) {
		for (n = 1; n <= D->n; n++)
			A->mat[G->m+m][G->n+n] = D->mat[m][n];
	}
}

/* Evaluate whether M is the identity matrix. */
int
SC_MatrixIsIdentity(const SC_Matrix *M)
{
	Uint m, n;

	if (!SC_MatrixIsSquare(M))
		return (0);

	for (m = 1; m <= M->m; m++) {
		for (n = 1; n <= M->n; n++) {
			if (m == n) {
				if (M->mat[m][n] != 1.0)
					return (0);
			} else {
				if (M->mat[m][n] != 0.0)
					return (0);
			}
		}
	}
	return (1);
}

/* Evaluate whether M is a lower-triangular matrix. */
int
SC_MatrixIsLowTri(const SC_Matrix *M)
{
	Uint m, n;

	for (m = 1; m <= M->m; m++) {
		for (n = m+1; n <= M->n; n++) {
			if (M->mat[m][n] != 0.0)
				return (0);
		}
	}
	return (1);
}

/* Evaluate whether M is a normed lower-triangular matrix. */
int
SC_MatrixIsLowTriNormed(const SC_Matrix *M)
{
	Uint m, n;

	for (m = 1; m <= M->m; m++) {
		if (M->mat[m][m] != 1.0) {
			return (0);
		}
		for (n = m+1; n <= M->n; n++) {
			if (M->mat[m][n] != 0.0)
				return (0);
		}
	}
	return (1);
}

/* Evaluate whether M is a strictly lower-triangular matrix. */
int
SC_MatrixIsLowTriStrict(const SC_Matrix *M)
{
	Uint m, n;

	for (m = 1; m <= M->m; m++) {
		for (n = m; n <= M->n; n++) {
			if (M->mat[m][n] != 0.0)
				return (0);
		}
	}
	return (1);
}

/* Evaluate whether M is an upper-triangular matrix. */
int
SC_MatrixIsUpTri(const SC_Matrix *M)
{
	Uint m, n;

	for (m = 1; m <= M->m; m++) {
		for (n = m-1; n >= 1; n--) {
			if (M->mat[m][n] != 0.0)
				return (0);
		}
	}
	return (1);
}

/* Evaluate whether M is a normed upper-triangular matrix. */
int
SC_MatrixIsUpTriNormed(const SC_Matrix *M)
{
	Uint m, n;

	for (m = 1; m <= M->m; m++) {
		if (M->mat[m][m] != 1.0) {
			return (0);
		}
		for (n = m-1; n >= 1; n--) {
			if (M->mat[m][n] != 0.0)
				return (0);
		}
	}
	return (1);
}

/* Evaluate whether M is a strictly upper-triangular matrix. */
int
SC_MatrixIsUpTriStrict(const SC_Matrix *M)
{
	Uint m, n;

	for (m = 1; m <= M->m; m++) {
		for (n = m; n >= 1; n--) {
			if (M->mat[m][n] != 0.0)
				return (0);
		}
	}
	return (1);
}

/* Evaluate whether M is the zero matrix. */
int
SC_MatrixIsZero(const SC_Matrix *M)
{
	Uint m, n;

	for (m = 1; m <= M->m; m++) {
		for (n = 1; n <= M->n; n++) {
			if (M->mat[m][n] != 0.0)
				return (0);
		}
	}
	return (1);
}

/* Evaluate whether A is symmetric. */
int
SC_MatrixIsSymmetric(const SC_Matrix *A)
{
	Uint m, n;

	if (!SC_MatrixIsSquare(A))
		return (0);
	
	for (m = 1; m <= A->m; m++) {
		for (n = 1; n <= A->n; n++) {
			if (A->mat[m][n] != A->mat[n][m])
				return (0);
		}
	}
	return (1);
}

/* Copy the elements of matrix A to matrix B. */
void
SC_MatrixCopy(const SC_Matrix *A, SC_Matrix *B)
{
	Uint n, m;
	
	assert_same_dimensions(A, B);

	for (m = 1; m <= A->m; m++)
		for (n = 1; n <= A->n; n++)
			B->mat[m][n] = A->mat[m][n];
}

/* Compare two matrices */
int
SC_MatrixCompare(const SC_Matrix *A, const SC_Matrix *B)
{
	Uint n, m;

	if (A->m != B->m || A->n != B->n) {
		return (1);
	}
	for (m = 1; m <= A->m; m++) {
		for (n = 1; n <= A->n; n++) {
			if (A->mat[m][n] != B->mat[m][n]) {
				return (1);
			}
		}
	}
	return (0);
}

SC_Matrix *
SC_MatrixDup(const SC_Matrix *A)
{
	SC_Matrix *B;
	Uint m, n;

	B = SC_MatrixNew(A->m, A->n);
	for (m = 1; m <= A->m; m++) {
		for (n = 1; n <= A->n; n++)
			B->mat[m][n] = A->mat[m][n];
	}
	return (B);
}

/* Add the individual elements of two m-by-n matrices. */
void
SC_MatrixSum(const SC_Matrix *A, SC_Matrix *B)
{
	Uint n, m;
	
	assert_same_dimensions(A, B);

	for (m = 1; m <= A->m; m++)
		for (n = 1; n <= A->n; n++)
			B->mat[m][n] = A->mat[m][n] + B->mat[m][n];
}

/* Compute the direct sum of two matrices. */
SC_Matrix *
SC_MatrixDirectSum(const SC_Matrix *A, const SC_Matrix *B)
{
	SC_Matrix *P;
	Uint m, n;

	P = SC_MatrixNew(A->m+B->m, A->n+B->n);
	for (m = 1; m <= P->m; m++) {
		for (n = 1; n <= P->n; n++) {
			if (m <= A->m && n <= A->n) {
				P->mat[m][n] = A->mat[m][n];
			} else if (m > A->m && n > A->n) {
				P->mat[m][n] = B->mat[m - A->m][n - A->n];
			} else {
				P->mat[m][n] = 0.0;
			}
		}
	}
	return (P);
}

/* Return the transpose of A. */
SC_Matrix *
SC_MatrixTranspose(const SC_Matrix *A, SC_Matrix *At)
{
	Uint m, n;
	SC_Matrix *T;

	if (At != NULL) {
#ifdef DEBUG
		if (At->m != A->n || At->n != A->m)
			fatal("matrix has incorrect dimensions");
#endif
		T = At;
	} else {
		T = SC_MatrixNew(A->n, A->m);
	}
	
	/* TODO optimize by block copying the rows */
	for (m = 1; m <= A->m; m++) {
		for (n = 1; n <= A->n; n++)
			T->mat[n][m] = A->mat[m][n];
	}
	return (T);
}

/* Return the trace of [A]. */
SC_Real
SC_MatrixTrace(const SC_Matrix *A)
{
	SC_Real sum = 0.0;
	Uint n;

	if (!SC_MatrixIsSquare(A))
		fatal("not a square matrix");

	for (n = 1; n <= A->n; n++) {
		sum += A->mat[n][n];
	}
	return (sum);
}

/*
 * Multiply the m*n matrix A by the n*p matrix B, writing the product
 * in the m*p matrix C.
 */
void
SC_MatrixMulv(const SC_Matrix *A, const SC_Matrix *B, SC_Matrix *C)
{
	Uint i, j, k;

	if (A->n != B->m)
		fatal("A(n)=%d != B(m)=%d", A->n, B->m);
	if (C->m != A->n || C->n != B->n)
		fatal("C=%dx%d != %dx%d", C->m, C->n, A->n, B->n);

	for (i = 1; i <= A->m; i++) {
		for (j = 1; j <= B->n; j++) {
			SC_Real sum = 0.0;

			for (k = 1; k <= A->n; k++) {
				sum += A->mat[i][k] * B->mat[k][j];
			}
			C->mat[i][j] = sum;
		}
	}
}

/* Calculate the Hadamard (entrywise) product of m*n matrices A*B into C. */
void
SC_MatrixEntrywiseMul(const SC_Matrix *A, const SC_Matrix *B, SC_Matrix *C)
{
	Uint i, j;

	assert_same_dimensions(A, B);
	assert_same_dimensions(B, C);

	for (i = 1; i <= A->m; i++)
		for (j = 1; j <= A->n; j++)
			C->mat[i][j] = A->mat[i][j]*B->mat[i][j];
}

void
SC_MatrixFreeElements(SC_Matrix *M)
{
	int m;

	if (M->n < 1) {
		return;
	}
	for (m = 1; m <= M->m; m++) {
		Free(M->mat[m], M_MATH);
	}
	Free(M->mat, M_MATH);
}

void
SC_MatrixFree(SC_Matrix *M)
{
	SC_MatrixFreeElements(M);
	Free(M, M_MATH);
}

/* Evaluate the squareness of a matrix. */
int
SC_MatrixIsSquare(const SC_Matrix *M)
{
	return (M->m == M->n);
}

void
SC_WriteMatrix(SC_Matrix *A, AG_Netbuf *buf)
{
	Uint i, j;

	AG_WriteUint32(buf, A->m);
	AG_WriteUint32(buf, A->n);
	for (i = 1; i <= A->m; i++) {
		for (j = 1; j <= A->n; j++)
			SC_WriteReal(buf, A->mat[i][j]);
	}
}

SC_Matrix *
SC_ReadMatrix(AG_Netbuf *buf)
{
	SC_Matrix *A;
	Uint m, n;
	Uint i, j;

	m = AG_ReadUint32(buf);
	n = AG_ReadUint32(buf);
	A = SC_MatrixNew(m, n);
	for (i = 1; i <= m; i++) {
		for (j = 1; j <= n; j++)
			A->mat[i][j] = SC_ReadReal(buf);
	}
	return (A);
}

#ifdef DEBUG
void
SC_MatrixPrint(const SC_Matrix *M)
{
	int m, n;

	fputs(" -\n", stdout);
	for (m = 1; m <= M->m; m++) {
		fputs("| ", stdout);
		for (n = 1; n <= M->n; n++) {
			if (M->mat[m][n] >= 0) {
				fputc(' ', stdout);
			}
			printf("%.02f ", M->mat[m][n]);
		}
		fputs("|\n", stdout);
	}
	fputs(" -\n", stdout);
}

void
SC_MatrixTest(void)
{
	SC_Matrix *A, *Alu;
	SC_Vector *c;
	SC_Ivector *iv;
	int i, j;
	SC_Real d;

	A = SC_MatrixNew(2, 2);
	A->mat[1][1] =  2.0;
	A->mat[2][1] =  5.0;
	A->mat[1][2] = -1.0;
	A->mat[2][2] = -3.0;
	printf("A: \n"); SC_MatrixPrint(A);

	c = SC_VectorNew(2);
	c->mat[1][1] = 7.0;
	c->mat[2][1] = 18.0;

	iv = SC_IvectorNew(2);
	if ((Alu = SC_FactorizeLU(A, NULL, iv, &d)) == NULL) {
		printf("singular matrix!\n");
		goto out;
	}
	printf("A (LU-decomposed):\n"); SC_MatrixPrint(Alu);
	SC_BacksubstLU(Alu, iv, c);
	printf("Solution vector:\n"); SC_VectorPrint(c);
	SC_MatrixFree(Alu);
out:
	SC_MatrixFree(A);
	SC_VectorFree(c);
	SC_IvectorFree(iv);
}
#endif /* DEBUG */
