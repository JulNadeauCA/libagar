/*	$Csoft: fspinbutton.c,v 1.22 2004/08/22 12:08:15 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004 CubeSoft Communications, Inc.
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

#include "fspinbutton.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/label.h>
#include <engine/widget/units.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

static struct widget_ops fspinbutton_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		fspinbutton_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	fspinbutton_draw,
	fspinbutton_scale
};

struct fspinbutton *
fspinbutton_new(void *parent, const char *unit, const char *fmt, ...)
{
	char label[LABEL_MAX];
	struct fspinbutton *fsu;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(label, sizeof(label), fmt, ap);
	va_end(ap);

	fsu = Malloc(sizeof(struct fspinbutton), M_OBJECT);
	fspinbutton_init(fsu, unit, label);
	object_attach(parent, fsu);
	return (fsu);
}


/* Adjust the default range depending on the data type of a new binding. */
static void
binding_changed(int argc, union evarg *argv)
{
	struct fspinbutton *fsu = argv[0].p;
	struct widget_binding *binding = argv[1].p;

	if (strcmp(binding->name, "value") == 0) {
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
key_pressed(int argc, union evarg *argv)
{
	struct fspinbutton *fsu = argv[0].p;
	int keysym = argv[1].i;

	pthread_mutex_lock(&fsu->lock);
	switch (keysym) {
	case SDLK_UP:
		fspinbutton_add_value(fsu, fsu->inc);
		break;
	case SDLK_DOWN:
		fspinbutton_add_value(fsu, -fsu->inc);
		break;
	}
	pthread_mutex_unlock(&fsu->lock);
}

static void
return_pressed(int argc, union evarg *argv)
{
	struct fspinbutton *fsu = argv[1].p;
	struct widget_binding *stringb;
	char *s;

	stringb = widget_get_binding(fsu->input, "string", &s);
	fspinbutton_set_value(fsu, unit2base(strtod(s, NULL), fsu->unit));
	widget_binding_unlock(stringb);

	event_post(NULL, fsu, "fspinbutton-return", NULL);
	WIDGET(fsu->input)->flags &= ~(WIDGET_FOCUSED);
}

static void
increment_pressed(int argc, union evarg *argv)
{
	struct fspinbutton *fsu = argv[1].p;

	pthread_mutex_lock(&fsu->lock);
	fspinbutton_add_value(fsu, fsu->inc);
	pthread_mutex_unlock(&fsu->lock);
}

static void
decrement_pressed(int argc, union evarg *argv)
{
	struct fspinbutton *fsu = argv[1].p;
	
	pthread_mutex_lock(&fsu->lock);
	fspinbutton_add_value(fsu, -fsu->inc);
	pthread_mutex_unlock(&fsu->lock);
}

static void
update_unit_button(struct fspinbutton *fsu)
{
	button_printf(fsu->units->button, "%s", unit_abbr(fsu->unit));
}

static void
selected_unit(int argc, union evarg *argv)
{
	struct ucombo *ucom = argv[0].p;
	struct fspinbutton *fsu = argv[1].p;
	struct tlist_item *ti = argv[2].p;

	fsu->unit = (const struct unit *)ti->p1;
	update_unit_button(fsu);
}

static void
init_unit_system(struct fspinbutton *fsu, const char *unit_key)
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
		    unit);
		if (unit == fsu->unit)
			it->selected++;
	}
	pthread_mutex_unlock(&fsu->units->list->lock);
}

