/*
 * Copyright (c) 2003-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Integer-only spinbutton group with two directions.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/mspinbutton.h>
#include <agar/gui/window.h>

#include <stdarg.h>
#include <string.h>

AG_MSpinbutton *
AG_MSpinbuttonNew(void *parent, Uint flags, const char *sep, const char *label)
{
	AG_MSpinbutton *msb;

	msb = Malloc(sizeof(AG_MSpinbutton));
	AG_ObjectInit(msb, &agMSpinbuttonClass);

	if (flags & AG_MSPINBUTTON_HFILL) { WIDGET(msb)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_MSPINBUTTON_VFILL) { WIDGET(msb)->flags |= AG_WIDGET_VFILL; }

	msb->sep = sep;

	if (label != NULL) {
		AG_TextboxSetLabelS(msb->input, label);
	}
	AG_ObjectAttach(parent, msb);
	return (msb);
}

static void
Bound(AG_Event *event)
{
	AG_MSpinbutton *msb = AG_MSPINBUTTON_SELF();
	const AG_Variable *V = AG_PTR(1);

	if (strcmp(V->name, "xvalue") != 0 &&
	    strcmp(V->name, "yvalue") != 0)
		return;

	switch (AG_VARIABLE_TYPE(V)) {
	case AG_VARIABLE_INT:
		msb->min = AG_INT_MIN+1;
		msb->max = AG_INT_MAX-1;
		break;
	case AG_VARIABLE_UINT:
		msb->min = 0;
		msb->max = AG_UINT_MAX-1;
		break;
	case AG_VARIABLE_UINT8:
		msb->min = 0;
		msb->max = 0xffU;
		break;
	case AG_VARIABLE_SINT8:
		msb->min = -0x7f+1;
		msb->max =  0x7f-1;
		break;
	case AG_VARIABLE_UINT16:
		msb->min = 0;
		msb->max = 0xffffU;
		break;
	case AG_VARIABLE_SINT16:
		msb->min = -0x7fff+1;
		msb->max =  0x7fff-1;
		break;
	case AG_VARIABLE_UINT32:
		msb->min = 0;
		msb->max = 0xffffffffU;
		break;
	case AG_VARIABLE_SINT32:
		msb->min = -0x7fffffff+1;
		msb->max =  0x7fffffff-1;
		break;
	default:
		break;
	}
	AG_Redraw(msb);
}

#if 0
static void
KeyDown(AG_Event *event)
{
	AG_MSpinbutton *msb = AG_MSPINBUTTON_PTR(1);
	int keysym = AG_INT(2);

	switch (keysym) {
	case AG_KEY_LEFT:
		AG_MSpinbuttonAddValue(msb, "xvalue", -msb->inc);
		break;
	case AG_KEY_RIGHT:
		AG_MSpinbuttonAddValue(msb, "xvalue", msb->inc);
		break;
	case AG_KEY_UP:
		AG_MSpinbuttonAddValue(msb, "yvalue", -msb->inc);
		break;
	case AG_KEY_DOWN:
		AG_MSpinbuttonAddValue(msb, "yvalue", msb->inc);
		break;
	default:
		break;
	}
}
#endif

static void
TextReturn(AG_Event *event)
{
	AG_MSpinbutton *msb = AG_MSPINBUTTON_PTR(1);
	char inTxt[64];
	char *tp = &inTxt[0], *s;

	AG_ObjectLock(msb);

	Strlcpy(inTxt, msb->inTxt, sizeof(inTxt));
	if ((s = AG_Strsep(&tp, msb->sep)) != NULL) {
		AG_MSpinbuttonSetValue(msb, "xvalue", atoi(s));
	}
	if ((s = AG_Strsep(&tp, msb->sep)) != NULL) {
		AG_MSpinbuttonSetValue(msb, "yvalue", atoi(s));
	}
	AG_PostEvent(msb, "mspinbutton-return", NULL);
	AG_WidgetUnfocus(msb->input);

	AG_Redraw(msb);
	AG_ObjectUnlock(msb);
}

static void
TextChanged(AG_Event *event)
{
	AG_MSpinbutton *msb = AG_MSPINBUTTON_PTR(1);
	char inTxt[64];
	char *tp = &inTxt[0], *s;
	
	AG_ObjectLock(msb);

	Strlcpy(inTxt, msb->inTxt, sizeof(inTxt));
	if ((s = AG_Strsep(&tp, msb->sep)) != NULL) {
		AG_MSpinbuttonSetValue(msb, "xvalue", atoi(s));
	}
	if ((s = AG_Strsep(&tp, msb->sep)) != NULL) {
		AG_MSpinbuttonSetValue(msb, "yvalue", atoi(s));
	}
	AG_PostEvent(msb, "mspinbutton-changed", NULL);

	AG_Redraw(msb);
	AG_ObjectUnlock(msb);
}

static void
Increment(AG_Event *event)
{
	AG_MSpinbutton *msb = AG_MSPINBUTTON_PTR(1);
	const char *which = AG_STRING(2);
	const int sign = AG_INT(3);

	AG_MSpinbuttonAddValue(msb, which, sign*msb->inc);
}

static void
Init(void *obj)
{
	AG_MSpinbutton *msb = obj;
	AG_Textbox *tb;
	const Uint btnFlags = AG_BUTTON_NO_FOCUS | AG_BUTTON_REPEAT;
	int i;

	msb->xvalue = 0;
	msb->yvalue = 0;
	msb->min = 0;
	msb->max = 0;
	msb->inc = 1;
	msb->writeable = 0;
	msb->sep = ",";
	msb->inTxt[0] = '\0';
	
	AG_BindInt(msb, "xvalue", &msb->xvalue);
	AG_BindInt(msb, "yvalue", &msb->yvalue);
	AG_BindInt(msb, "min", &msb->min);
	AG_BindInt(msb, "max", &msb->max);
	
	tb = msb->input = AG_TextboxNewS(msb, 0, NULL);
	AG_TextboxBindASCII(tb, msb->inTxt, sizeof(msb->inTxt));
	AG_TextboxSizeHint(tb, "8888,8888");
	AG_SetEvent(tb, "textbox-return", TextReturn, "%p", msb);
	AG_SetEvent(tb, "textbox-postchg", TextChanged, "%p", msb);
/*	AG_AddEvent(tb->ed, "key-down", KeyDown, "%p", msb); */

	msb->btn[0] = AG_ButtonNewS(msb, btnFlags, "\xE2\x96\xB4"); /* UP (U+25B4) */
	msb->btn[1] = AG_ButtonNewS(msb, btnFlags, "\xE2\x97\x82"); /* LEFT (U+25C2) */
	msb->btn[2] = AG_ButtonNewS(msb, btnFlags, "\xE2\x96\xBE"); /* DOWN (U+25BE) */
	msb->btn[3] = AG_ButtonNewS(msb, btnFlags, "\xE2\x96\xB8"); /* RIGHT (U+25B8) */

	AG_SetEvent(msb->btn[0], "button-pushed", Increment, "%p,%s,%i", msb, "yvalue", -1); /* UP */
	AG_SetEvent(msb->btn[1], "button-pushed", Increment, "%p,%s,%i", msb, "xvalue", -1); /* LEFT */
	AG_SetEvent(msb->btn[2], "button-pushed", Increment, "%p,%s,%i", msb, "yvalue", +1); /* DOWN */
	AG_SetEvent(msb->btn[3], "button-pushed", Increment, "%p,%s,%i", msb, "xvalue", +1); /* RIGHT */

	for (i = 0; i < 4; i++) {
		AG_SetStyle(msb->btn[i], "padding", "0");
		AG_SetStyle(msb->btn[i], "font-size", "70%");
	}

	AG_SetEvent(msb, "bound", Bound, NULL);
	OBJECT(msb)->flags |= AG_OBJECT_BOUND_EVENTS;
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_MSpinbutton *msb = obj;
	AG_SizeReq rc, rYinc, rYdec;

	r->w = WIDGET(msb)->paddingLeft + WIDGET(msb)->paddingRight;
	r->h = WIDGET(msb)->paddingTop + WIDGET(msb)->paddingBottom;

	AG_WidgetSizeReq(msb->input, &rc);
	r->w += rc.w;
	r->h += rc.h;
	AG_WidgetSizeReq(msb->btn[AG_MSPINBUTTON_LEFT], &rc);
	r->w += rc.w;
	AG_WidgetSizeReq(msb->btn[AG_MSPINBUTTON_RIGHT], &rc);
	r->w += rc.w;
	AG_WidgetSizeReq(msb->btn[AG_MSPINBUTTON_DOWN], &rYinc);
	AG_WidgetSizeReq(msb->btn[AG_MSPINBUTTON_UP], &rYdec);
	r->w += MAX(rYinc.w, rYdec.w);
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_MSpinbutton *msb = obj;
	const int wBtn = (a->h >> 1);
	const int wBtn_2 = (wBtn >> 1);
	const int wBtn3 = (wBtn * 3);
	int x = 0, y = 0;
	AG_SizeAlloc ac;

	if (a->w < wBtn3 + 4)
		return (-1);

	ac.x = x;                                          /* Input textbox */
	ac.y = y;
	ac.w = a->w - 4 - wBtn3;
	ac.h = a->h;
	AG_WidgetSizeAlloc(msb->input, &ac);
	x += ac.w + 2;

	ac.w = wBtn;                                  /* Button arrangement */
	ac.h = wBtn;
	ac.x = x;
	ac.y = y + wBtn_2;
	AG_WidgetSizeAlloc(msb->btn[AG_MSPINBUTTON_LEFT], &ac);
	ac.x = x + (wBtn << 1);
	ac.y = y + wBtn_2;
	AG_WidgetSizeAlloc(msb->btn[AG_MSPINBUTTON_RIGHT], &ac);
	ac.x = x + wBtn;
	ac.y = y;
	AG_WidgetSizeAlloc(msb->btn[AG_MSPINBUTTON_UP], &ac);
	ac.x = x + wBtn;
	ac.y = y + wBtn;
	AG_WidgetSizeAlloc(msb->btn[AG_MSPINBUTTON_DOWN], &ac);

	return (0);
}

