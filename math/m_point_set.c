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

#include <agar/core/core.h>
#include <agar/math/m.h>

/* Preallocate for a given number of points. */
int
M_PointSetAlloc2(M_PointSet2 *S, Uint nAlloc)
{
	M_Vector2 *pNew;

	if (nAlloc <= S->nMax) {
		return (0);
	}
	if ((pNew = TryRealloc(S->p, nAlloc*sizeof(M_Vector2))) == NULL) {
		return (-1);
	}
	S->p = pNew;
	S->nMax = nAlloc;
	return (0);
}
int
M_PointSetAlloc3(M_PointSet3 *S, Uint nAlloc)
{
	M_Vector3 *pNew;

	if (nAlloc <= S->nMax) {
		return (0);
	}
	if ((pNew = TryRealloc(S->p, nAlloc*sizeof(M_Vector3))) == NULL) {
		return (-1);
	}
	S->p = pNew;
	S->nMax = nAlloc;
	return (0);
}
int
M_PointSetAlloc2i(M_PointSet2i *S, Uint nAlloc)
{
	int *vNew;

	if (nAlloc <= S->nMax)
		return (0);

	if ((vNew = TryRealloc(S->x, nAlloc*sizeof(int))) == NULL) { return (-1); }
	S->x = vNew;
	if ((vNew = TryRealloc(S->y, nAlloc*sizeof(int))) == NULL) { return (-1); }
	S->y = vNew;
	
	S->nMax = nAlloc;
	return (0);
}
int
M_PointSetAlloc3i(M_PointSet3i *S, Uint nAlloc)
{
	int *vNew;

	if (nAlloc <= S->nMax)
		return (0);

	if ((vNew = TryRealloc(S->x, nAlloc*sizeof(int))) == NULL) { return (-1); }
	S->x = vNew;
	if ((vNew = TryRealloc(S->y, nAlloc*sizeof(int))) == NULL) { return (-1); }
	S->y = vNew;
	if ((vNew = TryRealloc(S->z, nAlloc*sizeof(int))) == NULL) { return (-1); }
	S->z = vNew;
	
	S->nMax = nAlloc;
	return (0);
}

/* Copy the contents a point set. */
int
M_PointSetCopy2(M_PointSet2 *D, const M_PointSet2 *S)
{
	if (M_PointSetAlloc2(D, S->n) == -1) {
		return (-1);
	}
	memcpy(D->p, S->p, S->n*sizeof(M_Vector2));
	return (0);
}
int
M_PointSetCopy3(M_PointSet3 *D, const M_PointSet3 *S)
{
	if (M_PointSetAlloc3(D, S->n) == -1) {
		return (-1);
	}
	memcpy(D->p, S->p, S->n*sizeof(M_Vector3));
	return (0);
}
int
M_PointSetCopy2i(M_PointSet2i *D, const M_PointSet2i *S)
{
	if (M_PointSetAlloc2i(D, S->n) == -1) {
		return (-1);
	}
	memcpy(D->x, S->x, S->n*sizeof(int));
	memcpy(D->y, S->y, S->n*sizeof(int));
	return (0);
}
int
M_PointSetCopy3i(M_PointSet3i *D, const M_PointSet3i *S)
{
	if (M_PointSetAlloc3i(D, S->n) == -1) {
		return (-1);
	}
	memcpy(D->x, S->x, S->n*sizeof(int));
	memcpy(D->y, S->y, S->n*sizeof(int));
	memcpy(D->z, S->z, S->n*sizeof(int));
	return (0);
}

/*
 * Point comparison routines for sort.
 */
