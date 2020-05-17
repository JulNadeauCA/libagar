/*
 * Copyright (c) 2007-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Numerical edition widget. It can connect to an integer or floating-point
 * value and allow the user to edit the number by either keyboard or mouse.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/numerical.h>
#include <agar/gui/primitive.h>

#ifdef __NetBSD__
#define _NETBSD_SOURCE
#endif

#include <string.h>

#include <agar/config/_mk_have_strtold.h>
#include <agar/config/_mk_have_strtoll.h>
#if defined(_MK_HAVE_STRTOLD_H) || defined(_MK_HAVE_STRTOLL_H)
# define _XOPEN_SOURCE 600
# include <stdlib.h>
#endif

static void UnitSelected(AG_Event *_Nonnull);

AG_Numerical *
AG_NumericalNew(void *parent, Uint flags, const char *unit, const char *fmt,
    ...)
{
	AG_Numerical *num;
	char *s;
	va_list ap;

	if (fmt != NULL) {
		va_start(ap, fmt);
		Vasprintf(&s, fmt, ap);
		va_end(ap);
		num = AG_NumericalNewS(parent, flags, unit, s);
		free(s);
	} else {
		num = AG_NumericalNewS(parent, flags, unit, NULL);
	}
	return (num);
}

AG_Numerical *
AG_NumericalNewS(void *parent, Uint flags, const char *unit, const char *label)
{
	AG_Numerical *num;

	num = Malloc(sizeof(AG_Numerical));
	AG_ObjectInit(num, &agNumericalClass);

	num->flags |= flags;
	if (flags & AG_NUMERICAL_HFILL) { WIDGET(num)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_NUMERICAL_VFILL) { WIDGET(num)->flags |= AG_WIDGET_VFILL; }

	if (label != NULL) {
		AG_TextboxSetLabelS(num->input, label);
	}
	if (unit != NULL) {
		num->units = AG_UComboNew(num, 0);
		AG_SetEvent(num->units, "ucombo-selected",
		    UnitSelected, "%p", num);
		AG_NumericalSetUnitSystem(num, unit);
		AG_WidgetSetFocusable(num->units, 0);
	}

	AG_ObjectAttach(parent, num);
	return (num);
}

AG_Numerical *
AG_NumericalNewDbl(void *parent, Uint flags, const char *unit, const char *label,
    double *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindDouble(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewDblR(void *parent, Uint flags, const char *unit, const char *label,
    double *v, double min, double max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindDouble(num, "value", v);
	AG_SetDouble(num, "min", min);
	AG_SetDouble(num, "max", max);
	return (num);
}

AG_Numerical *
AG_NumericalNewFlt(void *parent, Uint flags, const char *unit,
    const char *label, float *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindFloat(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewFltR(void *parent, Uint flags, const char *unit,
    const char *label, float *v, float min, float max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindFloat(num, "value", v);
	AG_SetFloat(num, "min", min);
	AG_SetFloat(num, "max", max);
	return (num);
}

AG_Numerical *
AG_NumericalNewInt(void *parent, Uint flags, const char *unit, const char *label,
    int *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindInt(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewIntR(void *parent, Uint flags, const char *unit, const char *label,
    int *v, int min, int max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindInt(num, "value", v);
	AG_SetInt(num, "min", min);
	AG_SetInt(num, "max", max);
	return (num);
}
AG_Numerical *
AG_NumericalNewUint(void *parent, Uint flags, const char *unit, const char *label,
    Uint *v)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindUint(num, "value", v);
	return (num);
}
AG_Numerical *
AG_NumericalNewUintR(void *parent, Uint flags, const char *unit, const char *label,
    Uint *v, Uint min, Uint max)
{
	AG_Numerical *num;
	num = AG_NumericalNewS(parent, flags, unit, label);
	AG_BindUint(num, "value", v);
	AG_SetUint(num, "min", min);
	AG_SetUint(num, "max", max);
	return (num);
}

static Uint32
UpdateTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Numerical *num = AG_NUMERICAL_SELF();
	
	if (!AG_WidgetIsFocused(num)) {
		AG_NumericalUpdate(num);
	}
	return (to->ival);
}

#undef SET_DEF
#define SET_DEF(fn,dmin,dmax,dinc) { 					\
	if (!AG_Defined(num, "min")) { fn(num, "min", dmin); }		\
	if (!AG_Defined(num, "max")) { fn(num, "max", dmax); }		\
	if (!AG_Defined(num, "inc")) { fn(num, "inc", dinc); }		\
}
static void
OnShow(AG_Event *_Nonnull event)
{
	AG_Numerical *num = AG_NUMERICAL_SELF();
	AG_Variable *V;

	if ((num->flags & AG_NUMERICAL_EXCL) == 0)
		AG_AddTimer(num, &num->toUpdate,
		            (num->flags & AG_NUMERICAL_SLOW) ? 2000 : 250,
		            UpdateTimeout, NULL);

	if ((V = AG_AccessVariable(num, "value")) == NULL) {
		if (num->flags & AG_NUMERICAL_INT) {
			V = AG_SetInt(num, "value", 0);
		} else {
			V = AG_SetDouble(num, "value", 0.0);
		}
		AG_LockVariable(V);
	}
	switch (AG_VARIABLE_TYPE(V)) {
	case AG_VARIABLE_FLOAT:  SET_DEF(AG_SetFloat, -AG_FLT_MAX, AG_FLT_MAX, 0.1f); break;
	case AG_VARIABLE_DOUBLE: SET_DEF(AG_SetDouble, -AG_DBL_MAX, AG_DBL_MAX, 0.1); break;
	case AG_VARIABLE_INT:    SET_DEF(AG_SetInt, AG_INT_MIN+1, AG_INT_MAX-1, 1); break;
	case AG_VARIABLE_UINT:   SET_DEF(AG_SetUint, 0U, AG_UINT_MAX-1, 1U); break;
	case AG_VARIABLE_UINT8:  SET_DEF(AG_SetUint8, 0U, 0xffU, 1U); break;
	case AG_VARIABLE_SINT8:  SET_DEF(AG_SetSint8, -0x7f, 0x7f, 1); break;
	case AG_VARIABLE_UINT16: SET_DEF(AG_SetUint16, 0U, 0xffffU, 1U); break;
	case AG_VARIABLE_SINT16: SET_DEF(AG_SetSint16, -0x7fff, 0x7fff, 1); break;
	case AG_VARIABLE_UINT32: SET_DEF(AG_SetUint32, 0UL, 0xffffffffUL, 1UL); break;
	case AG_VARIABLE_SINT32: SET_DEF(AG_SetSint32, -0x7fffffffL, 0x7fffffffL, 1L); break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64: SET_DEF(AG_SetUint64, 0ULL, 0xffffffffffffffffULL, 1ULL); break;
	case AG_VARIABLE_SINT64: SET_DEF(AG_SetSint64, -0x7fffffffffffffffLL, 0x7fffffffffffffffLL, 1LL); break;
#endif
	default: break;
	}
	switch (AG_VARIABLE_TYPE(V)) {
	case AG_VARIABLE_FLOAT:
	case AG_VARIABLE_DOUBLE:
		AG_TextboxSetFltOnly(num->input, 1);
		break;
	default:
		AG_TextboxSetIntOnly(num->input, 1);
		break;
	}
	AG_UnlockVariable(V);

	AG_NumericalUpdate(num);
}

static Uint32
KeyRepeat(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Numerical *num = AG_NUMERICAL_SELF();
	const AG_KeySym key = AG_INT(1);

	switch (key) {
	case AG_KEY_UP:
		AG_NumericalIncrement(num);
		break;
	case AG_KEY_DOWN:
		AG_NumericalDecrement(num);
		break;
	default:
		break;
	}
	return ((to->ival >> 1) >= 1) ? (to->ival >> 1) : 1;
}

static void
KeyDown(AG_Event *_Nonnull event)
{
	AG_Numerical *num = AG_NUMERICAL_PTR(1);
	const int keysym = AG_INT(2);

	switch (keysym) {
	case AG_KEY_UP:
		AG_NumericalIncrement(num);
		AG_AddTimer(num, &num->toInc, agKbdDelay, KeyRepeat, "%i", AG_KEY_UP);
		break;
	case AG_KEY_DOWN:
		AG_NumericalDecrement(num);
		AG_AddTimer(num, &num->toDec, agKbdDelay, KeyRepeat, "%i", AG_KEY_DOWN);
		break;
	default:
		break;
	}
}

static void
KeyUp(AG_Event *_Nonnull event)
{
	AG_Numerical *num = AG_NUMERICAL_PTR(1);
	const int keysym = AG_INT(2);

	switch (keysym) {
	case AG_KEY_UP:
		AG_DelTimer(num, &num->toInc);
		break;
	case AG_KEY_DOWN:
		AG_DelTimer(num, &num->toDec);
		break;
	default:
		break;
	}
}

static void
LostFocus(AG_Event *_Nonnull event)
{
	AG_Numerical *num = AG_NUMERICAL_PTR(1);

	AG_DelTimer(num, &num->toInc);
	AG_DelTimer(num, &num->toDec);
}

/*
 * Update the numerical value from the textbox.
 */
