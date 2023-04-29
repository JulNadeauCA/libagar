/*
 * Copyright (c) 2002-2023 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Scrollbar control.  Often used for panning / scrolling.  It can connect
 * to integer or floating-point variables representing a minimum, maximum,
 * current offset, and number of visible items.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/scrollbar.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>
#include <agar/gui/text.h>
#include <agar/gui/gui_math.h>
#include <agar/gui/box.h>
#include <agar/gui/label.h>
#include <agar/gui/radio.h>
#include <agar/gui/separator.h>
#include <agar/gui/checkbox.h>

#ifndef AG_SCROLLBAR_HOT
#define AG_SCROLLBAR_HOT 10  /* Increase control bar contrast below threshold */
#endif

static const int agScrollbarZoomSizes[AG_ZOOM_MAX] = {
	 8,  8,  8,  8,  10, 10, 10, 10,
	12, 12, 14, 14,  16, 16, 18, 18,
	18, 18, 20, 20,  20, 20, 22, 22,
	22, 22, 24, 24,  26, 28, 32, 34
};

#define SBPOS(sb,x,y) (((sb)->type == AG_SCROLLBAR_HORIZ) ? (x) : (y))
#define SBLEN(sb)     (((sb)->type == AG_SCROLLBAR_HORIZ) ? WIDTH(sb) : HEIGHT(sb))
#define SBTHICK(sb)   (((sb)->type == AG_SCROLLBAR_HORIZ) ? HEIGHT(sb) : WIDTH(sb))

static int  GetPxCoords(AG_Scrollbar *_Nonnull, int *_Nonnull, int *_Nonnull);
static void SeekToPxCoords(AG_Scrollbar *_Nonnull, int);
static int  GetPxCoordsGeneral(AG_Scrollbar *_Nonnull, int *_Nonnull, int *_Nonnull, AG_VariableType, void *_Nonnull, void *_Nonnull, void *_Nonnull, void *_Nonnull);
static void SeekToPxCoordsGeneral(AG_Scrollbar *_Nonnull, int, AG_VariableType, void *_Nonnull, void *_Nonnull, void *_Nonnull, void *_Nonnull);
static int  IncrementGeneral(AG_Scrollbar *_Nonnull, void *_Nonnull, AG_VariableType, int, void *_Nonnull, void *_Nonnull, void *_Nonnull, void *_Nonnull);
static void OnShowGeneral(AG_Scrollbar *_Nonnull, const AG_Event *_Nonnull, const AG_Variable *_Nonnull);
static void DrawHoriz(AG_Scrollbar *_Nonnull, int, int);
static void DrawVert(AG_Scrollbar *_Nonnull, int, int);
static void DrawHorizUndersize(AG_Scrollbar *_Nonnull);
static void DrawVertUndersize(AG_Scrollbar *_Nonnull);

AG_Scrollbar *
AG_ScrollbarNewHoriz(void *parent, Uint flags)
{
	return AG_ScrollbarNew(parent, AG_SCROLLBAR_HORIZ, flags);
}

AG_Scrollbar *
AG_ScrollbarNewVert(void *parent, Uint flags)
{
	return AG_ScrollbarNew(parent, AG_SCROLLBAR_VERT, flags);
}

/* Set an alternate handler for UP/LEFT button click. */
AG_Scrollbar *
AG_ScrollbarNew(void *parent, enum ag_scrollbar_type type, Uint flags)
{
	AG_Scrollbar *sb;

	sb = Malloc(sizeof(AG_Scrollbar));
	AG_ObjectInit(sb, &agScrollbarClass);

	sb->type = type;

	if (flags & AG_SCROLLBAR_HFILL) { WIDGET(sb)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_SCROLLBAR_VFILL) { WIDGET(sb)->flags |= AG_WIDGET_VFILL; }
	sb->flags |= flags;

	AG_ObjectAttach(parent, sb);
	return (sb);
}

/* Set an alternate handler for Increment (UP or LEFT button). */
void
AG_ScrollbarSetIncFn(AG_Scrollbar *sb, AG_EventFn fn, const char *fmt, ...)
{
	AG_OBJECT_ISA(sb, "AG_Widget:AG_Scrollbar:*");
	AG_ObjectLock(sb);

	if (fn) {
		sb->buttonIncFn = AG_SetEvent(sb, NULL, fn, NULL);
		if (fmt) {
			va_list ap;

			va_start(ap, fmt);
			AG_EventGetArgs(sb->buttonIncFn, fmt, ap);
			va_end(ap);
		}
	} else {
		sb->buttonIncFn = NULL;
	}

	AG_ObjectUnlock(sb);
}

/* Set an alternate handler for Decrement (DOWN or RIGHT button). */
void
AG_ScrollbarSetDecFn(AG_Scrollbar *sb, AG_EventFn fn, const char *fmt, ...)
{
	AG_OBJECT_ISA(sb, "AG_Widget:AG_Scrollbar:*");
	AG_ObjectLock(sb);

	if (fn) {
		sb->buttonDecFn = AG_SetEvent(sb, NULL, fn, NULL);
		if (fmt) {
			va_list ap;

			va_start(ap, fmt);
			AG_EventGetArgs(sb->buttonDecFn, fmt, ap);
			va_end(ap);
		}
	} else {
		sb->buttonDecFn = NULL;
	}

	AG_ObjectUnlock(sb);
}

