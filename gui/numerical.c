/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <compat/limits.h>
#include <config/have_strtoll.h>

#include <core/core.h>

#include "numerical.h"
#include "primitive.h"

#include <string.h>

AG_Numerical *
AG_NumericalNew(void *parent, Uint flags, const char *unit, const char *label)
{
	AG_Numerical *num;

	num = Malloc(sizeof(AG_Numerical), M_OBJECT);
	AG_NumericalInit(num, flags, unit, label);
	AG_ObjectAttach(parent, num);
	return (num);
}

/* Adjust the default range depending on the data type of a new binding. */
static void
AdjustRangeForBinding(AG_Event *event)
{
	AG_Numerical *num = AG_SELF();
	AG_WidgetBinding *binding = AG_PTR(1);

	if (strcmp(binding->name, "value") == 0) {
		AG_MutexLock(&num->lock);
		switch (binding->vtype) {
		case AG_WIDGET_DOUBLE:
			num->min = -AG_DBL_MAX+1;
			num->max =  AG_DBL_MAX-1;
			num->input->flags |= AG_TEXTBOX_FLT_ONLY;
			num->input->flags &= ~AG_TEXTBOX_INT_ONLY;
			break;
		case AG_WIDGET_FLOAT:
			num->min = -AG_FLT_MAX+1;
			num->max =  AG_FLT_MAX-1;
			num->input->flags |= AG_TEXTBOX_FLT_ONLY;
			num->input->flags &= ~AG_TEXTBOX_INT_ONLY;
			break;
		case AG_WIDGET_INT:
			num->min = AG_INT_MIN+1;
			num->max = AG_INT_MAX-1;
			num->input->flags |= AG_TEXTBOX_INT_ONLY;
			num->input->flags &= ~AG_TEXTBOX_FLT_ONLY;
			break;
		case AG_WIDGET_UINT:
			num->min = 0;
			num->max = AG_UINT_MAX-1;
			num->input->flags |= AG_TEXTBOX_INT_ONLY;
			num->input->flags &= ~AG_TEXTBOX_FLT_ONLY;
			break;
		case AG_WIDGET_UINT8:
			num->min = 0;
			num->max = 0xffU;
			num->input->flags |= AG_TEXTBOX_INT_ONLY;
			num->input->flags &= ~AG_TEXTBOX_FLT_ONLY;
			break;
		case AG_WIDGET_SINT8:
			num->min = -0x7f+1;
			num->max =  0x7f-1;
			num->input->flags |= AG_TEXTBOX_INT_ONLY;
			num->input->flags &= ~AG_TEXTBOX_FLT_ONLY;
			break;
		case AG_WIDGET_UINT16:
			num->min = 0;
			num->max = 0xffffU;
			num->input->flags |= AG_TEXTBOX_INT_ONLY;
			num->input->flags &= ~AG_TEXTBOX_FLT_ONLY;
			break;
		case AG_WIDGET_SINT16:
			num->min = -0x7fff+1;
			num->max =  0x7fff-1;
			num->input->flags |= AG_TEXTBOX_INT_ONLY;
			num->input->flags &= ~AG_TEXTBOX_FLT_ONLY;
			break;
		case AG_WIDGET_UINT32:
			num->min = 0;
			num->max = 0xffffffffU;
			num->input->flags |= AG_TEXTBOX_INT_ONLY;
			num->input->flags &= ~AG_TEXTBOX_FLT_ONLY;
			break;
		case AG_WIDGET_SINT32:
			num->min = -0x7fffffff+1;
			num->max =  0x7fffffff-1;
			num->input->flags |= AG_TEXTBOX_INT_ONLY;
			num->input->flags &= ~AG_TEXTBOX_FLT_ONLY;
			break;
#ifdef HAVE_64BIT
		case AG_WIDGET_UINT64:
			num->min = 0;
			num->max = 0xffffffffffffffffULL;
			num->input->flags |= AG_TEXTBOX_INT_ONLY;
			num->input->flags &= ~AG_TEXTBOX_FLT_ONLY;
			break;
		case AG_WIDGET_SINT64:
			num->min = -0x7fffffffffffffffULL+1;
			num->max =  0x7fffffffffffffffULL-1;
			num->input->flags |= AG_TEXTBOX_INT_ONLY;
			num->input->flags &= ~AG_TEXTBOX_FLT_ONLY;
			break;
#endif
		}
		AG_MutexUnlock(&num->lock);
	}
}

