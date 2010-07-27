/*
 * Copyright (c) 2008-2010 Hypertriton, Inc. <http://hypertriton.com/>
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
#include <core/config.h>

#include "slider.h"
#include "window.h"
#include "primitive.h"

#include "gui_math.h"

#define TOTSIZE(sl) (((sl)->type==AG_SLIDER_VERT) ? HEIGHT(sl) : WIDTH(sl))

AG_Slider *
AG_SliderNew(void *parent, enum ag_slider_type type, Uint flags)
{
	AG_Slider *sl;

	sl = Malloc(sizeof(AG_Slider));
	AG_ObjectInit(sl, &agSliderClass);
	sl->type = type;
	sl->flags |= flags;

	if (flags & AG_SLIDER_HFILL) { AG_ExpandHoriz(sl); }
	if (flags & AG_SLIDER_VFILL) { AG_ExpandVert(sl); }

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
AG_SliderNewUint8(void *parent, enum ag_slider_type type, Uint flags,
    Uint8 *val, Uint8 *min, Uint8 *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_BindUint8(sl, "value", val); }
	if (min != NULL) { AG_BindUint8(sl, "min", min); }
	if (max != NULL) { AG_BindUint8(sl, "max", max); }
	return (sl);
}
AG_Slider *
AG_SliderNewUint8R(void *parent, enum ag_slider_type type, Uint flags,
    Uint8 *val, Uint8 min, Uint8 max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_BindUint8(sl, "value", val); }
	AG_SetUint8(sl, "min", min);
	AG_SetUint8(sl, "max", max);
	return (sl);
}

AG_Slider *
AG_SliderNewSint8(void *parent, enum ag_slider_type type, Uint flags,
    Sint8 *val, Sint8 *min, Sint8 *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_BindSint8(sl, "value", val); }
	if (min != NULL) { AG_BindSint8(sl, "min", min); }
	if (max != NULL) { AG_BindSint8(sl, "max", max); }
	return (sl);
}
AG_Slider *
AG_SliderNewSint8R(void *parent, enum ag_slider_type type, Uint flags,
    Sint8 *val, Sint8 min, Sint8 max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_BindSint8(sl, "value", val); }
	AG_SetSint8(sl, "min", min);
	AG_SetSint8(sl, "max", max);
	return (sl);
}

AG_Slider *
AG_SliderNewUint16(void *parent, enum ag_slider_type type, Uint flags,
    Uint16 *val, Uint16 *min, Uint16 *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_BindUint16(sl, "value", val); }
	if (min != NULL) { AG_BindUint16(sl, "min", min); }
	if (max != NULL) { AG_BindUint16(sl, "max", max); }
	return (sl);
}
AG_Slider *
AG_SliderNewUint16R(void *parent, enum ag_slider_type type, Uint flags,
    Uint16 *val, Uint16 min, Uint16 max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_BindUint16(sl, "value", val); }
	AG_SetUint16(sl, "min", min);
	AG_SetUint16(sl, "max", max);
	return (sl);
}

AG_Slider *
AG_SliderNewSint16(void *parent, enum ag_slider_type type, Uint flags,
    Sint16 *val, Sint16 *min, Sint16 *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_BindSint16(sl, "value", val); }
	if (min != NULL) { AG_BindSint16(sl, "min", min); }
	if (max != NULL) { AG_BindSint16(sl, "max", max); }
	return (sl);
}
AG_Slider *
AG_SliderNewSint16R(void *parent, enum ag_slider_type type, Uint flags,
    Sint16 *val, Sint16 min, Sint16 max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_BindSint16(sl, "value", val); }
	AG_SetSint16(sl, "min", min);
	AG_SetSint16(sl, "max", max);
	return (sl);
}

AG_Slider *
AG_SliderNewUint32(void *parent, enum ag_slider_type type, Uint flags,
    Uint32 *val, Uint32 *min, Uint32 *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_BindUint32(sl, "value", val); }
	if (min != NULL) { AG_BindUint32(sl, "min", min); }
	if (max != NULL) { AG_BindUint32(sl, "max", max); }
	return (sl);
}
AG_Slider *
AG_SliderNewUint32R(void *parent, enum ag_slider_type type, Uint flags,
    Uint32 *val, Uint32 min, Uint32 max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_BindUint32(sl, "value", val); }
	AG_SetUint32(sl, "min", min);
	AG_SetUint32(sl, "max", max);
	return (sl);
}

AG_Slider *
AG_SliderNewSint32(void *parent, enum ag_slider_type type, Uint flags,
    Sint32 *val, Sint32 *min, Sint32 *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_BindSint32(sl, "value", val); }
	if (min != NULL) { AG_BindSint32(sl, "min", min); }
	if (max != NULL) { AG_BindSint32(sl, "max", max); }
	return (sl);
}
AG_Slider *
AG_SliderNewSint32R(void *parent, enum ag_slider_type type, Uint flags,
    Sint32 *val, Sint32 min, Sint32 max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_BindSint32(sl, "value", val); }
	AG_SetSint32(sl, "min", min);
	AG_SetSint32(sl, "max", max);
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
	AG_ObjectUnlock(sl);
	AG_Redraw(sl);
}

/* Set the base increment for integer bindings. */
void
AG_SliderSetIntIncrement(AG_Slider *sl, int inc)
{
	AG_ObjectLock(sl);
	sl->iInc = inc;
	AG_ObjectUnlock(sl);
}

