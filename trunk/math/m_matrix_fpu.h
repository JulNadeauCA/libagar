/*
 * Public domain.
 * Operations on m*n matrices (FPU version).
 */

typedef struct m_matrix_fpu {
	struct m_matrix _inherit;
	M_Real **v;			/* Values */
	struct m_matrix_fpu *LU;
	M_IntVector *ivec;
} M_MatrixFPU;

__BEGIN_DECLS
extern const M_MatrixOps mMatOps_FPU;

int M_MatrixCompare_FPU(const void *, const void *, M_Real *);
int M_MatrixTrace_FPU(M_Real *, const void *);

void M_MatrixPrint_FPU(void *);
void *M_MatrixRead_FPU(AG_DataSource *);
void  M_MatrixWrite_FPU(AG_DataSource *, const void *);

void M_MatrixToFloats_FPU(float *, const void *);
void M_MatrixToDoubles_FPU(double *, const void *);
void M_MatrixFromFloats_FPU(void *, const float *);
void M_MatrixFromDoubles_FPU(void *, const double *);

/* Return pointer to element at i,j */
static __inline__ M_Real *
M_GetElement_FPU(void *pM, Uint i, Uint j)
{
	M_MatrixFPU *M=pM;
	return &(M->v[i][j]);
}

/* Return element at i,j */
static __inline__ M_Real 
M_GetValue_FPU(void *pM, Uint i, Uint j)
{
	M_MatrixFPU *M=pM;
	return (M->v[i][j]);
}

/* Allocate matrix entries. */
static __inline__ int
M_MatrixAllocEnts_FPU(void *pA, Uint m, Uint n)
{
	M_MatrixFPU *A=pA;
	Uint i;

	if (m == 0 && n == 0) {
		A->v = NULL;
		MROWS(A) = 0;
		MCOLS(A) = 0;
		return (0);
	}
	if ((A->v = malloc(m*sizeof(M_Real *))) == NULL) {
		AG_SetError("Out of memory");
		return (-1);
	}
	for (i = 0; i < m; i++) {
		if ((A->v[i] = malloc(n*sizeof(M_Real))) == NULL) {
#if 0
			for (; i >= 0; i--) { free(A->v[i]); }
			free(A->v);
#endif
			AG_SetError("Out of memory");
			return (-1);
		}
	}
	MROWS(A) = m;
	MCOLS(A) = n;
	return (0);
}

/* Free all matrix entries. */
static __inline__ void
M_MatrixFreeEnts_FPU(void *pA)
{
	M_MatrixFPU *A=pA;
	Uint i;

	for (i = 0; i < MROWS(A); i++) {
		free(A->v[i]);
	}
	if(A->v != NULL) free(A->v);
	A->v = NULL;
	MROWS(A) = 0;
	MCOLS(A) = 0;
}

/* Resize a matrix to m*n without initializing new elements. */
static __inline__ int
M_MatrixResize_FPU(void *pA, Uint m, Uint n)
{
	M_MatrixFPU *A=pA;
	M_MatrixFreeEnts_FPU(A);
	return M_MatrixAllocEnts_FPU(A, m,n);
}

/* Free a Matrix object. */
static __inline__ void
M_MatrixFree_FPU(void *pA)
{
	M_MatrixFPU *A=pA;
	M_MatrixFreeEnts_FPU(A);
	free(A);
}

/* Create a new m*n matrix. */
static __inline__ void *
M_MatrixNew_FPU(Uint m, Uint n)
{
	M_MatrixFPU *A;

	A = AG_Malloc(sizeof(M_MatrixFPU));
	MMATRIX(A)->ops = &mMatOps_FPU;
	if (M_MatrixAllocEnts_FPU(A, m,n) == -1) {
		M_MatrixFreeEnts_FPU(A);
		return (NULL);
	}
	A->LU = NULL;
	A->ivec = NULL;
	return (A);
}

