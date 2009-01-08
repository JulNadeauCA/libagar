/*
 * Copyright (c) 2008 Hypertriton, Inc. <http://hypertriton.com/>
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
	if (val != NULL) { AG_WidgetBindInt(sl, "value", val); }
	if (min != NULL) { AG_WidgetBindInt(sl, "min", min); }
	if (max != NULL) { AG_WidgetBindInt(sl, "max", max); }
	return (sl);
}
AG_Slider *
AG_SliderNewIntR(void *parent, enum ag_slider_type type, Uint flags,
    int *val, int min, int max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindInt(sl, "value", val); }
	AG_WidgetSetInt(sl, "min", min);
	AG_WidgetSetInt(sl, "max", max);
	return (sl);
}

AG_Slider *
AG_SliderNewUint(void *parent, enum ag_slider_type type, Uint flags,
    Uint *val, Uint *min, Uint *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindUint(sl, "value", val); }
	if (min != NULL) { AG_WidgetBindUint(sl, "min", min); }
	if (max != NULL) { AG_WidgetBindUint(sl, "max", max); }
	return (sl);
}
AG_Slider *
AG_SliderNewUintR(void *parent, enum ag_slider_type type, Uint flags,
    Uint *val, Uint min, Uint max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindUint(sl, "value", val); }
	AG_WidgetSetUint(sl, "min", min);
	AG_WidgetSetUint(sl, "max", max);
	return (sl);
}

AG_Slider *
AG_SliderNewUint8(void *parent, enum ag_slider_type type, Uint flags,
    Uint8 *val, Uint8 *min, Uint8 *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindUint8(sl, "value", val); }
	if (min != NULL) { AG_WidgetBindUint8(sl, "min", min); }
	if (max != NULL) { AG_WidgetBindUint8(sl, "max", max); }
	return (sl);
}
AG_Slider *
AG_SliderNewUint8R(void *parent, enum ag_slider_type type, Uint flags,
    Uint8 *val, Uint8 min, Uint8 max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindUint8(sl, "value", val); }
	AG_WidgetSetUint8(sl, "min", min);
	AG_WidgetSetUint8(sl, "max", max);
	return (sl);
}

AG_Slider *
AG_SliderNewSint8(void *parent, enum ag_slider_type type, Uint flags,
    Sint8 *val, Sint8 *min, Sint8 *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindSint8(sl, "value", val); }
	if (min != NULL) { AG_WidgetBindSint8(sl, "min", min); }
	if (max != NULL) { AG_WidgetBindSint8(sl, "max", max); }
	return (sl);
}
AG_Slider *
AG_SliderNewSint8R(void *parent, enum ag_slider_type type, Uint flags,
    Sint8 *val, Sint8 min, Sint8 max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindSint8(sl, "value", val); }
	AG_WidgetSetSint8(sl, "min", min);
	AG_WidgetSetSint8(sl, "max", max);
	return (sl);
}

AG_Slider *
AG_SliderNewUint16(void *parent, enum ag_slider_type type, Uint flags,
    Uint16 *val, Uint16 *min, Uint16 *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindUint16(sl, "value", val); }
	if (min != NULL) { AG_WidgetBindUint16(sl, "min", min); }
	if (max != NULL) { AG_WidgetBindUint16(sl, "max", max); }
	return (sl);
}
AG_Slider *
AG_SliderNewUint16R(void *parent, enum ag_slider_type type, Uint flags,
    Uint16 *val, Uint16 min, Uint16 max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindUint16(sl, "value", val); }
	AG_WidgetSetUint16(sl, "min", min);
	AG_WidgetSetUint16(sl, "max", max);
	return (sl);
}

AG_Slider *
AG_SliderNewSint16(void *parent, enum ag_slider_type type, Uint flags,
    Sint16 *val, Sint16 *min, Sint16 *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindSint16(sl, "value", val); }
	if (min != NULL) { AG_WidgetBindSint16(sl, "min", min); }
	if (max != NULL) { AG_WidgetBindSint16(sl, "max", max); }
	return (sl);
}
AG_Slider *
AG_SliderNewSint16R(void *parent, enum ag_slider_type type, Uint flags,
    Sint16 *val, Sint16 min, Sint16 max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindSint16(sl, "value", val); }
	AG_WidgetSetSint16(sl, "min", min);
	AG_WidgetSetSint16(sl, "max", max);
	return (sl);
}

AG_Slider *
AG_SliderNewUint32(void *parent, enum ag_slider_type type, Uint flags,
    Uint32 *val, Uint32 *min, Uint32 *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindUint32(sl, "value", val); }
	if (min != NULL) { AG_WidgetBindUint32(sl, "min", min); }
	if (max != NULL) { AG_WidgetBindUint32(sl, "max", max); }
	return (sl);
}
AG_Slider *
AG_SliderNewUint32R(void *parent, enum ag_slider_type type, Uint flags,
    Uint32 *val, Uint32 min, Uint32 max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindUint32(sl, "value", val); }
	AG_WidgetSetUint32(sl, "min", min);
	AG_WidgetSetUint32(sl, "max", max);
	return (sl);
}

AG_Slider *
AG_SliderNewSint32(void *parent, enum ag_slider_type type, Uint flags,
    Sint32 *val, Sint32 *min, Sint32 *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindSint32(sl, "value", val); }
	if (min != NULL) { AG_WidgetBindSint32(sl, "min", min); }
	if (max != NULL) { AG_WidgetBindSint32(sl, "max", max); }
	return (sl);
}
AG_Slider *
AG_SliderNewSint32R(void *parent, enum ag_slider_type type, Uint flags,
    Sint32 *val, Sint32 min, Sint32 max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindSint32(sl, "value", val); }
	AG_WidgetSetSint32(sl, "min", min);
	AG_WidgetSetSint32(sl, "max", max);
	return (sl);
}

#ifdef HAVE_64BIT
AG_Slider *
AG_SliderNewUint64(void *parent, enum ag_slider_type type, Uint flags,
    Uint64 *val, Uint64 *min, Uint64 *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindUint64(sl, "value", val); }
	if (min != NULL) { AG_WidgetBindUint64(sl, "min", min); }
	if (max != NULL) { AG_WidgetBindUint64(sl, "max", max); }
	return (sl);
}
AG_Slider *
AG_SliderNewUint64R(void *parent, enum ag_slider_type type, Uint flags,
    Uint64 *val, Uint64 min, Uint64 max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindUint64(sl, "value", val); }
	AG_WidgetSetUint64(sl, "min", min);
	AG_WidgetSetUint64(sl, "max", max);
	return (sl);
}

AG_Slider *
AG_SliderNewSint64(void *parent, enum ag_slider_type type, Uint flags,
    Sint64 *val, Sint64 *min, Sint64 *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindSint64(sl, "value", val); }
	if (min != NULL) { AG_WidgetBindSint64(sl, "min", min); }
	if (max != NULL) { AG_WidgetBindSint64(sl, "max", max); }
	return (sl);
}

AG_Slider *
AG_SliderNewSint64R(void *parent, enum ag_slider_type type, Uint flags,
    Sint64 *val, Sint64 min, Sint64 max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindSint64(sl, "value", val); }
	AG_WidgetSetSint64(sl, "min", min);
	AG_WidgetSetSint64(sl, "max", max);
	return (sl);
}
#endif /* HAVE_64BIT */