/* Update the textbox contents from the binding value. */
static void
UpdateTextbox(AG_Numerical *num)
{
	AG_WidgetBinding *valueb;
	void *value;

	valueb = AG_WidgetGetBinding(num, "value", &value);
	switch (valueb->vtype) {
	case AG_WIDGET_DOUBLE:
		AG_TextboxPrintf(num->input, num->format,
		    AG_Base2Unit(*(double *)value, num->unit));
		break;
	case AG_WIDGET_FLOAT:
		AG_TextboxPrintf(num->input, num->format,
		    AG_Base2Unit(*(float *)value, num->unit));
		break;
	case AG_WIDGET_INT:
		AG_TextboxPrintf(num->input, "%d", *(int *)value);
		break;
	case AG_WIDGET_UINT:
		AG_TextboxPrintf(num->input, "%u", *(Uint *)value);
		break;
	case AG_WIDGET_UINT8:
		AG_TextboxPrintf(num->input, "%u", *(Uint8 *)value);
		break;
	case AG_WIDGET_SINT8:
		AG_TextboxPrintf(num->input, "%d", *(Sint8 *)value);
		break;
	case AG_WIDGET_UINT16:
		AG_TextboxPrintf(num->input, "%u", *(Uint16 *)value);
		break;
	case AG_WIDGET_SINT16:
		AG_TextboxPrintf(num->input, "%d", *(Sint16 *)value);
		break;
	case AG_WIDGET_UINT32:
		AG_TextboxPrintf(num->input, "%u", *(Uint32 *)value);
		break;
	case AG_WIDGET_SINT32:
		AG_TextboxPrintf(num->input, "%d", *(Sint32 *)value);
		break;
#ifdef HAVE_64BIT
	case AG_WIDGET_UINT64:
		AG_TextboxPrintf(num->input, "%lld", *(Uint64 *)value);
		break;
	case AG_WIDGET_SINT64:
		AG_TextboxPrintf(num->input, "%lld", *(Sint64 *)value);
		break;
#endif
	}
	AG_WidgetUnlockBinding(valueb);
}

static void
keydown(AG_Event *event)
{
	AG_Numerical *num = AG_SELF();
	int keysym = AG_INT(1);

	AG_MutexLock(&num->lock);
	switch (keysym) {
	case SDLK_UP:
		AG_NumericalAddValue(num, num->inc);
		break;
	case SDLK_DOWN:
		AG_NumericalAddValue(num, -num->inc);
		break;
	}
	AG_MutexUnlock(&num->lock);
}

static void
GainedFocus(AG_Event *event)
{
	AG_Numerical *num = AG_SELF();

	UpdateTextbox(num);
	AG_WidgetFocus(num->input);
}

/* Update the numerical value from the textbox. */
static void
UpdateFromText(AG_Event *event)
{
	AG_Numerical *num = AG_PTR(1);
	int unfocus = AG_INT(2);
	AG_WidgetBinding *stringb, *valueb;
	char *s;
	void *value;

	valueb = AG_WidgetGetBinding(num, "value", &value);
	stringb = AG_WidgetGetBinding(num->input, "string", &s);

	switch (valueb->vtype) {
	case AG_WIDGET_DOUBLE:
	case AG_WIDGET_FLOAT:
		AG_NumericalSetValue(num,
		    AG_Unit2Base(strtod(s, NULL), num->unit));
		break;
	case AG_WIDGET_INT:
	case AG_WIDGET_UINT:
	case AG_WIDGET_UINT8:
	case AG_WIDGET_SINT8:
	case AG_WIDGET_UINT16:
	case AG_WIDGET_SINT16:
	case AG_WIDGET_UINT32:
	case AG_WIDGET_SINT32:
		AG_NumericalSetValue(num, (double)strtol(s, NULL, 10));
		break;
#if defined(HAVE_64BIT) && defined(HAVE_STRTOLL)
	case AG_WIDGET_UINT64:
	case AG_WIDGET_SINT64:
		AG_NumericalSetValue(num, (double)strtoll(s, NULL, 10));
		break;
#endif
	}

	AG_WidgetUnlockBinding(stringb);
	AG_WidgetUnlockBinding(valueb);

	if (unfocus) {
		AG_WidgetUnfocus(num->input);
	}
	AG_PostEvent(NULL, num, "numerical-return", NULL);
}

