/*
 * Copyright (c) 2007-2008 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Routines related to lines, line segments and rays.
 */

#include <core/core.h>
#include "m.h"

M_Line2
M_LineRead2(AG_DataSource *ds)
{
	M_Line2 L;

	L.p = M_ReadVector2(ds);
	L.d = M_ReadVector2(ds);
	L.t = M_ReadReal(ds);
	return (L);
}

M_Line3
M_LineRead3(AG_DataSource *ds)
{
	M_Line3 L;

	L.p = M_ReadVector3(ds);
	L.d = M_ReadVector3(ds);
	L.t = M_ReadReal(ds);
	return (L);
}

void
M_LineWrite2(AG_DataSource *ds, M_Line2 *L)
{
	M_WriteVector2(ds, &L->p);
	M_WriteVector2(ds, &L->d);
	M_WriteReal(ds, L->t);
}

void
M_LineWrite3(AG_DataSource *ds, M_Line3 *L)
{
	M_WriteVector3(ds, &L->p);
	M_WriteVector3(ds, &L->d);
	M_WriteReal(ds, L->t);
}

/* Create a line from a point, direction vector and length. */
M_Line2
M_LineFromPtDir2(M_Vector2 p, M_Vector2 d, M_Real len)
{
	M_Line2 L;

	L.p = p;
	L.d = d;
	L.t = len;
	return (L);
}

/* Create a line from a point, direction vector and length. */
M_Line3
M_LineFromPtDir3(M_Vector3 p, M_Vector3 d, M_Real len)
{
	M_Line3 L;

	L.p = p;
	L.d = d;
	L.t = len;
	return (L);
}

/* Create a line from two points in R2. */
M_Line2
M_LineFromPts2(M_Vector2 p1, M_Vector2 p2)
{
	M_Line2 L;
	
	L.p = p1;
	L.d.x = p2.x - p1.x;
	L.d.y = p2.y - p1.y;
	L.t = M_VecLen2p(&L.d);
	L.d.x /= L.t;
	L.d.y /= L.t;
	return (L);
}

/* Create a line from two points in R3. */
M_Line3
M_LineFromPts3(M_Vector3 p1, M_Vector3 p2)
{
	M_Line3 L;
	
	L.p = p1;
	L.d = M_VecSub3p(&p2, &p1);
	L.t = M_VecLen3p(&L.d);
	M_VecScale3v(&L.d, 1.0/L.t);
	return (L);
}

/*
 * Compute a new line parallel to the given line, with perpendicular
 * endpoints.
 */
M_Line2
M_LineParallel2(M_Line2 L, M_Real dist)
{
	M_Vector2 p1, p2, pd;

	M_LineToPts2(L, &p1, &p2);
	pd.x =  L.d.y;
	pd.y = -L.d.x;
	M_VecScale2v(&pd, dist);
	M_VecAdd2v(&p1, &pd);
	M_VecAdd2v(&p2, &pd);
	return M_LineFromPts2(p1, p2);
}

/*
 * Compute a new line parallel to the given line, with perpendicular
 * endpoints. XXX this is a circle of solutions in R3
 */
M_Line3
M_LineParallel3(M_Line3 L, M_Real dist)
{
	M_Vector3 p1, p2, pd;

	M_LineToPts3(L, &p1, &p2);
	pd.x =  L.d.y;
	pd.y = -L.d.x;
	M_VecScale3v(&pd, dist);
	M_VecAdd3v(&p1, &pd);
	M_VecAdd3v(&p2, &pd);
	return M_LineFromPts3(p1, p2);
}

/* Compute a line in R2 from the projection on the X-Y plane of a line in R3. */
M_Line2
M_LineProject2(M_Line3 L3)
{
	M_Line2 L2;

	L2.p = M_Vector3to2(L3.p);
	L2.d = M_Vector3to2(L3.d);
	L2.t = M_VecLen2p(&L2.d);
	L2.d.x /= L2.t;
	L2.d.y /= L2.t;
	return (L2);
}