static void
MouseMotion(void *obj, int mx, int my, int dx, int dy)
{
	AG_Scrollbar *sb = obj;
	const int sbThick = SBTHICK(sb);
	const int x = SBPOS(sb,mx,my) - sbThick;
	enum ag_scrollbar_button mouseOverBtn;

	if (sb->curBtn == AG_SCROLLBAR_BUTTON_SCROLL) {
		SeekToPxCoords(sb, x - sb->xOffs);
		AG_Redraw(sb);
	} else if (AG_WidgetRelativeArea(sb, mx,my)) {
		if (x < 0) {
			mouseOverBtn = AG_SCROLLBAR_BUTTON_DEC;
		} else if (x > SBLEN(sb) - (sbThick << 1)) {
			mouseOverBtn = AG_SCROLLBAR_BUTTON_INC;
		} else {
			int pos, len;
	
			if (GetPxCoords(sb, &pos, &len) == -1 || /* No range */
			    (x >= pos && x <= pos+len)) {
				mouseOverBtn = AG_SCROLLBAR_BUTTON_SCROLL;
			} else {
				mouseOverBtn = AG_SCROLLBAR_BUTTON_NONE;
				if (sb->xSeek != -1)
					sb->xSeek = x;
			}
		}
		if (mouseOverBtn != sb->mouseOverBtn) {
			sb->mouseOverBtn = mouseOverBtn;
			AG_Redraw(sb);
		}
	} else {
		if (sb->mouseOverBtn != AG_SCROLLBAR_BUTTON_NONE) {
			sb->mouseOverBtn = AG_SCROLLBAR_BUTTON_NONE;
			AG_Redraw(sb);
		}
	}
}

/* Compute the total effective scrollable range in pixels. */
#define GET_EXTENT_PX(TYPE) {						\
	if (vis > 0) {							\
		if ((max-min) > 0) {					\
			extentPx = sbLen - (int)(vis*sbLen/(max-min));	\
		} else {						\
			extentPx = 0;					\
		}							\
	} else {							\
		extentPx = sbLen - sbThick;				\
	}								\
}

/*
 * GetPxCoords(): Compute control bar position in pixels, given the
 * current value ("value") and range ("min", "max", "visible").
 */
#define GET_PX_COORDS(TYPE) {						\
	TYPE min = *(TYPE *)pMin;					\
	TYPE max = *(TYPE *)pMax;					\
	TYPE vis = *(TYPE *)pVis;					\
	int extentPx, divPx;						\
									\
	if (min >= (max - vis)) {					\
		rv = -1;						\
		goto out;						\
	}								\
	GET_EXTENT_PX(TYPE);						\
	if ((divPx = (int)(max - vis - min)) < 1) {			\
		rv = -1;						\
		goto out;						\
	}								\
	*x = (int)(((*(TYPE *)pVal - min) * extentPx) / divPx);		\
	*len = (vis > 0) ? (vis * sbLen / (max-min)) : sbThick;		\
}
static int
GetPxCoords(AG_Scrollbar *_Nonnull sb, int *_Nonnull x, int *_Nonnull len)
{
	AG_Variable *bVal, *bMin, *bMax, *bVis;
	void        *pVal, *pMin, *pMax, *pVis;
	const int sbThick = SBTHICK(sb);
	const int sbLen = SBLEN(sb) - (sbThick << 1);
	AG_VariableType bvType;
	int rv = 0;

	*x = 0;
	*len = 0;

	bVal = AG_GetVariable(sb, "value", &pVal);
	bMin = AG_GetVariable(sb, "min", &pMin);
	bMax = AG_GetVariable(sb, "max", &pMax);
	bVis = AG_GetVariable(sb, "visible", &pVis);

	bvType = AG_VARIABLE_TYPE(bVal);
	switch (bvType) {
	case AG_VARIABLE_INT:
		GET_PX_COORDS(int);
		break;
	case AG_VARIABLE_UINT:
		GET_PX_COORDS(Uint);
		break;
	default:
		rv = GetPxCoordsGeneral(sb, x,len, bvType, pVal, pMin,pMax,pVis);
		break;
	}
out:
	AG_UnlockVariable(bVis);
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bVal);
	return (rv);
}

/*
 * Map specified pixel coordinates to a value.
 */
#define MAP_PX_COORDS(TYPE) {						\
	TYPE min = *(TYPE *)pMin;					\
	TYPE max = *(TYPE *)pMax;					\
	TYPE vis = *(TYPE *)pVis;					\
	int extentPx;							\
									\
	if (*(TYPE *)pMax > *(TYPE *)pVis) {				\
		GET_EXTENT_PX(TYPE);					\
		if (x <= 0) {						\
			*(TYPE *)pVal = min;				\
		} else if (x >= extentPx) {				\
			*(TYPE *)pVal = MAX(min, (max-vis));		\
		} else {						\
			*(TYPE *)pVal = min + x*(max-vis-min)/extentPx;	\
									\
			if (*(TYPE *)pVal < min)			\
				*(TYPE *)pVal = min; 			\
			if (*(TYPE *)pVal > max)			\
				*(TYPE *)pVal = max;			\
		}							\
	}								\
}
static void
SeekToPxCoords(AG_Scrollbar *_Nonnull sb, int x)
{
	AG_Variable *bVal, *bMin, *bMax, *bVis;
	void        *pVal, *pMin, *pMax, *pVis;
	const int sbThick = SBTHICK(sb);
	const int sbLen = SBLEN(sb) - (sbThick << 1);
	AG_VariableType bvType;

	bVal = AG_GetVariable(sb, "value", &pVal);
	bMin = AG_GetVariable(sb, "min", &pMin);
	bMax = AG_GetVariable(sb, "max", &pMax);
	bVis = AG_GetVariable(sb, "visible", &pVis);

	bvType = AG_VARIABLE_TYPE(bVal);
	switch (bvType) {
	case AG_VARIABLE_INT:
		MAP_PX_COORDS(int);
		break;
	case AG_VARIABLE_UINT:
		MAP_PX_COORDS(Uint);
		break;
	default:
		SeekToPxCoordsGeneral(sb, x, bvType, pVal,pMin,pMax,pVis);
		break;
	}

	AG_PostEvent(sb, "scrollbar-changed", NULL);

	AG_UnlockVariable(bVis);
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bVal);
}

