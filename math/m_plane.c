/*
 * Copyright (c) 2007-2011 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Plane in R^3.
 */

#include <core/core.h>
#include "m.h"

/* Create a plane given a normal vector and a distance. */
M_Plane
M_PlaneFromNormal(M_Vector3 n, M_Real d)
{
	M_Plane P;

	P.a = n.x;
	P.b = n.y;
	P.c = n.z;
	P.d = d;
	return (P);
}

/* Create a plane given three points in R^3. */
M_Plane
M_PlaneFromPts(M_Vector3 p1, M_Vector3 p2, M_Vector3 p3)
{
	M_Plane P;
	M_Vector3 n;

	n = M_VecNormCross3(M_VecSub3(p1,p2), M_VecSub3(p3,p2));
	P.a = n.x;
	P.b = n.y;
	P.c = n.z;
	P.d = -(P.a*p1.x + P.b*p1.y + P.c*p1.z);
	return (P);
}

/* Create a plane at distance d from another plane. */
M_Plane
M_PlaneAtDistance(M_Plane P1, M_Real d)
{
	M_Plane P2;

	P2.a = P1.a;
	P2.b = P1.b;
	P2.c = P1.c;
	P2.d = P1.d + d;
	return (P2);
}

M_Plane
M_PlaneRead(AG_DataSource *buf)
{
	M_Plane P;

	P.a = M_ReadReal(buf);
	P.b = M_ReadReal(buf);
	P.c = M_ReadReal(buf);
	P.d = M_ReadReal(buf);
	return (P);
}

void
M_PlaneWrite(AG_DataSource *buf, M_Plane *P)
{
	M_WriteReal(buf, P->a);
	M_WriteReal(buf, P->b);
	M_WriteReal(buf, P->c);
	M_WriteReal(buf, P->d);
}

int
M_PlaneIsValid(M_Plane P)
{
	return (P.a != 0.0 && P.b != 0.0 && P.c != 0.0);
}

M_Real
M_PlaneVectorAngle(M_Plane P, M_Vector3 v)
{
	return (M_PI - Acos(M_VecDot3(M_PlaneNormp(&P),M_VecNorm3p(&v))));
}
