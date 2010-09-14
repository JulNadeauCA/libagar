/*
 * Copyright (c) 2008-2010 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Basic operations on general sets of points in R^2 and R^3.
 */

#include <core/core.h>
#include "m.h"

/* Preallocate for a given number of points. */
void
M_PointSetAlloc2(M_PointSet2 *S, Uint nAlloc)
{
	if (nAlloc > S->nMax) {
		S->nMax = nAlloc;
		S->p = Realloc(S->p, S->nMax*sizeof(M_Vector2));
	}
}
void
M_PointSetAlloc3(M_PointSet3 *S, Uint nAlloc)
{
	if (nAlloc > S->nMax) {
		S->nMax = nAlloc;
		S->p = Realloc(S->p, S->nMax*sizeof(M_Vector3));
	}
}
void
M_PointSetAlloc2i(M_PointSet2i *S, Uint nAlloc)
{
	if (nAlloc > S->nMax) {
		S->nMax = nAlloc;
		S->p = Realloc(S->p, S->nMax*sizeof(M_Vector2));
		S->x = Realloc(S->x, S->nMax*sizeof(int));
		S->y = Realloc(S->y, S->nMax*sizeof(int));
	}
}
void
M_PointSetAlloc3i(M_PointSet3i *S, Uint nAlloc)
{
	if (nAlloc > S->nMax) {
		S->nMax = nAlloc;
		S->p = Realloc(S->p, S->nMax*sizeof(M_Vector3));
		S->x = Realloc(S->x, S->nMax*sizeof(int));
		S->y = Realloc(S->y, S->nMax*sizeof(int));
		S->z = Realloc(S->z, S->nMax*sizeof(int));
	}
}

/* Duplicate a point set. */
M_PointSet2
M_PointSetDup2(M_PointSet2 *S1)
{
	M_PointSet2 S2;

	S2.p = Malloc(S1->n*sizeof(M_Vector2));
	memcpy(S2.p, S1->p, S1->n*sizeof(M_Vector2));
	S2.n = S1->n;
	S2.nMax = S1->n;
	return (S2);
}
M_PointSet3
M_PointSetDup3(M_PointSet3 *S1)
{
	M_PointSet3 S2;

	S2.p = Malloc(S1->n*sizeof(M_Vector3));
	memcpy(S2.p, S1->p, S1->n*sizeof(M_Vector3));
	S2.n = S1->n;
	S2.nMax = S1->n;
	return (S2);
}
M_PointSet2i
M_PointSetDup2i(M_PointSet2i *S1)
{
	M_PointSet2i S2;

	S2.p = Malloc(S1->n*sizeof(M_Vector2));
	S2.x = Malloc(S1->n*sizeof(int));
	S2.y = Malloc(S1->n*sizeof(int));
	memcpy(S2.p, S1->p, S1->n*sizeof(M_Vector2));
	memcpy(S2.x, S1->x, S1->n*sizeof(int));
	memcpy(S2.y, S1->y, S1->n*sizeof(int));
	S2.n = S1->n;
	S2.nMax = S1->n;
	return (S2);
}
M_PointSet3i
M_PointSetDup3i(M_PointSet3i *S1)
{
	M_PointSet3i S2;

	S2.p = Malloc(S1->n*sizeof(M_Vector3));
	memcpy(S2.p, S1->p, S1->n*sizeof(M_Vector3));
	memcpy(S2.x, S1->x, S1->n*sizeof(int));
	memcpy(S2.y, S1->y, S1->n*sizeof(int));
	memcpy(S2.z, S1->z, S1->n*sizeof(int));
	S2.n = S1->n;
	S2.nMax = S1->n;
	return (S2);
}

/*
 * Point comparison routines for sort.
 */
