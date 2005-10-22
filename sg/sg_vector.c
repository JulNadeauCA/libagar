/*	$Csoft$	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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

#include <core/core.h>
#include "sg.h"

SG_Vector *
SG_VectorNew(Uint n)
{
	SG_Vector *v;

	v = Malloc(sizeof(SG_Vector), M_SG);
	v->v = Malloc(n*sizeof(SG_Real), M_SG);
	v->n = n;
	return (v);
}

SG_Real
SG_DotProduct(const SG_Vector *v1, const SG_Vector *v2)
{
	SG_Real p = 0.0;
	Uint n;

#ifdef DEBUG
	if (v1->n != v2->n) { fatal("vectors of different length"); }
#endif
	for (n = 0; n < v1->n; n++) {
		p += v1->v[n]*v2->v[n];
	}
	return (p);
}

SG_Real
SG_DotProduct2(const SG_Vector2 *v1, const SG_Vector2 *v2)
{
	return (v1->x*v2->x + v1->y*v2->y);
}

SG_Real
SG_DotProduct3(const SG_Vector3 *v1, const SG_Vector3 *v2)
{
	return (v1->x*v2->x + v1->y*v2->y + v1->z*v2->z);
}

SG_Real
SG_DotProduct4(const SG_Vector4 *v1, const SG_Vector4 *v2)
{
	return (v1->x*v2->x + v1->y*v2->y + v1->z*v2->z + v1->w*v2->w);
}

SG_Real
SG_Length(const SG_Vector *v)
{
	SG_Real len = 0.0;
	Uint n;

	for (n = 0; n < v->n; n++) {
		len += v->v[n]*v->v[n];
	}
	return (AG_Sqrt(len));
}

SG_Real
SG_Length2(const SG_Vector2 *v)
{
	return (AG_Sqrt(v->x*v->x + v->y*v->y));
}

SG_Real
SG_Length3(const SG_Vector3 *v)
{
	return (AG_Sqrt(v->x*v->x + v->y*v->y + v->z*v->z));
}

SG_Real
SG_Length4(const SG_Vector4 *v)
{
	return (AG_Sqrt(v->x*v->x + v->y*v->y + v->z*v->z + v->w*v->w));
}

void
SG_Normalize(SG_Vector3 *v)
{
	SG_Real len;

	len = SG_Length3(v);
#ifdef DEBUG
	if (len == 0.0) { fatal("zero-length vector"); }
#endif
	v->x /= len;
	v->y /= len;
	v->z /= len;
}

void
SG_CrossProd(SG_Vector3 *c, const SG_Vector3 *a, const SG_Vector3 *b)
{
#ifdef DEBUG
	if (c == a || c == b) { fatal("vector overlap"); }
#endif
	c->x = a->y*b->z - b->y*a->z;
	c->y = a->z*b->x - b->z*a->x;
	c->z = a->x*b->y - b->x*a->y;
}

void
SG_CrossProdNormed(SG_Vector3 *c, const SG_Vector3 *a, const SG_Vector3 *b)
{
	SG_CrossProd(c, a, b);
	SG_Normalize(c);
}

void
SG_MulVector3(SG_Vector3 *r, SG_Real c, const SG_Vector3 *v)
{
	r->x = c*v->x;
	r->y = c*v->y;
	r->z = c*v->z;
}

void
SG_DivVector3(SG_Vector3 *r, SG_Real c, const SG_Vector3 *v)
{
	r->x = v->x/c;
	r->y = v->y/c;
	r->z = v->z/c;
}

void
SG_MulVector4(SG_Vector4 *r, SG_Real c, const SG_Vector4 *v)
{
	r->x = c*v->x;
	r->y = c*v->y;
	r->z = c*v->z;
	r->w = c*v->w;
}

void
SG_DivVector4(SG_Vector4 *r, SG_Real c, const SG_Vector4 *v)
{
	r->x = v->x/c;
	r->y = v->y/c;
	r->z = v->z/c;
	r->w = v->w/c;
}