void
fspinbutton_init(struct fspinbutton *fsu, const char *unit, const char *label)
{
	widget_init(fsu, "fspinbutton", &fspinbutton_ops,
	    WIDGET_FOCUSABLE|WIDGET_WFILL);
	widget_bind(fsu, "value", WIDGET_DOUBLE, &fsu->value);
	widget_bind(fsu, "min", WIDGET_DOUBLE, &fsu->min);
	widget_bind(fsu, "max", WIDGET_DOUBLE, &fsu->max);
	
	fsu->value = 0.0;
	fsu->inc = 1.0;
	fsu->input = textbox_new(fsu, label);
	fsu->writeable = 1;
	strlcpy(fsu->format, "%.2f", sizeof(fsu->format));
	pthread_mutex_init(&fsu->lock, NULL);
	
	if (unit != NULL) {
		fsu->units = ucombo_new(fsu);
		event_new(fsu->units, "ucombo-selected", selected_unit,
		    "%p", fsu);
		init_unit_system(fsu, unit);
	} else {
		fsu->unit = unit_find("identity");
		fsu->units = NULL;
	}

	fsu->incbu = button_new(fsu, "+");
	fsu->decbu = button_new(fsu, "-");
	button_set_padding(fsu->incbu, 0);
	button_set_padding(fsu->decbu, 0);

	event_new(fsu, "widget-bound", binding_changed, NULL);
	event_new(fsu, "window-keydown", key_pressed, NULL);
	event_new(fsu->input, "textbox-return", return_pressed, "%p", fsu);
	event_new(fsu->incbu, "button-pushed", increment_pressed, "%p", fsu);
	event_new(fsu->decbu, "button-pushed", decrement_pressed, "%p", fsu);
}

void
fspinbutton_destroy(void *p)
{
	struct fspinbutton *fsu = p;

	pthread_mutex_destroy(&fsu->lock);
	widget_destroy(fsu);
}

void
fspinbutton_scale(void *p, int w, int h)
{
	struct fspinbutton *fsu = p;
	struct textbox *input = fsu->input;
	struct ucombo *units = fsu->units;
	struct button *incbu = fsu->incbu;
	struct button *decbu = fsu->decbu;
	const int bw = 10;
	int x = 0, y = 0;
	int uw, uh;

	if (units != NULL) {
		text_prescale("XXXXXXXX", &uw, &uh);
	} else {
		uw = 0;
		uh = 0;
	}

	if (w == -1 && h == -1) {
		WIDGET_SCALE(input, -1, -1);
		WIDGET(fsu)->w = WIDGET(input)->w;
		WIDGET(fsu)->h = WIDGET(input)->h;
		x += WIDGET(fsu)->w;

		if (units != NULL) {
			WIDGET_SCALE(units, -1, -1);
			WIDGET(fsu)->w += WIDGET(units)->w;
			x += WIDGET(units)->w;
		}

		WIDGET_SCALE(incbu, -1, -1);
		WIDGET(fsu)->w += WIDGET(incbu)->w;
		y += WIDGET(fsu)->h;
		return;
	}

	WIDGET(input)->x = 0;
	WIDGET(input)->y = 0;
	widget_scale(input, w-uw-bw-4, h);
	x += WIDGET(input)->w+2;
	if (units != NULL) {
		WIDGET(units)->x = x;
		WIDGET(units)->y = y;
		widget_scale(units, uw, h);
		x += WIDGET(units)->w+2;
	}
	WIDGET(incbu)->x = x;
	WIDGET(incbu)->y = y;
	widget_scale(incbu, bw, h/2);
	y += h/2;
	WIDGET(decbu)->x = x;
	WIDGET(decbu)->y = y;
	widget_scale(decbu, bw, h/2);
}

void
fspinbutton_draw(void *p)
{
	struct fspinbutton *fsu = p;
	struct widget_binding *valueb;
	void *value;

	if (WIDGET(fsu->input)->flags & WIDGET_FOCUSED)
		return;

	valueb = widget_get_binding(fsu, "value", &value);
	switch (valueb->vtype) {
	case WIDGET_DOUBLE:
		textbox_printf(fsu->input, fsu->format,
		    base2unit(*(double *)value, fsu->unit));
		break;
	case WIDGET_FLOAT:
		textbox_printf(fsu->input, fsu->format,
		    base2unit(*(float *)value, fsu->unit));
		break;
	}
	widget_binding_unlock(valueb);
}