static void
IncrementValue(AG_Event *event)
{
	AG_Numerical *num = AG_PTR(1);

	AG_MutexLock(&num->lock);
	AG_NumericalAddValue(num, num->inc);
	AG_MutexUnlock(&num->lock);
}

static void
DecrementValue(AG_Event *event)
{
	AG_Numerical *num = AG_PTR(1);
	
	AG_MutexLock(&num->lock);
	AG_NumericalAddValue(num, -num->inc);
	AG_MutexUnlock(&num->lock);
}

static void
UpdateUnitSelector(AG_Numerical *num)
{
	AG_ButtonText(num->units->button, "%s", AG_UnitAbbr(num->unit));
}

static void
SelectUnit(AG_Event *event)
{
	AG_Numerical *num = AG_PTR(1);
	AG_TlistItem *ti = AG_PTR(2);

	num->unit = (const AG_Unit *)ti->p1;
	UpdateUnitSelector(num);
	UpdateTextbox(num);
}

void
AG_NumericalSetUnitSystem(AG_Numerical *num, const char *unit_key)
{
	const AG_Unit *unit = NULL;
	const AG_Unit *ugroup = NULL;
	int found = 0, i;
	int w, h, nUnits = 0;
	AG_Window *pWin;

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
		fatal("unknown unit: `%s'", unit_key);
	}
	num->unit = unit;
	UpdateUnitSelector(num);

	num->wUnitSel = 0;
	num->hUnitSel = 0;
	num->wPreUnit = 0;

	AG_MutexLock(&num->units->list->lock);
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
	AG_MutexUnlock(&num->units->list->lock);

	if (num->wPreUnit > 0) { num->wPreUnit += 8; }
	AG_UComboSizeHintPixels(num->units, num->wPreUnit,
	    nUnits<6 ? (nUnits + 1) : 6);
	
	if ((pWin = AG_WidgetParentWindow(num)))
		AG_WindowUpdate(pWin);
}

void
AG_NumericalInit(AG_Numerical *num, Uint flags, const char *unit,
    const char *label)
{
	Uint wflags = 0;

	if (flags & AG_NUMERICAL_HFILL) { wflags |= AG_WIDGET_HFILL; }
	if (flags & AG_NUMERICAL_VFILL) { wflags |= AG_WIDGET_VFILL; }

	AG_WidgetInit(num, &agNumericalOps, wflags);
	AG_WidgetBindDouble(num, "value", &num->value);
	AG_WidgetBindDouble(num, "min", &num->min);
	AG_WidgetBindDouble(num, "max", &num->max);
	
	num->inc = 1.0;
	num->value = 0.0;
	num->input = AG_TextboxNew(num, 0, label);
	num->writeable = 1;
	num->wUnitSel = 0;
	num->hUnitSel = 0;
	strlcpy(num->format, "%g", sizeof(num->format));
	AG_MutexInitRecursive(&num->lock);
	AG_TextboxSizeHint(num->input, "88.88");
	
	if (unit != NULL) {
		num->units = AG_UComboNew(num, 0);
		AG_SetEvent(num->units, "ucombo-selected", SelectUnit,
		    "%p", num);
		AG_NumericalSetUnitSystem(num, unit);
		AG_WidgetSetFocusable(num->units, 0);
	} else {
		num->unit = AG_FindUnit("identity");
		num->units = NULL;
	}

	num->incbu = AG_ButtonNew(num, AG_BUTTON_REPEAT, _("+"));
	AG_ButtonSetPadding(num->incbu, 1,1,1,1);
	AG_WidgetSetFocusable(num->incbu, 0);

	num->decbu = AG_ButtonNew(num, AG_BUTTON_REPEAT, _("-"));
	AG_ButtonSetPadding(num->decbu, 1,1,1,1);
	AG_WidgetSetFocusable(num->decbu, 0);

	AG_SetEvent(num, "window-keydown", keydown, NULL);
	AG_SetEvent(num, "widget-gainfocus", GainedFocus, NULL);
	AG_SetEvent(num, "widget-bound", AdjustRangeForBinding, NULL);
	AG_SetEvent(num->input, "textbox-return", UpdateFromText, "%p,%i",
	    num, 1);
	AG_SetEvent(num->input, "textbox-changed", UpdateFromText, "%p,%i",
	    num, 0);
	AG_SetEvent(num->incbu, "button-pushed", IncrementValue, "%p", num);
	AG_SetEvent(num->decbu, "button-pushed", DecrementValue, "%p", num);
}

