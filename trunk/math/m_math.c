/*
 * Copyright (c) 2005-2008 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>
#include <config/enable_gui.h>
#include <config/have_long_double.h>

#include "m.h"

#ifdef ENABLE_GUI
# include <gui/gui.h>
# include <gui/label.h>
# include <gui/units.h>
# include "m_plotter.h"
# include "m_matview.h"
#endif

#ifdef ENABLE_GUI

/*
 * Math library extensions to AG_Label(3).
 */
static void
PrintReal(AG_Label *lbl, char *s, size_t len, int fPos)
{
	M_Real r = AG_LABEL_ARG(lbl,M_Real);
#if defined(QUAD_PRECISION)
	Snprintf(s, len, "%llf", r);
#else
	Snprintf(s, len, "%f", r);
#endif
}
static void
PrintTime(AG_Label *lbl, char *s, size_t len, int fPos)
{
	M_Time t = AG_LABEL_ARG(lbl,M_Time);
	AG_UnitFormat((double)t, agTimeUnits, s, len);
}
static void
PrintComplex(AG_Label *lbl, char *s, size_t len, int fPos)
{
	M_Complex *c = AG_LABEL_ARG(lbl,M_Complex *);
	Snprintf(s, len, "%f+%fi", c->r, c->i);
}
static void
PrintVector(AG_Label *lbl, char *s, size_t len, int fPos)
{
	char num[16];
	M_Vector *v = AG_LABEL_ARG(lbl,M_Vector *);
	Uint i;

	s[0] = '[';
	s[1] = '\0';
	for (i = 0; i < v->m; i++) {
		M_Real *e = M_VecGetElement(v,i);
		Snprintf(num, sizeof(num), "%.2g", *e);
		Strlcat(s, num, len);
		if (i < v->m-1) { Strlcat(s, "; ", len); }
	}
	Strlcat(s, "]", len);
}
static void
PrintMatrix(AG_Label *lbl, char *s, size_t len, int fPos)
{
	char num[16];
	M_Matrix *M = AG_LABEL_ARG(lbl,M_Matrix *);
	Uint i, j;

	s[0] = '[';
	s[1] = '\0';
	for (i = 0; i < M->m; i++) {
		for (j = 0; j < M->n; j++) {
			M_Real *e = M_GetElement(M,i,j);
			Snprintf(num, sizeof(num), "%.2g", *e);
			Strlcat(s, num, len);
			if (j < M->n-1) { Strlcat(s, ", ", len); }
		}
		if (i < M->m-1) { Strlcat(s, "; ", len); }
	}
	Strlcat(s, "]", len);
}
#endif /* ENABLE_GUI */

/* Initialize the math library. */
void
M_InitSubsystem(void)
{
	M_VectorInitEngine();
	M_MatrixInitEngine();

#ifdef ENABLE_GUI
	if (agGUI) {
		AG_RegisterClass(&mPlotterClass);
		AG_RegisterClass(&mMatviewClass);

		AG_RegisterLabelFormat("R", PrintReal);
		AG_RegisterLabelFormat("T", PrintTime);
		AG_RegisterLabelFormat("C", PrintComplex);
		AG_RegisterLabelFormat("V", PrintVector);
		AG_RegisterLabelFormat("M", PrintMatrix);
	}
#endif /* ENABLE_GUI */
}

/* Release resources allocated by the math library. */
void
M_DestroySubsystem(void)
{
}

/* Unserialize a real number. */
M_Real
M_ReadReal(AG_DataSource *ds)
{
	Uint8 prec;

	prec = AG_ReadUint8(ds);
	switch (prec) {
	case 1:
		return ((M_Real)AG_ReadFloat(ds));
	case 2:
		return ((M_Real)AG_ReadDouble(ds));
	case 4:
#ifdef HAVE_LONG_DOUBLE
		return ((M_Real)AG_ReadLongDouble(ds));
#else
		/* XXX TODO convert */
#endif
	default:
		AG_FatalError("Cannot convert real (%d)", (int)prec);
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
	case 4:
#ifdef HAVE_LONG_DOUBLE
		*rv = (M_Real)AG_ReadLongDouble(ds);
#else
		/* XXX TODO convert */
#endif
		break;
	default:
		AG_FatalError("Cannot convert real (%d)", (int)prec);
	}
}

/* Serialize a real number. */
void
M_WriteReal(AG_DataSource *ds, M_Real v)
{
#if defined(QUAD_PRECISION)
	AG_WriteUint8(ds, 4);
	AG_WriteLongDouble(ds, (long double)v);
#elif defined(DOUBLE_PRECISION)
	AG_WriteUint8(ds, 2);
	AG_WriteDouble(ds, (double)v);
#else
	AG_WriteUint8(ds, 1);
	AG_WriteFloat(ds, (float)v);
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