static void
Draw(void *obj)
{
	AG_MSpinbutton *msb = obj;
	AG_Variable *xvalueb, *yvalueb;
	void *xvalue, *yvalue;

	AG_WidgetDraw(msb->input);
	AG_WidgetDraw(msb->btn[AG_MSPINBUTTON_LEFT]);
	AG_WidgetDraw(msb->btn[AG_MSPINBUTTON_UP]);
	AG_WidgetDraw(msb->btn[AG_MSPINBUTTON_RIGHT]);
	AG_WidgetDraw(msb->btn[AG_MSPINBUTTON_DOWN]);

	if (AG_WidgetIsFocused(msb->input))
		return;

	xvalueb = AG_GetVariable(msb, "xvalue", &xvalue);
	yvalueb = AG_GetVariable(msb, "yvalue", &yvalue);
	switch (AG_VARIABLE_TYPE(xvalueb)) {
	case AG_VARIABLE_INT:
		Snprintf(msb->inTxt, sizeof(msb->inTxt), "%d%s%d",
		    *(int *)xvalue, msb->sep, *(int *)yvalue);
		break;
	case AG_VARIABLE_UINT:
		Snprintf(msb->inTxt, sizeof(msb->inTxt), "%u%s%u",
		    *(Uint *)xvalue, msb->sep, *(Uint *)yvalue);
		break;
	case AG_VARIABLE_UINT8:
		Snprintf(msb->inTxt, sizeof(msb->inTxt), "%u%s%u",
		    *(Uint8 *)xvalue, msb->sep, *(Uint8 *)yvalue);
		break;
	case AG_VARIABLE_SINT8:
		Snprintf(msb->inTxt, sizeof(msb->inTxt), "%d%s%d",
		    *(Sint8 *)xvalue, msb->sep, *(Sint8 *)yvalue);
		break;
	case AG_VARIABLE_UINT16:
		Snprintf(msb->inTxt, sizeof(msb->inTxt), "%u%s%u",
		    *(Uint16 *)xvalue, msb->sep, *(Uint16 *)yvalue);
		break;
	case AG_VARIABLE_SINT16:
		Snprintf(msb->inTxt, sizeof(msb->inTxt), "%d%s%d",
		    *(Sint16 *)xvalue, msb->sep, *(Sint16 *)yvalue);
		break;
	case AG_VARIABLE_UINT32:
		Snprintf(msb->inTxt, sizeof(msb->inTxt), "%u%s%u",
		    (Uint)*(Uint32 *)xvalue, msb->sep, (Uint)*(Uint32 *)yvalue);
		break;
	case AG_VARIABLE_SINT32:
		Snprintf(msb->inTxt, sizeof(msb->inTxt), "%d%s%d",
		    (int)*(Sint32 *)xvalue, msb->sep, (int)*(Sint32 *)yvalue);
		break;
	default:
		break;
	}
	AG_UnlockVariable(xvalueb);
	AG_UnlockVariable(yvalueb);
}