void
AG_NumericalSizeHint(AG_Numerical *num, const char *text)
{
	AG_TextboxSizeHint(num->input, text);
}

static void
Destroy(void *p)
{
	AG_Numerical *num = p;

	AG_MutexDestroy(&num->lock);
	AG_WidgetDestroy(num);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Numerical *num = p;
	AG_SizeReq rChld, rInc, rDec;

	AG_WidgetSizeReq(num->input, &rChld);
	r->w = rChld.w + num->wUnitSel + 4;
	r->h = MAX(rChld.h, num->hUnitSel);

	AG_WidgetSizeReq(num->incbu, &rInc);
	AG_WidgetSizeReq(num->decbu, &rDec);
	r->w += MAX(rInc.w, rDec.w) + 4;
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Numerical *num = p;
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
Draw(void *p)
{
	AG_Numerical *num = p;

	if (!AG_WidgetFocused(num->input))
		UpdateTextbox(num);
}

#define ADD_CONVERTED(TYPE) do { \
	n = (double)(*(TYPE *)value); \
	if ((n+inc) < *min) { n = *min; } \
	else if ((n+inc) > *max) { n = *(max); } \
	else { n += inc; } \
	*(TYPE *)value = (TYPE)n; \
} while (0)
#define ADD_REAL(TYPE) do { \
	n = AG_Base2Unit(*(TYPE *)value, num->unit); \
	if ((n+inc) < *min) { n = *min; } \
	else if ((n+inc) > *max) { n = *(max); } \
	else { n += inc; } \
	*(TYPE *)value = AG_Unit2Base(n, num->unit); \
} while (0)
void
AG_NumericalAddValue(AG_Numerical *num, double inc)
{
	AG_WidgetBinding *valueb, *minb, *maxb;
	void *value;
	double n;
	double *min, *max;

	valueb = AG_WidgetGetBinding(num, "value", &value);
	minb = AG_WidgetGetBinding(num, "min", &min);
	maxb = AG_WidgetGetBinding(num, "max", &max);

	switch (valueb->vtype) {
	case AG_WIDGET_DOUBLE:	ADD_REAL(double);	break;
	case AG_WIDGET_FLOAT:	ADD_REAL(float);	break;
	case AG_WIDGET_INT:	ADD_CONVERTED(int);	break;
	case AG_WIDGET_UINT:	ADD_CONVERTED(Uint);	break;
	case AG_WIDGET_UINT8:	ADD_CONVERTED(Uint8);	break;
	case AG_WIDGET_SINT8:	ADD_CONVERTED(Sint8);	break;
	case AG_WIDGET_UINT16:	ADD_CONVERTED(Uint16);	break;
	case AG_WIDGET_SINT16:	ADD_CONVERTED(Sint16);	break;
	case AG_WIDGET_UINT32:	ADD_CONVERTED(Uint32);	break;
	case AG_WIDGET_SINT32:	ADD_CONVERTED(Sint32);	break;
#ifdef HAVE_64BIT
	case AG_WIDGET_UINT64:	ADD_CONVERTED(Uint64);	break;
	case AG_WIDGET_SINT64:	ADD_CONVERTED(Sint64);	break;
#endif
	}
	AG_PostEvent(NULL, num, "numerical-changed", NULL);
	AG_WidgetBindingChanged(valueb);

	AG_WidgetUnlockBinding(valueb);
	AG_WidgetUnlockBinding(minb);
	AG_WidgetUnlockBinding(maxb);

	UpdateTextbox(num);
}
#undef ADD_REAL
#undef ADD_CONVERTED

