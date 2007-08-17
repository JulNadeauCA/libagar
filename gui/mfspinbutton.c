/*
 * Copyright (c) 2004-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "mfspinbutton.h"

#include "window.h"

#include <string.h>
#include <limits.h>

AG_MFSpinbutton *
AG_MFSpinbuttonNew(void *parent, Uint flags, const char *unit, const char *sep,
    const char *label)
{
	AG_MFSpinbutton *fsu;

	fsu = Malloc(sizeof(AG_MFSpinbutton), M_OBJECT);
	AG_MFSpinbuttonInit(fsu, flags, unit, sep, label);
	AG_ObjectAttach(parent, fsu);
	return (fsu);
}


/* Adjust the default range depending on the data type of a new binding. */
static void
mfspinbutton_bound(AG_Event *event)
{
	AG_MFSpinbutton *fsu = AG_SELF();
	AG_WidgetBinding *binding = AG_PTR(1);

	if (strcmp(binding->name, "xvalue") == 0 ||
	    strcmp(binding->name, "yvalue") == 0) {
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
		}
		AG_MutexUnlock(&fsu->lock);
	}
}

static void
mfspinbutton_keydown(AG_Event *event)
{
	AG_MFSpinbutton *fsu = AG_SELF();
	int keysym = AG_INT(1);

	AG_MutexLock(&fsu->lock);
	switch (keysym) {
	case SDLK_LEFT:
		AG_MFSpinbuttonAddValue(fsu, "xvalue", -fsu->inc);
		break;
	case SDLK_RIGHT:
		AG_MFSpinbuttonAddValue(fsu, "xvalue", fsu->inc);
		break;
	case SDLK_UP:
		AG_MFSpinbuttonAddValue(fsu, "yvalue", -fsu->inc);
		break;
	case SDLK_DOWN:
		AG_MFSpinbuttonAddValue(fsu, "yvalue", fsu->inc);
		break;
	}
	AG_MutexUnlock(&fsu->lock);
}

static void
mfspinbutton_changed(AG_Event *event)
{
	char text[AG_TEXTBOX_STRING_MAX];
	AG_MFSpinbutton *fsu = AG_PTR(1);
	int unfocus = AG_INT(2);
	AG_WidgetBinding *stringb;
	char *tp = &text[0], *s;

	stringb = AG_WidgetGetBinding(fsu->input, "string", &s);
	strlcpy(text, s, sizeof(text));

	if ((s = AG_Strsep(&tp, fsu->sep)) != NULL) {
		AG_MFSpinbuttonSetValue(fsu, "xvalue",
		    strtod(s, NULL)*fsu->unit->divider);
	}
	if ((s = AG_Strsep(&tp, fsu->sep)) != NULL) {
		AG_MFSpinbuttonSetValue(fsu, "yvalue",
		    strtod(s, NULL)*fsu->unit->divider);
	}
	AG_WidgetUnlockBinding(stringb);

	AG_PostEvent(NULL, fsu, "mfspinbutton-return", NULL);

	if (unfocus)
		AG_WidgetUnfocus(fsu->input);
}

static void
mfspinbutton_up(AG_Event *event)
{
	AG_MFSpinbutton *fsu = AG_PTR(1);

	AG_MutexLock(&fsu->lock);
	AG_MFSpinbuttonAddValue(fsu, "yvalue", -fsu->inc);
	AG_MutexUnlock(&fsu->lock);
}

static void
mfspinbutton_down(AG_Event *event)
{
	AG_MFSpinbutton *fsu = AG_PTR(1);
	
	AG_MutexLock(&fsu->lock);
	AG_MFSpinbuttonAddValue(fsu, "yvalue", fsu->inc);
	AG_MutexUnlock(&fsu->lock);
}

static void
mfspinbutton_left(AG_Event *event)
{
	AG_MFSpinbutton *fsu = AG_PTR(1);
	
	AG_MutexLock(&fsu->lock);
	AG_MFSpinbuttonAddValue(fsu, "xvalue", -fsu->inc);
	AG_MutexUnlock(&fsu->lock);
}

