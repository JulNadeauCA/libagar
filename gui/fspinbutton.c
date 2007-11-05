/*
 * Copyright (c) 2003-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <config/_mk_have_strtoll.h>

#include <core/limits.h>
#include <core/core.h>

#include "fspinbutton.h"
#include "primitive.h"

#include <string.h>

AG_FSpinbutton *
AG_FSpinbuttonNew(void *parent, Uint flags, const char *unit, const char *label)
{
	AG_FSpinbutton *fsu;

	fsu = Malloc(sizeof(AG_FSpinbutton), M_OBJECT);
	AG_FSpinbuttonInit(fsu, flags, unit, label);
	AG_ObjectAttach(parent, fsu);
	return (fsu);
}


/* Adjust the default range depending on the data type of a new binding. */
static void
binding_changed(AG_Event *event)
{
	AG_FSpinbutton *fsu = AG_SELF();
	AG_WidgetBinding *binding = AG_PTR(1);

	if (strcmp(binding->name, "value") == 0) {
		AG_MutexLock(&fsu->lock);
		switch (binding->vtype) {
		case AG_WIDGET_DOUBLE:
			fsu->min = -AG_DBL_MAX+1;
			fsu->max =  AG_DBL_MAX-1;
			fsu->input->flags |= AG_TEXTBOX_FLT_ONLY;
			fsu->input->flags &= ~AG_TEXTBOX_INT_ONLY;
			break;
		case AG_WIDGET_FLOAT:
			fsu->min = -AG_FLT_MAX+1;
			fsu->max =  AG_FLT_MAX-1;
			fsu->input->flags |= AG_TEXTBOX_FLT_ONLY;
			fsu->input->flags &= ~AG_TEXTBOX_INT_ONLY;
			break;
		case AG_WIDGET_INT:
			fsu->min = AG_INT_MIN+1;
			fsu->max = AG_INT_MAX-1;
			fsu->input->flags |= AG_TEXTBOX_INT_ONLY;
			fsu->input->flags &= ~AG_TEXTBOX_FLT_ONLY;
			break;
		case AG_WIDGET_UINT:
			fsu->min = 0;
			fsu->max = AG_UINT_MAX-1;
			fsu->input->flags |= AG_TEXTBOX_INT_ONLY;
			fsu->input->flags &= ~AG_TEXTBOX_FLT_ONLY;
			break;
		case AG_WIDGET_UINT8:
			fsu->min = 0;
			fsu->max = 0xffU;
			fsu->input->flags |= AG_TEXTBOX_INT_ONLY;
			fsu->input->flags &= ~AG_TEXTBOX_FLT_ONLY;
			break;
		case AG_WIDGET_SINT8:
			fsu->min = -0x7f+1;
			fsu->max =  0x7f-1;
			fsu->input->flags |= AG_TEXTBOX_INT_ONLY;
			fsu->input->flags &= ~AG_TEXTBOX_FLT_ONLY;
			break;
		case AG_WIDGET_UINT16:
			fsu->min = 0;
			fsu->max = 0xffffU;
			fsu->input->flags |= AG_TEXTBOX_INT_ONLY;
			fsu->input->flags &= ~AG_TEXTBOX_FLT_ONLY;
			break;
		case AG_WIDGET_SINT16:
			fsu->min = -0x7fff+1;
			fsu->max =  0x7fff-1;
			fsu->input->flags |= AG_TEXTBOX_INT_ONLY;
			fsu->input->flags &= ~AG_TEXTBOX_FLT_ONLY;
			break;
		case AG_WIDGET_UINT32:
			fsu->min = 0;
			fsu->max = 0xffffffffU;
			fsu->input->flags |= AG_TEXTBOX_INT_ONLY;
			fsu->input->flags &= ~AG_TEXTBOX_FLT_ONLY;
			break;
		case AG_WIDGET_SINT32:
			fsu->min = -0x7fffffff+1;
			fsu->max =  0x7fffffff-1;
			fsu->input->flags |= AG_TEXTBOX_INT_ONLY;
			fsu->input->flags &= ~AG_TEXTBOX_FLT_ONLY;
			break;
#ifdef HAVE_64BIT
		case AG_WIDGET_UINT64:
			fsu->min = 0;
			fsu->max = 0xffffffffffffffffULL;
			fsu->input->flags |= AG_TEXTBOX_INT_ONLY;
			fsu->input->flags &= ~AG_TEXTBOX_FLT_ONLY;
			break;
		case AG_WIDGET_SINT64:
			fsu->min = -0x7fffffffffffffffULL+1;
			fsu->max =  0x7fffffffffffffffULL-1;
			fsu->input->flags |= AG_TEXTBOX_INT_ONLY;
			fsu->input->flags &= ~AG_TEXTBOX_FLT_ONLY;
			break;
#endif
		}
		AG_MutexUnlock(&fsu->lock);
	}
}

