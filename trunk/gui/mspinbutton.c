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

#include <agar/core/core.h>
#include <agar/gui/mspinbutton.h>
#include <agar/gui/window.h>

#include <stdarg.h>
#include <string.h>

AG_MSpinbutton *
AG_MSpinbuttonNew(void *parent, Uint flags, const char *sep, const char *label)
{
	AG_MSpinbutton *sbu;

	sbu = Malloc(sizeof(AG_MSpinbutton));
	AG_ObjectInit(sbu, &agMSpinbuttonClass);
	sbu->sep = sep;

	if (!(flags & AG_MSPINBUTTON_NOHFILL))	{ AG_ExpandHoriz(sbu); }
	if (  flags & AG_MSPINBUTTON_VFILL)	{ AG_ExpandVert(sbu); }

	if (label != NULL) {
		AG_TextboxSetLabelS(sbu->input, label);
	}
	AG_ObjectAttach(parent, sbu);
	return (sbu);
}

static void
Bound(AG_Event *event)
{
	AG_MSpinbutton *sbu = AG_SELF();
	AG_Variable *binding = AG_PTR(1);

	if (strcmp(binding->name, "xvalue") == 0 ||
	    strcmp(binding->name, "yvalue") == 0) {
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
		}
	}
	AG_Redraw(sbu);
}

static void
KeyDown(AG_Event *event)
{
	AG_MSpinbutton *sbu = AG_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case AG_KEY_LEFT:
		AG_MSpinbuttonAddValue(sbu, "xvalue", -sbu->inc);
		break;
	case AG_KEY_RIGHT:
		AG_MSpinbuttonAddValue(sbu, "xvalue", sbu->inc);
		break;
	case AG_KEY_UP:
		AG_MSpinbuttonAddValue(sbu, "yvalue", -sbu->inc);
		break;
	case AG_KEY_DOWN:
		AG_MSpinbuttonAddValue(sbu, "yvalue", sbu->inc);
		break;
	}
}

static void
TextReturn(AG_Event *event)
{
	AG_MSpinbutton *sbu = AG_PTR(1);
	char inTxt[64];
	char *tp = &inTxt[0], *s;

	AG_ObjectLock(sbu);
	Strlcpy(inTxt, sbu->inTxt, sizeof(inTxt));
	if ((s = AG_Strsep(&tp, sbu->sep)) != NULL) {
		AG_MSpinbuttonSetValue(sbu, "xvalue", atoi(s));
	}
	if ((s = AG_Strsep(&tp, sbu->sep)) != NULL) {
		AG_MSpinbuttonSetValue(sbu, "yvalue", atoi(s));
	}
	AG_PostEvent(NULL, sbu, "mspinbutton-return", NULL);
	AG_WidgetUnfocus(sbu->input);
	AG_ObjectUnlock(sbu);
	AG_Redraw(sbu);
}

static void
TextChanged(AG_Event *event)
{
	AG_MSpinbutton *sbu = AG_PTR(1);
	char inTxt[64];
	char *tp = &inTxt[0], *s;
	
	AG_ObjectLock(sbu);
	Strlcpy(inTxt, sbu->inTxt, sizeof(inTxt));
	if ((s = AG_Strsep(&tp, sbu->sep)) != NULL) {
		AG_MSpinbuttonSetValue(sbu, "xvalue", atoi(s));
	}
	if ((s = AG_Strsep(&tp, sbu->sep)) != NULL) {
		AG_MSpinbuttonSetValue(sbu, "yvalue", atoi(s));
	}
	AG_PostEvent(NULL, sbu, "mspinbutton-changed", NULL);
	AG_ObjectUnlock(sbu);
	AG_Redraw(sbu);
}

static void
DecrementY(AG_Event *event)
{
	AG_MSpinbutton *sbu = AG_PTR(1);

	AG_ObjectLock(sbu);
	AG_MSpinbuttonAddValue(sbu, "yvalue", -sbu->inc);
	AG_ObjectUnlock(sbu);
}

static void
IncrementY(AG_Event *event)
{
	AG_MSpinbutton *sbu = AG_PTR(1);
	
	AG_ObjectLock(sbu);
	AG_MSpinbuttonAddValue(sbu, "yvalue", sbu->inc);
	AG_ObjectUnlock(sbu);
}

static void
DecrementX(AG_Event *event)
{
	AG_MSpinbutton *sbu = AG_PTR(1);
	
	AG_ObjectLock(sbu);
	AG_MSpinbuttonAddValue(sbu, "xvalue", -sbu->inc);
	AG_ObjectUnlock(sbu);
}

static void
IncrementX(AG_Event *event)
{
	AG_MSpinbutton *sbu = AG_PTR(1);
	
	AG_ObjectLock(sbu);
	AG_MSpinbuttonAddValue(sbu, "xvalue", sbu->inc);
	AG_ObjectUnlock(sbu);
}

