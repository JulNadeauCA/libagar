/*	$Csoft: spinbutton.c,v 1.13 2004/03/25 04:23:18 vedge Exp $	*/

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

#include "spinbutton.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/label.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

static struct widget_ops spinbutton_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
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
	char label[LABEL_MAX];
	struct spinbutton *sbu;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(label, sizeof(label), fmt, ap);
	va_end(ap);

	sbu = Malloc(sizeof(struct spinbutton), M_OBJECT);
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
		pthread_mutex_lock(&sbu->lock);
		switch (binding->vtype) {
		case WIDGET_INT:
			sbu->min = INT_MIN+1;
			sbu->max = INT_MAX-1;
			break;
		case WIDGET_UINT:
			sbu->min = 0;
			sbu->max = UINT_MAX-1;
			break;
		case WIDGET_UINT8:
			sbu->min = 0;
			sbu->max = 0xffU;
			break;
		case WIDGET_SINT8:
			sbu->min = -0x7f+1;
			sbu->max =  0x7f-1;
			break;
		case WIDGET_UINT16:
			sbu->min = 0;
			sbu->max = 0xffffU;
			break;
		case WIDGET_SINT16:
			sbu->min = -0x7fff+1;
			sbu->max =  0x7fff-1;
			break;
		case WIDGET_UINT32:
			sbu->min = 0;
			sbu->max = 0xffffffffU;
			break;
		case WIDGET_SINT32:
			sbu->min = -0x7fffffff+1;
			sbu->max =  0x7fffffff-1;
			break;
		}
		pthread_mutex_unlock(&sbu->lock);
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
		spinbutton_add_value(sbu, sbu->incr);
		break;
	case SDLK_DOWN:
		spinbutton_add_value(sbu, -sbu->incr);
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

	stringb = widget_get_binding(sbu->input, "string", &s);
	spinbutton_set_value(sbu, atoi(s));
	widget_binding_unlock(stringb);

	WIDGET(sbu->input)->flags &= ~(WIDGET_FOCUSED);

	event_post(NULL, sbu, "spinbutton-return", NULL);
	event_post(NULL, sbu, "spinbutton-changed", NULL);
}

static void
spinbutton_up(int argc, union evarg *argv)
{
	struct spinbutton *sbu = argv[1].p;

	pthread_mutex_lock(&sbu->lock);
	spinbutton_add_value(sbu, sbu->incr);
	pthread_mutex_unlock(&sbu->lock);
	
	event_post(NULL, sbu, "spinbutton-changed", NULL);
}

static void
spinbutton_down(int argc, union evarg *argv)
{
	struct spinbutton *sbu = argv[1].p;
	
	pthread_mutex_lock(&sbu->lock);
	spinbutton_add_value(sbu, -sbu->incr);
	pthread_mutex_unlock(&sbu->lock);
	
	event_post(NULL, sbu, "spinbutton-changed", NULL);
}

void
spinbutton_init(struct spinbutton *sbu, const char *label)
{
	widget_init(sbu, "spinbutton", &spinbutton_ops,
	    WIDGET_FOCUSABLE|WIDGET_WFILL);
	
	sbu->value = 0;
	sbu->incr = 1;
	sbu->writeable = 0;
	sbu->min = 0;
	sbu->max = 0;
	pthread_mutex_init(&sbu->lock, NULL);
	sbu->input = textbox_new(sbu, label);

	event_new(sbu, "widget-bound", spinbutton_bound, NULL);
	event_new(sbu, "window-keydown", spinbutton_keydown, NULL);

	widget_bind(sbu, "value", WIDGET_INT, &sbu->value);
	widget_bind(sbu, "min", WIDGET_INT, &sbu->min);
	widget_bind(sbu, "max", WIDGET_INT, &sbu->max);

	sbu->incbu = button_new(sbu, "+");
	sbu->decbu = button_new(sbu, "-");
	button_set_padding(sbu->incbu, 0);
	button_set_padding(sbu->decbu, 0);
	
	event_new(sbu->input, "textbox-return", spinbutton_return, "%p", sbu);
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
	int x = 0, y = 0;
	int bw = 10;
	int bh = h/2;

	if (w == -1 && h == -1) {
		WIDGET_SCALE(sbu->input, -1, -1);
		WIDGET(sbu)->w = WIDGET(sbu->input)->w;
		WIDGET(sbu)->h = WIDGET(sbu->input)->h;
		WIDGET_SCALE(sbu->incbu, -1, -1);
		WIDGET_SCALE(sbu->decbu, -1, -1);
		WIDGET(sbu)->w += max(WIDGET(sbu->incbu)->w,
		                      WIDGET(sbu->decbu)->w);
		return;
	}

	WIDGET(sbu->input)->x = x;
	WIDGET(sbu->input)->y = y;
	widget_scale(sbu->input, w - bw, h);
	x += WIDGET(sbu->input)->w;

	WIDGET(sbu->incbu)->x = x;
	WIDGET(sbu->incbu)->y = y;
	widget_scale(sbu->incbu, bw, bh);
	y += h/2;

	WIDGET(sbu->decbu)->x = x;
	WIDGET(sbu->decbu)->y = y;
	widget_scale(sbu->decbu, bw, bh);
}

