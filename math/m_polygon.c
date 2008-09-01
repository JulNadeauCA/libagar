/*
 * Copyright (c) 2008 Hypertriton, Inc. <http://hypertriton.com/>
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
M_PolygonInit2(M_Polygon2 *poly)
{
	poly->s = NULL;
	poly->n = 0;
}

void
M_PolygonInit3(M_Polygon3 *poly)
{
	poly->s = NULL;
	poly->n = 0;
}

void
M_PolygonFree2(M_Polygon2 *poly)
{
	Free(poly->s);
	poly->s = NULL;
	poly->n = 0;
}

void
M_PolygonFree3(M_Polygon3 *poly)
{
	Free(poly->s);
	poly->s = NULL;
	poly->n = 0;
}

M_Polygon2
M_PolygonRead2(AG_DataSource *ds)
{
	M_Polygon2 P;
	Uint i;

	P.n = (Uint)AG_ReadUint32(ds);
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

M_Polygon2
M_PolygonFromLines2(Uint n, M_Line2 *lines)
{
	M_Polygon2 P;
	Uint i;

	P.s = Malloc(n*sizeof(M_Line2));
	P.n = n;
	for (i = 0; i < n; i++) {
		P.s[i] = lines[i];
	}
	return (P);
}

M_Polygon3
M_PolygonFromLines3(Uint n, M_Line3 *lines)
{
	M_Polygon3 P;
	Uint i;

	P.s = Malloc(n*sizeof(M_Line3));
	P.n = n;
	for (i = 0; i < n; i++) {
		P.s[i] = lines[i];
	}
	return (P);
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
/* XXX test; assumes no self-intersections */
int
M_PolygonIsConvex2(M_Polygon2 *poly)
{
	int i, flag = 0;

	if (poly->n < 3) {
		AG_SetError("<3 vertices");
		return (-1);
	}
	for (i = 0; i < poly->n; i++) {
		M_Vector2 pi = poly->s[(i)   % poly->n].p;
		M_Vector2 pj = poly->s[(i+1) % poly->n].p;
		M_Vector2 pk = poly->s[(i+2) % poly->n].p;
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
M_PolygonIsPlanar3(M_Polygon3 *poly)
{
	M_Plane3 P;
	Uint i;

	if (poly->n < 3) {
		AG_SetError("<3 vertices");
		return (-1);
	}
	if (poly->n == 3)
		return (1);

	P = M_PlaneFromPts3(poly->s[0].p, poly->s[1].p, poly->s[2].p);
	for (i = 3; i < poly->n; i++) {
		M_Vector3 p = poly->s[i].p;

		if ((P.a*p.x + P.b*p.y + P.c*p.z + P.d) > M_MACHEP)
			return (0);
	}
	return (1);
}