/* Set the base increment for real bindings. */
void
AG_SliderSetRealIncrement(AG_Slider *sl, double inc)
{
	AG_ObjectLock(sl);
	sl->rInc = inc;
	AG_ObjectUnlock(sl);
}

/*
 * Return the current position of the slider, in terms of the position of the
 * Left (or Top) edge in pixels. Returns 0 on success and -1 if the range
 * is currently <= 0.
 */
#define GET_POSITION(type)						\
	if (*(type *)pMin >= (*(type *)pMax)) {				\
		goto fail;						\
	}								\
	*x = (int)(((*(type *)pVal - *(type *)pMin) * sl->extent) /	\
	          (*(type *)pMax - *(type *)pMin)) 

static __inline__ int
GetPosition(AG_Slider *sl, int *x)
{
	AG_Variable *bMin, *bMax, *bVal;
	void *pMin, *pMax, *pVal;

	bVal = AG_GetVariable(sl, "value", &pVal);
	bMin = AG_GetVariable(sl, "min", &pMin);
	bMax = AG_GetVariable(sl, "max", &pMax);

	switch (AG_VARIABLE_TYPE(bVal)) {
	case AG_VARIABLE_INT:		GET_POSITION(int);	break;
	case AG_VARIABLE_UINT:		GET_POSITION(Uint);	break;
	case AG_VARIABLE_FLOAT:		GET_POSITION(float);	break;
	case AG_VARIABLE_DOUBLE:	GET_POSITION(double);	break;
	case AG_VARIABLE_UINT8:		GET_POSITION(Uint8);	break;
	case AG_VARIABLE_SINT8:		GET_POSITION(Sint8);	break;
	case AG_VARIABLE_UINT16:	GET_POSITION(Uint16);	break;
	case AG_VARIABLE_SINT16:	GET_POSITION(Sint16);	break;
	case AG_VARIABLE_UINT32:	GET_POSITION(Uint32);	break;
	case AG_VARIABLE_SINT32:	GET_POSITION(Sint32);	break;
	default: *x = 0;					break;
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
#define SEEK_TO_POSITION(type)						\
	if (x <= 0) {							\
		*(type *)pVal = *(type *)pMin;				\
	} else if (x >= sl->extent) {					\
		*(type *)pVal = *(type *)pMax;				\
	} else {							\
		*(type *)pVal = x *					\
		    (*(type *)pMax - *(type *)pMin) / sl->extent;	\
		*(type *)pVal += *(type *)pMin;				\
	}

static __inline__ void
SeekToPosition(AG_Slider *sl, int x)
{
	AG_Variable *bMin, *bMax, *bVal;
	void *pMin, *pMax, *pVal;

	bVal = AG_GetVariable(sl, "value", &pVal);
	bMin = AG_GetVariable(sl, "min", &pMin);
	bMax = AG_GetVariable(sl, "max", &pMax);

	switch (AG_VARIABLE_TYPE(bVal)) {
	case AG_VARIABLE_INT:		SEEK_TO_POSITION(int);		break;
	case AG_VARIABLE_UINT:		SEEK_TO_POSITION(Uint);		break;
	case AG_VARIABLE_FLOAT:		SEEK_TO_POSITION(float);	break;
	case AG_VARIABLE_DOUBLE:	SEEK_TO_POSITION(double);	break;
	case AG_VARIABLE_UINT8:		SEEK_TO_POSITION(Uint8);	break;
	case AG_VARIABLE_SINT8:		SEEK_TO_POSITION(Sint8);	break;
	case AG_VARIABLE_UINT16:	SEEK_TO_POSITION(Uint16);	break;
	case AG_VARIABLE_SINT16:	SEEK_TO_POSITION(Sint16);	break;
	case AG_VARIABLE_UINT32:	SEEK_TO_POSITION(Uint32);	break;
	case AG_VARIABLE_SINT32:	SEEK_TO_POSITION(Sint32);	break;
	default:							break;
	} 
	AG_PostEvent(NULL, sl, "slider-changed", NULL);
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bVal);
	AG_Redraw(sl);
}
#undef SEEK_TO_POSITION

/*
 * Decrement the value by the specified amount multiplied by iInc/rInc.
 */
#define DECREMENT_INT(type)						\
	if ((*(type *)pVal - ((type)sl->iInc)*v) < *(type *)pMin) 	\
		*(type *)pVal = *(type *)pMin;				\
	else 								\
		*(type *)pVal -= ((type)sl->iInc)*v

#define DECREMENT_REAL(type)						\
	if ((*(type *)pVal - ((type)v)*sl->rInc) < *(type *)pMin) 	\
		*(type *)pVal = *(type *)pMin;				\
	else 								\
		*(type *)pVal -= ((type)v)*sl->rInc

static void
Decrement(AG_Slider *sl, int v)
{
	AG_Variable *bMin, *bMax, *bVal;
	void *pMin, *pMax, *pVal;

	bVal = AG_GetVariable(sl, "value", &pVal);
	bMin = AG_GetVariable(sl, "min", &pMin);
	bMax = AG_GetVariable(sl, "max", &pMax);

	switch (AG_VARIABLE_TYPE(bVal)) {
	case AG_VARIABLE_INT:		DECREMENT_INT(int);	break;
	case AG_VARIABLE_UINT:		DECREMENT_INT(Uint);	break;
	case AG_VARIABLE_FLOAT:		DECREMENT_REAL(float);	break;
	case AG_VARIABLE_DOUBLE:	DECREMENT_REAL(double);	break;
	case AG_VARIABLE_UINT8:		DECREMENT_INT(Uint8);	break;
	case AG_VARIABLE_SINT8:		DECREMENT_INT(Sint8);	break;
	case AG_VARIABLE_UINT16:	DECREMENT_INT(Uint16);	break;
	case AG_VARIABLE_SINT16:	DECREMENT_INT(Sint16);	break;
	case AG_VARIABLE_UINT32:	DECREMENT_INT(Uint32);	break;
	case AG_VARIABLE_SINT32:	DECREMENT_INT(Sint32);	break;
	default:						break;
	} 
	AG_PostEvent(NULL, sl, "slider-changed", NULL);
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bVal);
	AG_Redraw(sl);
}
#undef DECREMENT_INT
#undef DECREMENT_REAL

