/*
 * Copyright (c) 2003-2010 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * LEGACY: Use AG_Numerical(3) instead of this widget.
 */

#include <config/_mk_have_strtoll.h>

#include <core/core.h>

#include "fspinbutton.h"
#include "primitive.h"

#include <string.h>

static void SelectedUnit(AG_Event *);
static void InitUnitSystem(AG_FSpinbutton *, const char *);

AG_FSpinbutton *
AG_FSpinbuttonNew(void *parent, Uint flags, const char *unit, const char *label)
{
	AG_FSpinbutton *fsu;

	fsu = Malloc(sizeof(AG_FSpinbutton));
	AG_ObjectInit(fsu, &agFSpinbuttonClass);

	if (!(flags & AG_FSPINBUTTON_NOHFILL)) { AG_ExpandHoriz(fsu); }
	if (  flags & AG_FSPINBUTTON_VFILL) { AG_ExpandVert(fsu); }

	if (label != NULL) {
		AG_TextboxSetLabelS(fsu->input, label);
	}
	if (unit != NULL) {
		fsu->units = AG_UComboNew(fsu, 0);
		AG_SetEvent(fsu->units, "ucombo-selected",
		    SelectedUnit, "%p", fsu);
		InitUnitSystem(fsu, unit);
		AG_WidgetSetFocusable(fsu->units, 0);
	}

	AG_ObjectAttach(parent, fsu);
	return (fsu);
}

static void
Bound(AG_Event *event)
{
	AG_FSpinbutton *fsu = AG_SELF();
	AG_Variable *binding = AG_PTR(1);

	if (strcmp(binding->name, "value") == 0) {
		switch (AG_VARIABLE_TYPE(binding)) {
		case AG_VARIABLE_DOUBLE:
			fsu->min = -AG_DBL_MAX+1;
			fsu->max =  AG_DBL_MAX-1;
			AG_TextboxSetFltOnly(fsu->input, 1);
			break;
		case AG_VARIABLE_FLOAT:
			fsu->min = -AG_FLT_MAX+1;
			fsu->max =  AG_FLT_MAX-1;
			AG_TextboxSetFltOnly(fsu->input, 1);
			break;
		case AG_VARIABLE_INT:
			fsu->min = AG_INT_MIN+1;
			fsu->max = AG_INT_MAX-1;
			AG_TextboxSetIntOnly(fsu->input, 1);
			break;
		case AG_VARIABLE_UINT:
			fsu->min = 0;
			fsu->max = AG_UINT_MAX-1;
			AG_TextboxSetIntOnly(fsu->input, 1);
			break;
		case AG_VARIABLE_UINT8:
			fsu->min = 0;
			fsu->max = 0xffU;
			AG_TextboxSetIntOnly(fsu->input, 1);
			break;
		case AG_VARIABLE_SINT8:
			fsu->min = -0x7f+1;
			fsu->max =  0x7f-1;
			AG_TextboxSetIntOnly(fsu->input, 1);
			break;
		case AG_VARIABLE_UINT16:
			fsu->min = 0;
			fsu->max = 0xffffU;
			AG_TextboxSetIntOnly(fsu->input, 1);
			break;
		case AG_VARIABLE_SINT16:
			fsu->min = -0x7fff+1;
			fsu->max =  0x7fff-1;
			AG_TextboxSetIntOnly(fsu->input, 1);
			break;
		case AG_VARIABLE_UINT32:
			fsu->min = 0;
			fsu->max = 0xffffffffU;
			AG_TextboxSetIntOnly(fsu->input, 1);
			break;
		case AG_VARIABLE_SINT32:
			fsu->min = -0x7fffffff+1;
			fsu->max =  0x7fffffff-1;
			AG_TextboxSetIntOnly(fsu->input, 1);
			break;
		default:
			break;
		}
	}
	AG_Redraw(fsu);
}

static void
KeyDown(AG_Event *event)
{
	AG_FSpinbutton *fsu = AG_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case AG_KEY_UP:
		AG_FSpinbuttonAddValue(fsu, fsu->inc);
		break;
	case AG_KEY_DOWN:
		AG_FSpinbuttonAddValue(fsu, -fsu->inc);
		break;
	}
}

