/*	$Csoft: button.c,v 1.94 2005/09/27 14:06:35 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
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

#include <engine/engine.h>
#include <engine/view.h>

#include <stdarg.h>

#include "button.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/label.h>

const AG_WidgetOps agButtonOps = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		NULL,		/* destroy XXX */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_ButtonDraw,
	AG_ButtonScale
};

static void button_mousemotion(int, union evarg *);
static void button_mousebuttonup(int, union evarg *);
static void button_mousebuttondown(int, union evarg *);
static void button_keyup(int, union evarg *);
static void button_keydown(int, union evarg *);

AG_Button *
AG_ButtonNew(void *parent, const char *caption)
{
	AG_Button *btn;

	btn = Malloc(sizeof(AG_Button), M_OBJECT);
	AG_ButtonInit(btn, caption, 0);
	AG_ObjectAttach(parent, btn);
	return (btn);
}

AG_Button *
AG_ButtonAct(void *parent, const char *caption, u_int flags,
    void (*fn)(int, union evarg *), const char *fmt, ...)
{
	AG_Button *btn;
	AG_Event *ev;
	va_list ap;

	btn = Malloc(sizeof(AG_Button), M_OBJECT);
	AG_ButtonInit(btn, caption, flags);
	AG_ObjectAttach(parent, btn);

	ev = AG_SetEvent(btn, "button-pushed", fn, NULL);
	if (fmt != NULL) {
		va_start(ap, fmt);
		for (; *fmt != '\0'; fmt++) {
			AG_EVENT_PUSH_ARG(ap, *fmt, ev);
		}
		va_end(ap);
	}
	if (flags & AG_BUTTON_FOCUS) {
		AG_WidgetFocus(btn);
	}
	return (btn);
}


static Uint32
repeat_expire(void *obj, Uint32 ival, void *arg)
{
	AG_PostEvent(NULL, obj, "button-pushed", "%i", 1);
	return (agMouseSpinIval);
}

static Uint32
delay_expire(void *obj, Uint32 ival, void *arg)
{
	AG_Button *bu = obj;

	AG_ReplaceTimeout(bu, &bu->repeat_to, agMouseSpinIval);
	return (0);
}

void
AG_ButtonInit(AG_Button *bu, const char *caption, u_int flags)
{
	SDL_Surface *label;

	/* XXX replace the unfocused motion flag with a timer */
	AG_WidgetInit(bu, "button", &agButtonOps,
	    AG_WIDGET_FOCUSABLE|AG_WIDGET_UNFOCUSED_MOTION);
	AG_WidgetBind(bu, "state", AG_WIDGET_BOOL, &bu->state);

	label = (caption == NULL) ? NULL :
	    AG_TextRender(NULL, -1, AG_COLOR(BUTTON_TXT_COLOR), caption);
	AG_WidgetMapSurface(bu, label);

	bu->flags = 0;
	bu->state = 0;
	bu->justify = AG_BUTTON_CENTER;
	bu->padding = 4;

	AG_SetTimeout(&bu->repeat_to, repeat_expire, NULL, 0);
	AG_SetTimeout(&bu->delay_to, delay_expire, NULL, 0);

	AG_SetEvent(bu, "window-mousebuttonup", button_mousebuttonup, NULL);
	AG_SetEvent(bu, "window-mousebuttondown", button_mousebuttondown, NULL);
	AG_SetEvent(bu, "window-mousemotion", button_mousemotion, NULL);
	AG_SetEvent(bu, "window-keyup", button_keyup, NULL);
	AG_SetEvent(bu, "window-keydown", button_keydown, NULL);

	if (flags & AG_BUTTON_WFILL) { AGWIDGET(bu)->flags |= AG_WIDGET_WFILL; }
	if (flags & AG_BUTTON_HFILL) { AGWIDGET(bu)->flags |= AG_WIDGET_HFILL; }
}

void
AG_ButtonScale(void *p, int w, int h)
{
	AG_Button *bu = p;
	SDL_Surface *label = AGWIDGET_SURFACE(bu,0);

	if (w == -1 && h == -1) {
		if (label != NULL) {
			AGWIDGET(bu)->w = label->w + bu->padding*2;
			AGWIDGET(bu)->h = label->h + bu->padding*2;
		} else {
			AGWIDGET(bu)->w = 1;
			AGWIDGET(bu)->h = 1;
		}
	}
}

