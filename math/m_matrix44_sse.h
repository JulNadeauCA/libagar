/*
 * Copyright (c) 2007-2018 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Operations on 4x4 matrices using Streaming SIMD Extensions.
 */

#ifdef HAVE_SSE

__BEGIN_DECLS

static __inline__ M_Matrix44
M_MatrixZero44_SSE(void)
{
	M_Matrix44 out;

	out.m1 = _mm_setzero_ps();
	out.m2 = _mm_setzero_ps();
	out.m3 = _mm_setzero_ps();
	out.m4 = _mm_setzero_ps();
	return (out);
}
static __inline__ void
M_MatrixZero44v_SSE(M_Matrix44 *_Nonnull M)
{
	M->m1 = _mm_setzero_ps();
	M->m2 = _mm_setzero_ps();
	M->m3 = _mm_setzero_ps();
	M->m4 = _mm_setzero_ps();
}

static __inline__ M_Matrix44
M_MatrixIdentity44_SSE(void)
{
	M_Matrix44 I;

	I.m1 = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);
	I.m2 = _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f);
	I.m3 = _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f);
	I.m4 = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
	return (I);
}
static __inline__ void
M_MatrixIdentity44v_SSE(M_Matrix44 *_Nonnull M)
{
	M->m1 = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);
	M->m2 = _mm_set_ps(0.0f, 0.0f, 1.0f, 0.0f);
	M->m3 = _mm_set_ps(0.0f, 1.0f, 0.0f, 0.0f);
	M->m4 = _mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f);
}

static __inline__ M_Matrix44
M_MatrixTranspose44_SSE(M_Matrix44 M)
{
	M_Matrix44 out;
	out.m1 = M.m1;
	out.m2 = M.m2;
	out.m3 = M.m3;
	out.m4 = M.m4;
	_MM_TRANSPOSE4_PS(out.m1, out.m2, out.m3, out.m4);
	return (out);
}
static __inline__ M_Matrix44
M_MatrixTranspose44p_SSE(const M_Matrix44 *_Nonnull M)
{
	M_Matrix44 out;
	out.m1 = M->m1;
	out.m2 = M->m2;
	out.m3 = M->m3;
	out.m4 = M->m4;
	_MM_TRANSPOSE4_PS(out.m1, out.m2, out.m3, out.m4);
	return (out);
}
static __inline__ void
M_MatrixTranspose44v_SSE(M_Matrix44 *_Nonnull M)
{
	_MM_TRANSPOSE4_PS(M->m1, M->m2, M->m3, M->m4);
}

static __inline__ M_Matrix44
M_MatrixMult44_SSE(M_Matrix44 A, M_Matrix44 B)
{
	__m128 r1;
	M_Matrix44 out;

	r1 = A.m1;
	out.m1 = _mm_add_ps(
	    _mm_add_ps(
	        _mm_add_ps(
	            _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(0,0,0,0)), B.m1),
		    _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(1,1,1,1)), B.m2)
	        ),
	        _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(2,2,2,2)), B.m3)),
	        _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(3,3,3,3)), B.m4)
	    );
	r1 = A.m2;
	out.m2 = _mm_add_ps(
	    _mm_add_ps(
	        _mm_add_ps(
	            _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(0,0,0,0)), B.m1),
	            _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(1,1,1,1)), B.m2)
	        ),
	        _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(2,2,2,2)), B.m3)),
	        _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(3,3,3,3)), B.m4)
	    );
	r1 = A.m3;
	out.m3 = _mm_add_ps(
	    _mm_add_ps(
	        _mm_add_ps(
	            _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(0,0,0,0)), B.m1),
	            _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(1,1,1,1)), B.m2)
	        ),
	        _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(2,2,2,2)), B.m3)),
	        _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(3,3,3,3)), B.m4)
	    );
	r1 = A.m4;
	out.m4 = _mm_add_ps(
	    _mm_add_ps(
	        _mm_add_ps(
	            _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(0,0,0,0)), B.m1),
		    _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(1,1,1,1)), B.m2)
	        ),
	        _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(2,2,2,2)), B.m3)),
	        _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(3,3,3,3)), B.m4)
	    );

	return (out);
}
static __inline__ void
M_MatrixMult44v_SSE(M_Matrix44 *_Nonnull A, const M_Matrix44 *_Nonnull B)
{
	__m128 r1;
	M_Matrix44 out;

	r1 = A->m1;
	out.m1 = _mm_add_ps(
	    _mm_add_ps(
	        _mm_add_ps(
	            _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(0,0,0,0)), B->m1),
	            _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(1,1,1,1)), B->m2)
	        ),
	        _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(2,2,2,2)), B->m3)),
	        _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(3,3,3,3)), B->m4)
	    );
	r1 = A->m2;
	out.m2 = _mm_add_ps(
	    _mm_add_ps(
	        _mm_add_ps(
	            _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(0,0,0,0)), B->m1),
	            _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(1,1,1,1)), B->m2)
	        ),
	        _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(2,2,2,2)), B->m3)),
	        _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(3,3,3,3)), B->m4)
	    );
	r1 = A->m3;
	out.m3 = _mm_add_ps(
	    _mm_add_ps(
	        _mm_add_ps(
	            _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(0,0,0,0)), B->m1),
	            _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(1,1,1,1)), B->m2)
	        ),
	        _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(2,2,2,2)), B->m3)),
	        _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(3,3,3,3)), B->m4)
	    );
	r1 = A->m4;
	out.m4 = _mm_add_ps(
	    _mm_add_ps(
	        _mm_add_ps(
	            _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(0,0,0,0)), B->m1),
	            _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(1,1,1,1)), B->m2)
	        ),
	        _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(2,2,2,2)), B->m3)),
	        _mm_mul_ps(_mm_shuffle_ps(r1,r1,_MM_SHUFFLE(3,3,3,3)), B->m4)
	    );

	A->m1 = out.m1;
	A->m2 = out.m2;
	A->m3 = out.m3;
	A->m4 = out.m4;
}