static void
Init(void *obj)
{
	AG_MSpinbutton *sbu = obj;
	AG_Button *b[4];
	int i;

	AG_BindInt(sbu, "xvalue", &sbu->xvalue);
	AG_BindInt(sbu, "yvalue", &sbu->yvalue);
	AG_BindInt(sbu, "min", &sbu->min);
	AG_BindInt(sbu, "max", &sbu->max);

	sbu->xvalue = 0;
	sbu->yvalue = 0;
	sbu->min = 0;
	sbu->max = 0;
	sbu->inc = 1;
	sbu->writeable = 0;
	sbu->sep = ",";
	sbu->inTxt[0] = '\0';
	
	sbu->input = AG_TextboxNewS(sbu, 0, NULL);
	AG_TextboxBindASCII(sbu->input, sbu->inTxt, sizeof(sbu->inTxt));
	AG_SetEvent(sbu->input, "textbox-return", TextReturn, "%p", sbu);
	AG_SetEvent(sbu->input, "textbox-postchg", TextChanged, "%p", sbu);
	AG_TextboxSizeHint(sbu->input, "88888");

	sbu->xdecbu = b[0] = AG_ButtonNewS(sbu, AG_BUTTON_REPEAT, _("-"));
	sbu->xincbu = b[1] = AG_ButtonNewS(sbu, AG_BUTTON_REPEAT, _("+"));
	sbu->ydecbu = b[2] = AG_ButtonNewS(sbu, AG_BUTTON_REPEAT, _("-"));
	sbu->yincbu = b[3] = AG_ButtonNewS(sbu, AG_BUTTON_REPEAT, _("+"));
	AG_SetEvent(sbu->xdecbu, "button-pushed", DecrementX, "%p", sbu);
	AG_SetEvent(sbu->xincbu, "button-pushed", IncrementX, "%p", sbu);
	AG_SetEvent(sbu->ydecbu, "button-pushed", DecrementY, "%p", sbu);
	AG_SetEvent(sbu->yincbu, "button-pushed", IncrementY, "%p", sbu);
	for (i = 0; i < 4; i++) {
		AG_ButtonSetPadding(b[i], 0,0,0,0);
		AG_LabelSetPadding(b[i]->lbl, 0,0,0,0);
		AG_WidgetSetFocusable(b[i], 0);
	}

	AG_SetEvent(sbu, "bound", Bound, NULL);
	AG_SetEvent(sbu, "key-down", KeyDown, NULL);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_MSpinbutton *fsu = obj;
	AG_SizeReq rChld, rYinc, rYdec;

	AG_WidgetSizeReq(fsu->input, &rChld);
	r->w = rChld.w;
	r->h = rChld.h;
	AG_WidgetSizeReq(fsu->xdecbu, &rChld);
	r->w += rChld.w;
	AG_WidgetSizeReq(fsu->xincbu, &rChld);
	r->w += rChld.w;
	AG_WidgetSizeReq(fsu->yincbu, &rYinc);
	AG_WidgetSizeReq(fsu->ydecbu, &rYdec);
	r->w += MAX(rYinc.w,rYdec.w);
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_MSpinbutton *fsu = obj;
	int szBtn = a->h/2;
	int x = 0, y = 0;
	AG_SizeAlloc aChld;

	if (a->w < szBtn*3 + 4)
		return (-1);

	/* Input textbox */
	aChld.x = x;
	aChld.y = y;
	aChld.w = a->w - 4 - szBtn*3;
	aChld.h = a->h;
	AG_WidgetSizeAlloc(fsu->input, &aChld);
	x += aChld.w + 2;

	/* Buttons */
	aChld.w = szBtn;
	aChld.h = szBtn;
	aChld.x = x;
	aChld.y = y + szBtn/2;
	AG_WidgetSizeAlloc(fsu->xdecbu, &aChld);
	aChld.x = x + szBtn*2;
	aChld.y = y + szBtn/2;
	AG_WidgetSizeAlloc(fsu->xincbu, &aChld);
	aChld.x = x + szBtn;
	aChld.y = y;
	AG_WidgetSizeAlloc(fsu->ydecbu, &aChld);
	aChld.x = x + szBtn;
	aChld.y = y + szBtn;
	AG_WidgetSizeAlloc(fsu->yincbu, &aChld);

	return (0);
}

