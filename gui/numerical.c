/*
 * Copyright (c) 2007-2012 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>

#include "numerical.h"
#include "primitive.h"

#include <string.h>

#include <config/_mk_have_strtold.h>
#include <config/_mk_have_strtoll.h>
#if defined(_MK_HAVE_STRTOLD_H) || defined(_MK_HAVE_STRTOLL_H)
# define _XOPEN_SOURCE 600
# include <stdlib.h>
#endif

static void UnitSelected(AG_Event *);

AG_Numerical *
AG_NumericalNew(void *parent, Uint flags, const char *unit, const char *fmt,
    ...)
{
	char s[AG_LABEL_MAX];
	va_list ap;

	if (fmt != NULL) {
		va_start(ap, fmt);
		Vsnprintf(s, sizeof(s), fmt, ap);
		va_end(ap);
		return AG_NumericalNewS(parent, flags, unit, s);
	} else {
		return AG_NumericalNewS(parent, flags, unit, NULL);
	}
}

AG_Numerical *
AG_NumericalNewS(void *parent, Uint flags, const char *unit, const char *label)
{
	AG_Numerical *num;

	num = Malloc(sizeof(AG_Numerical));
	AG_ObjectInit(num, &agNumericalClass);

	if (flags & AG_NUMERICAL_HFILL) { AG_ExpandHoriz(num); }
	if (flags & AG_NUMERICAL_VFILL) { AG_ExpandVert(num); }
	if (label != NULL) {
		AG_TextboxSetLabelS(num->input, label);
	}
	if (unit != NULL) {
		num->units = AG_UComboNew(num, 0);
		AG_SetEvent(num->units, "ucombo-selected",
		    UnitSelected, "%p", num);
		AG_NumericalSetUnitSystem(num, unit);
		AG_WidgetSetFocusable(num->units, 0);
	}

	AG_ObjectAttach(parent, num);
	return (num);
}

AG_Numerical *
AG_NumericalNewDbl(void *parent, Uint flags, const char *unit, const char *label, double *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindDouble(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewDblR(void *parent, Uint flags, const char *unit, const char *label, double *v, double min, double max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindDouble(num, "value", v);
	num->min.dbl = min;
	num->max.dbl = max;
	AG_BindDouble(num, "min", &num->min.dbl);
	AG_BindDouble(num, "max", &num->max.dbl);
	return (num);
}

#ifdef HAVE_LONG_DOUBLE
AG_Numerical *
AG_NumericalNewLdbl(void *parent, Uint flags, const char *unit, const char *label, long double *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindLongDouble(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewLdblR(void *parent, Uint flags, const char *unit, const char *label, long double *v, long double min, long double max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindLongDouble(num, "value", v);
	num->min.ldbl = min;
	num->max.ldbl = max;
	AG_BindLongDouble(num, "min", &num->min.ldbl);
	AG_BindLongDouble(num, "max", &num->max.ldbl);
	return (num);
}
#endif /* HAVE_LONG_DOUBLE */