/* Add to the value; the fspinbutton must be locked. */
void
fspinbutton_add_value(struct fspinbutton *fsu, double inc)
{
	struct widget_binding *valueb, *minb, *maxb;
	void *value;
	double *min, *max;
	double binc;

	binc = unit2base(inc, fsu->unit);
	valueb = widget_get_binding(fsu, "value", &value);
	minb = widget_get_binding(fsu, "min", &min);
	maxb = widget_get_binding(fsu, "max", &max);

	switch (valueb->vtype) {
	case WIDGET_DOUBLE:
		*(double *)value = *(double *)value+binc < *min ? *min :
		                   *(double *)value+binc > *max ? *max :
				   *(double *)value+binc;
		
		break;
	case WIDGET_FLOAT:
		*(float *)value = *(float *)value+binc < *min ? *min :
		                  *(float *)value+binc > *max ? *max :
				  *(float *)value+binc;
		break;
	default:
		break;
	}
	event_post(NULL, fsu, "fspinbutton-changed", NULL);
	widget_binding_modified(valueb);

	widget_binding_unlock(valueb);
	widget_binding_unlock(minb);
	widget_binding_unlock(maxb);
}

void
fspinbutton_set_value(struct fspinbutton *fsu, double nvalue)
{
	struct widget_binding *valueb, *minb, *maxb;
	void *value;
	double *min, *max;

	valueb = widget_get_binding(fsu, "value", &value);
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
				  nvalue;
		break;
	}

	event_post(NULL, fsu, "fspinbutton-changed", NULL);
	widget_binding_modified(valueb);

	widget_binding_unlock(valueb);
	widget_binding_unlock(minb);
	widget_binding_unlock(maxb);
}

void
fspinbutton_set_min(struct fspinbutton *fsu, double nmin)
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
fspinbutton_set_max(struct fspinbutton *fsu, double nmax)
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
fspinbutton_set_increment(struct fspinbutton *fsu, double inc)
{
	pthread_mutex_lock(&fsu->lock);
	fsu->inc = inc;
	pthread_mutex_unlock(&fsu->lock);
}

void
fspinbutton_set_precision(struct fspinbutton *fsu, const char *mode,
    int precision)
{
	pthread_mutex_lock(&fsu->lock);
	fsu->format[0] = '%';
	fsu->format[1] = '.';
	snprintf(&fsu->format[2], sizeof(fsu->format)-2, "%d", precision);
	strlcat(fsu->format, mode, sizeof(fsu->format));
	pthread_mutex_unlock(&fsu->lock);
}

void
fspinbutton_select_unit(struct fspinbutton *fsu, const char *uname)
{
	struct tlist_item *it;

	pthread_mutex_lock(&fsu->units->list->lock);
	tlist_unselect_all(fsu->units->list);
	TAILQ_FOREACH(it, &fsu->units->list->items, items) {
		const struct unit *unit = it->p1;

		if (strcmp(unit->key, uname) == 0) {
			it->selected++;
			fsu->unit = unit;
			update_unit_button(fsu);
			break;
		}
	}
	pthread_mutex_unlock(&fsu->units->list->lock);
}

void
fspinbutton_set_writeable(struct fspinbutton *fsu, int writeable)
{
	pthread_mutex_lock(&fsu->lock);
	fsu->writeable = writeable;
	textbox_set_writeable(fsu->input, writeable);
	if (writeable) {
		button_enable(fsu->incbu);
		button_enable(fsu->decbu);
	} else {
		button_disable(fsu->incbu);
		button_disable(fsu->decbu);
	}
	pthread_mutex_unlock(&fsu->lock);
}

void
fspinbutton_set_range(struct fspinbutton *fsu, double min, double max)
{
	pthread_mutex_lock(&fsu->lock);
	fspinbutton_set_min(fsu, min);
	fspinbutton_set_max(fsu, max);
	pthread_mutex_unlock(&fsu->lock);
}
