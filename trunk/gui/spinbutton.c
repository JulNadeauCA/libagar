/*
 * Copyright (c) 2003-2010 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * LEGACY: Use AG_Numerical(3) instead of this widget.
 */

#include <agar/core/core.h>
#include <agar/gui/spinbutton.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>

#include <stdarg.h>
#include <string.h>

AG_Spinbutton *
AG_SpinbuttonNew(void *parent, Uint flags, const char *label)
{
	AG_Spinbutton *sbu;

	sbu = Malloc(sizeof(AG_Spinbutton));
	AG_ObjectInit(sbu, &agSpinbuttonClass);
	
	if (label != NULL) {
		AG_TextboxSetLabelS(sbu->input, label);
	}
	if (!(flags & AG_SPINBUTTON_NOHFILL))	{ AG_ExpandHoriz(sbu); }
	if (  flags & AG_SPINBUTTON_VFILL)	{ AG_ExpandVert(sbu); }

	AG_ObjectAttach(parent, sbu);
	return (sbu);
}

static void
Bound(AG_Event *event)
{
	AG_Spinbutton *sbu = AG_SELF();
	AG_Variable *binding = AG_PTR(1);

	if (strcmp(binding->name, "value") == 0) {
		switch (AG_VARIABLE_TYPE(binding)) {
		case AG_VARIABLE_INT:
			sbu->min = AG_INT_MIN+1;
			sbu->max = AG_INT_MAX-1;
			break;
		case AG_VARIABLE_UINT:
			sbu->min = 0;
			sbu->max = AG_UINT_MAX-1;
			break;
		case AG_VARIABLE_UINT8:
			sbu->min = 0;
			sbu->max = 0xffU;
			break;
		case AG_VARIABLE_SINT8:
			sbu->min = -0x7f+1;
			sbu->max =  0x7f-1;
			break;
		case AG_VARIABLE_UINT16:
			sbu->min = 0;
			sbu->max = 0xffffU;
			break;
		case AG_VARIABLE_SINT16:
			sbu->min = -0x7fff+1;
			sbu->max =  0x7fff-1;
			break;
		case AG_VARIABLE_UINT32:
			sbu->min = 0;
			sbu->max = 0xffffffffU;
			break;
		case AG_VARIABLE_SINT32:
			sbu->min = -0x7fffffff+1;
			sbu->max =  0x7fffffff-1;
			break;
		default:
			break;
		}
	}
	AG_Redraw(sbu);
}

static void
KeyDown(AG_Event *event)
{
	AG_Spinbutton *sbu = AG_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case AG_KEY_UP:
		AG_SpinbuttonAddValue(sbu, sbu->incr);
		break;
	case AG_KEY_DOWN:
		AG_SpinbuttonAddValue(sbu, -sbu->incr);
		break;
	default:
		break;
	}
}

static void
PostChange(AG_Event *event)
{
	AG_Spinbutton *sbu = AG_PTR(1);

	AG_ObjectLock(sbu);
	AG_SpinbuttonSetValue(sbu, atoi(sbu->inTxt));
	AG_PostEvent(NULL, sbu, "spinbutton-changed", NULL);
	AG_ObjectUnlock(sbu);
}

static void
Return(AG_Event *event)
{
	AG_Spinbutton *sbu = AG_PTR(1);

	AG_ObjectLock(sbu);
	AG_PostEvent(NULL, sbu, "spinbutton-return", NULL);
	AG_WidgetUnfocus(sbu->input);
	AG_ObjectUnlock(sbu);
}

static void
Increment(AG_Event *event)
{
	AG_Spinbutton *sbu = AG_PTR(1);

	AG_ObjectLock(sbu);
	AG_SpinbuttonAddValue(sbu, sbu->incr);
	AG_PostEvent(NULL, sbu, "spinbutton-changed", NULL);
	AG_ObjectUnlock(sbu);
}

