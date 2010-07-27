/*
 * Copyright (c) 2002-2010 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "scrollbar.h"
#include "window.h"
#include "primitive.h"
#include "text.h"

#include "gui_math.h"

#define TOTSIZE(sb) (((sb)->type==AG_SCROLLBAR_VERT) ? HEIGHT(sb) : WIDTH(sb))

int agPrefScrollbarSize = 16;		/* Preferred width for new scrollbars */

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

/* Configure an initial length for the size requisition. */
void
AG_ScrollbarSizeHint(AG_Scrollbar *sb, int len)
{
	AG_ObjectLock(sb);
	sb->lenPre = len;
	AG_ObjectUnlock(sb);
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

#define GET_EXTENT(type) do {						\
	if (sb->flags & AG_SCROLLBAR_AUTOSIZE) {			\
		if ((*(type *)pMax - *(type *)pMin) != 0) {		\
			extent = sb->length -				\
			    (*(type *)pVis * sb->length /		\
			     (*(type *)pMax - *(type *)pMin));		\
		} else {						\
			extent = 0;					\
		}							\
	} else {							\
		extent = (sb->wBar == -1) ? 0 : (sb->length - sb->wBar); \
	}								\
} while (0)

#define GET_PX_COORDS(type) do {					\
	int extent;							\
	if (*(type *)pMin >= (*(type *)pMax - *(type *)pVis)) {		\
		goto fail;						\
	}								\
	GET_EXTENT(type);						\
	*x = (int)(((*(type *)pVal - *(type *)pMin) * extent) /		\
	          (*(type *)pMax - *(type *)pVis - *(type *)pMin));	\
	if (len != NULL) { 						\
		if (sb->flags & AG_SCROLLBAR_AUTOSIZE) {		\
			*len = *(type *)pVis * sb->length /		\
			    *(type *)pMax;				\
		} else {						\
			*len = (sb->wBar == -1) ? sb->length : sb->wBar; \
		}							\
	}								\
} while (0)

/*
 * Return the the position of the left/top edge, and the scrollbar
 * control size in pixels.
 * Returns 0 on success and -1 if the range is currently <= 0.
 */
static __inline__ int
GetPxCoords(AG_Scrollbar *sb, int *x, int *len)
{
	AG_Variable *bMin, *bMax, *bVis, *bVal;
	void *pMin, *pMax, *pVal, *pVis;

	*x = 0;
	if (len != NULL) { *len = 0; }

	bVal = AG_GetVariable(sb, "value", &pVal);
	bMin = AG_GetVariable(sb, "min", &pMin);
	bMax = AG_GetVariable(sb, "max", &pMax);
	bVis = AG_GetVariable(sb, "visible", &pVis);

	switch (AG_VARIABLE_TYPE(bVal)) {
	case AG_VARIABLE_INT:		GET_PX_COORDS(int);	break;
	case AG_VARIABLE_UINT:		GET_PX_COORDS(Uint);	break;
	case AG_VARIABLE_FLOAT:		GET_PX_COORDS(float);	break;
	case AG_VARIABLE_DOUBLE:	GET_PX_COORDS(double);	break;
	case AG_VARIABLE_UINT8:		GET_PX_COORDS(Uint8);	break;
	case AG_VARIABLE_SINT8:		GET_PX_COORDS(Sint8);	break;
	case AG_VARIABLE_UINT16:	GET_PX_COORDS(Uint16);	break;
	case AG_VARIABLE_SINT16:	GET_PX_COORDS(Sint16);	break;
	case AG_VARIABLE_UINT32:	GET_PX_COORDS(Uint32);	break;
	case AG_VARIABLE_SINT32:	GET_PX_COORDS(Sint32);	break;
	default:						break;
	} 
#if 0
	if (sb->wBar == -1) {
		*x = 0;
	}
#endif
	AG_UnlockVariable(bVis);
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bVal);
	return (0);
fail:
	AG_UnlockVariable(bVis);
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bVal);
	return (-1);
}
#undef GET_PX_COORDS

/*
 * Set the value from a specified position in pixels.
 */
