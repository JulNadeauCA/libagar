/*
 * Operations on m*n matrices (Sparse1.4 version).
 *
 * Sparse1.4 is distributed as open-source software under the Berkeley
 * license model. Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that the following
 * conditions are met:
 * 
 * Redistributions of source code must retain the original copyright notice,
 * this list of conditions and the following disclaimer.  Redistributions
 * in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.  Neither the name of
 * the copyright holder nor the names of the authors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * 
 * This software is provided by the copyright holders and contributors
 * ``as is'' and any express or implied warranties, including, but not
 * limited to, the implied warranties of merchantability and fitness for
 * a particular purpose are disclaimed. In no event shall the copyright
 * owner or contributors be liable for any direct, indirect, incidental,
 * special, exemplary, or consequential damages (including, but not
 * limited to, procurement of substitute goods or services; loss of use,
 * data, or profits; or business interruption) however caused and on any
 * theory of liability, whether in contract, strict liability, or tort
 * (including negligence or otherwise) arising in any way out of the use
 * of this software, even if advised of the possibility of such damage.
 */

#include <agar/core/core.h>
#include <agar/math/m.h>
#include <agar/math/m_sparse.h>

const M_MatrixOps mMatOps_SP = {
	"scalar",
	M_GetElement_SP,
	M_Get_SP,
	M_MatrixResize_SP,
	M_MatrixFree_SP,
	M_MatrixNew_SP,
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
	NULL,			/* EntMulv */
	NULL,			/* Compare */
	NULL,			/* Trace */
	M_MatrixRead_SP,
	M_MatrixWrite_SP,
	NULL,			/* ToFloats */
	NULL,			/* ToDoubles */
	NULL,			/* FromFloats */
	NULL,			/* FromDoubles */
	NULL,			/* GaussJordan */
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
M_Get_SP(void *pM, Uint i, Uint j)
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
	Free(A);
}

void *
M_MatrixNew_SP(Uint m, Uint n)
{
	M_MatrixSP *A;
	int error;

	A = Malloc(sizeof(M_MatrixSP));
	MMATRIX(A)->ops = &mMatOps_SP;
	A->d = spCreate(0, 0, &error);
	MROWS(A) = m;
	MCOLS(A) = n;
	return (A);
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
		spErrorMessage( A->d, stderr, "agar");
		return -1;
	}
	return 0;
}

void 
M_BacksubstLU_SP(void *pA, void *pV)
{
	M_MatrixSP *A = pA;
	M_Vector *v = pV;
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
	AG_FatalError("Unimplemented function");
	return (buf);
}

void
M_MatrixWrite_SP(AG_DataSource *buf, const void *pA)
{
	AG_FatalError("Unimplemented function");
}
