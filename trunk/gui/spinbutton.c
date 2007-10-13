/*
 * Copyright (c) 2003-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <core/core.h>
#include <core/view.h>

#include "spinbutton.h"

#include "window.h"
#include "primitive.h"

#include <stdarg.h>
#include <string.h>
#include <limits.h>

AG_Spinbutton *
AG_SpinbuttonNew(void *parent, Uint flags, const char *label)
{
	AG_Spinbutton *sbu;

	sbu = Malloc(sizeof(AG_Spinbutton), M_OBJECT);
	AG_SpinbuttonInit(sbu, flags, label);
	AG_ObjectAttach(parent, sbu);
	return (sbu);
}

static void
spinbutton_bound(AG_Event *event)
{
	AG_Spinbutton *sbu = AG_SELF();
	AG_WidgetBinding *binding = AG_PTR(1);

	if (strcmp(binding->name, "value") == 0) {
		AG_MutexLock(&sbu->lock);
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
		AG_MutexUnlock(&sbu->lock);
	}
}

static void
spinbutton_keydown(AG_Event *event)
{
	AG_Spinbutton *sbu = AG_SELF();
	int keysym = AG_INT(1);

	AG_MutexLock(&sbu->lock);
	switch (keysym) {
	case SDLK_UP:
		AG_SpinbuttonAddValue(sbu, sbu->incr);
		break;
	case SDLK_DOWN:
		AG_SpinbuttonAddValue(sbu, -sbu->incr);
		break;
	}
	AG_MutexUnlock(&sbu->lock);
}

static void
spinbutton_changed(AG_Event *event)
{
	AG_Spinbutton *sbu = AG_PTR(1);
	AG_WidgetBinding *stringb;
	char *s;

	stringb = AG_WidgetGetBinding(sbu->input, "string", &s);
	AG_SpinbuttonSetValue(sbu, atoi(s));
	AG_WidgetUnlockBinding(stringb);

	AG_PostEvent(NULL, sbu, "spinbutton-changed", NULL);
}

static void
spinbutton_return(AG_Event *event)
{
	AG_Spinbutton *sbu = AG_PTR(1);

	AG_PostEvent(NULL, sbu, "spinbutton-return", NULL);
	AG_WidgetUnfocus(sbu->input);
}

static void
spinbutton_inc(AG_Event *event)
{
	AG_Spinbutton *sbu = AG_PTR(1);

	AG_MutexLock(&sbu->lock);
	AG_SpinbuttonAddValue(sbu, sbu->incr);
	AG_MutexUnlock(&sbu->lock);
	
	AG_PostEvent(NULL, sbu, "spinbutton-changed", NULL);
}

static void
spinbutton_dec(AG_Event *event)
{
	AG_Spinbutton *sbu = AG_PTR(1);
	
	AG_MutexLock(&sbu->lock);
	AG_SpinbuttonAddValue(sbu, -sbu->incr);
	AG_MutexUnlock(&sbu->lock);
	
	AG_PostEvent(NULL, sbu, "spinbutton-changed", NULL);
}

void
AG_SpinbuttonInit(AG_Spinbutton *sbu, Uint flags, const char *label)
{
	Uint wflags = 0;

	if ((flags & AG_SPINBUTTON_NOHFILL)==0) { wflags |= AG_WIDGET_HFILL; }
	if (flags & AG_SPINBUTTON_VFILL) { wflags |= AG_WIDGET_VFILL; }

	AG_WidgetInit(sbu, &agSpinbuttonOps, wflags);
	AG_WidgetBind(sbu, "value", AG_WIDGET_INT, &sbu->value);
	AG_WidgetBind(sbu, "min", AG_WIDGET_INT, &sbu->min);
	AG_WidgetBind(sbu, "max", AG_WIDGET_INT, &sbu->max);

	sbu->value = 0;
	sbu->incr = 1;
	sbu->writeable = 0;
	sbu->min = 0;
	sbu->max = 0;
	AG_MutexInit(&sbu->lock);
	sbu->input = AG_TextboxNew(sbu, 0, label);
	AG_TextboxSizeHint(sbu->input, "8888");

	AG_SetEvent(sbu, "widget-bound", spinbutton_bound, NULL);
	AG_SetEvent(sbu, "window-keydown", spinbutton_keydown, NULL);

	sbu->incbu = AG_ButtonNew(sbu, AG_BUTTON_REPEAT, _("+"));
	AG_ButtonSetPadding(sbu->incbu, 1, 1, 1, 1);
	sbu->decbu = AG_ButtonNew(sbu, AG_BUTTON_REPEAT, _("-"));
	AG_ButtonSetPadding(sbu->decbu, 1, 1, 1, 1);
	AG_WidgetSetFocusable(sbu->incbu, 0);
	AG_WidgetSetFocusable(sbu->decbu, 0);
	
	AG_SetEvent(sbu->input, "textbox-return", spinbutton_return, "%p", sbu);
	AG_SetEvent(sbu->input, "textbox-postchg", spinbutton_changed, "%p",
	    sbu);
	AG_SetEvent(sbu->incbu, "button-pushed", spinbutton_inc, "%p", sbu);
	AG_SetEvent(sbu->decbu, "button-pushed", spinbutton_dec, "%p", sbu);
}

static void
Destroy(void *p)
{
	AG_Spinbutton *sbu = p;

	AG_MutexDestroy(&sbu->lock);
	AG_WidgetDestroy(sbu);
}

static void
Draw(void *p)
{
	AG_Spinbutton *sbu = p;
	AG_WidgetBinding *valueb;
	void *value;

	if (AG_WidgetFocused(sbu->input)) {
		/* The value is being edited. */
		return;
	}

	valueb = AG_WidgetGetBinding(sbu, "value", &value);
	switch (valueb->type) {
	case AG_WIDGET_INT:
		AG_TextboxPrintf(sbu->input, "%d", *(int *)value);
		break;
	case AG_WIDGET_UINT:
		AG_TextboxPrintf(sbu->input, "%u", *(Uint *)value);
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
	}
	AG_WidgetUnlockBinding(valueb);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Spinbutton *num = p;
	AG_SizeReq rChld, rInc, rDec;

	AG_WidgetSizeReq(num->input, &rChld);
	r->w = rChld.w;
	r->h = rChld.h;
	AG_WidgetSizeReq(num->incbu, &rInc);
	AG_WidgetSizeReq(num->decbu, &rDec);
	r->w += MAX(rInc.w, rDec.w);
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Spinbutton *num = p;
	AG_SizeAlloc aChld;
	AG_SizeReq rUnits;
	int szBtn = a->h/2;

	if (a->h < 4 || a->w < szBtn+4)
		return (-1);

	rUnits.w = 0;
	rUnits.h = 0;
	
	/* Size input textbox */
	aChld.x = 0;
	aChld.y = 0;
	aChld.w = a->w - rUnits.w - szBtn - 4;
	aChld.h = a->h;
	AG_WidgetSizeAlloc(num->input, &aChld);
	aChld.x += aChld.w + 2;

	/* Size increment buttons */
	aChld.w = szBtn;
	aChld.h = szBtn;
	AG_WidgetSizeAlloc(num->incbu, &aChld);
	aChld.y += aChld.h;
	if (aChld.h*2 < a->h) {
		aChld.h++;
	}
	AG_WidgetSizeAlloc(num->decbu, &aChld);
	return (0);
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
	AG_MutexLock(&sbu->lock);
	sbu->incr = incr;
	AG_MutexUnlock(&sbu->lock);
}

void
AG_SpinbuttonSetWriteable(AG_Spinbutton *sbu, int writeable)
{
	AG_MutexLock(&sbu->lock);
	sbu->writeable = writeable;
	if (writeable) {
		AG_WidgetEnable(sbu->incbu);
		AG_WidgetEnable(sbu->decbu);
		AG_WidgetEnable(sbu->input);
	} else {
		AG_WidgetDisable(sbu->incbu);
		AG_WidgetDisable(sbu->decbu);
		AG_WidgetDisable(sbu->input);
	}
	AG_MutexUnlock(&sbu->lock);
}

const AG_WidgetOps agSpinbuttonOps = {
	{
		"AG_Widget:AG_Spinbutton",
		sizeof(AG_Spinbutton),
		{ 0,0 },
		NULL,			/* init */
		NULL,			/* reinit */
		Destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
