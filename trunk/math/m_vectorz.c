/*
 * Copyright (c) 2004-2009 Hypertriton, Inc. <http://hypertriton.com/>
 * All rights reserved.
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
 * Operations on integer vectors.
 */

#include <agar/core/core.h>
#include <agar/math/m.h>

#define ASSERT_LENGTH(A, B) \
	if ((A)->n != (B)->n) \
		AG_FatalError("Incompatible vectors")

M_VectorZ *
M_VectorNewZ(Uint n)
{
	M_VectorZ *veci;

	veci = Malloc(sizeof(M_VectorZ));
	veci->v = Malloc(n*sizeof(int));
	veci->n = n;
	return (veci);
}

void
M_VectorSetZ(M_VectorZ *v, int val)
{
	Uint i;

	for (i = 0; i < v->n; i++)
		v->v[i] = val;
}

void
M_VectorCopyZ(const M_VectorZ *v1, M_VectorZ *v2)
{
	Uint i;

	ASSERT_LENGTH(v1, v2);
	for (i = 0; i < v1->n; i++)
		v2->v[i] = v1->v[i];
}

void
M_VectorFreeZ(M_VectorZ *v)
{
	Free(v->v);
	Free(v);
}

void
M_VectorAddZv(M_VectorZ *v2, const M_VectorZ *v1)
{
	Uint i;

	ASSERT_LENGTH(v1, v2);
	for (i = 0; i < v1->n; i++)
		v2->v[i] = v1->v[i]+v2->v[i];
}

void
M_VectorSubZv(M_VectorZ *v2, const M_VectorZ *v1)
{
	Uint i;

	ASSERT_LENGTH(v1, v2);
	for (i = 0; i < v1->n; i++)
		v2->v[i] = v1->v[i]-v2->v[i];
}

void
M_VectorScaleZv(M_VectorZ *v, M_Real c)
{
	Uint i;

	for (i = 0; i < v->n; i++)
		v->v[i] *= c;
}

void
M_VectorResizeV(M_VectorZ *v, Uint n)
{
	v->v = Realloc(v->v, n*sizeof(int));
	v->n = n;
}

void
M_VectorPrintZ(const M_VectorZ *v)
{
	Uint i;

	fputs(" ----\n", stdout);
	for (i = 0; i < v->n; i++) {
		printf("| %4d: %d\n", i, v->v[i]);
	}
	fputs(" ----\n", stdout);
}