AG_Numerical *
AG_NumericalNewFlt(void *parent, Uint flags, const char *unit, const char *label, float *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindFloat(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewFltR(void *parent, Uint flags, const char *unit, const char *label, float *v, float min, float max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindFloat(num, "value", v);
	num->min.flt = min;
	num->max.flt = max;
	AG_BindFloat(num, "min", &num->min.flt);
	AG_BindFloat(num, "max", &num->max.flt);
	return (num);
}

AG_Numerical *
AG_NumericalNewInt(void *parent, Uint flags, const char *unit, const char *label, int *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindInt(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewIntR(void *parent, Uint flags, const char *unit, const char *label, int *v, int min, int max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindInt(num, "value", v);
	num->min.i = min;
	num->max.i = max;
	AG_BindInt(num, "min", &num->min.i);
	AG_BindInt(num, "max", &num->max.i);
	return (num);
}
AG_Numerical *
AG_NumericalNewUint(void *parent, Uint flags, const char *unit, const char *label, Uint *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindUint(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewUintR(void *parent, Uint flags, const char *unit, const char *label, Uint *v, Uint min, Uint max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindUint(num, "value", v);
	num->min.u = min;
	num->max.u = max;
	AG_BindUint(num, "min", &num->min.u);
	AG_BindUint(num, "max", &num->max.u);
	return (num);
}
AG_Numerical *
AG_NumericalNewUint8(void *parent, Uint flags, const char *unit, const char *label, Uint8 *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindUint8(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewUint8R(void *parent, Uint flags, const char *unit, const char *label, Uint8 *v, Uint8 min, Uint8 max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindUint8(num, "value", v);
	num->min.u8 = min;
	num->max.u8 = max;
	AG_BindUint8(num, "min", &num->min.u8);
	AG_BindUint8(num, "max", &num->max.u8);
	return (num);
}
AG_Numerical *
AG_NumericalNewSint8(void *parent, Uint flags, const char *unit, const char *label, Sint8 *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindSint8(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewSint8R(void *parent, Uint flags, const char *unit, const char *label, Sint8 *v, Sint8 min, Sint8 max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindSint8(num, "value", v);
	num->min.s8 = min;
	num->max.s8 = max;
	AG_BindSint8(num, "min", &num->min.s8);
	AG_BindSint8(num, "max", &num->max.s8);
	return (num);
}
AG_Numerical *
AG_NumericalNewUint16(void *parent, Uint flags, const char *unit, const char *label, Uint16 *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindUint16(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewUint16R(void *parent, Uint flags, const char *unit, const char *label, Uint16 *v, Uint16 min, Uint16 max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindUint16(num, "value", v);
	num->min.u16 = min;
	num->max.u16 = max;
	AG_BindUint16(num, "min", &num->min.u16);
	AG_BindUint16(num, "max", &num->max.u16);
	return (num);
}
AG_Numerical *
AG_NumericalNewSint16(void *parent, Uint flags, const char *unit, const char *label, Sint16 *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindSint16(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewSint16R(void *parent, Uint flags, const char *unit, const char *label, Sint16 *v, Sint16 min, Sint16 max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindSint16(num, "value", v);
	num->min.s16 = min;
	num->max.s16 = max;
	AG_BindSint16(num, "min", &num->min.s16);
	AG_BindSint16(num, "max", &num->max.s16);
	return (num);
}

AG_Numerical *
AG_NumericalNewUint32(void *parent, Uint flags, const char *unit, const char *label, Uint32 *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindUint32(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewUint32R(void *parent, Uint flags, const char *unit, const char *label, Uint32 *v, Uint32 min, Uint32 max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindUint32(num, "value", v);
	num->min.u32 = min;
	num->max.u32 = max;
	AG_BindUint32(num, "min", &num->min.u32);
	AG_BindUint32(num, "max", &num->max.u32);
	return (num);
}

AG_Numerical *
AG_NumericalNewSint32(void *parent, Uint flags, const char *unit, const char *label, Sint32 *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindSint32(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewSint32R(void *parent, Uint flags, const char *unit, const char *label, Sint32 *v, Sint32 min, Sint32 max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindSint32(num, "value", v);
	num->min.s32 = min;
	num->max.s32 = max;
	AG_BindSint32(num, "min", &num->min.s32);
	AG_BindSint32(num, "max", &num->max.s32);
	return (num);
}

#ifdef HAVE_64BIT
AG_Numerical *
AG_NumericalNewUint64(void *parent, Uint flags, const char *unit, const char *label, Uint64 *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindUint64(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewUint64R(void *parent, Uint flags, const char *unit, const char *label, Uint64 *v, Uint64 min, Uint64 max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindUint64(num, "value", v);
	num->min.u64 = min;
	num->max.u64 = max;
	AG_BindUint64(num, "min", &num->min.u64);
	AG_BindUint64(num, "max", &num->max.u64);
	return (num);
}

AG_Numerical *
AG_NumericalNewSint64(void *parent, Uint flags, const char *unit, const char *label, Sint64 *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindSint64(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewSint64R(void *parent, Uint flags, const char *unit, const char *label, Sint64 *v, Sint64 min, Sint64 max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindSint64(num, "value", v);
	num->min.s64 = min;
	num->max.s64 = max;
	AG_BindSint64(num, "min", &num->min.s64);
	AG_BindSint64(num, "max", &num->max.s64);
	return (num);
}
#endif /* HAVE_64BIT */

static Uint32
UpdateTimeout(AG_Timer *to, AG_Event *event)
{
	AG_Numerical *num = AG_SELF();
	
	if (!AG_WidgetIsFocused(num)) {
		AG_NumericalUpdate(num);
	}
	return (to->ival);
}

static void
OnShow(AG_Event *event)
{
	AG_Numerical *num = AG_SELF();
	AG_Variable *V;

	if ((num->flags & AG_NUMERICAL_EXCL) == 0) {
		AG_AddTimer(num, &num->updateTo, 250, UpdateTimeout, NULL);
	}
	if ((V = AG_GetVariableLocked(num, "value")) == NULL) {
		if (num->flags & AG_NUMERICAL_INT) {
			num->value.i = 0;
			V = AG_BindInt(num, "value", &num->value.i);
		} else {
			num->value.dbl = 0.0;
			V = AG_BindDouble(num, "value", &num->value.dbl);
		}
		if (V == NULL) {
			return;
		}
		AG_LockVariable(V);
	}
	switch (V->type) {
	case AG_VARIABLE_P_FLOAT:
		if (!AG_Defined(num,"min")) {
			num->min.flt = -AG_FLT_MAX+1;
			AG_BindFloat(num, "min", &num->min.flt);
		}
		if (!AG_Defined(num,"max")) {
			num->max.flt = +AG_FLT_MAX-1;
			AG_BindFloat(num, "max", &num->max.flt);
		}
		break;
	case AG_VARIABLE_P_DOUBLE:
		if (!AG_Defined(num,"min")) {
			num->min.dbl = -AG_DBL_MAX+1;
			AG_BindDouble(num, "min", &num->min.dbl);
		}
		if (!AG_Defined(num,"max")) {
			num->max.dbl = +AG_DBL_MAX-1;
			AG_BindDouble(num, "max", &num->max.dbl);
		}
		break;
#ifdef HAVE_LONG_DOUBLE
	case AG_VARIABLE_P_LONG_DOUBLE:
		if (!AG_Defined(num,"min")) {
			num->min.ldbl = -AG_LDBL_MIN+1;
			AG_BindLongDouble(num, "min", &num->min.ldbl);
		}
		if (!AG_Defined(num,"max")) {
			num->max.ldbl = +AG_LDBL_MAX-1;
			AG_BindLongDouble(num, "max", &num->max.ldbl);
		}
		break;
#endif
	case AG_VARIABLE_P_INT:
		if (!AG_Defined(num,"min")) {
			num->min.i = AG_INT_MIN+1;
			AG_BindInt(num, "min", &num->min.i);
		}
		if (!AG_Defined(num,"max")) {
			num->max.i = AG_INT_MAX-1;
			AG_BindInt(num, "max", &num->max.i);
		}
		break;
	case AG_VARIABLE_P_UINT:
		if (!AG_Defined(num,"min")) {
			num->min.u = 0U;
			AG_BindUint(num, "min", &num->min.u);
		}
		if (!AG_Defined(num,"max")) {
			num->max.u = AG_UINT_MAX-1;
			AG_BindUint(num, "max", &num->max.u);
		}
		break;
	case AG_VARIABLE_P_UINT8:
		if (!AG_Defined(num,"min")) {
			num->min.u8 = 0U;
			AG_BindUint8(num, "min", &num->min.u8);
		}
		if (!AG_Defined(num,"max")) {
			num->max.u8 = 0xffU;
			AG_BindUint8(num, "max", &num->max.u8);
		}
		break;
	case AG_VARIABLE_P_SINT8:
		if (!AG_Defined(num,"min")) {
			num->min.s8 = -0x7f+1;
			AG_BindSint8(num, "min", &num->min.s8);
		}
		if (!AG_Defined(num,"max")) {
			num->max.s8 = +0x7f-1;
			AG_BindSint8(num, "max", &num->max.s8);
		}
		break;
	case AG_VARIABLE_P_UINT16:
		if (!AG_Defined(num,"min")) {
			num->min.u16 = 0U;
			AG_BindUint16(num, "min", &num->min.u16);
		}
		if (!AG_Defined(num,"max")) {
			num->max.u16 = 0xffffU;
			AG_BindUint16(num, "max", &num->max.u16);
		}
		break;
	case AG_VARIABLE_P_SINT16:
		if (!AG_Defined(num,"min")) {
			num->min.s16 = -0x7fff+1;
			AG_BindSint16(num, "min", &num->min.s16);
		}
		if (!AG_Defined(num,"max")) {
			num->max.s16 = +0x7fff-1;
			AG_BindSint16(num, "max", &num->max.s16);
		}
		break;
	case AG_VARIABLE_P_UINT32:
		if (!AG_Defined(num,"min")) {
			num->min.u32 = 0UL;
			AG_BindUint32(num, "min", &num->min.u32);
		}
		if (!AG_Defined(num,"max")) {
			num->max.u32 = 0xffffffffUL;
			AG_BindUint32(num, "max", &num->max.u32);
		}
		break;
	case AG_VARIABLE_P_SINT32:
		if (!AG_Defined(num,"min")) {
			num->min.s32 = -0x7fffffffL+1;
			AG_BindSint32(num, "min", &num->min.s32);
		}
		if (!AG_Defined(num,"max")) {
			num->max.s32 = +0x7fffffffL-1;
			AG_BindSint32(num, "max", &num->max.s32);
		}
		break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_P_UINT64:
		if (!AG_Defined(num,"min")) {
			num->min.u64 = 0ULL;
			AG_BindUint64(num, "min", &num->min.u64);
		}
		if (!AG_Defined(num,"max")) {
			num->max.u64 = 0xffffffffffffffffULL;
			AG_BindUint64(num, "max", &num->max.u64);
		}
		break;
	case AG_VARIABLE_P_SINT64:
		if (!AG_Defined(num,"min")) {
			num->max.s64 = -0x7fffffffffffffffLL+1;
			AG_BindSint64(num, "min", &num->min.s64);
		}
		if (!AG_Defined(num,"max")) {
			num->max.s64 = +0x7fffffffffffffffLL-1;
			AG_BindSint64(num, "max", &num->max.s64);
		}
		break;
#endif /* HAVE_64BIT */
	default:
		break;
	}
	switch (V->type) {
	case AG_VARIABLE_P_FLOAT:
	case AG_VARIABLE_P_DOUBLE:
	case AG_VARIABLE_P_LONG_DOUBLE:
		AG_TextboxSetFltOnly(num->input, 1);
		break;
	default:
		AG_TextboxSetIntOnly(num->input, 1);
		break;
	}
	AG_UnlockVariable(V);
	AG_NumericalUpdate(num);
}

static void
KeyDown(AG_Event *event)
{
	AG_Numerical *num = AG_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case AG_KEY_UP:
		AG_NumericalIncrement(num);
		break;
	case AG_KEY_DOWN:
		AG_NumericalDecrement(num);
		break;
	}
}

/* Update the numerical value from the textbox. */
static void
UpdateFromText(AG_Event *event)
{
	AG_NumericalValue val;
	AG_Numerical *num = AG_PTR(1);
	int unfocus = AG_INT(2);
	AG_Variable *valueb;
	void *value;

	valueb = AG_GetVariable(num, "value", &value);

	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_FLOAT:
		val.flt = (float)AG_Unit2Base(strtod(num->inTxt, NULL), num->unit);
		break;
	case AG_VARIABLE_DOUBLE:
		val.dbl = AG_Unit2Base(strtod(num->inTxt, NULL), num->unit);
		break;
#ifdef HAVE_LONG_DOUBLE
	case AG_VARIABLE_LONG_DOUBLE:
# ifdef _MK_HAVE_STRTOLD
		val.ldbl = AG_Unit2BaseLDBL(strtold(num->inTxt, NULL), num->unit);
# else
		val.ldbl = AG_Unit2BaseLDBL((long double)strtod(num->inTxt, NULL), num->unit);
# endif
		break;
#endif
	case AG_VARIABLE_INT:
		val.i = (int)strtol(num->inTxt, NULL, 10);
		break;
	case AG_VARIABLE_UINT:
		val.u = (Uint)strtol(num->inTxt, NULL, 10);
		break;
	case AG_VARIABLE_UINT8:
		val.u8 = (Uint8)strtol(num->inTxt, NULL, 10);
		break;
	case AG_VARIABLE_SINT8:
		val.s8 = (Sint8)strtol(num->inTxt, NULL, 10);
		break;
	case AG_VARIABLE_UINT16:
		val.u16 = (Uint16)strtol(num->inTxt, NULL, 10);
		break;
	case AG_VARIABLE_SINT16:
		val.s16 = (Sint16)strtol(num->inTxt, NULL, 10);
		break;
	case AG_VARIABLE_UINT32:
		val.u32 = (Uint32)strtol(num->inTxt, NULL, 10);
		break;
	case AG_VARIABLE_SINT32:
		val.s32 = (Sint32)strtol(num->inTxt, NULL, 10);
		break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:
# ifdef _MK_HAVE_STRTOLL
		val.u64 = (Uint64)strtoll(num->inTxt, NULL, 10);
# else
		val.u64 = (Uint64)strtol(num->inTxt, NULL, 10);
# endif
		break;
	case AG_VARIABLE_SINT64:
# ifdef _MK_HAVE_STRTOLL
		val.s64 = (Sint64)strtoll(num->inTxt, NULL, 10);
# else
		val.s64 = (Sint64)strtol(num->inTxt, NULL, 10);
# endif
		break;
#endif
	default:
		break;
	}
	AG_NumericalSetValue(num, val);
	AG_UnlockVariable(valueb);

	if (unfocus) {
		AG_WidgetUnfocus(num->input);
	}
	AG_PostEvent(NULL, num, "numerical-return", NULL);
}

static void
IncrementValue(AG_Event *event)
{
	AG_Numerical *num = AG_PTR(1);

	AG_ObjectLock(num);
	AG_NumericalIncrement(num);
	AG_ObjectUnlock(num);
}

static void
DecrementValue(AG_Event *event)
{
	AG_Numerical *num = AG_PTR(1);
	
	AG_ObjectLock(num);
	AG_NumericalDecrement(num);
	AG_ObjectUnlock(num);
}

static void
UpdateUnitSelector(AG_Numerical *num)
{
	AG_ButtonTextS(num->units->button, AG_UnitAbbr(num->unit));
	if (WIDGET(num)->window != NULL &&
	    WIDGET(num)->window->visible)
		AG_NumericalUpdate(num);
}

static void
UnitSelected(AG_Event *event)
{
	AG_Numerical *num = AG_PTR(1);
	AG_TlistItem *ti = AG_PTR(2);

	AG_ObjectLock(num);
	num->unit = (const AG_Unit *)ti->p1;
	UpdateUnitSelector(num);
	AG_ObjectUnlock(num);
}

int
AG_NumericalSetUnitSystem(AG_Numerical *num, const char *unit_key)
{
	const AG_Unit *unit = NULL;
	const AG_Unit *ugroup = NULL;
	int found = 0, i;
	int w, h, nUnits = 0;

	AG_ObjectLock(num);

	for (i = 0; i < agnUnitGroups; i++) {
		ugroup = agUnitGroups[i];
		for (unit = &ugroup[0]; unit->key != NULL; unit++) {
			if (strcmp(unit->key, unit_key) == 0) {
				found++;
				break;
			}
		}
		if (found)
			break;
	}
	if (!found) {
		AG_SetError(_("No such unit: %s"), unit_key);
		AG_ObjectUnlock(num);
		return (-1);
	}
	num->unit = unit;
	UpdateUnitSelector(num);

	num->wUnitSel = 0;
	num->hUnitSel = 0;
	num->wPreUnit = 0;

	AG_ObjectLock(num->units->list);
	AG_TlistDeselectAll(num->units->list);
	AG_TlistBegin(num->units->list);
	for (unit = &ugroup[0]; unit->key != NULL; unit++) {
		AG_TlistItem *it;
	
		AG_TextSize(AG_UnitAbbr(unit), &w, &h);
		if (w > num->wUnitSel) { num->wUnitSel = w; }
		if (h > num->hUnitSel) { num->hUnitSel = h; }
		
		AG_TextSize(unit->name, &w, NULL);
		if (w > num->wPreUnit) { num->wPreUnit = w; }

		it = AG_TlistAddPtr(num->units->list, NULL, _(unit->name),
		    (void *)unit);
		if (unit == num->unit)
			it->selected++;

		nUnits++;
	}
	AG_TlistEnd(num->units->list);
	AG_TlistSizeHintLargest(num->units->list, 5);
	AG_ObjectUnlock(num->units->list);

	if (num->wPreUnit > 0) { num->wPreUnit += 8; }
	AG_UComboSizeHintPixels(num->units, num->wPreUnit,
	    nUnits<6 ? (nUnits + 1) : 6);
	
	AG_WidgetUpdate(num);
	AG_ObjectUnlock(num);
	return (0);
}

/* Update the input text from the binding value. */
void
AG_NumericalUpdate(AG_Numerical *num)
{
	AG_Variable *valueb;
	void *value;
	char s[64];

	valueb = AG_GetVariable(num, "value", &value);
	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_DOUBLE:
		Snprintf(s, sizeof(s), num->format,
		    AG_Base2Unit(*(double *)value, num->unit));
		break;
	case AG_VARIABLE_FLOAT:
		Snprintf(s, sizeof(s), num->format,
		    AG_Base2Unit(*(float *)value, num->unit));
		break;
	case AG_VARIABLE_INT:	 StrlcpyInt(s, *(int *)value, sizeof(s));	break;
	case AG_VARIABLE_UINT:	 StrlcpyUint(s, *(Uint *)value, sizeof(s));	break;
	case AG_VARIABLE_UINT8:	 StrlcpyUint(s, *(Uint8 *)value, sizeof(s));	break;
	case AG_VARIABLE_SINT8:	 StrlcpyInt(s, *(Sint8 *)value, sizeof(s));	break;
	case AG_VARIABLE_UINT16: StrlcpyUint(s, *(Uint16 *)value, sizeof(s));	break;
	case AG_VARIABLE_SINT16: StrlcpyInt(s, *(Sint16 *)value, sizeof(s));	break;
	case AG_VARIABLE_UINT32: StrlcpyUint(s, *(Uint32 *)value, sizeof(s));	break;
	case AG_VARIABLE_SINT32: StrlcpyInt(s, *(Sint32 *)value, sizeof(s));	break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64: Snprintf(s, sizeof(s), "%llu", (unsigned long long)*(Uint64 *)value);	break;
	case AG_VARIABLE_SINT64: Snprintf(s, sizeof(s), "%lld", (long long)*(Sint64 *)value);		break;
#endif
	default:
		break;
	}
	if (strcmp(num->inTxt, s) != 0) {
		AG_TextboxSetString(num->input, s);
	}
	AG_UnlockVariable(valueb);
}

static void
Init(void *obj)
{
	AG_Numerical *num = obj;

	WIDGET(num)->flags |= AG_WIDGET_FOCUSABLE|
	                      AG_WIDGET_TABLE_EMBEDDABLE;

	num->flags = 0;
	num->incr = 1.0;
	num->writeable = 1;
	num->wUnitSel = 0;
	num->hUnitSel = 0;
	num->inTxt[0] = '\0';
	Strlcpy(num->format, "%.02f", sizeof(num->format));
	
	num->input = AG_TextboxNewS(num, AG_TEXTBOX_EXCL, NULL);
	AG_TextboxBindASCII(num->input, num->inTxt, sizeof(num->inTxt));
	AG_TextboxSizeHint(num->input, "8888.88");
	
	num->unit = AG_FindUnit("identity");
	num->units = NULL;
	
	num->incbu = AG_ButtonNewS(num, AG_BUTTON_REPEAT, _("+"));
	AG_ButtonSetPadding(num->incbu, 0,0,0,0);
	AG_LabelSetPadding(num->incbu->lbl, 0,0,0,0);
	AG_WidgetSetFocusable(num->incbu, 0);

	num->decbu = AG_ButtonNewS(num, AG_BUTTON_REPEAT, _("-"));
	AG_ButtonSetPadding(num->decbu, 0,0,0,0);
	AG_LabelSetPadding(num->decbu->lbl, 0,0,0,0);
	AG_WidgetSetFocusable(num->decbu, 0);

	AG_AddEvent(num, "widget-shown", OnShow, NULL);
	AG_SetEvent(num, "key-down", KeyDown, NULL);
	AG_SetEvent(num->incbu, "button-pushed", IncrementValue, "%p", num);
	AG_SetEvent(num->decbu, "button-pushed", DecrementValue, "%p", num);
	AG_SetEvent(num->input, "textbox-return", UpdateFromText, "%p,%i", num, 1);
	AG_SetEvent(num->input, "textbox-changed", UpdateFromText, "%p,%i", num, 0);
	AG_WidgetForwardFocus(num, num->input);
}

void
AG_NumericalSizeHint(AG_Numerical *num, const char *text)
{
	AG_ObjectLock(num);
	AG_TextboxSizeHint(num->input, text);
	AG_ObjectUnlock(num);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Numerical *num = obj;
	AG_SizeReq rChld, rInc, rDec;

	AG_WidgetSizeReq(num->input, &rChld);
	r->w = rChld.w + num->wUnitSel + 4;
	r->h = MAX(rChld.h, num->hUnitSel);

	AG_WidgetSizeReq(num->incbu, &rInc);
	AG_WidgetSizeReq(num->decbu, &rDec);
	r->w += MAX(rInc.w, rDec.w) + 4;
	r->h += 2;
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Numerical *num = obj;
	AG_SizeAlloc aChld;
	int szBtn = a->h/2;
	int wUnitSel = num->wUnitSel + 4;
	int hUnitSel = num->hUnitSel;

	if (a->h < 4 || a->w < szBtn+4)
		return (-1);

	if (num->units != NULL) {
		if (wUnitSel > a->w - szBtn-4) {
			wUnitSel = a->w - szBtn-4;
		}
		if (hUnitSel > a->h) {
			hUnitSel = a->h;
		}
	} else {
		wUnitSel = 0;
		hUnitSel = 0;
	}

	/* Size input textbox */
	aChld.x = 0;
	aChld.y = 0;
	aChld.w = a->w - wUnitSel - szBtn - 4;
	aChld.h = a->h;
	AG_WidgetSizeAlloc(num->input, &aChld);
	aChld.x += aChld.w + 2;

	/* Size unit selector */
	if (num->units != NULL) {
		aChld.w = wUnitSel;
		aChld.h = a->h;
		AG_WidgetSizeAlloc(num->units, &aChld);
		aChld.x += aChld.w + 2;
	}

	/* Size increment buttons */
	aChld.w = szBtn;
	aChld.h = szBtn;
	AG_WidgetSizeAlloc(num->incbu, &aChld);
	aChld.y += aChld.h;
	if (aChld.h*2 < a->h) {
		aChld.h++;
	}
	AG_WidgetSizeAlloc(num->decbu, &aChld);
	return (0);
}

static void
Draw(void *obj)
{
	AG_Numerical *num = obj;

	AG_WidgetDraw(num->input);
	if (num->units != NULL) { AG_WidgetDraw(num->units); }
	AG_WidgetDraw(num->incbu);
	AG_WidgetDraw(num->decbu);
}

/*
 * Type-independent increment operation.
 */
#undef ADD_INT
#define ADD_INT(TYPE,_memb) do { \
	n._memb = *(TYPE *)value; \
	if ((n._memb + num->incr) < *(TYPE *)min) { n._memb = *(TYPE *)min; } \
	else if ((n._memb + num->incr) > *(TYPE *)max) { n._memb = *(TYPE *)max; } \
	else { n._memb += num->incr; } \
	*(TYPE *)value = n._memb; \
} while (0)
#undef ADD_REAL
#define ADD_REAL(TYPE,_memb) do { \
	n._memb = AG_Base2Unit((double)*(TYPE *)value, num->unit); \
	if ((n._memb + num->incr) < *(TYPE *)min) { n._memb = *(TYPE *)min; } \
	else if ((n._memb + num->incr) > *(TYPE *)max) { n._memb = *(TYPE *)max; } \
	else { n._memb += num->incr; } \
	*(TYPE *)value = AG_Unit2Base((double)n._memb, num->unit); \
} while (0)
#undef ADD_LDBL
#define ADD_LDBL(TYPE,_memb) do { \
	n._memb = AG_Base2UnitLDBL((long double)*(TYPE *)value, num->unit); \
	if ((n._memb + num->incr) < *(TYPE *)min) { n._memb = *(TYPE *)min; } \
	else if ((n._memb + num->incr) > *(TYPE *)max) { n._memb = *(TYPE *)max; } \
	else { n._memb += num->incr; } \
	*(TYPE *)value = AG_Unit2BaseLDBL((long double)n._memb, num->unit); \
} while (0)
void
AG_NumericalIncrement(AG_Numerical *num)
{
	AG_NumericalValue n;
	AG_Variable *valueb, *minb, *maxb;
	void *value, *min, *max;

	AG_ObjectLock(num);
	valueb = AG_GetVariable(num, "value", &value);
	minb = AG_GetVariable(num, "min", &min);
	maxb = AG_GetVariable(num, "max", &max);

	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_FLOAT:		ADD_REAL(float,flt);		break;
	case AG_VARIABLE_DOUBLE:	ADD_REAL(double,dbl);		break;
#ifdef HAVE_LONG_DOUBLE
	case AG_VARIABLE_LONG_DOUBLE:	ADD_LDBL(long double,ldbl);	break;
#endif
	case AG_VARIABLE_INT:		ADD_INT(int,i);			break;
	case AG_VARIABLE_UINT:		ADD_INT(Uint,u);		break;
	case AG_VARIABLE_UINT8:		ADD_INT(Uint8,u8);		break;
	case AG_VARIABLE_SINT8:		ADD_INT(Sint8,s8);		break;
	case AG_VARIABLE_UINT16:	ADD_INT(Uint16,u16);		break;
	case AG_VARIABLE_SINT16:	ADD_INT(Sint16,s16);		break;
	case AG_VARIABLE_UINT32:	ADD_INT(Uint32,u32);		break;
	case AG_VARIABLE_SINT32:	ADD_INT(Sint32,s32);		break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	ADD_INT(Uint64,u64);		break;
	case AG_VARIABLE_SINT64:	ADD_INT(Sint64,s64);		break;
#endif
	default:							break;
	}
	AG_PostEvent(NULL, num, "numerical-changed", NULL);
	AG_UnlockVariable(valueb);
	AG_UnlockVariable(minb);
	AG_UnlockVariable(maxb);

	AG_NumericalUpdate(num);
	AG_ObjectUnlock(num);
}
#undef ADD_INT
#undef ADD_REAL
#undef ADD_LDBL

/*
 * Type-independent decrement operation.
 */
#undef SUB_INT
#define SUB_INT(TYPE,_memb) do { \
	n._memb = *(TYPE *)value; \
	if ((n._memb - num->incr) < *(TYPE *)min) { n._memb = *(TYPE *)min; } \
	else if ((n._memb - num->incr) > *(TYPE *)max) { n._memb = *(TYPE *)max; } \
	else { n._memb -= num->incr; } \
	*(TYPE *)value = n._memb; \
} while (0)
#undef SUB_REAL
#define SUB_REAL(TYPE,_memb) do { \
	n._memb = AG_Base2Unit((double)*(TYPE *)value, num->unit); \
	if ((n._memb - num->incr) < *(TYPE *)min) { n._memb = *(TYPE *)min; } \
	else if ((n._memb - num->incr) > *(TYPE *)max) { n._memb = *(TYPE *)max; } \
	else { n._memb -= num->incr; } \
	*(TYPE *)value = AG_Unit2Base((double)n._memb, num->unit); \
} while (0)
#undef SUB_LDBL
#define SUB_LDBL(TYPE,_memb) do { \
	n._memb = AG_Base2UnitLDBL((long double)*(TYPE *)value, num->unit); \
	if ((n._memb - num->incr) < *(TYPE *)min) { n._memb = *(TYPE *)min; } \
	else if ((n._memb - num->incr) > *(TYPE *)max) { n._memb = *(TYPE *)max; } \
	else { n._memb -= num->incr; } \
	*(TYPE *)value = AG_Unit2BaseLDBL((long double)n._memb, num->unit); \
} while (0)
void
AG_NumericalDecrement(AG_Numerical *num)
{
	AG_NumericalValue n;
	AG_Variable *valueb, *minb, *maxb;
	void *value, *min, *max;

	AG_ObjectLock(num);
	valueb = AG_GetVariable(num, "value", &value);
	minb = AG_GetVariable(num, "min", &min);
	maxb = AG_GetVariable(num, "max", &max);

	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_FLOAT:		SUB_REAL(float,flt);		break;
	case AG_VARIABLE_DOUBLE:	SUB_REAL(double,dbl);		break;
#ifdef HAVE_LONG_DOUBLE
	case AG_VARIABLE_LONG_DOUBLE:	SUB_LDBL(long double,ldbl);	break;
#endif
	case AG_VARIABLE_INT:		SUB_INT(int,i);			break;
	case AG_VARIABLE_UINT:		SUB_INT(Uint,u);		break;
	case AG_VARIABLE_UINT8:		SUB_INT(Uint8,u8);		break;
	case AG_VARIABLE_SINT8:		SUB_INT(Sint8,s8);		break;
	case AG_VARIABLE_UINT16:	SUB_INT(Uint16,u16);		break;
	case AG_VARIABLE_SINT16:	SUB_INT(Sint16,s16);		break;
	case AG_VARIABLE_UINT32:	SUB_INT(Uint32,u32);		break;
	case AG_VARIABLE_SINT32:	SUB_INT(Sint32,s32);		break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	SUB_INT(Uint64,u64);		break;
	case AG_VARIABLE_SINT64:	SUB_INT(Sint64,s64);		break;
#endif
	default:							break;
	}
	AG_PostEvent(NULL, num, "numerical-changed", NULL);
	AG_UnlockVariable(valueb);
	AG_UnlockVariable(minb);
	AG_UnlockVariable(maxb);

	AG_NumericalUpdate(num);
	AG_ObjectUnlock(num);
}
#undef SUB_INT
#undef SUB_REAL
#undef SUB_LDBL

/*
 * Type-independent set operation.
 */
#undef SET_NUM
#define SET_NUM(TYPE,_memb) \
    *(TYPE *)value = newVal._memb < *(TYPE *)min ? *(TYPE *)min : \
                     newVal._memb > *(TYPE *)max ? *(TYPE *)max : \
		     newVal._memb 
void
AG_NumericalSetValue(AG_Numerical *num, AG_NumericalValue newVal)
{
	AG_Variable *valueb, *minb, *maxb;
	void *value, *min, *max;

	AG_ObjectLock(num);
	valueb = AG_GetVariable(num, "value", &value);
	minb = AG_GetVariable(num, "min", &min);
	maxb = AG_GetVariable(num, "max", &max);

	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_FLOAT:		SET_NUM(float,flt);		break;
	case AG_VARIABLE_DOUBLE:	SET_NUM(double,dbl);		break;
#ifdef HAVE_LONG_DOUBLE
	case AG_VARIABLE_LONG_DOUBLE:	SET_NUM(long double,ldbl);	break;
#endif
	case AG_VARIABLE_INT:		SET_NUM(int,i);			break;
	case AG_VARIABLE_UINT:		SET_NUM(Uint,u);		break;
	case AG_VARIABLE_UINT8:		SET_NUM(Uint8,u8);		break;
	case AG_VARIABLE_SINT8:		SET_NUM(Sint8,s8);		break;
	case AG_VARIABLE_UINT16:	SET_NUM(Uint16,u16);		break;
	case AG_VARIABLE_SINT16:	SET_NUM(Sint16,s16);		break;
	case AG_VARIABLE_UINT32:	SET_NUM(Uint32,u32);		break;
	case AG_VARIABLE_SINT32:	SET_NUM(Sint32,s32);		break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	SET_NUM(Uint64,u64);		break;
	case AG_VARIABLE_SINT64:	SET_NUM(Sint64,s64);		break;
#endif
	default:							break;
	}
	AG_PostEvent(NULL, num, "numerical-changed", NULL);
	AG_UnlockVariable(valueb);
	AG_UnlockVariable(minb);
	AG_UnlockVariable(maxb);

	AG_NumericalUpdate(num);
	AG_ObjectUnlock(num);
}
#undef SET_NUM

void
AG_NumericalSetIncrement(AG_Numerical *num, double incr)
{
	AG_ObjectLock(num);
	num->incr = incr;
	AG_ObjectUnlock(num);
}

void
AG_NumericalSetPrecision(AG_Numerical *num, const char *mode,
    int precision)
{
	AG_ObjectLock(num);
	num->format[0] = '%';
	num->format[1] = '.';
	num->format[2] = '\0';
	StrlcatInt(num->format, precision, sizeof(num->format));
	Strlcat(num->format, mode, sizeof(num->format));
	AG_NumericalUpdate(num);
	AG_ObjectUnlock(num);
}

void
AG_NumericalSelectUnit(AG_Numerical *num, const char *uname)
{
	AG_TlistItem *it;

	AG_ObjectLock(num);
	AG_ObjectLock(num->units->list);
	AG_TlistDeselectAll(num->units->list);
	TAILQ_FOREACH(it, &num->units->list->items, items) {
		const AG_Unit *unit = it->p1;

		if (strcmp(unit->key, uname) == 0) {
			it->selected++;
			num->unit = unit;
			UpdateUnitSelector(num);
			break;
		}
	}
	AG_ObjectUnlock(num->units->list);
	AG_ObjectUnlock(num);
}

void
AG_NumericalSetWriteable(AG_Numerical *num, int writeable)
{
	AG_ObjectLock(num);
	num->writeable = writeable;
	if (writeable) {
		AG_WidgetEnable(num->incbu);
		AG_WidgetEnable(num->decbu);
		AG_WidgetEnable(num->input);
	} else {
		AG_WidgetDisable(num->incbu);
		AG_WidgetDisable(num->decbu);
		AG_WidgetDisable(num->input);
	}
	AG_ObjectUnlock(num);
}

/* Convert the bound value to a float. */
float
AG_NumericalGetFlt(AG_Numerical *num)
{
	AG_Variable *bValue;
	void *value;

	bValue = AG_GetVariable(num, "value", &value);
	switch (AG_VARIABLE_TYPE(bValue)) {
	case AG_VARIABLE_FLOAT:		return *(float *)value;
	case AG_VARIABLE_DOUBLE:	return (float)(*(double *)value);
#ifdef HAVE_LONG_DOUBLE
	case AG_VARIABLE_LONG_DOUBLE:	return (float)(*(long double *)value);
#endif
	case AG_VARIABLE_INT:		return (float)(*(int *)value);
	case AG_VARIABLE_UINT:		return (float)(*(Uint *)value);
	case AG_VARIABLE_UINT8:		return (float)(*(Uint8 *)value);
	case AG_VARIABLE_UINT16:	return (float)(*(Uint16 *)value);
	case AG_VARIABLE_UINT32:	return (float)(*(Uint32 *)value);
	case AG_VARIABLE_SINT8:		return (float)(*(Sint8 *)value);
	case AG_VARIABLE_SINT16:	return (float)(*(Sint16 *)value);
	case AG_VARIABLE_SINT32:	return (float)(*(Sint32 *)value);
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	return (float)(*(Uint64 *)value);
	case AG_VARIABLE_SINT64:	return (float)(*(Sint64 *)value);
#endif
	default:			return (0.0f);
	}
}

/* Convert the bound value to a double. */
double
AG_NumericalGetDbl(AG_Numerical *num)
{
	AG_Variable *bValue;
	void *value;

	bValue = AG_GetVariable(num, "value", &value);
	switch (AG_VARIABLE_TYPE(bValue)) {
	case AG_VARIABLE_FLOAT:		return (double)(*(float *)value);
	case AG_VARIABLE_DOUBLE:	return *(double *)value;
#ifdef HAVE_LONG_DOUBLE
	case AG_VARIABLE_LONG_DOUBLE:	return (double)(*(long double *)value);
#endif
	case AG_VARIABLE_INT:		return (double)(*(int *)value);
	case AG_VARIABLE_UINT:		return (double)(*(Uint *)value);
	case AG_VARIABLE_UINT8:		return (double)(*(Uint8 *)value);
	case AG_VARIABLE_UINT16:	return (double)(*(Uint16 *)value);
	case AG_VARIABLE_UINT32:	return (double)(*(Uint32 *)value);
	case AG_VARIABLE_SINT8:		return (double)(*(Sint8 *)value);
	case AG_VARIABLE_SINT16:	return (double)(*(Sint16 *)value);
	case AG_VARIABLE_SINT32:	return (double)(*(Sint32 *)value);
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	return (double)(*(Uint64 *)value);
	case AG_VARIABLE_SINT64:	return (double)(*(Sint64 *)value);
#endif
	default:			return (0.0);
	}
}

#ifdef HAVE_LONG_DOUBLE
/* Convert the bound value to a long double. */
long double
AG_NumericalGetLdbl(AG_Numerical *num)
{
	AG_Variable *bValue;
	void *value;

	bValue = AG_GetVariable(num, "value", &value);
	switch (AG_VARIABLE_TYPE(bValue)) {
	case AG_VARIABLE_FLOAT:		return (long double)(*(float *)value);
	case AG_VARIABLE_DOUBLE:	return (long double)(*(double *)value);
	case AG_VARIABLE_LONG_DOUBLE:	return *(long double *)value;
	case AG_VARIABLE_INT:		return (long double)(*(int *)value);
	case AG_VARIABLE_UINT:		return (long double)(*(Uint *)value);
	case AG_VARIABLE_UINT8:		return (long double)(*(Uint8 *)value);
	case AG_VARIABLE_UINT16:	return (long double)(*(Uint16 *)value);
	case AG_VARIABLE_UINT32:	return (long double)(*(Uint32 *)value);
	case AG_VARIABLE_SINT8:		return (long double)(*(Sint8 *)value);
	case AG_VARIABLE_SINT16:	return (long double)(*(Sint16 *)value);
	case AG_VARIABLE_SINT32:	return (long double)(*(Sint32 *)value);
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	return (long double)(*(Uint64 *)value);
	case AG_VARIABLE_SINT64:	return (long double)(*(Sint64 *)value);
#endif
	default:			return (0.0L);
	}
}
#endif /* HAVE_LONG_DOUBLE */

/* Convert the bound value to a natural integer. */
int
AG_NumericalGetInt(AG_Numerical *num)
{
	AG_Variable *bValue;
	void *value;

	bValue = AG_GetVariable(num, "value", &value);
	switch (AG_VARIABLE_TYPE(bValue)) {
	case AG_VARIABLE_FLOAT:		return (int)(*(float *)value);
	case AG_VARIABLE_DOUBLE:	return (int)(*(double *)value);
#ifdef HAVE_LONG_DOUBLE
	case AG_VARIABLE_LONG_DOUBLE:	return (int)(*(long double *)value);
#endif
	case AG_VARIABLE_INT:		return *(int *)value;
	case AG_VARIABLE_UINT:		return (int)(*(Uint *)value);
	case AG_VARIABLE_UINT8:		return (int)(*(Uint8 *)value);
	case AG_VARIABLE_UINT16:	return (int)(*(Uint16 *)value);
	case AG_VARIABLE_UINT32:	return (int)(*(Uint32 *)value);
	case AG_VARIABLE_SINT8:		return (int)(*(Sint8 *)value);
	case AG_VARIABLE_SINT16:	return (int)(*(Sint16 *)value);
	case AG_VARIABLE_SINT32:	return (int)(*(Sint32 *)value);
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	return (int)(*(Uint64 *)value);
	case AG_VARIABLE_SINT64:	return (int)(*(Sint64 *)value);
#endif
	default:			return (0);
	}
}

/* Convert the bound value to a 32-bit integer. */
Uint32
AG_NumericalGetUint32(AG_Numerical *num)
{
	AG_Variable *bValue;
	void *value;

	bValue = AG_GetVariable(num, "value", &value);
	switch (AG_VARIABLE_TYPE(bValue)) {
	case AG_VARIABLE_FLOAT:		return (Uint32)(*(float *)value);
	case AG_VARIABLE_DOUBLE:	return (Uint32)(*(double *)value);
#ifdef HAVE_LONG_DOUBLE
	case AG_VARIABLE_LONG_DOUBLE:	return (Uint32)(*(long double *)value);
#endif
	case AG_VARIABLE_INT:		return (Uint32)(*(int *)value);
	case AG_VARIABLE_UINT:		return (Uint32)(*(Uint *)value);
	case AG_VARIABLE_UINT8:		return (Uint32)(*(Uint8 *)value);
	case AG_VARIABLE_UINT16:	return (Uint32)(*(Uint16 *)value);
	case AG_VARIABLE_UINT32:	return *(Uint32 *)value;
	case AG_VARIABLE_SINT8:		return (Uint32)(*(Sint8 *)value);
	case AG_VARIABLE_SINT16:	return (Uint32)(*(Sint16 *)value);
	case AG_VARIABLE_SINT32:	return (Uint32)(*(Sint32 *)value);
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	return (Uint32)(*(Uint64 *)value);
	case AG_VARIABLE_SINT64:	return (Uint32)(*(Sint64 *)value);
#endif
	default:			return (0UL);
	}
}

#ifdef HAVE_64BIT
/* Convert the bound value to a 64-bit integer. */
Uint64
AG_NumericalGetUint64(AG_Numerical *num)
{
	AG_Variable *bValue;
	void *value;

	bValue = AG_GetVariable(num, "value", &value);
	switch (AG_VARIABLE_TYPE(bValue)) {
	case AG_VARIABLE_FLOAT:		return (Uint64)(*(float *)value);
	case AG_VARIABLE_DOUBLE:	return (Uint64)(*(double *)value);
#ifdef HAVE_LONG_DOUBLE
	case AG_VARIABLE_LONG_DOUBLE:	return (Uint64)(*(long double *)value);
#endif
	case AG_VARIABLE_INT:		return (Uint64)(*(int *)value);
	case AG_VARIABLE_UINT:		return (Uint64)(*(Uint *)value);
	case AG_VARIABLE_UINT8:		return (Uint64)(*(Uint8 *)value);
	case AG_VARIABLE_UINT16:	return (Uint64)(*(Uint16 *)value);
	case AG_VARIABLE_UINT32:	return (Uint64)(*(Uint32 *)value);
	case AG_VARIABLE_UINT64:	return *(Uint64 *)value;
	case AG_VARIABLE_SINT8:		return (Uint64)(*(Sint8 *)value);
	case AG_VARIABLE_SINT16:	return (Uint64)(*(Sint16 *)value);
	case AG_VARIABLE_SINT32:	return (Uint64)(*(Sint32 *)value);
	case AG_VARIABLE_SINT64:	return (Uint64)(*(Sint64 *)value);
	default:			return (0ULL);
	}
}
#endif /* HAVE_64BIT */

#ifdef AG_LEGACY
void
AG_NumericalSetRangeDbl(AG_Numerical *num, double min, double max)
{ }
void
AG_NumericalSetRangeInt(AG_Numerical *num, int min, int max)
{ }
void
AG_NumericalSetRangeFlt(AG_Numerical *num, float min, float max)
{ }
#endif /* AG_LEGACY */

AG_WidgetClass agNumericalClass = {
	{
		"Agar(Widget:Numerical)",
		sizeof(AG_Numerical),
		{ 0,0 },
		Init,
		NULL,			/* free */
		NULL,			/* destroy */
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

