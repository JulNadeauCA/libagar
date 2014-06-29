/*
 * Copyright (c) 2005-2012 Hypertriton, Inc. <http://hypertriton.com/>
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
 * Utility and I/O routines for math library.
 */

#include <agar/core/core.h>

#include <agar/config/enable_gui.h>
#include <agar/config/have_long_double.h>

#include <agar/math/m.h>

#ifdef ENABLE_GUI
# include <agar/gui/gui.h>
# include <agar/gui/label.h>
# include <agar/gui/units.h>
# include <agar/math/m_plotter.h>
# include <agar/math/m_matview.h>
#endif

int mInitedSubsystem = 0;

/*
 * Math library extensions to AG_Printf(3) and AG_PrintfP(3).
 */
static size_t
PrintReal(AG_FmtString *fs, char *dst, size_t dstSize)
{
	M_Real *r = AG_FMTSTRING_ARG(fs);

#if defined(QUAD_PRECISION)
	return Snprintf(dst, dstSize, "%.03llf", *r);
#else
	return Snprintf(dst, dstSize, "%.03f", *r);
#endif
}
static size_t
PrintTime(AG_FmtString *fs, char *dst, size_t dstSize)
{
	M_Time *t = AG_FMTSTRING_ARG(fs);
	return AG_UnitFormat((double)(*t), agTimeUnits, dst, dstSize);
}
static size_t
PrintComplex(AG_FmtString *fs, char *dst, size_t dstSize)
{
	M_Complex *c = AG_FMTSTRING_ARG(fs);
	return Snprintf(dst, dstSize, "[%.03f%+.03fi]", c->r, c->i);
}
static size_t
PrintVector2(AG_FmtString *fs, char *dst, size_t dstSize)
{
	M_Vector2 *v = AG_FMTSTRING_ARG(fs);
	return Snprintf(dst, dstSize, "[%.03f, %.03f]",
	    v->x, v->y);
}
static size_t
PrintVector3(AG_FmtString *fs, char *dst, size_t dstSize)
{
	M_Vector3 *v = AG_FMTSTRING_ARG(fs);
	return Snprintf(dst, dstSize, "[%.03f, %.03f, %.03f]",
	    v->x, v->y, v->z);
}
static size_t
PrintVector4(AG_FmtString *fs, char *dst, size_t dstSize)
{
	M_Vector4 *v = AG_FMTSTRING_ARG(fs);
	return Snprintf(dst, dstSize, "[%.03f, %.03f, %.03f, %.03f]",
	    v->x, v->y, v->z, v->w);
}
static size_t
PrintVector(AG_FmtString *fs, char *dst, size_t dstSize)
{
	M_Vector *v = AG_FMTSTRING_ARG(fs);
	char *pDst, *pEnd = &dst[dstSize-1];
	size_t rv;
	Uint i;

	if (dstSize < 3) {	/* "[]" + NUL */
		return (3);
	}
	dst[0] = '[';
	dst[1] = '\0';
	pDst = &dst[1];
	for (i = 0; i < v->m; i++) {
		M_Real *e = M_VecGetElement(v,i);

		rv = Snprintf(pDst, (pEnd-pDst), "%.03f", *e);
		if ((pDst += rv) > pEnd) { *pEnd = '\0'; goto out; }
		if (i < (v->m - 1)) {
			if (pEnd-pDst < 3) { *pDst = '\0'; goto out; }
			pDst[0] = ',';
			pDst[1] = ' ';
			pDst[2] = '\0';
			pDst+=2;
		}
	}
	if (pEnd-pDst < 2) { *pDst = '\0'; goto out; }
	pDst[0] = ']';
	pDst[1] = '\0';
	pDst++;
out:
	return (pDst - dst);
}
static size_t
PrintMatrix(AG_FmtString *fs, char *dst, size_t dstSize)
{
	M_Matrix *M = AG_FMTSTRING_ARG(fs);
	char *pDst, *pEnd = &dst[dstSize-1];
	size_t rv;
	Uint i, j;

	if (dstSize < 3) {	/* "[]" + NUL */
		return (3);
	}
	dst[0] = '[';
	dst[1] = '\0';
	pDst = &dst[1];
	for (i = 0; i < M->m; i++) {
		for (j = 0; j < M->n; j++) {
			M_Real *e = M_GetElement(M,i,j);

			rv = Snprintf(pDst, (pEnd-pDst), "%.02f", *e);
			if ((pDst += rv) > pEnd) { *pEnd = '\0'; goto out; }

			if (j < (M->n - 1)) {
				if (pEnd-pDst < 3) { *pDst = '\0'; goto out; }
				pDst[0] = ',';
				pDst[1] = ' ';
				pDst[2] = '\0';
				pDst+=2;
			}
		}
		if (i < (M->m - 1)) {
			if (pEnd-pDst < 3) { *pDst = '\0'; goto out; }
			pDst[0] = ';';
			pDst[1] = ' ';
			pDst[2] = '\0';
			pDst+=2;
		}
	}
	if (pEnd-pDst < 2) { *pDst = '\0'; goto out; }
	pDst[0] = ']';
	pDst[1] = '\0';
	pDst++;
out:
	return (pDst - dst);
}
static size_t
PrintMatrix44(AG_FmtString *fs, char *dst, size_t dstSize)
{
	M_Matrix44 *M = AG_FMTSTRING_ARG(fs);
	return Snprintf(dst, dstSize,
	    "[%.02f, %.02f, %.02f, %.02f; "
	    " %.02f, %.02f, %.02f, %.02f; "
	    " %.02f, %.02f, %.02f, %.02f; "
	    " %.02f, %.02f, %.02f, %.02f]",
	    M->m[0][0], M->m[0][1], M->m[0][2], M->m[0][3],
	    M->m[1][0], M->m[1][1], M->m[1][2], M->m[1][3],
	    M->m[2][0], M->m[2][1], M->m[2][2], M->m[2][3],
	    M->m[3][0], M->m[3][1], M->m[3][2], M->m[3][3]);
}