/* Create a new m*n matrix initialized to 0. */
static __inline__ void *
M_MatrixNewZero_FPU(Uint m, Uint n)
{
	M_MatrixFPU *A;
	Uint i, j;

	A = AG_Malloc(sizeof(M_MatrixFPU));
	if (M_MatrixAllocEnts_FPU(A, m,n) == -1) {
		M_MatrixFreeEnts_FPU(A);
		return (NULL);
	}
	for (i = 0; i < m; i++) {
		for (j = 0; j < n; j++)
			A->v[i][j] = 0.0;
	}
	return (A);
}

/* Initialize A as the identity matrix. */
static __inline__ void
M_MatrixSetIdentity_FPU(void *pA)
{
	M_MatrixFPU *A=pA;
	Uint i, j;

	for (i = 0; i < MROWS(A); i++)
		for (j = 0; j < MCOLS(A); j++)
			A->v[i][j] = (i==j) ? 1.0 : 0.0;
}

/* Initialize A as the zero matrix. */
static __inline__ void
M_MatrixSetZero_FPU(void *pA)
{
	M_MatrixFPU *A=pA;
	Uint i, j;

	for (i = 0; i < MROWS(A); i++)
		for (j = 0; j < MCOLS(A); j++)
			A->v[i][j] = 0.0;
}

/* Return the transpose of matrix A. */
static __inline__ void *
M_MatrixTranspose_FPU(const void *pA)
{
	const M_MatrixFPU *A=pA;
	M_MatrixFPU *At;
	Uint i, j;

	M_ASSERT_SQUARE_MATRIX(A, NULL);
	if ((At = M_MatrixNew_FPU(MROWS(A), MCOLS(A))) == NULL) {
		return (NULL);
	}
	for (i = 0; i < MROWS(A); i++) {
		for (j = 0; j < MROWS(A); j++)
			At->v[i][j] = A->v[j][i];
	}
	return (At);
}

/* Copy the contents of a matrix into another. */
static __inline__ int
M_MatrixCopy_FPU(void *pB, const void *pA)
{
	M_MatrixFPU *B=pB;
	const M_MatrixFPU *A=pA;
	Uint i, j;

	M_ASSERT_COMPAT_MATRICES(A,B, -1);
	for (i = 0; i < MROWS(A); i++) {
		for (j = 0; j < MCOLS(A); j++) {
			B->v[i][j] = A->v[i][j];
		}
	}
	return (0);
}

/* Return the duplicate of a matrix. */
static __inline__ void *
M_MatrixDup_FPU(const void *pA)
{
	const M_MatrixFPU *A=pA;
	M_MatrixFPU *B;
	
	if ((B = M_MatrixNew_FPU(MROWS(A), MCOLS(A))) == NULL) {
		return (NULL);
	}
	M_MatrixCopy_FPU(B, A);
	return (B);
}

/* Add the individual elements of two m-by-n matrices. */
static __inline__ void *
M_MatrixAdd_FPU(const void *pA, const void *pB)
{
	const M_MatrixFPU *A=pA, *B=pB;
	M_MatrixFPU *P;
	Uint n, m;

	M_ASSERT_COMPAT_MATRICES(A,B, NULL);
	P = M_MatrixNew_FPU(MROWS(A), MCOLS(A));
	for (m = 0; m < MROWS(A); m++) {
		for (n = 0; n < MCOLS(A); n++)
			P->v[m][n] = A->v[m][n] + B->v[m][n];
	}
	return (P);
}

/* Add the individual elements of A and B into A. */
static __inline__ int
M_MatrixAddv_FPU(void *pA, const void *pB)
{
	M_MatrixFPU *A=pA;
	const M_MatrixFPU *B=pB;
	Uint n, m;
	
	M_ASSERT_COMPAT_MATRICES(A,B, -1);
	for (m = 0; m < MROWS(A); m++) {
		for (n = 0; n < MCOLS(A); n++)
			A->v[m][n] += B->v[m][n];
	}
	return (0);
}

