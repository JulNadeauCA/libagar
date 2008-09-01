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
 * Triangle routines.
 */

#include <core/core.h>
#include "m.h"

M_Triangle2
M_TriangleRead2(AG_DataSource *ds)
{
	M_Triangle2 T;

	T.a = M_LineRead2(ds);
	T.b = M_LineRead2(ds);
	T.c = M_LineRead2(ds);
	return (T);
}

M_Triangle3
M_TriangleRead3(AG_DataSource *ds)
{
	M_Triangle3 T;

	T.a = M_LineRead3(ds);
	T.b = M_LineRead3(ds);
	T.c = M_LineRead3(ds);
	return (T);
}

void
M_TriangleWrite2(AG_DataSource *ds, M_Triangle2 *T)
{
	M_LineWrite2(ds, &T->a);
	M_LineWrite2(ds, &T->b);
	M_LineWrite2(ds, &T->c);
}

void
M_TriangleWrite3(AG_DataSource *ds, M_Triangle3 *T)
{
	M_LineWrite3(ds, &T->a);
	M_LineWrite3(ds, &T->b);
	M_LineWrite3(ds, &T->c);
}

M_Triangle2
M_TriangleFromPts2(M_Vector2 a, M_Vector2 b, M_Vector2 c)
{
	M_Triangle2 T;

	T.a = M_LineFromPts2(a, b);
	T.b = M_LineFromPts2(b, c);
	T.c = M_LineFromPts2(c, a);
	return (T);
}

M_Triangle3
M_TriangleFromPts3(M_Vector3 a, M_Vector3 b, M_Vector3 c)
{
	M_Triangle3 T;

	T.a = M_LineFromPts3(a, b);
	T.b = M_LineFromPts3(b, c);
	T.c = M_LineFromPts3(c, a);
	return (T);
}

M_Triangle2
M_TriangleFromLines2(M_Line2 a, M_Line2 b, M_Line2 c)
{
	M_Triangle2 T;

	T.a = a;
	T.b = b;
	T.c = c;
	return (T);
}

M_Triangle3
M_TriangleFromLines3(M_Line3 a, M_Line3 b, M_Line3 c)
{
	M_Triangle3 T;

	T.a = a;
	T.b = b;
	T.c = c;
	return (T);
}

/*
 * Test whether the given point lies inside the polygon using barycentric
 * coordinates. From http://blackpawn.com/texts/pointinpoly/.
 */
int
M_PointInTriangle2(M_Triangle2 T, M_Vector2 p)
{
	M_Vector2 ca, ba, pa;
	M_Real dot00, dot01, dot02, dot11, dot12;
	M_Real d, u, v;

	ca = M_VecSub2(T.c.p, T.a.p);
	ba = M_VecSub2(T.b.p, T.a.p);
	pa = M_VecSub2(p,     T.a.p);
	dot00 = M_VecDot2p(&ca, &ca);
	dot01 = M_VecDot2p(&ca, &ba);
	dot02 = M_VecDot2p(&ca, &pa);
	dot11 = M_VecDot2p(&ba, &ba);
	dot12 = M_VecDot2p(&ba, &pa);
	d = 1.0 / (dot00*dot11 - dot01*dot01);
	u = (dot11*dot02 - dot01*dot12)*d;
	v = (dot00*dot12 - dot01*dot02)*d;
	return (u > 0) && (v > 0) && (u+v < 1);
}
