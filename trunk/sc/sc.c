/*	$Csoft$	*/

/*
 * Copyright (c) 2006 CubeSoft Communications, Inc.
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
#include <agar/gui/gui.h>

#if 0
static void
SC_VectorSetProp(void *obj, AG_Prop *prop, void *arg)
{
	SC_Vector *a = arg;

	prop->size = a->m*sizeof(SC_Real);
	prop->data.p = Malloc(prop->size, M_OBJECT);
	SC_VectorCopy(a, (SC_Vector *)prop->data.p);
}

static void *
SC_VectorGetProp(void *obj, AG_Prop *prop)
{
	SC_Vector *a = prop->data.p;
	SC_Vector *b;

	b = SC_VectorNew(a->m);
	SC_VectorCopy(a, b);
	return (b);
}

static void
SC_VectorPrintProp(char *buf, size_t len, void *obj, AG_Prop *prop)
{
	SC_Vector *v = prop->data.p;
	char ent[16];
	Uint i;

	buf[0] = '\0';
	for (i = 1; i <= v->m; i++) {
		snprintf(ent, sizeof(ent), "%f ", v->mat[i][1]);
		if (strlcat(buf, ent, sizeof(buf)) >= sizeof(buf))
			return;
	}
}

int
SC_VectorCompareProp(AG_Prop *p1, AG_Prop *p2)
{
	SC_Vector *v1 = p1->data.p;
	SC_Vector *v2 = p2->data.p;
	Uint i;

	if (v1->m != v2->m) { return (1); }
	for (i = 1; i <= v1->m; i++) {
		if (v1->mat[i][1] != v2->mat[i][1])
			return ((int)(v2->mat[i][1] - v1->mat[i][1]));
	}
	return (0);
}

void *
SC_VectorLoadProp(void *obj, AG_Prop *prop, AG_Netbuf *buf)
{
	return (SC_ReadVector(buf));
}

void
SC_VectorSaveProp(void *obj, AG_Prop *prop, AG_Netbuf *buf)
{
	return (SC_WriteVector(buf, (SC_Vector *)prop->data.p));
}

const AG_PropOps scVectorOps = {
	SC_PROP_VECTOR,
	N_("Vector"),
	SC_VectorSetProp,
	SC_VectorGetProp,
	SC_VectorPrintProp,
	SC_VectorCompareProp,
	SC_VectorLoadProp,
	SC_VectorSaveProp
};

int
SC_Init(Uint flags)
{
	AG_PropRegister(&scVectorOps);
}

void
SC_Destroy(void)
{
}
#endif
