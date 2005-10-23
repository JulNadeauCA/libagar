/*	$Csoft: fspinbutton.c,v 1.33 2005/10/01 14:15:38 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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
#include <core/view.h>

#include "fspinbutton.h"

#include <gui/window.h>
#include <gui/primitive.h>
#include <gui/label.h>
#include <gui/units.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

static AG_WidgetOps agFSpinbuttonOps = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		AG_FSpinbuttonDestroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	AG_FSpinbuttonDraw,
	AG_FSpinbuttonScale
};

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
			fsu->min = -DBL_MAX+1;
			fsu->max = DBL_MAX-1;
			break;
		case AG_WIDGET_FLOAT:
			fsu->min = -FLT_MAX+1;
			fsu->max = FLT_MAX-1;
			break;
		case AG_WIDGET_INT:
			fsu->min = INT_MIN+1;
			fsu->max = INT_MAX-1;
			break;
		case AG_WIDGET_UINT:
			fsu->min = 0;
			fsu->max = UINT_MAX-1;
			break;
		case AG_WIDGET_UINT8:
			fsu->min = 0;
			fsu->max = 0xffU;
			break;
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
	AG_WidgetBinding *stringb;
	char *s;

	stringb = AG_WidgetGetBinding(fsu->input, "string", &s);
	AG_FSpinbuttonSetValue(fsu, AG_Unit2Base(strtod(s, NULL), fsu->unit));
	AG_WidgetUnlockBinding(stringb);

	AG_PostEvent(NULL, fsu, "fspinbutton-return", NULL);

	if (unfocus)
		AG_WidgetUnfocus(fsu->input);
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
	AG_ButtonPrintf(fsu->units->button, "%s", AG_UnitAbbr(fsu->unit));
}

static void
selected_unit(AG_Event *event)
{
	AG_UCombo *ucom = AG_SELF();
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
	AG_WidgetInit(fsu, "fspinbutton", &agFSpinbuttonOps,
	    AG_WIDGET_FOCUSABLE|AG_WIDGET_WFILL);

	AG_WidgetBind(fsu, "value", AG_WIDGET_DOUBLE, &fsu->value);
	AG_WidgetBind(fsu, "min", AG_WIDGET_DOUBLE, &fsu->min);
	AG_WidgetBind(fsu, "max", AG_WIDGET_DOUBLE, &fsu->max);
	
	fsu->inc = 1.0;
	fsu->value = 0.0;
	fsu->input = AG_TextboxNew(fsu, 0, label);
	fsu->writeable = 1;
	strlcpy(fsu->format, "%g", sizeof(fsu->format));
	AG_MutexInit(&fsu->lock);
	
	if (unit != NULL) {
		fsu->units = AG_UComboNew(fsu);
		AG_SetEvent(fsu->units, "ucombo-selected", selected_unit,
		    "%p", fsu);
		init_unit_system(fsu, unit);
	} else {
		fsu->unit = AG_FindUnit("identity");
		fsu->units = NULL;
	}

	fsu->incbu = AG_ButtonNew(fsu, AG_BUTTON_REPEAT, _("+"));
	AG_ButtonSetPadding(fsu->incbu, 0);
	fsu->decbu = AG_ButtonNew(fsu, AG_BUTTON_REPEAT, _("-"));
	AG_ButtonSetPadding(fsu->decbu, 0);

	AG_SetEvent(fsu, "widget-bound", binding_changed, NULL);
	AG_SetEvent(fsu, "window-keydown", key_pressed, NULL);
	AG_SetEvent(fsu->input, "textbox-return", changed, "%p,%i", fsu, 1);
	AG_SetEvent(fsu->input, "textbox-changed", changed, "%p,%i", fsu, 0);
	AG_SetEvent(fsu->incbu, "button-pushed", increment_pressed, "%p", fsu);
	AG_SetEvent(fsu->decbu, "button-pushed", decrement_pressed, "%p", fsu);
}

void
AG_FSpinbuttonPrescale(AG_FSpinbutton *fsu, const char *text)
{
	AG_TextboxPrescale(fsu->input, text);
}

void
AG_FSpinbuttonDestroy(void *p)
{
	AG_FSpinbutton *fsu = p;

	AG_MutexDestroy(&fsu->lock);
	AG_WidgetDestroy(fsu);
}

void
AG_FSpinbuttonScale(void *p, int w, int h)
{
	AG_FSpinbutton *fsu = p;
	AG_Textbox *input = fsu->input;
	AG_UCombo *units = fsu->units;
	AG_Button *incbu = fsu->incbu;
	AG_Button *decbu = fsu->decbu;
	const int bw = 10;
	int x = 0, y = 0;
	int uw, uh;

	if (units != NULL) {
		AG_TextPrescale("XXXXXXXX", &uw, &uh);
	} else {
		uw = 0;
		uh = 0;
	}

	if (w == -1 && h == -1) {
		AGWIDGET_SCALE(input, -1, -1);
		AGWIDGET(fsu)->w = AGWIDGET(input)->w + input->xpadding*2;
		AGWIDGET(fsu)->h = AGWIDGET(input)->h;

		x += AGWIDGET(fsu)->w;
		
		if (units != NULL) {
			AGWIDGET_SCALE(units, -1, -1);
			AGWIDGET(fsu)->w += AGWIDGET(units)->w;
			x += AGWIDGET(units)->w;
		}

		AGWIDGET_SCALE(incbu, -1, -1);
		AGWIDGET(fsu)->w += AGWIDGET(incbu)->w;
		y += AGWIDGET(fsu)->h;
		return;
	}

	AGWIDGET(input)->x = 0;
	AGWIDGET(input)->y = 0;
	AG_WidgetScale(input, w-uw-bw-4, h);
	x += AGWIDGET(input)->w+2;
	if (units != NULL) {
		AGWIDGET(units)->x = x;
		AGWIDGET(units)->y = y;
		AG_WidgetScale(units, uw, h);
		x += AGWIDGET(units)->w+2;
	}
	AGWIDGET(incbu)->x = x;
	AGWIDGET(incbu)->y = y;
	AG_WidgetScale(incbu, bw, h/2);
	y += h/2;
	AGWIDGET(decbu)->x = x;
	AGWIDGET(decbu)->y = y;
	AG_WidgetScale(decbu, bw, h/2);
}

void
AG_FSpinbuttonDraw(void *p)
{
	AG_FSpinbutton *fsu = p;
	AG_WidgetBinding *valueb;
	void *value;

	if (AGWIDGET_FOCUSED(fsu->input))
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
	}
	AG_WidgetUnlockBinding(valueb);
}

/* Add to the value; the fspinbutton must be locked. */
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
	case AG_WIDGET_DOUBLE:
		n = AG_Base2Unit(*(double *)value, fsu->unit);
		if ((n+inc) < *min) {
			n = *min;
		} else if ((n+inc) > *max) {
			n = *max;
		} else {
			n += inc;
		}
		*(double *)value = AG_Unit2Base(n, fsu->unit);
		break;
	case AG_WIDGET_FLOAT:
		n = AG_Base2Unit(*(float *)value, fsu->unit);
		if ((n+inc) < *min) {
			n = *min;
		} else if ((n+inc) > *max) {
			n = *max;
		} else {
			n += inc;
		}
		*(float *)value = AG_Unit2Base(n, fsu->unit);
		break;
	case AG_WIDGET_INT:
		n = AG_Base2Unit((double)(*(int *)value), fsu->unit);
		if ((n+inc) < *min) {
			n = *min;
		} else if ((n+inc) > *max) {
			n = *max;
		} else {
			n += inc;
		}
		*(int *)value = (int)n;
		break;
	case AG_WIDGET_UINT:
		n = AG_Base2Unit((double)(*(Uint *)value), fsu->unit);
		if ((n+inc) < *min) {
			n = *min;
		} else if ((n+inc) > *max) {
			n = *max;
		} else {
			n += inc;
		}
		*(Uint *)value = (Uint)n;
		break;
	case AG_WIDGET_UINT8:
		n = AG_Base2Unit((double)(*(Uint8 *)value), fsu->unit);
		if ((n+inc) < *min) {
			n = *min;
		} else if ((n+inc) > *max) {
			n = *max;
		} else {
			n += inc;
		}
		*(Uint8 *)value = (Uint8)n;
		break;
	default:
		break;
	}
	AG_PostEvent(NULL, fsu, "fspinbutton-changed", NULL);
	AG_WidgetBindingChanged(valueb);

	AG_WidgetUnlockBinding(valueb);
	AG_WidgetUnlockBinding(minb);
	AG_WidgetUnlockBinding(maxb);
}

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
	case AG_WIDGET_DOUBLE:
		*(double *)value = nvalue < *min ? *min :
		                   nvalue > *max ? *max :
				   nvalue;
		break;
	case AG_WIDGET_FLOAT:
		*(float *)value = nvalue < *min ? *min :
		                  nvalue > *max ? *max :
				  nvalue;
		break;
	case AG_WIDGET_INT:
		*(int *)value = nvalue < *min ? (int)*min :
		                nvalue > *max ? (int)*max :
				(int)nvalue;
		break;
	case AG_WIDGET_UINT:
		*(Uint *)value = nvalue < *min ? (Uint)*min :
		                  nvalue > *max ? (Uint)*max :
				  (Uint)nvalue;
		break;
	case AG_WIDGET_UINT8:
		*(Uint8 *)value = nvalue < *min ? (Uint8)*min :
		                  nvalue > *max ? (Uint8)*max :
				  (Uint8)nvalue;
		break;
	}

	AG_PostEvent(NULL, fsu, "fspinbutton-changed", NULL);
	AG_WidgetBindingChanged(valueb);

	AG_WidgetUnlockBinding(valueb);
	AG_WidgetUnlockBinding(minb);
	AG_WidgetUnlockBinding(maxb);
}

void
AG_FSpinbuttonSetMin(AG_FSpinbutton *fsu, double nmin)
{
	AG_WidgetBinding *minb;
	void *min;
	
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
	AG_TextboxSetWriteable(fsu->input, writeable);
	if (writeable) {
		AG_ButtonEnable(fsu->incbu);
		AG_ButtonEnable(fsu->decbu);
	} else {
		AG_ButtonDisable(fsu->incbu);
		AG_ButtonDisable(fsu->decbu);
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
