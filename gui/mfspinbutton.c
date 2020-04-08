/*
 * Copyright (c) 2004-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Floating-point spinbutton group with two directions.
 */

#include <agar/core/core.h>
#if defined(AG_WIDGETS)

#include <agar/gui/mfspinbutton.h>
#include <agar/gui/window.h>

#include <string.h>

AG_MFSpinbutton *
AG_MFSpinbuttonNew(void *parent, Uint flags, const char *sep, const char *label)
{
	AG_MFSpinbutton *msb;

	msb = Malloc(sizeof(AG_MFSpinbutton));
	AG_ObjectInit(msb, &agMFSpinbuttonClass);
	msb->sep = sep;
	msb->flags |= flags;
	if (!(flags & AG_MFSPINBUTTON_NOHFILL))	{ WIDGET(msb)->flags |= AG_WIDGET_HFILL; }
	if (  flags & AG_MFSPINBUTTON_VFILL)	{ WIDGET(msb)->flags |= AG_WIDGET_VFILL; }

	if (label != NULL)
		AG_TextboxSetLabelS(msb->input, label);

	AG_ObjectAttach(parent, msb);
	return (msb);
}

static Uint32
UpdateTimeout(AG_Timer *to, AG_Event *event)
{
	AG_MFSpinbutton *msb = AG_MFSPINBUTTON_SELF();
	
	if (!AG_WidgetIsFocused(msb)) {
		AG_MFSpinbuttonUpdate(msb);
	}
	return (to->ival);
}

static void
OnShow(AG_Event *event)
{
	AG_MFSpinbutton *msb = AG_MFSPINBUTTON_SELF();
	AG_Variable *Vx, *Vy;

	if ((msb->flags & AG_MFSPINBUTTON_EXCL) == 0)
		AG_AddTimer(msb, &msb->updateTo, 250, UpdateTimeout, NULL);

	if ((Vx = AG_AccessVariable(msb, "xvalue")) == NULL) {
		msb->xvalue = 0.0;
		Vx = AG_BindDouble(msb, "xvalue", &msb->xvalue);
		AG_LockVariable(Vx);
	}
	if ((Vy = AG_AccessVariable(msb, "yvalue")) == NULL) {
		msb->yvalue = 0.0;
		Vy = AG_BindDouble(msb, "yvalue", &msb->yvalue);
		AG_LockVariable(Vy);
	}
	if (Vx->type != Vy->type) {
		AG_FatalError("[xy]value type mismatch");
	}
	switch (Vx->type) {
	case AG_VARIABLE_P_FLOAT:
		if (!AG_Defined(msb, "min")) {
			msb->minFlt = -AG_FLT_MAX+1;
			AG_BindFloat(msb, "min", &msb->minFlt);
		}
		if (!AG_Defined(msb, "max")) {
			msb->maxFlt = +AG_FLT_MAX-1;
			AG_BindFloat(msb, "max", &msb->maxFlt);
		}
		break;
	case AG_VARIABLE_P_DOUBLE:
		if (!AG_Defined(msb, "min")) {
			msb->min = -AG_DBL_MAX+1;
			AG_BindDouble(msb, "min", &msb->min);
		}
		if (!AG_Defined(msb, "max")) {
			msb->max = +AG_DBL_MAX-1;
			AG_BindDouble(msb, "max", &msb->max);
		}
		break;
	default:
		break;
	}
	AG_MFSpinbuttonUpdate(msb);

	AG_UnlockVariable(Vy);
	AG_UnlockVariable(Vx);
}

/* Update the input text from the binding values. */
void
AG_MFSpinbuttonUpdate(AG_MFSpinbutton *msb)
{
	char s[128], buf[64];
	AG_Variable *V;
	void *value;

	V = AG_GetVariable(msb, "xvalue", &value);
	switch (AG_VARIABLE_TYPE(V)) {
	case AG_VARIABLE_DOUBLE:
		Snprintf(buf, sizeof(buf), msb->format, *(double *)value);
		break;
	case AG_VARIABLE_FLOAT:
		Snprintf(buf, sizeof(buf), msb->format, *(float *)value);
		break;
	default:
		break;
	}
	AG_UnlockVariable(V);

	Strlcpy(s, buf, sizeof(s));                                /* "<X>" */
	
	V = AG_GetVariable(msb, "yvalue", &value);
	switch (AG_VARIABLE_TYPE(V)) {
	case AG_VARIABLE_DOUBLE:
		Snprintf(buf, sizeof(buf), msb->format, *(double *)value);
		break;
	case AG_VARIABLE_FLOAT:
		Snprintf(buf, sizeof(buf), msb->format, *(float *)value);
		break;
	default:
		break;
	}

	Strlcat(s, msb->sep, sizeof(s));                   /* "<X><Sep><Y>" */
	Strlcat(s, buf, sizeof(s));

	if (strcmp(s, msb->inTxt) != 0) {
		AG_TextboxSetString(msb->input, s);
	}
	AG_UnlockVariable(V);
}

