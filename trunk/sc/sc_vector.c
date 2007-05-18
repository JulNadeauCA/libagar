/*	$Csoft: vec.c,v 1.5 2005/09/14 01:48:29 vedge Exp $	*/

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

#define AssertSameLength(A, B) \
	if (((A)->m != (B)->m) || ((A)->n != 1) || ((B)->n != 1)) \
		fatal("different vector length (v1=%u, v2=%u)", \
		    (A)->m, (B)->m);

SC_Vector *
SC_VectorNew(Uint n)
{
	SC_Vector *v;

	v = Malloc(sizeof(SC_Vector), M_MATH);
	SC_MatrixAlloc(v, n, 1);
	return (v);
}

SC_Vector *
SC_VectorNewZero(Uint n)
{
	SC_Vector *v;

	v = Malloc(sizeof(SC_Vector), M_MATH);
	SC_MatrixAlloc(v, n, 1);
	SC_VectorSetZero(v);
	return (v);
}

void
SC_VectorCopy(const SC_Vector *v1, SC_Vector *v2)
{
	AssertSameLength(v1, v2);
	SC_MatrixCopy(v1, v2);
}

/* Return the length of a vector in R^n space. */
SC_Real
SC_VectorLength(const SC_Vector *a)
{
	SC_Real sum = 0.0;
	Uint n;

	for (n = 1; n <= a->m; n++) {
		sum += SC_Pow(SC_Fabs(a->mat[n][1]), 2);
	}
	return (SC_Sqrt(sum));
}

#ifdef DEBUG
void
SC_VectorPrint(const SC_Vector *v)
{
	int m;

	fputs(" ----\n", stdout);
	for (m = 1; m <= v->m; m++) {
		printf("| %4d: %f\n", m, v->mat[m][1]);
	}
	fputs(" ----\n", stdout);
}
#endif /* DEBUG */

void
SC_VectorMinimum(SC_Vector *c, const SC_Vector *a, const SC_Vector *b)
{
	Uint i;
	
	AssertSameLength(a, b);
	AssertSameLength(b, c);
	for (i = 1; i <= c->m; i++) {
		if (vEnt(a,i) < vEnt(b,i)) {
			vSet(c,i, vEnt(a,i));
		} else {
			vSet(c,i, vEnt(b,i));
		}
	}
}

void
SC_VectorMaximum(SC_Vector *c, const SC_Vector * a, const SC_Vector *b)
{
	Uint i;

	AssertSameLength(a, b);
	AssertSameLength(b, c);
	for (i = 1; i <= c->m; i++) {
		if (vEnt(a,i) > vEnt(b,i)) {
			vSet(c,i, vEnt(a,i));
		} else {
			vSet(c,i, vEnt(b,i));
		}
	}
}