AG_Slider *
AG_SliderNewFlt(void *parent, enum ag_slider_type type, Uint flags,
    float *val, float *min, float *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindFloat(sl, "value", val); }
	if (min != NULL) { AG_WidgetBindFloat(sl, "min", min); }
	if (max != NULL) { AG_WidgetBindFloat(sl, "max", max); }
	return (sl);
}

AG_Slider *
AG_SliderNewFltR(void *parent, enum ag_slider_type type, Uint flags,
    float *val, float min, float max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindFloat(sl, "value", val); }
	AG_WidgetSetFloat(sl, "min", min);
	AG_WidgetSetFloat(sl, "max", max);
	return (sl);
}

AG_Slider *
AG_SliderNewDbl(void *parent, enum ag_slider_type type, Uint flags,
    double *val, double *min, double *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindDouble(sl, "value", val); }
	if (min != NULL) { AG_WidgetBindDouble(sl, "min", min); }
	if (max != NULL) { AG_WidgetBindDouble(sl, "max", max); }
	return (sl);
}

AG_Slider *
AG_SliderNewDblR(void *parent, enum ag_slider_type type, Uint flags,
    double *val, double min, double max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindDouble(sl, "value", val); }
	AG_WidgetSetDouble(sl, "min", min);
	AG_WidgetSetDouble(sl, "max", max);
	return (sl);
}