#if 0
static void
KeyDown(AG_Event *event)
{
	AG_MFSpinbutton *msb = AG_MFSPINBUTTON_PTR(1);
	int keysym = AG_INT(2);

	switch (keysym) {
	case AG_KEY_LEFT:
		AG_MFSpinbuttonAddValue(msb, "xvalue", -msb->inc);
		break;
	case AG_KEY_RIGHT:
		AG_MFSpinbuttonAddValue(msb, "xvalue", msb->inc);
		break;
	case AG_KEY_UP:
		AG_MFSpinbuttonAddValue(msb, "yvalue", -msb->inc);
		break;
	case AG_KEY_DOWN:
		AG_MFSpinbuttonAddValue(msb, "yvalue", msb->inc);
		break;
	default:
		break;
	}
	AG_Redraw(msb);
}
#endif

static void
TextChanged(AG_Event *event)
{
	char buf[128];
	AG_MFSpinbutton *msb = AG_MFSPINBUTTON_PTR(1);
	int unfocus = AG_INT(2);
	char *tok = &buf[0], *s;

	AG_ObjectLock(msb);

	Strlcpy(buf, msb->inTxt, sizeof(buf));

	if ((s = AG_Strsep(&tok, msb->sep)) != NULL)
		AG_MFSpinbuttonSetValue(msb, "xvalue", strtod(s, NULL));
	if ((s = AG_Strsep(&tok, msb->sep)) != NULL)
		AG_MFSpinbuttonSetValue(msb, "yvalue", strtod(s, NULL));

	AG_PostEvent(msb, "mfspinbutton-return", NULL);
	if (unfocus) {
		AG_WidgetUnfocus(msb->input);
	}
	AG_MFSpinbuttonUpdate(msb);

	AG_ObjectUnlock(msb);
}

static void
Increment(AG_Event *event)
{
	AG_MFSpinbutton *msb = AG_MFSPINBUTTON_PTR(1);
	const char *which = AG_STRING(2);
	const int dir = AG_INT(3);

	AG_MFSpinbuttonAddValue(msb, which, dir*msb->inc);
}

static void
Init(void *obj)
{
	AG_MFSpinbutton *msb = obj;
	AG_Textbox *tb;
	const Uint btnFlags = AG_BUTTON_NO_FOCUS | AG_BUTTON_REPEAT;
	int i;

	WIDGET(msb)->flags |= AG_WIDGET_FOCUSABLE;

	msb->flags = 0;
	msb->writeable = 1;
	msb->inc = 1.0;
	msb->sep = ",";
	msb->inTxt[0] = '\0';
	Strlcpy(msb->format, "%.02f", sizeof(msb->format));

	tb = msb->input = AG_TextboxNewS(msb, AG_TEXTBOX_EXCL, NULL);
	AG_TextboxBindASCII(tb, msb->inTxt, sizeof(msb->inTxt));
	AG_TextboxSizeHint(tb, "88.88,88.88");
	AG_SetEvent(tb, "textbox-return", TextChanged, "%p,%i", msb, 1);
	AG_SetEvent(tb, "textbox-changed", TextChanged, "%p,%i", msb, 0);
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

	AG_InitTimer(&msb->updateTo, "update", 0);

	AG_AddEvent(msb, "widget-shown", OnShow, NULL);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_MFSpinbutton *msb = obj;
	AG_SizeReq rc, rDown, rUp;

	r->w = WIDGET(msb)->paddingLeft + WIDGET(msb)->paddingRight;
	r->h = WIDGET(msb)->paddingTop + WIDGET(msb)->paddingBottom;
	
	AG_WidgetSizeReq(msb->input, &rc);
	r->w += rc.w;
	r->h += rc.h;
	AG_WidgetSizeReq(msb->btn[AG_MFSPINBUTTON_LEFT], &rc);
	r->w += rc.w;
	AG_WidgetSizeReq(msb->btn[AG_MFSPINBUTTON_RIGHT], &rc);
	r->w += rc.w;
	AG_WidgetSizeReq(msb->btn[AG_MFSPINBUTTON_DOWN], &rDown);
	AG_WidgetSizeReq(msb->btn[AG_MFSPINBUTTON_UP], &rUp);
	r->w += MAX(rDown.w, rUp.w);
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_MFSpinbutton *msb = obj;
	AG_SizeAlloc ac;
	const int paddingLeft   = WIDGET(msb)->paddingLeft;
	const int paddingRight  = WIDGET(msb)->paddingRight;
	const int paddingTop    = WIDGET(msb)->paddingTop;
	const int paddingBottom = WIDGET(msb)->paddingBottom;
	const int spacingHoriz  = WIDGET(msb)->spacingHoriz;
	const int wBtn = ((a->h - paddingTop - paddingBottom) >> 1);
	const int wBtn_2 = (wBtn >> 1);
	const int wBtns = spacingHoriz + wBtn*3;
	int x = paddingLeft, y = paddingTop;

	if (a->w < paddingLeft + paddingRight + wBtns)
		return (-1);

	ac.x = x;                                          /* Input textbox */
	ac.y = y;
	ac.w = a->w - paddingLeft - paddingRight - wBtns;
	ac.h = a->h - paddingTop - paddingBottom;
	AG_WidgetSizeAlloc(msb->input, &ac);
	x += ac.w + spacingHoriz;

	ac.w = wBtn;                                         /* X-Decrement */
	ac.h = wBtn;
	ac.x = x;
	ac.y = y + wBtn_2;
	AG_WidgetSizeAlloc(msb->btn[AG_MFSPINBUTTON_LEFT], &ac);
	ac.x = x + (wBtn << 1);                              /* X-Increment */
	ac.y = y + wBtn_2;
	AG_WidgetSizeAlloc(msb->btn[AG_MFSPINBUTTON_RIGHT], &ac);
	ac.x = x + wBtn;                                     /* Y-Decrement */
	ac.y = y;
	AG_WidgetSizeAlloc(msb->btn[AG_MFSPINBUTTON_UP], &ac);
	ac.x = x + wBtn;                                     /* Y-Increment */
	ac.y = y + wBtn;
	AG_WidgetSizeAlloc(msb->btn[AG_MFSPINBUTTON_DOWN], &ac);
	return (0);
}