/* Initialize the math library. */
void
M_InitSubsystem(void)
{
	if (mInitedSubsystem++ > 0)
		return;

	/*
	 * Base SSE support is set based on cpuinfo (unless compiled inline).
	 * However, the SSE revision is not checked at runtime, and must match
	 * the compiled revision.
	 */
	M_VectorInitEngine();
	M_MatrixInitEngine();

#ifdef ENABLE_GUI
	if (agGUI) {
		AG_RegisterClass(&mPlotterClass);
		AG_RegisterClass(&mMatviewClass);
	}
#endif
	AG_RegisterFmtStringExt("R", PrintReal);
	AG_RegisterFmtStringExt("T", PrintTime);
	AG_RegisterFmtStringExt("C", PrintComplex);
	AG_RegisterFmtStringExt("V2", PrintVector2);
	AG_RegisterFmtStringExt("V3", PrintVector3);
	AG_RegisterFmtStringExt("V4", PrintVector4);
	AG_RegisterFmtStringExt("V", PrintVector);
	AG_RegisterFmtStringExt("M44", PrintMatrix44);
	AG_RegisterFmtStringExt("M", PrintMatrix);
}

/* Release resources allocated by the math library. */
void
M_DestroySubsystem(void)
{
	if (--mInitedSubsystem > 0)
		return;

#ifdef ENABLE_GUI
	if (agGUI) {
		AG_UnregisterClass(&mPlotterClass);
		AG_UnregisterClass(&mMatviewClass);
	}
#endif
	AG_UnregisterFmtStringExt("R");
	AG_UnregisterFmtStringExt("T");
	AG_UnregisterFmtStringExt("C");
	AG_UnregisterFmtStringExt("V2");
	AG_UnregisterFmtStringExt("V3");
	AG_UnregisterFmtStringExt("V4");
	AG_UnregisterFmtStringExt("V");
	AG_UnregisterFmtStringExt("M44");
	AG_UnregisterFmtStringExt("M");
}

