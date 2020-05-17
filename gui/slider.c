/*
 * Copyright (c) 2008-2019 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Slider widget. It can connect to an integer or floating-point variable.
 * 
 * XXX TODO reimplement this whole thing as a tiny subclass of Scrollbar
 * (with a different styling, fixed control bar and no "visible" binding).
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/slider.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>
#include <agar/gui/text.h>
#include <agar/gui/gui_math.h>

#define TOTSIZE(sl) (((sl)->type==AG_SLIDER_VERT) ? HEIGHT(sl) : WIDTH(sl))

AG_Slider *
AG_SliderNew(void *parent, enum ag_slider_type type, Uint flags)
{
	AG_Slider *sl;

	sl = Malloc(sizeof(AG_Slider));
	AG_ObjectInit(sl, &agSliderClass);

	if (flags & AG_SLIDER_HFILL) { WIDGET(sl)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_SLIDER_VFILL) { WIDGET(sl)->flags |= AG_WIDGET_VFILL; }
	sl->flags |= flags;

	sl->type = type;

	AG_ObjectAttach(parent, sl);
	return (sl);
}

AG_Slider *
AG_SliderNewInt(void *parent, enum ag_slider_type type, Uint flags,
    int *val, int *min, int *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_BindInt(sl, "value", val); }
	if (min != NULL) { AG_BindInt(sl, "min", min); }
	if (max != NULL) { AG_BindInt(sl, "max", max); }
	return (sl);
}
AG_Slider *
AG_SliderNewIntR(void *parent, enum ag_slider_type type, Uint flags,
    int *val, int min, int max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_BindInt(sl, "value", val); }
	AG_SetInt(sl, "min", min);
	AG_SetInt(sl, "max", max);
	return (sl);
}

AG_Slider *
AG_SliderNewUint(void *parent, enum ag_slider_type type, Uint flags,
    Uint *val, Uint *min, Uint *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_BindUint(sl, "value", val); }
	if (min != NULL) { AG_BindUint(sl, "min", min); }
	if (max != NULL) { AG_BindUint(sl, "max", max); }
	return (sl);
}
AG_Slider *
AG_SliderNewUintR(void *parent, enum ag_slider_type type, Uint flags,
    Uint *val, Uint min, Uint max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_BindUint(sl, "value", val); }
	AG_SetUint(sl, "min", min);
	AG_SetUint(sl, "max", max);
	return (sl);
}

AG_Slider *
AG_SliderNewFlt(void *parent, enum ag_slider_type type, Uint flags,
    float *val, float *min, float *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_BindFloat(sl, "value", val); }
	if (min != NULL) { AG_BindFloat(sl, "min", min); }
	if (max != NULL) { AG_BindFloat(sl, "max", max); }
	return (sl);
}

AG_Slider *
AG_SliderNewFltR(void *parent, enum ag_slider_type type, Uint flags,
    float *val, float min, float max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_BindFloat(sl, "value", val); }
	AG_SetFloat(sl, "min", min);
	AG_SetFloat(sl, "max", max);
	return (sl);
}

AG_Slider *
AG_SliderNewDbl(void *parent, enum ag_slider_type type, Uint flags,
    double *val, double *min, double *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_BindDouble(sl, "value", val); }
	if (min != NULL) { AG_BindDouble(sl, "min", min); }
	if (max != NULL) { AG_BindDouble(sl, "max", max); }
	return (sl);
}

AG_Slider *
AG_SliderNewDblR(void *parent, enum ag_slider_type type, Uint flags,
    double *val, double min, double max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_BindDouble(sl, "value", val); }
	AG_SetDouble(sl, "min", min);
	AG_SetDouble(sl, "max", max);
	return (sl);
}

/* Set the size of the control in pixels. */
void
AG_SliderSetControlSize(AG_Slider *sl, int size)
{
	AG_OBJECT_ISA(sl, "AG_Widget:AG_Slider:*");
	AG_ObjectLock(sl);

	sl->wControlPref = size;

	switch (sl->type) {
	case AG_SLIDER_HORIZ:
		sl->wControl = MIN(size, HEIGHT(sl));
		break;
	case AG_SLIDER_VERT:
		sl->wControl = MIN(size, WIDTH(sl));
		break;
	}

	AG_Redraw(sl);
	AG_ObjectUnlock(sl);
}