void
AG_MSpinbuttonAddValue(AG_MSpinbutton *msb, const char *which, int inc)
{
	AG_Variable *valueb, *minb, *maxb;
	void *value;
	int *min, *max;

	AG_OBJECT_ISA(msb, "AG_Widget:AG_MSpinbutton:*");
	AG_ObjectLock(msb);

	valueb = AG_GetVariable(msb, which, &value);
	minb = AG_GetVariable(msb, "min", (void *)&min);
	maxb = AG_GetVariable(msb, "max", (void *)&max);

	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_INT:
		if (*(int *)value+inc >= *min &&
		    *(int *)value+inc <= *max)
			*(int *)value += inc;
		break;
	case AG_VARIABLE_UINT:
		if (*(Uint *)value+inc >= *min &&
		    *(Uint *)value+inc <= *max)
			*(Uint *)value += inc;
		break;
	case AG_VARIABLE_UINT8:
		if (*(Uint8 *)value+inc >= *min &&
		    *(Uint8 *)value+inc <= *max)
			*(Uint8 *)value += inc;
		break;
	case AG_VARIABLE_SINT8:
		if (*(Sint8 *)value+inc >= *min &&
		    *(Sint8 *)value+inc <= *max)
			*(Sint8 *)value += inc;
		break;
	case AG_VARIABLE_UINT16:
		if (*(Uint16 *)value+inc >= *min &&
		    *(Uint16 *)value+inc <= *max)
			*(Uint16 *)value += inc;
		break;
	case AG_VARIABLE_SINT16:
		if (*(Sint16 *)value+inc >= *min &&
		    *(Sint16 *)value+inc <= *max)
			*(Sint16 *)value += inc;
		break;
	case AG_VARIABLE_UINT32:
		if (*(Uint32 *)value+inc >= *min &&
		    *(Uint32 *)value+inc <= *max)
			*(Uint32 *)value += inc;
		break;
	case AG_VARIABLE_SINT32:
		if (*(Sint32 *)value+inc >= *min &&
		    *(Sint32 *)value+inc <= *max)
			*(Sint32 *)value += inc;
		break;
	default:
		break;
	}

	AG_PostEvent(msb, "mspinbutton-changed", "%s", which);

	AG_UnlockVariable(maxb);
	AG_UnlockVariable(minb);
	AG_UnlockVariable(valueb);

	AG_Redraw(msb);
	AG_ObjectUnlock(msb);
}

