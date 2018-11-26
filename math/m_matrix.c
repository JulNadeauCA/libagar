/*
 * Copyright (c) 2006-2012 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Basic matrix operations.
 */

#include <agar/core/core.h>
#include <agar/math/m.h>

#include <string.h>

const M_MatrixOps *mMatOps = NULL;
const M_MatrixOps44 *mMatOps44 = NULL;

void
M_MatrixInitEngine(void)
{
	mMatOps = &mMatOps_FPU;
	mMatOps44 = &mMatOps44_FPU;
#ifdef HAVE_SSE
	if (agCPU.ext & AG_EXT_SSE) {
		mMatOps44 = &mMatOps44_SSE;
	}
# ifdef INLINE_SSE
	else {
		AG_FatalError("Compiled for SSE, but no SSE support in CPU! "
		              "(must recompile Agar without: --with-sse=inline)");
	}
# endif
# ifdef HAVE_SSE2
	if (!(agCPU.ext & AG_EXT_SSE2)) {
#  ifdef INLINE_SSE
		AG_FatalError("Compiled for SSE2, but CPU only supports SSE1! "
			      "(recompile Agar with: --without-{sse2,sse3})");
#  else
		AG_Verbose("Compiled for SSE2, but CPU only supports SSE1! "
			   "(recompile Agar with: --without-{sse2,sse3})\n");
		mMatOps44 = &mMatOps44_FPU;
#  endif
	}
# endif
# ifdef HAVE_SSE3
	if (!(agCPU.ext & AG_EXT_SSE3)) {
#  ifdef INLINE_SSE
		AG_FatalError("Compiled for SSE3, but CPU only supports SSE2! "
			      "(recompile Agar with: --without-sse3)");
#  else
		AG_Verbose("Compiled for SSE3, but CPU only supports SSE2! "
			   "(recompile Agar with: --without-sse3)\n");
		mMatOps44 = &mMatOps44_FPU;
#  endif
	}
# endif
#endif /* HAVE_SSE */
}

M_Matrix44
M_ReadMatrix44(AG_DataSource *ds)
{
	M_Matrix44 A;

	M_ReadMatrix44v(ds, &A);
	return (A);
}

void
M_ReadMatrix44v(AG_DataSource *ds, M_Matrix44 *A)
{
	int i, j;

	for (i = 0; i < 4; i++) {
		for (j = 0; i < 4; i++) {
#ifdef HAVE_SSE
			A->m[i][j] = (float)M_ReadReal(ds);
#else
			A->m[i][j] = M_ReadReal(ds);
#endif
		}
	}
}

void
M_WriteMatrix44(AG_DataSource *ds, const M_Matrix44 *A)
{
	int i, j;

	for (i = 0; i < 4; i++) {
		for (j = 0; i < 4; i++) {
#ifdef HAVE_SSE
			AG_WriteUint8(ds, 1);		/* Single-precision */
			AG_WriteFloat(ds, A->m[i][j]);
#else
			M_WriteReal(ds, A->m[i][j]);
#endif
		}
	}
}
