/*
 * Copyright (c) 2002-2008 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "scrollbar.h"
#include "window.h"
#include "primitive.h"
#include "text.h"

#include "gui_math.h"

#define TOTSIZE(sb) (((sb)->type==AG_SCROLLBAR_VERT) ? HEIGHT(sb) : WIDTH(sb))

AG_Scrollbar *
AG_ScrollbarNew(void *parent, enum ag_scrollbar_type type, Uint flags)
{
	AG_Scrollbar *sb;

	sb = Malloc(sizeof(AG_Scrollbar));
	AG_ObjectInit(sb, &agScrollbarClass);
	sb->type = type;
	sb->flags |= flags;

	if (flags & AG_SCROLLBAR_HFILL) { AG_ExpandHoriz(sb); }
	if (flags & AG_SCROLLBAR_VFILL) { AG_ExpandVert(sb); }

	AG_ObjectAttach(parent, sb);
	return (sb);
}

AG_Scrollbar *
AG_ScrollbarNewInt(void *parent, enum ag_scrollbar_type type, Uint flags,
    int *val, int *min, int *max, int *vis)
{
	AG_Scrollbar *sb = AG_ScrollbarNew(parent, type, flags);
	if (val != NULL) { AG_BindInt(sb, "value", val); }
	if (min != NULL) { AG_BindInt(sb, "min", min); }
	if (max != NULL) { AG_BindInt(sb, "max", max); }
	if (vis != NULL) { AG_BindInt(sb, "visible", vis); }
	return (sb);
}
AG_Scrollbar *
AG_ScrollbarNewUint(void *parent, enum ag_scrollbar_type type, Uint flags,
    Uint *val, Uint *min, Uint *max, Uint *vis)
{
	AG_Scrollbar *sb = AG_ScrollbarNew(parent, type, flags);
	if (val != NULL) { AG_BindUint(sb, "value", val); }
	if (min != NULL) { AG_BindUint(sb, "min", min); }
	if (max != NULL) { AG_BindUint(sb, "max", max); }
	if (vis != NULL) { AG_BindUint(sb, "visible", vis); }
	return (sb);
}

AG_Scrollbar *
AG_ScrollbarNewUint8(void *parent, enum ag_scrollbar_type type, Uint flags,
    Uint8 *val, Uint8 *min, Uint8 *max, Uint8 *vis)
{
	AG_Scrollbar *sb = AG_ScrollbarNew(parent, type, flags);
	if (val != NULL) { AG_BindUint8(sb, "value", val); }
	if (min != NULL) { AG_BindUint8(sb, "min", min); }
	if (max != NULL) { AG_BindUint8(sb, "max", max); }
	if (vis != NULL) { AG_BindUint8(sb, "visible", vis); }
	return (sb);
}
AG_Scrollbar *
AG_ScrollbarNewSint8(void *parent, enum ag_scrollbar_type type, Uint flags,
    Sint8 *val, Sint8 *min, Sint8 *max, Sint8 *vis)
{
	AG_Scrollbar *sb = AG_ScrollbarNew(parent, type, flags);
	if (val != NULL) { AG_BindSint8(sb, "value", val); }
	if (min != NULL) { AG_BindSint8(sb, "min", min); }
	if (max != NULL) { AG_BindSint8(sb, "max", max); }
	if (vis != NULL) { AG_BindSint8(sb, "visible", vis); }
	return (sb);
}

AG_Scrollbar *
AG_ScrollbarNewUint16(void *parent, enum ag_scrollbar_type type, Uint flags,
    Uint16 *val, Uint16 *min, Uint16 *max, Uint16 *vis)
{
	AG_Scrollbar *sb = AG_ScrollbarNew(parent, type, flags);
	if (val != NULL) { AG_BindUint16(sb, "value", val); }
	if (min != NULL) { AG_BindUint16(sb, "min", min); }
	if (max != NULL) { AG_BindUint16(sb, "max", max); }
	if (vis != NULL) { AG_BindUint16(sb, "visible", vis); }
	return (sb);
}
AG_Scrollbar *
AG_ScrollbarNewSint16(void *parent, enum ag_scrollbar_type type, Uint flags,
    Sint16 *val, Sint16 *min, Sint16 *max, Sint16 *vis)
{
	AG_Scrollbar *sb = AG_ScrollbarNew(parent, type, flags);
	if (val != NULL) { AG_BindSint16(sb, "value", val); }
	if (min != NULL) { AG_BindSint16(sb, "min", min); }
	if (max != NULL) { AG_BindSint16(sb, "max", max); }
	if (vis != NULL) { AG_BindSint16(sb, "visible", vis); }
	return (sb);
}

AG_Scrollbar *
AG_ScrollbarNewUint32(void *parent, enum ag_scrollbar_type type, Uint flags,
    Uint32 *val, Uint32 *min, Uint32 *max, Uint32 *vis)
{
	AG_Scrollbar *sb = AG_ScrollbarNew(parent, type, flags);
	if (val != NULL) { AG_BindUint32(sb, "value", val); }
	if (min != NULL) { AG_BindUint32(sb, "min", min); }
	if (max != NULL) { AG_BindUint32(sb, "max", max); }
	if (vis != NULL) { AG_BindUint32(sb, "visible", vis); }
	return (sb);
}
AG_Scrollbar *
AG_ScrollbarNewSint32(void *parent, enum ag_scrollbar_type type, Uint flags,
    Sint32 *val, Sint32 *min, Sint32 *max, Sint32 *vis)
{
	AG_Scrollbar *sb = AG_ScrollbarNew(parent, type, flags);
	if (val != NULL) { AG_BindSint32(sb, "value", val); }
	if (min != NULL) { AG_BindSint32(sb, "min", min); }
	if (max != NULL) { AG_BindSint32(sb, "max", max); }
	if (vis != NULL) { AG_BindSint32(sb, "visible", vis); }
	return (sb);
}

AG_Scrollbar *
AG_ScrollbarNewFloat(void *parent, enum ag_scrollbar_type type, Uint flags,
    float *val, float *min, float *max, float *vis)
{
	AG_Scrollbar *sb = AG_ScrollbarNew(parent, type, flags);
	if (val != NULL) { AG_BindFloat(sb, "value", val); }
	if (min != NULL) { AG_BindFloat(sb, "min", min); }
	if (max != NULL) { AG_BindFloat(sb, "max", max); }
	if (vis != NULL) { AG_BindFloat(sb, "visible", vis); }
	return (sb);
}

AG_Scrollbar *
AG_ScrollbarNewDouble(void *parent, enum ag_scrollbar_type type, Uint flags,
    double *val, double *min, double *max, double *vis)
{
	AG_Scrollbar *sb = AG_ScrollbarNew(parent, type, flags);
	if (val != NULL) { AG_BindDouble(sb, "value", val); }
	if (min != NULL) { AG_BindDouble(sb, "min", min); }
	if (max != NULL) { AG_BindDouble(sb, "max", max); }
	if (vis != NULL) { AG_BindDouble(sb, "visible", vis); }
	return (sb);
}

/* Set an alternate handler for UP/LEFT button click. */
void
AG_ScrollbarSetIncFn(AG_Scrollbar *sb, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(sb);
	sb->buttonIncFn = AG_SetEvent(sb, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(sb->buttonIncFn, fmt);
	AG_ObjectUnlock(sb);
}

/* Set an alternate handler for DOWN/RIGHT button click. */
void
AG_ScrollbarSetDecFn(AG_Scrollbar *sb, AG_EventFn fn, const char *fmt, ...)
{
	AG_ObjectLock(sb);
	sb->buttonDecFn = AG_SetEvent(sb, NULL, fn, NULL);
	AG_EVENT_GET_ARGS(sb->buttonDecFn, fmt);
	AG_ObjectUnlock(sb);
}

/* Set the base increment for integer bindings. */
void
AG_ScrollbarSetIntIncrement(AG_Scrollbar *sb, int inc)
{
	AG_ObjectLock(sb);
	sb->iInc = inc;
	AG_ObjectUnlock(sb);
}

/* Set the base increment for real bindings. */
void
AG_ScrollbarSetRealIncrement(AG_Scrollbar *sb, double inc)
{
	AG_ObjectLock(sb);
	sb->rInc = inc;
	AG_ObjectUnlock(sb);
}

/*
 * Return the current position of the scrollbar, in terms of the position of
 * the Left (or Top) edge in pixels. Returns 0 on success and -1 if the range
 * is currently <= 0.
 */
#define GET_POSITION(type)						\
	if (*(type *)pMin >= (*(type *)pMax) - *(type *)pVis) {		\
		goto fail;						\
	}								\
	*x = (int)(((*(type *)pVal - *(type *)pMin) * sb->extent) /	\
	          (*(type *)pMax - *(type *)pVis - *(type *)pMin)) 

static __inline__ int
GetPosition(AG_Scrollbar *sb, int *x)
{
	AG_WidgetBinding *bMin, *bMax, *bVis, *bVal;
	void *pMin, *pMax, *pVal, *pVis;

	bVal = AG_WidgetGetBinding(sb, "value", &pVal);
	bMin = AG_WidgetGetBinding(sb, "min", &pMin);
	bMax = AG_WidgetGetBinding(sb, "max", &pMax);
	bVis = AG_WidgetGetBinding(sb, "visible", &pVis);

	switch (bVal->type) {
	case AG_WIDGET_INT:	GET_POSITION(int);	break;
	case AG_WIDGET_UINT:	GET_POSITION(Uint);	break;
	case AG_WIDGET_FLOAT:	GET_POSITION(float);	break;
	case AG_WIDGET_DOUBLE:	GET_POSITION(double);	break;
	case AG_WIDGET_UINT8:	GET_POSITION(Uint8);	break;
	case AG_WIDGET_SINT8:	GET_POSITION(Sint8);	break;
	case AG_WIDGET_UINT16:	GET_POSITION(Uint16);	break;
	case AG_WIDGET_SINT16:	GET_POSITION(Sint16);	break;
	case AG_WIDGET_UINT32:	GET_POSITION(Uint32);	break;
	case AG_WIDGET_SINT32:	GET_POSITION(Sint32);	break;
	default:					break;
	} 
	if (sb->wBar == -1) {
		*x = 0;
	}
	AG_WidgetUnlockBinding(bVis);
	AG_WidgetUnlockBinding(bMax);
	AG_WidgetUnlockBinding(bMin);
	AG_WidgetUnlockBinding(bVal);
	return (0);
fail:
	AG_WidgetUnlockBinding(bVis);
	AG_WidgetUnlockBinding(bMax);
	AG_WidgetUnlockBinding(bMin);
	AG_WidgetUnlockBinding(bVal);
	return (-1);
}
#undef GET_POSITION

/*
 * Set the value from a specified position in pixels.
 */
#define SEEK_TO_POSITION(type)						\
	if (x <= 0) {							\
		*(type *)pVal = *(type *)pMin;				\
	} else if (x >= sb->extent) {					\
		*(type *)pVal = *(type *)pMax - *(type *)pVis;		\
	} else {							\
		*(type *)pVal = x *					\
		    (*(type *)pMax - *(type *)pVis - *(type *)pMin) /	\
		    sb->extent;						\
		*(type *)pVal += *(type *)pMin;				\
		if (*(type *)pVal < *(type *)pMin) {			\
			*(type *)pVal = *(type *)pMin;			\
		}							\
		if (*(type *)pVal > *(type *)pMax) {			\
			*(type *)pVal = *(type *)pMax;			\
		}							\
	}

static __inline__ void
SeekToPosition(AG_Scrollbar *sb, int x)
{
	AG_WidgetBinding *bMin, *bMax, *bVis, *bVal;
	void *pMin, *pMax, *pVal, *pVis;

	bVal = AG_WidgetGetBinding(sb, "value", &pVal);
	bMin = AG_WidgetGetBinding(sb, "min", &pMin);
	bMax = AG_WidgetGetBinding(sb, "max", &pMax);
	bVis = AG_WidgetGetBinding(sb, "visible", &pVis);

	switch (bVal->type) {
	case AG_WIDGET_INT:	SEEK_TO_POSITION(int);		break;
	case AG_WIDGET_UINT:	SEEK_TO_POSITION(Uint);		break;
	case AG_WIDGET_FLOAT:	SEEK_TO_POSITION(float);	break;
	case AG_WIDGET_DOUBLE:	SEEK_TO_POSITION(double);	break;
	case AG_WIDGET_UINT8:	SEEK_TO_POSITION(Uint8);	break;
	case AG_WIDGET_SINT8:	SEEK_TO_POSITION(Sint8);	break;
	case AG_WIDGET_UINT16:	SEEK_TO_POSITION(Uint16);	break;
	case AG_WIDGET_SINT16:	SEEK_TO_POSITION(Sint16);	break;
	case AG_WIDGET_UINT32:	SEEK_TO_POSITION(Uint32);	break;
	case AG_WIDGET_SINT32:	SEEK_TO_POSITION(Sint32);	break;
	default:						break;
	} 
	AG_PostEvent(NULL, sb, "scrollbar-changed", NULL);
	AG_WidgetUnlockBinding(bVis);
	AG_WidgetUnlockBinding(bMax);
	AG_WidgetUnlockBinding(bMin);
	AG_WidgetUnlockBinding(bVal);
}
#undef SEEK_TO_POSITION

/*
 * Decrement the value by the specified amount multiplied by iInc/rInc.
 */
#define DECREMENT_INT(type)						\
	if ((*(type *)pVal - ((type)sb->iInc)*v) < *(type *)pMin) 	\
		*(type *)pVal = *(type *)pMin;				\
	else 								\
		*(type *)pVal -= ((type)sb->iInc)*v

#define DECREMENT_REAL(type)						\
	if ((*(type *)pVal - ((type)v)*sb->rInc) < *(type *)pMin) 	\
		*(type *)pVal = *(type *)pMin;				\
	else 								\
		*(type *)pVal -= ((type)v)*sb->rInc

static void
Decrement(AG_Scrollbar *sb, int v)
{
	AG_WidgetBinding *bMin, *bMax, *bVis, *bVal;
	void *pMin, *pMax, *pVal, *pVis;

	bVal = AG_WidgetGetBinding(sb, "value", &pVal);
	bMin = AG_WidgetGetBinding(sb, "min", &pMin);
	bMax = AG_WidgetGetBinding(sb, "max", &pMax);
	bVis = AG_WidgetGetBinding(sb, "visible", &pVis);

	switch (bVal->type) {
	case AG_WIDGET_INT:	DECREMENT_INT(int);	break;
	case AG_WIDGET_UINT:	DECREMENT_INT(Uint);	break;
	case AG_WIDGET_FLOAT:	DECREMENT_REAL(float);	break;
	case AG_WIDGET_DOUBLE:	DECREMENT_REAL(double);	break;
	case AG_WIDGET_UINT8:	DECREMENT_INT(Uint8);	break;
	case AG_WIDGET_SINT8:	DECREMENT_INT(Sint8);	break;
	case AG_WIDGET_UINT16:	DECREMENT_INT(Uint16);	break;
	case AG_WIDGET_SINT16:	DECREMENT_INT(Sint16);	break;
	case AG_WIDGET_UINT32:	DECREMENT_INT(Uint32);	break;
	case AG_WIDGET_SINT32:	DECREMENT_INT(Sint32);	break;
	default:					break;
	} 
	AG_PostEvent(NULL, sb, "scrollbar-changed", NULL);
	AG_WidgetUnlockBinding(bVis);
	AG_WidgetUnlockBinding(bMax);
	AG_WidgetUnlockBinding(bMin);
	AG_WidgetUnlockBinding(bVal);
}
#undef DECREMENT_INT
#undef DECREMENT_REAL

/*
 * Increment the value by the specified amount multiplied by iInc/rInc.
 */
#define INCREMENT_INT(type)						\
	if ((*(type *)pVal + ((type)sb->iInc)*v) > *(type *)pMax) 	\
		*(type *)pVal = *(type *)pMax;				\
	else 								\
		*(type *)pVal += ((type)sb->iInc)*v

#define INCREMENT_REAL(type)						\
	if ((*(type *)pVal + ((type)v)*sb->rInc) > *(type *)pMax) 	\
		*(type *)pVal = *(type *)pMax;				\
	else 								\
		*(type *)pVal += ((type)v)*sb->rInc

static void
Increment(AG_Scrollbar *sb, int v)
{
	AG_WidgetBinding *bMin, *bMax, *bVis, *bVal;
	void *pMin, *pMax, *pVal, *pVis;

	bVal = AG_WidgetGetBinding(sb, "value", &pVal);
	bMin = AG_WidgetGetBinding(sb, "min", &pMin);
	bMax = AG_WidgetGetBinding(sb, "max", &pMax);
	bVis = AG_WidgetGetBinding(sb, "visible", &pVis);

	switch (bVal->type) {
	case AG_WIDGET_INT:	INCREMENT_INT(int);	break;
	case AG_WIDGET_UINT:	INCREMENT_INT(Uint);	break;
	case AG_WIDGET_FLOAT:	INCREMENT_REAL(float);	break;
	case AG_WIDGET_DOUBLE:	INCREMENT_REAL(double);	break;
	case AG_WIDGET_UINT8:	INCREMENT_INT(Uint8);	break;
	case AG_WIDGET_SINT8:	INCREMENT_INT(Sint8);	break;
	case AG_WIDGET_UINT16:	INCREMENT_INT(Uint16);	break;
	case AG_WIDGET_SINT16:	INCREMENT_INT(Sint16);	break;
	case AG_WIDGET_UINT32:	INCREMENT_INT(Uint32);	break;
	case AG_WIDGET_SINT32:	INCREMENT_INT(Sint32);	break;
	default:					break;
	} 
	AG_PostEvent(NULL, sb, "scrollbar-changed", NULL);
	AG_WidgetUnlockBinding(bVis);
	AG_WidgetUnlockBinding(bMax);
	AG_WidgetUnlockBinding(bMin);
	AG_WidgetUnlockBinding(bVal);
}
#undef INCREMENT_INT
#undef INCREMENT_REAL

static void
MouseButtonUp(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();

	AG_DelTimeout(sb, &sb->scrollTo);
	AG_DelTimeout(sb, &sb->incTo);
	AG_DelTimeout(sb, &sb->decTo);

	if (sb->curBtn == AG_SCROLLBAR_BUTTON_DEC && sb->buttonDecFn != NULL) {
		AG_PostEvent(NULL, sb, sb->buttonDecFn->name, "%i", 0);
	}
	if (sb->curBtn == AG_SCROLLBAR_BUTTON_INC && sb->buttonIncFn != NULL) {
		AG_PostEvent(NULL, sb, sb->buttonIncFn->name, "%i", 0);
	}

	if (sb->curBtn != AG_SCROLLBAR_BUTTON_NONE) {
		sb->curBtn = AG_SCROLLBAR_BUTTON_NONE;
		sb->xOffs = 0;
	}
	AG_PostEvent(NULL, sb, "scrollbar-drag-end", NULL);
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();
	int button = AG_INT(1);
	int x = ((sb->type == AG_SCROLLBAR_HORIZ) ? AG_INT(2) : AG_INT(3)) -
	        sb->wButton;
	int pos, posFound;

	if (button != SDL_BUTTON_LEFT) {
		return;
	}
	AG_WidgetFocus(sb);
	posFound = (GetPosition(sb, &pos) == 0);
	if (x < 0) {
		/*
		 * Click on DECREMENT button. Unless user provided a handler
		 * function, we decrement once and start the timer.
		 */
		sb->curBtn = AG_SCROLLBAR_BUTTON_DEC;
		if (sb->buttonDecFn != NULL) {
			AG_PostEvent(NULL, sb, sb->buttonDecFn->name, "%i", 1);
		} else {
			Decrement(sb, 1);
			AG_ScheduleTimeout(sb, &sb->scrollTo, agMouseSpinDelay);
			AG_DelTimeout(sb, &sb->incTo);
			AG_DelTimeout(sb, &sb->decTo);
		}
	} else if (x > TOTSIZE(sb) - sb->wButton*2) {
		/*
		 * Click on INCREMENT button. Unless user provided a handler
		 * function, we increment once and start the timer.
		 */
		sb->curBtn = AG_SCROLLBAR_BUTTON_INC;
		if (sb->buttonIncFn != NULL) {
			AG_PostEvent(NULL, sb, sb->buttonIncFn->name, "%i", 1);
		} else {
			Increment(sb, 1);
			AG_ScheduleTimeout(sb, &sb->scrollTo, agMouseSpinDelay);
			AG_DelTimeout(sb, &sb->incTo);
			AG_DelTimeout(sb, &sb->decTo);
		}
	} else if (!posFound || (x >= pos && x <= (pos + sb->wBar))) {
		/*
		 * Click on the scrollbar itself. We don't do anything except
		 * saving the cursor position which we will use in future
		 * mousemotion events.
		 */
		sb->curBtn = AG_SCROLLBAR_BUTTON_SCROLL;
		sb->xOffs = posFound ? (x - pos) : x;
	} else if (sb->wBar != -1) {
		/*
		 * Click outside of scrollbar. We seek to the absolute position
		 * described by the cursor.
		 *
		 * XXX TODO: Provide an option to scroll progressively to the
		 * position since many users will expect that.
		 */
		sb->curBtn = AG_SCROLLBAR_BUTTON_SCROLL;
		sb->xOffs = sb->wBar/2;
		SeekToPosition(sb, x - sb->xOffs);
	}
	AG_PostEvent(NULL, sb, "scrollbar-drag-begin", NULL);
}

static void
MouseMotion(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();

	if (sb->curBtn != AG_SCROLLBAR_BUTTON_SCROLL) {
		return;
	}
	SeekToPosition(sb, ((sb->type == AG_SCROLLBAR_HORIZ) ?
	                    AG_INT(1):AG_INT(2)) - sb->wButton - sb->xOffs);
}

static Uint32
ScrollTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_Scrollbar *sb = obj;

	switch (sb->curBtn) {
	case AG_SCROLLBAR_BUTTON_DEC:
		Decrement(sb, 1);
		break;
	case AG_SCROLLBAR_BUTTON_INC:
		Increment(sb, 1);
		break;
	default:
		break;
	}
	return (agMouseSpinIval);
}