static void
mfspinbutton_right(AG_Event *event)
{
	AG_MFSpinbutton *fsu = AG_PTR(1);
	
	AG_MutexLock(&fsu->lock);
	AG_MFSpinbuttonAddValue(fsu, "xvalue", fsu->inc);
	AG_MutexUnlock(&fsu->lock);
}

static void
update_unit_button(AG_MFSpinbutton *fsu)
{
	AG_ButtonText(fsu->units->button, "%s", AG_UnitAbbr(fsu->unit));
}

static void
selected_unit(AG_Event *event)
{
	AG_UCombo *ucom = AG_SELF();
	AG_MFSpinbutton *fsu = AG_PTR(1);
	AG_TlistItem *ti = AG_PTR(2);

	fsu->unit = (const AG_Unit *)ti->p1;
	update_unit_button(fsu);
}

static void
init_unit_system(AG_MFSpinbutton *fsu, const char *unit_key)
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
AG_MFSpinbuttonInit(AG_MFSpinbutton *fsu, Uint flags, const char *unit,
    const char *sep, const char *label)
{
	Uint wflags = AG_WIDGET_FOCUSABLE;

	if ((flags & AG_MFSPINBUTTON_NOHFILL)==0) { wflags |= AG_WIDGET_HFILL; }
	if (flags & AG_MFSPINBUTTON_VFILL) { wflags |= AG_WIDGET_VFILL; }

	AG_WidgetInit(fsu, &agMFSpinbuttonOps, wflags);
	AG_WidgetBind(fsu, "xvalue", AG_WIDGET_DOUBLE, &fsu->xvalue);
	AG_WidgetBind(fsu, "yvalue", AG_WIDGET_DOUBLE, &fsu->yvalue);
	AG_WidgetBind(fsu, "min", AG_WIDGET_DOUBLE, &fsu->min);
	AG_WidgetBind(fsu, "max", AG_WIDGET_DOUBLE, &fsu->max);
	
	fsu->xvalue = 0.0;
	fsu->yvalue = 0.0;
	fsu->inc = 1.0;
	fsu->input = AG_TextboxNew(fsu, 0, label);
	fsu->writeable = 1;
	fsu->sep = sep;
	AG_MutexInitRecursive(&fsu->lock);
	AG_TextboxPrescale(fsu->input, "888.88");

	strlcpy(fsu->format, "%g", sizeof(fsu->format));
	strlcat(fsu->format, sep, sizeof(fsu->format));
	strlcat(fsu->format, "%g", sizeof(fsu->format));
	
	if (unit != NULL) {
		fsu->units = AG_UComboNew(fsu, 0);
		AG_SetEvent(fsu->units, "ucombo-selected", selected_unit,
		    "%p", fsu);
		init_unit_system(fsu, unit);
	} else {
		fsu->unit = AG_FindUnit("identity");
		fsu->units = NULL;
	}

	fsu->xincbu = AG_ButtonNew(fsu, AG_BUTTON_REPEAT, _("+"));
	AG_ButtonSetPadding(fsu->xincbu, 1,1,1,1);
	AG_SetEvent(fsu->xincbu, "button-pushed", mfspinbutton_right,
	    "%p", fsu);

	fsu->xdecbu = AG_ButtonNew(fsu, AG_BUTTON_REPEAT, _("-"));
	AG_ButtonSetPadding(fsu->xdecbu, 1,1,1,1);
	AG_SetEvent(fsu->xdecbu, "button-pushed", mfspinbutton_left, "%p", fsu);

	fsu->yincbu = AG_ButtonNew(fsu, AG_BUTTON_REPEAT, _("+"));
	AG_ButtonSetPadding(fsu->yincbu, 1,1,1,1);
	AG_SetEvent(fsu->yincbu, "button-pushed", mfspinbutton_down, "%p", fsu);

	fsu->ydecbu = AG_ButtonNew(fsu, AG_BUTTON_REPEAT, _("-"));
	AG_ButtonSetPadding(fsu->ydecbu, 1,1,1,1);
	AG_SetEvent(fsu->ydecbu, "button-pushed", mfspinbutton_up, "%p", fsu);

	AG_SetEvent(fsu, "widget-bound", mfspinbutton_bound, NULL);
	AG_SetEvent(fsu, "window-keydown", mfspinbutton_keydown, NULL);
	AG_SetEvent(fsu->input, "textbox-return", mfspinbutton_changed,
	    "%p,%i", fsu, 1);
	AG_SetEvent(fsu->input, "textbox-changed", mfspinbutton_changed,
	    "%p,%i", fsu, 0);
}