#define SET_NUM(TYPE,expr) {						\
    	TYPE val = (TYPE)(expr);					\
	*(TYPE *)value = val < *(TYPE *)min ? *(TYPE *)min :		\
                         val > *(TYPE *)max ? *(TYPE *)max : val;	\
}
static void
UpdateFromText(AG_Event *_Nonnull event)
{
	AG_Numerical *num = AG_NUMERICAL_PTR(1);
	const int unfocus = AG_INT(2);
	AG_Variable *valueb, *minb, *maxb;
	void *value, *min, *max;

	valueb = AG_GetVariable(num, "value", &value);
	minb = AG_GetVariable(num, "min", &min);
	maxb = AG_GetVariable(num, "max", &max);

	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_FLOAT:
		SET_NUM(float, AG_Unit2Base(strtod(num->inTxt,NULL),num->unit));
		break;
	case AG_VARIABLE_DOUBLE:
		SET_NUM(double, AG_Unit2Base(strtod(num->inTxt,NULL),num->unit));
		break;
	case AG_VARIABLE_INT:    SET_NUM(int, strtol(num->inTxt,NULL,10));	break;
	case AG_VARIABLE_UINT:   SET_NUM(Uint, strtoul(num->inTxt,NULL,10));	break;
	case AG_VARIABLE_UINT8:  SET_NUM(Uint8, strtoul(num->inTxt,NULL,10));	break;
	case AG_VARIABLE_SINT8:  SET_NUM(Sint8, strtol(num->inTxt,NULL,10));	break;
	case AG_VARIABLE_UINT16: SET_NUM(Uint16, strtoul(num->inTxt,NULL,10));	break;
	case AG_VARIABLE_SINT16: SET_NUM(Sint16, strtol(num->inTxt,NULL,10));	break;
	case AG_VARIABLE_UINT32: SET_NUM(Uint32, strtoul(num->inTxt,NULL,10));	break;
	case AG_VARIABLE_SINT32: SET_NUM(Sint32, strtol(num->inTxt,NULL,10));	break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:
# ifdef _MK_HAVE_STRTOLL
		SET_NUM(Uint64, strtoull(num->inTxt,NULL,10));
# else
		SET_NUM(Uint64, strtoul(num->inTxt,NULL,10));
# endif
		break;
	case AG_VARIABLE_SINT64:
# ifdef _MK_HAVE_STRTOLL
		SET_NUM(Sint64, strtoll(num->inTxt,NULL,10));
# else
		SET_NUM(Sint64, strtol(num->inTxt,NULL,10));
# endif
		break;
#endif
	default:
		break;
	}

	AG_PostEvent(num, "numerical-changed", NULL);

	AG_UnlockVariable(valueb);
	AG_UnlockVariable(minb);
	AG_UnlockVariable(maxb);

	if (unfocus) {
		AG_WidgetUnfocus(num->input);
	}
	AG_PostEvent(num, "numerical-return", NULL);
}
#undef SET_NUM