static void
Draw(void *obj)
{
	AG_MSpinbutton *sbu = obj;
	AG_Variable *xvalueb, *yvalueb;
	void *xvalue, *yvalue;

	AG_WidgetDraw(sbu->input);
	AG_WidgetDraw(sbu->xdecbu);
	AG_WidgetDraw(sbu->ydecbu);
	AG_WidgetDraw(sbu->xincbu);
	AG_WidgetDraw(sbu->yincbu);

	if (AG_WidgetIsFocused(sbu->input))
		return;

	xvalueb = AG_GetVariable(sbu, "xvalue", &xvalue);
	yvalueb = AG_GetVariable(sbu, "yvalue", &yvalue);
	switch (AG_VARIABLE_TYPE(xvalueb)) {
	case AG_VARIABLE_INT:
		Snprintf(sbu->inTxt, sizeof(sbu->inTxt), "%d%s%d",
		    *(int *)xvalue, sbu->sep, *(int *)yvalue);
		break;
	case AG_VARIABLE_UINT:
		Snprintf(sbu->inTxt, sizeof(sbu->inTxt), "%u%s%u",
		    *(Uint *)xvalue, sbu->sep, *(Uint *)yvalue);
		break;
	case AG_VARIABLE_UINT8:
		Snprintf(sbu->inTxt, sizeof(sbu->inTxt), "%u%s%u",
		    *(Uint8 *)xvalue, sbu->sep, *(Uint8 *)yvalue);
		break;
	case AG_VARIABLE_SINT8:
		Snprintf(sbu->inTxt, sizeof(sbu->inTxt), "%d%s%d",
		    *(Sint8 *)xvalue, sbu->sep, *(Sint8 *)yvalue);
		break;
	case AG_VARIABLE_UINT16:
		Snprintf(sbu->inTxt, sizeof(sbu->inTxt), "%u%s%u",
		    *(Uint16 *)xvalue, sbu->sep, *(Uint16 *)yvalue);
		break;
	case AG_VARIABLE_SINT16:
		Snprintf(sbu->inTxt, sizeof(sbu->inTxt), "%d%s%d",
		    *(Sint16 *)xvalue, sbu->sep, *(Sint16 *)yvalue);
		break;
	case AG_VARIABLE_UINT32:
		Snprintf(sbu->inTxt, sizeof(sbu->inTxt), "%u%s%u",
		    (Uint)*(Uint32 *)xvalue, sbu->sep, (Uint)*(Uint32 *)yvalue);
		break;
	case AG_VARIABLE_SINT32:
		Snprintf(sbu->inTxt, sizeof(sbu->inTxt), "%d%s%d",
		    (int)*(Sint32 *)xvalue, sbu->sep, (int)*(Sint32 *)yvalue);
		break;
	}
	AG_UnlockVariable(xvalueb);
	AG_UnlockVariable(yvalueb);
}

void
AG_MSpinbuttonAddValue(AG_MSpinbutton *sbu, const char *which, int inc)
{
	AG_Variable *valueb, *minb, *maxb;
	void *value;
	int *min, *max;

	AG_ObjectLock(sbu);
	valueb = AG_GetVariable(sbu, which, &value);
	minb = AG_GetVariable(sbu, "min", &min);
	maxb = AG_GetVariable(sbu, "max", &max);

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
	}

	AG_PostEvent(NULL, sbu, "mspinbutton-changed", "%s", which);

	AG_UnlockVariable(maxb);
	AG_UnlockVariable(minb);
	AG_UnlockVariable(valueb);
	AG_ObjectUnlock(sbu);
	AG_Redraw(sbu);
}

void
AG_MSpinbuttonSetValue(AG_MSpinbutton *sbu, const char *which, ...)
{
	AG_Variable *valueb, *minb, *maxb;
	void *value;
	int *min, *max;
	va_list ap;

	AG_ObjectLock(sbu);
	valueb = AG_GetVariable(sbu, which, &value);
	minb = AG_GetVariable(sbu, "min", &min);
	maxb = AG_GetVariable(sbu, "max", &max);

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
	}
	va_end(ap);

	AG_PostEvent(NULL, sbu, "mspinbutton-changed", "%s", which);

	AG_UnlockVariable(valueb);
	AG_UnlockVariable(minb);
	AG_UnlockVariable(maxb);
	AG_ObjectUnlock(sbu);
	AG_Redraw(sbu);
}

void
AG_MSpinbuttonSetMin(AG_MSpinbutton *sbu, int nmin)
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
AG_MSpinbuttonSetMax(AG_MSpinbutton *sbu, int nmax)
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
AG_MSpinbuttonSetRange(AG_MSpinbutton *sbu, int nmin, int nmax)
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
AG_MSpinbuttonSetIncrement(AG_MSpinbutton *sbu, int inc)
{
	AG_ObjectLock(sbu);
	sbu->inc = inc;
	AG_ObjectUnlock(sbu);
}

void
AG_MSpinbuttonSetWriteable(AG_MSpinbutton *sbu, int writeable)
{
	AG_ObjectLock(sbu);
	sbu->writeable = writeable;
	if (writeable) {
		AG_WidgetEnable(sbu->xincbu);
		AG_WidgetEnable(sbu->xdecbu);
		AG_WidgetEnable(sbu->yincbu);
		AG_WidgetEnable(sbu->ydecbu);
		AG_WidgetEnable(sbu->input);
	} else {
		AG_WidgetDisable(sbu->xincbu);
		AG_WidgetDisable(sbu->xdecbu);
		AG_WidgetDisable(sbu->yincbu);
		AG_WidgetDisable(sbu->ydecbu);
		AG_WidgetDisable(sbu->input);
	}
	AG_ObjectUnlock(sbu);
	AG_Redraw(sbu);
}

AG_WidgetClass agMSpinbuttonClass = {
	{
		"Agar(Widget:MSpinbutton)",
		sizeof(AG_MSpinbutton),
		{ 0,0 },
		Init,
		NULL,		/* free */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
