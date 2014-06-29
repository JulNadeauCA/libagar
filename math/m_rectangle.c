/*
 * Copyright (c) 2008-2011 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/core/core.h>
#include <agar/math/m.h>

M_Rectangle2
M_RectangleRead2(AG_DataSource *ds)
{
	M_Rectangle2 R;

	R.a = M_ReadVector2(ds);
	R.b = M_ReadVector2(ds);
	R.c = M_ReadVector2(ds);
	R.d = M_ReadVector2(ds);
	return (R);
}

M_Rectangle3
M_RectangleRead3(AG_DataSource *ds)
{
	M_Rectangle3 R;

	R.a = M_ReadVector3(ds);
	R.b = M_ReadVector3(ds);
	R.c = M_ReadVector3(ds);
	R.d = M_ReadVector3(ds);
	return (R);
}

void
M_RectangleWrite2(AG_DataSource *ds, M_Rectangle2 *R)
{
	M_WriteVector2(ds, &R->a);
	M_WriteVector2(ds, &R->b);
	M_WriteVector2(ds, &R->c);
	M_WriteVector2(ds, &R->d);
}

void
M_RectangleWrite3(AG_DataSource *ds, M_Rectangle3 *R)
{
	M_WriteVector3(ds, &R->a);
	M_WriteVector3(ds, &R->b);
	M_WriteVector3(ds, &R->c);
	M_WriteVector3(ds, &R->d);
}

int
M_PointInRectangle2(M_Rectangle2 R, M_Vector2 p)
{
	return (p.x >= R.a.x && p.y >= R.a.y &&
	        p.x <= R.b.x && p.y <= R.b.y);
}
