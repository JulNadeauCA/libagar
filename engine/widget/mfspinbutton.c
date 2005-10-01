/*	$Csoft: mfspinbutton.c,v 1.9 2005/09/27 00:25:22 vedge Exp $	*/

/*
 * Copyright (c) 2004, 2005 CubeSoft Communications, Inc.
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

#include <engine/engine.h>
#include <engine/view.h>

#include "mfspinbutton.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/label.h>
#include <engine/widget/units.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

static AG_WidgetOps agMFSpinbuttonOps = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		AG_MFSpinbuttonDestroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	AG_MFSpinbuttonDraw,
	AG_MFSpinbuttonScale
};

AG_MFSpinbutton *
AG_MFSpinbuttonNew(void *parent, const char *unit, const char *sep,
    const char *fmt, ...)
{
	char label[AG_LABEL_MAX];
	AG_MFSpinbutton *fsu;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(label, sizeof(label), fmt, ap);
	va_end(ap);

	fsu = Malloc(sizeof(AG_MFSpinbutton), M_OBJECT);
	AG_MFSpinbuttonInit(fsu, unit, sep, label);
	AG_ObjectAttach(parent, fsu);
	return (fsu);
}


/* Adjust the default range depending on the data type of a new binding. */
static void
mfspinbutton_bound(int argc, union evarg *argv)
{
	AG_MFSpinbutton *fsu = argv[0].p;
	AG_WidgetBinding *binding = argv[1].p;

	if (strcmp(binding->name, "xvalue") == 0 ||
	    strcmp(binding->name, "yvalue") == 0) {
		pthread_mutex_lock(&fsu->lock);
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
		pthread_mutex_unlock(&fsu->lock);
	}
}

static void
mfspinbutton_keydown(int argc, union evarg *argv)
{
	AG_MFSpinbutton *fsu = argv[0].p;
	int keysym = argv[1].i;

	pthread_mutex_lock(&fsu->lock);
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
	pthread_mutex_unlock(&fsu->lock);
}

static void
mfspinbutton_changed(int argc, union evarg *argv)
{
	char text[AG_TEXTBOX_STRING_MAX];
	AG_MFSpinbutton *fsu = argv[1].p;
	int unfocus = argv[2].i;
	AG_WidgetBinding *stringb;
	char *tp = &text[0], *s;

	stringb = AG_WidgetGetBinding(fsu->input, "string", &s);
	strlcpy(text, s, sizeof(text));

	if ((s = strsep(&tp, fsu->sep)) != NULL) {
		AG_MFSpinbuttonSetValue(fsu, "xvalue",
		    strtod(s, NULL)*fsu->unit->divider);
	}
	if ((s = strsep(&tp, fsu->sep)) != NULL) {
		AG_MFSpinbuttonSetValue(fsu, "yvalue",
		    strtod(s, NULL)*fsu->unit->divider);
	}
	AG_WidgetUnlockBinding(stringb);

	AG_PostEvent(NULL, fsu, "mfspinbutton-return", NULL);

	if (unfocus)
		AGWIDGET(fsu->input)->flags &= ~(AG_WIDGET_FOCUSED);
}

static void
mfspinbutton_up(int argc, union evarg *argv)
{
	AG_MFSpinbutton *fsu = argv[1].p;

	pthread_mutex_lock(&fsu->lock);
	AG_MFSpinbuttonAddValue(fsu, "yvalue", -fsu->inc);
	pthread_mutex_unlock(&fsu->lock);
}

static void
mfspinbutton_down(int argc, union evarg *argv)
{
	AG_MFSpinbutton *fsu = argv[1].p;
	
	pthread_mutex_lock(&fsu->lock);
	AG_MFSpinbuttonAddValue(fsu, "yvalue", fsu->inc);
	pthread_mutex_unlock(&fsu->lock);
}

static void
mfspinbutton_left(int argc, union evarg *argv)
{
	AG_MFSpinbutton *fsu = argv[1].p;
	
	pthread_mutex_lock(&fsu->lock);
	AG_MFSpinbuttonAddValue(fsu, "xvalue", -fsu->inc);
	pthread_mutex_unlock(&fsu->lock);
}

static void
mfspinbutton_right(int argc, union evarg *argv)
{
	AG_MFSpinbutton *fsu = argv[1].p;
	
	pthread_mutex_lock(&fsu->lock);
	AG_MFSpinbuttonAddValue(fsu, "xvalue", fsu->inc);
	pthread_mutex_unlock(&fsu->lock);
}

static void
update_unit_button(AG_MFSpinbutton *fsu)
{
	AG_ButtonPrintf(fsu->units->button, "%s", AG_UnitAbbr(fsu->unit));
}

