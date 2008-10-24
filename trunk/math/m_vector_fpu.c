/*
 * Public domain.
 * Operations on vectors in R^n using standard FPU instructions.
 */

#include <core/core.h>
#include "m.h"

const M_VectorOps mVecOps_FPU = {
	"scalar",
	/* Inline */
	M_VectorNew_FPU,
	M_VectorGetElement_FPU,
	M_VectorSetZero_FPU,
	M_VectorResize_FPU,
	M_VectorFree_FPU,
	M_VectorMirror_FPU,
	M_VectorMirrorv_FPU,
	M_VectorScale_FPU,
	M_VectorScalev_FPU,
	M_VectorAdd_FPU,
	M_VectorAddv_FPU,
	M_VectorSub_FPU,
	M_VectorSubv_FPU,
	M_VectorLen_FPU,
	M_VectorDot_FPU,
	M_VectorDistance_FPU,
	M_VectorNorm_FPU,
	M_VectorLERP_FPU,
	M_VectorElemPow_FPU,
	M_VectorCopy_FPU,
	/* Non-inline */
	M_ReadVector_FPU,
	M_WriteVector_FPU,
	M_VectorFromReals_FPU,
	M_VectorFromFloats_FPU,
	M_VectorFromDoubles_FPU,
#ifdef HAVE_LONG_DOUBLE
	M_VectorFromLongDoubles_FPU,
#else
	M_VectorFromDoubles_FPU,	/* Padding */
#endif
	M_VectorPrint_FPU
};

void *
M_ReadVector_FPU(AG_DataSource *buf)
{
	M_VectorFPU *v;
	Uint i, n;

	n = (Uint)AG_ReadUint32(buf);
	v = M_VecNew(n);
	for (i = 0; i < n; i++) {
		v->v[i] = M_ReadReal(buf);
	}
	return (v);
}

void
M_WriteVector_FPU(AG_DataSource *buf, const void *pv)
{
	const M_VectorFPU *v=pv;
	Uint i;

	AG_WriteUint32(buf, (Uint)MVECSIZE(v));
	for (i = 0; i < MVECSIZE(v); i++)
		M_WriteReal(buf, v->v[i]);
}

void *
M_VectorFromReals_FPU(Uint n, const M_Real *r)
{
	M_VectorFPU *a=M_VecNew(n);
	Uint i;

	for (i = 0; i < n; i++) {
		a->v[i] = (M_Real)r[i];
	}
	return (a);
}

void *
M_VectorFromFloats_FPU(Uint n, const float *fv)
{
	M_VectorFPU *a=M_VecNew(n);
	Uint i;

	for (i = 0; i < n; i++) {
		a->v[i] = (M_Real)fv[i];
	}
	return (a);
}

void *
M_VectorFromDoubles_FPU(Uint n, const double *fv)
{
	M_VectorFPU *a=M_VecNew(n);
	Uint i;

	for (i = 0; i < n; i++) {
		a->v[i] = (M_Real)fv[i];
	}
	return (a);
}

#ifdef HAVE_LONG_DOUBLE
void *
M_VectorFromLongDoubles_FPU(Uint n, const long double *fv)
{
	M_VectorFPU *a=M_VecNew(n);
	Uint i;

	for (i = 0; i < n; i++) {
		a->v[i] = (M_Real)fv[i];
	}
	return (a);
}
#endif /* HAVE_LONG_DOUBLE */

void
M_VectorPrint_FPU(const void *pv)
{
	const M_VectorFPU *v = pv;
	Uint i;

	fputs(" ----\n", stdout);
	for (i = 0; i < MVECSIZE(v); i++) {
		printf("|%.3f\n", v->v[i]);
	}
	fputs(" ----\n", stdout);
}
