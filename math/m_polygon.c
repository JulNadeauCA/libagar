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
 * Polygon routines. Note that unlike most other math library structures,
 * M_Polygon uses dynamic allocation.
 */

#include <core/core.h>
#include "m.h"

void
M_PolygonInit2(M_Polygon2 *P)
{
	P->s = NULL;
	P->n = 0;
	P->vn = 0;
}

void
M_PolygonInit3(M_Polygon3 *P)
{
	P->s = NULL;
	P->n = 0;
	P->vn = 0;
}

void
M_PolygonFree2(M_Polygon2 *P)
{
	Free(P->s);
	P->s = NULL;
	P->n = 0;
	P->vn = 0;
}

void
M_PolygonFree3(M_Polygon3 *P)
{
	Free(P->s);
	P->s = NULL;
	P->n = 0;
	P->vn = 0;
}

M_Polygon2
M_PolygonRead2(AG_DataSource *ds)
{
	M_Polygon2 P;
	Uint i;

	P.n = (Uint)AG_ReadUint32(ds);
	P.vn = P.n*2;
	P.s = Malloc(P.n*sizeof(M_Line2));
	for (i = 0; i < P.n; i++) {
		P.s[i] = M_LineRead2(ds);
	}
	return (P);
}

M_Polygon3
M_PolygonRead3(AG_DataSource *ds)
{
	M_Polygon3 P;
	Uint i;

	P.n = (Uint)AG_ReadUint32(ds);
	P.vn = P.n*2;
	P.s = Malloc(P.n*sizeof(M_Line3));
	for (i = 0; i < P.n; i++) {
		P.s[i] = M_LineRead3(ds);
	}
	return (P);
}

void
M_PolygonWrite2(AG_DataSource *ds, M_Polygon2 *P)
{
	Uint i;

	AG_WriteUint32(ds, (Uint32)P->n);
	for (i = 0; i < P->n; i++)
		M_LineWrite2(ds, &P->s[i]);
}

void
M_PolygonWrite3(AG_DataSource *ds, M_Polygon3 *P)
{
	Uint i;

	AG_WriteUint32(ds, (Uint32)P->n);
	for (i = 0; i < P->n; i++)
		M_LineWrite3(ds, &P->s[i]);
}

/* Create a new polygon in R^2 from a set of lines. */
M_Polygon2
M_PolygonFromLines2(Uint n, M_Line2 *lines)
{
	M_Polygon2 P;
	Uint i;

	P.s = Malloc(n*sizeof(M_Line2));
	P.n = n;
	P.vn = n*2;
	for (i = 0; i < n; i++) {
		P.s[i] = lines[i];
	}
	return (P);
}

/* Create a new polygon in R^3 from a set of lines. */
M_Polygon3
M_PolygonFromLines3(Uint n, M_Line3 *lines)
{
	M_Polygon3 P;
	Uint i;

	P.s = Malloc(n*sizeof(M_Line3));
	P.n = n;
	P.vn = n*2;
	for (i = 0; i < n; i++) {
		P.s[i] = lines[i];
	}
	return (P);
}

/*
 * Add a line segment to a polygon in R^2.
 * Return segment index on success, -1 on failure.
 */
int
M_PolygonAddLine2(M_Polygon2 *P, M_Line2 L)
{
	M_Line2 *sNew;

	if ((sNew = TryRealloc(P->s, (P->n+1)*sizeof(M_Line2))) == NULL) {
		return (-1);
	}
	P->s = sNew;
	P->s[P->n] = L;
	P->vn += 2;
	return (P->n++);
}

/*
 * Add a line segment to a polygon in R^3.
 * Return segment index on success, -1 on failure.
 */
int
M_PolygonAddLine3(M_Polygon3 *P, M_Line3 L)
{
	M_Line3 *sNew;

	if ((sNew = TryRealloc(P->s, (P->n+1)*sizeof(M_Line3))) == NULL) {
		return (-1);
	}
	P->s = sNew;
	P->s[P->n] = L;
	P->vn += 2;
	return (P->n++);
}

/*
 * Add a vertex to a polygon in R^2.
 * Return vertex index on success, -1 on failure.
 */
int
M_PolygonAddVertex2(M_Polygon2 *P, M_Vector2 v)
{
	M_Line2 *sNew;

	if (((P->vn+1)%2) == 1) {
		if ((sNew = TryRealloc(P->s, (P->n+1)*sizeof(M_Line2))) == NULL) {
			return (-1);
		}
		P->s = sNew;
	}
	if (P->n == 0) {
		P->s[0] = M_LineFromPts2(v, v);			/* Degenerate */
	} else {
		M_Line2 *sPrev = &P->s[P->n - 1];
		*sPrev = M_LineFromPts2(sPrev->p, v);
		P->s[P->n] = M_LineFromPts2(v, P->s[0].p);	/* Close */
	}
	if (((P->n+1)%2) == 1) {
		P->n++;
	}
	return (P->vn++);
}

