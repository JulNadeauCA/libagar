/*	$Csoft: spinbutton.c,v 1.6 2003/11/09 10:55:43 vedge Exp $	*/

/*
 * Copyright (c) 2003 CubeSoft Communications, Inc.
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
fspinbutton_new(void *parent, const char *fmt, ...)
{
	char label[LABEL_MAX];
	struct fspinbutton *fsu;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(label, sizeof(label), fmt, ap);
	va_end(ap);

	fsu = Malloc(sizeof(struct fspinbutton));
	fspinbutton_init(fsu, label);
	object_attach(parent, fsu);
	return (fsu);
}

static void
fspinbutton_bound(int argc, union evarg *argv)
{
	struct fspinbutton *fsu = argv[0].p;
	struct widget_binding *binding = argv[1].p;

	if (strcmp(binding->name, "value") == 0) {
		pthread_mutex_lock(&fsu->lock);
		switch (binding->type) {
		case WIDGET_FLOAT:
			fsu->min = -FLT_MAX+1;
			fsu->max = FLT_MAX-1;
			break;
		case WIDGET_DOUBLE:
			fsu->min = -DBL_MAX+1;
			fsu->max = DBL_MAX-1;
			break;
		default:
			break;
		}
		textbox_printf(fsu->tbox, fsu->format, *(double *)binding->p1);
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

	stringb = widget_get_binding(fsu->tbox, "string", &s);
	fspinbutton_set_value(fsu, strtod(s, NULL));
	widget_binding_unlock(stringb);

	WIDGET(fsu->tbox)->flags &= ~(WIDGET_FOCUSED);

	event_post(fsu, "fspinbutton-return", NULL);
	event_post(fsu, "fspinbutton-changed", NULL);
}

static void
fspinbutton_up(int argc, union evarg *argv)
{
	struct fspinbutton *fsu = argv[1].p;

	pthread_mutex_lock(&fsu->lock);
	fspinbutton_add(fsu, fsu->incr);
	pthread_mutex_unlock(&fsu->lock);
	
	event_post(fsu, "fspinbutton-changed", NULL);
}

static void
fspinbutton_down(int argc, union evarg *argv)
{
	struct fspinbutton *fsu = argv[1].p;
	
	pthread_mutex_lock(&fsu->lock);
	fspinbutton_add(fsu, -fsu->incr);
	pthread_mutex_unlock(&fsu->lock);
	
	event_post(fsu, "fspinbutton-changed", NULL);
}

void
fspinbutton_init(struct fspinbutton *fsu, const char *label)
{
	widget_init(fsu, "fspinbutton", &fspinbutton_ops,
	    WIDGET_FOCUSABLE|WIDGET_WFILL);
	
	fsu->value = 0;
	fsu->incr = 0.1;
	fsu->tbox = textbox_new(fsu, label);
	pthread_mutex_init(&fsu->lock, NULL);

	event_new(fsu, "widget-bound", fspinbutton_bound, NULL);
	event_new(fsu, "window-keydown", fspinbutton_keydown, NULL);

	widget_bind(fsu, "value", WIDGET_DOUBLE, &fsu->value);
	widget_bind(fsu, "min", WIDGET_DOUBLE, &fsu->min);
	widget_bind(fsu, "max", WIDGET_DOUBLE, &fsu->max);

	fsu->incbu = button_new(fsu, "+");
	fsu->decbu = button_new(fsu, "-");
	button_set_padding(fsu->incbu, 0);
	button_set_padding(fsu->decbu, 0);
	
	event_new(fsu->tbox, "textbox-return", fspinbutton_return, "%p", fsu);
	event_new(fsu->incbu, "button-pushed", fspinbutton_up, "%p", fsu);
	event_new(fsu->decbu, "button-pushed", fspinbutton_down, "%p", fsu);

	strlcpy(fsu->format, "%.2f", sizeof(fsu->format));
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
	struct textbox *tbox = fsu->tbox;
	struct button *incbu = fsu->incbu;
	struct button *decbu = fsu->decbu;
	int x = 0, y = 0;

	if (w == -1 && h == -1) {
		WIDGET_SCALE(fsu->tbox, -1, -1);
		WIDGET(fsu)->w = WIDGET(tbox)->w;
		WIDGET(fsu)->h = WIDGET(tbox)->h;
		x += WIDGET(fsu)->w;

		WIDGET_SCALE(incbu, -1, -1);
		WIDGET(fsu)->w += WIDGET(incbu)->w;
		y += WIDGET(fsu)->h;
		return;
	}

	WIDGET(tbox)->x = x;
	WIDGET(tbox)->y = y;
	widget_scale(tbox, w - 10, h);
	x += WIDGET(tbox)->w;

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

	if (WIDGET(fsu->tbox)->flags & WIDGET_FOCUSED)
		return;

	valueb = widget_get_binding(fsu, "value", &value);
	textbox_printf(fsu->tbox, fsu->format, *value);
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
	case WIDGET_FLOAT:
		if (*(float *)value + inc >= fsu->min &&
		    *(float *)value + inc <= fsu->max)
			*(float *)value += inc;
		break;
	case WIDGET_DOUBLE:
		if (*(double *)value + inc >= fsu->min &&
		    *(double *)value + inc <= fsu->max)
			*(double *)value += inc;
		break;
	default:
		break;
	}
	event_post(fsu, "fspinbutton-changed", NULL);
	widget_binding_modified(valueb);
	widget_binding_unlock(valueb);
}

void
fspinbutton_set_value(struct fspinbutton *fsu, double nvalue)
{
	struct widget_binding *valueb;
	double *value;

	valueb = widget_get_binding(fsu, "value", &value);
	*value = nvalue;
	event_post(fsu, "fspinbutton-changed", NULL);
	widget_binding_modified(valueb);
	widget_binding_unlock(valueb);
}

void
fspinbutton_set_min(struct fspinbutton *fsu, double nmin)
{
	pthread_mutex_lock(&fsu->lock);
	fsu->min = nmin;
	pthread_mutex_unlock(&fsu->lock);
}

void
fspinbutton_set_max(struct fspinbutton *fsu, double nmax)
{
	pthread_mutex_lock(&fsu->lock);
	fsu->max = nmax;
	pthread_mutex_unlock(&fsu->lock);
}

void
fspinbutton_set_increment(struct fspinbutton *fsu, double incr)
{
	pthread_mutex_lock(&fsu->lock);
	fsu->incr = incr;
	pthread_mutex_unlock(&fsu->lock);
}

void
fspinbutton_set_precision(struct fspinbutton *fsu, int precision)
{
	pthread_mutex_lock(&fsu->lock);
	fsu->format[0] = '%';
	fsu->format[1] = '.';
	snprintf(&fsu->format[2], sizeof(fsu->format)-2, "%d", precision);
	strlcat(fsu->format, "f", sizeof(fsu->format));
	dprintf("prec: `%s'\n", fsu->format)
	pthread_mutex_unlock(&fsu->lock);
}
