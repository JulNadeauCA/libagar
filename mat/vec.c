/*	$Csoft: vec.c,v 1.3 2005/09/10 05:06:06 vedge Exp $	*/

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

#include <engine/engine.h>

#include "mat.h"

#define assert_same_length(A, B) \
	if (((A)->m != (B)->m) || ((A)->n != 1) || ((B)->n != 1)) \
		fatal("different vector length")

vec_t *
vec_new(u_int n)
{
	vec_t *v;

	v = Malloc(sizeof(vec_t), M_MATH);
	mat_alloc_elements(v, n, 1);
	return (v);
}

void
vec_copy(const vec_t *v1, vec_t *v2)
{
	assert_same_length(v1, v2);
	mat_copy(v1, v2);
}

/* Return the length of a vector in R^n space. */
double
vec_len(const vec_t *a)
{
	double sum = 0.0;
	u_int n;

	for (n = 1; n <= a->m; n++) {
		sum += pow(fabs(a->mat[n][0]), 2);
	}
	return (sqrt(sum));
}

#ifdef DEBUG
void
vec_print(const vec_t *v)
{
	int m;

	fputs(" ----\n", stdout);
	for (m = 1; m <= v->m; m++) {
		printf("| %4d: %f\n", m, v->mat[m][0]);
	}
	fputs(" ----\n", stdout);
}
#endif /* DEBUG */
