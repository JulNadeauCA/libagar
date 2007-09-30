/*
 * Copyright (c) 2006-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Miscellaneous utility routines for matrices.
 */

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>
#include "sg.h"
#include "sg_bitstring.h"
#include <string.h>

const SG_MatrixOps44 *sgMatOps44 = NULL;

void
SG_MatrixInitEngine(void)
{
	sgMatOps44 = &sgMatOps44_FPU;
	
	if (HasSSE()) {
#ifdef HAVE_SSE
		sgMatOps44 = &sgMatOps44_SSE;
#elif defined(DEBUG)
		fprintf(stderr, "SSE is available, but support disabled "
		                "at compile time\n");
#endif
	}

#ifdef DEBUG
	printf("Matrix operations: ");
#if defined(INLINE_ALTIVEC)
	printf("altivec (inline)\n");
#elif defined(INLINE_SSE3)
	printf("sse3 (inline)\n");
#elif defined(INLINE_SSE2)
	printf("sse2 (inline)\n");
#elif defined(INLINE_SSE)
	printf("sse (inline)\n");
#else
	printf("%s\n", sgMatOps44->name);
#endif
#endif /* DEBUG */
}

void
SG_MatrixPrint(const SG_Matrix *A)
{
	int m, n;

	for (n = 0; n < 4; n++) {
		for (m = 0; m < 4; m++) { printf("%f ", A->m[m][n]); }
		printf("\n");
	}
}

void
SG_MatrixGetRotationXYZ(const SG_Matrix *R, SG_Real *pitch, SG_Real *yaw,
    SG_Real *roll)
{
	*roll = SG_Atan2(R->m[1][0], R->m[0][0]);
	*pitch = -SG_Asin(R->m[2][0]);
	*yaw = SG_Atan2(R->m[2][1], R->m[2][2]);
}

void
SG_MatrixGetTranslation(const SG_Matrix *T, SG_Vector *t)
{
	t->x = T->m[0][3];
	t->y = T->m[1][3];
	t->z = T->m[2][3];
}

SG_Matrix
SG_ReadMatrix(AG_Netbuf *buf)
{
	SG_Matrix A;

	SG_ReadMatrixv(buf, &A);
	return (A);
}

void
SG_ReadMatrixv(AG_Netbuf *buf, SG_Matrix *A)
{
	SG_Real *pm = &A->m[0][0];
	int i;
#if 0
	bitstr_t bit_decl(map0, 16);
	bitstr_t bit_decl(map1, 16);

	AG_NetbufRead(&map0, sizeof(map0), 1, buf);
	AG_NetbufRead(&map1, sizeof(map1), 1, buf);

	for (i = 0; i < 16; i++) {
		if (bit_test(map0, i)) {
			*pm = 0.0;
		} else if (bit_test(map1, i)) {
			*pm = 1.0;
		} else {
			*pm = (SG_Real)AG_ReadDouble(buf);
		}
		pm++;
	}
#else
	for (i = 0; i < 16; i++) {
		*pm = (SG_Real)AG_ReadDouble(buf);
		pm++;
	}
#endif
}

void
SG_WriteMatrix(AG_Netbuf *buf, SG_Matrix *A)
{
	SG_Real *pm = &A->m[0][0];
	int i;
#if 0
	bitstr_t bit_decl(map0, 16);
	bitstr_t bit_decl(map1, 16);
	off_t offs;

	offs = AG_NetbufTell(buf);
	AG_NetbufSeek(buf, 4, SEEK_CUR);
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
			AG_WriteDouble(buf, (SG_Real)(*pm));
		}
		pm++;
	}
	AG_NetbufPwrite(&map0, sizeof(map0), 1, offs, buf);
	AG_NetbufPwrite(&map1, sizeof(map1), 1, offs+2, buf);
#else
	for (i = 0; i < 16; i++) {
		AG_WriteDouble(buf, (double)(*pm));
		pm++;
	}
#endif
}

void
SG_LoadMatrixGL(const SG_Matrix *M)
{
	float fv[4][4];

	MatToFloats(&fv[0][0], M);
	glLoadMatrixf(&fv[0][0]);
}

void
SG_GetMatrixGL(int which, SG_Matrix *M)
{
	float fv[4][4];

	glGetFloatv((GLenum)which, &fv[0][0]);
	MatFromFloats(M, &fv[0][0]);
}
#endif /* HAVE_OPENGL */