static void
selected_unit(int argc, union evarg *argv)
{
	AG_UCombo *ucom = argv[0].p;
	AG_MFSpinbutton *fsu = argv[1].p;
	AG_TlistItem *ti = argv[2].p;

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

	pthread_mutex_lock(&fsu->units->list->lock);
	AG_TlistDeselectAll(fsu->units->list);
	for (unit = &ugroup[0]; unit->key != NULL; unit++) {
		AG_TlistItem *it;

		it = AG_TlistAddPtr(fsu->units->list, NULL, _(unit->name),
		    (void *)unit);
		if (unit == fsu->unit)
			it->selected++;
	}
	pthread_mutex_unlock(&fsu->units->list->lock);
}

void
AG_MFSpinbuttonInit(AG_MFSpinbutton *fsu, const char *unit,
    const char *sep, const char *label)
{
	AG_WidgetInit(fsu, "mfspinbutton", &agMFSpinbuttonOps,
	    AG_WIDGET_FOCUSABLE|AG_WIDGET_WFILL);
	AG_WidgetBind(fsu, "xvalue", AG_WIDGET_DOUBLE, &fsu->xvalue);
	AG_WidgetBind(fsu, "yvalue", AG_WIDGET_DOUBLE, &fsu->yvalue);
	AG_WidgetBind(fsu, "min", AG_WIDGET_DOUBLE, &fsu->min);
	AG_WidgetBind(fsu, "max", AG_WIDGET_DOUBLE, &fsu->max);
	
	fsu->xvalue = 0.0;
	fsu->yvalue = 0.0;
	fsu->inc = 1.0;
	fsu->input = AG_TextboxNew(fsu, label);
	fsu->writeable = 1;
	fsu->sep = sep;
	pthread_mutex_init(&fsu->lock, NULL);

	strlcpy(fsu->format, "%g", sizeof(fsu->format));
	strlcat(fsu->format, sep, sizeof(fsu->format));
	strlcat(fsu->format, "%g", sizeof(fsu->format));
	
	if (unit != NULL) {
		fsu->units = AG_UComboNew(fsu);
		AG_SetEvent(fsu->units, "ucombo-selected", selected_unit,
		    "%p", fsu);
		init_unit_system(fsu, unit);
	} else {
		fsu->unit = AG_FindUnit("identity");
		fsu->units = NULL;
	}

	fsu->xincbu = AG_ButtonNew(fsu, "+");
	AG_ButtonSetPadding(fsu->xincbu, 0);
	AG_ButtonSetRepeatMode(fsu->xincbu, 1);
	AG_SetEvent(fsu->xincbu, "button-pushed", mfspinbutton_right,
	    "%p", fsu);

	fsu->xdecbu = AG_ButtonNew(fsu, "-");
	AG_ButtonSetPadding(fsu->xdecbu, 0);
	AG_ButtonSetRepeatMode(fsu->xdecbu, 1);
	AG_SetEvent(fsu->xdecbu, "button-pushed", mfspinbutton_left, "%p", fsu);

	fsu->yincbu = AG_ButtonNew(fsu, "+");
	AG_ButtonSetPadding(fsu->yincbu, 0);
	AG_ButtonSetRepeatMode(fsu->yincbu, 1);
	AG_SetEvent(fsu->yincbu, "button-pushed", mfspinbutton_down, "%p", fsu);

	fsu->ydecbu = AG_ButtonNew(fsu, "-");
	AG_ButtonSetPadding(fsu->ydecbu, 0);
	AG_ButtonSetRepeatMode(fsu->ydecbu, 1);
	AG_SetEvent(fsu->ydecbu, "button-pushed", mfspinbutton_up, "%p", fsu);

	AG_SetEvent(fsu, "widget-bound", mfspinbutton_bound, NULL);
	AG_SetEvent(fsu, "window-keydown", mfspinbutton_keydown, NULL);
	AG_SetEvent(fsu->input, "textbox-return", mfspinbutton_changed,
	    "%p,%i", fsu, 1);
	AG_SetEvent(fsu->input, "textbox-changed", mfspinbutton_changed,
	    "%p,%i", fsu, 0);
}

void
AG_MFSpinbuttonDestroy(void *p)
{
	AG_MFSpinbutton *fsu = p;

	pthread_mutex_destroy(&fsu->lock);
	AG_WidgetDestroy(fsu);
}