static void
Draw(void *obj)
{
	AG_MFSpinbutton *msb = obj;
	AG_Variable *xvalueb, *yvalueb;
	double *xvalue, *yvalue;

	AG_WidgetDraw(msb->input);
	AG_WidgetDraw(msb->btn[AG_MFSPINBUTTON_RIGHT]);
	AG_WidgetDraw(msb->btn[AG_MFSPINBUTTON_DOWN]);
	AG_WidgetDraw(msb->btn[AG_MFSPINBUTTON_LEFT]);
	AG_WidgetDraw(msb->btn[AG_MFSPINBUTTON_UP]);

	xvalueb = AG_GetVariable(msb, "xvalue", (void *)&xvalue);
	yvalueb = AG_GetVariable(msb, "yvalue", (void *)&yvalue);

	Snprintf(msb->inTxt, sizeof(msb->inTxt), msb->format, *xvalue, *yvalue);

	AG_UnlockVariable(xvalueb);
	AG_UnlockVariable(yvalueb);
}

void
AG_MFSpinbuttonAddValue(AG_MFSpinbutton *msb, const char *which, double inc)
{
	AG_Variable *valueb, *minb, *maxb;
	void *value;
	double *min, *max;

	AG_OBJECT_ISA(msb, "AG_Widget:AG_MFSpinbutton:*");
	AG_ObjectLock(msb);
	
	valueb = AG_GetVariable(msb, which, &value);
	minb = AG_GetVariable(msb, "min", (void *)&min);
	maxb = AG_GetVariable(msb, "max", (void *)&max);

	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_DOUBLE:
		*(double *)value = *(double *)value+inc < *min ? *min :
		                   *(double *)value+inc > *max ? *max :
				   *(double *)value+inc;
		break;
	case AG_VARIABLE_FLOAT:
		*(float *)value = *(float *)value+inc < *min ? *min :
		                  *(float *)value+inc > *max ? *max :
				  *(float *)value+inc;
		break;
	default:
		break;
	}
	AG_PostEvent(msb, "mfspinbutton-changed", "%s", which);

	AG_UnlockVariable(valueb);
	AG_UnlockVariable(minb);
	AG_UnlockVariable(maxb);
	
	AG_MFSpinbuttonUpdate(msb);
	AG_ObjectUnlock(msb);

	AG_Redraw(msb);
}