static void
KeyDown(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case SDLK_UP:
	case SDLK_LEFT:
		Decrement(sb, 1);
		AG_DelTimeout(sb, &sb->incTo);
		AG_ScheduleTimeout(sb, &sb->decTo, agKbdDelay);
		break;
	case SDLK_DOWN:
	case SDLK_RIGHT:
		Increment(sb, 1);
		AG_DelTimeout(sb, &sb->decTo);
		AG_ScheduleTimeout(sb, &sb->incTo, agKbdDelay);
		break;
	}
}

static void
KeyUp(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case SDLK_UP:
	case SDLK_LEFT:
		AG_DelTimeout(sb, &sb->decTo);
		break;
	case SDLK_DOWN:
	case SDLK_RIGHT:
		AG_DelTimeout(sb, &sb->incTo);
		break;
	}
}

static Uint32
IncrementTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_Scrollbar *sb = obj;
	Increment(sb, 1);
	return (agKbdRepeat);
}

static Uint32
DecrementTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_Scrollbar *sb = obj;
	Decrement(sb, 1);
	return (agKbdRepeat);
}

static void
LostFocus(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();

	AG_DelTimeout(sb, &sb->scrollTo);
	AG_DelTimeout(sb, &sb->incTo);
	AG_DelTimeout(sb, &sb->decTo);
}