void
spinbutton_draw(void *p)
{
	struct spinbutton *sbu = p;
	struct widget_binding *valueb;
	int *value;

	if (WIDGET(sbu->input)->flags & WIDGET_FOCUSED)
		return;

	valueb = widget_get_binding(sbu, "value", &value);
	textbox_printf(sbu->input, "%d", *value);
	widget_binding_unlock(valueb);
}

void
spinbutton_add_value(struct spinbutton *sbu, ...)
{
	struct widget_binding *valueb, *minb, *maxb;
	void *value;
	int *min, *max;
	va_list ap;

	valueb = widget_get_binding(sbu, "value", &value);
	minb = widget_get_binding(sbu, "min", &min);
	maxb = widget_get_binding(sbu, "max", &max);

	va_start(ap, sbu);
	switch (valueb->vtype) {
	case WIDGET_INT:
		{
			int inc = va_arg(ap, int);

			if (*(int *)value+inc >= *min &&
			    *(int *)value+inc <= *max)
				*(int *)value += inc;
		}
		break;
	case WIDGET_UINT:
		{
			unsigned int inc = va_arg(ap, unsigned int);

			if (*(unsigned int *)value+inc >= *min &&
			    *(unsigned int *)value+inc <= *max)
				*(unsigned int *)value += inc;
		}
		break;
	case WIDGET_UINT8:
		{
			Uint8 inc = va_arg(ap, Uint8);

			if (*(Uint8 *)value+inc >= *min &&
			    *(Uint8 *)value+inc <= *max)
				*(Uint8 *)value += inc;
		}
		break;
	case WIDGET_SINT8:
		{
			Sint8 inc = va_arg(ap, Sint8);

			if (*(Sint8 *)value+inc >= *min &&
			    *(Sint8 *)value+inc <= *max)
				*(Sint8 *)value += inc;
		}
		break;
	case WIDGET_UINT16:
		{
			Uint16 inc = va_arg(ap, Uint16);

			if (*(Uint16 *)value+inc >= *min &&
			    *(Uint16 *)value+inc <= *max)
				*(Uint16 *)value += inc;
		}
		break;
	case WIDGET_SINT16:
		{
			Sint16 inc = va_arg(ap, Sint16);

			if (*(Sint16 *)value+inc >= *min &&
			    *(Sint16 *)value+inc <= *max)
				*(Sint16 *)value += inc;
		}
		break;
	case WIDGET_UINT32:
		{
			Uint32 inc = va_arg(ap, Uint32);

			if (*(Uint32 *)value+inc >= *min &&
			    *(Uint32 *)value+inc <= *max)
				*(Uint32 *)value += inc;
		}
		break;
	case WIDGET_SINT32:
		{
			Sint32 inc = va_arg(ap, Sint32);

			if (*(Sint32 *)value+inc >= *min &&
			    *(Sint32 *)value+inc <= *max)
				*(Sint32 *)value += inc;
		}
		break;
	default:
		break;
	}
	va_end(ap);

	event_post(NULL, sbu, "spinbutton-changed", NULL);
	widget_binding_modified(valueb);

	widget_binding_unlock(maxb);
	widget_binding_unlock(minb);
	widget_binding_unlock(valueb);
}

