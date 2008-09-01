/*
 * Public domain.
 * Operations on m*n matrices (SP version).
 */

#include <core/core.h>
#include "m.h"

const M_MatrixOps mMatOps_SP = {
	"scalar",
	/* Inline */
	M_GetElement_SP,
	M_GetValue_SP,
	NULL,
	NULL,
	M_MatrixResize_SP,
	M_MatrixFree_SP,
	M_MatrixNew_SP,
	M_MatrixNewZero_SP,
	M_MatrixPrint_FPU, /* trick to have same output on sp and fpu */
	M_MatrixSetIdentity_SP,
	M_MatrixSetZero_SP,
	M_MatrixTranspose_SP,
	M_MatrixCopy_SP,
	M_MatrixDup_SP,
	M_MatrixAdd_SP,
	M_MatrixAddv_SP,
	M_MatrixDirectSum_SP,
	M_MatrixMul_SP,
	M_MatrixMulv_SP,
	M_MatrixEntMul_SP,
	/* Not inline */
	M_MatrixCompare_SP,
	M_MatrixTrace_SP,
	M_MatrixRead_SP,
	M_MatrixWrite_SP,
	M_MatrixToFloats_SP,
	M_MatrixToDoubles_SP,
	M_MatrixFromFloats_SP,
	M_MatrixFromDoubles_SP,

	NULL, /* gauss jordan */
	NULL,
	M_FactorizeLU_SP,
	M_BacksubstLU_SP,
	M_MNAPreorder_SP,
	M_AddToDiag_SP
};

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

/* Compare two matrices entrywise and return the largest difference. */
int
M_MatrixCompare_SP(const void *pA, const void *pB, M_Real *diff)
{
	ERRIMPL;
	return 0;
}

/* Return the trace of matrix A. */
int
M_MatrixTrace_SP(M_Real *sum, const void *pA)
{
	ERRIMPL;
	return 0;
}

/* Convert matrix A to an array of floats. */
void
M_MatrixToFloats_SP(float *fv, const void *pA)
{
	ERRIMPL;
}

/* Convert matrix A to an array of doubles. */
void
M_MatrixToDoubles_SP(double *dv, const void *pA)
{
	ERRIMPL;
}

/* Load matrix A from an array of floats. */
void
M_MatrixFromFloats_SP(void *pA, const float *fv)
{
	ERRIMPL;
}

/* Load matrix A from an array of doubles. */
void
M_MatrixFromDoubles_SP(void *pA, const double *fv)
{
	ERRIMPL;
}