static int
GetPxCoordsGeneral(AG_Scrollbar *_Nonnull sb, int *_Nonnull x, int *_Nonnull len,
    AG_VariableType bvType, void *_Nonnull pVal,
    void *_Nonnull pMin, void *_Nonnull pMax, void *_Nonnull pVis)
{
	const int sbThick = SBTHICK(sb);
	const int sbLen = SBLEN(sb) - (sbThick << 1);
	int rv = 0;

	switch (bvType) {
	case AG_VARIABLE_FLOAT:   GET_PX_COORDS(float);   break;
	case AG_VARIABLE_DOUBLE:  GET_PX_COORDS(double);  break;
	case AG_VARIABLE_UINT8:   GET_PX_COORDS(Uint8);   break;
	case AG_VARIABLE_SINT8:   GET_PX_COORDS(Sint8);   break;
	case AG_VARIABLE_UINT16:  GET_PX_COORDS(Uint16);  break;
	case AG_VARIABLE_SINT16:  GET_PX_COORDS(Sint16);  break;
	case AG_VARIABLE_UINT32:  GET_PX_COORDS(Uint32);  break;
	case AG_VARIABLE_SINT32:  GET_PX_COORDS(Sint32);  break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:  GET_PX_COORDS(Uint64);  break;
	case AG_VARIABLE_SINT64:  GET_PX_COORDS(Sint64);  break;
#endif
	default:
		break;
	}
out:
	return (rv);
}
#undef GET_PX_COORDS

static void
SeekToPxCoordsGeneral(AG_Scrollbar *_Nonnull sb, int x,
    AG_VariableType bvType, void *_Nonnull pVal,
    void *_Nonnull pMin, void *_Nonnull pMax, void *_Nonnull pVis)
{
	const int sbThick = SBTHICK(sb);
	const int sbLen = SBLEN(sb) - (sbThick << 1);
	
	switch (bvType) {
	case AG_VARIABLE_FLOAT:   MAP_PX_COORDS(float);   break;
	case AG_VARIABLE_DOUBLE:  MAP_PX_COORDS(double);  break;
	case AG_VARIABLE_UINT8:   MAP_PX_COORDS(Uint8);   break;
	case AG_VARIABLE_SINT8:   MAP_PX_COORDS(Sint8);   break;
	case AG_VARIABLE_UINT16:  MAP_PX_COORDS(Uint16);  break;
	case AG_VARIABLE_SINT16:  MAP_PX_COORDS(Sint16);  break;
	case AG_VARIABLE_UINT32:  MAP_PX_COORDS(Uint32);  break;
	case AG_VARIABLE_SINT32:  MAP_PX_COORDS(Sint32);  break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:  MAP_PX_COORDS(Uint64);  break;
	case AG_VARIABLE_SINT64:  MAP_PX_COORDS(Sint64);  break;
#endif
	default:
		break;
	} 
}
#undef MAP_PX_COORDS

/*
 * Increment(): Based on direction, increment/decrement the target
 * of the "value" binding.
 */
#define INCREMENT(TYPE)	{						\
	if (*(TYPE *)pMax > *(TYPE *)pVis) {				\
		if (direction == +1) {					\
			if ((*(TYPE *)pVal + *(TYPE *)pInc) >		\
			    (*(TYPE *)pMax - (*(TYPE *)pVis))) {	\
				*(TYPE *)pVal = (*(TYPE *)pMax) -	\
				                (*(TYPE *)pVis);	\
				rv = 1;					\
			} else { 					\
				*(TYPE *)pVal += *(TYPE *)pInc;		\
			}						\
		} else {						\
			if (*(TYPE *)pVal < *(TYPE *)pMin +		\
			                    *(TYPE *)pInc) {		\
				*(TYPE *)pVal = *(TYPE *)pMin;		\
				rv = 1;					\
			} else { 					\
				*(TYPE *)pVal -= *(TYPE *)pInc;		\
			}						\
		}							\
	}								\
}
static int
Increment(AG_Scrollbar *_Nonnull sb, int direction)
{
	AG_Variable *bVal, *bMin, *bMax, *bInc, *bVis;
	void        *pVal, *pMin, *pMax, *pInc, *pVis;
	AG_VariableType bvType;
	int rv = 0;

	bVal = AG_GetVariable(sb, "value", &pVal);
	bMin = AG_GetVariable(sb, "min", &pMin);
	bMax = AG_GetVariable(sb, "max", &pMax);
	bInc = AG_GetVariable(sb, "inc", &pInc);
	bVis = AG_GetVariable(sb, "visible", &pVis);
	bvType = AG_VARIABLE_TYPE(bVal);

	switch (bvType) {
	case AG_VARIABLE_INT:
		INCREMENT(int);
		break;
	case AG_VARIABLE_UINT:
		INCREMENT(Uint);
		break;
	default:
		rv = IncrementGeneral(sb, pVal, bvType, direction,
		                      pMin, pMax, pInc, pVis);
	} 
	
	AG_UnlockVariable(bVis);
	AG_UnlockVariable(bInc);
	AG_UnlockVariable(bMax);
	AG_UnlockVariable(bMin);
	AG_UnlockVariable(bVal);

	AG_PostEvent(sb, "scrollbar-changed", NULL);
	AG_Redraw(sb);
	return (rv);
}
static int
IncrementGeneral(AG_Scrollbar *_Nonnull sb, void *_Nonnull pVal,
    AG_VariableType bvType, int direction, void *_Nonnull pMin,
    void *_Nonnull pMax, void *_Nonnull pInc, void *_Nonnull pVis)
{
	int rv = 0;

	switch (bvType) {
	case AG_VARIABLE_FLOAT:	  INCREMENT(float);	break;
	case AG_VARIABLE_DOUBLE:  INCREMENT(double);	break;
	case AG_VARIABLE_UINT8:   INCREMENT(Uint8);	break;
	case AG_VARIABLE_SINT8:   INCREMENT(Sint8);	break;
	case AG_VARIABLE_UINT16:  INCREMENT(Uint16);	break;
	case AG_VARIABLE_SINT16:  INCREMENT(Sint16);	break;
	case AG_VARIABLE_UINT32:  INCREMENT(Uint32);	break;
	case AG_VARIABLE_SINT32:  INCREMENT(Sint32);	break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64:  INCREMENT(Uint64);	break;
	case AG_VARIABLE_SINT64:  INCREMENT(Sint64);	break;
#endif
	default: break;
	}
	return (rv);
}
#undef INCREMENT