static void
key_pressed(AG_Event *event)
{
	AG_FSpinbutton *fsu = AG_SELF();
	int keysym = AG_INT(1);

	AG_MutexLock(&fsu->lock);
	switch (keysym) {
	case SDLK_UP:
		AG_FSpinbuttonAddValue(fsu, fsu->inc);
		break;
	case SDLK_DOWN:
		AG_FSpinbuttonAddValue(fsu, -fsu->inc);
		break;
	}
	AG_MutexUnlock(&fsu->lock);
}

static void
changed(AG_Event *event)
{
	AG_FSpinbutton *fsu = AG_PTR(1);
	int unfocus = AG_INT(2);
	AG_WidgetBinding *stringb, *valueb;
	char *s;
	void *value;

	valueb = AG_WidgetGetBinding(fsu, "value", &value);
	stringb = AG_WidgetGetBinding(fsu->input, "string", &s);

	switch (valueb->vtype) {
	case AG_WIDGET_DOUBLE:
	case AG_WIDGET_FLOAT:
		AG_FSpinbuttonSetValue(fsu,
		    AG_Unit2Base(strtod(s, NULL), fsu->unit));
		break;
	case AG_WIDGET_INT:
	case AG_WIDGET_UINT:
	case AG_WIDGET_UINT8:
	case AG_WIDGET_SINT8:
	case AG_WIDGET_UINT16:
	case AG_WIDGET_SINT16:
	case AG_WIDGET_UINT32:
	case AG_WIDGET_SINT32:
		AG_FSpinbuttonSetValue(fsu, (double)strtol(s, NULL, 10));
		break;
#if defined(HAVE_64BIT) && defined(_MK_HAVE_STRTOLL)
	case AG_WIDGET_UINT64:
	case AG_WIDGET_SINT64:
		AG_FSpinbuttonSetValue(fsu, (double)strtoll(s, NULL, 10));
		break;
#endif
	}

	AG_WidgetUnlockBinding(stringb);
	AG_WidgetUnlockBinding(valueb);

	if (unfocus) {
		AG_WidgetUnfocus(fsu->input);
	}
	AG_PostEvent(NULL, fsu, "fspinbutton-return", NULL);
}

static void
increment_pressed(AG_Event *event)
{
	AG_FSpinbutton *fsu = AG_PTR(1);

	AG_MutexLock(&fsu->lock);
	AG_FSpinbuttonAddValue(fsu, fsu->inc);
	AG_MutexUnlock(&fsu->lock);
}

static void
decrement_pressed(AG_Event *event)
{
	AG_FSpinbutton *fsu = AG_PTR(1);
	
	AG_MutexLock(&fsu->lock);
	AG_FSpinbuttonAddValue(fsu, -fsu->inc);
	AG_MutexUnlock(&fsu->lock);
}

static void
update_unit_button(AG_FSpinbutton *fsu)
{
	AG_ButtonText(fsu->units->button, "%s", AG_UnitAbbr(fsu->unit));
}

static void
selected_unit(AG_Event *event)
{
	AG_FSpinbutton *fsu = AG_PTR(1);
	AG_TlistItem *ti = AG_PTR(2);

	fsu->unit = (const AG_Unit *)ti->p1;
	update_unit_button(fsu);
}

static void
init_unit_system(AG_FSpinbutton *fsu, const char *unit_key)
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
		fatal("unknown unit: `%s'", unit_key);
	}
	fsu->unit = unit;
	update_unit_button(fsu);

	AG_MutexLock(&fsu->units->list->lock);
	AG_TlistDeselectAll(fsu->units->list);
	for (unit = &ugroup[0]; unit->key != NULL; unit++) {
		AG_TlistItem *it;

		it = AG_TlistAddPtr(fsu->units->list, NULL, _(unit->name),
		    (void *)unit);
		if (unit == fsu->unit)
			it->selected++;
	}
	AG_MutexUnlock(&fsu->units->list->lock);
}