static void
BoundValue(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();
	AG_WidgetBinding *bNew = AG_PTR(1);
	AG_WidgetBinding *bValue;
	void *pValue;

	/*
	 * Require that "min", "max" and "visible" be of the same binding
	 * type as "value" to avoid derelict hell. Mixing of types could
	 * always be implemented if some application really requires it.
	 */
	if (!strcmp(bNew->name, "min") || !strcmp(bNew->name, "max") ||
	    !strcmp(bNew->name, "visible")) {
		bValue = AG_WidgetGetBinding(sb, "value", &pValue);
		if (bValue->type != bNew->type) {
			AG_FatalError("Scrollbar \"%s\" binding type disagree "
			              "with \"value\" binding", bNew->name);
		}
		AG_WidgetUnlockBinding(bValue);
	}
}

static void
Init(void *obj)
{
	AG_Scrollbar *sb = obj;

	WIDGET(sb)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP|
	                     AG_WIDGET_UNFOCUSED_MOTION|
			     AG_WIDGET_FOCUSABLE;

	AG_BindInt(sb, "value", &sb->value);
	AG_BindInt(sb, "min", &sb->min);
	AG_BindInt(sb, "max", &sb->max);
	AG_BindInt(sb, "visible", &sb->visible);

	sb->type = AG_SCROLLBAR_HORIZ;
	sb->curBtn = AG_SCROLLBAR_BUTTON_NONE;
	sb->flags = 0;
	sb->value = 0;
	sb->min = 0;
	sb->max = 0;
	sb->visible = 0;
	sb->buttonIncFn = NULL;
	sb->buttonDecFn = NULL;
	sb->xOffs = 0;
	sb->rInc = 1.0;	
	sb->iInc = 1;

	sb->wBar = 8;
	sb->wButton = 16;
	sb->hArrow = sb->wButton*5/9;

	AG_SetEvent(sb, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(sb, "window-mousebuttonup", MouseButtonUp, NULL);
	AG_SetEvent(sb, "window-mousemotion", MouseMotion, NULL);
	AG_SetEvent(sb, "window-keydown", KeyDown, NULL);
	AG_SetEvent(sb, "window-keyup", KeyUp, NULL);
	AG_SetEvent(sb, "widget-lostfocus", LostFocus, NULL);
	AG_SetEvent(sb, "widget-hidden", LostFocus, NULL);
	AG_SetEvent(sb, "widget-bound", BoundValue, NULL);

	AG_SetTimeout(&sb->scrollTo, ScrollTimeout, NULL, 0);
	AG_SetTimeout(&sb->decTo, DecrementTimeout, NULL, 0);
	AG_SetTimeout(&sb->incTo, IncrementTimeout, NULL, 0);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Scrollbar *sb = obj;

	switch (sb->type) {
	case AG_SCROLLBAR_HORIZ:
		r->w = sb->wButton*2;
		r->h = sb->wButton;
		break;
	case AG_SCROLLBAR_VERT:
		r->w = sb->wButton;
		r->h = sb->wButton*2;
		break;
	}
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Scrollbar *sb = obj;

	if (a->w < 4 || a->h < 4) {
		return (-1);
	}
	sb->extent = ((sb->type==AG_SCROLLBAR_VERT) ? a->h : a->w) -
	             sb->wButton*2 - sb->wBar;
	if (sb->extent < 0) { sb->extent = 0; }
	return (0);
}

static void
DrawText(AG_Scrollbar *sb)
{
	AG_Surface *txt;
	char label[32];

	AG_PushTextState();
	AG_TextColor(TEXT_COLOR);
	AG_TextBGColorHex(0xccccccff);

	Snprintf(label, sizeof(label), "%d < %d < %d(%d)",
	    AG_WidgetInt(sb,"min"),
	    AG_WidgetInt(sb,"value"),
	    AG_WidgetInt(sb,"max"),
	    AG_WidgetInt(sb,"visible"));

	/* XXX inefficient */
	txt = AG_TextRender(label);
	AG_WidgetBlit(sb, txt,
	    WIDTH(sb)/2 - txt->w/2,
	    HEIGHT(sb)/2 - txt->h/2);
	AG_SurfaceFree(txt);
	
	AG_PopTextState();
}

static void
Draw(void *obj)
{
	AG_Scrollbar *sb = obj;
	int x, size;

	if (GetPosition(sb, &x) == 0) {
		size = (sb->wBar == -1) ? sb->extent : sb->wBar;
		if (size < 0) { size = 0; }
	} else {
		x = 0;
		size = sb->extent + sb->wBar;
	}
	
	switch (sb->type) {
	case AG_SCROLLBAR_VERT:
		STYLE(sb)->ScrollbarVert(sb, x, size);
		break;
	case AG_SCROLLBAR_HORIZ:
		STYLE(sb)->ScrollbarHoriz(sb, x, size);
		break;
	}
	if (sb->flags & AG_SCROLLBAR_TEXT)
		DrawText(sb);
}

/* Return 1 if it is useful to display the scrollbar given the current range. */
int
AG_ScrollbarVisible(AG_Scrollbar *sb)
{
	int rv, x;

	AG_ObjectLock(sb);
	rv = (GetPosition(sb, &x) == -1) ? 0 : 1;
	AG_ObjectUnlock(sb);
	return (rv);
}

AG_WidgetClass agScrollbarClass = {
	{
		"Agar(Widget:Scrollbar)",
		sizeof(AG_Scrollbar),
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
