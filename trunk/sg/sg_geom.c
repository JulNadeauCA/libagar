/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Geometry routines.
 */

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>
#include "sg.h"

/* Create a line from two points in R2. */
SG_Line2
SG_LineFromPts2(SG_Vector2 p1, SG_Vector2 p2)
{
	SG_Line2 L;
	
	L.x0 = p1.x;
	L.y0 = p1.y;
	L.dx = p2.x - p1.x;
	L.dy = p2.y - p1.y;
	L.t = SG_VectorDistance2p(&p1, &p2);
	L.dx /= L.t;
	L.dy /= L.t;
	return (L);
}

/* Create a line from two points in R3. */
SG_Line
SG_LineFromPts(SG_Vector p1, SG_Vector p2)
{
	SG_Vector vd;
	SG_Line L;
	
	L.x0 = p1.x;
	L.y0 = p1.y;
	L.z0 = p1.z;
	L.dx = p2.x - p1.x;
	L.dy = p2.y - p1.y;
	L.dz = p2.z - p1.z;
	L.t = SG_VectorDistancep(&p1, &p2);
	L.dx /= L.t;
	L.dy /= L.t;
	L.dz /= L.t;
	return (L);
}

/* Create a plane from three points in R3. */
SG_Plane
SG_PlaneFrom3Pts(SG_Vector p1, SG_Vector p2, SG_Vector p3)
{
	SG_Plane P;
	SG_Vector n;

	n = SG_VectorNormCross(SG_VectorSub(p1,p2), SG_VectorSub(p3,p2));
	P.a = n.x;
	P.b = n.y;
	P.c = n.z;
	P.d = -(P.a*p1.x + P.b*p1.y + P.c*p1.z);
	return (P);
}

/* Create a plane from a normal and distance from origin. */
SG_Plane
SG_PlaneFromNormal(SG_Vector n, SG_Real d)
{
	SG_Plane P;

	P.a = n.x;
	P.b = n.y;
	P.c = n.z;
	P.d = d;
	return (P);
}

/* Create a plane at distance d from another plane. */
SG_Plane
SG_PlaneAtDistance(SG_Plane P1, SG_Real d)
{
	SG_Plane P2;

	P2.a = P1.a;
	P2.b = P1.b;
	P2.c = P1.c;
	P2.d = P1.d + d;
	return (P2);
}

SG_Plane
SG_PlaneRead(AG_Netbuf *buf)
{
	SG_Plane P;

	P.a = SG_ReadReal(buf);
	P.b = SG_ReadReal(buf);
	P.c = SG_ReadReal(buf);
	P.d = SG_ReadReal(buf);
	return (P);
}

void
SG_PlaneWrite(AG_Netbuf *buf, SG_Plane *P)
{
	SG_WriteReal(buf, P->a);
	SG_WriteReal(buf, P->b);
	SG_WriteReal(buf, P->c);
	SG_WriteReal(buf, P->d);
}

int
SG_PlaneIsValid(SG_Plane P)
{
	return (P.a != 0.0 && P.b != 0.0 && P.c != 0.0);
}

SG_Vector
SG_PlaneNorm(SG_Plane P)
{
	SG_Vector n;

	n.x = P.a;
	n.y = P.b;
	n.z = P.c;
	return (n);
}

SG_Vector
SG_PlaneNormp(const SG_Plane *P)
{
	SG_Vector n;

	n.x = P->a;
	n.y = P->b;
	n.z = P->c;
	return (n);
}

SG_Real
SG_VectorPlaneAngle(SG_Vector v, SG_Plane P)
{
	return (M_PI/2.0 -
	        SG_Acos(SG_VectorDot(SG_PlaneNormp(&P),
		                     SG_VectorNormp(&v))));
}

/* Compute minimal distance from a line segment L to a point p. */
SG_Real
SG_PointLineDistance2(SG_Vector2 p, SG_Line2 L)
{
	SG_Real u;
	SG_Vector2 v0, v1;
	SG_Vector2 x;

	u = ((p.x - L.x0)*(L.dx*L.t - L.x0) +
	     (p.y - L.y0)*(L.dy*L.t - L.y0)) / (L.t*L.t);

	v0 = SG_VECTOR2(L.x0, L.y0);
	v1 = SG_VECTOR2(L.dx, L.dy);
	SG_VectorScale2v(&v1, L.t);

	x = SG_VectorAdd2(v0, SG_VectorScale2(SG_VectorSub2p(&v1,&v0), u));
	return (SG_VectorDistance2p(&p, &x));
}

SG_Real
SG_LineLineAngle2(SG_Line2 L1, SG_Line2 L2)
{
	return (Atan2(L2.dy-L1.dy, L2.dx-L1.dx));
}

/* Compute intersection between two line segments in R2. */
SG_Intersect2
SG_IntersectLineLine2(SG_Line2 L1, SG_Line2 L2)
{
	SG_Intersect2 ix;
	SG_Real a = L2.dx*(L1.y0 - L2.y0) - L2.dy*(L1.x0 - L2.x0);
	SG_Real b = L1.dx*(L1.y0 - L2.y0) - L1.dy*(L1.x0 - L2.x0);
	SG_Real c = L2.dy*L1.dx - L2.dx*L1.dy;

	if (c != 0.0) {
		SG_Real ac = a/c;
		SG_Real bc = b/c;

		if (ac >= 0.0 && ac <= 1.0 &&
		    bc >= 0.0 && bc <= 1.0) {
			ix.type = SG_POINT;
			ix.ix_p = SG_VECTOR2(L1.x0 + ac*L1.dx,
			                     L1.y0 + ac*L1.dy);
			return (ix);
		} else {
			ix.type = SG_NONE;
			return (ix);
		}
	} else {
		if (a == 0.0 || b == 0.0) {
			ix.type = SG_LINE;
			ix.ix_L = L1;		/* XXX */
			return (ix);
		} else {
			ix.type = SG_NONE;
			return (ix);
		}
	}
}

#endif /* HAVE_OPENGL */
