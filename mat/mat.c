/*	$Csoft: mat.c,v 1.2 2004/07/29 05:05:38 vedge Exp $	*/

/*
 * Copyright (c) 2004 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <engine/engine.h>

#include "mat.h"

#define assert_same_dimensions(A, B) \
	if ((A)->m != (B)->m || (A)->n != (B)->n) \
	    fatal("matrix A = %dx%d, B = %dx%d", (A)->m, (A)->n, (B)->m, (B)->n)

/* Allocate a new, uninitialized matrix of m rows by n columns. */
struct mat *
mat_new(int m, int n)
{
	struct mat *M;

	M = Malloc(sizeof(struct mat), M_MATH);
	mat_alloc_elements(M, m, n);
	return (M);
}

void
mat_alloc_elements(struct mat *M, int m, int n)
{
	int i;

	if (m != 0 && n != 0) {
		M->mat = Malloc((m+1)*sizeof(double), M_MATH);
		for (i = 1; i <= m; i++) {
			M->mat[i] = Malloc((n+1)*sizeof(double), M_MATH);
		}
	} else {
		M->mat = NULL;
	}
	M->m = m;
	M->n = n;
}

/* Resize a matrix, leaving any new element uninitialized. */
void
mat_resize(struct mat *M, int m, int n)
{
	mat_free_elements(M);
	mat_alloc_elements(M, m, n);
}

/* Initialize all elements of a matrix to v. */
void
mat_set(struct mat *M, double v)
{
	int m, n;

	for (m = 1; m <= M->m; m++)
		for (n = 1; n <= M->n; n++)
			M->mat[m][n] = v;
}

/* Assign the identity matrix. */
void
mat_set_identity(struct mat *M)
{
	int m, n;

	if (!mat_is_square(M))
		fatal("not a square matrix");

	for (m = 1; m <= M->m; m++)
		for (n = 1; n <= M->n; n++)
			M->mat[m][n] = (n == m) ? 1.0 : 0.0;
}

/* Evaluate whether M is the identity matrix. */
int
mat_is_identity(struct mat *M)
{
	int m, n;

	if (!mat_is_square(M))
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

/* Copy the elements of matrix A to matrix B. */
void
mat_copy(const struct mat *A, struct mat *B)
{
	int n, m;
	
	assert_same_dimensions(A, B);

	for (m = 1; m <= A->m; m++)
		for (n = 1; n <= A->n; n++)
			B->mat[m][n] = A->mat[m][n];
}

/* Add the individual elements of two m-by-n matrices. */
void
mat_sum(const struct mat *A, struct mat *B)
{
	int n, m;
	
	assert_same_dimensions(A, B);

	for (m = 1; m <= A->m; m++)
		for (n = 1; n <= A->n; n++)
			B->mat[m][n] = A->mat[m][n] + B->mat[m][n];
}

/*
 * Multiply the m*n matrix A by the n*p matrix B, writing the product
 * in the m*p matrix C.
 *
 * TODO look into Coppersmith & Winograd algorithm to reduce complexity
 * from O(n^3) to O(n^2.376).
 */
void
mat_mul(const struct mat *A, const struct mat *B, struct mat *C)
{
	int i, j, k;

	if (A->n != B->m)
		fatal("A = %d columns, B = %d rows", A->n, B->m);
	if (C->m != A->n || C->n != B->n)
		fatal("C = %dx%d != %dx%d", C->m, C->n, A->n, B->n);

	for (i = 1; i <= A->m; i++) {
		for (j = 1; j <= B->n; j++) {
			double sum = 0.0;

			for (k = 1; k <= A->n; k++) {
				sum += A->mat[i][k] * B->mat[k][j];
			}
			C->mat[i][j] = sum;
		}
	}
}

/* Calculate the Hadamard product of m*n matrices A*B into C. */
void
mat_entmul(const struct mat *A, const struct mat *B, struct mat *C)
{
	int i, j;

	assert_same_dimensions(A, B);
	assert_same_dimensions(B, C);

	for (i = 1; i <= A->m; i++)
		for (j = 1; j <= A->n; j++)
			C->mat[i][j] = A->mat[i][j]*B->mat[i][j];
}

void
mat_free_elements(struct mat *M)
{
	int m;

	for (m = 1; m <= M->m; m++) {
		Free(M->mat[m], M_MATH);
	}
	Free(M->mat, M_MATH);
}

void
mat_free(struct mat *M)
{
	mat_free_elements(M);
	Free(M, M_MATH);
}

/* Evaluate the squareness of a matrix. */
int
mat_is_square(struct mat *M)
{
	return (M->m == M->n);
}

#ifdef DEBUG
void
mat_print(const struct mat *M)
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
mat_test(void)
{
	struct mat *A;
	struct vec *c;
	struct veci *iv;
	int i, j;
	double d;

	A = mat_new(2, 2);
	A->mat[1][1] =  2.0;
	A->mat[2][1] =  5.0;
	A->mat[1][2] = -1.0;
	A->mat[2][2] = -3.0;
	printf("A: \n"); mat_print(A);

	c = vec_new(2);
	c->vec[1] = 7.0;
	c->vec[2] = 18.0;

	iv = veci_new(2);
	mat_lu_decompose(A, iv, &d);
	printf("A (LU-decomposed):\n"); mat_print(A);

	mat_lu_backsubst(A, iv, c);
	printf("Solution vector:\n"); vec_print(c);

	mat_free(A);
	vec_free(c);
	veci_free(iv);
}
#endif /* DEBUG */