static M_Real
ComparePoints2_XY(const void *_Nonnull p1, const void *_Nonnull p2)
{
	const M_Vector2 *v1 = p1, *v2 = p2;
	return (Fabs(v2->x - v1->x) <= M_MACHEP) ?
	       (v2->y - v1->y) : (v1->x - v2->x);
}
static M_Real
ComparePoints2_YX(const void *_Nonnull p1, const void *_Nonnull p2)
{
	const M_Vector2 *v1 = p1, *v2 = p2;
	return (Fabs(v2->y - v1->y) <= M_MACHEP) ?
	       (v2->x - v1->x) : (v1->y - v2->y);
}
static M_Real
ComparePoints3_XYZ(const void *_Nonnull p1, const void *_Nonnull p2)
{
	const M_Vector3 *v1 = p1, *v2 = p2;
	return (Fabs(v2->x - v1->x) <= M_MACHEP) ?
	       (Fabs(v2->y - v1->y) <= M_MACHEP) ?
	       (v2->z - v1->z) :
	       (v1->y - v2->y) :
	       (v1->x - v2->x);
}
static M_Real
ComparePoints3_XZY(const void *_Nonnull p1, const void *_Nonnull p2)
{
	const M_Vector3 *v1 = p1, *v2 = p2;
	return (Fabs(v2->x - v1->x) <= M_MACHEP) ?
	       (Fabs(v2->z - v1->z) <= M_MACHEP) ?
	       (v2->y - v1->y) :
	       (v1->z - v2->z) :
	       (v1->x - v2->x);
}
static M_Real
ComparePoints3_YXZ(const void *_Nonnull p1, const void *_Nonnull p2)
{
	const M_Vector3 *v1 = p1, *v2 = p2;
	return (Fabs(v2->y - v1->y) <= M_MACHEP) ?
	       (Fabs(v2->x - v1->x) <= M_MACHEP) ?
	       (v2->z - v1->z) :
	       (v1->x - v2->x) :
	       (v1->y - v2->y);
}
static M_Real
ComparePoints3_YZX(const void *_Nonnull p1, const void *_Nonnull p2)
{
	const M_Vector3 *v1 = p1, *v2 = p2;
	return (Fabs(v2->y - v1->y) <= M_MACHEP) ?
	       (Fabs(v2->z - v1->z) <= M_MACHEP) ?
	       (v2->x - v1->x) :
	       (v1->z - v2->z) :
	       (v1->y - v2->y);
}
static M_Real
ComparePoints3_ZXY(const void *_Nonnull p1, const void *_Nonnull p2)
{
	const M_Vector3 *v1 = p1, *v2 = p2;
	return (Fabs(v2->z - v1->z) <= M_MACHEP) ?
	       (Fabs(v2->x - v1->z) <= M_MACHEP) ?
	       (v2->y - v1->y) :
	       (v1->x - v2->x) :
	       (v1->z - v2->z);
}
static M_Real
ComparePoints3_ZYX(const void *_Nonnull p1, const void *_Nonnull p2)
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
M_PointSetSort2(M_PointSet2 *P, enum m_point_set_sort_mode2 mode)
{
	switch (mode) {
	case M_POINT_SET_SORT_XY:
		M_QSort(P->p, P->n, sizeof(M_Vector2), ComparePoints2_XY);
		break;
	case M_POINT_SET_SORT_YX:
		M_QSort(P->p, P->n, sizeof(M_Vector2), ComparePoints2_YX);
		break;
	}
}

/* Sort points in R3 by their X, Y or Z coordinates. */
void
M_PointSetSort3(M_PointSet3 *P, enum m_point_set_sort_mode3 mode)
{
	switch (mode) {
	case M_POINT_SET_SORT_XYZ:
		M_QSort(P->p, P->n, sizeof(M_Vector3), ComparePoints3_XYZ);
		break;
	case M_POINT_SET_SORT_XZY:
		M_QSort(P->p, P->n, sizeof(M_Vector3), ComparePoints3_XZY);
		break;
	case M_POINT_SET_SORT_YXZ:
		M_QSort(P->p, P->n, sizeof(M_Vector3), ComparePoints3_YXZ);
		break;
	case M_POINT_SET_SORT_YZX:
		M_QSort(P->p, P->n, sizeof(M_Vector3), ComparePoints3_YZX);
		break;
	case M_POINT_SET_SORT_ZXY:
		M_QSort(P->p, P->n, sizeof(M_Vector3), ComparePoints3_ZXY);
		break;
	case M_POINT_SET_SORT_ZYX:
		M_QSort(P->p, P->n, sizeof(M_Vector3), ComparePoints3_ZYX);
		break;
	}
}
