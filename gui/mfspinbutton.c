/*
 * Copyright (c) 2004-2012 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/core/core.h>
#include <agar/gui/mfspinbutton.h>
#include <agar/gui/window.h>

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
	fsu->flags |= flags;
	if (!(flags & AG_MFSPINBUTTON_NOHFILL))	{ AG_ExpandHoriz(fsu); }
	if (  flags & AG_MFSPINBUTTON_VFILL)	{ AG_ExpandVert(fsu); }

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

static Uint32
UpdateTimeout(AG_Timer *to, AG_Event *event)
{
	AG_MFSpinbutton *fsu = AG_SELF();
	
	if (!AG_WidgetIsFocused(fsu)) {
		AG_MFSpinbuttonUpdate(fsu);
	}
	return (to->ival);
}

static void
OnShow(AG_Event *event)
{
	AG_MFSpinbutton *fsu = AG_SELF();
	AG_Variable *Vx, *Vy;

	if ((fsu->flags & AG_MFSPINBUTTON_EXCL) == 0) {
		AG_AddTimer(fsu, &fsu->updateTo, 250, UpdateTimeout, NULL);
	}
	if ((Vx = AG_GetVariableLocked(fsu, "xvalue")) == NULL) {
		fsu->xvalue = 0.0;
		Vx = AG_BindDouble(fsu, "xvalue", &fsu->xvalue);
		if (Vx == NULL) {
			return;
		}
		AG_LockVariable(Vx);
	}
	if ((Vy = AG_GetVariableLocked(fsu, "yvalue")) == NULL) {
		fsu->yvalue = 0.0;
		Vy = AG_BindDouble(fsu, "yvalue", &fsu->yvalue);
		if (Vy == NULL) {
			return;
		}
		AG_LockVariable(Vy);
	}
	if (Vx->type != Vy->type) {
		AG_FatalError("MFSpinbutton xvalue/yvalue types disagree");
	}
	switch (Vx->type) {
	case AG_VARIABLE_P_FLOAT:
		if (!AG_Defined(fsu, "min")) {
			fsu->minFlt = -AG_FLT_MAX+1;
			AG_BindFloat(fsu, "min", &fsu->minFlt);
		}
		if (!AG_Defined(fsu, "max")) {
			fsu->maxFlt = +AG_FLT_MAX-1;
			AG_BindFloat(fsu, "max", &fsu->maxFlt);
		}
		break;
	case AG_VARIABLE_P_DOUBLE:
		if (!AG_Defined(fsu, "min")) {
			fsu->min = -AG_DBL_MAX+1;
			AG_BindDouble(fsu, "min", &fsu->min);
		}
		if (!AG_Defined(fsu, "max")) {
			fsu->max = +AG_DBL_MAX-1;
			AG_BindDouble(fsu, "max", &fsu->max);
		}
		break;
	default:
		break;
	}
	AG_MFSpinbuttonUpdate(fsu);
}

/* Update the input text from the binding values. */
void
AG_MFSpinbuttonUpdate(AG_MFSpinbutton *fsu)
{
	char sx[64], sy[64], s[128];
	AG_Variable *valueb;
	void *value;

	/* Get X value */
	valueb = AG_GetVariable(fsu, "xvalue", &value);
	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_DOUBLE:
		Snprintf(sx, sizeof(sx), fsu->format,
		    AG_Base2Unit(*(double *)value, fsu->unit));
		break;
	case AG_VARIABLE_FLOAT:
		Snprintf(sx, sizeof(sx), fsu->format,
		    AG_Base2Unit(*(float *)value, fsu->unit));
		break;
	case AG_VARIABLE_INT:
		StrlcpyInt(sx, *(int *)value, sizeof(sx));
		break;
	case AG_VARIABLE_UINT:
		StrlcpyUint(sx, *(Uint *)value, sizeof(sx));
		break;
	case AG_VARIABLE_UINT8:
		StrlcpyUint(sx, (unsigned)(*(Uint8 *)value), sizeof(sx));
		break;
	case AG_VARIABLE_SINT8:
		StrlcpyInt(sx, (int)(*(Sint8 *)value), sizeof(sx));
		break;
	case AG_VARIABLE_UINT16:
		StrlcpyUint(sx, (unsigned)(*(Uint16 *)value), sizeof(sx));
		break;
	case AG_VARIABLE_SINT16:
		StrlcpyInt(sx, (int)(*(Sint16 *)value), sizeof(sx));
		break;
	case AG_VARIABLE_UINT32:
		StrlcpyUint(sx, (unsigned)(*(Uint32 *)value), sizeof(sx));
		break;
	case AG_VARIABLE_SINT32:
		StrlcpyInt(sx, (int)(*(Sint32 *)value), sizeof(sx));
		break;
	default:
		break;
	}
	AG_UnlockVariable(valueb);
	
	/* Get Y value */
	valueb = AG_GetVariable(fsu, "yvalue", &value);
	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_DOUBLE:
		Snprintf(sy, sizeof(sy), fsu->format,
		    AG_Base2Unit(*(double *)value, fsu->unit));
		break;
	case AG_VARIABLE_FLOAT:
		Snprintf(sy, sizeof(sy), fsu->format,
		    AG_Base2Unit(*(float *)value, fsu->unit));
		break;
	case AG_VARIABLE_INT:
		StrlcpyInt(sy, *(int *)value, sizeof(sy));
		break;
	case AG_VARIABLE_UINT:
		StrlcpyUint(sy, *(Uint *)value, sizeof(sy));
		break;
	case AG_VARIABLE_UINT8:
		StrlcpyUint(sy, (unsigned)(*(Uint8 *)value), sizeof(sy));
		break;
	case AG_VARIABLE_SINT8:
		StrlcpyInt(sy, (int)(*(Sint8 *)value), sizeof(sy));
		break;
	case AG_VARIABLE_UINT16:
		StrlcpyUint(sy, (unsigned)(*(Uint16 *)value), sizeof(sy));
		break;
	case AG_VARIABLE_SINT16:
		StrlcpyInt(sy, (int)(*(Sint16 *)value), sizeof(sy));
		break;
	case AG_VARIABLE_UINT32:
		StrlcpyUint(sy, (unsigned)(*(Uint32 *)value), sizeof(sy));
		break;
	case AG_VARIABLE_SINT32:
		StrlcpyInt(sy, (int)(*(Sint32 *)value), sizeof(sy));
		break;
	default:
		break;
	}
	Strlcpy(s, sx, sizeof(s));
	Strlcat(s, fsu->sep, sizeof(s));
	Strlcat(s, sy, sizeof(s));
	if (strcmp(s, fsu->inTxt) != 0) {
		AG_TextboxSetString(fsu->input, s);
	}
	AG_UnlockVariable(valueb);
}

