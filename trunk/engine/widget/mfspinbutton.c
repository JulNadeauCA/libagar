/*	$Csoft: mfspinbutton.c,v 1.6 2005/01/23 11:48:04 vedge Exp $	*/

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

static struct widget_ops mfspinbutton_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		mfspinbutton_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	mfspinbutton_draw,
	mfspinbutton_scale
};

struct mfspinbutton *
mfspinbutton_new(void *parent, const char *unit, const char *sep,
    const char *fmt, ...)
{
	char label[LABEL_MAX];
	struct mfspinbutton *fsu;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(label, sizeof(label), fmt, ap);
	va_end(ap);

	fsu = Malloc(sizeof(struct mfspinbutton), M_OBJECT);
	mfspinbutton_init(fsu, unit, sep, label);
	object_attach(parent, fsu);
	return (fsu);
}


/* Adjust the default range depending on the data type of a new binding. */
static void
mfspinbutton_bound(int argc, union evarg *argv)
{
	struct mfspinbutton *fsu = argv[0].p;
	struct widget_binding *binding = argv[1].p;

	if (strcmp(binding->name, "xvalue") == 0 ||
	    strcmp(binding->name, "yvalue") == 0) {
		pthread_mutex_lock(&fsu->lock);
		switch (binding->vtype) {
		case WIDGET_DOUBLE:
			fsu->min = -DBL_MAX+1;
			fsu->max = DBL_MAX-1;
			break;
		case WIDGET_FLOAT:
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
	struct mfspinbutton *fsu = argv[0].p;
	int keysym = argv[1].i;

	pthread_mutex_lock(&fsu->lock);
	switch (keysym) {
	case SDLK_LEFT:
		mfspinbutton_add_value(fsu, "xvalue", -fsu->inc);
		break;
	case SDLK_RIGHT:
		mfspinbutton_add_value(fsu, "xvalue", fsu->inc);
		break;
	case SDLK_UP:
		mfspinbutton_add_value(fsu, "yvalue", -fsu->inc);
		break;
	case SDLK_DOWN:
		mfspinbutton_add_value(fsu, "yvalue", fsu->inc);
		break;
	}
	pthread_mutex_unlock(&fsu->lock);
}

static void
mfspinbutton_return(int argc, union evarg *argv)
{
	char text[TEXTBOX_STRING_MAX];
	struct mfspinbutton *fsu = argv[1].p;
	struct widget_binding *stringb;
	char *tp = &text[0], *s;

	stringb = widget_get_binding(fsu->input, "string", &s);
	strlcpy(text, s, sizeof(text));

	if ((s = strsep(&tp, fsu->sep)) != NULL) {
		mfspinbutton_set_value(fsu, "xvalue",
		    strtod(s, NULL)*fsu->unit->divider);
	}
	if ((s = strsep(&tp, fsu->sep)) != NULL) {
		mfspinbutton_set_value(fsu, "yvalue",
		    strtod(s, NULL)*fsu->unit->divider);
	}
	widget_binding_unlock(stringb);

	event_post(NULL, fsu, "mfspinbutton-return", NULL);
	WIDGET(fsu->input)->flags &= ~(WIDGET_FOCUSED);
}

static void
mfspinbutton_up(int argc, union evarg *argv)
{
	struct mfspinbutton *fsu = argv[1].p;

	pthread_mutex_lock(&fsu->lock);
	mfspinbutton_add_value(fsu, "yvalue", -fsu->inc);
	pthread_mutex_unlock(&fsu->lock);
}

static void
mfspinbutton_down(int argc, union evarg *argv)
{
	struct mfspinbutton *fsu = argv[1].p;
	
	pthread_mutex_lock(&fsu->lock);
	mfspinbutton_add_value(fsu, "yvalue", fsu->inc);
	pthread_mutex_unlock(&fsu->lock);
}

static void
mfspinbutton_left(int argc, union evarg *argv)
{
	struct mfspinbutton *fsu = argv[1].p;
	
	pthread_mutex_lock(&fsu->lock);
	mfspinbutton_add_value(fsu, "xvalue", -fsu->inc);
	pthread_mutex_unlock(&fsu->lock);
}

static void
mfspinbutton_right(int argc, union evarg *argv)
{
	struct mfspinbutton *fsu = argv[1].p;
	
	pthread_mutex_lock(&fsu->lock);
	mfspinbutton_add_value(fsu, "xvalue", fsu->inc);
	pthread_mutex_unlock(&fsu->lock);
}

static void
update_unit_button(struct mfspinbutton *fsu)
{
	button_printf(fsu->units->button, "%s", unit_abbr(fsu->unit));
}

static void
selected_unit(int argc, union evarg *argv)
{
	struct ucombo *ucom = argv[0].p;
	struct mfspinbutton *fsu = argv[1].p;
	struct tlist_item *ti = argv[2].p;

	fsu->unit = (const struct unit *)ti->p1;
	update_unit_button(fsu);
}

static void
init_unit_system(struct mfspinbutton *fsu, const char *unit_key)
{
	const struct unit *unit = NULL;
	const struct unit *ugroup = NULL;
	int found = 0;
	int i;
	
	for (i = 0; i < nunit_groups; i++) {
		ugroup = unit_groups[i];
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
	tlist_unselect_all(fsu->units->list);
	for (unit = &ugroup[0]; unit->key != NULL; unit++) {
		struct tlist_item *it;

		it = tlist_insert_item(fsu->units->list, NULL, _(unit->name),
		    (void *)unit);
		if (unit == fsu->unit)
			it->selected++;
	}
	pthread_mutex_unlock(&fsu->units->list->lock);
}

void
mfspinbutton_init(struct mfspinbutton *fsu, const char *unit,
    const char *sep, const char *label)
{
	widget_init(fsu, "mfspinbutton", &mfspinbutton_ops,
	    WIDGET_FOCUSABLE|WIDGET_WFILL);
	widget_bind(fsu, "xvalue", WIDGET_DOUBLE, &fsu->xvalue);
	widget_bind(fsu, "yvalue", WIDGET_DOUBLE, &fsu->yvalue);
	widget_bind(fsu, "min", WIDGET_DOUBLE, &fsu->min);
	widget_bind(fsu, "max", WIDGET_DOUBLE, &fsu->max);
	
	fsu->xvalue = 0.0;
	fsu->yvalue = 0.0;
	fsu->inc = 1.0;
	fsu->input = textbox_new(fsu, label);
	fsu->writeable = 1;
	fsu->sep = sep;
	pthread_mutex_init(&fsu->lock, NULL);

	strlcpy(fsu->format, "%g", sizeof(fsu->format));
	strlcat(fsu->format, sep, sizeof(fsu->format));
	strlcat(fsu->format, "%g", sizeof(fsu->format));
	
	if (unit != NULL) {
		fsu->units = ucombo_new(fsu);
		event_new(fsu->units, "ucombo-selected", selected_unit,
		    "%p", fsu);
		init_unit_system(fsu, unit);
	} else {
		fsu->unit = unit_find("identity");
		fsu->units = NULL;
	}

	fsu->xincbu = button_new(fsu, "+");
	button_set_padding(fsu->xincbu, 0);
	button_set_repeat(fsu->xincbu, 1);
	event_new(fsu->xincbu, "button-pushed", mfspinbutton_right, "%p", fsu);

	fsu->xdecbu = button_new(fsu, "-");
	button_set_padding(fsu->xdecbu, 0);
	button_set_repeat(fsu->xdecbu, 1);
	event_new(fsu->xdecbu, "button-pushed", mfspinbutton_left, "%p", fsu);

	fsu->yincbu = button_new(fsu, "+");
	button_set_padding(fsu->yincbu, 0);
	button_set_repeat(fsu->yincbu, 1);
	event_new(fsu->yincbu, "button-pushed", mfspinbutton_down, "%p", fsu);

	fsu->ydecbu = button_new(fsu, "-");
	button_set_padding(fsu->ydecbu, 0);
	button_set_repeat(fsu->ydecbu, 1);
	event_new(fsu->ydecbu, "button-pushed", mfspinbutton_up, "%p", fsu);

	event_new(fsu, "widget-bound", mfspinbutton_bound, NULL);
	event_new(fsu, "window-keydown", mfspinbutton_keydown, NULL);
	event_new(fsu->input, "textbox-return", mfspinbutton_return, "%p", fsu);
}

void
mfspinbutton_destroy(void *p)
{
	struct mfspinbutton *fsu = p;

	pthread_mutex_destroy(&fsu->lock);
	widget_destroy(fsu);
}

void
mfspinbutton_scale(void *p, int w, int h)
{
	struct mfspinbutton *fsu = p;
	struct ucombo *units = fsu->units;
	const int bw = 10;
	int bh = h/2;
	int uw = units != NULL ? 25 : 0;
	int x = 0, y = 0;

	if (w == -1 && h == -1) {
		WIDGET_SCALE(fsu->input, -1, -1);
		WIDGET(fsu)->w = WIDGET(fsu->input)->w;
		WIDGET(fsu)->h = WIDGET(fsu->input)->h;

		if (units != NULL) {
			WIDGET_SCALE(units, -1, -1);
			WIDGET(fsu)->w += WIDGET(units)->w+4;
		}
		WIDGET_SCALE(fsu->yincbu, -1, -1);
		WIDGET_SCALE(fsu->ydecbu, -1, -1);
		WIDGET_SCALE(fsu->xincbu, -1, -1);
		WIDGET_SCALE(fsu->xdecbu, -1, -1);

		WIDGET(fsu)->w += WIDGET(fsu->xdecbu)->w +
		                  max(WIDGET(fsu->yincbu)->w,
				      WIDGET(fsu->ydecbu)->w) +
		                  WIDGET(fsu->xincbu)->w;
		return;
	}

	WIDGET(fsu->input)->x = x;
	WIDGET(fsu->input)->y = y;
	widget_scale(fsu->input, w - 2 - uw - 2 - bw*3, h);
	x += WIDGET(fsu->input)->w+2;

	if (units != NULL) {
		WIDGET(units)->x = x;
		WIDGET(units)->y = y;
		widget_scale(units, uw, h);
		x += WIDGET(units)->w+2;
	}

	WIDGET(fsu->xdecbu)->x = x;
	WIDGET(fsu->xdecbu)->y = y + bh/2;
	widget_scale(fsu->xdecbu, bw, bh);

	WIDGET(fsu->xincbu)->x = x + bh*2;
	WIDGET(fsu->xincbu)->y = y + bh/2;
	widget_scale(fsu->xincbu, bw, bh);

	WIDGET(fsu->ydecbu)->x = x + bh;
	WIDGET(fsu->ydecbu)->y = y;
	widget_scale(fsu->ydecbu, bw, bh);
	
	WIDGET(fsu->yincbu)->x = x + bh;
	WIDGET(fsu->yincbu)->y = y + bh;
	widget_scale(fsu->yincbu, bw, bh);
}

void
mfspinbutton_draw(void *p)
{
	struct mfspinbutton *fsu = p;
	struct widget_binding *xvalueb, *yvalueb;
	double *xvalue, *yvalue;

	if (WIDGET(fsu->input)->flags & WIDGET_FOCUSED)
		return;

	xvalueb = widget_get_binding(fsu, "xvalue", &xvalue);
	yvalueb = widget_get_binding(fsu, "yvalue", &yvalue);

	textbox_printf(fsu->input, fsu->format,
	    *xvalue/fsu->unit->divider,
	    *yvalue/fsu->unit->divider);

	widget_binding_unlock(xvalueb);
	widget_binding_unlock(yvalueb);
}

void
mfspinbutton_add_value(struct mfspinbutton *fsu, const char *which, double inc)
{
	struct widget_binding *valueb, *minb, *maxb;
	void *value;
	double *min, *max;

	inc *= fsu->unit->divider;

	valueb = widget_get_binding(fsu, which, &value);
	minb = widget_get_binding(fsu, "min", &min);
	maxb = widget_get_binding(fsu, "max", &max);

	switch (valueb->vtype) {
	case WIDGET_DOUBLE:
		*(double *)value = *(double *)value+inc < *min ? *min :
		                   *(double *)value+inc > *max ? *max :
				   *(double *)value+inc;
		break;
	case WIDGET_FLOAT:
		*(float *)value = *(float *)value+inc < *min ? *min :
		                  *(float *)value+inc > *max ? *max :
				  *(float *)value+inc;
		break;
	default:
		break;
	}
	event_post(NULL, fsu, "mfspinbutton-changed", "%s", which);
	widget_binding_modified(valueb);

	widget_binding_unlock(valueb);
	widget_binding_unlock(minb);
	widget_binding_unlock(maxb);
}

void
mfspinbutton_set_value(struct mfspinbutton *fsu, const char *which,
    double nvalue)
{
	struct widget_binding *valueb, *minb, *maxb;
	void *value;
	double *min, *max;

	valueb = widget_get_binding(fsu, which, &value);
	minb = widget_get_binding(fsu, "min", &min);
	maxb = widget_get_binding(fsu, "max", &max);

	switch (valueb->vtype) {
	case WIDGET_DOUBLE:
		*(double *)value = nvalue < *min ? *min :
		                   nvalue > *max ? *max :
				   nvalue;
		break;
	case WIDGET_FLOAT:
		*(float *)value = nvalue < *min ? *min :
		                  nvalue > *max ? *max :
				  (float)nvalue;
		break;
	}
	event_post(NULL, fsu, "mfspinbutton-changed", "%s", which);
	widget_binding_modified(valueb);

	widget_binding_unlock(valueb);
	widget_binding_unlock(minb);
	widget_binding_unlock(maxb);
}

void
mfspinbutton_set_min(struct mfspinbutton *fsu, double nmin)
{
	struct widget_binding *minb;
	void *min;
	
	minb = widget_get_binding(fsu, "min", &min);
	switch (minb->vtype) {
	case WIDGET_DOUBLE:
		*(double *)min = nmin;
		break;
	case WIDGET_FLOAT:
		*(float *)min = (float)nmin;
		break;
	}
	widget_binding_unlock(minb);
}

void
mfspinbutton_set_max(struct mfspinbutton *fsu, double nmax)
{
	struct widget_binding *maxb;
	void *max;
	
	maxb = widget_get_binding(fsu, "max", &max);
	switch (maxb->vtype) {
	case WIDGET_DOUBLE:
		*(double *)max = nmax;
		break;
	case WIDGET_FLOAT:
		*(float *)max = (float)nmax;
		break;
	}
	widget_binding_unlock(maxb);
}

void
mfspinbutton_set_increment(struct mfspinbutton *fsu, double inc)
{
	pthread_mutex_lock(&fsu->lock);
	fsu->inc = inc;
	pthread_mutex_unlock(&fsu->lock);
}

void
mfspinbutton_set_precision(struct mfspinbutton *fsu, const char *mode,
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
mfspinbutton_select_unit(struct mfspinbutton *fsu, const char *uname)
{
	struct tlist_item *it;

	pthread_mutex_lock(&fsu->units->list->lock);
	tlist_unselect_all(fsu->units->list);
	TAILQ_FOREACH(it, &fsu->units->list->items, items) {
		const struct unit *u = it->p1;

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
mfspinbutton_set_writeable(struct mfspinbutton *fsu, int writeable)
{
	pthread_mutex_lock(&fsu->lock);
	fsu->writeable = writeable;
	textbox_set_writeable(fsu->input, writeable);
	if (writeable) {
		button_enable(fsu->xincbu);
		button_enable(fsu->xdecbu);
		button_enable(fsu->yincbu);
		button_enable(fsu->ydecbu);
	} else {
		button_disable(fsu->xincbu);
		button_disable(fsu->xdecbu);
		button_disable(fsu->yincbu);
		button_disable(fsu->ydecbu);
	}
	pthread_mutex_unlock(&fsu->lock);
}

void
mfspinbutton_set_range(struct mfspinbutton *fsu, double min, double max)
{
	pthread_mutex_lock(&fsu->lock);
	mfspinbutton_set_min(fsu, min);
	mfspinbutton_set_max(fsu, max);
	pthread_mutex_unlock(&fsu->lock);
}
