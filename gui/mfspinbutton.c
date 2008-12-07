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

#include "mfspinbutton.h"
#include "window.h"

#include <string.h>

static void SelectedUnit(AG_Event *);
static void InitUnitSystem(AG_MFSpinbutton *, const char *);

AG_MFSpinbutton *
AG_MFSpinbuttonNew(void *parent, Uint flags, const char *unit, const char *sep,
    const char *label)
{
	AG_MFSpinbutton *fsu;

	fsu = Malloc(sizeof(AG_MFSpinbutton));
	AG_ObjectInit(fsu, &agMFSpinbuttonClass);
	fsu->sep = sep;
	
	if (!(flags & AG_MFSPINBUTTON_NOHFILL))	{ AG_ExpandHoriz(fsu); }
	if (  flags & AG_MFSPINBUTTON_VFILL)	{ AG_ExpandVert(fsu); }

	if (label != NULL) {
		AG_TextboxSetLabel(fsu->input, "%s", label);
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
	AG_MFSpinbutton *fsu = AG_SELF();
	AG_WidgetBinding *binding = AG_PTR(1);

	if (strcmp(binding->name, "xvalue") == 0 ||
	    strcmp(binding->name, "yvalue") == 0) {
		switch (binding->type) {
		case AG_WIDGET_DOUBLE:
			fsu->min = -AG_DBL_MAX+1;
			fsu->max =  AG_DBL_MAX-1;
			break;
		case AG_WIDGET_FLOAT:
			fsu->min = -AG_FLT_MAX+1;
			fsu->max =  AG_FLT_MAX-1;
			break;
		}
	}
}

static void
KeyDown(AG_Event *event)
{
	AG_MFSpinbutton *fsu = AG_SELF();
	int keysym = AG_INT(1);

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
}

static void
TextChanged(AG_Event *event)
{
	char text[AG_TEXTBOX_STRING_MAX];
	AG_MFSpinbutton *fsu = AG_PTR(1);
	int unfocus = AG_INT(2);
	AG_WidgetBinding *stringb;
	char *tp = &text[0], *s;

	AG_ObjectLock(fsu);

	stringb = AG_WidgetGetBinding(fsu->input->ed, "string", &s);
	Strlcpy(text, s, sizeof(text));

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
	
	AG_ObjectUnlock(fsu);
}

static void
DecrementY(AG_Event *event)
{
	AG_MFSpinbutton *fsu = AG_PTR(1);

	AG_ObjectLock(fsu);
	AG_MFSpinbuttonAddValue(fsu, "yvalue", -fsu->inc);
	AG_ObjectUnlock(fsu);
}

static void
IncrementY(AG_Event *event)
{
	AG_MFSpinbutton *fsu = AG_PTR(1);
	
	AG_ObjectLock(fsu);
	AG_MFSpinbuttonAddValue(fsu, "yvalue", fsu->inc);
	AG_ObjectUnlock(fsu);
}

static void
DecrementX(AG_Event *event)
{
	AG_MFSpinbutton *fsu = AG_PTR(1);
	
	AG_ObjectLock(fsu);
	AG_MFSpinbuttonAddValue(fsu, "xvalue", -fsu->inc);
	AG_ObjectUnlock(fsu);
}

static void
IncrementX(AG_Event *event)
{
	AG_MFSpinbutton *fsu = AG_PTR(1);
	
	AG_ObjectLock(fsu);
	AG_MFSpinbuttonAddValue(fsu, "xvalue", fsu->inc);
	AG_ObjectUnlock(fsu);
}

/* Widget must be locked. */
static void
UpdateUnitSelector(AG_MFSpinbutton *fsu)
{
	AG_ButtonText(fsu->units->button, "%s", AG_UnitAbbr(fsu->unit));
}

static void
SelectedUnit(AG_Event *event)
{
	AG_MFSpinbutton *fsu = AG_PTR(1);
	AG_TlistItem *ti = AG_PTR(2);

	AG_ObjectLock(fsu);
	fsu->unit = (const AG_Unit *)ti->p1;
	UpdateUnitSelector(fsu);
	AG_ObjectUnlock(fsu);
}

static void
InitUnitSystem(AG_MFSpinbutton *fsu, const char *unit_key)
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
		AG_FatalError("AG_MFSpinbutton: No such unit: %s", unit_key);
	}
	fsu->unit = unit;
	UpdateUnitSelector(fsu);

	AG_ObjectLock(fsu->units->list);
	AG_TlistDeselectAll(fsu->units->list);
	for (unit = &ugroup[0]; unit->key != NULL; unit++) {
		AG_TlistItem *it;

		it = AG_TlistAddPtr(fsu->units->list, NULL, _(unit->name),
		    (void *)unit);
		if (unit == fsu->unit)
			it->selected++;
	}
	AG_ObjectUnlock(fsu->units->list);
}