/*
 * Return the current position of the slider, in terms of the position of the
 * Left (or Top) edge in pixels. Returns 0 on success and -1 if the range
 * is currently <= 0.
 */
#define GET_POSITION(TYPE) {						\
	TYPE min = *(TYPE *)pMin;					\
	TYPE max = *(TYPE *)pMax;					\
	if (min >= max) {						\
		goto fail;						\
	}								\
	*x = (int)(((*(TYPE *)pVal - min) * sl->extent) / (max - min));	\
}
static __inline__ int
GetPosition(AG_Slider *_Nonnull sl, int *_Nonnull x)
{
	AG_Variable *bMin, *bMax, *bVal;
	void *pMin, *pMax, *pVal;

	bVal = AG_GetVariable(sl, "value", &pVal);
	bMin = AG_GetVariable(sl, "min", &pMin);
	bMax = AG_GetVariable(sl, "max", &pMax);

	switch (AG_VARIABLE_TYPE(bVal)) {
	case AG_VARIABLE_FLOAT:		GET_POSITION(float);		break;
	case AG_VARIABLE_DOUBLE:	GET_POSITION(double);		break;
	case AG_VARIABLE_INT:		GET_POSITION(int);		break;
	case AG_VARIABLE_UINT:		GET_POSITION(Uint);		break;
	case AG_VARIABLE_UINT8:		GET_POSITION(Uint8);		break;
	case AG_VARIABLE_SINT8:		GET_POSITION(Sint8);		break;
	case AG_VARIABLE_UINT16:	GET_POSITION(Uint16);		break;
	case AG_VARIABLE_SINT16:	GET_POSITION(Sint16);		break;
	case AG_VARIABLE_UINT32:	GET_POSITION(Uint32);		break;
	case AG_VARIABLE_SINT32:	GET_POSITION(Sint32);		break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	GET_POSITION(Uint64);		break;
	case AG_VARIABLE_SINT64:	GET_POSITION(Sint64);		break;
#endif
	default:			*x = 0;				break;
	} 
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bVal);
	return (0);
fail:
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bVal);
	return (-1);
}
#undef GET_POSITION

/*
 * Set the value from a specified position in pixels.
 */
#define SEEK_TO_POSITION(TYPE) {					\
	TYPE min = *(TYPE *)pMin;					\
	TYPE max = *(TYPE *)pMax;					\
	if (x <= 0) {							\
		*(TYPE *)pVal = min;					\
	} else if (x >= sl->extent) {					\
		*(TYPE *)pVal = max;					\
	} else {							\
		*(TYPE *)pVal = min + x*(max-min)/sl->extent;		\
	}								\
}
static __inline__ void
SeekToPosition(AG_Slider *_Nonnull sl, int x)
{
	AG_Variable *bMin, *bMax, *bVal;
	void *pMin, *pMax, *pVal;

	bVal = AG_GetVariable(sl, "value", &pVal);
	bMin = AG_GetVariable(sl, "min", &pMin);
	bMax = AG_GetVariable(sl, "max", &pMax);

	switch (AG_VARIABLE_TYPE(bVal)) {
	case AG_VARIABLE_FLOAT:		SEEK_TO_POSITION(float);	break;
	case AG_VARIABLE_DOUBLE:	SEEK_TO_POSITION(double);	break;
	case AG_VARIABLE_INT:		SEEK_TO_POSITION(int);		break;
	case AG_VARIABLE_UINT:		SEEK_TO_POSITION(Uint);		break;
	case AG_VARIABLE_UINT8:		SEEK_TO_POSITION(Uint8);	break;
	case AG_VARIABLE_SINT8:		SEEK_TO_POSITION(Sint8);	break;
	case AG_VARIABLE_UINT16:	SEEK_TO_POSITION(Uint16);	break;
	case AG_VARIABLE_SINT16:	SEEK_TO_POSITION(Sint16);	break;
	case AG_VARIABLE_UINT32:	SEEK_TO_POSITION(Uint32);	break;
	case AG_VARIABLE_SINT32:	SEEK_TO_POSITION(Sint32);	break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	SEEK_TO_POSITION(Uint64);	break;
	case AG_VARIABLE_SINT64:	SEEK_TO_POSITION(Sint64);	break;
#endif
	default:							break;
	} 

	AG_PostEvent(sl, "slider-changed", NULL);
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bVal);
	AG_Redraw(sl);
}
#undef SEEK_TO_POSITION

