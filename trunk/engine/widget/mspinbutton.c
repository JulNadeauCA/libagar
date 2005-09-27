/*	$Csoft: mspinbutton.c,v 1.9 2005/07/08 04:36:47 vedge Exp $	*/

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

#include "mspinbutton.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/label.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

static AG_WidgetOps mspinbutton_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		AG_MSpinbuttonDestroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	AG_MSpinbuttonDraw,
	AG_MSpinbuttonScale
};

AG_MSpinbutton *
AG_MSpinbuttonNew(void *parent, const char *sep, const char *fmt, ...)
{
	char label[AG_LABEL_MAX];
	AG_MSpinbutton *sbu;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(label, sizeof(label), fmt, ap);
	va_end(ap);

	sbu = Malloc(sizeof(AG_MSpinbutton), M_OBJECT);
	AG_MSpinbuttonInit(sbu, sep, label);
	AG_ObjectAttach(parent, sbu);
	return (sbu);
}

static void
mspinbutton_bound(int argc, union evarg *argv)
{
	AG_MSpinbutton *sbu = argv[0].p;
	AG_WidgetBinding *binding = argv[1].p;

	if (strcmp(binding->name, "xvalue") == 0 ||
	    strcmp(binding->name, "yvalue") == 0) {
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
mspinbutton_keydown(int argc, union evarg *argv)
{
	AG_MSpinbutton *sbu = argv[0].p;
	int keysym = argv[1].i;

	pthread_mutex_lock(&sbu->lock);
	switch (keysym) {
	case SDLK_LEFT:
		AG_MSpinbuttonAddValue(sbu, "xvalue", -sbu->inc);
		break;
	case SDLK_RIGHT:
		AG_MSpinbuttonAddValue(sbu, "xvalue", sbu->inc);
		break;
	case SDLK_UP:
		AG_MSpinbuttonAddValue(sbu, "yvalue", -sbu->inc);
		break;
	case SDLK_DOWN:
		AG_MSpinbuttonAddValue(sbu, "yvalue", sbu->inc);
		break;
	}
	pthread_mutex_unlock(&sbu->lock);
}

static void
mspinbutton_return(int argc, union evarg *argv)
{
	char text[AG_TEXTBOX_STRING_MAX];
	AG_MSpinbutton *sbu = argv[1].p;
	AG_WidgetBinding *stringb;
	char *tp = &text[0], *s;

	stringb = AG_WidgetGetBinding(sbu->input, "string", &s);
	strlcpy(text, s, sizeof(text));

	if ((s = strsep(&tp, sbu->sep)) != NULL) {
		AG_MSpinbuttonSetValue(sbu, "xvalue", atoi(s));
	}
	if ((s = strsep(&tp, sbu->sep)) != NULL) {
		AG_MSpinbuttonSetValue(sbu, "yvalue", atoi(s));
	}
	AG_WidgetUnlockBinding(stringb);

	AG_PostEvent(NULL, sbu, "mspinbutton-return", NULL);
	AGWIDGET(sbu->input)->flags &= ~(AG_WIDGET_FOCUSED);
}

static void
mspinbutton_textchg(int argc, union evarg *argv)
{
	char text[AG_TEXTBOX_STRING_MAX];
	AG_MSpinbutton *sbu = argv[1].p;
	AG_WidgetBinding *stringb;
	char *tp = &text[0], *s;

	stringb = AG_WidgetGetBinding(sbu->input, "string", &s);
	strlcpy(text, s, sizeof(text));

	if ((s = strsep(&tp, sbu->sep)) != NULL) {
		AG_MSpinbuttonSetValue(sbu, "xvalue", atoi(s));
	}
	if ((s = strsep(&tp, sbu->sep)) != NULL) {
		AG_MSpinbuttonSetValue(sbu, "yvalue", atoi(s));
	}
	AG_WidgetUnlockBinding(stringb);

	AG_PostEvent(NULL, sbu, "mspinbutton-changed", NULL);
}

static void
mspinbutton_up(int argc, union evarg *argv)
{
	AG_MSpinbutton *sbu = argv[1].p;

	pthread_mutex_lock(&sbu->lock);
	AG_MSpinbuttonAddValue(sbu, "yvalue", -sbu->inc);
	pthread_mutex_unlock(&sbu->lock);
}

static void
mspinbutton_down(int argc, union evarg *argv)
{
	AG_MSpinbutton *sbu = argv[1].p;
	
	pthread_mutex_lock(&sbu->lock);
	AG_MSpinbuttonAddValue(sbu, "yvalue", sbu->inc);
	pthread_mutex_unlock(&sbu->lock);
}

static void
mspinbutton_left(int argc, union evarg *argv)
{
	AG_MSpinbutton *sbu = argv[1].p;
	
	pthread_mutex_lock(&sbu->lock);
	AG_MSpinbuttonAddValue(sbu, "xvalue", -sbu->inc);
	pthread_mutex_unlock(&sbu->lock);
}

static void
mspinbutton_right(int argc, union evarg *argv)
{
	AG_MSpinbutton *sbu = argv[1].p;
	
	pthread_mutex_lock(&sbu->lock);
	AG_MSpinbuttonAddValue(sbu, "xvalue", sbu->inc);
	pthread_mutex_unlock(&sbu->lock);
}

void
AG_MSpinbuttonInit(AG_MSpinbutton *sbu, const char *sep, const char *label)
{
	AG_WidgetInit(sbu, "mspinbutton", &mspinbutton_ops,
	    AG_WIDGET_FOCUSABLE|AG_WIDGET_WFILL);
	AG_WidgetBind(sbu, "xvalue", AG_WIDGET_INT, &sbu->xvalue);
	AG_WidgetBind(sbu, "yvalue", AG_WIDGET_INT, &sbu->yvalue);
	AG_WidgetBind(sbu, "min", AG_WIDGET_INT, &sbu->min);
	AG_WidgetBind(sbu, "max", AG_WIDGET_INT, &sbu->max);

	sbu->xvalue = 0;
	sbu->yvalue = 0;
	sbu->min = 0;
	sbu->max = 0;
	sbu->inc = 1;
	sbu->writeable = 0;
	sbu->sep = sep;
	pthread_mutex_init(&sbu->lock, NULL);
	
	sbu->input = AG_TextboxNew(sbu, label);
	AG_SetEvent(sbu->input, "textbox-return", mspinbutton_return,
	    "%p", sbu);
	AG_SetEvent(sbu->input, "textbox-postchg", mspinbutton_textchg,
	    "%p", sbu);

	sbu->xdecbu = AG_ButtonNew(sbu, "-");
	AG_ButtonSetPadding(sbu->xdecbu, 0);
	AG_ButtonSetRepeatMode(sbu->xdecbu, 1);
	AG_SetEvent(sbu->xdecbu, "button-pushed", mspinbutton_left, "%p", sbu);

	sbu->xincbu = AG_ButtonNew(sbu, "+");
	AG_ButtonSetPadding(sbu->xincbu, 0);
	AG_ButtonSetRepeatMode(sbu->xincbu, 1);
	AG_SetEvent(sbu->xincbu, "button-pushed", mspinbutton_right, "%p", sbu);
	
	sbu->ydecbu = AG_ButtonNew(sbu, "-");
	AG_ButtonSetPadding(sbu->ydecbu, 0);
	AG_ButtonSetRepeatMode(sbu->ydecbu, 1);
	AG_SetEvent(sbu->ydecbu, "button-pushed", mspinbutton_up, "%p", sbu);

	sbu->yincbu = AG_ButtonNew(sbu, "+");
	AG_ButtonSetPadding(sbu->yincbu, 0);
	AG_ButtonSetRepeatMode(sbu->yincbu, 1);
	AG_SetEvent(sbu->yincbu, "button-pushed", mspinbutton_down, "%p", sbu);

	AG_SetEvent(sbu, "widget-bound", mspinbutton_bound, NULL);
	AG_SetEvent(sbu, "window-keydown", mspinbutton_keydown, NULL);
}

void
AG_MSpinbuttonDestroy(void *p)
{
	AG_MSpinbutton *sbu = p;

	pthread_mutex_destroy(&sbu->lock);
	AG_WidgetDestroy(sbu);
}

void
AG_MSpinbuttonScale(void *p, int w, int h)
{
	AG_MSpinbutton *sbu = p;
	int x = 0, y = 0;
	const int bw = 10;
	int bh = h/2;

	if (w == -1 && h == -1) {
		AGWIDGET_SCALE(sbu->input, -1, -1);
		AGWIDGET(sbu)->w = AGWIDGET(sbu->input)->w;
		AGWIDGET(sbu)->h = AGWIDGET(sbu->input)->h;

		AGWIDGET_SCALE(sbu->yincbu, -1, -1);
		AGWIDGET_SCALE(sbu->ydecbu, -1, -1);
		AGWIDGET_SCALE(sbu->xincbu, -1, -1);
		AGWIDGET_SCALE(sbu->xdecbu, -1, -1);

		AGWIDGET(sbu)->w += AGWIDGET(sbu->xdecbu)->w +
		                  max(AGWIDGET(sbu->yincbu)->w,
				      AGWIDGET(sbu->ydecbu)->w) +
		                  AGWIDGET(sbu->xincbu)->w;
		return;
	}

	AGWIDGET(sbu->input)->x = x;
	AGWIDGET(sbu->input)->y = y;
	AG_WidgetScale(sbu->input, w - bw*3, h);
	x += AGWIDGET(sbu->input)->w;

	AGWIDGET(sbu->xdecbu)->x = x;
	AGWIDGET(sbu->xdecbu)->y = y + bh/2;
	AG_WidgetScale(sbu->xdecbu, bw, bh);

	AGWIDGET(sbu->xincbu)->x = x + bh*2;
	AGWIDGET(sbu->xincbu)->y = y + bh/2;
	AG_WidgetScale(sbu->xincbu, bw, bh);

	AGWIDGET(sbu->ydecbu)->x = x + bh;
	AGWIDGET(sbu->ydecbu)->y = y;
	AG_WidgetScale(sbu->ydecbu, bw, bh);
	
	AGWIDGET(sbu->yincbu)->x = x + bh;
	AGWIDGET(sbu->yincbu)->y = y + bh;
	AG_WidgetScale(sbu->yincbu, bw, bh);
}

void
AG_MSpinbuttonDraw(void *p)
{
	AG_MSpinbutton *sbu = p;
	AG_WidgetBinding *xvalueb, *yvalueb;
	void *xvalue, *yvalue;

	if (AGWIDGET(sbu->input)->flags & AG_WIDGET_FOCUSED)
		return;

	xvalueb = AG_WidgetGetBinding(sbu, "xvalue", &xvalue);
	yvalueb = AG_WidgetGetBinding(sbu, "yvalue", &yvalue);
	switch (xvalueb->vtype) {
	case AG_WIDGET_INT:
		AG_TextboxPrintf(sbu->input, "%d%s%d",
		    *(int *)xvalue, sbu->sep, *(int *)yvalue);
		break;
	case AG_WIDGET_UINT:
		AG_TextboxPrintf(sbu->input, "%u%s%u",
		    *(u_int *)xvalue, sbu->sep, *(u_int *)yvalue);
		break;
	case AG_WIDGET_UINT8:
		AG_TextboxPrintf(sbu->input, "%u%s%u",
		    *(Uint8 *)xvalue, sbu->sep, *(Uint8 *)yvalue);
		break;
	case AG_WIDGET_SINT8:
		AG_TextboxPrintf(sbu->input, "%d%s%d",
		    *(Sint8 *)xvalue, sbu->sep, *(Sint8 *)yvalue);
		break;
	case AG_WIDGET_UINT16:
		AG_TextboxPrintf(sbu->input, "%u%s%u",
		    *(Uint16 *)xvalue, sbu->sep, *(Uint16 *)yvalue);
		break;
	case AG_WIDGET_SINT16:
		AG_TextboxPrintf(sbu->input, "%d%s%d",
		    *(Sint16 *)xvalue, sbu->sep, *(Sint16 *)yvalue);
		break;
	case AG_WIDGET_UINT32:
		AG_TextboxPrintf(sbu->input, "%u%s%u",
		    *(Uint32 *)xvalue, sbu->sep, *(Uint32 *)yvalue);
		break;
	case AG_WIDGET_SINT32:
		AG_TextboxPrintf(sbu->input, "%d%s%d",
		    *(Sint32 *)xvalue, sbu->sep, *(Sint32 *)yvalue);
		break;
	}
	AG_WidgetUnlockBinding(xvalueb);
	AG_WidgetUnlockBinding(yvalueb);
}

void
AG_MSpinbuttonAddValue(AG_MSpinbutton *sbu, const char *which, int inc)
{
	AG_WidgetBinding *valueb, *minb, *maxb;
	void *value;
	int *min, *max;

	valueb = AG_WidgetGetBinding(sbu, which, &value);
	minb = AG_WidgetGetBinding(sbu, "min", &min);
	maxb = AG_WidgetGetBinding(sbu, "max", &max);

	switch (valueb->vtype) {
	case AG_WIDGET_INT:
		if (*(int *)value+inc >= *min &&
		    *(int *)value+inc <= *max)
			*(int *)value += inc;
		break;
	case AG_WIDGET_UINT:
		if (*(u_int *)value+inc >= *min &&
		    *(u_int *)value+inc <= *max)
			*(u_int *)value += inc;
		break;
	case AG_WIDGET_UINT8:
		if (*(Uint8 *)value+inc >= *min &&
		    *(Uint8 *)value+inc <= *max)
			*(Uint8 *)value += inc;
		break;
	case AG_WIDGET_SINT8:
		if (*(Sint8 *)value+inc >= *min &&
		    *(Sint8 *)value+inc <= *max)
			*(Sint8 *)value += inc;
		break;
	case AG_WIDGET_UINT16:
		if (*(Uint16 *)value+inc >= *min &&
		    *(Uint16 *)value+inc <= *max)
			*(Uint16 *)value += inc;
		break;
	case AG_WIDGET_SINT16:
		if (*(Sint16 *)value+inc >= *min &&
		    *(Sint16 *)value+inc <= *max)
			*(Sint16 *)value += inc;
		break;
	case AG_WIDGET_UINT32:
		if (*(Uint32 *)value+inc >= *min &&
		    *(Uint32 *)value+inc <= *max)
			*(Uint32 *)value += inc;
		break;
	case AG_WIDGET_SINT32:
		if (*(Sint32 *)value+inc >= *min &&
		    *(Sint32 *)value+inc <= *max)
			*(Sint32 *)value += inc;
		break;
	default:
		break;
	}

	AG_PostEvent(NULL, sbu, "mspinbutton-changed", "%s", which);
	AG_WidgetBindingChanged(valueb);

	AG_WidgetUnlockBinding(maxb);
	AG_WidgetUnlockBinding(minb);
	AG_WidgetUnlockBinding(valueb);
}

void
AG_MSpinbuttonSetValue(AG_MSpinbutton *sbu, const char *which, ...)
{
	AG_WidgetBinding *valueb, *minb, *maxb;
	void *value;
	int *min, *max;
	va_list ap;

	valueb = AG_WidgetGetBinding(sbu, which, &value);
	minb = AG_WidgetGetBinding(sbu, "min", &min);
	maxb = AG_WidgetGetBinding(sbu, "max", &max);

	va_start(ap, which);
	switch (valueb->vtype) {
	case AG_WIDGET_INT:
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
	case AG_WIDGET_UINT:
		{
			u_int i = va_arg(ap, unsigned int);

			if (i < (u_int)*min) {
				*(u_int *)value = *min;
			} else if (i > (u_int)*max) {
				*(u_int *)value = *max;
			} else {
				*(u_int *)value = i;
			}
		}
		break;
	case AG_WIDGET_UINT8:
		{
			Uint8 i = (Uint8)va_arg(ap, int);

			if (i < (Uint8)*min) {
				*(Uint8 *)value = *min;
			} else if (i > (Uint8)*max) {
				*(Uint8 *)value = *max;
			} else {
				*(Uint8 *)value = i;
			}
		}
		break;
	case AG_WIDGET_SINT8:
		{
			Sint8 i = (Sint8)va_arg(ap, int);

			if (i < (Sint8)*min) {
				*(Sint8 *)value = *min;
			} else if (i > (Sint8)*max) {
				*(Sint8 *)value = *max;
			} else {
				*(Sint8 *)value = i;
			}
		}
		break;
	case AG_WIDGET_UINT16:
		{
			Uint16 i = (Uint16)va_arg(ap, int);

			if (i < (Uint16)*min) {
				*(Uint16 *)value = *min;
			} else if (i > (Uint16)*max) {
				*(Uint16 *)value = *max;
			} else {
				*(Uint16 *)value = i;
			}
		}
		break;
	case AG_WIDGET_SINT16:
		{
			Sint16 i = (Sint16)va_arg(ap, int);

			if (i < (Sint16)*min) {
				*(Sint16 *)value = *min;
			} else if (i > (Sint16)*max) {
				*(Sint16 *)value = *max;
			} else {
				*(Sint16 *)value = i;
			}
		}
		break;
	case AG_WIDGET_UINT32:
		{
			Uint32 i = (Uint32)va_arg(ap, int);

			if (i < (Uint32)*min) {
				*(Uint32 *)value = *min;
			} else if (i > (Uint32)*max) {
				*(Uint32 *)value = *max;
			} else {
				*(Uint32 *)value = i;
			}
		}
		break;
	case AG_WIDGET_SINT32:
		{
			Sint32 i = (Sint32)va_arg(ap, int);

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

	AG_PostEvent(NULL, sbu, "mspinbutton-changed", "%s", which);
	AG_WidgetBindingChanged(valueb);

	AG_WidgetUnlockBinding(valueb);
	AG_WidgetUnlockBinding(minb);
	AG_WidgetUnlockBinding(maxb);
}

void
AG_MSpinbuttonSetMin(AG_MSpinbutton *sbu, int nmin)
{
	AG_WidgetBinding *minb;
	int *min;

	minb = AG_WidgetGetBinding(sbu, "min", &min);
	*min = nmin;
	AG_WidgetBindingChanged(minb);
	AG_WidgetUnlockBinding(minb);
}

void
AG_MSpinbuttonSetMax(AG_MSpinbutton *sbu, int nmax)
{
	AG_WidgetBinding *maxb;
	int *max;

	maxb = AG_WidgetGetBinding(sbu, "max", &max);
	*max = nmax;
	AG_WidgetBindingChanged(maxb);
	AG_WidgetUnlockBinding(maxb);
}

void
AG_MSpinbuttonSetRange(AG_MSpinbutton *sbu, int nmin, int nmax)
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
AG_MSpinbuttonSetIncrement(AG_MSpinbutton *sbu, int inc)
{
	pthread_mutex_lock(&sbu->lock);
	sbu->inc = inc;
	pthread_mutex_unlock(&sbu->lock);
}

void
AG_MSpinbuttonSetWriteable(AG_MSpinbutton *sbu, int writeable)
{
	pthread_mutex_lock(&sbu->lock);

	sbu->writeable = writeable;
	AG_TextboxSetWriteable(sbu->input, writeable);
	if (writeable) {
		AG_ButtonEnable(sbu->xincbu);
		AG_ButtonEnable(sbu->xdecbu);
		AG_ButtonEnable(sbu->yincbu);
		AG_ButtonEnable(sbu->ydecbu);
	} else {
		AG_ButtonDisable(sbu->xincbu);
		AG_ButtonDisable(sbu->xdecbu);
		AG_ButtonDisable(sbu->yincbu);
		AG_ButtonDisable(sbu->ydecbu);
	}
	pthread_mutex_unlock(&sbu->lock);
}