/*
 * Increment the value by the specified amount multiplied by iInc/rInc.
 */
#define INCREMENT_INT(type)						\
	if ((*(type *)pVal + ((type)sl->iInc)*v) > *(type *)pMax) 	\
		*(type *)pVal = *(type *)pMax;				\
	else 								\
		*(type *)pVal += ((type)sl->iInc)*v

#define INCREMENT_REAL(type)						\
	if ((*(type *)pVal + ((type)v)*sl->rInc) > *(type *)pMax) 	\
		*(type *)pVal = *(type *)pMax;				\
	else 								\
		*(type *)pVal += ((type)v)*sl->rInc

static void
Increment(AG_Slider *sl, int v)
{
	AG_Variable *bMin, *bMax, *bVal;
	void *pMin, *pMax, *pVal;

	bVal = AG_GetVariable(sl, "value", &pVal);
	bMin = AG_GetVariable(sl, "min", &pMin);
	bMax = AG_GetVariable(sl, "max", &pMax);

	switch (AG_VARIABLE_TYPE(bVal)) {
	case AG_VARIABLE_INT:		INCREMENT_INT(int);	break;
	case AG_VARIABLE_UINT:		INCREMENT_INT(Uint);	break;
	case AG_VARIABLE_FLOAT:		INCREMENT_REAL(float);	break;
	case AG_VARIABLE_DOUBLE:	INCREMENT_REAL(double);	break;
	case AG_VARIABLE_UINT8:		INCREMENT_INT(Uint8);	break;
	case AG_VARIABLE_SINT8:		INCREMENT_INT(Sint8);	break;
	case AG_VARIABLE_UINT16:	INCREMENT_INT(Uint16);	break;
	case AG_VARIABLE_SINT16:	INCREMENT_INT(Sint16);	break;
	case AG_VARIABLE_UINT32:	INCREMENT_INT(Uint32);	break;
	case AG_VARIABLE_SINT32:	INCREMENT_INT(Sint32);	break;
	default:						break;
	} 
	AG_PostEvent(NULL, sl, "slider-changed", NULL);
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bVal);
	AG_Redraw(sl);
}
#undef INCREMENT_INT
#undef INCREMENT_REAL

