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
	
	L.p = p1;
	L.d.x = p2.x - p1.x;
	L.d.y = p2.y - p1.y;
	L.t = VecLen2p(&L.d);
	L.d.x /= L.t;
	L.d.y /= L.t;
	return (L);
}

/* Create a line from two points in R3. */
SG_Line
SG_LineFromPts(SG_Vector p1, SG_Vector p2)
{
	SG_Line L;
	
	L.p = p1;
	L.d.x = p2.x - p1.x;
	L.d.y = p2.y - p1.y;
	L.d.z = p2.z - p1.z;
	L.t = VecLenp(&L.d);
	L.d.x /= L.t;
	L.d.y /= L.t;
	L.d.z /= L.t;
	return (L);
}

/* Create a plane from three points in R3. */
SG_Plane
SG_PlaneFrom3Pts(SG_Vector p1, SG_Vector p2, SG_Vector p3)
{
	SG_Plane P;
	SG_Vector n;

	n = VecNormCross(VecSub(p1,p2), VecSub(p3,p2));
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
	return (SG_PI - Acos(VecDot(SG_PlaneNormp(&P),VecNormp(&v))));
}

/* Compute minimal distance from a line segment L to a point p. */
SG_Real
SG_PointLineDistance2(SG_Vector2 p, SG_Line2 L)
{
	SG_Real u;
	SG_Vector2 d;
	SG_Vector2 x;

	u = ((p.x - L.p.x)*(L.d.x*L.t - L.p.y) +
	     (p.y - L.p.y)*(L.d.y*L.t - L.p.y)) / (L.t*L.t);
	d = L.d;
	VecScale2v(&d, L.t);
	x = VecAdd2(L.p, VecScale2(VecSub2p(&d,&L.p), u));
	return (VecDistance2p(&p, &x));
}

SG_Real
SG_LineLineAngle(SG_Line L1, SG_Line L2)
{
	SG_Real theta;

	theta = SG_PI - Atan2(VecPerpDot2(Vec3to2(L1.d),Vec3to2(L2.d)),
	                      VecDot(L1.d,L2.d));
	return (theta);
}

SG_Real
SG_LineLineAngle2(SG_Line2 L1, SG_Line2 L2)
{
	return (Atan2(L2.d.y - L1.d.y,
	              L2.d.x - L1.d.x));
}

/* Compute intersection between two line segments in R2. */
SG_Intersect2
SG_IntersectLineLine2(SG_Line2 L1, SG_Line2 L2)
{
	SG_Intersect2 ix;
	SG_Real a = L2.d.x*(L1.p.y - L2.p.y) - L2.d.y*(L1.p.x - L2.p.x);
	SG_Real b = L1.d.x*(L1.p.y - L2.p.y) - L1.d.y*(L1.p.x - L2.p.x);
	SG_Real c = L2.d.y*L1.d.x - L2.d.x*L1.d.y;

	if (c != 0.0) {
		SG_Real ac = a/c;
		SG_Real bc = b/c;

		if (ac >= 0.0 && ac <= 1.0 &&
		    bc >= 0.0 && bc <= 1.0) {
			ix.type = SG_POINT;
			ix.ix_p = VecAdd2(L1.p, VecScale2(L1.d,ac));
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