static void
TextChanged(AG_Event *event)
{
	AG_FSpinbutton *fsu = AG_PTR(1);
	int unfocus = AG_INT(2);
	AG_Variable *stringb, *valueb;
	char *s;
	void *value;

	AG_ObjectLock(fsu);

	valueb = AG_GetVariable(fsu, "value", &value);
	stringb = AG_GetVariable(fsu->input->ed, "string", &s);

	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_DOUBLE:
	case AG_VARIABLE_FLOAT:
		AG_FSpinbuttonSetValue(fsu,
		    AG_Unit2Base(strtod(s, NULL), fsu->unit));
		break;
	case AG_VARIABLE_INT:
	case AG_VARIABLE_UINT:
	case AG_VARIABLE_UINT8:
	case AG_VARIABLE_SINT8:
	case AG_VARIABLE_UINT16:
	case AG_VARIABLE_SINT16:
	case AG_VARIABLE_UINT32:
	case AG_VARIABLE_SINT32:
		AG_FSpinbuttonSetValue(fsu, (double)strtol(s, NULL, 10));
		break;
	default:
		break;
	}

	AG_UnlockVariable(stringb);
	AG_UnlockVariable(valueb);

	if (unfocus) {
		AG_WidgetUnfocus(fsu->input);
	}
	AG_PostEvent(NULL, fsu, "fspinbutton-return", NULL);

	AG_ObjectUnlock(fsu);
}

static void
Increment(AG_Event *event)
{
	AG_FSpinbutton *fsu = AG_PTR(1);

	AG_ObjectLock(fsu);
	AG_FSpinbuttonAddValue(fsu, fsu->inc);
	AG_ObjectUnlock(fsu);
}

static void
Decrement(AG_Event *event)
{
	AG_FSpinbutton *fsu = AG_PTR(1);
	
	AG_ObjectLock(fsu);
	AG_FSpinbuttonAddValue(fsu, -fsu->inc);
	AG_ObjectUnlock(fsu);
}

/* Widget must be locked. */
static __inline__ void
UpdateUnitButton(AG_FSpinbutton *fsu)
{
	AG_ButtonTextS(fsu->units->button, AG_UnitAbbr(fsu->unit));
}

static void
SelectedUnit(AG_Event *event)
{
	AG_FSpinbutton *fsu = AG_PTR(1);
	AG_TlistItem *ti = AG_PTR(2);

	AG_ObjectLock(fsu);
	fsu->unit = (const AG_Unit *)ti->p1;
	UpdateUnitButton(fsu);
	AG_ObjectUnlock(fsu);
}

static void
InitUnitSystem(AG_FSpinbutton *fsu, const char *unit_key)
{
	const AG_Unit *unit = NULL;
	const AG_Unit *ugroup = NULL;
	int found = 0;
	int i;

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
		AG_FatalError("AG_FSpinbutton: No such unit: %s", unit_key);
	}
	fsu->unit = unit;
	UpdateUnitButton(fsu);

	AG_ObjectLock(fsu->units->list);
	AG_TlistDeselectAll(fsu->units->list);
	for (unit = &ugroup[0]; unit->key != NULL; unit++) {
		AG_TlistItem *it;

		it = AG_TlistAddPtr(fsu->units->list, NULL, _(unit->name),
		    (void *)unit);
		if (unit == fsu->unit)
			it->selected++;
	}
	AG_TlistSizeHintLargest(fsu->units->list, 5);
	AG_ObjectUnlock(fsu->units->list);
}

static void
Init(void *obj)
{
	AG_FSpinbutton *fsu = obj;
	
	WIDGET(fsu)->flags |= AG_WIDGET_TABLE_EMBEDDABLE;

	AG_BindDouble(fsu, "value", &fsu->value);
	AG_BindDouble(fsu, "min", &fsu->min);
	AG_BindDouble(fsu, "max", &fsu->max);
	
	AG_RedrawOnChange(fsu, 250, "value");
	
	fsu->inc = 1.0;
	fsu->value = 0.0;
	fsu->input = AG_TextboxNewS(fsu, AG_TEXTBOX_FLT_ONLY|AG_TEXTBOX_STATIC,
	    NULL);
	fsu->writeable = 1;
	Strlcpy(fsu->format, "%.02f", sizeof(fsu->format));
	AG_TextboxSizeHint(fsu->input, "88.88");
	
	fsu->unit = AG_FindUnit("identity");
	fsu->units = NULL;

	fsu->incbu = AG_ButtonNewS(fsu, AG_BUTTON_REPEAT, _("+"));
	AG_ButtonSetPadding(fsu->incbu, 0,0,0,0);
	AG_LabelSetPadding(fsu->incbu->lbl, 0,0,0,0);
	AG_WidgetSetFocusable(fsu->incbu, 0);
	
	fsu->decbu = AG_ButtonNewS(fsu, AG_BUTTON_REPEAT, _("-"));
	AG_ButtonSetPadding(fsu->decbu, 0,0,0,0);
	AG_LabelSetPadding(fsu->decbu->lbl, 0,0,0,0);
	AG_WidgetSetFocusable(fsu->incbu, 0);

	AG_SetEvent(fsu, "bound", Bound, NULL);
	AG_SetEvent(fsu, "key-down", KeyDown, NULL);
	AG_SetEvent(fsu->input, "textbox-return", TextChanged, "%p,%i",fsu,1);
	AG_SetEvent(fsu->input, "textbox-changed", TextChanged, "%p,%i",fsu,0);
	AG_SetEvent(fsu->incbu, "button-pushed", Increment, "%p",fsu);
	AG_SetEvent(fsu->decbu, "button-pushed", Decrement, "%p",fsu);
}