static void
KeyDown(AG_Event *event)
{
	AG_MFSpinbutton *fsu = AG_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case AG_KEY_LEFT:
		AG_MFSpinbuttonAddValue(fsu, "xvalue", -fsu->inc);
		break;
	case AG_KEY_RIGHT:
		AG_MFSpinbuttonAddValue(fsu, "xvalue", fsu->inc);
		break;
	case AG_KEY_UP:
		AG_MFSpinbuttonAddValue(fsu, "yvalue", -fsu->inc);
		break;
	case AG_KEY_DOWN:
		AG_MFSpinbuttonAddValue(fsu, "yvalue", fsu->inc);
		break;
	default:
		break;
	}
}

static void
TextChanged(AG_Event *event)
{
	AG_MFSpinbutton *fsu = AG_PTR(1);
	int unfocus = AG_INT(2);
	char inTxt[128];
	char *tp = &inTxt[0], *s;

	AG_ObjectLock(fsu);
	Strlcpy(inTxt, fsu->inTxt, sizeof(inTxt));

	if ((s = AG_Strsep(&tp, fsu->sep)) != NULL) {
		AG_MFSpinbuttonSetValue(fsu, "xvalue",
		    strtod(s, NULL)*fsu->unit->divider);
	}
	if ((s = AG_Strsep(&tp, fsu->sep)) != NULL) {
		AG_MFSpinbuttonSetValue(fsu, "yvalue",
		    strtod(s, NULL)*fsu->unit->divider);
	}

	AG_PostEvent(NULL, fsu, "mfspinbutton-return", NULL);

	if (unfocus)
		AG_WidgetUnfocus(fsu->input);
	
	AG_MFSpinbuttonUpdate(fsu);
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
	AG_ButtonTextS(fsu->units->button, AG_UnitAbbr(fsu->unit));
	AG_MFSpinbuttonUpdate(fsu);
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
	AG_TlistSizeHintLargest(fsu->units->list, 5);
	AG_ObjectUnlock(fsu->units->list);
}