static void
Init(void *obj)
{
	AG_MFSpinbutton *fsu = obj;

	WIDGET(fsu)->flags |= AG_WIDGET_FOCUSABLE;

	AG_WidgetBind(fsu, "xvalue", AG_WIDGET_DOUBLE, &fsu->xvalue);
	AG_WidgetBind(fsu, "yvalue", AG_WIDGET_DOUBLE, &fsu->yvalue);
	AG_WidgetBind(fsu, "min", AG_WIDGET_DOUBLE, &fsu->min);
	AG_WidgetBind(fsu, "max", AG_WIDGET_DOUBLE, &fsu->max);

	fsu->xvalue = 0.0;
	fsu->yvalue = 0.0;
	fsu->inc = 1.0;
	fsu->input = AG_TextboxNew(fsu, 0, NULL);
	fsu->writeable = 1;
	fsu->sep = ",";
	Strlcpy(fsu->format, "%.02f", sizeof(fsu->format));
	AG_TextboxSizeHint(fsu->input, "888.88");
	
	fsu->unit = AG_FindUnit("identity");
	fsu->units = NULL;

	fsu->xincbu = AG_ButtonNew(fsu, AG_BUTTON_REPEAT, _("+"));
	fsu->xdecbu = AG_ButtonNew(fsu, AG_BUTTON_REPEAT, _("-"));
	fsu->yincbu = AG_ButtonNew(fsu, AG_BUTTON_REPEAT, _("+"));
	fsu->ydecbu = AG_ButtonNew(fsu, AG_BUTTON_REPEAT, _("-"));
	AG_ButtonSetPadding(fsu->xincbu, 1,1,1,1);
	AG_ButtonSetPadding(fsu->xdecbu, 1,1,1,1);
	AG_ButtonSetPadding(fsu->yincbu, 1,1,1,1);
	AG_ButtonSetPadding(fsu->ydecbu, 1,1,1,1);
	AG_SetEvent(fsu->xincbu, "button-pushed", IncrementX, "%p", fsu);
	AG_SetEvent(fsu->xdecbu, "button-pushed", DecrementX, "%p", fsu);
	AG_SetEvent(fsu->yincbu, "button-pushed", IncrementY, "%p", fsu);
	AG_SetEvent(fsu->ydecbu, "button-pushed", DecrementY, "%p", fsu);
	AG_WidgetSetFocusable(fsu->xincbu, 0);
	AG_WidgetSetFocusable(fsu->xdecbu, 0);
	AG_WidgetSetFocusable(fsu->yincbu, 0);
	AG_WidgetSetFocusable(fsu->ydecbu, 0);

	AG_SetEvent(fsu, "widget-bound", Bound, NULL);
	AG_SetEvent(fsu, "window-keydown", KeyDown, NULL);
	AG_SetEvent(fsu->input, "textbox-return", TextChanged, "%p,%i",fsu,1);
	AG_SetEvent(fsu->input, "textbox-changed", TextChanged, "%p,%i",fsu,0);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_MFSpinbutton *fsu = obj;
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
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_MFSpinbutton *fsu = obj;
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

/* Update the textbox contents from the binding value. */
static void
UpdateTextbox(AG_MFSpinbutton *num)
{
	char sx[64], sy[64];
	AG_WidgetBinding *valueb;
	void *value;

	/* Get X value */
	valueb = AG_WidgetGetBinding(num, "xvalue", &value);
	switch (valueb->type) {
	case AG_WIDGET_DOUBLE:
		Snprintf(sx, sizeof(sx), num->format,
		    AG_Base2Unit(*(double *)value, num->unit));
		break;
	case AG_WIDGET_FLOAT:
		Snprintf(sx, sizeof(sx), num->format,
		    AG_Base2Unit(*(float *)value, num->unit));
		break;
	case AG_WIDGET_INT:
		Snprintf(sx, sizeof(sx), "%d", *(int *)value);
		break;
	case AG_WIDGET_UINT:
		Snprintf(sx, sizeof(sx), "%u", *(Uint *)value);
		break;
	case AG_WIDGET_UINT8:
		Snprintf(sx, sizeof(sx), "%u", (unsigned)(*(Uint8 *)value));
		break;
	case AG_WIDGET_SINT8:
		Snprintf(sx, sizeof(sx), "%d", (int)(*(Sint8 *)value));
		break;
	case AG_WIDGET_UINT16:
		Snprintf(sx, sizeof(sx), "%u", (unsigned)(*(Uint16 *)value));
		break;
	case AG_WIDGET_SINT16:
		Snprintf(sx, sizeof(sx), "%d", (int)(*(Sint16 *)value));
		break;
	case AG_WIDGET_UINT32:
		Snprintf(sx, sizeof(sx), "%u", (unsigned)(*(Uint32 *)value));
		break;
	case AG_WIDGET_SINT32:
		Snprintf(sx, sizeof(sx), "%d", (int)(*(Sint32 *)value));
		break;
#ifdef HAVE_64BIT
	case AG_WIDGET_UINT64:
		Snprintf(sx, sizeof(sx), "%llu",
		    (unsigned long long)(*(Uint64 *)value));
		break;
	case AG_WIDGET_SINT64:
		Snprintf(sx, sizeof(sx), "%lld",
		    (long long)(*(Sint64 *)value));
		break;
#endif
	}
	AG_WidgetUnlockBinding(valueb);
	
	/* Get Y value */
	valueb = AG_WidgetGetBinding(num, "yvalue", &value);
	switch (valueb->type) {
	case AG_WIDGET_DOUBLE:
		Snprintf(sy, sizeof(sy), num->format,
		    AG_Base2Unit(*(double *)value, num->unit));
		break;
	case AG_WIDGET_FLOAT:
		Snprintf(sy, sizeof(sy), num->format,
		    AG_Base2Unit(*(float *)value, num->unit));
		break;
	case AG_WIDGET_INT:
		Snprintf(sy, sizeof(sy), "%d", *(int *)value);
		break;
	case AG_WIDGET_UINT:
		Snprintf(sy, sizeof(sy), "%u", *(Uint *)value);
		break;
	case AG_WIDGET_UINT8:
		Snprintf(sy, sizeof(sy), "%u", (unsigned)(*(Uint8 *)value));
		break;
	case AG_WIDGET_SINT8:
		Snprintf(sy, sizeof(sy), "%d", (int)(*(Sint8 *)value));
		break;
	case AG_WIDGET_UINT16:
		Snprintf(sy, sizeof(sy), "%u", (unsigned)(*(Uint16 *)value));
		break;
	case AG_WIDGET_SINT16:
		Snprintf(sy, sizeof(sy), "%d", (int)(*(Sint16 *)value));
		break;
	case AG_WIDGET_UINT32:
		Snprintf(sy, sizeof(sy), "%u", (unsigned)(*(Uint32 *)value));
		break;
	case AG_WIDGET_SINT32:
		Snprintf(sy, sizeof(sy), "%d", (int)(*(Sint32 *)value));
		break;
#ifdef HAVE_64BIT
	case AG_WIDGET_UINT64:
		Snprintf(sy, sizeof(sy), "%llu",
		    (unsigned long long)(*(Uint64 *)value));
		break;
	case AG_WIDGET_SINT64:
		Snprintf(sy, sizeof(sy), "%lld",
		    (long long)(*(Uint64 *)value));
		break;
#endif
	}
	AG_TextboxPrintf(num->input, "%s%s%s", sx, num->sep, sy);
	AG_WidgetUnlockBinding(valueb);
}

static void
Draw(void *obj)
{
	AG_MFSpinbutton *fsu = obj;
	AG_WidgetBinding *xvalueb, *yvalueb;
	double *xvalue, *yvalue;

	AG_WidgetDraw(fsu->input);
	if (fsu->units != NULL) { AG_WidgetDraw(fsu->units); }
	AG_WidgetDraw(fsu->xincbu);
	AG_WidgetDraw(fsu->yincbu);
	AG_WidgetDraw(fsu->xdecbu);
	AG_WidgetDraw(fsu->ydecbu);

	if (!AG_WidgetFocused(fsu->input))
		UpdateTextbox(fsu);

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

	AG_ObjectLock(fsu);
	
	inc *= fsu->unit->divider;
	valueb = AG_WidgetGetBinding(fsu, which, &value);
	minb = AG_WidgetGetBinding(fsu, "min", &min);
	maxb = AG_WidgetGetBinding(fsu, "max", &max);

	switch (valueb->type) {
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
	
	AG_ObjectUnlock(fsu);
}

void
AG_MFSpinbuttonSetValue(AG_MFSpinbutton *fsu, const char *which,
    double nvalue)
{
	AG_WidgetBinding *valueb, *minb, *maxb;
	void *value;
	double *min, *max;
	
	AG_ObjectLock(fsu);

	valueb = AG_WidgetGetBinding(fsu, which, &value);
	minb = AG_WidgetGetBinding(fsu, "min", &min);
	maxb = AG_WidgetGetBinding(fsu, "max", &max);

	switch (valueb->type) {
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
	
	AG_ObjectUnlock(fsu);
}

void
AG_MFSpinbuttonSetMin(AG_MFSpinbutton *fsu, double nmin)
{
	AG_WidgetBinding *minb;
	void *min;
	
	AG_ObjectLock(fsu);
	minb = AG_WidgetGetBinding(fsu, "min", &min);
	switch (minb->type) {
	case AG_WIDGET_DOUBLE:
		*(double *)min = nmin;
		break;
	case AG_WIDGET_FLOAT:
		*(float *)min = (float)nmin;
		break;
	}
	AG_WidgetUnlockBinding(minb);
	AG_ObjectUnlock(fsu);
}

void
AG_MFSpinbuttonSetMax(AG_MFSpinbutton *fsu, double nmax)
{
	AG_WidgetBinding *maxb;
	void *max;
	
	AG_ObjectLock(fsu);
	maxb = AG_WidgetGetBinding(fsu, "max", &max);
	switch (maxb->type) {
	case AG_WIDGET_DOUBLE:
		*(double *)max = nmax;
		break;
	case AG_WIDGET_FLOAT:
		*(float *)max = (float)nmax;
		break;
	}
	AG_WidgetUnlockBinding(maxb);
	AG_ObjectUnlock(fsu);
}

void
AG_MFSpinbuttonSetIncrement(AG_MFSpinbutton *fsu, double inc)
{
	AG_ObjectLock(fsu);
	fsu->inc = inc;
	AG_ObjectUnlock(fsu);
}

void
AG_MFSpinbuttonSetPrecision(AG_MFSpinbutton *fsu, const char *mode,
    int precision)
{
	AG_ObjectLock(fsu);
	Snprintf(fsu->format, sizeof(fsu->format), "%%.%d%s", precision, mode);
	AG_ObjectUnlock(fsu);
}

void
AG_MFSpinbuttonSelectUnit(AG_MFSpinbutton *fsu, const char *uname)
{
	AG_TlistItem *it;

	AG_ObjectLock(fsu);
	AG_ObjectLock(fsu->units->list);
	AG_TlistDeselectAll(fsu->units->list);
	TAILQ_FOREACH(it, &fsu->units->list->items, items) {
		const AG_Unit *u = it->p1;

		if (strcmp(u->key, uname) == 0) {
			it->selected++;
			fsu->unit = u;
			UpdateUnitSelector(fsu);
			break;
		}
	}
	AG_ObjectUnlock(fsu->units->list);
	AG_ObjectUnlock(fsu);
}

void
AG_MFSpinbuttonSetWriteable(AG_MFSpinbutton *fsu, int writeable)
{
	AG_ObjectLock(fsu);
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
	AG_ObjectUnlock(fsu);
}

void
AG_MFSpinbuttonSetRange(AG_MFSpinbutton *fsu, double min, double max)
{
	AG_ObjectLock(fsu);
	AG_MFSpinbuttonSetMin(fsu, min);
	AG_MFSpinbuttonSetMax(fsu, max);
	AG_ObjectUnlock(fsu);
}

AG_WidgetClass agMFSpinbuttonClass = {
	{
		"Agar(Widget:MFSpinbutton)",
		sizeof(AG_MFSpinbutton),
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
