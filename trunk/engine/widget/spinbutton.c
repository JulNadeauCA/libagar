/*	$Csoft: spinbutton.c,v 1.24 2005/09/03 14:19:33 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004, 2005 CubeSoft Communications, Inc.
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

static AG_WidgetOps spinbutton_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		AG_SpinbuttonDestroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	AG_SpinbuttonDraw,
	AG_SpinbuttonScale
};

AG_Spinbutton *
AG_SpinbuttonNew(void *parent, const char *fmt, ...)
{
	char label[AG_LABEL_MAX];
	AG_Spinbutton *sbu;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(label, sizeof(label), fmt, ap);
	va_end(ap);

	sbu = Malloc(sizeof(AG_Spinbutton), M_OBJECT);
	AG_SpinbuttonInit(sbu, label);
	AG_ObjectAttach(parent, sbu);
	return (sbu);
}

static void
spinbutton_bound(int argc, union evarg *argv)
{
	AG_Spinbutton *sbu = argv[0].p;
	AG_WidgetBinding *binding = argv[1].p;

	if (strcmp(binding->name, "value") == 0) {
		pthread_mutex_lock(&sbu->lock);
		switch (binding->vtype) {
		case AG_WIDGET_INT:
			sbu->min = INT_MIN+1;
			sbu->max = INT_MAX-1;
			break;
		case AG_WIDGET_UINT:
			sbu->min = 0;
			sbu->max = UINT_MAX-1;
			break;
		case AG_WIDGET_UINT8:
			sbu->min = 0;
			sbu->max = 0xffU;
			break;
		case AG_WIDGET_SINT8:
			sbu->min = -0x7f+1;
			sbu->max =  0x7f-1;
			break;
		case AG_WIDGET_UINT16:
			sbu->min = 0;
			sbu->max = 0xffffU;
			break;
		case AG_WIDGET_SINT16:
			sbu->min = -0x7fff+1;
			sbu->max =  0x7fff-1;
			break;
		case AG_WIDGET_UINT32:
			sbu->min = 0;
			sbu->max = 0xffffffffU;
			break;
		case AG_WIDGET_SINT32:
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
	AG_Spinbutton *sbu = argv[0].p;
	int keysym = argv[1].i;

	pthread_mutex_lock(&sbu->lock);
	switch (keysym) {
	case SDLK_UP:
		AG_SpinbuttonAddValue(sbu, sbu->incr);
		break;
	case SDLK_DOWN:
		AG_SpinbuttonAddValue(sbu, -sbu->incr);
		break;
	}
	pthread_mutex_unlock(&sbu->lock);
}

static void
spinbutton_changed(int argc, union evarg *argv)
{
	AG_Spinbutton *sbu = argv[1].p;
	AG_WidgetBinding *stringb;
	char *s;

	stringb = AG_WidgetGetBinding(sbu->input, "string", &s);
	AG_SpinbuttonSetValue(sbu, atoi(s));
	AG_WidgetUnlockBinding(stringb);

	AG_PostEvent(NULL, sbu, "spinbutton-changed", NULL);
}

static void
spinbutton_return(int argc, union evarg *argv)
{
	AG_Spinbutton *sbu = argv[1].p;
	AG_WidgetBinding *stringb;

	AG_PostEvent(NULL, sbu, "spinbutton-return", NULL);
	AGWIDGET(sbu->input)->flags &= ~(AG_WIDGET_FOCUSED);
}

static void
spinbutton_inc(int argc, union evarg *argv)
{
	AG_Spinbutton *sbu = argv[1].p;

	pthread_mutex_lock(&sbu->lock);
	AG_SpinbuttonAddValue(sbu, sbu->incr);
	pthread_mutex_unlock(&sbu->lock);
	
	AG_PostEvent(NULL, sbu, "spinbutton-changed", NULL);
}

static void
spinbutton_dec(int argc, union evarg *argv)
{
	AG_Spinbutton *sbu = argv[1].p;
	
	pthread_mutex_lock(&sbu->lock);
	AG_SpinbuttonAddValue(sbu, -sbu->incr);
	pthread_mutex_unlock(&sbu->lock);
	
	AG_PostEvent(NULL, sbu, "spinbutton-changed", NULL);
}

void
AG_SpinbuttonInit(AG_Spinbutton *sbu, const char *label)
{
	AG_WidgetInit(sbu, "spinbutton", &spinbutton_ops,
	    AG_WIDGET_FOCUSABLE|AG_WIDGET_WFILL);
	AG_WidgetBind(sbu, "value", AG_WIDGET_INT, &sbu->value);
	AG_WidgetBind(sbu, "min", AG_WIDGET_INT, &sbu->min);
	AG_WidgetBind(sbu, "max", AG_WIDGET_INT, &sbu->max);

	sbu->value = 0;
	sbu->incr = 1;
	sbu->writeable = 0;
	sbu->min = 0;
	sbu->max = 0;
	pthread_mutex_init(&sbu->lock, NULL);
	sbu->input = AG_TextboxNew(sbu, label);

	AG_SetEvent(sbu, "widget-bound", spinbutton_bound, NULL);
	AG_SetEvent(sbu, "window-keydown", spinbutton_keydown, NULL);

	sbu->incbu = AG_ButtonNew(sbu, "+");
	AG_ButtonSetPadding(sbu->incbu, 0);
	AG_ButtonSetRepeatMode(sbu->incbu, 1);

	sbu->decbu = AG_ButtonNew(sbu, "-");
	AG_ButtonSetPadding(sbu->decbu, 0);
	AG_ButtonSetRepeatMode(sbu->decbu, 1);
	
	AG_SetEvent(sbu->input, "textbox-return", spinbutton_return, "%p", sbu);
	AG_SetEvent(sbu->input, "textbox-postchg", spinbutton_changed, "%p",
	    sbu);
	AG_SetEvent(sbu->incbu, "button-pushed", spinbutton_inc, "%p", sbu);
	AG_SetEvent(sbu->decbu, "button-pushed", spinbutton_dec, "%p", sbu);
}

void
AG_SpinbuttonDestroy(void *p)
{
	AG_Spinbutton *sbu = p;

	pthread_mutex_destroy(&sbu->lock);
	AG_WidgetDestroy(sbu);
}

void
AG_SpinbuttonScale(void *p, int w, int h)
{
	AG_Spinbutton *sbu = p;
	int x = 0, y = 0;
	int bw = 10;
	int bh = h/2;

	if (w == -1 && h == -1) {
		AGWIDGET_SCALE(sbu->input, -1, -1);
		AGWIDGET(sbu)->w = AGWIDGET(sbu->input)->w;
		AGWIDGET(sbu)->h = AGWIDGET(sbu->input)->h;
		AGWIDGET_SCALE(sbu->incbu, -1, -1);
		AGWIDGET_SCALE(sbu->decbu, -1, -1);
		AGWIDGET(sbu)->w += max(AGWIDGET(sbu->incbu)->w,
		                      AGWIDGET(sbu->decbu)->w);
		return;
	}

	AGWIDGET(sbu->input)->x = x;
	AGWIDGET(sbu->input)->y = y;
	AG_WidgetScale(sbu->input, w - bw, h);
	x += AGWIDGET(sbu->input)->w;

	AGWIDGET(sbu->incbu)->x = x;
	AGWIDGET(sbu->incbu)->y = y;
	AG_WidgetScale(sbu->incbu, bw, bh);
	y += h/2;

	AGWIDGET(sbu->decbu)->x = x;
	AGWIDGET(sbu->decbu)->y = y;
	AG_WidgetScale(sbu->decbu, bw, bh);
}

void
AG_SpinbuttonDraw(void *p)
{
	AG_Spinbutton *sbu = p;
	AG_WidgetBinding *valueb;
	void *value;

	if (AGWIDGET(sbu->input)->flags & AG_WIDGET_FOCUSED)
		return;

	valueb = AG_WidgetGetBinding(sbu, "value", &value);
	switch (valueb->type) {
	case AG_WIDGET_INT:
		AG_TextboxPrintf(sbu->input, "%d", *(int *)value);
		break;
	case AG_WIDGET_UINT:
		AG_TextboxPrintf(sbu->input, "%u", *(u_int *)value);
		break;
	case AG_WIDGET_UINT8:
		AG_TextboxPrintf(sbu->input, "%u", *(Uint8 *)value);
		break;
	case AG_WIDGET_SINT8:
		AG_TextboxPrintf(sbu->input, "%d", *(Sint8 *)value);
		break;
	case AG_WIDGET_UINT16:
		AG_TextboxPrintf(sbu->input, "%u", *(Uint16 *)value);
		break;
	case AG_WIDGET_SINT16:
		AG_TextboxPrintf(sbu->input, "%d", *(Sint16 *)value);
		break;
	case AG_WIDGET_UINT32:
		AG_TextboxPrintf(sbu->input, "%u", *(Uint32 *)value);
		break;
	case AG_WIDGET_SINT32:
		AG_TextboxPrintf(sbu->input, "%d", *(Sint32 *)value);
		break;
	default:
		break;
	}
	AG_WidgetUnlockBinding(valueb);
}

void
AG_SpinbuttonAddValue(AG_Spinbutton *sbu, int inc)
{
	AG_WidgetBinding *valueb, *minb, *maxb;
	void *value;
	int *min, *max;

	valueb = AG_WidgetGetBinding(sbu, "value", &value);
	minb = AG_WidgetGetBinding(sbu, "min", &min);
	maxb = AG_WidgetGetBinding(sbu, "max", &max);

	switch (valueb->vtype) {
	case AG_WIDGET_INT:
		*(int *)value = *(int *)value+inc < *min ? *min :
		                *(int *)value+inc > *max ? *max :
				*(int *)value+inc;
		break;
	case AG_WIDGET_UINT:
		*(unsigned *)value = *(unsigned *)value+inc < *min ? *min :
		                     *(unsigned *)value+inc > *max ? *max :
			  	     *(unsigned *)value+inc;
		break;
	case AG_WIDGET_UINT8:
		*(Uint8 *)value = *(Uint8 *)value+inc < *min ? *min :
		                  *(Uint8 *)value+inc > *max ? *max :
			  	  *(Uint8 *)value+inc;
		break;
	case AG_WIDGET_SINT8:
		*(Sint8 *)value = *(Sint8 *)value+inc < *min ? *min :
		                  *(Sint8 *)value+inc > *max ? *max :
			  	  *(Sint8 *)value+inc;
		break;
	case AG_WIDGET_UINT16:
		*(Uint16 *)value = *(Uint16 *)value+inc < *min ? *min :
		                   *(Uint16 *)value+inc > *max ? *max :
			  	   *(Uint16 *)value+inc;
		break;
	case AG_WIDGET_SINT16:
		*(Sint16 *)value = *(Sint16 *)value+inc < *min ? *min :
		                   *(Sint16 *)value+inc > *max ? *max :
			  	   *(Sint16 *)value+inc;
		break;
	case AG_WIDGET_UINT32:
		*(Uint32 *)value = *(Uint32 *)value+inc < *min ? *min :
		                   *(Uint32 *)value+inc > *max ? *max :
			  	   *(Uint32 *)value+inc;
		break;
	case AG_WIDGET_SINT32:
		*(Sint32 *)value = *(Sint32 *)value+inc < *min ? *min :
		                   *(Sint32 *)value+inc > *max ? *max :
			  	   *(Sint32 *)value+inc;
		break;
	default:
		break;
	}

	AG_PostEvent(NULL, sbu, "spinbutton-changed", NULL);
	AG_WidgetBindingChanged(valueb);

	AG_WidgetUnlockBinding(maxb);
	AG_WidgetUnlockBinding(minb);
	AG_WidgetUnlockBinding(valueb);
}

void
AG_SpinbuttonSetValue(AG_Spinbutton *sbu, ...)
{
	AG_WidgetBinding *valueb, *minb, *maxb;
	void *value;
	int *min, *max;
	va_list ap;

	valueb = AG_WidgetGetBinding(sbu, "value", &value);
	minb = AG_WidgetGetBinding(sbu, "min", &min);
	maxb = AG_WidgetGetBinding(sbu, "max", &max);

	va_start(ap, sbu);
	switch (valueb->vtype) {
	case AG_WIDGET_INT:
		{
			int i = va_arg(ap, int);

			*(int *)value = i < *min ? *min :
			                i > *max ? *max :
					i;
		}
		break;
	case AG_WIDGET_UINT:
		{
			unsigned i = va_arg(ap, unsigned int);

			*(unsigned *)value = i<(unsigned)*min ? (unsigned)*min :
			                     i>(unsigned)*max ? (unsigned)*max :
					     i;
		}
		break;
	case AG_WIDGET_UINT8:
		{
			Uint8 i = (Uint8)va_arg(ap, int);

			*(Uint8 *)value = i < (Uint8)*min ? (Uint8)*min :
			                  i > (Uint8)*max ? (Uint8)*max :
					  i;
		}
		break;
	case AG_WIDGET_SINT8:
		{
			Sint8 i = (Sint8)va_arg(ap, int);

			*(Sint8 *)value = i < (Sint8)*min ? (Sint8)*min :
			                  i > (Sint8)*max ? (Sint8)*max :
					  i;
		}
		break;
	case AG_WIDGET_UINT16:
		{
			Uint16 i = (Uint16)va_arg(ap, int);

			*(Uint16 *)value = i < (Uint16)*min ? (Uint16)*min :
			                   i > (Uint16)*max ? (Uint16)*max :
					   i;
		}
		break;
	case AG_WIDGET_SINT16:
		{
			Sint16 i = (Sint16)va_arg(ap, int);

			*(Sint16 *)value = i < (Sint16)*min ? (Sint16)*min :
			                   i > (Sint16)*max ? (Sint16)*max :
					   i;
		}
		break;
	case AG_WIDGET_UINT32:
		{
			Uint32 i = (Uint32)va_arg(ap, int);

			*(Uint32 *)value = i < (Uint32)*min ? (Uint32)*min :
			                   i > (Uint32)*max ? (Uint32)*max :
					   i;
		}
		break;
	case AG_WIDGET_SINT32:
		{
			Sint32 i = (Sint32)va_arg(ap, int);

			*(Sint32 *)value = i < (Sint32)*min ? (Sint32)*min :
			                   i > (Sint32)*max ? (Sint32)*max :
					   i;
		}
		break;
	default:
		break;
	}
	va_end(ap);

	AG_PostEvent(NULL, sbu, "spinbutton-changed", NULL);
	AG_WidgetBindingChanged(valueb);

	AG_WidgetUnlockBinding(valueb);
	AG_WidgetUnlockBinding(minb);
	AG_WidgetUnlockBinding(maxb);
}

void
AG_SpinbuttonSetMin(AG_Spinbutton *sbu, int nmin)
{
	AG_WidgetBinding *minb;
	int *min;

	minb = AG_WidgetGetBinding(sbu, "min", &min);
	*min = nmin;
	AG_WidgetBindingChanged(minb);
	AG_WidgetUnlockBinding(minb);
}

void
AG_SpinbuttonSetMax(AG_Spinbutton *sbu, int nmax)
{
	AG_WidgetBinding *maxb;
	int *max;

	maxb = AG_WidgetGetBinding(sbu, "max", &max);
	*max = nmax;
	AG_WidgetBindingChanged(maxb);
	AG_WidgetUnlockBinding(maxb);
}

void
AG_SpinbuttonSetRange(AG_Spinbutton *sbu, int nmin, int nmax)
{
	AG_WidgetBinding *minb, *maxb;
	int *min, *max;

	minb = AG_WidgetGetBinding(sbu, "min", &min);
	maxb = AG_WidgetGetBinding(sbu, "max", &max);
	*min = nmin;
	*max = nmax;
	AG_WidgetBindingChanged(minb);
	AG_WidgetBindingChanged(maxb);
	AG_WidgetUnlockBinding(minb);
	AG_WidgetUnlockBinding(maxb);
}

void
AG_SpinbuttonSetIncrement(AG_Spinbutton *sbu, int incr)
{
	pthread_mutex_lock(&sbu->lock);
	sbu->incr = incr;
	pthread_mutex_unlock(&sbu->lock);
}

void
AG_SpinbuttonSetWriteable(AG_Spinbutton *sbu, int writeable)
{
	pthread_mutex_lock(&sbu->lock);

	sbu->writeable = writeable;
	AG_TextboxSetWriteable(sbu->input, writeable);
	if (writeable) {
		AG_ButtonEnable(sbu->incbu);
		AG_ButtonEnable(sbu->decbu);
	} else {
		AG_ButtonDisable(sbu->incbu);
		AG_ButtonDisable(sbu->decbu);
	}
	pthread_mutex_unlock(&sbu->lock);
}