static void
Decrement(AG_Event *event)
{
	AG_Spinbutton *sbu = AG_PTR(1);
	
	AG_ObjectLock(sbu);
	AG_SpinbuttonAddValue(sbu, -sbu->incr);
	AG_PostEvent(NULL, sbu, "spinbutton-changed", NULL);
	AG_ObjectUnlock(sbu);
}

static void
Init(void *obj)
{
	AG_Spinbutton *sbu = obj;
	
	WIDGET(sbu)->flags |= AG_WIDGET_TABLE_EMBEDDABLE;

	AG_BindInt(sbu, "value", &sbu->value);
	AG_BindInt(sbu, "min", &sbu->min);
	AG_BindInt(sbu, "max", &sbu->max);
	
	AG_RedrawOnChange(sbu, 250, "value");

	sbu->value = 0;
	sbu->incr = 1;
	sbu->writeable = 0;
	sbu->min = 0;
	sbu->max = 0;
	sbu->inTxt[0] = '\0';

	sbu->input = AG_TextboxNewS(sbu, AG_TEXTBOX_EXCL, NULL);
	AG_TextboxBindASCII(sbu->input, sbu->inTxt, sizeof(sbu->inTxt));
	AG_TextboxSizeHint(sbu->input, "8888");

	AG_SetEvent(sbu, "bound", Bound, NULL);
	AG_SetEvent(sbu, "key-down", KeyDown, NULL);

	sbu->incbu = AG_ButtonNewS(sbu, AG_BUTTON_REPEAT, _("+"));
	AG_ButtonSetPadding(sbu->incbu, 0,0,0,0);
	AG_LabelSetPadding(sbu->incbu->lbl, 0,0,0,0);
	AG_WidgetSetFocusable(sbu->incbu, 0);

	sbu->decbu = AG_ButtonNewS(sbu, AG_BUTTON_REPEAT, _("-"));
	AG_ButtonSetPadding(sbu->decbu, 0,0,0,0);
	AG_LabelSetPadding(sbu->decbu->lbl, 0,0,0,0);
	AG_WidgetSetFocusable(sbu->decbu, 0);
	
	AG_SetEvent(sbu->input, "textbox-return", Return, "%p", sbu);
	AG_SetEvent(sbu->input, "textbox-postchg", PostChange, "%p", sbu);
	AG_SetEvent(sbu->incbu, "button-pushed", Increment, "%p", sbu);
	AG_SetEvent(sbu->decbu, "button-pushed", Decrement, "%p", sbu);
}

