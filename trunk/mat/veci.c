/*	$Csoft: veci.c,v 1.1 2004/11/23 02:32:39 vedge Exp $	*/

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
	if ((A)->n != (B)->n) \
		fatal("different vector length")

struct veci *
veci_new(int n)
{
	struct veci *veci;

	veci = Malloc(sizeof(struct veci), M_MATH);
	veci->vec = Malloc((n+1)*sizeof(int), M_MATH);
	veci->n = n;
	return (veci);
}

void
veci_set(struct veci *v, int val)
{
	int i;

	for (i = 1; i <= v->n; i++)
		v->vec[i] = val;
}

void
veci_copy(const struct veci *v1, struct veci *v2)
{
	int i;

	assert_same_length(v1, v2);
	for (i = 1; i <= v1->n; i++)
		v2->vec[i] = v1->vec[i];
}

void
veci_free(struct veci *v)
{
	Free(v->vec, M_MATH);
	Free(v, M_MATH);
}

void
veci_add(const struct veci *v1, struct veci *v2)
{
	int i;

	assert_same_length(v1, v2);
	for (i = 1; i <= v1->n; i++)
		v2->vec[i] = v1->vec[i]+v2->vec[i];
}

#ifdef DEBUG
void
veci_print(const struct veci *v)
{
	int i;

	fputs(" ----\n", stdout);
	for (i = 1; i <= v->n; i++) {
		printf("| %4d: %d\n", i, v->vec[i]);
	}
	fputs(" ----\n", stdout);
}
#endif /* DEBUG */