static void
MouseButtonUp(void *obj, AG_MouseButton button, int x, int y)
{
	AG_Scrollbar *sb = obj;

	AG_DelTimer(sb, &sb->moveTo);

	switch (sb->curBtn) {
	case AG_SCROLLBAR_BUTTON_DEC:
		if (sb->buttonDecFn) {
			AG_PostEventByPtr(sb, sb->buttonDecFn, "%i", 0);
		}
		break;
	case AG_SCROLLBAR_BUTTON_INC:
		if (sb->buttonIncFn)
			AG_PostEventByPtr(sb, sb->buttonIncFn, "%i", 0);
		break;
	default:
		break;
	}

	sb->curBtn = AG_SCROLLBAR_BUTTON_NONE;
	sb->xOffs = 0;
	AG_PostEvent(sb, "scrollbar-drag-end", NULL);
	AG_Redraw(sb);
}

/* Timer for scrolling controlled by buttons (mouse spin setting). */
static Uint32
MoveButtonsTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Scrollbar *sb = AG_SCROLLBAR_SELF();
	const int dir = AG_INT(1);
	int rv;

	rv = Increment(sb, dir);

	if (sb->xSeek != -1) {
		int pos, len;
		
		if (GetPxCoords(sb, &pos, &len) == -1 ||
		    ((dir == -1 && sb->xSeek >= pos) ||
		     (dir == +1 && sb->xSeek <= pos+len))) {
			sb->curBtn = AG_SCROLLBAR_BUTTON_SCROLL;
			sb->xSeek = -1;
			if (dir == +1) { sb->xOffs = (len >> 1); }
			return (0);
		}
	}
	return (rv != 1) ? agMouseScrollIval : 0;
}

/* Timer for scrolling controlled by keyboard (keyrepeat setting). */
static Uint32
MoveKbdTimeout(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Scrollbar *sb = AG_SCROLLBAR_SELF();
	const int dir = AG_INT(1);

	return (Increment(sb, dir) != 1) ? agKbdRepeat : 0;
}

static void
MouseButtonDown(void *obj, AG_MouseButton button, int mx, int my)
{
	AG_Scrollbar *sb = obj;
	const int sbThick = SBTHICK(sb);
	const int x = SBPOS(sb,mx,my) - sbThick;
	int posCur, len = SBLEN(sb);

	switch (button) {
	case AG_MOUSE_WHEELUP:
		Increment(sb, -1);
		return;
	case AG_MOUSE_WHEELDOWN:
		Increment(sb, +1);
		return;
	case AG_MOUSE_LEFT:
		break;
	default:
		return;
	}

	if (!AG_WidgetIsFocused(sb)) {
		AG_WidgetFocus(sb);
	}
	if (x < 0) {						/* Decrement */
		sb->curBtn = AG_SCROLLBAR_BUTTON_DEC;
		if (sb->buttonDecFn) {
			AG_PostEventByPtr(sb, sb->buttonDecFn, "%i", 1);
		} else {
			if (Increment(sb,-1) != 1) {
				sb->xSeek = -1;
				AG_AddTimer(sb, &sb->moveTo, agScrollButtonIval,
				    MoveButtonsTimeout,"%i",-1);
			}
		}
	} else if (x > len - (sbThick << 1)) {			/* Increment */
		sb->curBtn = AG_SCROLLBAR_BUTTON_INC;
		if (sb->buttonIncFn) {
			AG_PostEventByPtr(sb, sb->buttonIncFn, "%i", 1);
		} else {
			if (Increment(sb,+1) != 1) {
				sb->xSeek = -1;
				AG_AddTimer(sb, &sb->moveTo, agScrollButtonIval,
				    MoveButtonsTimeout,"%i",+1);
			}
		}
	} else {
		if (GetPxCoords(sb, &posCur, &len) == -1) {	/* No range */
			sb->curBtn = AG_SCROLLBAR_BUTTON_SCROLL;
			sb->xOffs = x;
		} else if (x >= posCur && x <= posCur+len) {
			sb->curBtn = AG_SCROLLBAR_BUTTON_SCROLL;
			sb->xOffs = (x - posCur);
		} else {
			if (x < posCur) {
				if (sb->flags & AG_SCROLLBAR_SMOOTH) {
					sb->curBtn = AG_SCROLLBAR_BUTTON_DEC;
					if (Increment(sb,-1) != 1) {
						sb->xSeek = x;
						AG_AddTimer(sb, &sb->moveTo,
						    agScrollButtonIval,
						    MoveButtonsTimeout,"%i,",-1);
					}
				} else {
					SeekToPxCoords(sb, x - (len >> 1));
					sb->curBtn = AG_SCROLLBAR_BUTTON_SCROLL;
					sb->xOffs = (len >> 1);
				}
			} else {
				if (sb->flags & AG_SCROLLBAR_SMOOTH) {
					sb->curBtn = AG_SCROLLBAR_BUTTON_INC;
					if (Increment(sb,+1) != 1) {
						sb->xSeek = x;
						AG_AddTimer(sb, &sb->moveTo,
						    agScrollButtonIval,
						    MoveButtonsTimeout,"%i",+1);
					}
				} else {
					SeekToPxCoords(sb, x - (len >> 1));
					sb->curBtn = AG_SCROLLBAR_BUTTON_SCROLL;
					sb->xOffs = (len >> 1);
				}
			}
		}
	}
	AG_PostEvent(sb, "scrollbar-drag-begin", NULL);
	AG_Redraw(sb);
}