#define ASSIGN_VALUE(TYPE) *(TYPE *)value = nvalue < *min ? *min : \
    nvalue > *max ? *max : nvalue
#define CONV_VALUE(TYPE) \
    *(TYPE *)value = nvalue < *min ? (TYPE)(*min) : \
     nvalue > *max ? (TYPE)(*max) : (TYPE)nvalue

/* TODO int types directly */
void
AG_NumericalSetValue(AG_Numerical *num, double nvalue)
{
	AG_WidgetBinding *valueb, *minb, *maxb;
	void *value;
	double *min, *max;

	valueb = AG_WidgetGetBinding(num, "value", &value);
	minb = AG_WidgetGetBinding(num, "min", &min);
	maxb = AG_WidgetGetBinding(num, "max", &max);

	switch (valueb->vtype) {
	case AG_WIDGET_DOUBLE:	ASSIGN_VALUE(double);	break;
	case AG_WIDGET_FLOAT:	ASSIGN_VALUE(float);	break;
	case AG_WIDGET_INT:	CONV_VALUE(int);	break;
	case AG_WIDGET_UINT:	CONV_VALUE(Uint);	break;
	case AG_WIDGET_UINT8:	CONV_VALUE(Uint8);	break;
	case AG_WIDGET_SINT8:	CONV_VALUE(Sint8);	break;
	case AG_WIDGET_UINT16:	CONV_VALUE(Uint16);	break;
	case AG_WIDGET_SINT16:	CONV_VALUE(Sint16);	break;
	case AG_WIDGET_UINT32:	CONV_VALUE(Uint32);	break;
	case AG_WIDGET_SINT32:	CONV_VALUE(Sint32);	break;
#ifdef HAVE_64BIT
	case AG_WIDGET_UINT64:	CONV_VALUE(Uint64);	break;
	case AG_WIDGET_SINT64:	CONV_VALUE(Sint64);	break;
#endif
	}

	AG_PostEvent(NULL, num, "numerical-changed", NULL);
	AG_WidgetBindingChanged(valueb);

	AG_WidgetUnlockBinding(valueb);
	AG_WidgetUnlockBinding(minb);
	AG_WidgetUnlockBinding(maxb);
}
#undef ASSIGN_VALUE
#undef CONV_VALUE

void
AG_NumericalSetMin(AG_Numerical *num, double nmin)
{
	AG_WidgetBinding *minb;
	void *min;

	/* TODO allow integer min/max bindings */
	minb = AG_WidgetGetBinding(num, "min", &min);
	switch (minb->vtype) {
	case AG_WIDGET_DOUBLE:
		*(double *)min = nmin;
		break;
	case AG_WIDGET_FLOAT:
		*(float *)min = (float)nmin;
		break;
	}
	AG_WidgetUnlockBinding(minb);
}

void
AG_NumericalSetMax(AG_Numerical *num, double nmax)
{
	AG_WidgetBinding *maxb;
	void *max;
	
	/* TODO allow integer min/max bindings */
	maxb = AG_WidgetGetBinding(num, "max", &max);
	switch (maxb->vtype) {
	case AG_WIDGET_DOUBLE:
		*(double *)max = nmax;
		break;
	case AG_WIDGET_FLOAT:
		*(float *)max = (float)nmax;
		break;
	}
	AG_WidgetUnlockBinding(maxb);
}

void
AG_NumericalSetIncrement(AG_Numerical *num, double inc)
{
	AG_MutexLock(&num->lock);
	num->inc = inc;
	AG_MutexUnlock(&num->lock);
}