static void
ButtonIncrement(AG_Event *_Nonnull event)
{
	void (*pf[])(AG_Numerical *_Nonnull) = {
		AG_NumericalIncrement,
		AG_NumericalDecrement
	};
	AG_Numerical *num = AG_NUMERICAL_PTR(1);
	const int dir = AG_INT(2);
#ifdef AG_DEBUG
	if (dir < 0 || dir > 1) { AG_FatalError("dir"); }
#endif
	pf[dir](num);
}

static void
UpdateUnitSelector(AG_Numerical *_Nonnull num)
{
	AG_ButtonTextS(num->units->button, AG_UnitAbbr(num->unit));

	if (WIDGET(num)->window != NULL &&
	    WIDGET(num)->window->visible)
		AG_NumericalUpdate(num);
}

static void
UnitSelected(AG_Event *_Nonnull event)
{
	AG_Numerical *num = AG_NUMERICAL_PTR(1);
	const AG_TlistItem *ti = AG_TLIST_ITEM_PTR(2);

	AG_ObjectLock(num);

	num->unit = (const AG_Unit *)ti->p1;
	UpdateUnitSelector(num);

	AG_ObjectUnlock(num);
}

int
AG_NumericalSetUnitSystem(AG_Numerical *num, const char *unit_key)
{
	const AG_Unit *unit = NULL;
	const AG_Unit *ugroup = NULL;
	AG_Tlist *tl;
	int found = 0, i;
	int w, h, nUnits = 0;

	AG_OBJECT_ISA(num, "AG_Widget:AG_Numerical:*");
	AG_ObjectLock(num);

	for (i = 0; i < agnUnitGroups; i++) {
		ugroup = agUnitGroups[i];
		for (unit = &ugroup[0]; unit->key != NULL; unit++) {
			if (strcmp(unit->key, unit_key) == 0) {
				found++;
				break;
			}
		}
		if (found)
			break;
	}
	if (!found) {
		AG_SetError(_("No such unit: %s"), unit_key);
		goto fail;
	}
	num->unit = unit;
	UpdateUnitSelector(num);

	num->wUnitSel = 0;
	num->hUnitSel = 0;
	num->wPreUnit = 0;

	tl = num->units->list;
	AG_ObjectLock(tl);

	AG_TlistDeselectAll(tl);
	AG_TlistBegin(tl);

	for (unit = &ugroup[0]; unit->key != NULL; unit++) {
		AG_TlistItem *it;
	
		AG_TextSize(AG_UnitAbbr(unit), &w, &h);
		if (w > num->wUnitSel) { num->wUnitSel = w; }
		if (h > num->hUnitSel) { num->hUnitSel = h; }
		
		AG_TextSize(unit->name, &w, NULL);
		if (w > num->wPreUnit) { num->wPreUnit = w; }

		it = AG_TlistAddPtr(tl, NULL, _(unit->name), (void *)unit);
		if (unit == num->unit)
			it->selected++;

		nUnits++;
	}
	AG_TlistEnd(tl);
	AG_TlistSizeHintLargest(tl, 5);

	AG_ObjectUnlock(tl);

	if (num->wPreUnit > 0) { num->wPreUnit += 8; }       /* XXX */

	AG_UComboSizeHintPixels(num->units, num->wPreUnit,
	    nUnits<6 ? (nUnits + 1) : 6);
	
	WIDGET(num)->flags |= AG_WIDGET_UPDATE_WINDOW;

	AG_ObjectUnlock(num);
	return (0);
fail:
	AG_ObjectUnlock(num);
	return (-1);
}

