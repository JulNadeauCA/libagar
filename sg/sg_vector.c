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
 * Generic utility routines for vectors.
 */

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>
#include "sg.h"

const SG_VectorOps2 *sgVecOps2 = NULL;
const SG_VectorOps3 *sgVecOps3 = NULL;
const SG_VectorOps4 *sgVecOps4 = NULL;

void
SG_VectorInitEngine(void)
{
	sgVecOps2 = &sgVecOps2_FPU;
	sgVecOps3 = &sgVecOps3_FPU;
	sgVecOps4 = &sgVecOps4_FPU;
}

void
SG_VectorDestroyEngine(void)
{
}

SG_Vector2
SG_Vector3to2(SG_Vector v)
{
	SG_Vector2 v2;
	v2.x = v.x;
	v2.y = v.y;
	return (v2);
}

SG_Vector
SG_Vector2to3(SG_Vector2 v)
{
	SG_Vector v3;
	v3.x = v.x;
	v3.y = v.y;
	v3.z = 0.0;
	return (v3);
}

SG_Vector4
SG_Vector3to4(SG_Vector v)
{
	SG_Vector4 v4;
	v4.x = v.x;
	v4.y = v.y;
	v4.z = v.z;
	v4.w = 0.0;
	return (v4);
}

SG_Vector2
SG_ReadVector2(AG_Netbuf *buf)
{
	SG_Vector2 v;

	v.x = (SG_Real)AG_ReadDouble(buf);
	v.y = (SG_Real)AG_ReadDouble(buf);
	return (v);
}

SG_Vector
SG_ReadVector(AG_Netbuf *buf)
{
	SG_Vector v;

	v.x = (SG_Real)AG_ReadDouble(buf);
	v.y = (SG_Real)AG_ReadDouble(buf);
	v.z = (SG_Real)AG_ReadDouble(buf);
	return (v);
}

SG_Vector4
SG_ReadVector4(AG_Netbuf *buf)
{
	SG_Vector4 v;

	v.x = (SG_Real)AG_ReadDouble(buf);
	v.y = (SG_Real)AG_ReadDouble(buf);
	v.z = (SG_Real)AG_ReadDouble(buf);
	v.w = (SG_Real)AG_ReadDouble(buf);
	return (v);
}

void
SG_ReadVector2v(AG_Netbuf *buf, SG_Vector2 *v)
{
	v->x = (SG_Real)AG_ReadDouble(buf);
	v->y = (SG_Real)AG_ReadDouble(buf);
}

void
SG_ReadVectorv(AG_Netbuf *buf, SG_Vector *v)
{
	v->x = (SG_Real)AG_ReadDouble(buf);
	v->y = (SG_Real)AG_ReadDouble(buf);
	v->z = (SG_Real)AG_ReadDouble(buf);
}

void
SG_ReadVector4v(AG_Netbuf *buf, SG_Vector4 *v)
{
	v->x = (SG_Real)AG_ReadDouble(buf);
	v->y = (SG_Real)AG_ReadDouble(buf);
	v->z = (SG_Real)AG_ReadDouble(buf);
	v->w = (SG_Real)AG_ReadDouble(buf);
}

void
SG_WriteVector2(AG_Netbuf *buf, SG_Vector2 *v)
{
	AG_WriteDouble(buf, (double)v->x);
	AG_WriteDouble(buf, (double)v->y);
}

void
SG_WriteVector(AG_Netbuf *buf, SG_Vector *v)
{
	AG_WriteDouble(buf, (double)v->x);
	AG_WriteDouble(buf, (double)v->y);
	AG_WriteDouble(buf, (double)v->z);
}

void
SG_WriteVector4(AG_Netbuf *buf, SG_Vector4 *v)
{
	AG_WriteDouble(buf, (double)v->x);
	AG_WriteDouble(buf, (double)v->y);
	AG_WriteDouble(buf, (double)v->z);
	AG_WriteDouble(buf, (double)v->w);
}

SG_Vector2
SG_ReadVectorf2(AG_Netbuf *buf)
{
	SG_Vector2 v;

	v.x = (SG_Real)AG_ReadFloat(buf);
	v.y = (SG_Real)AG_ReadFloat(buf);
	return (v);
}

SG_Vector
SG_ReadVectorf(AG_Netbuf *buf)
{
	SG_Vector v;

	v.x = (SG_Real)AG_ReadFloat(buf);
	v.y = (SG_Real)AG_ReadFloat(buf);
	v.z = (SG_Real)AG_ReadFloat(buf);
	return (v);
}

SG_Vector4
SG_ReadVectorf4(AG_Netbuf *buf)
{
	SG_Vector4 v;

	v.x = (SG_Real)AG_ReadFloat(buf);
	v.y = (SG_Real)AG_ReadFloat(buf);
	v.z = (SG_Real)AG_ReadFloat(buf);
	v.w = (SG_Real)AG_ReadFloat(buf);
	return (v);
}

void
SG_ReadVectorf2v(AG_Netbuf *buf, SG_Vector2 *v)
{
	v->x = (SG_Real)AG_ReadFloat(buf);
	v->y = (SG_Real)AG_ReadFloat(buf);
}

void
SG_ReadVectorfv(AG_Netbuf *buf, SG_Vector *v)
{
	v->x = (SG_Real)AG_ReadFloat(buf);
	v->y = (SG_Real)AG_ReadFloat(buf);
	v->z = (SG_Real)AG_ReadFloat(buf);
}

void
SG_ReadVectorf4v(AG_Netbuf *buf, SG_Vector4 *v)
{
	v->x = (SG_Real)AG_ReadFloat(buf);
	v->y = (SG_Real)AG_ReadFloat(buf);
	v->z = (SG_Real)AG_ReadFloat(buf);
	v->w = (SG_Real)AG_ReadFloat(buf);
}


void
SG_WriteVectorf2(AG_Netbuf *buf, SG_Vector2 *v)
{
	AG_WriteFloat(buf, (float)v->x);
	AG_WriteFloat(buf, (float)v->y);
}

void
SG_WriteVectorf(AG_Netbuf *buf, SG_Vector *v)
{
	AG_WriteFloat(buf, (float)v->x);
	AG_WriteFloat(buf, (float)v->y);
	AG_WriteFloat(buf, (float)v->z);
}

void
SG_WriteVectorf4(AG_Netbuf *buf, SG_Vector4 *v)
{
	AG_WriteFloat(buf, (float)v->x);
	AG_WriteFloat(buf, (float)v->y);
	AG_WriteFloat(buf, (float)v->z);
	AG_WriteFloat(buf, (float)v->w);
}

#endif /* HAVE_OPENGL */
