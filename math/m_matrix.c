/*
 * Copyright (c) 2006-2008 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>
#include "m.h"
#include "m_bitstring.h"

#include <string.h>

const M_MatrixOps *mMatOps = NULL;
const M_MatrixOps44 *mMatOps44 = NULL;

void
M_MatrixInitEngine(void)
{
	mMatOps = &mMatOps_FPU;
	mMatOps44 = &mMatOps44_FPU;
	
	if (HasSSE()) {
#ifdef HAVE_SSE
		mMatOps44 = &mMatOps44_SSE;
#elif defined(AG_DEBUG)
		AG_Verbose("SSE available, but disabled at compile time\n");
#endif
	}

#ifdef AG_DEBUG
	AG_Verbose("Matrix operations: ");
#if defined(INLINE_ALTIVEC)
	AG_Verbose("altivec (inline)\n");
#elif defined(INLINE_SSE3)
	AG_Verbose("sse3 (inline)\n");
#elif defined(INLINE_SSE2)
	AG_Verbose("sse2 (inline)\n");
#elif defined(INLINE_SSE)
	AG_Verbose("sse (inline)\n");
#else
	AG_Verbose("%s\n", mMatOps44->name);
#endif
#endif /* AG_DEBUG */
}


void
M_MatrixPrint44(const M_Matrix44 *A)
{
	int m, n;

	for (n = 0; n < 4; n++) {
		for (m = 0; m < 4; m++) { printf("%f ", A->m[m][n]); }
		printf("\n");
	}
}

M_Matrix44
M_ReadMatrix44(AG_DataSource *buf)
{
	M_Matrix44 A;

	M_ReadMatrix44v(buf, &A);
	return (A);
}

void
M_ReadMatrix44v(AG_DataSource *buf, M_Matrix44 *A)
{
	M_Real *pm = &A->m[0][0];
	int i;
#if 0
	/*
	 * Trivial compression for sparse matrices.
	 */
	bitstr_t bit_decl(map0, 16);
	bitstr_t bit_decl(map1, 16);

	AG_Read(buf, &map0, sizeof(map0), 1);
	AG_Read(buf, &map1, sizeof(map1), 1);

	for (i = 0; i < 16; i++) {
		if (bit_test(map0, i)) {
			*pm = 0.0;
		} else if (bit_test(map1, i)) {
			*pm = 1.0;
		} else {
			*pm = (M_Real)AG_ReadDouble(buf);
		}
		pm++;
	}
#else
	for (i = 0; i < 16; i++) {
		*pm = M_ReadReal(buf);
		pm++;
	}
#endif
}

void
M_WriteMatrix44(AG_DataSource *buf, const M_Matrix44 *A)
{
	const M_Real *pm = &A->m[0][0];
	int i;
#if 0
	/*
	 * Trivial compression for sparse matrices.
	 */
	bitstr_t bit_decl(map0, 16);
	bitstr_t bit_decl(map1, 16);
	off_t offs;

	offs = AG_Tell(buf);
	AG_Seek(buf, 4, AG_SEEK_CUR);
	for (i = 0; i < 16; i++) {
		if (*pm == 0.0) {
			bit_set(map0, i);
			bit_clear(map1, i);
		} else if (*pm == 1.0) {
			bit_clear(map0, i);
			bit_set(map1, i);
		} else {
			bit_clear(map0, i);
			bit_clear(map1, i);
			AG_WriteDouble(buf, (M_Real)(*pm));
		}
		pm++;
	}
	AG_WriteAt(buf, &map0, sizeof(map0), 1, offs);
	AG_WriteAt(buf, &map1, sizeof(map1), 1, offs+2);
#else
	for (i = 0; i < 16; i++) {
		M_WriteReal(buf, *pm);
		pm++;
	}
#endif
}

