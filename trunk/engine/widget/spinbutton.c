/*	$Csoft: spinbutton.c,v 1.43 2003/06/08 00:21:04 vedge Exp $	*/

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

#include "spinbutton.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>

static struct widget_ops spinbutton_ops = {
	{
		NULL,			/* init */
		spinbutton_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	spinbutton_draw,
	spinbutton_scale
};

struct spinbutton *
spinbutton_new(void *parent, const char *fmt, ...)
{
	char label[SPINBUTTON_LABEL_MAX];
	struct spinbutton *sbu;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(label, sizeof(label), fmt, ap);
	va_end(ap);

	sbu = Malloc(sizeof(struct spinbutton));
	spinbutton_init(sbu, label);
	object_attach(parent, sbu);
	return (sbu);
}

static void
spinbutton_bound(int argc, union evarg *argv)
{
	struct spinbutton *sbu = argv[0].p;
	struct widget_binding *binding = argv[1].p;

	if (strcmp(binding->name, "value") == 0) {
		textbox_printf(sbu->tbox, "%d", *(int *)binding->p1);
	}
}

static void
spinbutton_keydown(int argc, union evarg *argv)
{
	struct spinbutton *sbu = argv[0].p;
	int keysym = argv[1].i;

	pthread_mutex_lock(&sbu->lock);
	switch (keysym) {
	case SDLK_UP:
		spinbutton_increment(sbu, sbu->incr);
		break;
	case SDLK_DOWN:
		spinbutton_decrement(sbu, sbu->incr);
		break;
	}
	pthread_mutex_unlock(&sbu->lock);
}

static void
spinbutton_return(int argc, union evarg *argv)
{
	struct spinbutton *sbu = argv[1].p;
	struct widget_binding *stringb;
	char *s;

	stringb = widget_binding_get_locked(sbu->tbox, "string", &s);
	spinbutton_set_value(sbu, atoi(s));
	widget_binding_unlock(stringb);

	WIDGET(sbu->tbox)->flags &= ~(WIDGET_FOCUSED);
}

static void
spinbutton_up(int argc, union evarg *argv)
{
	struct spinbutton *sbu = argv[1].p;

	pthread_mutex_lock(&sbu->lock);
	spinbutton_increment(sbu, sbu->incr);
	pthread_mutex_unlock(&sbu->lock);
}

static void
spinbutton_down(int argc, union evarg *argv)
{
	struct spinbutton *sbu = argv[1].p;
	
	pthread_mutex_lock(&sbu->lock);
	spinbutton_decrement(sbu, sbu->incr);
	pthread_mutex_unlock(&sbu->lock);
}

void
spinbutton_init(struct spinbutton *sbu, const char *label)
{
	widget_init(sbu, "spinbutton", &spinbutton_ops,
	    WIDGET_FOCUSABLE|WIDGET_CLIPPING|WIDGET_WFILL);
	
	sbu->value = 0;
	sbu->min = -1;
	sbu->max = -1;
	sbu->tbox = textbox_new(sbu, label);
	
	pthread_mutex_init(&sbu->lock, NULL);
	sbu->incr = 1;

	event_new(sbu, "widget-bound", spinbutton_bound, NULL);
	event_new(sbu, "window-keydown", spinbutton_keydown, NULL);

	widget_bind(sbu, "value", WIDGET_INT, NULL, &sbu->value);
	widget_bind(sbu, "min", WIDGET_INT, NULL, &sbu->min);
	widget_bind(sbu, "max", WIDGET_INT, NULL, &sbu->max);

	sbu->incbu = button_new(sbu, "+");
	sbu->decbu = button_new(sbu, "-");
	button_set_padding(sbu->incbu, 0);
	button_set_padding(sbu->decbu, 0);
	
	event_new(sbu->tbox, "textbox-return", spinbutton_return, "%p", sbu);
	event_new(sbu->incbu, "button-pushed", spinbutton_up, "%p", sbu);
	event_new(sbu->decbu, "button-pushed", spinbutton_down, "%p", sbu);
}

void
spinbutton_destroy(void *p)
{
	struct spinbutton *sbu = p;

	pthread_mutex_destroy(&sbu->lock);

	widget_destroy(sbu);
}

void
spinbutton_scale(void *p, int w, int h)
{
	struct spinbutton *sbu = p;
	struct textbox *tbox = sbu->tbox;
	struct button *incbu = sbu->incbu;
	struct button *decbu = sbu->decbu;
	int x = 0, y = 0;

	if (w == -1 && h == -1) {
		WIDGET_SCALE(sbu->tbox, -1, -1);
		WIDGET(sbu)->w = WIDGET(tbox)->w;
		WIDGET(sbu)->h = WIDGET(tbox)->h;
		x += WIDGET(sbu)->w;

		WIDGET_SCALE(incbu, -1, -1);
		WIDGET(sbu)->w += WIDGET(incbu)->w;
		y += WIDGET(sbu)->h;
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
spinbutton_draw(void *p)
{
	struct spinbutton *sbu = p;
	struct widget_binding *valueb;
	int *value;

	if (WIDGET(sbu->tbox)->flags & WIDGET_FOCUSED)
		return;

	valueb = widget_binding_get_locked(sbu, "value", &value);
	textbox_printf(sbu->tbox, "%d", *value);
	widget_binding_unlock(valueb);
}

/* Increment the bound value by inc. */
void
spinbutton_increment(struct spinbutton *sbu, int inc)
{
	struct widget_binding *valueb;
	int *value;

	valueb = widget_binding_get_locked(sbu, "value", &value);
	*value += inc;
	event_post(sbu, "spinbutton-changed", "%i", *value);
	widget_binding_modified(valueb);
	widget_binding_unlock(valueb);
}

/* Decrement the bound value by dec. */
void
spinbutton_decrement(struct spinbutton *sbu, int dec)
{
	struct widget_binding *valueb;
	int *value;

	valueb = widget_binding_get_locked(sbu, "value", &value);
	(*value) -= dec;
	event_post(sbu, "spinbutton-changed", "%i", *value);
	widget_binding_modified(valueb);
	widget_binding_unlock(valueb);
}

/* Alter the spinbutton's current value. */
void
spinbutton_set_value(struct spinbutton *sbu, int nvalue)
{
	struct widget_binding *valueb;
	int *value;

	valueb = widget_binding_get_locked(sbu, "value", &value);
	*value = nvalue;
	event_post(sbu, "spinbutton-changed", "%i", *value);
	widget_binding_modified(valueb);
	widget_binding_unlock(valueb);
}

/* Alter the spinbutton's minimum value. */
void
spinbutton_set_min(struct spinbutton *sbu, int nmin)
{
	struct widget_binding *minb;
	int *min;

	minb = widget_binding_get_locked(sbu, "min", &min);
	*min = nmin;
	event_post(sbu, "spinbutton-changed", "%i", *min);
	widget_binding_modified(minb);
	widget_binding_unlock(minb);
}

/* Alter the spinbutton's maximum value. */
void
spinbutton_set_max(struct spinbutton *sbu, int nmax)
{
	struct widget_binding *maxb;
	int *max;

	maxb = widget_binding_get_locked(sbu, "max", &max);
	*max = nmax;
	event_post(sbu, "spinbutton-changed", "%i", *max);
	widget_binding_modified(maxb);
	widget_binding_unlock(maxb);
}

/* Set the increment. */
void
spinbutton_set_increment(struct spinbutton *sbu, int incr)
{
	pthread_mutex_lock(&sbu->lock);
	sbu->incr = incr;
	pthread_mutex_unlock(&sbu->lock);
}