static void
MouseButtonUp(AG_Event *event)
{
	AG_Slider *sl = AG_SELF();

	if (sl->ctlPressed) {
		sl->ctlPressed = 0;
		sl->xOffs = 0;
		AG_PostEvent(NULL, sl, "slider-drag-end", NULL);
		AG_Redraw(sl);
	}
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Slider *sl = AG_SELF();
	int button = AG_INT(1);
	int x = ((sl->type == AG_SLIDER_HORIZ) ? AG_INT(2) : AG_INT(3));
	int pos;

	if (button != AG_MOUSE_LEFT) {
		return;
	}
	if (GetPosition(sl, &pos) == -1)
		return;

	AG_WidgetFocus(sl);
	if (x >= pos && x <= (pos + sl->wControl)) {
		/*
		 * Click on the slider itself. We don't do anything except
		 * saving the cursor position which we will use in future
		 * mousemotion events.
		 */
		sl->ctlPressed = 1;
		sl->xOffs = x - pos;
		AG_PostEvent(NULL, sl, "slider-drag-begin", NULL);
	} else {
		/*
		 * Click outside of control. We seek to the absolute position
		 * described by the cursor.
		 */
		sl->ctlPressed = 1;
		sl->xOffs = sl->wControl/2;
		SeekToPosition(sl, x - sl->xOffs);
		AG_PostEvent(NULL, sl, "slider-drag-begin", NULL);
	}
	AG_Redraw(sl);
}

static void
MouseMotion(AG_Event *event)
{
	AG_Slider *sl = AG_SELF();

	if (!sl->ctlPressed) {
		return;
	}
	SeekToPosition(sl, ((sl->type == AG_SLIDER_HORIZ) ?
	                    AG_INT(1):AG_INT(2)) - sl->xOffs);
}

static void
KeyDown(AG_Event *event)
{
	AG_Slider *sl = AG_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case AG_KEY_UP:
	case AG_KEY_LEFT:
		Decrement(sl, 1);
		AG_DelTimeout(sl, &sl->incTo);
		AG_ScheduleTimeout(sl, &sl->decTo, agKbdDelay);
		break;
	case AG_KEY_DOWN:
	case AG_KEY_RIGHT:
		Increment(sl, 1);
		AG_DelTimeout(sl, &sl->decTo);
		AG_ScheduleTimeout(sl, &sl->incTo, agKbdDelay);
		break;
	}
}

static void
KeyUp(AG_Event *event)
{
	AG_Slider *sl = AG_SELF();
	int keysym = AG_INT(1);

	switch (keysym) {
	case AG_KEY_UP:
	case AG_KEY_LEFT:
		AG_DelTimeout(sl, &sl->decTo);
		break;
	case AG_KEY_DOWN:
	case AG_KEY_RIGHT:
		AG_DelTimeout(sl, &sl->incTo);
		break;
	}
}

static Uint32
IncrementTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_Slider *sl = obj;
	Increment(sl, 1);
	return (agKbdRepeat);
}

