/*	$Csoft: veci.c,v 1.3 2005/09/10 05:06:06 vedge Exp $	*/

/*
 * Copyright (c) 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <agar/core/core.h>

#include "sc_pvt.h"

#define assert_same_length(A, B) \
	if ((A)->n != (B)->n) \
		fatal("different vector length")

SC_Ivector *
SC_IvectorNew(Uint n)
{
	SC_Ivector *veci;

	veci = Malloc(sizeof(SC_Ivector), M_MATH);
	veci->vec = Malloc((n+1)*sizeof(int), M_MATH);
	veci->n = n;
	return (veci);
}

void
SC_IvectorSet(SC_Ivector *v, int val)
{
	int i;

	for (i = 1; i <= v->n; i++)
		v->vec[i] = val;
}

void
SC_IvectorCopy(const SC_Ivector *v1, SC_Ivector *v2)
{
	int i;

	assert_same_length(v1, v2);
	for (i = 1; i <= v1->n; i++)
		v2->vec[i] = v1->vec[i];
}

void
SC_IvectorFree(SC_Ivector *v)
{
	Free(v->vec, M_MATH);
	Free(v, M_MATH);
}

void
SC_IvectorAdd(const SC_Ivector *v1, SC_Ivector *v2)
{
	int i;

	assert_same_length(v1, v2);
	for (i = 1; i <= v1->n; i++)
		v2->vec[i] = v1->vec[i]+v2->vec[i];
}

void
SC_IvectorResize(SC_Ivector *v, Uint n)
{
	v->vec = Realloc(v->vec, (n+1)*sizeof(int));
	v->n = n;
}

#ifdef DEBUG
void
SC_IvectorPrint(const SC_Ivector *v)
{
	int i;

	fputs(" ----\n", stdout);
	for (i = 1; i <= v->n; i++) {
		printf("| %4d: %d\n", i, v->vec[i]);
	}
	fputs(" ----\n", stdout);
}
#endif /* DEBUG */
