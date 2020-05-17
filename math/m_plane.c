/*
 * Copyright (c) 2007-2013 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/core/core.h>
#include <agar/math/m.h>

/* Return a plane given three points in R^3. */
M_Plane
M_PlaneFromPts(M_Vector3 p1, M_Vector3 p2, M_Vector3 p3)
{
	M_Plane P;

	P.n = M_VecNormCross3(M_VecSub3(p1,p2), M_VecSub3(p3,p2));
	P.d = -(P.n.x*p1.x + P.n.y*p1.y + P.n.z*p1.z);
	memset(&P._pad, 0, sizeof(P._pad));
	return (P);
}

M_Plane
M_PlaneRead(AG_DataSource *ds)
{
	M_Plane P;

	P.n = M_ReadVector3(ds);
	P.d = M_ReadReal(ds);
	return (P);
}

void
M_PlaneWrite(AG_DataSource *ds, M_Plane *P)
{
	M_WriteVector3(ds, &P->n);
	M_WriteReal(ds, P->d);
}

M_Real
M_PlaneVectorAngle(M_Plane P, M_Vector3 v)
{
	return (M_PI - Acos(M_VecDot3(P.n, M_VecNorm3(v))));
}