/*
 * Type-independent increment/decrement operation.
 */
#define INCREMENT(TYPE) {							\
	if ((*(TYPE *)pVal + *(TYPE *)pInc) > *(TYPE *)pMax) 		\
		*(TYPE *)pVal = *(TYPE *)pMax;				\
	else 								\
		*(TYPE *)pVal += *(TYPE *)pInc;				\
}
#define DECREMENT(TYPE) {							\
	if ((*(TYPE *)pVal - *(TYPE *)pInc) < *(TYPE *)pMin) 		\
		*(TYPE *)pVal = *(TYPE *)pMin;				\
	else 								\
		*(TYPE *)pVal -= *(TYPE *)pInc;				\
}

static void
Increment(AG_Slider *_Nonnull sl)
{
	AG_Variable *bVal, *bMin, *bMax, *bInc;
	void *pVal, *pMin, *pMax, *pInc;

	bVal = AG_GetVariable(sl, "value", &pVal);
	bMin = AG_GetVariable(sl, "min", &pMin);
	bMax = AG_GetVariable(sl, "max", &pMax);
	bInc = AG_GetVariable(sl, "inc", &pInc);

	switch (AG_VARIABLE_TYPE(bVal)) {
	case AG_VARIABLE_FLOAT:		INCREMENT(float);	break;
	case AG_VARIABLE_DOUBLE:	INCREMENT(double);	break;
	case AG_VARIABLE_INT:		INCREMENT(int);		break;
	case AG_VARIABLE_UINT:		INCREMENT(Uint);	break;
	case AG_VARIABLE_UINT8:		INCREMENT(Uint8);	break;
	case AG_VARIABLE_SINT8:		INCREMENT(Sint8);	break;
	case AG_VARIABLE_UINT16:	INCREMENT(Uint16);	break;
	case AG_VARIABLE_SINT16:	INCREMENT(Sint16);	break;
	case AG_VARIABLE_UINT32:	INCREMENT(Uint32);	break;
	case AG_VARIABLE_SINT32:	INCREMENT(Sint32);	break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	INCREMENT(Uint64);	break;
	case AG_VARIABLE_SINT64:	INCREMENT(Sint64);	break;
#endif
	default:						break;
	} 

	AG_PostEvent(sl, "slider-changed", NULL);
	AG_UnlockVariable(bVal);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bInc);
	AG_Redraw(sl);
}

static void
Decrement(AG_Slider *_Nonnull sl)
{
	AG_Variable *bVal, *bMin, *bMax, *bInc;
	void *pVal, *pMin, *pMax, *pInc;

	bVal = AG_GetVariable(sl, "value", &pVal);
	bMin = AG_GetVariable(sl, "min", &pMin);
	bMax = AG_GetVariable(sl, "max", &pMax);
	bInc = AG_GetVariable(sl, "inc", &pInc);

	switch (AG_VARIABLE_TYPE(bVal)) {
	case AG_VARIABLE_FLOAT:		DECREMENT(float);	break;
	case AG_VARIABLE_DOUBLE:	DECREMENT(double);	break;
	case AG_VARIABLE_INT:		DECREMENT(int);		break;
	case AG_VARIABLE_UINT:		DECREMENT(Uint);	break;
	case AG_VARIABLE_UINT8:		DECREMENT(Uint8);	break;
	case AG_VARIABLE_SINT8:		DECREMENT(Sint8);	break;
	case AG_VARIABLE_UINT16:	DECREMENT(Uint16);	break;
	case AG_VARIABLE_SINT16:	DECREMENT(Sint16);	break;
	case AG_VARIABLE_UINT32:	DECREMENT(Uint32);	break;
	case AG_VARIABLE_SINT32:	DECREMENT(Sint32);	break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:	DECREMENT(Uint64);	break;
	case AG_VARIABLE_SINT64:	DECREMENT(Sint64);	break;
#endif
	default:						break;
	} 

	AG_PostEvent(sl, "slider-changed", NULL);
	AG_UnlockVariable(bVal);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bInc);
	AG_Redraw(sl);
}
#undef INCREMENT
#undef DECREMENT