static void
Draw(void *obj)
{
	AG_Spinbutton *sbu = obj;
	AG_Variable *value;
	void *p;

	AG_WidgetDraw(sbu->input);
	AG_WidgetDraw(sbu->incbu);
	AG_WidgetDraw(sbu->decbu);

	if (AG_WidgetIsFocused(sbu->input)) {
		/* The value is being edited. */
		return;
	}

	value = AG_GetVariable(sbu, "value", &p);
	switch (AG_VARIABLE_TYPE(value)) {
	case AG_VARIABLE_INT:
		AG_TextboxPrintf(sbu->input, "%d", *(int *)p);
		break;
	case AG_VARIABLE_UINT:
		AG_TextboxPrintf(sbu->input, "%u", *(Uint *)p);
		break;
	case AG_VARIABLE_UINT8:
		AG_TextboxPrintf(sbu->input, "%u", *(Uint8 *)p);
		break;
	case AG_VARIABLE_SINT8:
		AG_TextboxPrintf(sbu->input, "%d", *(Sint8 *)p);
		break;
	case AG_VARIABLE_UINT16:
		AG_TextboxPrintf(sbu->input, "%u", *(Uint16 *)p);
		break;
	case AG_VARIABLE_SINT16:
		AG_TextboxPrintf(sbu->input, "%d", *(Sint16 *)p);
		break;
	case AG_VARIABLE_UINT32:
		AG_TextboxPrintf(sbu->input, "%u", *(Uint32 *)p);
		break;
	case AG_VARIABLE_SINT32:
		AG_TextboxPrintf(sbu->input, "%d", *(Sint32 *)p);
		break;
	default:
		break;
	}
	AG_UnlockVariable(value);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Spinbutton *num = obj;
	AG_SizeReq rChld, rInc, rDec;

	AG_WidgetSizeReq(num->input, &rChld);
	r->w = rChld.w;
	r->h = rChld.h;
	AG_WidgetSizeReq(num->incbu, &rInc);
	AG_WidgetSizeReq(num->decbu, &rDec);
	r->w += MAX(rInc.w, rDec.w);
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Spinbutton *num = obj;
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
	AG_Variable *valueb, *minb, *maxb;
	void *value;
	int *min, *max;

	AG_ObjectLock(sbu);
	valueb = AG_GetVariable(sbu, "value", &value);
	minb = AG_GetVariable(sbu, "min", &min);
	maxb = AG_GetVariable(sbu, "max", &max);

	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_INT:
		*(int *)value = *(int *)value+inc < *min ? *min :
		                *(int *)value+inc > *max ? *max :
				*(int *)value+inc;
		break;
	case AG_VARIABLE_UINT:
		*(unsigned *)value = *(unsigned *)value+inc < *min ? *min :
		                     *(unsigned *)value+inc > *max ? *max :
			  	     *(unsigned *)value+inc;
		break;
	case AG_VARIABLE_UINT8:
		*(Uint8 *)value = *(Uint8 *)value+inc < *min ? *min :
		                  *(Uint8 *)value+inc > *max ? *max :
			  	  *(Uint8 *)value+inc;
		break;
	case AG_VARIABLE_SINT8:
		*(Sint8 *)value = *(Sint8 *)value+inc < *min ? *min :
		                  *(Sint8 *)value+inc > *max ? *max :
			  	  *(Sint8 *)value+inc;
		break;
	case AG_VARIABLE_UINT16:
		*(Uint16 *)value = *(Uint16 *)value+inc < *min ? *min :
		                   *(Uint16 *)value+inc > *max ? *max :
			  	   *(Uint16 *)value+inc;
		break;
	case AG_VARIABLE_SINT16:
		*(Sint16 *)value = *(Sint16 *)value+inc < *min ? *min :
		                   *(Sint16 *)value+inc > *max ? *max :
			  	   *(Sint16 *)value+inc;
		break;
	case AG_VARIABLE_UINT32:
		*(Uint32 *)value = *(Uint32 *)value+inc < *min ? *min :
		                   *(Uint32 *)value+inc > *max ? *max :
			  	   *(Uint32 *)value+inc;
		break;
	case AG_VARIABLE_SINT32:
		*(Sint32 *)value = *(Sint32 *)value+inc < *min ? *min :
		                   *(Sint32 *)value+inc > *max ? *max :
			  	   *(Sint32 *)value+inc;
		break;
	default:
		break;
	}

	AG_PostEvent(NULL, sbu, "spinbutton-changed", NULL);

	AG_UnlockVariable(maxb);
	AG_UnlockVariable(minb);
	AG_UnlockVariable(valueb);
	AG_ObjectUnlock(sbu);
	AG_Redraw(sbu);
}

