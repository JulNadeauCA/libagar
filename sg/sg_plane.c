/*
 * Copyright (c) 2006-2007 Hypertriton, Inc. 
 * <http://www.hypertriton.com/>
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

#include <agar/config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <agar/core/core.h>

#include "sg.h"

SG_Plane
SG_PlaneFrom3Pts(SG_Vector p1, SG_Vector p2, SG_Vector p3)
{
	SG_Plane P;
	SG_Vector n;

	n = SG_VectorNCross(SG_VectorSub(p1,p2), SG_VectorSub(p3,p2));
	P.a = n.x;
	P.b = n.y;
	P.c = n.z;
	P.d = -(P.a*p1.x + P.b*p1.y + P.c*p1.z);
	return (P);
}

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
SG_ReadPlane(AG_Netbuf *buf)
{
	SG_Plane P;

	P.a = SG_ReadReal(buf);
	P.b = SG_ReadReal(buf);
	P.c = SG_ReadReal(buf);
	P.d = SG_ReadReal(buf);
	return (P);
}

void
SG_WritePlane(AG_Netbuf *buf, SG_Plane *P)
{
	SG_WriteReal(buf, P->a);
	SG_WriteReal(buf, P->b);
	SG_WriteReal(buf, P->c);
	SG_WriteReal(buf, P->d);
}

int
SG_ValidPlane(SG_Plane P)
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

#endif /* HAVE_OPENGL */