static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	AG_Slider *sl = AG_SLIDER_SELF();

	if (sl->ctlPressed) {
		sl->ctlPressed = 0;
		sl->xOffs = 0;
		AG_PostEvent(sl, "slider-drag-end", NULL);
		AG_Redraw(sl);
	}
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Slider *sl = AG_SLIDER_SELF();
	int button = AG_INT(1);
	int x = ((sl->type == AG_SLIDER_HORIZ) ? AG_INT(2) : AG_INT(3));
	int pos;

	if (button != AG_MOUSE_LEFT) {
		return;
	}
	if (GetPosition(sl, &pos) == -1)
		return;

	if (!AG_WidgetIsFocused(sl)) {
		AG_WidgetFocus(sl);
	}
	if (x >= pos && x <= (pos + sl->wControl)) {
		/*
		 * Click on the slider itself. We don't do anything except
		 * saving the cursor position which we will use in future
		 * mousemotion events.
		 */
		sl->ctlPressed = 1;
		sl->xOffs = x - pos;
		AG_PostEvent(sl, "slider-drag-begin", NULL);
	} else {
		/*
		 * Click outside of control. We seek to the absolute position
		 * described by the cursor.
		 */
		sl->ctlPressed = 1;
		sl->xOffs = (sl->wControl >> 1);
		SeekToPosition(sl, x - sl->xOffs);
		AG_PostEvent(sl, "slider-drag-begin", NULL);
	}
	AG_Redraw(sl);
}

static void
MouseMotion(AG_Event *_Nonnull event)
{
	AG_Slider *sl = AG_SLIDER_SELF();

	if (!sl->ctlPressed) {
		return;
	}
	SeekToPosition(sl, ((sl->type == AG_SLIDER_HORIZ) ?
	                    AG_INT(1):AG_INT(2)) - sl->xOffs);
}

/* Timer callback for keyboard motion. */
static Uint32
MoveTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Slider *sl = AG_SLIDER_SELF();
	int dir = AG_INT(1);

	if (dir < 0) {
		Decrement(sl);
	} else {
		Increment(sl);
	}
	return (agKbdRepeat);
}

static void
KeyDown(AG_Event *_Nonnull event)
{
	AG_Slider *sl = AG_SLIDER_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case AG_KEY_UP:
	case AG_KEY_LEFT:
		Decrement(sl);
		AG_AddTimer(sl, &sl->moveTo, agKbdDelay, MoveTimeout, "%i", -1);
		break;
	case AG_KEY_DOWN:
	case AG_KEY_RIGHT:
		Increment(sl);
		AG_AddTimer(sl, &sl->moveTo, agKbdDelay, MoveTimeout, "%i", +1);
		break;
	}
}

static void
KeyUp(AG_Event *_Nonnull event)
{
	AG_Slider *sl = AG_SLIDER_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case AG_KEY_UP:
	case AG_KEY_LEFT:
	case AG_KEY_DOWN:
	case AG_KEY_RIGHT:
		AG_DelTimer(sl, &sl->moveTo);
		break;
	}
}

static void
OnFocusLoss(AG_Event *_Nonnull event)
{
	AG_Slider *sl = AG_SLIDER_SELF();

	AG_DelTimer(sl, &sl->moveTo);
}

