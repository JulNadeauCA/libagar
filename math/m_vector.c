/*
 * Copyright (c) 2005-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>
#include <agar/math/m.h>

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
#ifdef HAVE_SSE
	if (agCPU.ext & AG_EXT_SSE)
		mVecOps3 = &mVecOps3_SSE;
# ifdef INLINE_SSE
	else {
		AG_FatalError("Compiled for SSE, but no SSE support in CPU! "
		              "(must recompile Agar without --with-sse=inline)");
	}
# endif
# ifdef HAVE_SSE2
	if (!(agCPU.ext & AG_EXT_SSE2)) {
#  ifdef INLINE_SSE
		AG_FatalError("Compiled for SSE2, but CPU only supports SSE1! "
			      "(recompile Agar with: --without-{sse2,sse3})");
#  else
		AG_Verbose("Compiled for SSE2, but CPU only supports SSE1! "
			   "(recompile Agar with: --without-{sse2,sse3})\n");
		mVecOps3 = &mVecOps3_FPU;
#  endif
	}
# endif
# ifdef HAVE_SSE3
	if (!(agCPU.ext & AG_EXT_SSE3)) {
#  ifdef INLINE_SSE
		AG_FatalError("Compiled for SSE3, but CPU only supports SSE2! "
			      "(recompile Agar with: --without-sse3)");
#  else
		AG_Verbose("Compiled for SSE3, but CPU only supports SSE2! "
			   "(recompile Agar with: --without-sse3)\n");
		mVecOps3 = &mVecOps3_FPU;
#  endif
	}
# endif
#endif /* HAVE_SSE */
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

#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
	v.x = (float)r[0];
	v.y = (float)r[1];
	v.z = (float)r[2];
#else
	v.x = r[0];
	v.y = r[1];
	v.z = r[2];
#endif
	return (v);
}

M_Vector4
M_RealvToVector4(const M_Real *r)
{
	M_Vector4 v;

#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
	v.x = (float)r[0];
	v.y = (float)r[1];
	v.z = (float)r[2];
	v.w = (float)r[3];
#else
	v.x = r[0];
	v.y = r[1];
	v.z = r[2];
	v.w = r[3];
#endif
	return (v);
}

int
M_ReadVector2v(AG_DataSource *ds, M_Vector2 *v)
{
	Uint8 type;

	type = AG_ReadUint8(ds);
	switch (type) {
	case 21:
		v->x = (M_Real)AG_ReadFloat(ds);
		v->y = (M_Real)AG_ReadFloat(ds);
		break;
	case 22:
		v->x = (M_Real)AG_ReadDouble(ds);
		v->y = (M_Real)AG_ReadDouble(ds);
		break;
	default:
		AG_SetError("Bad vector2: %u", type);
		return (-1);
	}
	return (0);
}
int
M_ReadVector3v(AG_DataSource *ds, M_Vector3 *v)
{
	Uint8 type;

	type = AG_ReadUint8(ds);
	switch (type) {
	case 31:
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
		v->x = AG_ReadFloat(ds);
		v->y = AG_ReadFloat(ds);
		v->z = AG_ReadFloat(ds);
#else
		v->x = (M_Real)AG_ReadFloat(ds);
		v->y = (M_Real)AG_ReadFloat(ds);
		v->z = (M_Real)AG_ReadFloat(ds);
#endif
		break;
	case 32:
#ifdef HAVE_SSE
		v->x = (float)AG_ReadDouble(ds);
		v->y = (float)AG_ReadDouble(ds);
		v->z = (float)AG_ReadDouble(ds);
#else
		v->x = (M_Real)AG_ReadDouble(ds);
		v->y = (M_Real)AG_ReadDouble(ds);
		v->z = (M_Real)AG_ReadDouble(ds);
#endif
		break;
	default:
		AG_SetError("Bad vector3: %u", type);
		return (-1);
	}
	return (0);
}
int
M_ReadVector4v(AG_DataSource *ds, M_Vector4 *v)
{
	Uint8 type;

	type = AG_ReadUint8(ds);
	switch (type) {
	case 41:
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
		v->x = AG_ReadFloat(ds);
		v->y = AG_ReadFloat(ds);
		v->z = AG_ReadFloat(ds);
		v->w = AG_ReadFloat(ds);
#else
		v->x = (M_Real)AG_ReadFloat(ds);
		v->y = (M_Real)AG_ReadFloat(ds);
		v->z = (M_Real)AG_ReadFloat(ds);
		v->w = (M_Real)AG_ReadFloat(ds);
#endif
		break;
	case 42:
#ifdef HAVE_SSE
		v->x = (float)AG_ReadDouble(ds);
		v->y = (float)AG_ReadDouble(ds);
		v->z = (float)AG_ReadDouble(ds);
		v->w = (float)AG_ReadDouble(ds);
#else
		v->x = (M_Real)AG_ReadDouble(ds);
		v->y = (M_Real)AG_ReadDouble(ds);
		v->z = (M_Real)AG_ReadDouble(ds);
		v->w = (M_Real)AG_ReadDouble(ds);
#endif
		break;
	default:
		AG_SetError("Bad vector4: %u", type);
		return (-1);
	}
	return (0);
}

M_Vector2
M_ReadVector2(AG_DataSource *ds)
{
	M_Vector2 v;

	if (M_ReadVector2v(ds, &v) == -1) {
		AG_FatalError(NULL);
	}
	return (v);
}

M_Vector3
M_ReadVector3(AG_DataSource *ds)
{
	M_Vector3 v;

	if (M_ReadVector3v(ds, &v) == -1) {
		AG_FatalError(NULL);
	}
	return (v);
}

M_Vector4
M_ReadVector4(AG_DataSource *ds)
{
	M_Vector4 v;

	if (M_ReadVector4v(ds, &v) == -1) {
		AG_FatalError(NULL);
	}
	return (v);
}

void
M_WriteVector2(AG_DataSource *ds, const M_Vector2 *v)
{
#if defined(SINGLE_PRECISION)
	AG_WriteUint8(ds, 21);
	AG_WriteFloat(ds, v->x);
	AG_WriteFloat(ds, v->y);
#elif defined(DOUBLE_PRECISION)
	AG_WriteUint8(ds, 22);
	AG_WriteDouble(ds, v->x);
	AG_WriteDouble(ds, v->y);
#endif
}

void
M_WriteVector3(AG_DataSource *ds, const M_Vector3 *v)
{
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
	AG_WriteUint8(ds, 31);
	AG_WriteFloat(ds, v->x);
	AG_WriteFloat(ds, v->y);
	AG_WriteFloat(ds, v->z);
#elif defined(DOUBLE_PRECISION)
	AG_WriteUint8(ds, 32);
	AG_WriteDouble(ds, v->x);
	AG_WriteDouble(ds, v->y);
	AG_WriteDouble(ds, v->z);
#endif
}

void
M_WriteVector4(AG_DataSource *ds, const M_Vector4 *v)
{
#if defined(SINGLE_PRECISION) || defined(HAVE_SSE)
	AG_WriteUint8(ds, 41);
	AG_WriteFloat(ds, v->x);
	AG_WriteFloat(ds, v->y);
	AG_WriteFloat(ds, v->z);
	AG_WriteFloat(ds, v->w);
#elif defined(DOUBLE_PRECISION)
	AG_WriteUint8(ds, 42);
	AG_WriteDouble(ds, v->x);
	AG_WriteDouble(ds, v->y);
	AG_WriteDouble(ds, v->z);
	AG_WriteDouble(ds, v->w);
#endif
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
