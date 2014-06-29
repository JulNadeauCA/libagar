/*
 * Copyright (c) 2008 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Conversion between different coordinate systems.
 */

#include <agar/core/core.h>
#include <agar/math/m.h>

M_Rectangular
M_RectangularFromSpherical(M_Spherical s)
{
	M_Rectangular r;

	r.x = s.r*Cos(s.phi)*Cos(s.theta);
	r.y = s.r*Cos(s.phi)*Sin(s.theta);
	r.z = s.r*Sin(s.phi);
	return (r);
}

M_Rectangular
M_RectangularFromCylindrical(M_Cylindrical c)
{
	M_Rectangular r;

	r.x = c.rho*Cos(c.phi);
	r.y = c.rho*Sin(c.phi);
	r.z = c.z;
	return (r);
}

M_Spherical
M_SphericalFromRectangular(M_Rectangular r)
{
	M_Spherical s;
	M_Real xy2 = r.x*r.x + r.y*r.y;

	s.theta = Atan2(r.y, r.x);
	s.phi = Atan2(Sqrt(xy2), r.z);
	s.r = Sqrt(xy2 + r.z*r.z);
	return (s);
}

M_Spherical
M_SphericalFromCylindrical(M_Cylindrical c)
{
	M_Spherical s;

	s.theta = Atan2(c.rho, c.z);
	s.phi = c.phi;
	s.r = Sqrt(c.rho*c.rho + c.z*c.z);
	return (s);
}

M_Cylindrical
M_CylindricalFromRectangular(M_Rectangular r)
{
	M_Cylindrical c;

	c.rho = Sqrt(r.x*r.x + r.y*r.y);
	c.phi = Atan2(r.y, r.x);
	c.z = r.z;
	return (c);
}

M_Cylindrical
M_CylindricalFromSpherical(M_Spherical s)
{
	M_Cylindrical c;

	c.rho = s.r*Sin(s.theta);
	c.phi = s.phi;
	c.z = s.r*Cos(s.theta);
	return (c);
}