void
AG_MFSpinbuttonSetValue(AG_MFSpinbutton *msb, const char *which,
    double nvalue)
{
	AG_Variable *valueb, *minb, *maxb;
	void *value;
	double *min, *max;
	
	AG_OBJECT_ISA(msb, "AG_Widget:AG_MFSpinbutton:*");
	AG_ObjectLock(msb);

	valueb = AG_GetVariable(msb, which, &value);
	minb = AG_GetVariable(msb, "min", (void *)&min);
	maxb = AG_GetVariable(msb, "max", (void *)&max);

	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_DOUBLE:
		*(double *)value = nvalue < *min ? *min :
		                   nvalue > *max ? *max :
				   nvalue;
		break;
	case AG_VARIABLE_FLOAT:
		*(float *)value = nvalue < *min ? *min :
		                  nvalue > *max ? *max :
				  (float)nvalue;
		break;
	default:
		break;
	}
	AG_PostEvent(msb, "mfspinbutton-changed", "%s", which);

	AG_UnlockVariable(valueb);
	AG_UnlockVariable(minb);
	AG_UnlockVariable(maxb);
	
	AG_MFSpinbuttonUpdate(msb);
	AG_ObjectUnlock(msb);
}

void
AG_MFSpinbuttonSetMin(AG_MFSpinbutton *msb, double nmin)
{
	AG_Variable *minb;
	void *min;
	
	AG_OBJECT_ISA(msb, "AG_Widget:AG_MFSpinbutton:*");
	AG_ObjectLock(msb);

	minb = AG_GetVariable(msb, "min", (void *)&min);
	switch (AG_VARIABLE_TYPE(minb)) {
	case AG_VARIABLE_DOUBLE:
		*(double *)min = nmin;
		break;
	case AG_VARIABLE_FLOAT:
		*(float *)min = (float)nmin;
		break;
	default:
		break;
	}
	AG_UnlockVariable(minb);

	AG_ObjectUnlock(msb);
}

void
AG_MFSpinbuttonSetMax(AG_MFSpinbutton *msb, double nmax)
{
	AG_Variable *maxb;
	void *max;
	
	AG_OBJECT_ISA(msb, "AG_Widget:AG_MFSpinbutton:*");
	AG_ObjectLock(msb);

	maxb = AG_GetVariable(msb, "max", (void *)&max);
	switch (AG_VARIABLE_TYPE(maxb)) {
	case AG_VARIABLE_DOUBLE:
		*(double *)max = nmax;
		break;
	case AG_VARIABLE_FLOAT:
		*(float *)max = (float)nmax;
		break;
	default:
		break;
	}
	AG_UnlockVariable(maxb);

	AG_ObjectUnlock(msb);
}

void
AG_MFSpinbuttonSetIncrement(AG_MFSpinbutton *msb, double inc)
{
	AG_OBJECT_ISA(msb, "AG_Widget:AG_MFSpinbutton:*");
	msb->inc = inc;
}

void
AG_MFSpinbuttonSetPrecision(AG_MFSpinbutton *msb, const char *mode,
    int precision)
{
	AG_OBJECT_ISA(msb, "AG_Widget:AG_MFSpinbutton:*");
	AG_ObjectLock(msb);

	Snprintf(msb->format, sizeof(msb->format), "%%.%d%s", precision, mode);

	AG_MFSpinbuttonUpdate(msb);
	AG_ObjectUnlock(msb);
}

void
AG_MFSpinbuttonSetWriteable(AG_MFSpinbutton *msb, int writeable)
{
	AG_OBJECT_ISA(msb, "AG_Widget:AG_MFSpinbutton:*");
	AG_ObjectLock(msb);

	msb->writeable = writeable;

	if (writeable) {
		AG_WidgetEnable(msb->btn[AG_MFSPINBUTTON_RIGHT]);
		AG_WidgetEnable(msb->btn[AG_MFSPINBUTTON_LEFT]);
		AG_WidgetEnable(msb->btn[AG_MFSPINBUTTON_DOWN]);
		AG_WidgetEnable(msb->btn[AG_MFSPINBUTTON_UP]);
		AG_WidgetEnable(msb->input);
	} else {
		AG_WidgetDisable(msb->btn[AG_MFSPINBUTTON_RIGHT]);
		AG_WidgetDisable(msb->btn[AG_MFSPINBUTTON_LEFT]);
		AG_WidgetDisable(msb->btn[AG_MFSPINBUTTON_DOWN]);
		AG_WidgetDisable(msb->btn[AG_MFSPINBUTTON_UP]);
		AG_WidgetDisable(msb->input);
	}

	AG_Redraw(msb);
	AG_ObjectUnlock(msb);
}

void
AG_MFSpinbuttonSetRange(AG_MFSpinbutton *msb, double min, double max)
{
	AG_MFSpinbuttonSetMin(msb, min);
	AG_MFSpinbuttonSetMax(msb, max);
}

AG_WidgetClass agMFSpinbuttonClass = {
	{
		"Agar(Widget:MFSpinbutton)",
		sizeof(AG_MFSpinbutton),
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