/* Compute the direct sum of two matrices. */
static __inline__ void *
M_MatrixDirectSum_FPU(const void *pA, const void *pB)
{
	const M_MatrixFPU *A=pA, *B=pB;
	M_MatrixFPU *P;
	Uint m, n;

	P = M_MatrixNew_FPU(MROWS(A)+MROWS(B), MCOLS(A)+MCOLS(B));
	for (m = 0; m < MROWS(P); m++) {
		for (n = 0; n < MCOLS(P); n++) {
			if (m < MROWS(A) && n < MCOLS(A)) {
				P->v[m][n] = A->v[m][n];
			} else if (m >= MROWS(A) && n >= MCOLS(A)) {
				P->v[m][n] = B->v[m - MROWS(A)][n - MCOLS(A)];
			} else {
				P->v[m][n] = 0.0;
			}
		}
	}
	return (P);
}

/* Return the product of matrices A and B. */
static __inline__ void *
M_MatrixMul_FPU(const void *pA, const void *pB)
{
	const M_MatrixFPU *A=pA, *B=pB;
	Uint i, j, k;
	M_MatrixFPU *AB;
	M_Real sum;

	M_ASSERT_MULTIPLIABLE_MATRICES(A,B, NULL);
	AB = M_MatrixNew_FPU(MROWS(A), MCOLS(B));
	for (i = 0; i < MROWS(A); i++) {
		for (j = 0; j < MCOLS(B); j++) {
			for (sum = 0.0, k = 0; k < MCOLS(A); k++) {
				sum += A->v[i][k] * B->v[k][j];
			}
			AB->v[i][j] = sum;
		}
	}
	return (AB);
}

/* Return the product of matrices A and B into C. */
static __inline__ int
M_MatrixMulv_FPU(const void *pA, const void *pB, void *pC)
{
	const M_MatrixFPU *A=pA, *B=pB;
	M_MatrixFPU *C=pC;
	Uint i, j, k;

	M_ASSERT_MULTIPLIABLE_MATRICES(A,B, -1);
#ifdef DEBUG
	if (MROWS(C) != MCOLS(A) || MCOLS(C) != MCOLS(B)) {
		AG_SetError("C=%dx%d != %dx%d", MROWS(C), MCOLS(C),
		    MCOLS(A), MCOLS(B));
		return (-1);
	}
#endif
	for (i = 0; i < MROWS(A); i++) {
		for (j = 0; j < MCOLS(B); j++) {
			M_Real sum = 0.0;

			for (k = 0; k < MCOLS(A); k++) {
				sum += A->v[i][k] * B->v[k][j];
			}
			C->v[i][j] = sum;
		}
	}
	return (0);
}

/* Return the Hadamard (entrywise) product of m*n matrices A*B into C. */
static __inline__ void *
M_MatrixEntMul_FPU(const void *pA, const void *pB)
{
	const M_MatrixFPU *A=pA, *B=pB;
	M_MatrixFPU *AB;
	Uint i, j;

	M_ASSERT_COMPAT_MATRICES(A,B, NULL);
	AB = M_MatrixNew_FPU(MROWS(A), MCOLS(A));
	for (i = 0; i < MROWS(A); i++) {
		for (j = 0; j < MCOLS(A); j++)
			AB->v[i][j] = A->v[i][j] * B->v[i][j];
	}
	return (AB);
}

static __inline__ void
M_MNAPreorder_FPU(void *A)
{
}

static __inline__ void
M_AddToDiag_FPU(void *pA, M_Real g)
{
	Uint i, N;
	M_MatrixFPU *A=pA;

	N = M_Min(MROWS(A), MCOLS(A));
	for(i = 1; i < N; i++)
		A->v[i][i] += g;
}

__END_DECLS