static void
KeyDown(void *obj, AG_KeySym ks, AG_KeyMod kmod, AG_Char ch)
{
	AG_Scrollbar *sb = obj;

	switch (ks) {
	case AG_KEY_UP:
	case AG_KEY_LEFT:
		if (Increment(sb,-1) != 1) {
			AG_AddTimer(sb, &sb->moveTo, agKbdDelay,
			    MoveKbdTimeout,"%i",-1);
		}
		break;
	case AG_KEY_DOWN:
	case AG_KEY_RIGHT:
		if (Increment(sb,+1) != 1) {
			AG_AddTimer(sb, &sb->moveTo, agKbdDelay,
			    MoveKbdTimeout,"%i",+1);
		}
		break;
	default:
		break;
	}
}

static void
KeyUp(void *obj, AG_KeySym ks, AG_KeyMod kmod, AG_Char ch)
{
	AG_Scrollbar *sb = obj;
	
	switch (ks) {
	case AG_KEY_UP:
	case AG_KEY_LEFT:
	case AG_KEY_DOWN:
	case AG_KEY_RIGHT:
		AG_DelTimer(sb, &sb->moveTo);
		break;
	default:
		break;
	}
}


static void
Ctrl(void *obj, void *inputDevice, const AG_DriverEvent *dev)
{
	AG_Scrollbar *sb = obj;
	
	switch (dev->type) {
	case AG_DRIVER_CTRL_AXIS_MOTION:
		if (dev->ctrlAxis.axis == AG_CTRL_AXIS_LEFT_Y &&
		    sb->type == AG_SCROLLBAR_VERT)
		{
			if (dev->ctrlAxis.value == -0x8000 &&
			    Increment(sb,-1) != 1) {
				AG_AddTimer(sb, &sb->moveTo, agKbdDelay,
				    MoveKbdTimeout,"%i",-1);
			} else if (dev->ctrlAxis.value == 0x7fff &&
			    Increment(sb,+1) != 1) {
				AG_AddTimer(sb, &sb->moveTo, agKbdDelay,
				    MoveKbdTimeout,"%i",+1);
			} else {
				AG_DelTimer(sb, &sb->moveTo);
			}
		} else if (dev->ctrlAxis.axis == AG_CTRL_AXIS_LEFT_X &&
		           sb->type == AG_SCROLLBAR_HORIZ)
		{
			if (dev->ctrlAxis.value == -0x8000 &&
			    Increment(sb,-1) != 1) {
				AG_AddTimer(sb, &sb->moveTo, agKbdDelay,
				    MoveKbdTimeout,"%i",-1);
			} else if (dev->ctrlAxis.value == 0x7fff &&
			    Increment(sb,+1) != 1) {
				AG_AddTimer(sb, &sb->moveTo, agKbdDelay,
				    MoveKbdTimeout,"%i",+1);
			} else {
				AG_DelTimer(sb, &sb->moveTo);
			}
		}
		break;
	default:
		break;
	}
}

static void
OnFocusLoss(AG_Event *_Nonnull event)
{
	AG_Scrollbar *sb = AG_SCROLLBAR_SELF();

	AG_DelTimer(sb, &sb->moveTo);
}

static void
OnFontChange(AG_Event *_Nonnull event)
{
	AG_Scrollbar *sb = AG_SCROLLBAR_SELF();

	if (sb->lbl1 != -1) {
		AG_WidgetUnmapSurface(sb, sb->lbl1);
		sb->lbl1 = -1;
	}
	if (sb->lbl2 != -1) {
		AG_WidgetUnmapSurface(sb, sb->lbl2);
		sb->lbl2 = -1;
	}
}