void
AG_SpinbuttonSetValue(AG_Spinbutton *sbu, ...)
{
	AG_Variable *valueb, *minb, *maxb;
	void *value;
	int *min, *max;
	va_list ap;

	AG_ObjectLock(sbu);
	valueb = AG_GetVariable(sbu, "value", &value);
	minb = AG_GetVariable(sbu, "min", &min);
	maxb = AG_GetVariable(sbu, "max", &max);

	va_start(ap, sbu);
	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_INT:
		{
			int i = va_arg(ap, int);

			*(int *)value = i < *min ? *min :
			                i > *max ? *max :
					i;
		}
		break;
	case AG_VARIABLE_UINT:
		{
			unsigned i = va_arg(ap, unsigned int);

			*(unsigned *)value = i<(unsigned)*min ? (unsigned)*min :
			                     i>(unsigned)*max ? (unsigned)*max :
					     i;
		}
		break;
	case AG_VARIABLE_UINT8:
		{
			Uint8 i = (Uint8)va_arg(ap, int);

			*(Uint8 *)value = i < (Uint8)*min ? (Uint8)*min :
			                  i > (Uint8)*max ? (Uint8)*max :
					  i;
		}
		break;
	case AG_VARIABLE_SINT8:
		{
			Sint8 i = (Sint8)va_arg(ap, int);

			*(Sint8 *)value = i < (Sint8)*min ? (Sint8)*min :
			                  i > (Sint8)*max ? (Sint8)*max :
					  i;
		}
		break;
	case AG_VARIABLE_UINT16:
		{
			Uint16 i = (Uint16)va_arg(ap, int);

			*(Uint16 *)value = i < (Uint16)*min ? (Uint16)*min :
			                   i > (Uint16)*max ? (Uint16)*max :
					   i;
		}
		break;
	case AG_VARIABLE_SINT16:
		{
			Sint16 i = (Sint16)va_arg(ap, int);

			*(Sint16 *)value = i < (Sint16)*min ? (Sint16)*min :
			                   i > (Sint16)*max ? (Sint16)*max :
					   i;
		}
		break;
	case AG_VARIABLE_UINT32:
		{
			Uint32 i = (Uint32)va_arg(ap, int);

			*(Uint32 *)value = i < (Uint32)*min ? (Uint32)*min :
			                   i > (Uint32)*max ? (Uint32)*max :
					   i;
		}
		break;
	case AG_VARIABLE_SINT32:
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

	AG_UnlockVariable(valueb);
	AG_UnlockVariable(minb);
	AG_UnlockVariable(maxb);
	AG_ObjectUnlock(sbu);
	AG_Redraw(sbu);
}

void
AG_SpinbuttonSetMin(AG_Spinbutton *sbu, int nmin)
{
	AG_Variable *minb;
	int *min;

	AG_ObjectLock(sbu);
	minb = AG_GetVariable(sbu, "min", &min);
	*min = nmin;
	AG_UnlockVariable(minb);
	AG_ObjectUnlock(sbu);
}

void
AG_SpinbuttonSetMax(AG_Spinbutton *sbu, int nmax)
{
	AG_Variable *maxb;
	int *max;

	AG_ObjectLock(sbu);
	maxb = AG_GetVariable(sbu, "max", &max);
	*max = nmax;
	AG_UnlockVariable(maxb);
	AG_ObjectUnlock(sbu);
}

void
AG_SpinbuttonSetRange(AG_Spinbutton *sbu, int nmin, int nmax)
{
	AG_Variable *minb, *maxb;
	int *min, *max;

	AG_ObjectLock(sbu);
	minb = AG_GetVariable(sbu, "min", &min);
	maxb = AG_GetVariable(sbu, "max", &max);
	*min = nmin;
	*max = nmax;
	AG_UnlockVariable(minb);
	AG_UnlockVariable(maxb);
	AG_ObjectUnlock(sbu);
}

void
AG_SpinbuttonSetIncrement(AG_Spinbutton *sbu, int incr)
{
	AG_ObjectLock(sbu);
	sbu->incr = incr;
	AG_ObjectUnlock(sbu);
}

void
AG_SpinbuttonSetWriteable(AG_Spinbutton *sbu, int writeable)
{
	AG_ObjectLock(sbu);
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
	AG_ObjectUnlock(sbu);
	AG_Redraw(sbu);
}

AG_WidgetClass agSpinbuttonClass = {
	{
		"Agar(Widget:Spinbutton)",
		sizeof(AG_Spinbutton),
		{ 0,0 },
		Init,
		NULL,			/* free */
		NULL,			/* destroy */
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
