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
 * Routines related to planes in R3.
 */

#include <core/core.h>
#include "m.h"

/* Create a plane in R3 from a normal and distance from origin. */
M_Plane3
M_PlaneFromNormal3(M_Vector3 n, M_Real d)
{
	M_Plane3 P;

	P.a = n.x;
	P.b = n.y;
	P.c = n.z;
	P.d = d;
	return (P);
}

/* Create a plane in R3 from three points in R3. */
M_Plane3
M_PlaneFromPts3(M_Vector3 p1, M_Vector3 p2, M_Vector3 p3)
{
	M_Plane3 P;
	M_Vector3 n;

	n = M_VecNormCross3(M_VecSub3(p1,p2), M_VecSub3(p3,p2));
	P.a = n.x;
	P.b = n.y;
	P.c = n.z;
	P.d = -(P.a*p1.x + P.b*p1.y + P.c*p1.z);
	return (P);
}

/* Create a plane in R3 at distance d from another plane. */
M_Plane3
M_PlaneAtDistance3(M_Plane3 P1, M_Real d)
{
	M_Plane3 P2;

	P2.a = P1.a;
	P2.b = P1.b;
	P2.c = P1.c;
	P2.d = P1.d + d;
	return (P2);
}

M_Plane3
M_PlaneRead3(AG_DataSource *buf)
{
	M_Plane3 P;

	P.a = M_ReadReal(buf);
	P.b = M_ReadReal(buf);
	P.c = M_ReadReal(buf);
	P.d = M_ReadReal(buf);
	return (P);
}

void
M_PlaneWrite3(AG_DataSource *buf, M_Plane3 *P)
{
	M_WriteReal(buf, P->a);
	M_WriteReal(buf, P->b);
	M_WriteReal(buf, P->c);
	M_WriteReal(buf, P->d);
}

int
M_PlaneIsValid3(M_Plane3 P)
{
	return (P.a != 0.0 && P.b != 0.0 && P.c != 0.0);
}

M_Vector3
M_PlaneNorm3(M_Plane3 P)
{
	M_Vector3 n;

	n.x = P.a;
	n.y = P.b;
	n.z = P.c;
	return (n);
}

M_Vector3
M_PlaneNorm3p(const M_Plane3 *P)
{
	M_Vector3 n;

	n.x = P->a;
	n.y = P->b;
	n.z = P->c;
	return (n);
}

M_Real
M_PlaneVectorAngle3(M_Plane3 P, M_Vector3 v)
{
	return (M_PI - Acos(M_VecDot3(M_PlaneNorm3p(&P),M_VecNorm3p(&v))));
}
