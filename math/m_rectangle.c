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
 * Rectangle routines.
 */

#include <core/core.h>
#include "m.h"

M_Rectangle2
M_RectangleRead2(AG_DataSource *ds)
{
	M_Rectangle2 R;

	R.a = M_LineRead2(ds);
	R.b = M_LineRead2(ds);
	R.c = M_LineRead2(ds);
	R.d = M_LineRead2(ds);
	return (R);
}

M_Rectangle3
M_RectangleRead3(AG_DataSource *ds)
{
	M_Rectangle3 R;

	R.a = M_LineRead3(ds);
	R.b = M_LineRead3(ds);
	R.c = M_LineRead3(ds);
	R.d = M_LineRead3(ds);
	return (R);
}

void
M_RectangleWrite2(AG_DataSource *ds, M_Rectangle2 *R)
{
	M_LineWrite2(ds, &R->a);
	M_LineWrite2(ds, &R->b);
	M_LineWrite2(ds, &R->c);
	M_LineWrite2(ds, &R->d);
}

void
M_RectangleWrite3(AG_DataSource *ds, M_Rectangle3 *R)
{
	M_LineWrite3(ds, &R->a);
	M_LineWrite3(ds, &R->b);
	M_LineWrite3(ds, &R->c);
	M_LineWrite3(ds, &R->d);
}

M_Rectangle2
M_RectangleFromPts2(M_Vector2 a, M_Vector2 c)
{
	M_Rectangle2 R;
	M_Vector2 b, d;

	b.x = c.x;
	b.y = a.y;
	d.x = a.x;
	d.y = c.y;
	R.a = M_LineFromPts2(a, b);
	R.b = M_LineFromPts2(b, c);
	R.c = M_LineFromPts2(c, d);
	R.d = M_LineFromPts2(d, a);
	return (R);
}

M_Rectangle3
M_RectangleFromPts3(M_Vector3 a, M_Vector3 b, M_Vector3 c)
{
	M_Rectangle3 R;
	M_Vector3 d;
	M_Plane3 P;
	
	P = M_PlaneFromPts3(a, b, c);
	d.x = a.x;
	d.y = c.y;
	d.z = -(P.a*d.x + P.b*d.y + P.d)/P.d;

	R.a = M_LineFromPts3(a, b);
	R.b = M_LineFromPts3(b, c);
	R.c = M_LineFromPts3(c, d);
	R.d = M_LineFromPts3(d, a);
	return (R);
}

int
M_PointInRectangle2(M_Rectangle2 R, M_Vector2 p)
{
	return (p.x >= R.a.p.x && p.y >= R.a.p.y &&
	        p.x <= R.c.p.x && p.y <= R.c.p.y);
}