/*
 * Update the input text from the binding value.
 * The Numerical object must be locked.
 */
void
AG_NumericalUpdate(AG_Numerical *num)
{
	char s[64];
	AG_Variable *valueb;
	void *value;

	AG_OBJECT_ISA(num, "AG_Widget:AG_Numerical:*");

	if (!AG_Defined(num,"value")) {
		return;
	}
	valueb = AG_GetVariable(num, "value", &value);
	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_DOUBLE:
		Snprintf(s, sizeof(s), num->format,
		    AG_Base2Unit(*(double *)value, num->unit));
		break;
	case AG_VARIABLE_FLOAT:
		Snprintf(s, sizeof(s), num->format,
		    AG_Base2Unit(*(float *)value, num->unit));
		break;
	case AG_VARIABLE_INT:	 StrlcpyInt(s, *(int *)value, sizeof(s));	break;
	case AG_VARIABLE_UINT:	 StrlcpyUint(s, *(Uint *)value, sizeof(s));	break;
	case AG_VARIABLE_UINT8:	 StrlcpyUint(s, *(Uint8 *)value, sizeof(s));	break;
	case AG_VARIABLE_SINT8:	 StrlcpyInt(s, *(Sint8 *)value, sizeof(s));	break;
	case AG_VARIABLE_UINT16: StrlcpyUint(s, *(Uint16 *)value, sizeof(s));	break;
	case AG_VARIABLE_SINT16: StrlcpyInt(s, *(Sint16 *)value, sizeof(s));	break;
	case AG_VARIABLE_UINT32: Snprintf(s, sizeof(s), "%lu", (unsigned long)*(Uint32 *)value); break;
	case AG_VARIABLE_SINT32: Snprintf(s, sizeof(s), "%ld", (long)*(Sint32 *)value);          break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64: Snprintf(s, sizeof(s), "%llu", (unsigned long long)*(Uint64 *)value); break;
	case AG_VARIABLE_SINT64: Snprintf(s, sizeof(s), "%lld", (long long)*(Sint64 *)value);          break;
#endif
	default: break;
	}
	if (strcmp(num->inTxt, s) != 0) {
		AG_TextboxSetString(num->input, s);
	}
	AG_UnlockVariable(valueb);
}

