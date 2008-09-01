/*
 * Public domain.
 * Operations on m*n matrices (SP version).
 */

#include <core/core.h>
#include "m.h"
#include "SPARSE/spmatrix.h"

#define ERRIMPL AG_FatalError("Unimplemented function")

const M_MatrixOps mMatOps_SP = {
	"scalar",
	M_GetElement_SP,
	M_GetValue_SP,
	NULL,			/* AllocEnts */
	NULL,			/* FreeEnts */
	M_MatrixResize_SP,
	M_MatrixFree_SP,
	M_MatrixNew_SP,
	M_MatrixNewZero_SP,
	M_MatrixPrint_FPU,	/* trick to have same output on sp and fpu */
	NULL,			/* SetIdentity */
	M_MatrixSetZero_SP,
	NULL,			/* Transpose */
	NULL,			/* Copy */
	NULL,			/* Dup */
	NULL,			/* Add */
	NULL,			/* Addv */
	NULL,			/* DirectSum */
	NULL,			/* Mul */
	NULL,			/* Mulv */
	NULL,			/* EntMul */
	NULL,			/* Compare */
	NULL,			/* Trace */
	M_MatrixRead_SP,
	M_MatrixWrite_SP,
	NULL,			/* ToFloats */
	NULL,			/* ToDoubles */
	NULL,			/* FromFloats */
	NULL,			/* FromDoubles */
	NULL,			/* InvertGaussJordanv */
	NULL,			/* InvertGaussJordan */
	M_FactorizeLU_SP,
	M_BacksubstLU_SP,
	M_MNAPreorder_SP,
	M_AddToDiag_SP
};

M_Real *
M_GetElement_SP(void *pM, Uint i, Uint j)
{
	M_MatrixSP *M=pM;
	return spGetElement(M->d, i, j);
}

M_Real
M_GetValue_SP(void *pM, Uint i, Uint j)
{
	M_MatrixSP *M=pM;
	M_Real *element = spFindElement(M->d, i, j);
	return element == NULL ? 0.0 : *element;
}

int
M_MatrixResize_SP(void *pA, Uint m, Uint n)
{
	M_MatrixSP *A=pA;
	MROWS(A) = m;
	MCOLS(A) = n;
	/* no resizing of sparse matrices */
	return 0;
}

void
M_MatrixFree_SP(void *pA)
{
	M_MatrixSP *A=pA;
	spDestroy(A->d);
	free(A);
}

void *
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

void *
M_MatrixNewZero_SP(Uint m, Uint n)
{
	return M_MatrixNew_SP(m, n);
}

void
M_MatrixPrint_SP(void *pA)
{
	M_MatrixSP *A = pA;
	spPrint(A->d, 0, 1, 0);
}

void
M_MatrixSetZero_SP(void *pA)
{
	M_MatrixSP *A=pA;
	spClear(A->d);
}

int 
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

void 
M_BacksubstLU_SP(void *pA, void *pV)
{
	M_MatrixSP *A = pA;
	M_VectorFPU *v = pV;
	/* this makes SPARSE solve in place */
	spSolve(A->d, v->v, v->v);
}

void
M_MNAPreorder_SP(void *pA)
{
	M_MatrixSP *A = pA;
	spMNA_Preorder(A->d);
}

void
M_AddToDiag_SP(void *pA, M_Real g)
{
	M_MatrixSP *A = pA;
	spAddToReorderedDiag(A->d, g);
}

void *
M_MatrixRead_SP(AG_DataSource *buf)
{
	ERRIMPL;
	return NULL;
}

void
M_MatrixWrite_SP(AG_DataSource *buf, const void *pA)
{
	ERRIMPL;
}
