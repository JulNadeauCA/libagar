/*
 * Copyright (c) 2012 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Operations on vectors in R^3 using Streaming SIMD Extensions.
 */

#include <agar/config/have_sse.h>
#ifdef HAVE_SSE

#include <agar/core/core.h>
#include <agar/math/m.h>

const M_VectorOps3 mVecOps3_SSE = {
	"sse",
	M_VectorZero3_SSE,			/* -16 clks */
	M_VectorGet3_SSE,			/* = (sets w=0) */
	M_VectorSet3_SSE,			/* = (sets w=0) */
	M_VectorCopy3_SSE,			/* = */
	M_VectorFlip3_SSE,			/* = */
	M_VectorLen3_SSE,			/* = */
	M_VectorLen3p_SSE,			/* -3 clks */
	M_VectorDot3_SSE,			/* +14 clks (SSE3) */
	M_VectorDot3p_SSE,			/* -4 clks (SSE3) */
	M_VectorDistance3_SSE,			/* -55 clks */
	M_VectorDistance3p_SSE,			/* -120 clks */
	M_VectorNorm3_SSE,			/* -105 clks */
	M_VectorNorm3p_SSE,			/* -87 clks */
	M_VectorNorm3v_SSE,			/* -67 clks */
	M_VectorCross3_SSE,			/* = */
	M_VectorCross3p_SSE,			/* -20 clks */
	M_VectorNormCross3_SSE,			/* -42 clks */
	M_VectorNormCross3p_SSE,		/* -40 clks */
	M_VectorScale3_SSE,			/* -27 clks */
	M_VectorScale3p_SSE,			/* -15 clks */
	M_VectorScale3v_SSE,			/* -29 clks */
	M_VectorAdd3_SSE,			/* -29 clks */
	M_VectorAdd3p_SSE,			/* -15 clks */
	M_VectorAdd3v_SSE,			/* -3 clks */
	M_VectorSum3_SSE,			/* -58 clks (100 vecs) */
	M_VectorSub3_SSE,			/* -29 clks */
	M_VectorSub3p_SSE,			/* -15 clks */
	M_VectorSub3v_SSE,			/* -3 clks */
	M_VectorAvg3_SSE,			/* +11 clks */
	M_VectorAvg3p_SSE,			/* -9 clks */
	M_VectorLERP3_SSE,			/* */
	M_VectorLERP3p_SSE,			/* */
	M_VectorElemPow3_SSE,			/* */
	M_VectorVecAngle3_SSE			/* */
};

#endif /* HAVE_SSE */