#define MAP_PX_COORDS(type) do {					\
	int extent;							\
	GET_EXTENT(type);						\
	if (x <= 0) {							\
		*(type *)pVal = *(type *)pMin;				\
	} else if (x >= extent) {					\
		*(type *)pVal = MAX(*(type *)pMin,			\
		                    (*(type *)pMax - *(type *)pVis));	\
	} else {							\
		*(type *)pVal = x *					\
		    (*(type *)pMax - *(type *)pVis - *(type *)pMin) /	\
		    extent;						\
		*(type *)pVal += *(type *)pMin;				\
		if (*(type *)pVal < *(type *)pMin) {			\
			*(type *)pVal = *(type *)pMin;			\
		}							\
		if (*(type *)pVal > *(type *)pMax) {			\
			*(type *)pVal = *(type *)pMax;			\
		}							\
	}								\
} while (0)

static __inline__ void
SeekToPxCoords(AG_Scrollbar *sb, int x)
{
	AG_Variable *bMin, *bMax, *bVis, *bVal;
	void *pMin, *pMax, *pVal, *pVis;

	bVal = AG_GetVariable(sb, "value", &pVal);
	bMin = AG_GetVariable(sb, "min", &pMin);
	bMax = AG_GetVariable(sb, "max", &pMax);
	bVis = AG_GetVariable(sb, "visible", &pVis);

	switch (AG_VARIABLE_TYPE(bVal)) {
	case AG_VARIABLE_INT:		MAP_PX_COORDS(int);	break;
	case AG_VARIABLE_UINT:		MAP_PX_COORDS(Uint);	break;
	case AG_VARIABLE_FLOAT:		MAP_PX_COORDS(float);	break;
	case AG_VARIABLE_DOUBLE:	MAP_PX_COORDS(double);	break;
	case AG_VARIABLE_UINT8:		MAP_PX_COORDS(Uint8);	break;
	case AG_VARIABLE_SINT8:		MAP_PX_COORDS(Sint8);	break;
	case AG_VARIABLE_UINT16:	MAP_PX_COORDS(Uint16);	break;
	case AG_VARIABLE_SINT16:	MAP_PX_COORDS(Sint16);	break;
	case AG_VARIABLE_UINT32:	MAP_PX_COORDS(Uint32);	break;
	case AG_VARIABLE_SINT32:	MAP_PX_COORDS(Sint32);	break;
	default:						break;
	} 
	AG_PostEvent(NULL, sb, "scrollbar-changed", NULL);
	AG_UnlockVariable(bVis);
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bVal);
	AG_Redraw(sb);
}
#undef MAP_PX_COORDS

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
	AG_Variable *bMin, *bMax, *bVis, *bVal;
	void *pMin, *pMax, *pVal, *pVis;

	bVal = AG_GetVariable(sb, "value", &pVal);
	bMin = AG_GetVariable(sb, "min", &pMin);
	bMax = AG_GetVariable(sb, "max", &pMax);
	bVis = AG_GetVariable(sb, "visible", &pVis);

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
	default:					break;
	} 
	AG_PostEvent(NULL, sb, "scrollbar-changed", NULL);
	AG_UnlockVariable(bVis);
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bVal);
	AG_Redraw(sb);
}
#undef DECREMENT_INT
#undef DECREMENT_REAL

/*
 * Increment the value by the specified amount multiplied by iInc/rInc.
 */
#define INCREMENT_INT(type)						\
	if ((*(type *)pVal + ((type)sb->iInc)*v) >			\
	    (*(type *)pMax - *(type *)pVis)) 				\
		*(type *)pVal = *(type *)pMax - *(type *)pVis;		\
	else 								\
		*(type *)pVal += ((type)sb->iInc)*v

#define INCREMENT_REAL(type)						\
	if ((*(type *)pVal + ((type)v)*sb->rInc) >			\
	    (*(type *)pMax - *(type *)pVis)) 				\
		*(type *)pVal = *(type *)pMax - *(type *)pVis;		\
	else 								\
		*(type *)pVal += ((type)v)*sb->rInc