static void
Init(void *_Nonnull obj)
{
	AG_Numerical *num = obj;
	const Uint btnFlags = AG_BUTTON_REPEAT | AG_BUTTON_NO_FOCUS;

	WIDGET(num)->flags |= AG_WIDGET_FOCUSABLE;

	num->flags = 0;
	Strlcpy(num->format, "%.02f", sizeof(num->format));
	num->unit = AG_FindUnit("identity");
	num->units = NULL;
	num->inTxt[0] = '\0';

	/* Input textbox */
	num->input = AG_TextboxNewS(num, AG_TEXTBOX_EXCL, NULL);
	AG_TextboxBindASCII(num->input, num->inTxt, sizeof(num->inTxt));
	AG_TextboxSizeHint(num->input, "8888.88");
	AG_SetStyle(num->input, "padding", "inherit");

	num->incbu = AG_ButtonNewS(num, btnFlags, _("+"));     /* Increment */
	AG_SetStyle(num->incbu, "padding", "0");
	AG_SetStyle(num->incbu, "font-size", "80%");

	num->decbu = AG_ButtonNewS(num, btnFlags, _("-"));     /* Decrement */
	AG_SetStyle(num->decbu, "padding", "0");
	AG_SetStyle(num->decbu, "font-size", "80%");

	num->wUnitSel = 0;
	num->hUnitSel = 0;
	num->wPreUnit = 0;

	AG_AddEvent(num, "widget-shown", OnShow, NULL);

	AG_SetEvent(num->incbu, "button-pushed", ButtonIncrement, "%p,%i", num, 0);
	AG_SetEvent(num->decbu, "button-pushed", ButtonIncrement, "%p,%i", num, 1);

	AG_SetEvent(num->input, "textbox-return",  UpdateFromText, "%p,%i", num, 1);
	AG_SetEvent(num->input, "textbox-changed", UpdateFromText, "%p,%i", num, 0);

	AG_AddEvent(num->input->ed, "key-down", KeyDown, "%p", num);
	AG_AddEvent(num->input->ed, "key-up", KeyUp, "%p", num);
	AG_AddEvent(num->input->ed, "widget-lostfocus", LostFocus, "%p", num);

	AG_InitTimer(&num->toUpdate, "update", 0);
	AG_InitTimer(&num->toInc, "increment", 0);
	AG_InitTimer(&num->toDec, "decrement", 0);

	AG_WidgetForwardFocus(num, num->input);
}

void
AG_NumericalSizeHint(AG_Numerical *num, const char *text)
{
	AG_OBJECT_ISA(num, "AG_Widget:AG_Numerical:*");
	AG_ObjectLock(num);

	AG_TextboxSizeHint(num->input, text);

	AG_ObjectUnlock(num);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Numerical *num = obj;
	AG_SizeReq rInput, rInc, rDec;

	AG_WidgetSizeReq(num->input, &rInput);
	AG_WidgetSizeReq(num->incbu, &rInc);
	AG_WidgetSizeReq(num->decbu, &rDec);

	r->w = rInput.w + num->wUnitSel + WIDGET(num)->spacingHoriz +
	       MAX(rInc.w,rDec.w);

	r->h = rInput.h;
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_Numerical *num = obj;
	AG_SizeAlloc ac;
	const int wBtn = a->h >> 1;
	int wUnitSel, spacing;

	if (num->units) {
		spacing = WIDGET(num)->spacingHoriz;
		wUnitSel = MIN(num->wUnitSel, a->w - wBtn - spacing);
	} else {
		spacing = 0;
		wUnitSel = 0;
	}
	if (a->h < 4 || a->w < wBtn + wUnitSel + spacing)
		return (-1);

	ac.x = 0;                                          /* Input textbox */
	ac.y = 0;
	ac.w = a->w - wUnitSel - spacing - wBtn;
	ac.h = a->h;
	AG_WidgetSizeAlloc(num->input, &ac);
	ac.x += ac.w;

	if (num->units) {                                  /* Unit selector */
		ac.w = wUnitSel;
		ac.h = a->h;
		AG_WidgetSizeAlloc(num->units, &ac);
		ac.x += ac.w + spacing;
	}

	ac.w = wBtn;                         /* Increment/decrement buttons */
	ac.h = wBtn;
	AG_WidgetSizeAlloc(num->incbu, &ac);
	ac.y += ac.h;
	if ((ac.h << 1) < a->h) {
		ac.h++;
	}
	AG_WidgetSizeAlloc(num->decbu, &ac);
	return (0);
}

static void
Draw(void *_Nonnull obj)
{
	AG_Numerical *num = obj;

	AG_WidgetDraw(num->input);

	if (num->units)
		AG_WidgetDraw(num->units);

	AG_WidgetDraw(num->incbu);
	AG_WidgetDraw(num->decbu);
}

/*
 * Type-independent increment operation.
 */