void
AG_ButtonDraw(void *p)
{
	AG_Button *bu = p;
	SDL_Surface *label = AGWIDGET_SURFACE(bu,0);
	int x = 0, y = 0;
	int pressed;
	
	if (AGWIDGET(bu)->w < 8 || AGWIDGET(bu)->h < 8)
		return;

	pressed = AG_WidgetBool(bu, "state");
	if (bu->flags & AG_BUTTON_INSENSITIVE) {
		agPrim.box(bu,
		    0, 0,
		    AGWIDGET(bu)->w, AGWIDGET(bu)->h,
		    -1,
		    AG_COLOR(BUTTON_DIS_COLOR));
	} else {
		agPrim.box(bu,
		    0, 0,
		    AGWIDGET(bu)->w, AGWIDGET(bu)->h,
		    pressed ? -1 : 1,
		    AG_COLOR(BUTTON_COLOR));
	}

	if (label != NULL) {
		switch (bu->justify) {
		case AG_BUTTON_LEFT:
			x = bu->padding;
			break;
		case AG_BUTTON_CENTER:
			x = AGWIDGET(bu)->w/2 - label->w/2;
			break;
		case AG_BUTTON_RIGHT:
			x = AGWIDGET(bu)->w - label->w - bu->padding;
			break;
		}
		y = ((AGWIDGET(bu)->h - label->h)/2) - 1;	/* Middle */

		if (pressed) {
			x++;
			y++;
		}
		AG_WidgetBlitSurface(bu, 0, x, y);
	}
}

static void
button_mousemotion(int argc, union evarg *argv)
{
	AG_Button *bu = argv[0].p;
	AG_WidgetBinding *stateb;
	int x = argv[1].i;
	int y = argv[2].i;
	int *pressed;

	if (bu->flags & AG_BUTTON_INSENSITIVE)
		return;

	stateb = AG_WidgetGetBinding(bu, "state", &pressed);
	if (!AG_WidgetRelativeArea(bu, x, y)) {
		if ((bu->flags & AG_BUTTON_STICKY) == 0
		    && *pressed == 1) {
			*pressed = 0;
			AG_WidgetBindingChanged(stateb);
		}
		if (bu->flags & AG_BUTTON_MOUSEOVER) {
			bu->flags &= ~(AG_BUTTON_MOUSEOVER);
			AG_PostEvent(NULL, bu, "button-mouseoverlap", "%i", 0);
		}
	} else {
		bu->flags |= AG_BUTTON_MOUSEOVER;
		AG_PostEvent(NULL, bu, "button-mouseoverlap", "%i", 1);
	}
	AG_WidgetUnlockBinding(stateb);
}

static void
button_mousebuttondown(int argc, union evarg *argv)
{
	AG_Button *bu = argv[0].p;
	int button = argv[1].i;
	AG_WidgetBinding *stateb;
	int *pushed;
	
	if (bu->flags & AG_BUTTON_INSENSITIVE)
		return;

	AG_WidgetFocus(bu);

	if (button != SDL_BUTTON_LEFT)
		return;
	
	stateb = AG_WidgetGetBinding(bu, "state", &pushed);
	if (!(bu->flags & AG_BUTTON_STICKY)) {
		*pushed = 1;
	} else {
		*pushed = !(*pushed);
		AG_PostEvent(NULL, bu, "button-pushed", "%i", *pushed);
	}
	AG_WidgetBindingChanged(stateb);
	AG_WidgetUnlockBinding(stateb);

	if (bu->flags & AG_BUTTON_REPEAT) {
		AG_DelTimeout(bu, &bu->repeat_to);
		AG_ReplaceTimeout(bu, &bu->delay_to, agMouseSpinDelay);
	}
}

