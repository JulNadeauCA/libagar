/*	$Csoft: fspinbutton.c,v 1.10 2004/01/23 08:57:21 vedge Exp $	*/

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

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <compat/math.h>
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

static void	fspinbutton_unitsel(int, union evarg *);

struct fspinbutton *
fspinbutton_new(void *parent, const struct unit *unit, const char *fmt, ...)
{
	char label[LABEL_MAX];
	struct fspinbutton *fsu;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(label, sizeof(label), fmt, ap);
	va_end(ap);

	fsu = Malloc(sizeof(struct fspinbutton));
	fspinbutton_init(fsu, unit, label);
	object_attach(parent, fsu);
	return (fsu);
}


/* Adjust the default range depending on the data type of a new binding. */
static void
fspinbutton_bound(int argc, union evarg *argv)
{
	struct fspinbutton *fsu = argv[0].p;
	struct widget_binding *binding = argv[1].p;

	if (strcmp(binding->name, "value") == 0) {
		pthread_mutex_lock(&fsu->lock);
		switch (binding->type) {
		case WIDGET_DOUBLE:
			fsu->min = -DBL_MAX+1;
			fsu->max = DBL_MAX-1;
			break;
		case WIDGET_FLOAT:
			fsu->min = -FLT_MAX+1;
			fsu->max = FLT_MAX-1;
			break;
		}
		textbox_printf(fsu->input, fsu->format, *(double *)binding->p1 /
		    fsu->unit->divider);
		pthread_mutex_unlock(&fsu->lock);
	}
}

static void
fspinbutton_keydown(int argc, union evarg *argv)
{
	struct fspinbutton *fsu = argv[0].p;
	int keysym = argv[1].i;

	pthread_mutex_lock(&fsu->lock);
	switch (keysym) {
	case SDLK_UP:
		fspinbutton_add(fsu, fsu->incr);
		break;
	case SDLK_DOWN:
		fspinbutton_add(fsu, -fsu->incr);
		break;
	}
	pthread_mutex_unlock(&fsu->lock);
}

static void
fspinbutton_return(int argc, union evarg *argv)
{
	struct fspinbutton *fsu = argv[1].p;
	struct widget_binding *stringb;
	char *s;

	stringb = widget_get_binding(fsu->input, "string", &s);
	fspinbutton_set_value(fsu, strtod(s, NULL) * fsu->unit->divider);
	widget_binding_unlock(stringb);

	WIDGET(fsu->input)->flags &= ~(WIDGET_FOCUSED);

	event_post(NULL, fsu, "fspinbutton-return", NULL);
	event_post(NULL, fsu, "fspinbutton-changed", NULL);
}

static void
fspinbutton_up(int argc, union evarg *argv)
{
	struct fspinbutton *fsu = argv[1].p;

	pthread_mutex_lock(&fsu->lock);
	fspinbutton_add(fsu, fsu->incr);
	pthread_mutex_unlock(&fsu->lock);
	
	event_post(NULL, fsu, "fspinbutton-changed", NULL);
}

static void
fspinbutton_down(int argc, union evarg *argv)
{
	struct fspinbutton *fsu = argv[1].p;
	
	pthread_mutex_lock(&fsu->lock);
	fspinbutton_add(fsu, -fsu->incr);
	pthread_mutex_unlock(&fsu->lock);
	
	event_post(NULL, fsu, "fspinbutton-changed", NULL);
}

static void
fspinbutton_unitsel(int argc, union evarg *argv)
{
	struct ucombo *ucom = argv[0].p;
	struct fspinbutton *fsu = argv[1].p;
	struct tlist_item *ti = argv[2].p;
	const struct unit *unit = ti->p1;

	fsu->unit = unit;
	button_printf(ucom->button, "%s", unit->abbr);
}

void
fspinbutton_init(struct fspinbutton *fsu, const struct unit *unit,
    const char *label)
{
	widget_init(fsu, "fspinbutton", &fspinbutton_ops, WIDGET_FOCUSABLE|
	    WIDGET_WFILL);
	widget_bind(fsu, "value", WIDGET_DOUBLE, &fsu->value);
	
	fsu->value = 0;
	fsu->incr = 1;
	fsu->input = textbox_new(fsu, label);
	fsu->writeable = 1;
	strlcpy(fsu->format, "%.10g", sizeof(fsu->format));
	pthread_mutex_init(&fsu->lock, NULL);
	
	if (unit != NULL) {
		fsu->units = ucombo_new(fsu);
		event_new(fsu->units, "ucombo-selected", fspinbutton_unitsel,
		    "%p", fsu);
		fspinbutton_set_units(fsu, unit);
	} else {
		fsu->unit = &identity_unit;
		fsu->units = NULL;
	}

	fsu->incbu = button_new(fsu, _("+"));
	fsu->decbu = button_new(fsu, _("-"));
	button_set_padding(fsu->incbu, 0);
	button_set_padding(fsu->decbu, 0);

	event_new(fsu, "widget-bound", fspinbutton_bound, NULL);
	event_new(fsu, "window-keydown", fspinbutton_keydown, NULL);
	event_new(fsu->input, "textbox-return", fspinbutton_return, "%p", fsu);
	event_new(fsu->incbu, "button-pushed", fspinbutton_up, "%p", fsu);
	event_new(fsu->decbu, "button-pushed", fspinbutton_down, "%p", fsu);
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
	int x = 0, y = 0;

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

	WIDGET(input)->x = x;
	WIDGET(input)->y = y;

	/* XXX prescale */
	if (units != NULL) {
		widget_scale(input, w - 55 - 10, h);
	} else {
		widget_scale(input, w - 10, h);
	}
	x += WIDGET(input)->w;

	if (units != NULL) {
		WIDGET(units)->x = x;
		WIDGET(units)->y = y;
		widget_scale(units, 55, h);
		x += WIDGET(units)->w;
	}

	WIDGET(incbu)->x = x;
	WIDGET(incbu)->y = y;
	widget_scale(incbu, 10, h/2);
	y += h/2;

	WIDGET(decbu)->x = x;
	WIDGET(decbu)->y = y;
	widget_scale(decbu, 10, h/2);
}