#undef ADD_INT
#define ADD_INT(TYPE) {							\
	TYPE v = *(TYPE *)value;					\
	if ((v + *(TYPE *)inc) < *(TYPE *)min) { v = *(TYPE *)min; }	\
	else if ((v + *(TYPE *)inc) > *(TYPE *)max) { v = *(TYPE *)max; } \
	else { v += *(TYPE *)inc; }					\
	*(TYPE *)value = v;						\
}
#undef ADD_REAL
#define ADD_REAL(TYPE) {						\
	TYPE v = AG_Base2Unit((double)*(TYPE *)value, num->unit);	\
	if ((v + *(TYPE *)inc) < *(TYPE *)min) { v = *(TYPE *)min; } \
	else if ((v + *(TYPE *)inc) > *(TYPE *)max) { v = *(TYPE *)max; } \
	else { v += *(TYPE *)inc; }					\
	*(TYPE *)value = AG_Unit2Base((double)v, num->unit);		\
}
void
AG_NumericalIncrement(AG_Numerical *num)
{
	AG_Variable *valueb, *minb, *maxb, *incb;
	void *value, *min, *max, *inc;

	AG_OBJECT_ISA(num, "AG_Widget:AG_Numerical:*");
	AG_ObjectLock(num);

	valueb = AG_GetVariable(num, "value", &value);
	minb = AG_GetVariable(num, "min", &min);
	maxb = AG_GetVariable(num, "max", &max);
	incb = AG_GetVariable(num, "inc", &inc);

	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_FLOAT:		ADD_REAL(float);	break;
	case AG_VARIABLE_DOUBLE:	ADD_REAL(double);	break;
	case AG_VARIABLE_INT:		ADD_INT(int);		break;
	case AG_VARIABLE_UINT:		ADD_INT(Uint);		break;
	case AG_VARIABLE_UINT8:		ADD_INT(Uint8);		break;
	case AG_VARIABLE_SINT8:		ADD_INT(Sint8);		break;
	case AG_VARIABLE_UINT16:	ADD_INT(Uint16);	break;
	case AG_VARIABLE_SINT16:	ADD_INT(Sint16);	break;
	case AG_VARIABLE_UINT32:	ADD_INT(Uint32);	break;
	case AG_VARIABLE_SINT32:	ADD_INT(Sint32);	break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	ADD_INT(Uint64);	break;
	case AG_VARIABLE_SINT64:	ADD_INT(Sint64);	break;
#endif
	default:						break;
	}

	AG_PostEvent(num, "numerical-changed", NULL);

	AG_UnlockVariable(valueb);
	AG_UnlockVariable(minb);
	AG_UnlockVariable(maxb);
	AG_UnlockVariable(incb);

	AG_NumericalUpdate(num);
	AG_ObjectUnlock(num);
}
#undef ADD_INT
#undef ADD_REAL

/*
 * Type-independent decrement operation.
 */
#undef SUB_INT
#define SUB_INT(TYPE) {							\
	TYPE v = *(TYPE *)value;					\
	if ((v - *(TYPE *)inc) < *(TYPE *)min) { v = *(TYPE *)min; }	\
	else if ((v - *(TYPE *)inc) > *(TYPE *)max) { v = *(TYPE *)max; } \
	else { v -= *(TYPE *)inc; }					\
	*(TYPE *)value = v;						\
}
#undef SUB_REAL
#define SUB_REAL(TYPE) {						\
	TYPE v = AG_Base2Unit((double)*(TYPE *)value, num->unit);	\
	if ((v - *(TYPE *)inc) < *(TYPE *)min) { v = *(TYPE *)min; }	\
	else if ((v - *(TYPE *)inc) > *(TYPE *)max) { v = *(TYPE *)max; } \
	else { v -= *(TYPE *)inc; }					\
	*(TYPE *)value = AG_Unit2Base((double)v, num->unit);		\
}
void
AG_NumericalDecrement(AG_Numerical *num)
{
	AG_Variable *valueb, *minb, *maxb, *incb;
	void *value, *min, *max, *inc;

	AG_OBJECT_ISA(num, "AG_Widget:AG_Numerical:*");
	AG_ObjectLock(num);

	valueb = AG_GetVariable(num, "value", &value);
	minb = AG_GetVariable(num, "min", &min);
	maxb = AG_GetVariable(num, "max", &max);
	incb = AG_GetVariable(num, "inc", &inc);

	switch (AG_VARIABLE_TYPE(valueb)) {
	case AG_VARIABLE_FLOAT:		SUB_REAL(float);	break;
	case AG_VARIABLE_DOUBLE:	SUB_REAL(double);	break;
	case AG_VARIABLE_INT:		SUB_INT(int);		break;
	case AG_VARIABLE_UINT:		SUB_INT(Uint);		break;
	case AG_VARIABLE_UINT8:		SUB_INT(Uint8);		break;
	case AG_VARIABLE_SINT8:		SUB_INT(Sint8);		break;
	case AG_VARIABLE_UINT16:	SUB_INT(Uint16);	break;
	case AG_VARIABLE_SINT16:	SUB_INT(Sint16);	break;
	case AG_VARIABLE_UINT32:	SUB_INT(Uint32);	break;
	case AG_VARIABLE_SINT32:	SUB_INT(Sint32);	break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	SUB_INT(Uint64);	break;
	case AG_VARIABLE_SINT64:	SUB_INT(Sint64);	break;
#endif
	default:						break;
	}
	AG_PostEvent(num, "numerical-changed", NULL);

	AG_UnlockVariable(valueb);
	AG_UnlockVariable(minb);
	AG_UnlockVariable(maxb);
	AG_UnlockVariable(incb);

	AG_NumericalUpdate(num);
	AG_ObjectUnlock(num);
}
#undef SUB_INT
#undef SUB_REAL

