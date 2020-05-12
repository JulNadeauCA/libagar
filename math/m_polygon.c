/*
 * Copyright (c) 2008-2011 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Polygon routines.
 */

#include <agar/core/core.h>
#include <agar/math/m.h>

void
M_PolygonInit(M_Polygon *P)
{
	P->v = NULL;
	P->n = 0;
}

void
M_PolygonFree(M_Polygon *P)
{
	Free(P->v);
	P->v = NULL;
	P->n = 0;
}

M_Polygon
M_PolygonRead(AG_DataSource *ds)
{
	M_Polygon P;
	Uint i;

	P.n = (Uint)AG_ReadUint32(ds);
	P.v = Malloc(P.n*sizeof(M_Vector2));
	for (i = 0; i < P.n; i++) {
		P.v[i] = M_ReadVector2(ds);
	}
	P._pad = 0;
	return (P);
}

void
M_PolygonWrite(AG_DataSource *ds, const M_Polygon *P)
{
	Uint i;

	AG_WriteUint32(ds, (Uint32)P->n);
	for (i = 0; i < P->n; i++)
		M_WriteVector2(ds, &P->v[i]);
}

/* Duplicate a polygon structure. */
int
M_PolygonCopy(M_Polygon *D, const M_Polygon *S)
{
	if ((D->v = TryMalloc(S->n*sizeof(M_Vector2))) == NULL) {
		return (-1);
	}
	D->n = S->n;
	memcpy(D->v, S->v, S->n*sizeof(M_Vector2));
	return (0);
}

/* Scale the vertices of a polygon. */
void
M_PolygonScale(M_Polygon *P, M_Real xs, M_Real ys)
{
	Uint i;

	for (i = 0; i < P->n; i++) {
		P->v[i].x = xs*P->v[i].x;
		P->v[i].y = ys*P->v[i].y;
	}
}

/* Offset the vertices of a polygon. */
void
M_PolygonOffset(M_Polygon *P, M_Real xo, M_Real yo)
{
	Uint i;

	for (i = 0; i < P->n; i++) {
		P->v[i].x += xo;
		P->v[i].y += yo;
	}
}

/* Create a new polygon from an array of vectors. */
M_Polygon
M_PolygonFromPts(Uint n, const M_Vector2 *v)
{
	M_Polygon P;

	P.v = Malloc(n*sizeof(M_Vector2));
	P.n = n;
	memcpy(P.v, v, n*sizeof(M_Vector2));
	P._pad = 0;
	return (P);
}

/* Create a new polygon from a M_PointSet2. */
M_Polygon
M_PolygonFromPointSet2(const M_PointSet2 *ps)
{
	M_Polygon P;

	P.v = Malloc(ps->n*sizeof(M_Vector2));
	P.n = ps->n;
	memcpy(P.v, ps->p, ps->n*sizeof(M_Vector2));
	P._pad = 0;
	return (P);
}

/* Create a new polygon from a M_PointSet2i. */
M_Polygon
M_PolygonFromPointSet2i(const M_PointSet2i *ps)
{
	M_Polygon P;
	Uint i;

	P.v = Malloc(ps->n*sizeof(M_Vector2));
	P.n = ps->n;
	for (i = 0; i < ps->n; i++) {
		P.v[i].x = ((M_Real)ps->x[i])/ps->w;
		P.v[i].y = ((M_Real)ps->y[i])/ps->h;
	}
	P._pad = 0;
	return (P);
}

/* Convert polygon to M_PointSet2 */
M_PointSet2
M_PolygonToPointSet2(const M_Polygon *P)
{
	M_PointSet2 ps;

	M_PointSetInit2(&ps);
	M_PointSetAlloc2(&ps, P->n);
	memcpy(ps.p, P->v, P->n*sizeof(M_Vector2));
	return (ps);
}

/* Convert polygon to a M_PointSet2i of specified real dimensions. */
M_PointSet2i
M_PolygonToPointSet2i(const M_Polygon *P, M_Real w, M_Real h)
{
	M_PointSet2i ps;
	Uint i;

	M_PointSetInit2i(&ps, w, h);
	M_PointSetAlloc2i(&ps, P->n);
	for (i = 0; i < P->n; i++) {
		ps.x[i] = (int)(P->v[i].x*w);
		ps.y[i] = (int)(P->v[i].y*h);
	}
	return (ps);
}

/*
 * Create a new polygon from a set of lines.
 * XXX TODO sanity check point order
 */
M_Polygon
M_PolygonFromLines(Uint n, const M_Line2 *ln)
{
	M_Polygon P;
	Uint i;

	P.v = Malloc(n*sizeof(M_Vector2));
	P.n = n;
	for (i = 0; i < n; i++) {
		P.v[i] = ln[i].p;
	}
	P._pad = 0;
	return (P);
}

/*
 * Append a line segment to a polygon.
 * Return segment index on success, -1 on failure.
 * XXX TODO sanity check point order
 */
int
M_PolygonAddLine(M_Polygon *P, M_Line2 L)
{
	M_Vector2 *vNew;

	if ((vNew = TryRealloc(P->v, (P->n+1)*sizeof(M_Vector2))) == NULL) {
		return (-1);
	}
	P->v = vNew;
	P->v[P->n] = L.p;
	return (P->n++);
}

/*
 * Add a vertex to a polygon.
 * Return vertex index on success, -1 on failure.
 */
int
M_PolygonAddVertex(M_Polygon *_Nonnull P, M_Vector2 v)
{
	M_Vector2 *vNew;

	if ((vNew = TryRealloc(P->v, (P->n+1)*sizeof(M_Vector2))) == NULL) {
		return (-1);
	}
	P->v = vNew;
	P->v[P->n] = v;
	return (P->n++);
}

/* Remove a vertex from a polygon. */
int
M_PolygonDelVertex(M_Polygon *P, int i)
{
	if (i < 0 || i >= P->n) {
		AG_SetError("Bad vertex");
		return (-1);
	}
	if (i < P->n - 1) {
		memmove(&P->v[i], &P->v[i+1], (P->n - i - 1)*sizeof(M_Vector2));
	}
	P->n--;
	return (0);
}

/* Test whether point p lies inside the polygon. */
int
M_PointInPolygon(const M_Polygon *P, M_Vector2 p)
{
	int i, count = 0;
	M_Real ix;
	M_Vector2 p1, p2;

	if (P->n < 3)
		return (0);

	p1 = P->v[0];
	for (i = 1; i <= P->n; i++) {
		p2 = P->v[i % P->n];

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

/*
 * Test whether the polygon is convex. Polygon must be non self-intersecting.
 * Return 1 if the polygon is convex or 0 if the polygon is concave.
 */
int
M_PolygonIsConvex(const M_Polygon *P)
{
	int i, flag = 0;

	if (P->n < 3) {
		AG_SetError("<3 vertices");
		return (-1);
	}
	for (i = 0; i < P->n; i++) {
		M_Vector2 pi = P->v[(i)   % P->n];
		M_Vector2 pj = P->v[(i+1) % P->n];
		M_Vector2 pk = P->v[(i+2) % P->n];
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