void
AG_MFSpinbuttonScale(void *p, int w, int h)
{
	AG_MFSpinbutton *fsu = p;
	AG_UCombo *units = fsu->units;
	const int bw = 10;
	int bh = h/2;
	int uw = units != NULL ? 25 : 0;
	int x = 0, y = 0;

	if (w == -1 && h == -1) {
		AGWIDGET_SCALE(fsu->input, -1, -1);
		AGWIDGET(fsu)->w = AGWIDGET(fsu->input)->w;
		AGWIDGET(fsu)->h = AGWIDGET(fsu->input)->h;

		if (units != NULL) {
			AGWIDGET_SCALE(units, -1, -1);
			AGWIDGET(fsu)->w += AGWIDGET(units)->w+4;
		}
		AGWIDGET_SCALE(fsu->yincbu, -1, -1);
		AGWIDGET_SCALE(fsu->ydecbu, -1, -1);
		AGWIDGET_SCALE(fsu->xincbu, -1, -1);
		AGWIDGET_SCALE(fsu->xdecbu, -1, -1);

		AGWIDGET(fsu)->w += AGWIDGET(fsu->xdecbu)->w +
		                  max(AGWIDGET(fsu->yincbu)->w,
				      AGWIDGET(fsu->ydecbu)->w) +
		                  AGWIDGET(fsu->xincbu)->w;
		return;
	}

	AGWIDGET(fsu->input)->x = x;
	AGWIDGET(fsu->input)->y = y;
	AG_WidgetScale(fsu->input, w - 2 - uw - 2 - bw*3, h);
	x += AGWIDGET(fsu->input)->w+2;

	if (units != NULL) {
		AGWIDGET(units)->x = x;
		AGWIDGET(units)->y = y;
		AG_WidgetScale(units, uw, h);
		x += AGWIDGET(units)->w+2;
	}

	AGWIDGET(fsu->xdecbu)->x = x;
	AGWIDGET(fsu->xdecbu)->y = y + bh/2;
	AG_WidgetScale(fsu->xdecbu, bw, bh);

	AGWIDGET(fsu->xincbu)->x = x + bh*2;
	AGWIDGET(fsu->xincbu)->y = y + bh/2;
	AG_WidgetScale(fsu->xincbu, bw, bh);

	AGWIDGET(fsu->ydecbu)->x = x + bh;
	AGWIDGET(fsu->ydecbu)->y = y;
	AG_WidgetScale(fsu->ydecbu, bw, bh);
	
	AGWIDGET(fsu->yincbu)->x = x + bh;
	AGWIDGET(fsu->yincbu)->y = y + bh;
	AG_WidgetScale(fsu->yincbu, bw, bh);
}

void
AG_MFSpinbuttonDraw(void *p)
{
	AG_MFSpinbutton *fsu = p;
	AG_WidgetBinding *xvalueb, *yvalueb;
	double *xvalue, *yvalue;

	if (AGWIDGET(fsu->input)->flags & AG_WIDGET_FOCUSED)
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
	pthread_mutex_lock(&fsu->lock);
	fsu->inc = inc;
	pthread_mutex_unlock(&fsu->lock);
}

void
AG_MFSpinbuttonSetPrecision(AG_MFSpinbutton *fsu, const char *mode,
    int precision)
{
	char ps[8];

	snprintf(ps, sizeof(ps), "%d", precision);

	pthread_mutex_lock(&fsu->lock);
	snprintf(fsu->format, sizeof(fsu->format), "%%.%d%s%s%%.%d%s",
	    precision, mode, fsu->sep, precision, mode);
	pthread_mutex_unlock(&fsu->lock);
}

void
AG_MFSpinbuttonSelectUnit(AG_MFSpinbutton *fsu, const char *uname)
{
	AG_TlistItem *it;

	pthread_mutex_lock(&fsu->units->list->lock);
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
	pthread_mutex_unlock(&fsu->units->list->lock);
}

void
AG_MFSpinbuttonSetWriteable(AG_MFSpinbutton *fsu, int writeable)
{
	pthread_mutex_lock(&fsu->lock);
	fsu->writeable = writeable;
	AG_TextboxSetWriteable(fsu->input, writeable);
	if (writeable) {
		AG_ButtonEnable(fsu->xincbu);
		AG_ButtonEnable(fsu->xdecbu);
		AG_ButtonEnable(fsu->yincbu);
		AG_ButtonEnable(fsu->ydecbu);
	} else {
		AG_ButtonDisable(fsu->xincbu);
		AG_ButtonDisable(fsu->xdecbu);
		AG_ButtonDisable(fsu->yincbu);
		AG_ButtonDisable(fsu->ydecbu);
	}
	pthread_mutex_unlock(&fsu->lock);
}

void
AG_MFSpinbuttonSetRange(AG_MFSpinbutton *fsu, double min, double max)
{
	pthread_mutex_lock(&fsu->lock);
	AG_MFSpinbuttonSetMin(fsu, min);
	AG_MFSpinbuttonSetMax(fsu, max);
	pthread_mutex_unlock(&fsu->lock);
}