void
AG_NumericalSetPrecision(AG_Numerical *num, const char *mode, int precision)
{
	AG_OBJECT_ISA(num, "AG_Widget:AG_Numerical:*");
	AG_ObjectLock(num);

	num->format[0] = '%';
	num->format[1] = '.';
	num->format[2] = '\0';
	StrlcatInt(num->format, precision, sizeof(num->format));
	Strlcat(num->format, mode, sizeof(num->format));

	AG_NumericalUpdate(num);
	AG_ObjectUnlock(num);
}

void
AG_NumericalSelectUnit(AG_Numerical *num, const char *uname)
{
	AG_Tlist *tl;
	AG_TlistItem *it;

	AG_OBJECT_ISA(num, "AG_Widget:AG_Numerical:*");
	AG_ObjectLock(num);
	tl = num->units->list;
	AG_ObjectLock(tl);

	AG_TlistDeselectAll(tl);

	TAILQ_FOREACH(it, &tl->items, items) {
		const AG_Unit *unit = it->p1;

		if (strcmp(unit->key, uname) == 0) {
			it->selected++;
			num->unit = unit;
			UpdateUnitSelector(num);
			break;
		}
	}

	AG_ObjectUnlock(tl);
	AG_ObjectUnlock(num);
}

void
AG_NumericalSetWriteable(AG_Numerical *num, int enable)
{
	AG_OBJECT_ISA(num, "AG_Widget:AG_Numerical:*");
	AG_ObjectLock(num);

	if (enable) {
		num->flags &= ~(AG_NUMERICAL_READONLY);
		AG_WidgetEnable(num->incbu);
		AG_WidgetEnable(num->decbu);
		AG_WidgetEnable(num->input);
	} else {
		num->flags |= AG_NUMERICAL_READONLY;
		AG_WidgetDisable(num->incbu);
		AG_WidgetDisable(num->decbu);
		AG_WidgetDisable(num->input);
	}

	AG_ObjectUnlock(num);
}

/* Convert the bound value to a float. */
float
AG_NumericalGetFlt(AG_Numerical *num)
{
	AG_Variable *bValue;
	void *value;

	bValue = AG_GetVariable(num, "value", &value);
	switch (AG_VARIABLE_TYPE(bValue)) {
	case AG_VARIABLE_FLOAT:		return *(float *)value;
	case AG_VARIABLE_DOUBLE:	return (float)(*(double *)value);
	case AG_VARIABLE_INT:		return (float)(*(int *)value);
	case AG_VARIABLE_UINT:		return (float)(*(Uint *)value);
	case AG_VARIABLE_UINT8:		return (float)(*(Uint8 *)value);
	case AG_VARIABLE_SINT8:		return (float)(*(Sint8 *)value);
	case AG_VARIABLE_UINT16:	return (float)(*(Uint16 *)value);
	case AG_VARIABLE_SINT16:	return (float)(*(Sint16 *)value);
	case AG_VARIABLE_UINT32:	return (float)(*(Uint32 *)value);
	case AG_VARIABLE_SINT32:	return (float)(*(Sint32 *)value);
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	return (float)(*(Uint64 *)value);
	case AG_VARIABLE_SINT64:	return (float)(*(Sint64 *)value);
#endif
	default:			return (0.0f);
	}
}

/* Convert the bound value to a double. */
double
AG_NumericalGetDbl(AG_Numerical *num)
{
	AG_Variable *bValue;
	void *value;

	bValue = AG_GetVariable(num, "value", &value);
	switch (AG_VARIABLE_TYPE(bValue)) {
	case AG_VARIABLE_FLOAT:		return (double)(*(float *)value);
	case AG_VARIABLE_DOUBLE:	return *(double *)value;
	case AG_VARIABLE_INT:		return (double)(*(int *)value);
	case AG_VARIABLE_UINT:		return (double)(*(Uint *)value);
	case AG_VARIABLE_UINT8:		return (double)(*(Uint8 *)value);
	case AG_VARIABLE_SINT8:		return (double)(*(Sint8 *)value);
	case AG_VARIABLE_UINT16:	return (double)(*(Uint16 *)value);
	case AG_VARIABLE_SINT16:	return (double)(*(Sint16 *)value);
	case AG_VARIABLE_UINT32:	return (double)(*(Uint32 *)value);
	case AG_VARIABLE_SINT32:	return (double)(*(Sint32 *)value);
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	return (double)(*(Uint64 *)value);
	case AG_VARIABLE_SINT64:	return (double)(*(Sint64 *)value);
#endif
	default:			return (0.0);
	}
}

