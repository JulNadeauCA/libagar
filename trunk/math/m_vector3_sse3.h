/*
 * Copyright (c) 2007-2008 Hypertriton, Inc. <http://hypertriton.com/>
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

#ifdef HAVE_SSE3

__BEGIN_DECLS
static __inline__ M_Real
M_VectorDot3_SSE3(M_Vector3 v1, M_Vector3 v2)
{
	float rv;
	__m128 a;

	a = _mm_mul_ps(v1.m128, v2.m128);
	a = _mm_hadd_ps(a, a);
	a = _mm_hadd_ps(a, a);
	_mm_store_ss(&rv, a);
	return (M_Real)(rv);
}

static __inline__ M_Real
M_VectorDot3p_SSE3(const M_Vector3 *v1, const M_Vector3 *v2)
{
	float rv;
	__m128 a;

	a = _mm_mul_ps(v1->m128, v2->m128);
	a = _mm_hadd_ps(v1->m128, v1->m128);
	a = _mm_hadd_ps(v1->m128, v1->m128);
	_mm_store_ss(&rv, v1->m128);
	return (M_Real)(rv);
}
__END_DECLS

__BEGIN_DECLS
extern const M_VectorOps3 mVecOps3_SSE3;
__END_DECLS

#endif /* HAVE_SSE3 */