#ifdef HAVE_LONG_DOUBLE
AG_Slider *
AG_SliderNewLongDbl(void *parent, enum ag_slider_type type, Uint flags,
    long double *val, long double *min, long double *max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindLongDouble(sl, "value", val); }
	if (min != NULL) { AG_WidgetBindLongDouble(sl, "min", min); }
	if (max != NULL) { AG_WidgetBindLongDouble(sl, "max", max); }
	return (sl);
}

AG_Slider *
AG_SliderNewLongDblR(void *parent, enum ag_slider_type type, Uint flags,
    long double *val, long double min, long double max)
{
	AG_Slider *sl = AG_SliderNew(parent, type, flags);
	if (val != NULL) { AG_WidgetBindLongDouble(sl, "value", val); }
	AG_WidgetSetLongDouble(sl, "min", min);
	AG_WidgetSetLongDouble(sl, "max", max);
	return (sl);
}
#endif /* HAVE_LONG_DOUBLE */

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
	AG_WidgetBinding *bMin, *bMax, *bVal;
	void *pMin, *pMax, *pVal;

	bVal = AG_WidgetGetBinding(sl, "value", &pVal);
	bMin = AG_WidgetGetBinding(sl, "min", &pMin);
	bMax = AG_WidgetGetBinding(sl, "max", &pMax);

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
#ifdef HAVE_64BIT
	case AG_WIDGET_UINT64:	GET_POSITION(Uint64);	break;
	case AG_WIDGET_SINT64:	GET_POSITION(Sint64);	break;
#endif
#ifdef HAVE_LONG_DOUBLE
	case AG_WIDGET_LONG_DOUBLE: GET_POSITION(long double);	break;
#endif
	default: *x = 0; break;
	} 
	AG_WidgetUnlockBinding(bMax);
	AG_WidgetUnlockBinding(bMin);
	AG_WidgetUnlockBinding(bVal);
	return (0);
fail:
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
	AG_WidgetBinding *bMin, *bMax, *bVal;
	void *pMin, *pMax, *pVal;

	bVal = AG_WidgetGetBinding(sl, "value", &pVal);
	bMin = AG_WidgetGetBinding(sl, "min", &pMin);
	bMax = AG_WidgetGetBinding(sl, "max", &pMax);

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
#ifdef HAVE_64BIT
	case AG_WIDGET_UINT64:	SEEK_TO_POSITION(Uint64);	break;
	case AG_WIDGET_SINT64:	SEEK_TO_POSITION(Sint64);	break;
#endif
#ifdef HAVE_LONG_DOUBLE
	case AG_WIDGET_LONG_DOUBLE: SEEK_TO_POSITION(long double); break;
#endif
	} 
	AG_WidgetBindingChanged(bVal);
	AG_PostEvent(NULL, sl, "slider-changed", NULL);
	AG_WidgetUnlockBinding(bMax);
	AG_WidgetUnlockBinding(bMin);
	AG_WidgetUnlockBinding(bVal);
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
	AG_WidgetBinding *bMin, *bMax, *bVal;
	void *pMin, *pMax, *pVal;

	bVal = AG_WidgetGetBinding(sl, "value", &pVal);
	bMin = AG_WidgetGetBinding(sl, "min", &pMin);
	bMax = AG_WidgetGetBinding(sl, "max", &pMax);

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
#ifdef HAVE_64BIT
	case AG_WIDGET_UINT64:	DECREMENT_INT(Uint64);	break;
	case AG_WIDGET_SINT64:	DECREMENT_INT(Sint64);	break;