void
AG_FSpinbuttonInit(AG_FSpinbutton *fsu, Uint flags, const char *unit,
    const char *label)
{
	Uint wflags = 0;

	if ((flags & AG_FSPINBUTTON_NOHFILL)==0) { wflags |= AG_WIDGET_HFILL; }
	if (flags & AG_FSPINBUTTON_VFILL) { wflags |= AG_WIDGET_VFILL; }

	AG_WidgetInit(fsu, &agFSpinbuttonOps, wflags);
	AG_WidgetBind(fsu, "value", AG_WIDGET_DOUBLE, &fsu->value);
	AG_WidgetBind(fsu, "min", AG_WIDGET_DOUBLE, &fsu->min);
	AG_WidgetBind(fsu, "max", AG_WIDGET_DOUBLE, &fsu->max);
	
	fsu->inc = 1.0;
	fsu->value = 0.0;
	fsu->input = AG_TextboxNew(fsu, AG_TEXTBOX_FLT_ONLY, label);
	fsu->writeable = 1;
	strlcpy(fsu->format, "%g", sizeof(fsu->format));
	AG_MutexInit(&fsu->lock);
	AG_TextboxSizeHint(fsu->input, "88.88");
	
	if (unit != NULL) {
		fsu->units = AG_UComboNew(fsu, 0);
		AG_SetEvent(fsu->units, "ucombo-selected", selected_unit,
		    "%p", fsu);
		init_unit_system(fsu, unit);
		AG_WidgetSetFocusable(fsu->units, 0);
	} else {
		fsu->unit = AG_FindUnit("identity");
		fsu->units = NULL;
	}

	fsu->incbu = AG_ButtonNew(fsu, AG_BUTTON_REPEAT, _("+"));
	AG_ButtonSetPadding(fsu->incbu, 1,1,1,1);
	AG_WidgetSetFocusable(fsu->incbu, 0);
	
	fsu->decbu = AG_ButtonNew(fsu, AG_BUTTON_REPEAT, _("-"));
	AG_ButtonSetPadding(fsu->decbu, 1,1,1,1);
	AG_WidgetSetFocusable(fsu->incbu, 0);

	AG_SetEvent(fsu, "widget-bound", binding_changed, NULL);
	AG_SetEvent(fsu, "window-keydown", key_pressed, NULL);
	AG_SetEvent(fsu->input, "textbox-return", changed, "%p,%i", fsu, 1);
	AG_SetEvent(fsu->input, "textbox-changed", changed, "%p,%i", fsu, 0);
	AG_SetEvent(fsu->incbu, "button-pushed", increment_pressed, "%p", fsu);
	AG_SetEvent(fsu->decbu, "button-pushed", decrement_pressed, "%p", fsu);
}

void
AG_FSpinbuttonSizeHint(AG_FSpinbutton *fsu, const char *text)
{
	AG_TextboxSizeHint(fsu->input, text);
}

