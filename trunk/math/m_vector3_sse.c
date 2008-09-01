/*
 * Public domain.
 * Operations on vectors in R^3 using SSE1 operations.
 */

#include <config/have_sse.h>

#ifdef HAVE_SSE

#include <core/core.h>
#include "m.h"

const M_VectorOps3 mVecOps3_SSE = {
	"sse",
	M_VectorZero3_SSE,			/* -31 clks */
	M_VectorGet3_SSE,			/* -8 clks */
	M_VectorSet3_FPU,			/* +8 clks */
	M_VectorCopy3_FPU,			/* = */
	M_VectorMirror3_SSE,			/* TODO */
	M_VectorMirror3p_SSE,			/* TODO */
	M_VectorLen3_FPU,			/* = */
	M_VectorLen3p_FPU,			/* = */
	M_VectorDot3_FPU,			/* ? */
	M_VectorDot3p_FPU,			/* ? */
	M_VectorDistance3_SSE,			/* -31 clks */
	M_VectorDistance3p_SSE,		/* -27 clks */
	M_VectorNorm3_SSE,			/* -87 clks */
	M_VectorNorm3p_SSE,			/* -54 clks */
	M_VectorNorm3v_FPU,			/* +147 clks */
	M_VectorCross3_FPU,			/* TODO */
	M_VectorCross3p_FPU,			/* TODO */
	M_VectorNormCross3_FPU,		/* TODO */
	M_VectorNormCross3p_FPU,		/* TODO */
	M_VectorScale3_SSE,			/* -27 clks */
	M_VectorScale3p_SSE,			/* -15 clks */
	M_VectorScale3v_FPU,			/* = */
	M_VectorAdd3_SSE,			/* -29 clks */
	M_VectorAdd3p_SSE,			/* -15 clks */
	M_VectorAdd3v_FPU,			/* = */
	M_VectorAdd3n_SSE,			/* TODO */
	M_VectorSub3_SSE,			/* -29 clks */
	M_VectorSub3p_SSE,			/* -15 clks */
	M_VectorSub3v_FPU,			/* = */
	M_VectorSub3n_SSE,			/* TODO */
	M_VectorAvg3_SSE,			/* TODO */
	M_VectorAvg3p_SSE,			/* TODO */
	M_VectorLERP3_SSE,			/* TODO */
	M_VectorLERP3p_SSE,			/* TODO */
	M_VectorElemPow3_SSE,			/* TODO */
	M_VectorVecAngle3_SSE,		/* TODO */
	M_VectorRotate3_SSE,			/* TODO */
	M_VectorRotate3v_SSE			/* TODO */,
	M_VectorRotateQuat3_SSE,		/* TODO */
	M_VectorRotateI3_SSE,			/* TODO */
	M_VectorRotateJ3_SSE,			/* TODO */
	M_VectorRotateK3_SSE,			/* TODO */
};

M_Vector3
M_VectorAdd3n_SSE(int nvecs, ...)
{
	M_Vector3 c, *v;
	int i;
	va_list ap;

	va_start(ap, nvecs);
	v = va_arg(ap, void *);
	c.m128 = v->m128;
	for (i = 0; i < nvecs; i++) {
		v = va_arg(ap, void *);
		c.m128 = _mm_add_ps(c.m128, v->m128);
	}
	va_end(ap);
	return (c);
}

M_Vector3
M_VectorSub3n_SSE(int nvecs, ...)
{
	M_Vector3 c, *v;
	int i;
	va_list ap;

	va_start(ap, nvecs);
	v = va_arg(ap, void *);
	c.m128 = v->m128;
	for (i = 0; i < nvecs; i++) {
		v = va_arg(ap, void *);
		c.m128 = _mm_sub_ps(c.m128, v->m128);
	}
	va_end(ap);
	return (c);
}

M_Vector3
M_VectorRotate3_SSE(M_Vector3 v, M_Real theta, M_Vector3 n)
{
	M_Vector3 r = v;

	M_VectorRotate3v_SSE(&r, theta, n);
	return (r);
}

void
M_VectorRotate3v_SSE(M_Vector3 *v, M_Real theta, M_Vector3 n)
{
	M_Real s = Sin(theta);
	M_Real c = Cos(theta);
	M_Real t = 1.0 - c;
	M_Matrix44 R;

	R.m[0][0] = t*n.x*n.x + c;
	R.m[0][1] = t*n.x*n.y + s*n.z;
	R.m[0][2] = t*n.x*n.z - s*n.y;
	R.m[0][3] = 0.0;
	R.m[1][0] = t*n.x*n.y - s*n.z;
	R.m[1][1] = t*n.y*n.y + c;
	R.m[1][2] = t*n.y*n.z + s*n.x;
	R.m[1][3] = 0.0;
	R.m[2][0] = t*n.x*n.z + s*n.y;
	R.m[2][1] = t*n.y*n.z - s*n.x;
	R.m[2][2] = t*n.z*n.z + c;
	R.m[2][3] = 0.0;
	R.m[3][0] = 0.0;
	R.m[3][1] = 0.0;
	R.m[3][2] = 0.0;
	R.m[3][3] = 1.0;
	M_MatMult44Vector3v(v, &R);
}

M_Vector3
M_VectorRotateQuat3_SSE(M_Vector3 V, M_Quat Q)
{
	M_Matrix44 R;

	M_QuatToMatrix44(&R, &Q);
	return M_MatMult44Vector3p(&R, &V);
}

#endif /* HAVE_SSE */