void
spinbutton_set_value(struct spinbutton *sbu, ...)
{
	struct widget_binding *valueb, *minb, *maxb;
	void *value;
	int *min, *max;
	va_list ap;

	valueb = widget_get_binding(sbu, "value", &value);
	minb = widget_get_binding(sbu, "min", &min);
	maxb = widget_get_binding(sbu, "max", &max);

	va_start(ap, sbu);
	switch (valueb->vtype) {
	case WIDGET_INT:
		{
			int i = va_arg(ap, int);

			if (i < *min) {
				*(int *)value = *min;
			} else if (i > *max) {
				*(int *)value = *max;
			} else {
				*(int *)value = i;
			}
		}
		break;
	case WIDGET_UINT:
		{
			unsigned int i = va_arg(ap, unsigned int);

			if (i < (unsigned int)*min) {
				*(unsigned int *)value = *min;
			} else if (i > (unsigned int)*max) {
				*(unsigned int *)value = *max;
			} else {
				*(unsigned int *)value = i;
			}
		}
		break;
	case WIDGET_UINT8:
		{
			Uint8 i = va_arg(ap, Uint8);

			if (i < (Uint8)*min) {
				*(Uint8 *)value = *min;
			} else if (i > (Uint8)*max) {
				*(Uint8 *)value = *max;
			} else {
				*(Uint8 *)value = i;
			}
		}
		break;
	case WIDGET_SINT8:
		{
			Sint8 i = va_arg(ap, Sint8);

			if (i < (Sint8)*min) {
				*(Sint8 *)value = *min;
			} else if (i > (Sint8)*max) {
				*(Sint8 *)value = *max;
			} else {
				*(Sint8 *)value = i;
			}
		}
		break;
	case WIDGET_UINT16:
		{
			Uint16 i = va_arg(ap, Uint16);

			if (i < (Uint16)*min) {
				*(Uint16 *)value = *min;
			} else if (i > (Uint16)*max) {
				*(Uint16 *)value = *max;
			} else {
				*(Uint16 *)value = i;
			}
		}
		break;
	case WIDGET_SINT16:
		{
			Sint16 i = va_arg(ap, Sint16);

			if (i < (Sint16)*min) {
				*(Sint16 *)value = *min;
			} else if (i > (Sint16)*max) {
				*(Sint16 *)value = *max;
			} else {
				*(Sint16 *)value = i;
			}
		}
		break;
	case WIDGET_UINT32:
		{
			Uint32 i = va_arg(ap, Uint32);

			if (i < (Uint32)*min) {
				*(Uint32 *)value = *min;
			} else if (i > (Uint32)*max) {
				*(Uint32 *)value = *max;
			} else {
				*(Uint32 *)value = i;
			}
		}
		break;
	case WIDGET_SINT32:
		{
			Sint32 i = va_arg(ap, Sint32);

			if (i < (Sint32)*min) {
				*(Sint32 *)value = *min;
			} else if (i > (Sint32)*max) {
				*(Sint32 *)value = *max;
			} else {
				*(Sint32 *)value = i;
			}
		}
		break;
	default:
		break;
	}
	va_end(ap);

	event_post(NULL, sbu, "spinbutton-changed", NULL);
	widget_binding_modified(valueb);

	widget_binding_unlock(valueb);
	widget_binding_unlock(minb);
	widget_binding_unlock(maxb);
}

void
spinbutton_set_min(struct spinbutton *sbu, int nmin)
{
	struct widget_binding *minb;
	int *min;

	minb = widget_get_binding(sbu, "min", &min);
	*min = nmin;
	widget_binding_modified(minb);
	widget_binding_unlock(minb);
}

void
spinbutton_set_max(struct spinbutton *sbu, int nmax)
{
	struct widget_binding *maxb;
	int *max;

	maxb = widget_get_binding(sbu, "max", &max);
	*max = nmax;
	widget_binding_modified(maxb);
	widget_binding_unlock(maxb);
}

void
spinbutton_set_range(struct spinbutton *sbu, int nmin, int nmax)
{
	struct widget_binding *minb, *maxb;
	int *min, *max;

	minb = widget_get_binding(sbu, "min", &min);
	maxb = widget_get_binding(sbu, "max", &max);
	*min = nmin;
	*max = nmin;
	widget_binding_modified(minb);
	widget_binding_modified(maxb);
	widget_binding_unlock(minb);
	widget_binding_unlock(maxb);
}

void
spinbutton_set_increment(struct spinbutton *sbu, int incr)
{
	pthread_mutex_lock(&sbu->lock);
	sbu->incr = incr;
	pthread_mutex_unlock(&sbu->lock);
}

void
spinbutton_set_writeable(struct spinbutton *sbu, int writeable)
{
	pthread_mutex_lock(&sbu->lock);

	sbu->writeable = writeable;
	textbox_set_writeable(sbu->input, writeable);
	if (writeable) {
		button_enable(sbu->incbu);
		button_enable(sbu->decbu);
	} else {
		button_disable(sbu->incbu);
		button_disable(sbu->decbu);
	}
	pthread_mutex_unlock(&sbu->lock);
}