static void
Destroy(void *p)
{
	AG_FSpinbutton *fsu = p;

	AG_MutexDestroy(&fsu->lock);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_FSpinbutton *num = p;
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
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_FSpinbutton *num = p;
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
Draw(void *p)
{
	AG_FSpinbutton *fsu = p;
	AG_WidgetBinding *valueb;
	void *value;

	if (AG_WidgetFocused(fsu->input))
		return;

	valueb = AG_WidgetGetBinding(fsu, "value", &value);
	switch (valueb->vtype) {
	case AG_WIDGET_DOUBLE:
		AG_TextboxPrintf(fsu->input, fsu->format,
		    AG_Base2Unit(*(double *)value, fsu->unit));
		break;
	case AG_WIDGET_FLOAT:
		AG_TextboxPrintf(fsu->input, fsu->format,
		    AG_Base2Unit(*(float *)value, fsu->unit));
		break;
	case AG_WIDGET_INT:
		AG_TextboxPrintf(fsu->input, "%d", *(int *)value);
		break;
	case AG_WIDGET_UINT:
		AG_TextboxPrintf(fsu->input, "%u", *(Uint *)value);
		break;
	case AG_WIDGET_UINT8:
		AG_TextboxPrintf(fsu->input, "%u", *(Uint8 *)value);
		break;
	case AG_WIDGET_SINT8:
		AG_TextboxPrintf(fsu->input, "%d", *(Sint8 *)value);
		break;
	case AG_WIDGET_UINT16:
		AG_TextboxPrintf(fsu->input, "%u", *(Uint16 *)value);
		break;
	case AG_WIDGET_SINT16:
		AG_TextboxPrintf(fsu->input, "%d", *(Sint16 *)value);
		break;
	case AG_WIDGET_UINT32:
		AG_TextboxPrintf(fsu->input, "%u", *(Uint32 *)value);
		break;
	case AG_WIDGET_SINT32:
		AG_TextboxPrintf(fsu->input, "%d", *(Sint32 *)value);
		break;
#ifdef HAVE_64BIT
	case AG_WIDGET_UINT64:
		AG_TextboxPrintf(fsu->input, "%lld", *(Uint64 *)value);
		break;
	case AG_WIDGET_SINT64:
		AG_TextboxPrintf(fsu->input, "%lld", *(Sint64 *)value);
		break;
#endif
	}
	AG_WidgetUnlockBinding(valueb);
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
	AG_WidgetBinding *valueb, *minb, *maxb;
	void *value;
	double n;
	double *min, *max;

	valueb = AG_WidgetGetBinding(fsu, "value", &value);
	minb = AG_WidgetGetBinding(fsu, "min", &min);
	maxb = AG_WidgetGetBinding(fsu, "max", &max);

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
	AG_PostEvent(NULL, fsu, "fspinbutton-changed", NULL);
	AG_WidgetBindingChanged(valueb);

	AG_WidgetUnlockBinding(valueb);
	AG_WidgetUnlockBinding(minb);
	AG_WidgetUnlockBinding(maxb);
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
	AG_WidgetBinding *valueb, *minb, *maxb;
	void *value;
	double *min, *max;

	valueb = AG_WidgetGetBinding(fsu, "value", &value);
	minb = AG_WidgetGetBinding(fsu, "min", &min);
	maxb = AG_WidgetGetBinding(fsu, "max", &max);

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

	AG_PostEvent(NULL, fsu, "fspinbutton-changed", NULL);
	AG_WidgetBindingChanged(valueb);

	AG_WidgetUnlockBinding(valueb);
	AG_WidgetUnlockBinding(minb);
	AG_WidgetUnlockBinding(maxb);
}
#undef ASSIGN_VALUE
#undef CONV_VALUE

void
AG_FSpinbuttonSetMin(AG_FSpinbutton *fsu, double nmin)
{
	AG_WidgetBinding *minb;
	void *min;

	/* TODO allow integer min/max bindings */
	minb = AG_WidgetGetBinding(fsu, "min", &min);
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
AG_FSpinbuttonSetMax(AG_FSpinbutton *fsu, double nmax)
{
	AG_WidgetBinding *maxb;
	void *max;
	
	/* TODO allow integer min/max bindings */
	maxb = AG_WidgetGetBinding(fsu, "max", &max);
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
AG_FSpinbuttonSetIncrement(AG_FSpinbutton *fsu, double inc)
{
	AG_MutexLock(&fsu->lock);
	fsu->inc = inc;
	AG_MutexUnlock(&fsu->lock);
}

void
AG_FSpinbuttonSetPrecision(AG_FSpinbutton *fsu, const char *mode,
    int precision)
{
	AG_MutexLock(&fsu->lock);
	fsu->format[0] = '%';
	fsu->format[1] = '.';
	snprintf(&fsu->format[2], sizeof(fsu->format)-2, "%d", precision);
	strlcat(fsu->format, mode, sizeof(fsu->format));
	AG_MutexUnlock(&fsu->lock);
}

void
AG_FSpinbuttonSelectUnit(AG_FSpinbutton *fsu, const char *uname)
{
	AG_TlistItem *it;

	AG_MutexLock(&fsu->units->list->lock);
	AG_TlistDeselectAll(fsu->units->list);
	TAILQ_FOREACH(it, &fsu->units->list->items, items) {
		const AG_Unit *unit = it->p1;

		if (strcmp(unit->key, uname) == 0) {
			it->selected++;
			fsu->unit = unit;
			update_unit_button(fsu);
			break;
		}
	}
	AG_MutexUnlock(&fsu->units->list->lock);
}

void
AG_FSpinbuttonSetWriteable(AG_FSpinbutton *fsu, int writeable)
{
	AG_MutexLock(&fsu->lock);
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
	AG_MutexUnlock(&fsu->lock);
}

void
AG_FSpinbuttonSetRange(AG_FSpinbutton *fsu, double min, double max)
{
	AG_MutexLock(&fsu->lock);
	AG_FSpinbuttonSetMin(fsu, min);
	AG_FSpinbuttonSetMax(fsu, max);
	AG_MutexUnlock(&fsu->lock);
}

const AG_WidgetOps agFSpinbuttonOps = {
	{
		"AG_Widget:AG_FSpinbutton",
		sizeof(AG_FSpinbutton),
		{ 0,0 },
		NULL,			/* init */
		NULL,			/* free */
		Destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
