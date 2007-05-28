/*
 * Copyright (c) 2005-2007 Hypertriton, Inc.
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

SG_Vector
SG_SphToCart(SG_Spherical s)
{
	SG_Vector v;

	v.x = s.r*SG_Cos(s.phi)*SG_Cos(s.theta);
	v.y = s.r*SG_Cos(s.phi)*SG_Sin(s.theta);
	v.z = s.r*SG_Sin(s.phi);
	return (v);
}

SG_Spherical
SG_CartToSph(SG_Vector v)
{
	SG_Spherical s;
	SG_Real xy2 = v.x*v.x + v.y*v.y;

	s.theta = SG_Atan2(v.y, v.x);
	s.phi = SG_Atan2(SG_Sqrt(xy2), v.z);
	s.r = SG_Sqrt(xy2 + v.z*v.z);
	return (s);
}

#endif /* HAVE_OPENGL */
