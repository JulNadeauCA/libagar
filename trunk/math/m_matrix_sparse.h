/*
 * Public domain.
 * Operations on m*n matrices (SPARSE version).
 */
#include "SPARSE/spmatrix.h"

typedef struct m_matrix_sp {
	struct m_matrix _inherit;
	char *d; /* data to be used by SPARSE */
} M_MatrixSP;

#define ERRIMPL printf("Function at file %s, line %d is not implemented", __FILE__, __LINE__)

__BEGIN_DECLS
extern const M_MatrixOps mMatOps_SP;

int M_MatrixCompare_SP(const void *, const void *, M_Real *);
int M_MatrixTrace_SP(M_Real *, const void *);

void *M_MatrixRead_SP(AG_DataSource *);
void  M_MatrixWrite_SP(AG_DataSource *, const void *);

void M_MatrixToFloats_SP(float *, const void *);
void M_MatrixToDoubles_SP(double *, const void *);
void M_MatrixFromFloats_SP(void *, const float *);
void M_MatrixFromDoubles_SP(void *, const double *);

/* Return pointer to element at i,j */
static __inline__ M_Real *
M_GetElement_SP(void *pM, Uint i, Uint j)
{
	M_MatrixSP *M=pM;
	return spGetElement(M->d, i, j);
}

/* Return value of M(i,j) */
static __inline__ M_Real
M_GetValue_SP(void *pM, Uint i, Uint j)
{
	M_MatrixSP *M=pM;
	M_Real *element = spFindElement(M->d, i, j);
	return element == NULL ? 0.0 : *element;
}

/* Resize a matrix to m*n without initializing new elements. */
static __inline__ int
M_MatrixResize_SP(void *pA, Uint m, Uint n)
{
	M_MatrixSP *A=pA;
	MROWS(A) = m;
	MCOLS(A) = n;
	/* no resizing of sparse matrices */
	return 0;
}

/* Free a Matrix object. */
static __inline__ void
M_MatrixFree_SP(void *pA)
{
	M_MatrixSP *A=pA;
	spDestroy(A->d);
	free(A);
}

/* Create a new m*n matrix. */
static __inline__ void *
M_MatrixNew_SP(Uint m, Uint n)
{
	M_MatrixSP *A;
	int error;

	A = AG_Malloc(sizeof(M_MatrixSP));
	MMATRIX(A)->ops = &mMatOps_SP;
	A->d = spCreate(0, 0, &error);
	MROWS(A) = m;
	MCOLS(A) = n;
	return (A);
}

/* Create a new m*n matrix initialized to 0. */
static __inline__ void *
M_MatrixNewZero_SP(Uint m, Uint n)
{
	return M_MatrixNew_SP(m, n);
}

/* Print matrix */
static __inline__ void
M_MatrixPrint_SP(void *pA)
{
	M_MatrixSP *A = pA;
	spPrint(A->d, 0, 1, 0);
}

/* Initialize A as the identity matrix. */
static __inline__ void
M_MatrixSetIdentity_SP(void *pA)
{
	ERRIMPL;
}

/* Initialize A as the zero matrix. */
static __inline__ void
M_MatrixSetZero_SP(void *pA)
{
	M_MatrixSP *A=pA;
	spClear(A->d);
}

/* Return the transpose of matrix A. */
static __inline__ void *
M_MatrixTranspose_SP(const void *pA)
{
	ERRIMPL;
	return NULL;
}

/* Copy the contents of a matrix into another. */
static __inline__ int
M_MatrixCopy_SP(void *pB, const void *pA)
{
	ERRIMPL;
	return 0;
}

/* Return the duplicate of a matrix. */
static __inline__ void *
M_MatrixDup_SP(const void *pA)
{
	ERRIMPL;
	return NULL;
}

/* Add the individual elements of two m-by-n matrices. */
static __inline__ void *
M_MatrixAdd_SP(const void *pA, const void *pB)
{
	ERRIMPL;
	return NULL;
}

/* Add the individual elements of A and B into A. */
static __inline__ int
M_MatrixAddv_SP(void *pA, const void *pB)
{
	ERRIMPL;
	return 0;
}

/* Compute the direct sum of two matrices. */
static __inline__ void *
M_MatrixDirectSum_SP(const void *pA, const void *pB)
{
	ERRIMPL;
	return NULL;
}

/* Return the product of matrices A and B. */
static __inline__ void *
M_MatrixMul_SP(const void *pA, const void *pB)
{
	ERRIMPL;
	return NULL;
}

/* Return the product of matrices A and B into C. */
static __inline__ int
M_MatrixMulv_SP(const void *pA, const void *pB, void *pC)
{
	ERRIMPL;
	return 0;
}

/* Return the Hadamard (entrywise) product of m*n matrices A*B into C. */
static __inline__ void *
M_MatrixEntMul_SP(const void *pA, const void *pB)
{
	ERRIMPL;
	return NULL;
}


static __inline__ int 
M_FactorizeLU_SP(void *pA)
{
	M_MatrixSP *A = pA;
	int err;
	err = spFactor(A->d);
	if(err >= spFATAL) {
		spErrorMessage( A->d, stderr, "edacious");
		return -1;
	}
	return 0;
}
static __inline__ void 
M_BacksubstLU_SP(void *pA, void *pV)
{
	M_MatrixSP *A = pA;
	M_VectorFPU *v = pV;
	/* this makes SPARSE solve in place */
	spSolve(A->d, v->v, v->v);
}

static __inline__ void
M_MNAPreorder_SP(void *pA)
{
	M_MatrixSP *A = pA;
	spMNA_Preorder(A->d);
}

static __inline__ void
M_AddToDiag_SP(void *pA, M_Real g)
{
	M_MatrixSP *A = pA;
	spAddToReorderedDiag(A->d, g);
}


__END_DECLS