static Uint32
DecrementTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_Slider *sl = obj;
	Decrement(sl, 1);
	return (agKbdRepeat);
}

static void
LostFocus(AG_Event *event)
{
	AG_Slider *sl = AG_SELF();
	AG_DelTimeout(sl, &sl->incTo);
	AG_DelTimeout(sl, &sl->decTo);
}

static void
BoundValue(AG_Event *event)
{
	AG_Slider *sl = AG_SELF();
	AG_Variable *bNew = AG_PTR(1);
	AG_Variable *bValue;
	void *pValue;

	/*
	 * Require that "min" and "max" be of the same binding type as
	 * "value" to avoid derelict hell. Mixing of types could always be
	 * implemented if some application really requires it.
	 */
	if (!strcmp(bNew->name, "min") || !strcmp(bNew->name, "max")) {
		bValue = AG_GetVariable(sl, "value", &pValue);
		if (bValue->type != bNew->type) {
			AG_FatalError("Slider \"%s\" binding type disagree "
			              "with \"value\" binding", bNew->name);
		}
		AG_UnlockVariable(bValue);
	}
}

static void
Init(void *obj)
{
	AG_Slider *sl = obj;

	WIDGET(sl)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP|
	                     AG_WIDGET_UNFOCUSED_MOTION|
			     AG_WIDGET_FOCUSABLE|
			     AG_WIDGET_TABLE_EMBEDDABLE;

	sl->type = AG_SLIDER_HORIZ;
	sl->ctlPressed = 0;
	sl->flags = 0;
	sl->value = 0;
	sl->min = 0;
	sl->max = 0;
	sl->wControlPref = 16;
	sl->wControl = sl->wControlPref;
	sl->xOffs = 0;
	sl->rInc = 1.0;	
	sl->iInc = 1;

	AG_SetEvent(sl, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(sl, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(sl, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(sl, "key-down", KeyDown, NULL);
	AG_SetEvent(sl, "key-up", KeyUp, NULL);
	AG_SetEvent(sl, "widget-lostfocus", LostFocus, NULL);
	AG_AddEvent(sl, "widget-hidden", LostFocus, NULL);
	AG_SetEvent(sl, "bound", BoundValue, NULL);

	AG_SetTimeout(&sl->incTo, IncrementTimeout, NULL, 0);
	AG_SetTimeout(&sl->decTo, DecrementTimeout, NULL, 0);
	
	AG_BindInt(sl, "value", &sl->value);
	AG_BindInt(sl, "min", &sl->min);
	AG_BindInt(sl, "max", &sl->max);
	
	AG_RedrawOnChange(sl, 100, "value");
	AG_RedrawOnChange(sl, 250, "min");
	AG_RedrawOnChange(sl, 250, "max");

#ifdef AG_DEBUG
	AG_BindUint(sl, "flags", &sl->flags);
	AG_BindUint(sl, "type", &sl->type);
	AG_BindInt(sl, "ctlPressed", &sl->ctlPressed);
	AG_BindInt(sl, "wControlPref", &sl->wControlPref);
	AG_BindInt(sl, "wControl", &sl->wControl);
	AG_BindInt(sl, "xOffs", &sl->xOffs);
	AG_BindInt(sl, "extent", &sl->extent);
	AG_BindDouble(sl, "rInc", &sl->rInc);
	AG_BindInt(sl, "iInc", &sl->iInc);
#endif /* AG_DEBUG */
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Slider *sl = obj;
	
	r->w = sl->wControlPref*2 + 10;
	r->h = sl->wControlPref;
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
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
Draw(void *obj)
{
	AG_Slider *sl = obj;
	int x;

	if (GetPosition(sl, &x) == -1) {
		return;
	}
	switch (sl->type) {
	case AG_SLIDER_VERT:
		STYLE(sl)->SliderBackgroundVert(sl);
		STYLE(sl)->SliderControlVert(sl, x, sl->wControl);
		break;
	case AG_SLIDER_HORIZ:
		STYLE(sl)->SliderBackgroundHoriz(sl);
		STYLE(sl)->SliderControlHoriz(sl, x, sl->wControl);
		break;
	}
}

AG_WidgetClass agSliderClass = {
	{
		"Agar(Widget:Slider)",
		sizeof(AG_Slider),
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