#undef SET_DEF
#define SET_DEF(fn,dmin,dmax,dinc) { 					\
	if (!AG_Defined(sl, "min")) { fn(sl, "min", dmin); }		\
	if (!AG_Defined(sl, "max")) { fn(sl, "max", dmax); }		\
	if (!AG_Defined(sl, "inc")) { fn(sl, "inc", dinc); }		\
}
static void
OnShow(AG_Event *_Nonnull event)
{
	AG_Slider *sl = AG_SLIDER_SELF();
	AG_Variable *V;
	void *dummy;
	
	if ((V = AG_GetVariable(sl, "value", &dummy)) == NULL) {
		V = AG_SetInt(sl, "value", 0);
		AG_LockVariable(V);
	}
	switch (AG_VARIABLE_TYPE(V)) {
	case AG_VARIABLE_FLOAT:  SET_DEF(AG_SetFloat, 0.0f, 1.0f, 0.1f); break;
	case AG_VARIABLE_DOUBLE: SET_DEF(AG_SetDouble, 0.0, 1.0, 0.1); break;
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
	AG_UnlockVariable(V);

	if ((sl->flags & AG_SLIDER_EXCL) == 0) {
		AG_RedrawOnChange(sl, 100, "value");
		AG_RedrawOnChange(sl, 1000, "min");
		AG_RedrawOnChange(sl, 1000, "max");
	}
}
#undef SET_DEF

static void
Init(void *_Nonnull obj)
{
	AG_Slider *sl = obj;

	WIDGET(sl)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP |
	                     AG_WIDGET_UNFOCUSED_MOTION |
			     AG_WIDGET_FOCUSABLE;

	sl->type = AG_SLIDER_HORIZ;
	sl->ctlPressed = 0;
	sl->flags = 0;
	sl->wControlPref = agTextFontHeight;
	sl->wControl = sl->wControlPref;
	sl->xOffs = 0;
	
	AG_AddEvent(sl, "widget-shown", OnShow, NULL);
	AG_SetEvent(sl, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(sl, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(sl, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(sl, "key-down", KeyDown, NULL);
	AG_SetEvent(sl, "key-up", KeyUp, NULL);
	AG_AddEvent(sl, "widget-hidden", OnFocusLoss, NULL);
	AG_SetEvent(sl, "widget-lostfocus", OnFocusLoss, NULL);

	AG_InitTimer(&sl->moveTo, "move", 0);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Slider *sl = obj;
	
	r->w = (sl->wControlPref << 1) + 10;
	r->h =  sl->wControlPref;
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_Slider *sl = obj;

	if (a->w < 4 || a->h < 4) {
		return (-1);
	}
	switch (sl->type) {
	case AG_SLIDER_VERT:
		sl->wControl = MIN(sl->wControlPref, a->h);
		sl->extent = a->h;
		break;
	case AG_SLIDER_HORIZ:
		sl->wControl = MIN(sl->wControlPref, a->w);
		sl->extent = a->w;
		break;
	}
	sl->extent -= sl->wControl;
	return (0);
}

static void
Draw(void *_Nonnull obj)
{
	AG_Slider *sl = obj;
	const AG_Color *cBg = &WCOLOR(sl, BG_COLOR);
	const AG_Color *cFg = &WCOLOR(sl, FG_COLOR);
	AG_Rect r = WIDGET(sl)->r;
	int x;

	if (GetPosition(sl, &x) == -1)
		return;

	switch (sl->type) {
	case AG_SLIDER_VERT:
		AG_DrawBoxSunk(sl, &r, cBg);
		r.y = x;
		r.h = sl->wControl;
		if (sl->ctlPressed) {
			AG_DrawBoxSunk(sl, &r, cFg);
		} else {
			AG_DrawBoxRaised(sl, &r, cFg);
		}
		break;
	case AG_SLIDER_HORIZ:
		AG_DrawBoxSunk(sl, &r, cBg);
		r.x = x;
		r.w = sl->wControl;
		if (sl->ctlPressed) {
			AG_DrawBoxSunk(sl, &r, cFg);
		} else {
			AG_DrawBoxRaised(sl, &r, cFg);
		}
		break;
	}
}

AG_WidgetClass agSliderClass = {
	{
		"Agar(Widget:Slider)",
		sizeof(AG_Slider),
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
