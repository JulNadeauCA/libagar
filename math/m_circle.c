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
 * Routines related to circles.
 */

#include <agar/core/core.h>
#include <agar/math/m.h>

M_Circle2
M_CircleRead2(AG_DataSource *ds)
{
	M_Circle2 C;

	C.p = M_ReadVector2(ds);
	C.r = M_ReadReal(ds);
	return (C);
}
M_Circle3
M_CircleRead3(AG_DataSource *ds)
{
	M_Circle3 C;

	C.p = M_ReadVector3(ds);
	C.r = M_ReadReal(ds);
	return (C);
}

void
M_CircleWrite2(AG_DataSource *ds, M_Circle2 *C)
{
	M_WriteVector2(ds, &C->p);
	M_WriteReal(ds, C->r);
}
void
M_CircleWrite3(AG_DataSource *ds, M_Circle3 *C)
{
	M_WriteVector3(ds, &C->p);
	M_WriteReal(ds, C->r);
}

/* Create a circle from a point and radius. */
M_Circle2
M_CircleFromPt(M_Vector2 p, M_Real r)
{
	M_Circle2 C;

	C.p = p;
	C.r = r;
	return (C);
}

/* Compute minimal distance from a circle to a point p. */
M_Real
M_CirclePointDistance2(M_Circle2 C, M_Vector2 p)
{
	M_Vector2 vR = M_VecSub2(p, C.p);
	M_Real theta = Atan2(vR.y, vR.x);

	return M_VecDistance2(p, M_VECTOR2(C.p.x + C.r*Cos(theta),
	                                   C.p.y + C.r*Sin(theta)));
}

/* Compute the intersection of two circles. */
M_GeomSet2
M_IntersectCircleCircle2(M_Circle2 C1, M_Circle2 C2)
{
	M_GeomSet2 Sint = M_GEOM_SET_EMPTY;
	M_Real d12 = M_VecDistance2(C1.p, C2.p);
	M_Real a, h, b;
	M_Vector2 p;
	M_Geom2 G1, G2;

	if (Fabs(C1.p.x - C2.p.x) <= M_MACHEP &&
	    Fabs(C1.p.x - C2.p.x) <= M_MACHEP &&
	    Fabs(C1.r - C2.r) <= M_MACHEP) {
		G1.type = M_CIRCLE;
		G1.g.circle = C1;
		M_GeomSetAdd2(&Sint, &G1);
		return (Sint);
	}

	if (d12 > (C1.r + C2.r) ||
	    d12 < Fabs(C1.r - C2.r)) {
		return (Sint);
	}

	a = (C1.r*C1.r - C2.r*C2.r + d12*d12) / (2.0*d12);
	h = Sqrt(C1.r*C1.r - a*a);
	p = M_VecLERP2(C1.p, C2.p, a/d12);
	b = h/d12;

	G1.type = M_POINT;
	G1.g.point.x = p.x - b*(C2.p.y - C1.p.y);
	G1.g.point.y = p.y + b*(C2.p.x - C1.p.x);
	G2.type = M_POINT;
	G2.g.point.x = p.x + b*(C2.p.y - C1.p.y);
	G2.g.point.y = p.y - b*(C2.p.x - C1.p.x);

	M_GeomSetAdd2(&Sint, &G1);
	if (M_VecDistance2(G1.g.point, G2.g.point) > M_MACHEP) {
		M_GeomSetAdd2(&Sint, &G2);
	}
	return (Sint);
}

/* Compute the intersection of a circle and a line. */
M_GeomSet2
M_IntersectCircleLine2(M_Circle2 C, M_Line2 L)
{
	M_GeomSet2 Sint = M_GEOM_SET_EMPTY;
	M_Vector2 p1 = M_LineInitPt2(L);
	M_Vector2 p2 = M_LineTermPt2(L);
	M_Vector2 p3 = C.p;
	M_Real a, b, c, det;
	M_Geom2 G;

	a = (p2.x - p1.x)*(p2.x - p1.x) + (p2.y - p1.y)*(p2.y - p1.y);
	b = 2.0*( (p2.x - p1.x)*(p1.x - p3.x) + (p2.y - p1.y)*(p1.y - p3.y) );
	c = p3.x*p3.x + p3.y*p3.y + p1.x*p1.x + p1.y*p1.y -
	    2.0*(p3.x*p1.x + p3.y*p1.y) - C.r*C.r;
	det = b*b - 4.0*a*c;

	if (det < 0.0) {
		return (Sint);
	} else if (det == 0.0) {
		/* TODO Tangent! */
		return (Sint);
	} else {
		M_Real e = Sqrt(det);
		M_Real u1 = (-b + e) / (2.0*a);
		M_Real u2 = (-b - e) / (2.0*a);

		if ((u1 < 0.0 || u1 > 1.0) &&
		    (u2 < 0.0 || u2 > 1.0)) {
			if ((u1 < 0.0 && u2 < 0.0) ||
			    (u1 > 1.0 && u2 > 1.0)) {
				return (Sint);
			} else {
				if (u1 >= 0.0 && u1 <= 1.0) {
					G.type = M_POINT;
					G.g.point = M_VecLERP2(p1,p2,u1);
					M_GeomSetAdd2(&Sint, &G);
				}
				if (u2 >= 0.0 && u2 <= 1.0) {
					G.type = M_POINT;
					G.g.point = M_VecLERP2(p1,p2,u2);
					M_GeomSetAdd2(&Sint, &G);
				}
			}
		} else {
			if (u1 >= 0.0 && u1 <= 1.0) {
				G.type = M_POINT;
				G.g.point = M_VecLERP2(p1,p2,u1);
				M_GeomSetAdd2(&Sint, &G);
			}
			if (u2 >= 0.0 && u2 <= 1.0) {
				G.type = M_POINT;
				G.g.point = M_VecLERP2(p1,p2,u2);
				M_GeomSetAdd2(&Sint, &G);
			}
		}
	}
	return (Sint);
}