static void
Destroy(void *p)
{
	AG_MFSpinbutton *fsu = p;

	AG_MutexDestroy(&fsu->lock);
	AG_WidgetDestroy(fsu);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_MFSpinbutton *fsu = p;
	AG_SizeReq rChld, rYinc, rYdec;

	AG_WidgetSizeReq(fsu->input, &rChld);
	r->w = rChld.w;
	r->h = rChld.h;
	if (fsu->units != NULL) {
		AG_WidgetSizeReq(fsu->units, &rChld);
		r->w += rChld.w+4;
	}
	AG_WidgetSizeReq(fsu->xdecbu, &rChld);
	r->w += rChld.w;
	AG_WidgetSizeReq(fsu->xincbu, &rChld);
	r->w += rChld.w;
	AG_WidgetSizeReq(fsu->yincbu, &rYinc);
	AG_WidgetSizeReq(fsu->ydecbu, &rYdec);
	r->w += MAX(rYinc.w,rYdec.w);
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_MFSpinbutton *fsu = p;
	int szBtn = a->h/2;
	int wUnitBox = (fsu->units != NULL) ? 25 : 0;
	int x = 0, y = 0;
	AG_SizeAlloc aChld;

	if (a->w < szBtn*3 + wUnitBox + 4)
		return (-1);

	/* Input textbox */
	aChld.x = x;
	aChld.y = y;
	aChld.w = a->w - 2 - wUnitBox - 2 - szBtn*3;
	aChld.h = a->h;
	AG_WidgetSizeAlloc(fsu->input, &aChld);
	x += aChld.w + 2;

	/* Unit selector */
	if (fsu->units != NULL) {
		aChld.x = x;
		aChld.y = y;
		aChld.w = wUnitBox;
		aChld.h = a->h;
		AG_WidgetSizeAlloc(fsu->units, &aChld);
		x += aChld.w + 2;
	}
	
	/* Increment buttons */
	aChld.w = szBtn;
	aChld.h = szBtn;
	aChld.x = x;
	aChld.y = y + szBtn/2;
	AG_WidgetSizeAlloc(fsu->xdecbu, &aChld);
	aChld.x = x + szBtn*2;
	aChld.y = y + szBtn/2;
	AG_WidgetSizeAlloc(fsu->xincbu, &aChld);
	aChld.x = x + szBtn;
	aChld.y = y;
	AG_WidgetSizeAlloc(fsu->ydecbu, &aChld);
	aChld.x = x + szBtn;
	aChld.y = y + szBtn;
	AG_WidgetSizeAlloc(fsu->yincbu, &aChld);
	return (0);
}

static void
Draw(void *p)
{
	AG_MFSpinbutton *fsu = p;
	AG_WidgetBinding *xvalueb, *yvalueb;
	double *xvalue, *yvalue;

	if (AG_WidgetFocused(fsu->input))
		return;

	xvalueb = AG_WidgetGetBinding(fsu, "xvalue", &xvalue);
	yvalueb = AG_WidgetGetBinding(fsu, "yvalue", &yvalue);

	AG_TextboxPrintf(fsu->input, fsu->format,
	    *xvalue/fsu->unit->divider,
	    *yvalue/fsu->unit->divider);

	AG_WidgetUnlockBinding(xvalueb);
	AG_WidgetUnlockBinding(yvalueb);
}