void
AG_MSpinbuttonSetValue(AG_MSpinbutton *msb, const char *which, ...)
{
	AG_Variable *valueb, *minb, *maxb;
	void *value;
	int *min, *max;
	va_list ap;

	AG_OBJECT_ISA(msb, "AG_Widget:AG_MSpinbutton:*");
	AG_ObjectLock(msb);

	valueb = AG_GetVariable(msb, which, &value);
	minb = AG_GetVariable(msb, "min", (void *)&min);
	maxb = AG_GetVariable(msb, "max", (void *)&max);

	va_start(ap, which);
	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_INT:
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
	case AG_VARIABLE_UINT:
		{
			Uint i = va_arg(ap, unsigned int);

			if (i < (Uint)*min) {
				*(Uint *)value = *min;
			} else if (i > (Uint)*max) {
				*(Uint *)value = *max;
			} else {
				*(Uint *)value = i;
			}
		}
		break;
	case AG_VARIABLE_UINT8:
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
	case AG_VARIABLE_SINT8:
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
	case AG_VARIABLE_UINT16:
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
	case AG_VARIABLE_SINT16:
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
	case AG_VARIABLE_UINT32:
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
	case AG_VARIABLE_SINT32:
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

	AG_PostEvent(msb, "mspinbutton-changed", "%s", which);

	AG_UnlockVariable(valueb);
	AG_UnlockVariable(minb);
	AG_UnlockVariable(maxb);

	AG_Redraw(msb);
	AG_ObjectUnlock(msb);
}