void
fspinbutton_draw(void *p)
{
	struct fspinbutton *fsu = p;
	struct widget_binding *valueb;
	double *value;

	if (WIDGET(fsu->input)->flags & WIDGET_FOCUSED) {
		/* Assume edition is in progress. */
		return;
	}

	valueb = widget_get_binding(fsu, "value", &value);
	textbox_printf(fsu->input, fsu->format, *value / fsu->unit->divider);
	widget_binding_unlock(valueb);
}

/* Add to the value; the fspinbutton must be locked. */
void
fspinbutton_add(struct fspinbutton *fsu, double inc)
{
	struct widget_binding *valueb;
	void *value;

	valueb = widget_get_binding(fsu, "value", &value);
	switch (valueb->type) {
	case WIDGET_DOUBLE:
		if (*(double *)value + inc >= fsu->min &&
		    *(double *)value + inc <= fsu->max)
			*(double *)value += inc * fsu->unit->divider;
		break;
	case WIDGET_FLOAT:
		if (*(float *)value + inc >= fsu->min &&
		    *(float *)value + inc <= fsu->max)
			*(float *)value += (float)inc * fsu->unit->divider;
		break;
	default:
		break;
	}
	event_post(NULL, fsu, "fspinbutton-changed", NULL);
	widget_binding_modified(valueb);
	widget_binding_unlock(valueb);
}

/* Alter the value bound to the spinbutton. */
void
fspinbutton_set_value(struct fspinbutton *fsu, double nvalue)
{
	struct widget_binding *valueb;
	void *value;

	valueb = widget_get_binding(fsu, "value", &value);
	switch (valueb->type) {
	case WIDGET_DOUBLE:
		*(double *)value = nvalue;
		break;
	case WIDGET_FLOAT:
		*(float *)value = (float)nvalue;
		break;
	}
	event_post(NULL, fsu, "fspinbutton-changed", NULL);
	widget_binding_modified(valueb);
	widget_binding_unlock(valueb);
}

/* Set the minimum value. */
void
fspinbutton_set_min(struct fspinbutton *fsu, double nmin)
{
	pthread_mutex_lock(&fsu->lock);
	fsu->min = nmin;
	pthread_mutex_unlock(&fsu->lock);
}

/* Set the maximum value. */
void
fspinbutton_set_max(struct fspinbutton *fsu, double nmax)
{
	pthread_mutex_lock(&fsu->lock);
	fsu->max = nmax;
	pthread_mutex_unlock(&fsu->lock);
}

/* Set the increment for [+] and [-] buttons. */
void
fspinbutton_set_increment(struct fspinbutton *fsu, float incr)
{
	pthread_mutex_lock(&fsu->lock);
	fsu->incr = incr;
	pthread_mutex_unlock(&fsu->lock);
}

/* Set the precision to display. */
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

/* Select which unit system to use. */
void
fspinbutton_set_units(struct fspinbutton *fsu, const struct unit units[])
{
	const struct unit *unit;

	pthread_mutex_lock(&fsu->units->list->lock);
	tlist_unselect_all(fsu->units->list);
	for (unit = &units[0]; unit->abbr != NULL; unit++) {
		struct tlist_item *it;

		it = tlist_insert_item(fsu->units->list, NULL, _(unit->name),
		    unit);
		if (unit->divider == 1) {
			it->selected++;
			fsu->unit = unit;
			button_printf(fsu->units->button, _(unit->abbr));
		}
	}
	pthread_mutex_unlock(&fsu->units->list->lock);
}

/* Select the unit to use. */
void
fspinbutton_select_unit(struct fspinbutton *fsu, const char *uname)
{
	struct tlist_item *it;

	pthread_mutex_lock(&fsu->units->list->lock);
	tlist_unselect_all(fsu->units->list);
	TAILQ_FOREACH(it, &fsu->units->list->items, items) {
		const struct unit *u = it->p1;

		if (strcmp(u->abbr, uname) == 0) {
			it->selected++;
			fsu->unit = u;
			button_printf(fsu->units->button, "%s", u->abbr);
			break;
		}
	}
	pthread_mutex_unlock(&fsu->units->list->lock);
}

/* Set the writeability of a fspinbutton. */
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

/* Define the range of acceptable values. */
void
fspinbutton_set_range(struct fspinbutton *fsu, double min, double max)
{
	pthread_mutex_lock(&fsu->lock);
	fsu->min = min;
	fsu->max = max;
	pthread_mutex_unlock(&fsu->lock);
}