void
AG_MFSpinbuttonAddValue(AG_MFSpinbutton *fsu, const char *which, double inc)
{
	AG_WidgetBinding *valueb, *minb, *maxb;
	void *value;
	double *min, *max;

	inc *= fsu->unit->divider;

	valueb = AG_WidgetGetBinding(fsu, which, &value);
	minb = AG_WidgetGetBinding(fsu, "min", &min);
	maxb = AG_WidgetGetBinding(fsu, "max", &max);

	switch (valueb->vtype) {
	case AG_WIDGET_DOUBLE:
		*(double *)value = *(double *)value+inc < *min ? *min :
		                   *(double *)value+inc > *max ? *max :
				   *(double *)value+inc;
		break;
	case AG_WIDGET_FLOAT:
		*(float *)value = *(float *)value+inc < *min ? *min :
		                  *(float *)value+inc > *max ? *max :
				  *(float *)value+inc;
		break;
	default:
		break;
	}
	AG_PostEvent(NULL, fsu, "mfspinbutton-changed", "%s", which);
	AG_WidgetBindingChanged(valueb);

	AG_WidgetUnlockBinding(valueb);
	AG_WidgetUnlockBinding(minb);
	AG_WidgetUnlockBinding(maxb);
}

void
AG_MFSpinbuttonSetValue(AG_MFSpinbutton *fsu, const char *which,
    double nvalue)
{
	AG_WidgetBinding *valueb, *minb, *maxb;
	void *value;
	double *min, *max;

	valueb = AG_WidgetGetBinding(fsu, which, &value);
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
				  (float)nvalue;
		break;
	}
	AG_PostEvent(NULL, fsu, "mfspinbutton-changed", "%s", which);
	AG_WidgetBindingChanged(valueb);

	AG_WidgetUnlockBinding(valueb);
	AG_WidgetUnlockBinding(minb);
	AG_WidgetUnlockBinding(maxb);
}

void
AG_MFSpinbuttonSetMin(AG_MFSpinbutton *fsu, double nmin)
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
AG_MFSpinbuttonSetMax(AG_MFSpinbutton *fsu, double nmax)
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
AG_MFSpinbuttonSetIncrement(AG_MFSpinbutton *fsu, double inc)
{
	AG_MutexLock(&fsu->lock);
	fsu->inc = inc;
	AG_MutexUnlock(&fsu->lock);
}

void
AG_MFSpinbuttonSetPrecision(AG_MFSpinbutton *fsu, const char *mode,
    int precision)
{
	char ps[8];

	snprintf(ps, sizeof(ps), "%d", precision);

	AG_MutexLock(&fsu->lock);
	snprintf(fsu->format, sizeof(fsu->format), "%%.%d%s%s%%.%d%s",
	    precision, mode, fsu->sep, precision, mode);
	AG_MutexUnlock(&fsu->lock);
}

void
AG_MFSpinbuttonSelectUnit(AG_MFSpinbutton *fsu, const char *uname)
{
	AG_TlistItem *it;

	AG_MutexLock(&fsu->units->list->lock);
	AG_TlistDeselectAll(fsu->units->list);
	TAILQ_FOREACH(it, &fsu->units->list->items, items) {
		const AG_Unit *u = it->p1;

		if (strcmp(u->key, uname) == 0) {
			it->selected++;
			fsu->unit = u;
			update_unit_button(fsu);
			break;
		}
	}
	AG_MutexUnlock(&fsu->units->list->lock);
}

void
AG_MFSpinbuttonSetWriteable(AG_MFSpinbutton *fsu, int writeable)
{
	AG_MutexLock(&fsu->lock);
	fsu->writeable = writeable;
	if (writeable) {
		AG_WidgetEnable(fsu->xincbu);
		AG_WidgetEnable(fsu->xdecbu);
		AG_WidgetEnable(fsu->yincbu);
		AG_WidgetEnable(fsu->ydecbu);
		AG_WidgetEnable(fsu->input);
	} else {
		AG_WidgetDisable(fsu->xincbu);
		AG_WidgetDisable(fsu->xdecbu);
		AG_WidgetDisable(fsu->yincbu);
		AG_WidgetDisable(fsu->ydecbu);
		AG_WidgetDisable(fsu->input);
	}
	AG_MutexUnlock(&fsu->lock);
}

void
AG_MFSpinbuttonSetRange(AG_MFSpinbutton *fsu, double min, double max)
{
	AG_MutexLock(&fsu->lock);
	AG_MFSpinbuttonSetMin(fsu, min);
	AG_MFSpinbuttonSetMax(fsu, max);
	AG_MutexUnlock(&fsu->lock);
}

const AG_WidgetOps agMFSpinbuttonOps = {
	{
		"AG_Widget:AG_MFSpinbutton",
		sizeof(AG_MFSpinbutton),
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