static void
Increment(AG_Scrollbar *sb, int v)
{
	AG_Variable *bMin, *bMax, *bVis, *bVal;
	void *pMin, *pMax, *pVal, *pVis;

	bVal = AG_GetVariable(sb, "value", &pVal);
	bMin = AG_GetVariable(sb, "min", &pMin);
	bMax = AG_GetVariable(sb, "max", &pMax);
	bVis = AG_GetVariable(sb, "visible", &pVis);

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
	default:					break;
	} 
	AG_PostEvent(NULL, sb, "scrollbar-changed", NULL);
	AG_UnlockVariable(bVis);
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bVal);
	AG_Redraw(sb);
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
	AG_Redraw(sb);
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();
	int button = AG_INT(1);
	int x = ((sb->type == AG_SCROLLBAR_HORIZ) ? AG_INT(2) : AG_INT(3)) -
	        sb->width;
	int pos, posFound, len;

	if (button != AG_MOUSE_LEFT) {
		return;
	}
	AG_WidgetFocus(sb);
	posFound = (GetPxCoords(sb, &pos, &len) == 0);
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
	} else if (x > TOTSIZE(sb) - sb->width*2) {
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
	} else {
		/*
		 * Click outside of scrollbar. We seek to the absolute position
		 * described by the cursor.
		 *
		 * XXX TODO: Provide an option to scroll progressively to the
		 * position since many users will expect that.
		 */
		sb->curBtn = AG_SCROLLBAR_BUTTON_SCROLL;
		sb->xOffs = len/2;
		SeekToPxCoords(sb, x - sb->xOffs);
	}
	AG_PostEvent(NULL, sb, "scrollbar-drag-begin", NULL);
	AG_Redraw(sb);
}

static void
MouseMotion(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();

	if (sb->curBtn != AG_SCROLLBAR_BUTTON_SCROLL) {
		return;
	}
	SeekToPxCoords(sb, ((sb->type == AG_SCROLLBAR_HORIZ) ?
	                    AG_INT(1):AG_INT(2)) - sb->width - sb->xOffs);
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
	case AG_KEY_UP:
	case AG_KEY_LEFT:
		Decrement(sb, 1);
		AG_DelTimeout(sb, &sb->incTo);
		AG_ScheduleTimeout(sb, &sb->decTo, agKbdDelay);
		break;
	case AG_KEY_DOWN:
	case AG_KEY_RIGHT:
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
	case AG_KEY_UP:
	case AG_KEY_LEFT:
		AG_DelTimeout(sb, &sb->decTo);
		break;
	case AG_KEY_DOWN:
	case AG_KEY_RIGHT:
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

static Uint32
AutoHideTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_Scrollbar *sb = obj;
	int x, len;

	if ((GetPxCoords(sb, &x, &len) == -1 ||
	    len == sb->length)) {
		if (AG_WidgetVisible(sb))
			AG_WidgetHide(sb);
	} else {
		if (!AG_WidgetVisible(sb))
			AG_WidgetShow(sb);
	}
	return (ival);
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
	AG_Variable *bNew = AG_PTR(1);
	AG_Variable *bValue;
	void *pValue;

	/*
	 * Require that "min", "max" and "visible" be of the same binding
	 * type as "value" to avoid derelict hell. Mixing of types could
	 * always be implemented if some application really requires it.
	 */
	if (!strcmp(bNew->name, "min") || !strcmp(bNew->name, "max") ||
	    !strcmp(bNew->name, "visible")) {
		bValue = AG_GetVariable(sb, "value", &pValue);
		if (bValue->type != bNew->type) {
			AG_FatalError("Scrollbar \"%s\" binding type disagree "
			              "with \"value\" binding", bNew->name);
		}
		AG_UnlockVariable(bValue);
	}
}

static void
OnAttach(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();
	
	if (sb->flags & AG_SCROLLBAR_AUTOHIDE)
		AG_ScheduleTimeout(sb, &sb->autohideTo, 250);
}

static void
OnDetach(AG_Event *event)
{
	AG_Scrollbar *sb = AG_SELF();
	
	AG_DelTimeout(sb, &sb->autohideTo);
}