void
AG_NumericalSetPrecision(AG_Numerical *num, const char *mode,
    int precision)
{
	AG_MutexLock(&num->lock);
	num->format[0] = '%';
	num->format[1] = '.';
	snprintf(&num->format[2], sizeof(num->format)-2, "%d", precision);
	strlcat(num->format, mode, sizeof(num->format));
	AG_MutexUnlock(&num->lock);
}

void
AG_NumericalSelectUnit(AG_Numerical *num, const char *uname)
{
	AG_TlistItem *it;

	AG_MutexLock(&num->units->list->lock);
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
	AG_MutexUnlock(&num->units->list->lock);
}

void
AG_NumericalSetWriteable(AG_Numerical *num, int writeable)
{
	AG_MutexLock(&num->lock);
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
	AG_MutexUnlock(&num->lock);
}

void
AG_NumericalSetRange(AG_Numerical *num, double min, double max)
{
	AG_MutexLock(&num->lock);
	AG_NumericalSetMin(num, min);
	AG_NumericalSetMax(num, max);
	AG_MutexUnlock(&num->lock);
}

/* Convert the bound value to a float and return it. */
float
AG_NumericalGetFloat(AG_Numerical *num)
{
	AG_WidgetBinding *bValue;
	void *value;

	bValue = AG_WidgetGetBinding(num, "value", &value);
	switch (bValue->vtype) {
	case AG_WIDGET_FLOAT:	return *(float *)value;
	case AG_WIDGET_DOUBLE:	return  (float)(*(double *)value);
	case AG_WIDGET_INT:	return  (float)(*(int *)value);
	case AG_WIDGET_UINT:	return  (float)(*(Uint *)value);
	case AG_WIDGET_UINT8:	return  (float)(*(Uint8 *)value);
	case AG_WIDGET_UINT16:	return  (float)(*(Uint16 *)value);
	case AG_WIDGET_UINT32:	return  (float)(*(Uint32 *)value);
	case AG_WIDGET_SINT8:	return  (float)(*(Sint8 *)value);
	case AG_WIDGET_SINT16:	return  (float)(*(Sint16 *)value);
	case AG_WIDGET_SINT32:	return  (float)(*(Sint32 *)value);
#ifdef HAVE_64BIT
	case AG_WIDGET_UINT64:	return  (float)(*(Uint64 *)value);
	case AG_WIDGET_SINT64:	return  (float)(*(Sint64 *)value);
#endif
	default:		return (0.0);
	}
}

/* Convert the bound value to a double and return it. */
double
AG_NumericalGetDouble(AG_Numerical *num)
{
	AG_WidgetBinding *bValue;
	void *value;

	bValue = AG_WidgetGetBinding(num, "value", &value);
	switch (bValue->vtype) {
	case AG_WIDGET_DOUBLE:	return *(double *)value;
	case AG_WIDGET_FLOAT:	return  (double)(*(float *)value);
	case AG_WIDGET_INT:	return  (double)(*(int *)value);
	case AG_WIDGET_UINT:	return  (double)(*(Uint *)value);
	case AG_WIDGET_UINT8:	return  (double)(*(Uint8 *)value);
	case AG_WIDGET_UINT16:	return  (double)(*(Uint16 *)value);
	case AG_WIDGET_UINT32:	return  (double)(*(Uint32 *)value);
	case AG_WIDGET_SINT8:	return  (double)(*(Sint8 *)value);
	case AG_WIDGET_SINT16:	return  (double)(*(Sint16 *)value);
	case AG_WIDGET_SINT32:	return  (double)(*(Sint32 *)value);
#ifdef HAVE_64BIT
	case AG_WIDGET_UINT64:	return  (double)(*(Uint64 *)value);
	case AG_WIDGET_SINT64:	return  (double)(*(Sint64 *)value);
#endif
	default:		return (0.0);
	}
}