static void
button_mousebuttonup(int argc, union evarg *argv)
{
	AG_Button *bu = argv[0].p;
	int button = argv[1].i;
	AG_WidgetBinding *stateb;
	int *pushed;
	int x = argv[2].i;
	int y = argv[3].i;
		
	if (bu->flags & AG_BUTTON_REPEAT) {
		AG_DelTimeout(bu, &bu->repeat_to);
		AG_DelTimeout(bu, &bu->delay_to);
	}
	
	if ((bu->flags & AG_BUTTON_INSENSITIVE) ||
	    x < 0 || y < 0 ||
	    x > AGWIDGET(bu)->w || y > AGWIDGET(bu)->h) {
		return;
	}
	
	stateb = AG_WidgetGetBinding(bu, "state", &pushed);
	if (*pushed &&
	    button == SDL_BUTTON_LEFT &&
	    !(bu->flags & AG_BUTTON_STICKY)) {
	    	*pushed = 0;
		AG_PostEvent(NULL, bu, "button-pushed", "%i", *pushed);
		AG_WidgetBindingChanged(stateb);
	}
	AG_WidgetUnlockBinding(stateb);
}

static void
button_keydown(int argc, union evarg *argv)
{
	AG_Button *bu = argv[0].p;
	int keysym = argv[1].i;
	
	if (bu->flags & AG_BUTTON_INSENSITIVE)
		return;

	if (keysym == SDLK_RETURN || keysym == SDLK_SPACE) {
		AG_WidgetSetBool(bu, "state", 1);
		AG_PostEvent(NULL, bu, "button-pushed", "%i", 1);

		if (bu->flags & AG_BUTTON_REPEAT) {
			AG_DelTimeout(bu, &bu->repeat_to);
			AG_ReplaceTimeout(bu, &bu->delay_to, 800);
		}
	}
}

static void
button_keyup(int argc, union evarg *argv)
{
	AG_Button *bu = argv[0].p;
	int keysym = argv[1].i;
	
	if (bu->flags & AG_BUTTON_INSENSITIVE)
		return;
	
	if (bu->flags & AG_BUTTON_REPEAT) {
		AG_DelTimeout(bu, &bu->delay_to);
		AG_DelTimeout(bu, &bu->repeat_to);
	}

	if (keysym == SDLK_RETURN || keysym == SDLK_SPACE) {
		AG_WidgetSetBool(bu, "state", 0);
		AG_PostEvent(NULL, bu, "button-pushed", "%i", 0);
	}
}

void
AG_ButtonEnable(AG_Button *bu)
{
	bu->flags &= ~(AG_BUTTON_INSENSITIVE);
}

void
AG_ButtonDisable(AG_Button *bu)
{
	bu->flags |= (AG_BUTTON_INSENSITIVE);
}

void
AG_ButtonSetPadding(AG_Button *bu, int padding)
{
	bu->padding = padding;
}

void
AG_ButtonSetFocusable(AG_Button *bu, int focusable)
{
	if (focusable) {
		AGWIDGET(bu)->flags |= AG_WIDGET_FOCUSABLE;
		AGWIDGET(bu)->flags &= ~(AG_WIDGET_UNFOCUSED_BUTTONUP);
	} else {
		AGWIDGET(bu)->flags &= ~(AG_WIDGET_FOCUSABLE);
		AGWIDGET(bu)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP;
	}
}

void
AG_ButtonSetSticky(AG_Button *bu, int sticky)
{
	if (sticky) {
		bu->flags |= (AG_BUTTON_STICKY);
	} else {
		bu->flags &= ~(AG_BUTTON_STICKY);
	}
}

void
AG_ButtonSetJustification(AG_Button *bu, enum ag_button_justify jus)
{
	bu->justify = jus;
}

void
AG_ButtonSetSurface(AG_Button *bu, SDL_Surface *su)
{
	AG_WidgetReplaceSurface(bu, 0, su);
}

void
AG_ButtonSetRepeatMode(AG_Button *bu, int repeat)
{
	if (repeat) {
		bu->flags |= (AG_BUTTON_REPEAT);
	} else {
		AG_DelTimeout(bu, &bu->repeat_to);
		AG_DelTimeout(bu, &bu->delay_to);
		bu->flags &= ~(AG_BUTTON_REPEAT);
	}
}

void
AG_ButtonPrintf(AG_Button *bu, const char *fmt, ...)
{
	char buf[AG_LABEL_MAX];
	va_list args;

	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	AG_WidgetReplaceSurface(bu, 0,
	    AG_TextRender(NULL, -1, AG_COLOR(BUTTON_TXT_COLOR), buf));
}