static M_Real
ComparePoints2_XY(const void *p1, const void *p2)
{
	const M_Vector2 *v1 = p1, *v2 = p2;
	return (Fabs(v2->x - v1->x) <= M_MACHEP) ?
	       (v2->y - v1->y) : (v1->x - v2->x);
}
static M_Real
ComparePoints2_YX(const void *p1, const void *p2)
{
	const M_Vector2 *v1 = p1, *v2 = p2;
	return (Fabs(v2->y - v1->y) <= M_MACHEP) ?
	       (v2->x - v1->x) : (v1->y - v2->y);
}
static M_Real
ComparePoints3_XYZ(const void *p1, const void *p2)
{
	const M_Vector3 *v1 = p1, *v2 = p2;
	return (Fabs(v2->x - v1->x) <= M_MACHEP) ?
	       (Fabs(v2->y - v1->y) <= M_MACHEP) ?
	       (v2->z - v1->z) :
	       (v1->y - v2->y) :
	       (v1->x - v2->x);
}
static M_Real
ComparePoints3_XZY(const void *p1, const void *p2)
{
	const M_Vector3 *v1 = p1, *v2 = p2;
	return (Fabs(v2->x - v1->x) <= M_MACHEP) ?
	       (Fabs(v2->z - v1->z) <= M_MACHEP) ?
	       (v2->y - v1->y) :
	       (v1->z - v2->z) :
	       (v1->x - v2->x);
}
static M_Real
ComparePoints3_YXZ(const void *p1, const void *p2)
{
	const M_Vector3 *v1 = p1, *v2 = p2;
	return (Fabs(v2->y - v1->y) <= M_MACHEP) ?
	       (Fabs(v2->x - v1->x) <= M_MACHEP) ?
	       (v2->z - v1->z) :
	       (v1->x - v2->x) :
	       (v1->y - v2->y);
}
static M_Real
ComparePoints3_YZX(const void *p1, const void *p2)
{
	const M_Vector3 *v1 = p1, *v2 = p2;
	return (Fabs(v2->y - v1->y) <= M_MACHEP) ?
	       (Fabs(v2->z - v1->z) <= M_MACHEP) ?
	       (v2->x - v1->x) :
	       (v1->z - v2->z) :
	       (v1->y - v2->y);
}
static M_Real
ComparePoints3_ZXY(const void *p1, const void *p2)
{
	const M_Vector3 *v1 = p1, *v2 = p2;
	return (Fabs(v2->z - v1->z) <= M_MACHEP) ?
	       (Fabs(v2->x - v1->z) <= M_MACHEP) ?
	       (v2->y - v1->y) :
	       (v1->x - v2->x) :
	       (v1->z - v2->z);
}
static M_Real
ComparePoints3_ZYX(const void *p1, const void *p2)
{
	const M_Vector3 *v1 = p1, *v2 = p2;
	return (Fabs(v2->z - v1->z) <= M_MACHEP) ?
	       (Fabs(v2->y - v1->y) <= M_MACHEP) ?
	       (v2->x - v1->x) :
	       (v1->y - v2->y) :
	       (v1->z - v2->z);
}

/* Sort points in R2 by their X or Y coordinates. */
void
M_SortPoints2(M_PointSet2 *P, enum m_point_sort_mode2 mode)
{
	switch (mode) {
	case M_POINT_SORT_XY:
		M_QSort(P->p, P->n, sizeof(M_Vector2), ComparePoints2_XY);
		break;
	case M_POINT_SORT_YX:
		M_QSort(P->p, P->n, sizeof(M_Vector2), ComparePoints2_YX);
		break;
	}
}

/* Sort points in R3 by their X, Y or Z coordinates. */
void
M_SortPoints3(M_PointSet3 *P, enum m_point_sort_mode3 mode)
{
	switch (mode) {
	case M_POINT_SORT_XYZ:
		M_QSort(P->p, P->n, sizeof(M_Vector3), ComparePoints3_XYZ);
		break;
	case M_POINT_SORT_XZY:
		M_QSort(P->p, P->n, sizeof(M_Vector3), ComparePoints3_XZY);
		break;
	case M_POINT_SORT_YXZ:
		M_QSort(P->p, P->n, sizeof(M_Vector3), ComparePoints3_YXZ);
		break;
	case M_POINT_SORT_YZX:
		M_QSort(P->p, P->n, sizeof(M_Vector3), ComparePoints3_YZX);
		break;
	case M_POINT_SORT_ZXY:
		M_QSort(P->p, P->n, sizeof(M_Vector3), ComparePoints3_ZXY);
		break;
	case M_POINT_SORT_ZYX:
		M_QSort(P->p, P->n, sizeof(M_Vector3), ComparePoints3_ZYX);
		break;
	}
}
