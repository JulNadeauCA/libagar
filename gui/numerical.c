/*
 * Copyright (c) 2007-2010 Hypertriton, Inc. <http://hypertriton.com/>
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
AG_NumericalNewDbl(void *parent, Uint flags, const char *unit,
    const char *label, double *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindDouble(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewDblR(void *parent, Uint flags, const char *unit,
    const char *label, double *v, double min, double max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindDouble(num, "value", v);
	AG_NumericalSetMinDbl(num, min);
	AG_NumericalSetMaxDbl(num, max);
	return (num);
}

AG_Numerical *
AG_NumericalNewFlt(void *parent, Uint flags, const char *unit,
    const char *label, float *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindFloat(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewFltR(void *parent, Uint flags, const char *unit,
    const char *label, float *v, float min, float max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindFloat(num, "value", v);
	AG_NumericalSetMinFlt(num, min);
	AG_NumericalSetMaxFlt(num, max);
	return (num);
}

AG_Numerical *
AG_NumericalNewInt(void *parent, Uint flags, const char *unit,
    const char *label, int *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindInt(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewIntR(void *parent, Uint flags, const char *unit,
    const char *label, int *v, int min, int max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindInt(num, "value", v);
	AG_NumericalSetMin(num, (double)min);	/* XXX */
	AG_NumericalSetMax(num, (double)max);
	return (num);
}
AG_Numerical *
AG_NumericalNewUint(void *parent, Uint flags, const char *unit,
    const char *label, Uint *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindUint(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewUintR(void *parent, Uint flags, const char *unit,
    const char *label, Uint *v, Uint min, Uint max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindUint(num, "value", v);
	AG_NumericalSetMinInt(num, (int)min);
	AG_NumericalSetMaxInt(num, (int)max);
	return (num);
}

AG_Numerical *
AG_NumericalNewUint8(void *parent, Uint flags, const char *unit,
    const char *label, Uint8 *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindUint8(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewUint8R(void *parent, Uint flags, const char *unit,
    const char *label, Uint8 *v, Uint8 min, Uint8 max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindUint8(num, "value", v);
	AG_NumericalSetMinInt(num, (int)min);
	AG_NumericalSetMaxInt(num, (int)max);
	return (num);
}

AG_Numerical *
AG_NumericalNewSint8(void *parent, Uint flags, const char *unit,
    const char *label, Sint8 *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindSint8(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewSint8R(void *parent, Uint flags, const char *unit,
    const char *label, Sint8 *v, Sint8 min, Sint8 max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindSint8(num, "value", v);
	AG_NumericalSetMinInt(num, (int)min);
	AG_NumericalSetMaxInt(num, (int)max);
	return (num);
}

AG_Numerical *
AG_NumericalNewUint16(void *parent, Uint flags, const char *unit,
    const char *label, Uint16 *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindUint16(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewUint16R(void *parent, Uint flags, const char *unit,
    const char *label, Uint16 *v, Uint16 min, Uint16 max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindUint16(num, "value", v);
	AG_NumericalSetMinInt(num, (int)min);
	AG_NumericalSetMaxInt(num, (int)max);
	return (num);
}

AG_Numerical *
AG_NumericalNewSint16(void *parent, Uint flags, const char *unit,
    const char *label, Sint16 *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindSint16(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewSint16R(void *parent, Uint flags, const char *unit,
    const char *label, Sint16 *v, Sint16 min, Sint16 max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindSint16(num, "value", v);
	AG_NumericalSetMinInt(num, (int)min);
	AG_NumericalSetMaxInt(num, (int)max);
	return (num);
}

AG_Numerical *
AG_NumericalNewUint32(void *parent, Uint flags, const char *unit,
    const char *label, Uint32 *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindUint32(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewUint32R(void *parent, Uint flags, const char *unit,
    const char *label, Uint32 *v, Uint32 min, Uint32 max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindUint32(num, "value", v);
	AG_NumericalSetMinInt(num, (int)min);
	AG_NumericalSetMaxInt(num, (int)max);
	return (num);
}

AG_Numerical *
AG_NumericalNewSint32(void *parent, Uint flags, const char *unit,
    const char *label, Sint32 *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindSint32(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewSint32R(void *parent, Uint flags, const char *unit,
    const char *label, Sint32 *v, Sint32 min, Sint32 max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindSint32(num, "value", v);
	AG_NumericalSetMinInt(num, (int)min);
	AG_NumericalSetMaxInt(num, (int)max);
	return (num);
}

static void
Bound(AG_Event *event)
{
	AG_Numerical *num = AG_SELF();
	AG_Variable *binding = AG_PTR(1);

	if (strcmp(binding->name, "value") == 0) {
		switch (AG_VARIABLE_TYPE(binding)) {
		case AG_VARIABLE_DOUBLE:
			num->min = -AG_DBL_MAX+1;
			num->max =  AG_DBL_MAX-1;
			AG_TextboxSetFltOnly(num->input, 1);
			break;
		case AG_VARIABLE_FLOAT:
			num->min = -AG_FLT_MAX+1;
			num->max =  AG_FLT_MAX-1;
			AG_TextboxSetFltOnly(num->input, 1);
			break;
		case AG_VARIABLE_INT:
			num->min = AG_INT_MIN+1;
			num->max = AG_INT_MAX-1;
			AG_TextboxSetIntOnly(num->input, 1);
			break;
		case AG_VARIABLE_UINT:
			num->min = 0;
			num->max = AG_UINT_MAX-1;
			AG_TextboxSetIntOnly(num->input, 1);
			break;
		case AG_VARIABLE_UINT8:
			num->min = 0;
			num->max = 0xffU;
			AG_TextboxSetIntOnly(num->input, 1);
			break;
		case AG_VARIABLE_SINT8:
			num->min = -0x7f+1;
			num->max =  0x7f-1;
			AG_TextboxSetIntOnly(num->input, 1);
			break;
		case AG_VARIABLE_UINT16:
			num->min = 0;
			num->max = 0xffffU;
			AG_TextboxSetIntOnly(num->input, 1);
			break;
		case AG_VARIABLE_SINT16:
			num->min = -0x7fff+1;
			num->max =  0x7fff-1;
			AG_TextboxSetIntOnly(num->input, 1);
			break;
		case AG_VARIABLE_UINT32:
			num->min = 0;
			num->max = 0xffffffffU;
			AG_TextboxSetIntOnly(num->input, 1);
			break;
		case AG_VARIABLE_SINT32:
			num->min = -0x7fffffff+1;
			num->max =  0x7fffffff-1;
			AG_TextboxSetIntOnly(num->input, 1);
			break;
		default:
			break;
		}
	}
	AG_Redraw(num);
}

/* Update the textbox contents from the binding value. */
static void
UpdateTextbox(AG_Numerical *num)
{
	AG_Variable *valueb;
	void *value;

	valueb = AG_GetVariable(num, "value", &value);
	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_DOUBLE:
		AG_TextboxPrintf(num->input, num->format,
		    AG_Base2Unit(*(double *)value, num->unit));
		break;
	case AG_VARIABLE_FLOAT:
		AG_TextboxPrintf(num->input, num->format,
		    AG_Base2Unit(*(float *)value, num->unit));
		break;
	case AG_VARIABLE_INT:
		AG_TextboxPrintf(num->input, "%d", *(int *)value);
		break;
	case AG_VARIABLE_UINT:
		AG_TextboxPrintf(num->input, "%u", *(Uint *)value);
		break;
	case AG_VARIABLE_UINT8:
		AG_TextboxPrintf(num->input, "%u", *(Uint8 *)value);
		break;
	case AG_VARIABLE_SINT8:
		AG_TextboxPrintf(num->input, "%d", *(Sint8 *)value);
		break;
	case AG_VARIABLE_UINT16:
		AG_TextboxPrintf(num->input, "%u", *(Uint16 *)value);
		break;
	case AG_VARIABLE_SINT16:
		AG_TextboxPrintf(num->input, "%d", *(Sint16 *)value);
		break;
	case AG_VARIABLE_UINT32:
		AG_TextboxPrintf(num->input, "%u", *(Uint32 *)value);
		break;
	case AG_VARIABLE_SINT32:
		AG_TextboxPrintf(num->input, "%d", *(Sint32 *)value);
		break;
	default:
		break;
	}
	AG_UnlockVariable(valueb);
}

static void
KeyDown(AG_Event *event)
{
	AG_Numerical *num = AG_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case AG_KEY_UP:
		AG_NumericalAddValue(num, num->inc);
		break;
	case AG_KEY_DOWN:
		AG_NumericalAddValue(num, -num->inc);
		break;
	}
}

static void
GainedFocus(AG_Event *event)
{
	AG_Numerical *num = AG_SELF();

	UpdateTextbox(num);
}

/* Update the numerical value from the textbox. */
static void
UpdateFromText(AG_Event *event)
{
	AG_Numerical *num = AG_PTR(1);
	int unfocus = AG_INT(2);
	AG_Variable *stringb, *valueb;
	char *s;
	void *value;

	valueb = AG_GetVariable(num, "value", &value);
	stringb = AG_GetVariable(num->input->ed, "string", &s);

	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_DOUBLE:
	case AG_VARIABLE_FLOAT:
		AG_NumericalSetValue(num,
		    AG_Unit2Base(strtod(s, NULL), num->unit));
		break;
	case AG_VARIABLE_INT:
	case AG_VARIABLE_UINT:
	case AG_VARIABLE_UINT8:
	case AG_VARIABLE_SINT8:
	case AG_VARIABLE_UINT16:
	case AG_VARIABLE_SINT16:
	case AG_VARIABLE_UINT32:
	case AG_VARIABLE_SINT32:
		AG_NumericalSetValue(num, (double)strtol(s, NULL, 10));
		break;
	default:
		break;
	}

	AG_UnlockVariable(stringb);
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
	AG_NumericalAddValue(num, num->inc);
	AG_ObjectUnlock(num);
}

static void
DecrementValue(AG_Event *event)
{
	AG_Numerical *num = AG_PTR(1);
	
	AG_ObjectLock(num);
	AG_NumericalAddValue(num, -num->inc);
	AG_ObjectUnlock(num);
}

static void
UpdateUnitSelector(AG_Numerical *num)
{
	AG_ButtonTextS(num->units->button, AG_UnitAbbr(num->unit));
}

static void
UnitSelected(AG_Event *event)
{
	AG_Numerical *num = AG_PTR(1);
	AG_TlistItem *ti = AG_PTR(2);

	AG_ObjectLock(num);
	num->unit = (const AG_Unit *)ti->p1;
	UpdateUnitSelector(num);
	UpdateTextbox(num);
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

static void
Init(void *obj)
{
	AG_Numerical *num = obj;

	WIDGET(num)->flags |= AG_WIDGET_FOCUSABLE|
	                      AG_WIDGET_TABLE_EMBEDDABLE;

	num->inc = 1.0;
	num->value = 0.0;
	num->input = AG_TextboxNewS(num, AG_TEXTBOX_STATIC, NULL);
	num->writeable = 1;
	num->wUnitSel = 0;
	num->hUnitSel = 0;
	Strlcpy(num->format, "%.02f", sizeof(num->format));
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

	AG_SetEvent(num, "key-down", KeyDown, NULL);
	AG_SetEvent(num, "widget-gainfocus", GainedFocus, NULL);
	AG_SetEvent(num, "bound", Bound, NULL);
	AG_SetEvent(num->incbu, "button-pushed", IncrementValue, "%p", num);
	AG_SetEvent(num->decbu, "button-pushed", DecrementValue, "%p", num);
	AG_SetEvent(num->input, "textbox-return",
	    UpdateFromText, "%p,%i", num, 1);
	AG_SetEvent(num->input, "textbox-changed",
	    UpdateFromText, "%p,%i", num, 0);
	AG_WidgetForwardFocus(num, num->input);

	AG_BindDouble(num, "value", &num->value);
	AG_BindDouble(num, "min", &num->min);
	AG_BindDouble(num, "max", &num->max);

	AG_RedrawOnChange(num, 250, "value");

#ifdef AG_DEBUG
	AG_BindUint(num, "flags", &num->flags);
	AG_BindDouble(num, "inc", &num->inc);
	/* AG_BindString(num, "format", num->format, sizeof(num->format)); */
	AG_BindPointer(num, "unit", (void *)&num->unit);
	AG_BindInt(num, "writeable", &num->writeable);
	AG_BindInt(num, "wUnitSel", &num->wUnitSel);
	AG_BindInt(num, "hUnitSel", &num->hUnitSel);
	AG_BindInt(num, "wPreUnit", &num->wPreUnit);
#endif /* AG_DEBUG */
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

	if (!AG_WidgetIsFocused(num->input))
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
	AG_Variable *valueb, *minb, *maxb;
	void *value;
	double n;
	double *min, *max;

	AG_ObjectLock(num);
	valueb = AG_GetVariable(num, "value", &value);
	minb = AG_GetVariable(num, "min", &min);
	maxb = AG_GetVariable(num, "max", &max);

	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_DOUBLE:	ADD_REAL(double);	break;
	case AG_VARIABLE_FLOAT:		ADD_REAL(float);	break;
	case AG_VARIABLE_INT:		ADD_CONVERTED(int);	break;
	case AG_VARIABLE_UINT:		ADD_CONVERTED(Uint);	break;
	case AG_VARIABLE_UINT8:		ADD_CONVERTED(Uint8);	break;
	case AG_VARIABLE_SINT8:		ADD_CONVERTED(Sint8);	break;
	case AG_VARIABLE_UINT16:	ADD_CONVERTED(Uint16);	break;
	case AG_VARIABLE_SINT16:	ADD_CONVERTED(Sint16);	break;
	case AG_VARIABLE_UINT32:	ADD_CONVERTED(Uint32);	break;
	case AG_VARIABLE_SINT32:	ADD_CONVERTED(Sint32);	break;
	default:					break;
	}
	AG_PostEvent(NULL, num, "numerical-changed", NULL);

	AG_UnlockVariable(valueb);
	AG_UnlockVariable(minb);
	AG_UnlockVariable(maxb);

	UpdateTextbox(num);
	AG_ObjectUnlock(num);
	AG_Redraw(num);
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
	AG_Variable *valueb, *minb, *maxb;
	void *value;
	double *min, *max;

	AG_ObjectLock(num);
	valueb = AG_GetVariable(num, "value", &value);
	minb = AG_GetVariable(num, "min", &min);
	maxb = AG_GetVariable(num, "max", &max);

	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_DOUBLE:	ASSIGN_VALUE(double);	break;
	case AG_VARIABLE_FLOAT:		ASSIGN_VALUE(float);	break;
	case AG_VARIABLE_INT:		CONV_VALUE(int);	break;
	case AG_VARIABLE_UINT:		CONV_VALUE(Uint);	break;
	case AG_VARIABLE_UINT8:		CONV_VALUE(Uint8);	break;
	case AG_VARIABLE_SINT8:		CONV_VALUE(Sint8);	break;
	case AG_VARIABLE_UINT16:	CONV_VALUE(Uint16);	break;
	case AG_VARIABLE_SINT16:	CONV_VALUE(Sint16);	break;
	case AG_VARIABLE_UINT32:	CONV_VALUE(Uint32);	break;
	case AG_VARIABLE_SINT32:	CONV_VALUE(Sint32);	break;
	default:					break;
	}

	AG_PostEvent(NULL, num, "numerical-changed", NULL);

	AG_UnlockVariable(valueb);
	AG_UnlockVariable(minb);
	AG_UnlockVariable(maxb);
	AG_ObjectUnlock(num);
	AG_Redraw(num);
}
#undef ASSIGN_VALUE
#undef CONV_VALUE

void
AG_NumericalSetMin(AG_Numerical *num, double nmin)
{
	AG_Variable *minb;
	void *min;

	/* TODO allow integer min/max bindings */
	AG_ObjectLock(num);
	minb = AG_GetVariable(num, "min", &min);
	switch (AG_VARIABLE_TYPE(minb)) {
	case AG_VARIABLE_DOUBLE:
		*(double *)min = nmin;
		break;
	case AG_VARIABLE_FLOAT:
		*(float *)min = (float)nmin;
		break;
	default:
		break;
	}
	AG_UnlockVariable(minb);
	AG_ObjectUnlock(num);
}

void
AG_NumericalSetMax(AG_Numerical *num, double nmax)
{
	AG_Variable *maxb;
	void *max;
	
	/* TODO allow integer min/max bindings */
	AG_ObjectLock(num);
	maxb = AG_GetVariable(num, "max", &max);
	switch (AG_VARIABLE_TYPE(maxb)) {
	case AG_VARIABLE_DOUBLE:
		*(double *)max = nmax;
		break;
	case AG_VARIABLE_FLOAT:
		*(float *)max = (float)nmax;
		break;
	default:
		break;
	}
	AG_UnlockVariable(maxb);
	AG_ObjectUnlock(num);
}

void
AG_NumericalSetIncrement(AG_Numerical *num, double inc)
{
	AG_ObjectLock(num);
	num->inc = inc;
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
	AG_ObjectUnlock(num);
	AG_Redraw(num);
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
	AG_Redraw(num);
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

void
AG_NumericalSetRange(AG_Numerical *num, double min, double max)
{
	AG_ObjectLock(num);
	AG_NumericalSetMin(num, min);
	AG_NumericalSetMax(num, max);
	AG_ObjectUnlock(num);
}

/* Convert the bound value to a float and return it. */
float
AG_NumericalGetFlt(AG_Numerical *num)
{
	AG_Variable *bValue;
	void *value;

	bValue = AG_GetVariable(num, "value", &value);
	switch (AG_VARIABLE_TYPE(bValue)) {
	case AG_VARIABLE_FLOAT:		return *(float *)value;
	case AG_VARIABLE_DOUBLE:	return  (float)(*(double *)value);
	case AG_VARIABLE_INT:		return  (float)(*(int *)value);
	case AG_VARIABLE_UINT:		return  (float)(*(Uint *)value);
	case AG_VARIABLE_UINT8:		return  (float)(*(Uint8 *)value);
	case AG_VARIABLE_UINT16:	return  (float)(*(Uint16 *)value);
	case AG_VARIABLE_UINT32:	return  (float)(*(Uint32 *)value);
	case AG_VARIABLE_SINT8:		return  (float)(*(Sint8 *)value);
	case AG_VARIABLE_SINT16:	return  (float)(*(Sint16 *)value);
	case AG_VARIABLE_SINT32:	return  (float)(*(Sint32 *)value);
	default:			return (0.0);
	}
}

/* Convert the bound value to a double and return it. */
double
AG_NumericalGetDbl(AG_Numerical *num)
{
	AG_Variable *bValue;
	void *value;

	bValue = AG_GetVariable(num, "value", &value);
	switch (AG_VARIABLE_TYPE(bValue)) {
	case AG_VARIABLE_DOUBLE:	return *(double *)value;
	case AG_VARIABLE_FLOAT:		return  (double)(*(float *)value);
	case AG_VARIABLE_INT:		return  (double)(*(int *)value);
	case AG_VARIABLE_UINT:		return  (double)(*(Uint *)value);
	case AG_VARIABLE_UINT8:		return  (double)(*(Uint8 *)value);
	case AG_VARIABLE_UINT16:	return  (double)(*(Uint16 *)value);
	case AG_VARIABLE_UINT32:	return  (double)(*(Uint32 *)value);
	case AG_VARIABLE_SINT8:		return  (double)(*(Sint8 *)value);
	case AG_VARIABLE_SINT16:	return  (double)(*(Sint16 *)value);
	case AG_VARIABLE_SINT32:	return  (double)(*(Sint32 *)value);
	default:			return (0.0);
	}
}

/* Convert the bound value to a natural integer and return it. */
int
AG_NumericalGetInt(AG_Numerical *num)
{
	AG_Variable *bValue;
	void *value;

	bValue = AG_GetVariable(num, "value", &value);
	switch (AG_VARIABLE_TYPE(bValue)) {
	case AG_VARIABLE_INT:		return *(int *)value;
	case AG_VARIABLE_UINT:		return (int)(*(Uint *)value);
	case AG_VARIABLE_UINT8:		return (int)(*(Uint8 *)value);
	case AG_VARIABLE_UINT16:	return (int)(*(Uint16 *)value);
	case AG_VARIABLE_UINT32:	return (int)(*(Uint32 *)value);
	case AG_VARIABLE_SINT8:		return (int)(*(Sint8 *)value);
	case AG_VARIABLE_SINT16:	return (int)(*(Sint16 *)value);
	case AG_VARIABLE_SINT32:	return (int)(*(Sint32 *)value);
	case AG_VARIABLE_DOUBLE:	return (int)(*(double *)value);
	case AG_VARIABLE_FLOAT:		return (int)(*(float *)value);
	default:			return (0);
	}
}

/* Convert the bound value to a 32-bit integer and return it. */
Uint32
AG_NumericalGetUint32(AG_Numerical *num)
{
	AG_Variable *bValue;
	void *value;

	bValue = AG_GetVariable(num, "value", &value);
	switch (AG_VARIABLE_TYPE(bValue)) {
	case AG_VARIABLE_INT:		return (Uint32)(*(int *)value);
	case AG_VARIABLE_UINT:		return (Uint32)(*(Uint *)value);
	case AG_VARIABLE_UINT8:		return (Uint32)(*(Uint8 *)value);
	case AG_VARIABLE_UINT16:	return (Uint32)(*(Uint16 *)value);
	case AG_VARIABLE_UINT32:	return *(Uint32 *)value;
	case AG_VARIABLE_SINT8:		return (Uint32)(*(Sint8 *)value);
	case AG_VARIABLE_SINT16:	return (Uint32)(*(Sint16 *)value);
	case AG_VARIABLE_SINT32:	return (Uint32)(*(Sint32 *)value);
	case AG_VARIABLE_DOUBLE:	return (Uint32)(*(double *)value);
	case AG_VARIABLE_FLOAT:		return (Uint32)(*(float *)value);
	default:			return (0);
	}
}

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