void
AG_FSpinbuttonSizeHint(AG_FSpinbutton *fsu, const char *text)
{
	AG_ObjectLock(fsu);
	AG_TextboxSizeHint(fsu->input, text);
	AG_ObjectUnlock(fsu);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_FSpinbutton *num = obj;
	AG_SizeReq rChld, rInc, rDec;

	AG_WidgetSizeReq(num->input, &rChld);
	r->w = rChld.w;
	r->h = rChld.h;
	if (num->units != NULL) {
		AG_WidgetSizeReq(num->units, &rChld);
		r->w += rChld.w;
	}
	AG_WidgetSizeReq(num->incbu, &rInc);
	AG_WidgetSizeReq(num->decbu, &rDec);
	r->w += MAX(rInc.w, rDec.w);
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_FSpinbutton *num = obj;
	AG_SizeAlloc aChld;
	AG_SizeReq rUnits;
	int szBtn = a->h/2;

	if (a->h < 4 || a->w < szBtn+4)
		return (-1);

	if (num->units != NULL) {
		AG_WidgetSizeReq(num->units, &rUnits);
		if (rUnits.w > a->w - szBtn-4) { rUnits.w = a->w - szBtn-4; }
		if (rUnits.h > a->h) { rUnits.h = a->h; }
	} else {
		rUnits.w = 0;
		rUnits.h = 0;
	}
	
	/* Size input textbox */
	aChld.x = 0;
	aChld.y = 0;
	aChld.w = a->w - rUnits.w - szBtn - 4;
	aChld.h = a->h;
	AG_WidgetSizeAlloc(num->input, &aChld);
	aChld.x += aChld.w + 2;

	/* Size unit selector */
	if (num->units != NULL) {
		aChld.w = rUnits.w;
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
	AG_FSpinbutton *fsu = obj;
	AG_Variable *valueb;
	void *value;

	AG_WidgetDraw(fsu->input);
	AG_WidgetDraw(fsu->incbu);
	AG_WidgetDraw(fsu->decbu);
	if (fsu->units != NULL) { AG_WidgetDraw(fsu->units); }

	if (AG_WidgetIsFocused(fsu->input))
		return;

	valueb = AG_GetVariable(fsu, "value", &value);
	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_DOUBLE:
		AG_TextboxPrintf(fsu->input, fsu->format,
		    AG_Base2Unit(*(double *)value, fsu->unit));
		break;
	case AG_VARIABLE_FLOAT:
		AG_TextboxPrintf(fsu->input, fsu->format,
		    AG_Base2Unit(*(float *)value, fsu->unit));
		break;
	case AG_VARIABLE_INT:
		AG_TextboxPrintf(fsu->input, "%d", *(int *)value);
		break;
	case AG_VARIABLE_UINT:
		AG_TextboxPrintf(fsu->input, "%u", *(Uint *)value);
		break;
	case AG_VARIABLE_UINT8:
		AG_TextboxPrintf(fsu->input, "%u", *(Uint8 *)value);
		break;
	case AG_VARIABLE_SINT8:
		AG_TextboxPrintf(fsu->input, "%d", *(Sint8 *)value);
		break;
	case AG_VARIABLE_UINT16:
		AG_TextboxPrintf(fsu->input, "%u", *(Uint16 *)value);
		break;
	case AG_VARIABLE_SINT16:
		AG_TextboxPrintf(fsu->input, "%d", *(Sint16 *)value);
		break;
	case AG_VARIABLE_UINT32:
		AG_TextboxPrintf(fsu->input, "%u", *(Uint32 *)value);
		break;
	case AG_VARIABLE_SINT32:
		AG_TextboxPrintf(fsu->input, "%d", *(Sint32 *)value);
		break;
	default:
		break;
	}
	AG_UnlockVariable(valueb);
}

#define ADD_CONVERTED(TYPE) do { \
	n = (double)(*(TYPE *)value); \
	if ((n+inc) < *min) { n = *min; } \
	else if ((n+inc) > *max) { n = *(max); } \
	else { n += inc; } \
	*(TYPE *)value = (TYPE)n; \
} while (0)
#define ADD_REAL(TYPE) do { \
	n = AG_Base2Unit(*(TYPE *)value, fsu->unit); \
	if ((n+inc) < *min) { n = *min; } \
	else if ((n+inc) > *max) { n = *(max); } \
	else { n += inc; } \
	*(TYPE *)value = AG_Unit2Base(n, fsu->unit); \
} while (0)
void
AG_FSpinbuttonAddValue(AG_FSpinbutton *fsu, double inc)
{
	AG_Variable *valueb, *minb, *maxb;
	void *value;
	double n;
	double *min, *max;

	valueb = AG_GetVariable(fsu, "value", &value);
	minb = AG_GetVariable(fsu, "min", &min);
	maxb = AG_GetVariable(fsu, "max", &max);

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
	default:						break;
	}
	AG_PostEvent(NULL, fsu, "fspinbutton-changed", NULL);

	AG_UnlockVariable(valueb);
	AG_UnlockVariable(minb);
	AG_UnlockVariable(maxb);
	AG_Redraw(fsu);
}
#undef ADD_INCREMENT
#undef ADD_REAL
#undef ADD_CONVERTED