#define SET_DEF(fn,dmin,dmax,dinc) { 					\
	if (!AG_Defined(sb, "min")) { fn(sb, "min", dmin); }		\
	if (!AG_Defined(sb, "max")) { fn(sb, "max", dmax); }		\
	if (!AG_Defined(sb, "inc")) { fn(sb, "inc", dinc); }		\
	if (!AG_Defined(sb, "visible")) { fn(sb, "visible", 0); }	\
}
static void
OnShow(AG_Event *_Nonnull event)
{
	AG_Scrollbar *sb = AG_SCROLLBAR_SELF();
	AG_Variable *V;

	if ((V = AG_AccessVariable(sb, "value")) == NULL) {
		V = AG_BindInt(sb, "value", &sb->value);
	}
	switch (AG_VARIABLE_TYPE(V)) {
	case AG_VARIABLE_INT:
		SET_DEF(AG_SetInt, 0, AG_INT_MAX-1, 1);
		break;
	case AG_VARIABLE_UINT:
		SET_DEF(AG_SetUint, 0U, AG_UINT_MAX-1, 1U);
		break;
	default:
		OnShowGeneral(sb, event, V);
		break;
	}
	AG_UnlockVariable(V);

	if ((sb->flags & AG_SCROLLBAR_EXCL) == 0) {
		/* Trigger redraw upon external changes to the bindings. */
		AG_RedrawOnChange(sb, 500, "value");
		AG_RedrawOnChange(sb, 500, "min");
		AG_RedrawOnChange(sb, 500, "max");
		AG_RedrawOnChange(sb, 500, "visible");
	}
}
static void
OnShowGeneral(AG_Scrollbar *_Nonnull sb, const AG_Event *_Nonnull event,
    const AG_Variable *_Nonnull V)
{
	switch (AG_VARIABLE_TYPE(V)) {
	case AG_VARIABLE_FLOAT:	 SET_DEF(AG_SetFloat,  0.0f, 1.0f, 0.1f); break;
	case AG_VARIABLE_DOUBLE: SET_DEF(AG_SetDouble, 0.0,  1.0,  0.1); break;
	case AG_VARIABLE_UINT8:  SET_DEF(AG_SetUint8,  0U, 0xffU, 1U); break;
	case AG_VARIABLE_SINT8:  SET_DEF(AG_SetSint8,  0,  0x7f,  1); break;
	case AG_VARIABLE_UINT16: SET_DEF(AG_SetUint16, 0U,  0xffffU, 1U); break;
	case AG_VARIABLE_SINT16: SET_DEF(AG_SetSint16, 0,   0x7fff,  1); break;
	case AG_VARIABLE_UINT32: SET_DEF(AG_SetUint32, 0UL, 0xffffffffUL, 1UL); break;
	case AG_VARIABLE_SINT32: SET_DEF(AG_SetSint32, 0L,  0x7fffffffL,  1L); break;
#ifdef HAVE_64BIT
	case AG_VARIABLE_UINT64: SET_DEF(AG_SetUint64, 0ULL, 0xffffffffffffffffULL, 1ULL); break;
	case AG_VARIABLE_SINT64: SET_DEF(AG_SetSint64, 0LL,  0x7fffffffffffffffLL,  1LL); break;
#endif
	default: break;
	}
}
#undef SET_DEF

static void
OnHide(AG_Event *_Nonnull event)
{
	AG_Scrollbar *sb = AG_SCROLLBAR_SELF();
	
	AG_DelTimer(sb, &sb->moveTo);
}

static void
Init(void *_Nonnull obj)
{
	AG_Scrollbar *sb = obj;

	WIDGET(sb)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP |
	                     AG_WIDGET_UNFOCUSED_MOTION |
			     AG_WIDGET_FOCUSABLE |
			     AG_WIDGET_USE_MOUSEOVER |
			     AG_WIDGET_USE_TEXT;

	sb->type = AG_SCROLLBAR_HORIZ;
	sb->curBtn = AG_SCROLLBAR_BUTTON_NONE;
	sb->mouseOverBtn = AG_SCROLLBAR_BUTTON_NONE;
	sb->flags = 0;
	sb->lbl1 = -1;
	sb->lbl2 = -1;
	sb->value = 0;
	sb->buttonIncFn = NULL;
	sb->buttonDecFn = NULL;
	sb->xOffs = 0;
	sb->xSeek = -1;
	
	AG_AddEvent(sb, "widget-shown", OnShow, NULL);
	AG_AddEvent(sb, "widget-hidden", OnHide, NULL);
	AG_SetEvent(sb, "widget-lostfocus", OnFocusLoss, NULL);
	AG_AddEvent(sb, "font-changed", OnFontChange, NULL);

	AG_InitTimer(&sb->moveTo, "move", 0);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Scrollbar *sb = obj;
	const int zoomLvl = WIDGET(sb)->window->zoom;

#ifdef AG_DEBUG
	if (zoomLvl < 0 || zoomLvl >= sizeof(agScrollbarZoomSizes) / sizeof(int))
		AG_FatalError("zoomLvl");
#endif
	switch (sb->type) {
	case AG_SCROLLBAR_HORIZ:
		r->h = WIDGET(sb)->paddingTop + agScrollbarZoomSizes[zoomLvl] +
		       WIDGET(sb)->paddingBottom;
		r->w = WIDGET(sb)->paddingLeft + (r->h << 2) +
		       WIDGET(sb)->paddingRight;
		break;
	case AG_SCROLLBAR_VERT:
		r->w = WIDGET(sb)->paddingLeft + agScrollbarZoomSizes[zoomLvl] +
		       WIDGET(sb)->paddingRight;
		r->h = WIDGET(sb)->paddingTop + (r->w << 2) +
		       WIDGET(sb)->paddingBottom;
		break;
	}
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_Scrollbar *sb = obj;

	if (a->w < WIDGET(sb)->paddingLeft + WIDGET(sb)->paddingRight ||
	    a->h < WIDGET(sb)->paddingTop + WIDGET(sb)->paddingBottom) {
		return (-1);
	}
	return (0);
}

static void
Draw(void *_Nonnull obj)
{
	AG_Scrollbar *sb = obj;
	static void (*pf[])(AG_Scrollbar *_Nonnull, int, int) = {
		DrawHoriz,
		DrawVert
	};
	int x, len;

	if (GetPxCoords(sb, &x, &len) == -1) {
		x = 0;
		len = SBLEN(sb) - (SBTHICK(sb) << 1);
	}
#ifdef AG_DEBUG
	if (sb->type != AG_SCROLLBAR_HORIZ &&
	    sb->type != AG_SCROLLBAR_VERT)
		AG_FatalError("sb->type");
#endif
	pf[sb->type](sb, x, len);
}