static __inline__ M_Vector4
M_MatrixMultVector44_SSE(M_Matrix44 A, M_Vector4 b)
{
#ifdef HAVE_SSE3
	__m128 x, r1, r2;
	M_Vector4 out;

	x = b.m128;
	r1 = _mm_hadd_ps(_mm_mul_ps(A.m1,x), _mm_mul_ps(A.m2,x));
	r2 = _mm_hadd_ps(_mm_mul_ps(A.m3,x), _mm_mul_ps(A.m4,x));
	out.m128 = _mm_hadd_ps(r1, r2);
	return (out);
#else
	return M_MatrixMultVector44_FPU(A, b);
#endif
}
static __inline__ M_Vector4
M_MatrixMultVector44p_SSE(const M_Matrix44 *_Nonnull A,
    const M_Vector4 *_Nonnull b)
{
#ifdef HAVE_SSE3
	__m128 x, r1, r2;
	M_Vector4 out;

	x = b->m128;
	r1 = _mm_hadd_ps(_mm_mul_ps(A->m1,x), _mm_mul_ps(A->m2,x));
	r2 = _mm_hadd_ps(_mm_mul_ps(A->m3,x), _mm_mul_ps(A->m4,x));
	out.m128 = _mm_hadd_ps(r1, r2);
	return (out);
#else
	return M_MatrixMultVector44p_FPU(A, b);
#endif
}
static __inline__ void
M_MatrixMultVector44v_SSE(M_Vector4 *_Nonnull b, const M_Matrix44 *_Nonnull A)
{
#ifdef HAVE_SSE3
	__m128 x, r1, r2;

	x = b->m128;
	r1 = _mm_hadd_ps(_mm_mul_ps(A->m1,x), _mm_mul_ps(A->m2,x));
	r2 = _mm_hadd_ps(_mm_mul_ps(A->m3,x), _mm_mul_ps(A->m4,x));
	b->m128 = _mm_hadd_ps(r1, r2);
#else
	M_MatrixMultVector44v_FPU(b, A);
#endif
}

static __inline__ void
M_MatrixCopy44_SSE(M_Matrix44 *_Nonnull mDst, const M_Matrix44 *_Nonnull mSrc)
{
	mDst->m1 = mSrc->m1;
	mDst->m2 = mSrc->m2;
	mDst->m3 = mSrc->m3;
	mDst->m4 = mSrc->m4;
}
__END_DECLS

__BEGIN_DECLS
extern const M_MatrixOps44 mMatOps44_SSE;

M_Matrix44 M_MatrixInvert44_SSE(const M_Matrix44);
void       M_MatrixRotateAxis44_SSE(M_Matrix44 *_Nonnull, M_Real, M_Vector3);
void       M_MatrixRotate44I_SSE(M_Matrix44 *_Nonnull, M_Real);
void       M_MatrixRotate44J_SSE(M_Matrix44 *_Nonnull, M_Real);
void       M_MatrixRotate44K_SSE(M_Matrix44 *_Nonnull, M_Real);
void       M_MatrixTranslatev44_SSE(M_Matrix44 *_Nonnull, M_Vector3);
void       M_MatrixTranslate44_SSE(M_Matrix44 *_Nonnull, M_Real, M_Real, M_Real);
void       M_MatrixTranslateX44_SSE(M_Matrix44 *_Nonnull, M_Real);
void       M_MatrixTranslateY44_SSE(M_Matrix44 *_Nonnull, M_Real);
void       M_MatrixTranslateZ44_SSE(M_Matrix44 *_Nonnull, M_Real);
void       M_MatrixScale44_SSE(M_Matrix44 *_Nonnull, M_Real, M_Real, M_Real, M_Real);
void       M_MatrixUniScale44_SSE(M_Matrix44 *_Nonnull, M_Real);
__END_DECLS

#endif /* HAVE_SSE */