#define ASSIGN_VALUE(TYPE) *(TYPE *)value = nvalue < *min ? *min : \
    nvalue > *max ? *max : nvalue
#define CONV_VALUE(TYPE) \
    *(TYPE *)value = nvalue < *min ? (TYPE)(*min) : \
     nvalue > *max ? (TYPE)(*max) : (TYPE)nvalue

/* TODO int types directly */
void
AG_FSpinbuttonSetValue(AG_FSpinbutton *fsu, double nvalue)
{
	AG_Variable *valueb, *minb, *maxb;
	void *value;
	double *min, *max;

	valueb = AG_GetVariable(fsu, "value", &value);
	minb = AG_GetVariable(fsu, "min", &min);
	maxb = AG_GetVariable(fsu, "max", &max);

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
	default:						break;
	}

	AG_PostEvent(NULL, fsu, "fspinbutton-changed", NULL);

	AG_UnlockVariable(valueb);
	AG_UnlockVariable(minb);
	AG_UnlockVariable(maxb);
	AG_Redraw(fsu);
}
#undef ASSIGN_VALUE
#undef CONV_VALUE

void
AG_FSpinbuttonSetMin(AG_FSpinbutton *fsu, double nmin)
{
	AG_Variable *minb;
	void *min;

	/* TODO allow integer min/max bindings */
	minb = AG_GetVariable(fsu, "min", &min);
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
}

void
AG_FSpinbuttonSetMax(AG_FSpinbutton *fsu, double nmax)
{
	AG_Variable *maxb;
	void *max;
	
	/* TODO allow integer min/max bindings */
	maxb = AG_GetVariable(fsu, "max", &max);
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
}

void
AG_FSpinbuttonSetIncrement(AG_FSpinbutton *fsu, double inc)
{
	AG_ObjectLock(fsu);
	fsu->inc = inc;
	AG_ObjectUnlock(fsu);
}

void
AG_FSpinbuttonSetPrecision(AG_FSpinbutton *fsu, const char *mode,
    int precision)
{
	AG_ObjectLock(fsu);
	fsu->format[0] = '%';
	fsu->format[1] = '.';
	fsu->format[2] = '\0';
	StrlcatInt(fsu->format, precision, sizeof(fsu->format));
	Strlcat(fsu->format, mode, sizeof(fsu->format));
	AG_ObjectUnlock(fsu);
	AG_Redraw(fsu);
}

void
AG_FSpinbuttonSelectUnit(AG_FSpinbutton *fsu, const char *uname)
{
	AG_TlistItem *it;

	AG_ObjectLock(fsu);
	AG_ObjectLock(fsu->units->list);
	AG_TlistDeselectAll(fsu->units->list);
	TAILQ_FOREACH(it, &fsu->units->list->items, items) {
		const AG_Unit *unit = it->p1;

		if (strcmp(unit->key, uname) == 0) {
			it->selected++;
			fsu->unit = unit;
			UpdateUnitButton(fsu);
			break;
		}
	}
	AG_ObjectUnlock(fsu->units->list);
	AG_ObjectUnlock(fsu);
}

void
AG_FSpinbuttonSetWriteable(AG_FSpinbutton *fsu, int writeable)
{
	AG_ObjectLock(fsu);
	fsu->writeable = writeable;
	if (writeable) {
		AG_WidgetEnable(fsu->incbu);
		AG_WidgetEnable(fsu->decbu);
		AG_WidgetEnable(fsu->input);
	} else {
		AG_WidgetDisable(fsu->incbu);
		AG_WidgetDisable(fsu->decbu);
		AG_WidgetDisable(fsu->input);
	}
	AG_ObjectUnlock(fsu);
}

void
AG_FSpinbuttonSetRange(AG_FSpinbutton *fsu, double min, double max)
{
	AG_ObjectLock(fsu);
	AG_FSpinbuttonSetMin(fsu, min);
	AG_FSpinbuttonSetMax(fsu, max);
	AG_ObjectUnlock(fsu);
}

AG_WidgetClass agFSpinbuttonClass = {
	{
		"Agar(Widget:FSpinbutton)",
		sizeof(AG_FSpinbutton),
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