static void
DrawVert(AG_Scrollbar *_Nonnull sb, int y, int len)
{
	AG_Rect r;
	AG_Color cTinted;
	const AG_Color *cFg = &WCOLOR(sb, FG_COLOR);
	const AG_Color *c;
	const int w = WIDTH(sb);
	const int h = HEIGHT(sb);
	const int sbThick = SBTHICK(sb);
	const int x = (sbThick >> 1);
	const int btn = sb->curBtn;
	const AG_Surface *S;
	int lblX1, lblY1, lblX2, lblY2;

	if (h < (sbThick << 1)) {
		DrawVertUndersize(sb);
		return;
	}

	if (sb->lbl1 == -1) {
		AG_Color cNone;

		AG_ColorNone(&cNone);
		AG_TextBGColor(&cNone);
		AG_TextColor(&WCOLOR(sb, TEXT_COLOR));

		sb->lbl1 = AG_WidgetMapSurface(sb,
		    AG_TextRenderCropped(AGSI_ALGUE
			                 AGSI_BLACK_UP_POINTING_TRIANGLE));

		sb->lbl2 = AG_WidgetMapSurface(sb,
		    AG_TextRenderCropped(AGSI_ALGUE
			                 AGSI_BLACK_DN_POINTING_TRIANGLE));
	}

	r.x = 1;
	r.y = 0;
	r.w = w - 1;
	r.h = h;
	AG_DrawFrameSunk(sb, &r);

	r.h = sbThick;                                  /* Decrement button */
	S = WSURFACE(sb, sb->lbl1);
	if (btn == AG_SCROLLBAR_BUTTON_DEC) {
		AG_DrawBoxSunk(sb, &r, cFg);
		lblX1 = r.x + ((r.w >> 1) - (S->w >> 1));
		lblY1 = r.y + ((r.h >> 1) - (S->h >> 1)) + 1;
	} else {
		AG_DrawBoxRaised(sb, &r, cFg);
		lblX1 = r.x + ((r.w >> 1) - (S->w >> 1) - 1);
		lblY1 = r.y + ((r.h >> 1) - (S->h >> 1));
	}

	r.y = sbThick + y;                                   /* Control bar */
	r.h = MIN(len, h - x);
	if (r.h < 4) {
		r.h = 4;
		r.y -= (r.h >> 1);
	}
	c = (btn == AG_SCROLLBAR_BUTTON_SCROLL) ? &WCOLOR_HOVER(sb, FG_COLOR) :
	                                          cFg;
	if (r.h < AG_SCROLLBAR_HOT) {
		cTinted = *c;
		AG_ColorLighten(&cTinted, AG_SCROLLBAR_HOT - r.h);
		c = &cTinted;
	}
	if (btn == AG_SCROLLBAR_BUTTON_SCROLL) {
		AG_DrawBoxSunk(sb, &r, cFg);
	} else {
		AG_DrawBoxRaised(sb, &r, cFg);
	}
	
	r.y = h - sbThick;                              /* Increment button */
	r.h = sbThick;
	S = WSURFACE(sb, sb->lbl2);
	if (btn == AG_SCROLLBAR_BUTTON_INC) {
		AG_DrawBoxSunk(sb, &r, cFg);
		lblX2 = r.x + ((r.w >> 1) - (S->w >> 1));
		lblY2 = r.y + ((r.h >> 1) - (S->h >> 1)) + 1;
	} else {
		AG_DrawBoxRaised(sb, &r, cFg);
		lblX2 = r.x + ((r.w >> 1) - (S->w >> 1) - 1);
		lblY2 = r.y + ((r.h >> 1) - (S->h >> 1));
	}

	if (S->w < w && S->h < h) {
		AG_PushBlendingMode(sb, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
		AG_WidgetBlitSurface(sb, sb->lbl1, lblX1, lblY1);
		AG_WidgetBlitSurface(sb, sb->lbl2, lblX2, lblY2);
		AG_PopBlendingMode(sb);
	}
}

static void
DrawHoriz(AG_Scrollbar *_Nonnull sb, int x, int len)
{
	AG_Rect r;
	AG_Color cTinted;
	const AG_Color *cFg = &WCOLOR(sb, FG_COLOR);
	const AG_Color *c;
	const int w = WIDTH(sb);
	const int h = HEIGHT(sb);
	const int sbThick = SBTHICK(sb);
	const int y = (sbThick >> 1);
	const int btn = sb->curBtn;
	const AG_Surface *S;
	int lblX1, lblY1, lblX2, lblY2;
	
	if (w < (sbThick << 1)) {
		DrawHorizUndersize(sb);
		return;
	}

	if (sb->lbl1 == -1) {
		AG_Color cNone;

		AG_ColorNone(&cNone);
		AG_TextColor(&WCOLOR(sb, TEXT_COLOR));
		AG_TextBGColor(&cNone);

		sb->lbl1 = AG_WidgetMapSurface(sb,
		    AG_TextRenderCropped(AGSI_ALGUE
			                 AGSI_BLACK_L_POINTING_TRIANGLE));

		sb->lbl2 = AG_WidgetMapSurface(sb,
		    AG_TextRenderCropped(AGSI_ALGUE
			                 AGSI_BLACK_R_POINTING_TRIANGLE));
	}

	r.x = 0;
	r.y = 0;
	r.w = w;
	r.h = h;
	AG_DrawFrameSunk(sb, &r);

	r.w = sbThick;                                  /* Decrement button */
	S = WSURFACE(sb, sb->lbl1);
	if (btn == AG_SCROLLBAR_BUTTON_DEC) {
		AG_DrawBoxSunk(sb, &r, cFg);
		lblX1 = r.x + ((r.w >> 1) - (S->w >> 1) - 1) + 1;
		lblY1 = r.y + ((r.h >> 1) - (S->h >> 1)) + 1;
	} else {
		AG_DrawBoxRaised(sb, &r, cFg);
		lblX1 = r.x + ((r.w >> 1) - (S->w >> 1) - 1);
		lblY1 = r.y + ((r.h >> 1) - (S->h >> 1));
	}

	r.x = sbThick + x;                                   /* Control bar */
	r.w = MIN(len, w - y);
	if (r.w < 4) {
		r.w = 4;
		r.x -= (r.w >> 1);
	}
	c = (btn == AG_SCROLLBAR_BUTTON_SCROLL) ? &WCOLOR_HOVER(sb, FG_COLOR) :
	                                          cFg;
	if (r.w < AG_SCROLLBAR_HOT) {
		cTinted = *c;
		AG_ColorLighten(&cTinted, AG_SCROLLBAR_HOT-r.w);
		c = &cTinted;
	}
	if (btn == AG_SCROLLBAR_BUTTON_SCROLL) {
		AG_DrawBoxSunk(sb, &r, c);
	} else {
		AG_DrawBoxRaised(sb, &r, c);
	}

	r.x = w - sbThick;                              /* Increment button */
	r.w = sbThick;
	S = WSURFACE(sb, sb->lbl2);
	if (btn == AG_SCROLLBAR_BUTTON_INC) {
		AG_DrawBoxSunk(sb, &r, cFg);
		lblX2 = r.x + ((r.w >> 1) - (S->w >> 1) - 1) + 1;
		lblY2 = r.y + ((r.h >> 1) - (S->h >> 1)) + 1;
	} else {
		AG_DrawBoxRaised(sb, &r, cFg);
		lblX2 = r.x + ((r.w >> 1) - (S->w >> 1) - 1);
		lblY2 = r.y + ((r.h >> 1) - (S->h >> 1));
	}

	if (S->w < w && S->h < h) {
		AG_PushBlendingMode(sb, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);
		AG_WidgetBlitSurface(sb, sb->lbl1, lblX1, lblY1);
		AG_WidgetBlitSurface(sb, sb->lbl2, lblX2, lblY2);
		AG_PopBlendingMode(sb);
	}
}

static void
DrawVertUndersize(AG_Scrollbar *_Nonnull sb)
{
	const int w = WIDTH(sb);
	const int h = HEIGHT(sb);
	int mid, s;

	AG_DrawBoxRaised(sb, &WIDGET(sb)->r, &WCOLOR(sb,FG_COLOR));
	
	mid = (w >> 1);
	s = MIN(h >> 2, w);

	AG_DrawArrowUp(sb, mid, s, s,          &WCOLOR(sb,LINE_COLOR));
	AG_DrawArrowDown(sb, mid, (h>>1)+s, s, &WCOLOR(sb,LINE_COLOR));
}

static void
DrawHorizUndersize(AG_Scrollbar *_Nonnull sb)
{
	const int w = WIDTH(sb);
	const int h = HEIGHT(sb);
	int mid, s;

	AG_DrawBoxRaised(sb, &WIDGET(sb)->r, &WCOLOR(sb,FG_COLOR));

	mid = (h >> 1);
	s = MIN(w >> 2, h);

	AG_DrawArrowLeft(sb, s, mid, s,         &WCOLOR(sb,LINE_COLOR));
	AG_DrawArrowRight(sb, (w>>1)+s, mid, s, &WCOLOR(sb,LINE_COLOR));
}

/*
 * Evaluate whether it would be useful to display a scrollbar given its current
 * "value" and range ("min","max","visible"). Return 1=Yes or 0=No.
 *
 * This may be called by the size_allocate() of container widgets in order
 * to determine whether to allocate space for scrollbars or not.
 */
int
AG_ScrollbarIsUseful(AG_Scrollbar *sb)
{
	int rv, x, len;

	AG_OBJECT_ISA(sb, "AG_Widget:AG_Scrollbar:*");
	AG_ObjectLock(sb);

	if (!AG_Defined(sb, "value") ||
	    !AG_Defined(sb, "min") ||
	    !AG_Defined(sb, "max") ||
	    !AG_Defined(sb, "visible")) {
		AG_ObjectUnlock(sb);
		return (1);
	}
	rv = (GetPxCoords(sb, &x, &len) == -1) ? 0 : 1;

	AG_ObjectUnlock(sb);
	return (rv);
}

static void *_Nullable
Edit(void *_Nonnull obj)
{
	static const AG_FlagDescr flagDescr[] = {
		{ AG_SCROLLBAR_SMOOTH,   N_("Smooth scrolling behavior"), 1 },
		{ AG_SCROLLBAR_EXCL,     N_("Exclusive access"),          0 },
		{ 0,                     NULL,                            0 },
	};
	const char *sbTypeNames[] = {
		_("Horizontal"),
		_("Vertical"),
		NULL
	};
	AG_Scrollbar *sb = obj;
	AG_Box *box = AG_BoxNewVert(NULL, AG_BOX_EXPAND);

	AG_CheckboxSetFromFlags(box, 0, &sb->flags, flagDescr);

	AG_LabelNewS(box, 0, _("Direction:"));
	AG_RadioNewUint(box, 0, sbTypeNames, (Uint *)&sb->type);

	AG_SeparatorNewHoriz(box);

	AG_LabelNewPolled(box, AG_LABEL_HFILL, "Button: #%d (mouse over #%d)",
	    &sb->curBtn, &sb->mouseOverBtn);
	AG_LabelNewPolled(box, AG_LABEL_HFILL, "xOffs: %d, xSeek: %d",
	    &sb->xOffs, &sb->xSeek);

	return (box);
}

AG_WidgetClass agScrollbarClass = {
	{
		"Agar(Widget:Scrollbar)",
		sizeof(AG_Scrollbar),
		{ 1,0, AGC_SCROLLBAR, 0xE02B },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		Edit
	},
	Draw,
	SizeRequest,
	SizeAllocate,
	MouseButtonDown,
	MouseButtonUp,
	MouseMotion,
	KeyDown,
	KeyUp,
	NULL,			/* touch */
	Ctrl,
	NULL			/* joy */
};

#endif /* AG_WIDGETS */