static void
Init(void *obj)
{
	AG_MFSpinbutton *fsu = obj;
	AG_Button *b[4];
	int i;

	WIDGET(fsu)->flags |= AG_WIDGET_FOCUSABLE|
	                      AG_WIDGET_TABLE_EMBEDDABLE;

	fsu->inc = 1.0;
	fsu->writeable = 1;
	fsu->sep = ",";
	fsu->inTxt[0] = '\0';
	Strlcpy(fsu->format, "%.02f", sizeof(fsu->format));

	fsu->input = AG_TextboxNewS(fsu, AG_TEXTBOX_EXCL, NULL);
	AG_TextboxBindASCII(fsu->input, fsu->inTxt, sizeof(fsu->inTxt));
	AG_TextboxSizeHint(fsu->input, "888.88");

	fsu->unit = AG_FindUnit("identity");
	fsu->units = NULL;

	fsu->xincbu = b[0] = AG_ButtonNewS(fsu, AG_BUTTON_REPEAT, _("+"));
	fsu->xdecbu = b[1] = AG_ButtonNewS(fsu, AG_BUTTON_REPEAT, _("-"));
	fsu->yincbu = b[2] = AG_ButtonNewS(fsu, AG_BUTTON_REPEAT, _("+"));
	fsu->ydecbu = b[3] = AG_ButtonNewS(fsu, AG_BUTTON_REPEAT, _("-"));
	AG_SetEvent(fsu->xincbu, "button-pushed", IncrementX, "%p", fsu);
	AG_SetEvent(fsu->xdecbu, "button-pushed", DecrementX, "%p", fsu);
	AG_SetEvent(fsu->yincbu, "button-pushed", IncrementY, "%p", fsu);
	AG_SetEvent(fsu->ydecbu, "button-pushed", DecrementY, "%p", fsu);
	for (i = 0; i < 4; i++) {
		AG_ButtonSetPadding(b[i], 0,0,0,0);
		AG_LabelSetPadding(b[i]->lbl, 0,0,0,0);
		AG_WidgetSetFocusable(b[i], 0);
	}

	AG_InitTimer(&fsu->updateTo, "update", 0);

	AG_AddEvent(fsu, "widget-shown", OnShow, NULL);
	AG_SetEvent(fsu, "key-down", KeyDown, NULL);
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

static void
Draw(void *obj)
{
	AG_MFSpinbutton *fsu = obj;
	AG_Variable *xvalueb, *yvalueb;
	double *xvalue, *yvalue;

	AG_WidgetDraw(fsu->input);
	if (fsu->units != NULL) { AG_WidgetDraw(fsu->units); }
	AG_WidgetDraw(fsu->xincbu);
	AG_WidgetDraw(fsu->yincbu);
	AG_WidgetDraw(fsu->xdecbu);
	AG_WidgetDraw(fsu->ydecbu);

	xvalueb = AG_GetVariable(fsu, "xvalue", &xvalue);
	yvalueb = AG_GetVariable(fsu, "yvalue", &yvalue);

	Snprintf(fsu->inTxt, sizeof(fsu->inTxt), fsu->format,
	    *xvalue/fsu->unit->divider,
	    *yvalue/fsu->unit->divider);

	AG_UnlockVariable(xvalueb);
	AG_UnlockVariable(yvalueb);
}

void
AG_MFSpinbuttonAddValue(AG_MFSpinbutton *fsu, const char *which, double inc)
{
	AG_Variable *valueb, *minb, *maxb;
	void *value;
	double *min, *max;

	AG_ObjectLock(fsu);
	
	inc *= fsu->unit->divider;
	valueb = AG_GetVariable(fsu, which, &value);
	minb = AG_GetVariable(fsu, "min", &min);
	maxb = AG_GetVariable(fsu, "max", &max);

	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_DOUBLE:
		*(double *)value = *(double *)value+inc < *min ? *min :
		                   *(double *)value+inc > *max ? *max :
				   *(double *)value+inc;
		break;
	case AG_VARIABLE_FLOAT:
		*(float *)value = *(float *)value+inc < *min ? *min :
		                  *(float *)value+inc > *max ? *max :
				  *(float *)value+inc;
		break;
	default:
		break;
	}
	AG_PostEvent(NULL, fsu, "mfspinbutton-changed", "%s", which);

	AG_UnlockVariable(valueb);
	AG_UnlockVariable(minb);
	AG_UnlockVariable(maxb);
	
	AG_MFSpinbuttonUpdate(fsu);
	AG_ObjectUnlock(fsu);
}

void
AG_MFSpinbuttonSetValue(AG_MFSpinbutton *fsu, const char *which,
    double nvalue)
{
	AG_Variable *valueb, *minb, *maxb;
	void *value;
	double *min, *max;
	
	AG_ObjectLock(fsu);

	valueb = AG_GetVariable(fsu, which, &value);
	minb = AG_GetVariable(fsu, "min", &min);
	maxb = AG_GetVariable(fsu, "max", &max);

	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_DOUBLE:
		*(double *)value = nvalue < *min ? *min :
		                   nvalue > *max ? *max :
				   nvalue;
		break;
	case AG_VARIABLE_FLOAT:
		*(float *)value = nvalue < *min ? *min :
		                  nvalue > *max ? *max :
				  (float)nvalue;
		break;
	default:
		break;
	}
	AG_PostEvent(NULL, fsu, "mfspinbutton-changed", "%s", which);

	AG_UnlockVariable(valueb);
	AG_UnlockVariable(minb);
	AG_UnlockVariable(maxb);
	
	AG_MFSpinbuttonUpdate(fsu);
	AG_ObjectUnlock(fsu);
}

void
AG_MFSpinbuttonSetMin(AG_MFSpinbutton *fsu, double nmin)
{
	AG_Variable *minb;
	void *min;
	
	AG_ObjectLock(fsu);
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
	AG_ObjectUnlock(fsu);
}

void
AG_MFSpinbuttonSetMax(AG_MFSpinbutton *fsu, double nmax)
{
	AG_Variable *maxb;
	void *max;
	
	AG_ObjectLock(fsu);
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
	AG_MFSpinbuttonUpdate(fsu);
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
	AG_Redraw(fsu);
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