/* Convert the bound value to a natural integer and return it. */
int
AG_NumericalGetInt(AG_Numerical *num)
{
	AG_WidgetBinding *bValue;
	void *value;

	bValue = AG_WidgetGetBinding(num, "value", &value);
	switch (bValue->vtype) {
	case AG_WIDGET_INT:	return *(int *)value;
	case AG_WIDGET_UINT:	return (int)(*(Uint *)value);
	case AG_WIDGET_UINT8:	return (int)(*(Uint8 *)value);
	case AG_WIDGET_UINT16:	return (int)(*(Uint16 *)value);
	case AG_WIDGET_UINT32:	return (int)(*(Uint32 *)value);
	case AG_WIDGET_SINT8:	return (int)(*(Sint8 *)value);
	case AG_WIDGET_SINT16:	return (int)(*(Sint16 *)value);
	case AG_WIDGET_SINT32:	return (int)(*(Sint32 *)value);
#ifdef HAVE_64BIT
	case AG_WIDGET_UINT64:	return (int)(*(Uint64 *)value);
	case AG_WIDGET_SINT64:	return (int)(*(Sint64 *)value);
#endif
	case AG_WIDGET_DOUBLE:	return (int)(*(double *)value);
	case AG_WIDGET_FLOAT:	return (int)(*(float *)value);
	default:		return (0);
	}
}

/* Convert the bound value to a 32-bit integer and return it. */
Uint32
AG_NumericalGetUint32(AG_Numerical *num)
{
	AG_WidgetBinding *bValue;
	void *value;

	bValue = AG_WidgetGetBinding(num, "value", &value);
	switch (bValue->vtype) {
	case AG_WIDGET_INT:	return (Uint32)(*(int *)value);
	case AG_WIDGET_UINT:	return (Uint32)(*(Uint *)value);
	case AG_WIDGET_UINT8:	return (Uint32)(*(Uint8 *)value);
	case AG_WIDGET_UINT16:	return (Uint32)(*(Uint16 *)value);
	case AG_WIDGET_UINT32:	return *(Uint32 *)value;
	case AG_WIDGET_SINT8:	return (Uint32)(*(Sint8 *)value);
	case AG_WIDGET_SINT16:	return (Uint32)(*(Sint16 *)value);
	case AG_WIDGET_SINT32:	return (Uint32)(*(Sint32 *)value);
#ifdef HAVE_64BIT
	case AG_WIDGET_UINT64:	return (Uint32)(*(Uint64 *)value);
	case AG_WIDGET_SINT64:	return (Uint32)(*(Sint64 *)value);
#endif
	case AG_WIDGET_DOUBLE:	return (Uint32)(*(double *)value);
	case AG_WIDGET_FLOAT:	return (Uint32)(*(float *)value);
	default:		return (0);
	}
}

#ifdef HAVE_64BIT
/* Convert the bound value to a 64-bit integer and return it. */
Uint64
AG_NumericalGetUint64(AG_Numerical *num)
{
	AG_WidgetBinding *bValue;
	void *value;

	bValue = AG_WidgetGetBinding(num, "value", &value);
	switch (bValue->vtype) {
#ifdef HAVE_64BIT
	case AG_WIDGET_UINT64:	return *(Uint64 *)value;
	case AG_WIDGET_SINT64:	return (Uint64)(*(Sint64 *)value);
#endif
	case AG_WIDGET_INT:	return (Uint64)(*(int *)value);
	case AG_WIDGET_UINT:	return (Uint64)(*(Uint *)value);
	case AG_WIDGET_UINT8:	return (Uint64)(*(Uint8 *)value);
	case AG_WIDGET_UINT16:	return (Uint64)(*(Uint16 *)value);
	case AG_WIDGET_UINT32:	return (Uint64)(*(Uint32 *)value);
	case AG_WIDGET_SINT8:	return (Uint64)(*(Sint8 *)value);
	case AG_WIDGET_SINT16:	return (Uint64)(*(Sint16 *)value);
	case AG_WIDGET_SINT32:	return (Uint64)(*(Sint32 *)value);
	case AG_WIDGET_DOUBLE:	return (Uint64)(*(double *)value);
	case AG_WIDGET_FLOAT:	return (Uint64)(*(float *)value);
	default:		return (0);
	}
}
#endif /* HAVE_64BIT */

const AG_WidgetOps agNumericalOps = {
	{
		"AG_Widget:AG_Numerical",
		sizeof(AG_Numerical),
		{ 0,0 },
		NULL,			/* init */
		NULL,			/* reinit */
		Destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