/*
 * Add a vertex to a polygon in R^3.
 * Return vertex index on success, -1 on failure.
 */
int
M_PolygonAddVertex3(M_Polygon3 *P, M_Vector3 v)
{
	M_Line3 *sNew;

	if (((P->vn+1)%2) == 1) {
		if ((sNew = TryRealloc(P->s, (P->n+1)*sizeof(M_Line3))) == NULL) {
			return (-1);
		}
		P->s = sNew;
	}
	if (P->n == 0) {
		P->s[0] = M_LineFromPts3(v, v);			/* Degenerate */
	} else {
		M_Line3 *sPrev = &P->s[P->n - 1];
		*sPrev = M_LineFromPts3(sPrev->p, v);
		P->s[P->n] = M_LineFromPts3(v, P->s[0].p);	/* Close */
	}
	if (((P->n+1)%2) == 1) {
		P->n++;
	}
	return (P->vn++);
}

/* Remove a line segment from a polygon in R^2. */
int
M_PolygonDelLine2(M_Polygon2 *P, int sIdx)
{
	if (sIdx < 0 || sIdx >= P->n) {
		AG_SetError("Bad segment index");
		return (-1);
	}
	if (sIdx < P->n-1) {
		memmove(&P->s[sIdx], &P->s[sIdx+1],
		    (P->n - 1)*sizeof(M_Line2));
	}
	P->n--;
	P->vn -= 2;
	return (0);
}

/* Remove a line segment from a polygon in R^3. */
int
M_PolygonDelLine3(M_Polygon3 *P, int sIdx)
{
	if (sIdx < 0 || sIdx >= P->n) {
		AG_SetError("Bad segment index");
		return (-1);
	}
	if (sIdx < P->n-1) {
		memmove(&P->s[sIdx], &P->s[sIdx+1],
		    (P->n - 1)*sizeof(M_Line3));
	}
	P->n--;
	P->vn -= 2;
	return (0);
}

/* Test whether the given point lies inside the polygon. */
int
M_PointInPolygon2(M_Polygon2 *poly, M_Vector2 p)
{
	int i, count = 0;
	M_Real ix;
	M_Vector2 p1, p2;

	p1 = poly->s[0].p;
	for (i = 1; i <= poly->n; i++) {
		p2 = poly->s[i % poly->n].p;

		if (p.y >  MIN(p1.y, p2.y) &&
		    p.y <= MAX(p1.y, p2.y) &&
		    p.x <= MAX(p1.x, p2.x)) {
			if (p1.y != p2.y) {
				/* 
				 * Compute the intersection of the X axis
				 * with this line segment.
				 */
				ix = (p.y - p1.y)*(p2.x - p1.x) /
				     (p2.y - p1.y) + p1.x;
				if (p1.x == p2.x || p.x <= ix)
					count++;
			}
		}

		p1 = p2;
	}

	if (count%2 == 0) {
		return (0);
	} else {
		return (1);
	}
}

/* Test whether the given polygon is convex (1) or concave (0). */
/* XXX assumes no self-intersections */
int
M_PolygonIsConvex2(M_Polygon2 *P)
{
	int i, flag = 0;

	if (P->n < 3) {
		AG_SetError("<3 vertices");
		return (-1);
	}
	for (i = 0; i < P->n; i++) {
		M_Vector2 pi = P->s[(i)   % P->n].p;
		M_Vector2 pj = P->s[(i+1) % P->n].p;
		M_Vector2 pk = P->s[(i+2) % P->n].p;
		M_Vector2 pji = M_VecSub2p(&pj, &pi);
		M_Vector2 pkj = M_VecSub2p(&pk, &pj);
		M_Real dot;

		dot = M_VecPerpDot2p(&pji, &pkj);
		if (dot < 0.0) {
			flag |= 1;
		} else if (dot >= 0.0) {
			flag |= 2;
		}
		if (flag == 3) {
			return (0);
		}
	}
	if (flag != 0) {
		return (1);
	}
	return (0);
}

/* Test whether the given polygon is planar to machine precision. */
int
M_PolygonIsPlanar3(M_Polygon3 *P)
{
	M_Plane3 Pl;
	Uint i;

	if (P->n < 3) {
		AG_SetError("<3 vertices");
		return (-1);
	}
	if (P->n == 3)
		return (1);

	Pl = M_PlaneFromPts3(P->s[0].p, P->s[1].p, P->s[2].p);
	for (i = 3; i < P->n; i++) {
		M_Vector3 p = P->s[i].p;

		if ((Pl.a*p.x + Pl.b*p.y + Pl.c*p.z + Pl.d) > M_MACHEP)
			return (0);
	}
	return (1);
}