/* Unserialize a real number. */
M_Real
M_ReadReal(AG_DataSource *ds)
{
	Uint8 prec;

	prec = AG_ReadUint8(ds);
	switch (prec) {
	case 1:
		return (M_Real)AG_ReadFloat(ds);
	case 2:
		return (M_Real)AG_ReadDouble(ds);
#ifdef HAVE_LONG_DOUBLE
	case 4:
		return (M_Real)AG_ReadLongDouble(ds);
#endif
	default:
		AG_FatalError("M_ReadReal(%u)", prec);
	}
	return (0.0);
}

/* Unserialize a real number (pointer variant). */
void
M_CopyReal(AG_DataSource *ds, M_Real *rv)
{
	Uint8 prec;

	prec = AG_ReadUint8(ds);
	switch (prec) {
	case 1:
		*rv = (M_Real)AG_ReadFloat(ds);
		break;
	case 2:
		*rv = (M_Real)AG_ReadDouble(ds);
		break;
#ifdef HAVE_LONG_DOUBLE
	case 4:
		*rv = (M_Real)AG_ReadLongDouble(ds);
		break;
#endif
	default:
		AG_FatalError("M_CopyReal(%u)", prec);
	}
}

/* Serialize a real number. */
void
M_WriteReal(AG_DataSource *ds, M_Real v)
{
#if defined(SINGLE_PRECISION)
	AG_WriteUint8(ds, 1);
	AG_WriteFloat(ds, v);
#elif defined(DOUBLE_PRECISION)
	AG_WriteUint8(ds, 2);
	AG_WriteDouble(ds, v);
#elif defined(QUAD_PRECISION)
	AG_WriteUint8(ds, 4);
	AG_WriteLongDouble(ds, v);
#endif
}

/* Unserialize a complex number. */
M_Complex
M_ReadComplex(AG_DataSource *ds)
{
	M_Complex v;
	v.r = M_ReadReal(ds);
	v.i = M_ReadReal(ds);
	return (v);
}

/* Unserialize a complex number (pointer variant). */
void
M_CopyComplex(AG_DataSource *ds, M_Complex *v)
{
	v->r = M_ReadReal(ds);
	v->i = M_ReadReal(ds);
}

/* Serialize a complex number. */
void
M_WriteComplex(AG_DataSource *ds, M_Complex v)
{
	M_WriteReal(ds, v.r);
	M_WriteReal(ds, v.i);
}

/* Unserialize a range value. */
M_Range
M_ReadRange(AG_DataSource *ds)
{
	M_Range r;
	r.min = M_ReadReal(ds);
	r.typ = M_ReadReal(ds);
	r.max = M_ReadReal(ds);
	return (r);
}

/* Unserialize a range value (pointer variant). */
void
M_CopyRange(AG_DataSource *ds, M_Range *r)
{
	r->min = M_ReadReal(ds);
	r->typ = M_ReadReal(ds);
	r->max = M_ReadReal(ds);
}

/* Serialize a range value. */
void
M_WriteRange(AG_DataSource *ds, M_Range r)
{
	M_WriteReal(ds, r.min);
	M_WriteReal(ds, r.typ);
	M_WriteReal(ds, r.max);
}

/* Unserialize a time range value. */
M_TimeRange
M_ReadTimeRange(AG_DataSource *ds)
{
	M_TimeRange r;
	r.min = M_ReadTime(ds);
	r.typ = M_ReadTime(ds);
	r.max = M_ReadTime(ds);
	return (r);
}

/* Unserialize a time range value (pointer variant). */
void
M_CopyTimeRange(AG_DataSource *ds, M_TimeRange *r)
{
	r->min = M_ReadTime(ds);
	r->typ = M_ReadTime(ds);
	r->max = M_ReadTime(ds);
}

/* Serialize a time range value. */
void
M_WriteTimeRange(AG_DataSource *ds, M_TimeRange r)
{
	M_WriteTime(ds, r.min);
	M_WriteTime(ds, r.typ);
	M_WriteTime(ds, r.max);
}
