/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Miscellaneous utility routines for vectors.
 */

#include <core/core.h>
#include "m.h"

const M_VectorOps *mVecOps = NULL;
const M_VectorOps2 *mVecOps2 = NULL;
const M_VectorOps3 *mVecOps3 = NULL;
const M_VectorOps4 *mVecOps4 = NULL;

void
M_VectorInitEngine(void)
{
	mVecOps =  &mVecOps_FPU;
	mVecOps2 = &mVecOps2_FPU;
	mVecOps3 = &mVecOps3_FPU;
	mVecOps4 = &mVecOps4_FPU;

	if (HasSSE()) {
#ifdef HAVE_SSE
		mVecOps3 = &mVecOps3_SSE;
#elif defined(AG_DEBUG)
		AG_Verbose("SSE available, but disabled at compile time\n");
#endif
	}
	if (HasSSE3()) {
#ifdef HAVE_SSE3
		mVecOps3 = &mVecOps3_SSE3;
#elif defined(AG_DEBUG)
		AG_Verbose("SSE3 available, but disabled at compile time\n");
#endif
	}

#ifdef AG_DEBUG
	AG_Verbose("Vector operations: ");
#if defined(INLINE_ALTIVEC)
	AG_Verbose("altivec (inline)\n");
#elif defined(INLINE_SSE3)
	AG_Verbose("sse3 (inline)\n");
#elif defined(INLINE_SSE2)
	AG_Verbose("sse2 (inline)\n");
#elif defined(INLINE_SSE)
	AG_Verbose("sse (inline)\n");
#else
	AG_Verbose("%s\n", mVecOps3->name);
#endif
#endif /* AG_DEBUG */
}

M_Vector2
M_RealvToVector2(const M_Real *r)
{
	M_Vector2 v;

	v.x = r[0];
	v.y = r[1];
	return (v);
}

M_Vector3
M_RealvToVector3(const M_Real *r)
{
	M_Vector3 v;

	v.x = r[0];
	v.y = r[1];
	v.z = r[2];
	return (v);
}

M_Vector4
M_RealvToVector4(const M_Real *r)
{
	M_Vector4 v;

	v.x = r[0];
	v.y = r[1];
	v.z = r[2];
	v.w = r[3];
	return (v);
}

M_Vector2
M_Vector3to2(M_Vector3 v)
{
	M_Vector2 v2;
	v2.x = v.x;
	v2.y = v.y;
	return (v2);
}

M_Vector3
M_Vector2to3(M_Vector2 v)
{
	M_Vector3 v3;
	v3.x = v.x;
	v3.y = v.y;
	v3.z = 0.0;
	return (v3);
}

M_Vector4
M_Vector3to4(M_Vector3 v)
{
	M_Vector4 v4;
	v4.x = v.x;
	v4.y = v.y;
	v4.z = v.z;
	v4.w = 0.0;
	return (v4);
}

M_Vector2
M_ReadVector2(AG_DataSource *buf)
{
	M_Vector2 v;

	v.x = (M_Real)AG_ReadDouble(buf);
	v.y = (M_Real)AG_ReadDouble(buf);
	return (v);
}

M_Vector3
M_ReadVector3(AG_DataSource *buf)
{
	M_Vector3 v;

	v.x = (M_Real)AG_ReadDouble(buf);
	v.y = (M_Real)AG_ReadDouble(buf);
	v.z = (M_Real)AG_ReadDouble(buf);
	return (v);
}

M_Vector4
M_ReadVector4(AG_DataSource *buf)
{
	M_Vector4 v;

	v.x = (M_Real)AG_ReadDouble(buf);
	v.y = (M_Real)AG_ReadDouble(buf);
	v.z = (M_Real)AG_ReadDouble(buf);
	v.w = (M_Real)AG_ReadDouble(buf);
	return (v);
}

void
M_ReadVector2v(AG_DataSource *buf, M_Vector2 *v)
{
	v->x = (M_Real)AG_ReadDouble(buf);
	v->y = (M_Real)AG_ReadDouble(buf);
}

void
M_ReadVector3v(AG_DataSource *buf, M_Vector3 *v)
{
	v->x = (M_Real)AG_ReadDouble(buf);
	v->y = (M_Real)AG_ReadDouble(buf);
	v->z = (M_Real)AG_ReadDouble(buf);
}

void
M_ReadVector4v(AG_DataSource *buf, M_Vector4 *v)
{
	v->x = (M_Real)AG_ReadDouble(buf);
	v->y = (M_Real)AG_ReadDouble(buf);
	v->z = (M_Real)AG_ReadDouble(buf);
	v->w = (M_Real)AG_ReadDouble(buf);
}

void
M_WriteVector2(AG_DataSource *buf, M_Vector2 *v)
{
	AG_WriteDouble(buf, (double)v->x);
	AG_WriteDouble(buf, (double)v->y);
}

void
M_WriteVector3(AG_DataSource *buf, M_Vector3 *v)
{
	AG_WriteDouble(buf, (double)v->x);
	AG_WriteDouble(buf, (double)v->y);
	AG_WriteDouble(buf, (double)v->z);
}

void
M_WriteVector4(AG_DataSource *buf, M_Vector4 *v)
{
	AG_WriteDouble(buf, (double)v->x);
	AG_WriteDouble(buf, (double)v->y);
	AG_WriteDouble(buf, (double)v->z);
	AG_WriteDouble(buf, (double)v->w);
}

M_Vector2 *
M_VectorDup2(const M_Vector2 *v)
{
	M_Vector2 *vDup;

	vDup = Malloc(sizeof(M_Vector2));
	vDup->x = v->x;
	vDup->y = v->y;
	return (vDup);
}

M_Vector3 *
M_VectorDup3(const M_Vector3 *v)
{
	M_Vector3 *vDup;

	vDup = Malloc(sizeof(M_Vector3));
	vDup->x = v->x;
	vDup->y = v->y;
	vDup->z = v->z;
	return (vDup);
}

M_Vector4 *
M_VectorDup4(const M_Vector4 *v)
{
	M_Vector4 *vDup;

	vDup = Malloc(sizeof(M_Vector4));
	vDup->x = v->x;
	vDup->y = v->y;
	vDup->z = v->z;
	vDup->w = v->w;
	return (vDup);
}

