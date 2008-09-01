/*
 * Public domain.
 * Operations on vectors in R^3 using SSE3 operations.
 */

#include <config/have_sse3.h>

#ifdef HAVE_SSE3

#include <core/core.h>
#include "m.h"

const M_VectorOps3 mVecOps3_SSE3 = {
	"sse3",
	M_VectorZero3_SSE,			/* -31 clks */
	M_VectorGet3_SSE,			/* -8 clks */
	M_VectorSet3_FPU,			/* +8 clks */
	M_VectorCopy3_FPU,			/* = */
	M_VectorMirror3_SSE,			/* TODO */
	M_VectorMirror3p_SSE,			/* TODO */
	M_VectorLen3_FPU,			/* = */
	M_VectorLen3p_FPU,			/* = */
	M_VectorDot3_SSE3,			/* -26 clks */
	M_VectorDot3p_SSE3,			/* -59 clks */
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
	M_VectorVecAngle3_SSE,			/* TODO */
	M_VectorRotate3_SSE,			/* TODO */
	M_VectorRotate3v_SSE			/* TODO */,
	M_VectorRotateQuat3_SSE,		/* TODO */
	M_VectorRotateI3_SSE,			/* TODO */
	M_VectorRotateJ3_SSE,			/* TODO */
	M_VectorRotateK3_SSE,			/* TODO */
};

#endif /* HAVE_SSE3 */
