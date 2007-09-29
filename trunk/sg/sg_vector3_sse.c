/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * Operations on vectors in R^3 using SSE1 operations.
 */

#include <config/have_opengl.h>
#include <config/have_sse.h>

#if defined(HAVE_OPENGL) && defined(HAVE_SSE)

#include <core/core.h>
#include "sg.h"

const SG_VectorOps3 sgVecOps3_SSE = {
	"sse",
	SG_VectorZero3_SSE,			/* -31 clks */
	SG_VectorGet3_SSE,			/* -8 clks */
	SG_VectorSet3_FPU,			/* +8 clks */
	SG_VectorCopy3_FPU,			/* = */
	SG_VectorMirror3_SSE,			/* TODO */
	SG_VectorMirror3p_SSE,			/* TODO */
	SG_VectorLen3_FPU,			/* = */
	SG_VectorLen3p_FPU,			/* = */
	SG_VectorDot3_FPU,			/* ? */
	SG_VectorDot3p_FPU,			/* ? */
	SG_VectorDistance3_SSE,			/* -31 clks */
	SG_VectorDistance3p_SSE,		/* -27 clks */
	SG_VectorNorm3_SSE,			/* -87 clks */
	SG_VectorNorm3p_SSE,			/* -54 clks */
	SG_VectorNorm3v_FPU,			/* +147 clks */
	SG_VectorCross3_FPU,			/* TODO */
	SG_VectorCross3p_FPU,			/* TODO */
	SG_VectorNormCross3_FPU,		/* TODO */
	SG_VectorNormCross3p_FPU,		/* TODO */
	SG_VectorScale3_SSE,			/* -27 clks */
	SG_VectorScale3p_SSE,			/* -15 clks */
	SG_VectorScale3v_FPU,			/* = */
	SG_VectorAdd3_SSE,			/* -29 clks */
	SG_VectorAdd3p_SSE,			/* -15 clks */
	SG_VectorAdd3v_FPU,			/* = */
	SG_VectorAdd3n_SSE,			/* TODO */
	SG_VectorSub3_SSE,			/* -29 clks */
	SG_VectorSub3p_SSE,			/* -15 clks */
	SG_VectorSub3v_FPU,			/* = */
	SG_VectorSub3n_SSE,			/* TODO */
	SG_VectorAvg3_SSE,			/* TODO */
	SG_VectorAvg3p_SSE,			/* TODO */
	SG_VectorLERP3_SSE,			/* TODO */
	SG_VectorLERP3p_SSE,			/* TODO */
	SG_VectorElemPow3_SSE,			/* TODO */
	SG_VectorVecAngle3_SSE,		/* TODO */
	SG_VectorRotate3_SSE,			/* TODO */
	SG_VectorRotate3v_SSE			/* TODO */,
	SG_VectorRotateQuat3_SSE,		/* TODO */
	SG_VectorRotateI3_SSE,			/* TODO */
	SG_VectorRotateJ3_SSE,			/* TODO */
	SG_VectorRotateK3_SSE,			/* TODO */
};

SG_Vector
SG_VectorAdd3n_SSE(int nvecs, ...)
{
	SG_Vector c, *v;
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

SG_Vector
SG_VectorSub3n_SSE(int nvecs, ...)
{
	SG_Vector c, *v;
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

SG_Vector
SG_VectorRotate3_SSE(SG_Vector v, SG_Real theta, SG_Vector n)
{
	SG_Vector r = v;

	SG_VectorRotate3v_SSE(&r, theta, n);
	return (r);
}

void
SG_VectorRotate3v_SSE(SG_Vector *v, SG_Real theta, SG_Vector n)
{
	SG_Real s = Sin(theta);
	SG_Real c = Cos(theta);
	SG_Real t = 1.0 - c;
	SG_Matrix R;

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
	SG_MatrixMultVectorv(v, &R);
}

SG_Vector
SG_VectorRotateQuat3_SSE(SG_Vector V, SG_Quat Q)
{
	SG_Matrix R;

	SG_QuatToMatrix(&R, &Q);
	return (SG_MatrixMultVectorp(&R, &V));
}

#endif /* HAVE_OPENGL and HAVE_SSE */