#endif
#ifdef HAVE_LONG_DOUBLE
	case AG_WIDGET_LONG_DOUBLE: DECREMENT_REAL(long double); break;
#endif
	} 
	AG_WidgetBindingChanged(bVal);
	AG_PostEvent(NULL, sl, "slider-changed", NULL);
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
	AG_WidgetBinding *bMin, *bMax, *bVal;
	void *pMin, *pMax, *pVal;

	bVal = AG_WidgetGetBinding(sl, "value", &pVal);
	bMin = AG_WidgetGetBinding(sl, "min", &pMin);
	bMax = AG_WidgetGetBinding(sl, "max", &pMax);

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
#ifdef HAVE_64BIT
	case AG_WIDGET_UINT64:	INCREMENT_INT(Uint64);	break;
	case AG_WIDGET_SINT64:	INCREMENT_INT(Sint64);	break;
#endif
#ifdef HAVE_LONG_DOUBLE
	case AG_WIDGET_LONG_DOUBLE: INCREMENT_REAL(long double); break;
#endif
	} 
	AG_WidgetBindingChanged(bVal);
	AG_PostEvent(NULL, sl, "slider-changed", NULL);
	AG_WidgetUnlockBinding(bMax);
	AG_WidgetUnlockBinding(bMin);
	AG_WidgetUnlockBinding(bVal);
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
	}
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Slider *sl = AG_SELF();
	int button = AG_INT(1);
	int x = ((sl->type == AG_SLIDER_HORIZ) ? AG_INT(2) : AG_INT(3));
	int pos;

	if (button != SDL_BUTTON_LEFT) {
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
	case SDLK_UP:
	case SDLK_LEFT:
		Decrement(sl, 1);
		AG_DelTimeout(sl, &sl->incTo);
		AG_ScheduleTimeout(sl, &sl->decTo, agKbdDelay);
		break;
	case SDLK_DOWN:
	case SDLK_RIGHT:
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
	case SDLK_UP:
	case SDLK_LEFT:
		AG_DelTimeout(sl, &sl->decTo);
		break;
	case SDLK_DOWN:
	case SDLK_RIGHT:
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
	AG_WidgetBinding *bNew = AG_PTR(1);
	AG_WidgetBinding *bValue;
	void *pValue;

	/*
	 * Require that "min" and "max" be of the same binding type as
	 * "value" to avoid derelict hell. Mixing of types could always be
	 * implemented if some application really requires it.
	 */
	if (!strcmp(bNew->name, "min") || !strcmp(bNew->name, "max")) {
		bValue = AG_WidgetGetBinding(sl, "value", &pValue);
		if (bValue->type != bNew->type) {
			AG_FatalError("Slider \"%s\" binding type disagree "
			              "with \"value\" binding", bNew->name);
		}
		AG_WidgetUnlockBinding(bValue);
	}
}

static void
Init(void *obj)
{
	AG_Slider *sl = obj;

	WIDGET(sl)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP|
	                     AG_WIDGET_UNFOCUSED_MOTION|
			     AG_WIDGET_FOCUSABLE;

	AG_WidgetBindInt(sl, "value", &sl->value);
	AG_WidgetBindInt(sl, "min", &sl->min);
	AG_WidgetBindInt(sl, "max", &sl->max);

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

	AG_SetEvent(sl, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(sl, "window-mousebuttonup", MouseButtonUp, NULL);
	AG_SetEvent(sl, "window-mousemotion", MouseMotion, NULL);
	AG_SetEvent(sl, "window-keydown", KeyDown, NULL);
	AG_SetEvent(sl, "window-keyup", KeyUp, NULL);
	AG_SetEvent(sl, "widget-lostfocus", LostFocus, NULL);
	AG_SetEvent(sl, "widget-hidden", LostFocus, NULL);
	AG_SetEvent(sl, "widget-bound", BoundValue, NULL);

	AG_SetTimeout(&sl->incTo, IncrementTimeout, NULL, 0);
	AG_SetTimeout(&sl->decTo, DecrementTimeout, NULL, 0);
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
