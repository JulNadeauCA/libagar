/*	$Csoft: mspinbutton.c,v 1.2 2004/03/25 09:00:33 vedge Exp $	*/

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

#include "mspinbutton.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/label.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

static struct widget_ops mspinbutton_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		mspinbutton_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	mspinbutton_draw,
	mspinbutton_scale
};

struct mspinbutton *
mspinbutton_new(void *parent, const char *sep, const char *fmt, ...)
{
	char label[LABEL_MAX];
	struct mspinbutton *sbu;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(label, sizeof(label), fmt, ap);
	va_end(ap);

	sbu = Malloc(sizeof(struct mspinbutton), M_OBJECT);
	mspinbutton_init(sbu, sep, label);
	object_attach(parent, sbu);
	return (sbu);
}

static void
mspinbutton_bound(int argc, union evarg *argv)
{
	struct mspinbutton *sbu = argv[0].p;
	struct widget_binding *binding = argv[1].p;

	if (strcmp(binding->name, "xvalue") == 0 ||
	    strcmp(binding->name, "yvalue") == 0) {
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
mspinbutton_keydown(int argc, union evarg *argv)
{
	struct mspinbutton *sbu = argv[0].p;
	int keysym = argv[1].i;

	pthread_mutex_lock(&sbu->lock);
	switch (keysym) {
	case SDLK_LEFT:
		mspinbutton_add_value(sbu, "xvalue", -sbu->inc);
		break;
	case SDLK_RIGHT:
		mspinbutton_add_value(sbu, "xvalue", sbu->inc);
		break;
	case SDLK_UP:
		mspinbutton_add_value(sbu, "yvalue", -sbu->inc);
		break;
	case SDLK_DOWN:
		mspinbutton_add_value(sbu, "yvalue", sbu->inc);
		break;
	}
	pthread_mutex_unlock(&sbu->lock);
}

static void
mspinbutton_return(int argc, union evarg *argv)
{
	char text[TEXTBOX_STRING_MAX];
	struct mspinbutton *sbu = argv[1].p;
	struct widget_binding *stringb;
	char *tp = &text[0], *s;

	stringb = widget_get_binding(sbu->input, "string", &s);
	strlcpy(text, s, sizeof(text));

	if ((s = strsep(&tp, sbu->sep)) != NULL) {
		mspinbutton_set_value(sbu, "xvalue", atoi(s));
	}
	if ((s = strsep(&tp, sbu->sep)) != NULL) {
		mspinbutton_set_value(sbu, "yvalue", atoi(s));
	}
	widget_binding_unlock(stringb);

	event_post(NULL, sbu, "mspinbutton-return", NULL);
	WIDGET(sbu->input)->flags &= ~(WIDGET_FOCUSED);
}

static void
mspinbutton_up(int argc, union evarg *argv)
{
	struct mspinbutton *sbu = argv[1].p;

	pthread_mutex_lock(&sbu->lock);
	mspinbutton_add_value(sbu, "yvalue", -sbu->inc);
	pthread_mutex_unlock(&sbu->lock);
}

static void
mspinbutton_down(int argc, union evarg *argv)
{
	struct mspinbutton *sbu = argv[1].p;
	
	pthread_mutex_lock(&sbu->lock);
	mspinbutton_add_value(sbu, "yvalue", sbu->inc);
	pthread_mutex_unlock(&sbu->lock);
}

static void
mspinbutton_left(int argc, union evarg *argv)
{
	struct mspinbutton *sbu = argv[1].p;
	
	pthread_mutex_lock(&sbu->lock);
	mspinbutton_add_value(sbu, "xvalue", -sbu->inc);
	pthread_mutex_unlock(&sbu->lock);
}

static void
mspinbutton_right(int argc, union evarg *argv)
{
	struct mspinbutton *sbu = argv[1].p;
	
	pthread_mutex_lock(&sbu->lock);
	mspinbutton_add_value(sbu, "xvalue", sbu->inc);
	pthread_mutex_unlock(&sbu->lock);
}

void
mspinbutton_init(struct mspinbutton *sbu, const char *sep, const char *label)
{
	widget_init(sbu, "mspinbutton", &mspinbutton_ops,
	    WIDGET_FOCUSABLE|WIDGET_WFILL);
	widget_bind(sbu, "xvalue", WIDGET_INT, &sbu->xvalue);
	widget_bind(sbu, "yvalue", WIDGET_INT, &sbu->yvalue);
	widget_bind(sbu, "min", WIDGET_INT, &sbu->min);
	widget_bind(sbu, "max", WIDGET_INT, &sbu->max);

	sbu->xvalue = 0;
	sbu->yvalue = 0;
	sbu->min = 0;
	sbu->max = 0;
	sbu->inc = 1;
	sbu->writeable = 0;
	sbu->sep = sep;
	pthread_mutex_init(&sbu->lock, NULL);
	
	sbu->input = textbox_new(sbu, label);
	event_new(sbu->input, "textbox-return", mspinbutton_return, "%p", sbu);

	sbu->xdecbu = button_new(sbu, "-");
	button_set_padding(sbu->xdecbu, 0);
	event_new(sbu->xdecbu, "button-pushed", mspinbutton_left, "%p", sbu);
	sbu->xincbu = button_new(sbu, "+");
	button_set_padding(sbu->xincbu, 0);
	event_new(sbu->xincbu, "button-pushed", mspinbutton_right, "%p", sbu);
	
	sbu->ydecbu = button_new(sbu, "-");
	button_set_padding(sbu->ydecbu, 0);
	event_new(sbu->ydecbu, "button-pushed", mspinbutton_up, "%p", sbu);
	sbu->yincbu = button_new(sbu, "+");
	button_set_padding(sbu->yincbu, 0);
	event_new(sbu->yincbu, "button-pushed", mspinbutton_down, "%p", sbu);

	event_new(sbu, "widget-bound", mspinbutton_bound, NULL);
	event_new(sbu, "window-keydown", mspinbutton_keydown, NULL);
}

void
mspinbutton_destroy(void *p)
{
	struct mspinbutton *sbu = p;

	pthread_mutex_destroy(&sbu->lock);
	widget_destroy(sbu);
}

void
mspinbutton_scale(void *p, int w, int h)
{
	struct mspinbutton *sbu = p;
	int x = 0, y = 0;
	const int bw = 10;
	int bh = h/2;

	if (w == -1 && h == -1) {
		WIDGET_SCALE(sbu->input, -1, -1);
		WIDGET(sbu)->w = WIDGET(sbu->input)->w;
		WIDGET(sbu)->h = WIDGET(sbu->input)->h;

		WIDGET_SCALE(sbu->yincbu, -1, -1);
		WIDGET_SCALE(sbu->ydecbu, -1, -1);
		WIDGET_SCALE(sbu->xincbu, -1, -1);
		WIDGET_SCALE(sbu->xdecbu, -1, -1);

		WIDGET(sbu)->w += WIDGET(sbu->xdecbu)->w +
		                  max(WIDGET(sbu->yincbu)->w,
				      WIDGET(sbu->ydecbu)->w) +
		                  WIDGET(sbu->xincbu)->w;
		return;
	}

	WIDGET(sbu->input)->x = x;
	WIDGET(sbu->input)->y = y;
	widget_scale(sbu->input, w - bw*3, h);
	x += WIDGET(sbu->input)->w;

	WIDGET(sbu->xdecbu)->x = x;
	WIDGET(sbu->xdecbu)->y = y + bh/2;
	widget_scale(sbu->xdecbu, bw, bh);

	WIDGET(sbu->xincbu)->x = x + bh*2;
	WIDGET(sbu->xincbu)->y = y + bh/2;
	widget_scale(sbu->xincbu, bw, bh);

	WIDGET(sbu->ydecbu)->x = x + bh;
	WIDGET(sbu->ydecbu)->y = y;
	widget_scale(sbu->ydecbu, bw, bh);
	
	WIDGET(sbu->yincbu)->x = x + bh;
	WIDGET(sbu->yincbu)->y = y + bh;
	widget_scale(sbu->yincbu, bw, bh);
}

void
mspinbutton_draw(void *p)
{
	struct mspinbutton *sbu = p;
	struct widget_binding *xvalueb, *yvalueb;
	int *xvalue, *yvalue;

	if (WIDGET(sbu->input)->flags & WIDGET_FOCUSED)
		return;

	xvalueb = widget_get_binding(sbu, "xvalue", &xvalue);
	yvalueb = widget_get_binding(sbu, "yvalue", &yvalue);
	textbox_printf(sbu->input, "%d%s%d", *xvalue, sbu->sep, *yvalue);
	widget_binding_unlock(xvalueb);
	widget_binding_unlock(yvalueb);
}

void
mspinbutton_add_value(struct mspinbutton *sbu, const char *which, int inc)
{
	struct widget_binding *valueb, *minb, *maxb;
	void *value;
	int *min, *max;

	valueb = widget_get_binding(sbu, which, &value);
	minb = widget_get_binding(sbu, "min", &min);
	maxb = widget_get_binding(sbu, "max", &max);

	switch (valueb->vtype) {
	case WIDGET_INT:
		if (*(int *)value+inc >= *min &&
		    *(int *)value+inc <= *max)
			*(int *)value += inc;
		break;
	case WIDGET_UINT:
		if (*(unsigned int *)value+inc >= *min &&
		    *(unsigned int *)value+inc <= *max)
			*(unsigned int *)value += inc;
		break;
	case WIDGET_UINT8:
		if (*(Uint8 *)value+inc >= *min &&
		    *(Uint8 *)value+inc <= *max)
			*(Uint8 *)value += inc;
		break;
	case WIDGET_SINT8:
		if (*(Sint8 *)value+inc >= *min &&
		    *(Sint8 *)value+inc <= *max)
			*(Sint8 *)value += inc;
		break;
	case WIDGET_UINT16:
		if (*(Uint16 *)value+inc >= *min &&
		    *(Uint16 *)value+inc <= *max)
			*(Uint16 *)value += inc;
		break;
	case WIDGET_SINT16:
		if (*(Sint16 *)value+inc >= *min &&
		    *(Sint16 *)value+inc <= *max)
			*(Sint16 *)value += inc;
		break;
	case WIDGET_UINT32:
		if (*(Uint32 *)value+inc >= *min &&
		    *(Uint32 *)value+inc <= *max)
			*(Uint32 *)value += inc;
		break;
	case WIDGET_SINT32:
		if (*(Sint32 *)value+inc >= *min &&
		    *(Sint32 *)value+inc <= *max)
			*(Sint32 *)value += inc;
		break;
	default:
		break;
	}

	event_post(NULL, sbu, "mspinbutton-changed", "%s", which);
	widget_binding_modified(valueb);

	widget_binding_unlock(maxb);
	widget_binding_unlock(minb);
	widget_binding_unlock(valueb);
}

void
mspinbutton_set_value(struct mspinbutton *sbu, const char *which, ...)
{
	struct widget_binding *valueb, *minb, *maxb;
	void *value;
	int *min, *max;
	va_list ap;

	valueb = widget_get_binding(sbu, which, &value);
	minb = widget_get_binding(sbu, "min", &min);
	maxb = widget_get_binding(sbu, "max", &max);

	va_start(ap, which);
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

	event_post(NULL, sbu, "mspinbutton-changed", "%s", which);
	widget_binding_modified(valueb);

	widget_binding_unlock(valueb);
	widget_binding_unlock(minb);
	widget_binding_unlock(maxb);
}

void
mspinbutton_set_min(struct mspinbutton *sbu, int nmin)
{
	struct widget_binding *minb;
	int *min;

	minb = widget_get_binding(sbu, "min", &min);
	*min = nmin;
	widget_binding_modified(minb);
	widget_binding_unlock(minb);
}

void
mspinbutton_set_max(struct mspinbutton *sbu, int nmax)
{
	struct widget_binding *maxb;
	int *max;

	maxb = widget_get_binding(sbu, "max", &max);
	*max = nmax;
	widget_binding_modified(maxb);
	widget_binding_unlock(maxb);
}

void
mspinbutton_set_range(struct mspinbutton *sbu, int nmin, int nmax)
{
	struct widget_binding *minb, *maxb;
	int *min, *max;

	minb = widget_get_binding(sbu, "min", &min);
	maxb = widget_get_binding(sbu, "max", &max);
	*min = nmin;
	*max = nmax;
	widget_binding_modified(minb);
	widget_binding_modified(maxb);
	widget_binding_unlock(minb);
	widget_binding_unlock(maxb);
}

void
mspinbutton_set_increment(struct mspinbutton *sbu, int inc)
{
	pthread_mutex_lock(&sbu->lock);
	sbu->inc = inc;
	pthread_mutex_unlock(&sbu->lock);
}

void
mspinbutton_set_writeable(struct mspinbutton *sbu, int writeable)
{
	pthread_mutex_lock(&sbu->lock);

	sbu->writeable = writeable;
	textbox_set_writeable(sbu->input, writeable);
	if (writeable) {
		button_enable(sbu->xincbu);
		button_enable(sbu->xdecbu);
		button_enable(sbu->yincbu);
		button_enable(sbu->ydecbu);
	} else {
		button_disable(sbu->xincbu);
		button_disable(sbu->xdecbu);
		button_disable(sbu->yincbu);
		button_disable(sbu->ydecbu);
	}
	pthread_mutex_unlock(&sbu->lock);
}
