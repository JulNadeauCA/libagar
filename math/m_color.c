/*
 * Copyright (c) 2008 Hypertriton, Inc. <http://hypertriton.com/>
 * All rights reserved.
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
 * Color-related operations and conversions between different color spaces.
 */

#include <agar/core/core.h>
#include <agar/math/m.h>

M_Color
M_ColorHSVA(M_Real h, M_Real s, M_Real v, M_Real a)
{
	M_Color C;
	M_Real var[3], hv;
	int iv;

	if (M_MACHZERO(s)) {	/* Gray */
		C.r = v;
		C.g = v;
		C.b = v;
		C.a = a;
		return (C);
	}
	
	hv = h/60.0;
	iv = (int)Floor(hv);
	var[0] = v*(1.0F - s);
	var[1] = v*(1.0F - s*(hv-iv));
	var[2] = v*(1.0F - s*(1.0-hv-iv));

	switch (iv) {
	case 0:		C.r = v;	C.g = var[2];	C.b = var[0];	break;
	case 1:		C.r = var[1];	C.g = v;	C.b = var[0];	break;
	case 2:		C.r = var[0];	C.g = v;	C.b = var[2];	break;
	case 3:		C.r = var[0];	C.g = var[1];	C.b = v;	break;
	case 4:		C.r = var[2];	C.g = var[0];	C.b = v;	break;
	default:	C.r = v;	C.g = var[0];	C.b = var[1];	break;
	}

	C.a = a;
	return (C);
}

M_Color
M_ReadColor(AG_DataSource *buf)
{
	M_Color C;

	AG_ReadUint8(buf);				/* Expn: type */
	C.r = M_ReadReal(buf);
	C.g = M_ReadReal(buf);
	C.b = M_ReadReal(buf);
	C.a = M_ReadReal(buf);
	return (C);
}

void
M_WriteColor(AG_DataSource *buf, const M_Color *C)
{
	AG_WriteUint8(buf, 0);				/* Expn: type */
	M_WriteReal(buf, C->r);
	M_WriteReal(buf, C->g);
	M_WriteReal(buf, C->b);
	M_WriteReal(buf, C->a);
}

void
M_ColorTo4fv(const M_Color *C, float *v)
{
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
	memcpy(v, C, 4*sizeof(float));
#else
	v[0] = (float)C->r;
	v[1] = (float)C->g;
	v[2] = (float)C->b;
	v[3] = (float)C->a;
#endif
}

void
M_ColorTo4dv(const M_Color *C, double *v)
{
#if defined(DOUBLE_PRECISION) && !defined(HAVE_SSE)
	memcpy(v, C, 4*sizeof(double));
#else
	v[0] = (double)C->r;
	v[1] = (double)C->g;
	v[2] = (double)C->b;
	v[3] = (double)C->a;
#endif
}