/* Convert the bound value to a natural integer. */
int
AG_NumericalGetInt(AG_Numerical *num)
{
	AG_Variable *bValue;
	void *value;

	bValue = AG_GetVariable(num, "value", &value);
	switch (AG_VARIABLE_TYPE(bValue)) {
	case AG_VARIABLE_FLOAT:		return (int)(*(float *)value);
	case AG_VARIABLE_DOUBLE:	return (int)(*(double *)value);
	case AG_VARIABLE_INT:		return *(int *)value;
	case AG_VARIABLE_UINT:		return (int)(*(Uint *)value);
	case AG_VARIABLE_UINT8:		return (int)(*(Uint8 *)value);
	case AG_VARIABLE_SINT8:		return (int)(*(Sint8 *)value);
	case AG_VARIABLE_UINT16:	return (int)(*(Uint16 *)value);
	case AG_VARIABLE_SINT16:	return (int)(*(Sint16 *)value);
	case AG_VARIABLE_UINT32:	return (int)(*(Uint32 *)value);
	case AG_VARIABLE_SINT32:	return (int)(*(Sint32 *)value);
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	return (int)(*(Uint64 *)value);
	case AG_VARIABLE_SINT64:	return (int)(*(Sint64 *)value);
#endif
	default:			return (0);
	}
}

/* Convert the bound value to a 32-bit integer. */
Uint32
AG_NumericalGetUint32(AG_Numerical *num)
{
	AG_Variable *bValue;
	void *value;

	bValue = AG_GetVariable(num, "value", &value);
	switch (AG_VARIABLE_TYPE(bValue)) {
	case AG_VARIABLE_FLOAT:		return (Uint32)(*(float *)value);
	case AG_VARIABLE_DOUBLE:	return (Uint32)(*(double *)value);
	case AG_VARIABLE_INT:		return (Uint32)(*(int *)value);
	case AG_VARIABLE_UINT:		return (Uint32)(*(Uint *)value);
	case AG_VARIABLE_UINT8:		return (Uint32)(*(Uint8 *)value);
	case AG_VARIABLE_SINT8:		return (Uint32)(*(Sint8 *)value);
	case AG_VARIABLE_UINT16:	return (Uint32)(*(Uint16 *)value);
	case AG_VARIABLE_SINT16:	return (Uint32)(*(Sint16 *)value);
	case AG_VARIABLE_UINT32:	return *(Uint32 *)value;
	case AG_VARIABLE_SINT32:	return (Uint32)(*(Sint32 *)value);
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	return (Uint32)(*(Uint64 *)value);
	case AG_VARIABLE_SINT64:	return (Uint32)(*(Sint64 *)value);
#endif
	default:			return (0UL);
	}
}

#ifdef HAVE_64BIT
/* Convert the bound value to a 64-bit integer. */
Uint64
AG_NumericalGetUint64(AG_Numerical *num)
{
	AG_Variable *bValue;
	void *value;

	bValue = AG_GetVariable(num, "value", &value);
	switch (AG_VARIABLE_TYPE(bValue)) {
	case AG_VARIABLE_FLOAT:		return (Uint64)(*(float *)value);
	case AG_VARIABLE_DOUBLE:	return (Uint64)(*(double *)value);
	case AG_VARIABLE_INT:		return (Uint64)(*(int *)value);
	case AG_VARIABLE_UINT:		return (Uint64)(*(Uint *)value);
	case AG_VARIABLE_UINT8:		return (Uint64)(*(Uint8 *)value);
	case AG_VARIABLE_SINT8:		return (Uint64)(*(Sint8 *)value);
	case AG_VARIABLE_UINT16:	return (Uint64)(*(Uint16 *)value);
	case AG_VARIABLE_SINT16:	return (Uint64)(*(Sint16 *)value);
	case AG_VARIABLE_UINT32:	return (Uint64)(*(Uint32 *)value);
	case AG_VARIABLE_SINT32:	return (Uint64)(*(Sint32 *)value);
	case AG_VARIABLE_UINT64:	return *(Uint64 *)value;
	case AG_VARIABLE_SINT64:	return (Uint64)(*(Sint64 *)value);
	default:			return (0ULL);
	}
}
#endif /* HAVE_64BIT */

AG_WidgetClass agNumericalClass = {
	{
		"Agar(Widget:Numerical)",
		sizeof(AG_Numerical),
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