void
AG_MSpinbuttonSetMin(AG_MSpinbutton *msb, int nmin)
{
	AG_Variable *minb;
	int *min;

	AG_OBJECT_ISA(msb, "AG_Widget:AG_MSpinbutton:*");
	AG_ObjectLock(msb);

	minb = AG_GetVariable(msb, "min", (void *)&min);
	*min = nmin;
	AG_UnlockVariable(minb);

	AG_ObjectUnlock(msb);
}

void
AG_MSpinbuttonSetMax(AG_MSpinbutton *msb, int nmax)
{
	AG_Variable *maxb;
	int *max;

	AG_OBJECT_ISA(msb, "AG_Widget:AG_MSpinbutton:*");
	AG_ObjectLock(msb);

	maxb = AG_GetVariable(msb, "max", (void *)&max);
	*max = nmax;
	AG_UnlockVariable(maxb);

	AG_ObjectUnlock(msb);
}

void
AG_MSpinbuttonSetRange(AG_MSpinbutton *msb, int nmin, int nmax)
{
	AG_Variable *minb, *maxb;
	int *min, *max;

	AG_OBJECT_ISA(msb, "AG_Widget:AG_MSpinbutton:*");
	AG_ObjectLock(msb);

	minb = AG_GetVariable(msb, "min", (void *)&min);
	maxb = AG_GetVariable(msb, "max", (void *)&max);
	*min = nmin;
	*max = nmax;
	AG_UnlockVariable(minb);
	AG_UnlockVariable(maxb);

	AG_ObjectUnlock(msb);
}

void
AG_MSpinbuttonSetIncrement(AG_MSpinbutton *msb, int inc)
{
	AG_OBJECT_ISA(msb, "AG_Widget:AG_MSpinbutton:*");
	msb->inc = inc;
}

void
AG_MSpinbuttonSetWriteable(AG_MSpinbutton *msb, int writeable)
{
	AG_OBJECT_ISA(msb, "AG_Widget:AG_MSpinbutton:*");
	AG_ObjectLock(msb);

	msb->writeable = writeable;

	if (writeable) {
		AG_WidgetEnable(msb->btn[AG_MSPINBUTTON_RIGHT]);
		AG_WidgetEnable(msb->btn[AG_MSPINBUTTON_LEFT]);
		AG_WidgetEnable(msb->btn[AG_MSPINBUTTON_DOWN]);
		AG_WidgetEnable(msb->btn[AG_MSPINBUTTON_UP]);
		AG_WidgetEnable(msb->input);
	} else {
		AG_WidgetDisable(msb->btn[AG_MSPINBUTTON_RIGHT]);
		AG_WidgetDisable(msb->btn[AG_MSPINBUTTON_LEFT]);
		AG_WidgetDisable(msb->btn[AG_MSPINBUTTON_DOWN]);
		AG_WidgetDisable(msb->btn[AG_MSPINBUTTON_UP]);
		AG_WidgetDisable(msb->input);
	}

	AG_Redraw(msb);
	AG_ObjectUnlock(msb);
}

AG_WidgetClass agMSpinbuttonClass = {
	{
		"Agar(Widget:MSpinbutton)",
		sizeof(AG_MSpinbutton),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

#endif /* AG_WIDGETS */