/* Project a line in R2 onto the X-Y plane of R3. */
M_Line3
M_LineProject3(M_Line2 L2)
{
	M_Line3 L3;

	L3.p = M_Vector2to3(L2.p);
	L3.d = M_Vector2to3(L2.d);
	L3.t = M_VecLen3p(&L3.d);
	L3.d.x /= L3.t;
	L3.d.y /= L3.t;
	L3.d.z /= L3.t;
	return (L3);
}

/* Compute minimal distance from a line segment L to a point p. */
M_Real
M_LinePointDistance2(M_Line2 L, M_Vector2 p3)
{
	M_Vector2 p1, p2, s;
	M_Real u;

	/* [p3 - p1 - u(p2-p1)] dot (p2-p1) = 0 */
	M_LineToPts2(L, &p1, &p2);
	u = ((p3.x - p1.x)*(p2.x - p1.x) +
	     (p3.y - p1.y)*(p2.y - p1.y)) / (L.t*L.t);

	/* s = p1 + u(p2-p1) */
	s = M_VecAdd2(p1, M_VecScale2(M_VecSub2(p2,p1), u));
	return M_VecDistance2p(&p3, &s);
}

/* Compute minimal distance from a line segment L to a point p. */
M_Real
M_LinePointDistance3(M_Line3 L, M_Vector3 p3)
{
	M_Vector3 p1, p2, s;
	M_Real u;

	/* [p3 - p1 - u(p2-p1)] dot (p2-p1) = 0 */
	M_LineToPts3(L, &p1, &p2);
	u = ((p3.x - p1.x)*(p2.x - p1.x) +
	     (p3.y - p1.y)*(p2.y - p1.y) +
	     (p3.z - p1.z)*(p2.z - p1.z)) / (L.t*L.t);

	/* s = p1 + u(p2-p1) */
	s = M_VecAdd3(p1, M_VecScale3(M_VecSub3(p2,p1), u));
	return M_VecDistance3p(&p3, &s);
}

/* Compute the CCW angle between two lines in R2. */
M_Real
M_LineLineAngle2(M_Line2 L1, M_Line2 L2)
{
	return (Atan2(L2.d.y - L1.d.y,
	              L2.d.x - L1.d.x));
}

/* Compute the CCW angle between two lines in R3. */
M_Real
M_LineLineAngle3(M_Line3 L1, M_Line3 L2)
{
	return Acos(M_VecDot3(L1.d, L2.d));
}

/* Compute intersection between two line segments in R2. */
M_GeomSet2
M_IntersectLineLine2(M_Line2 L1, M_Line2 L2)
{
	M_GeomSet2 Sint = M_GEOM_SET_EMPTY;
	M_Vector2 a1 = M_LineFirstPt2(L1);
	M_Vector2 a2 = M_LineSecondPt2(L1);
	M_Vector2 b1 = M_LineFirstPt2(L2);
	M_Vector2 b2 = M_LineSecondPt2(L2);
	M_Real a = (b2.x - b1.x)*(a1.y - b1.y) - (b2.y - b1.y)*(a1.x - b1.x);
	M_Real b = (a2.x - a1.x)*(a1.y - b1.y) - (a2.y - a1.y)*(a1.x - b1.x);
	M_Real c = (b2.y - b1.y)*(a2.x - a1.x) - (b2.x - b1.x)*(a2.y - a1.y);
	M_Geom2 G;

	if (c != 0.0) {
		M_Real ac = a/c;
		M_Real bc = b/c;

		if (ac >= 0.0 && ac <= 1.0 &&
		    bc >= 0.0 && bc <= 1.0) {
			G.type = M_POINT;
			G.g_point = M_VecAdd2(a1,
			                      M_VecScale2(M_VecSub2(a2,a1),ac));
			M_GeomSetAdd2(&Sint, &G);
		}
	} else {
		/* XXX TODO */
		if (a == 0.0 || b == 0.0) {
			G.type = M_LINE;
			G.g_line = L1;
			M_GeomSetAdd2(&Sint, &G);
		}
	}
	return (Sint);
}