static void
Init(void *obj)
{
	AG_Scrollbar *sb = obj;

	WIDGET(sb)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP|
	                     AG_WIDGET_UNFOCUSED_MOTION|
			     AG_WIDGET_FOCUSABLE;

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
	sb->lenPre = 32;

	sb->wBar = agPrefScrollbarSize/2;
	sb->width = agPrefScrollbarSize;
	sb->hArrow = sb->width*5/9;

	AG_SetEvent(sb, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(sb, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(sb, "mouse-motion", MouseMotion, NULL);
	AG_SetEvent(sb, "key-down", KeyDown, NULL);
	AG_SetEvent(sb, "key-up", KeyUp, NULL);
	AG_SetEvent(sb, "widget-lostfocus", LostFocus, NULL);
	AG_AddEvent(sb, "widget-hidden", LostFocus, NULL);
	AG_SetEvent(sb, "bound", BoundValue, NULL);
	AG_AddEvent(sb, "attached", OnAttach, NULL);
	AG_AddEvent(sb, "detached", OnDetach, NULL);

	AG_SetTimeout(&sb->scrollTo, ScrollTimeout, NULL, 0);
	AG_SetTimeout(&sb->decTo, DecrementTimeout, NULL, 0);
	AG_SetTimeout(&sb->incTo, IncrementTimeout, NULL, 0);
	AG_SetTimeout(&sb->autohideTo, AutoHideTimeout, NULL, 0);
	
	AG_BindInt(sb, "value", &sb->value);
	AG_BindInt(sb, "min", &sb->min);
	AG_BindInt(sb, "max", &sb->max);
	AG_BindInt(sb, "visible", &sb->visible);

	AG_RedrawOnChange(sb, 100, "value");
	AG_RedrawOnChange(sb, 250, "min");
	AG_RedrawOnChange(sb, 250, "max");
	AG_RedrawOnChange(sb, 250, "visible");

#ifdef AG_DEBUG
	AG_BindUint(sb, "flags", &sb->flags);
	AG_BindInt(sb, "width", &sb->width);
	AG_BindInt(sb, "length", &sb->length);
	AG_BindInt(sb, "wBar", &sb->wBar);
	AG_BindInt(sb, "hArrow", &sb->hArrow);
	AG_BindInt(sb, "xOffs", &sb->xOffs);
#endif /* AG_DEBUG */
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Scrollbar *sb = obj;

	switch (sb->type) {
	case AG_SCROLLBAR_HORIZ:
		r->w = sb->width*2 + sb->lenPre;
		r->h = sb->width;
		break;
	case AG_SCROLLBAR_VERT:
		r->w = sb->width;
		r->h = sb->width*2 + sb->lenPre;
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
	sb->length = ((sb->type==AG_SCROLLBAR_VERT) ? a->h:a->w) - sb->width*2;
	if (sb->length < 0) { sb->length = 0; }
	return (0);
}

static void
DrawText(AG_Scrollbar *sb)
{
	AG_Driver *drv = WIDGET(sb)->drv;
	AG_Surface *txt;
	char label[32];
	AG_Rect r;

	AG_PushTextState();
	AG_TextColor(agColors[TEXT_COLOR]);
	AG_TextBGColor(AG_ColorRGB(127,127,127));

	Snprintf(label, sizeof(label),
	    (sb->type == AG_SCROLLBAR_HORIZ) ?
	    "%d|%d|%d|%d" : "%d\n:%d\n%d\nv%d\n",
	    AG_GetInt(sb,"min"),
	    AG_GetInt(sb,"value"),
	    AG_GetInt(sb,"max"),
	    AG_GetInt(sb,"visible"));

	txt = AG_TextRender(label);		/* XXX inefficient */
	r = AG_RECT(
	    WIDTH(sb)/2 - txt->w/2,
	    HEIGHT(sb)/2 - txt->h/2,
	    txt->w,
	    txt->h);
	AG_WidgetBlit(sb, txt, r.x, r.y);
	AG_SurfaceFree(txt);

	AG_RectTranslate(&r, WIDGET(sb)->rView.x1, WIDGET(sb)->rView.y1);

	if (AGDRIVER_CLASS(drv)->updateRegion != NULL) {
		AGDRIVER_CLASS(drv)->updateRegion(drv, r);
	}
	AG_PopTextState();
}

static void
Draw(void *obj)
{
	AG_Scrollbar *sb = obj;
	int x, len;

	if (GetPxCoords(sb, &x, &len) == -1) {
		x = 0;
		len = sb->length;
	}
	if (sb->flags & AG_SCROLLBAR_AUTOHIDE && len == sb->length)
		return;

	switch (sb->type) {
	case AG_SCROLLBAR_VERT:
		STYLE(sb)->ScrollbarVert(sb, x, len);
		break;
	case AG_SCROLLBAR_HORIZ:
		STYLE(sb)->ScrollbarHoriz(sb, x, len);
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
	rv = (GetPxCoords(sb, &x, NULL) == -1) ? 0 : 1;
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
