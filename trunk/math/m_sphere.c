/*
 * Copyright (c) 2009 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Routines related to spheres.
 */

#include <agar/core/core.h>
#include <agar/math/m.h>

M_Sphere
M_SphereRead(AG_DataSource *ds)
{
	M_Sphere S;

	S.p = M_ReadVector3(ds);
	S.r = M_ReadReal(ds);
	return (S);
}

void
M_SphereWrite(AG_DataSource *ds, M_Sphere *S)
{
	M_WriteVector3(ds, &S->p);
	M_WriteReal(ds, S->r);
}

/* Create a sphere from a point and radius. */
M_Sphere
M_SphereFromPt(M_Vector3 p, M_Real r)
{
	M_Sphere S;

	S.p = p;
	S.r = r;
	return (S);
}

/* Compute minimal distance from a sphere to a point p. */
M_Real
M_SpherePointDistance(M_Sphere S, M_Vector3 p)
{
	return M_VecDistance3(p,S.p) - S.r;
}

/* Compute the surface area of a sphere. */
M_Real
M_SphereSurfaceArea(M_Sphere S)
{
	return 4.0*M_PI*(S.r*S.r);
}

/* Compute the volume of a sphere. */
M_Real
M_SphereVolume(M_Sphere S)
{
	return (4.0/3.0)*M_PI*(S.r*S.r*S.r);
}
