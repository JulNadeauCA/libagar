/*	$Csoft: scrollbar.c,v 1.49 2005/06/21 04:31:34 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
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

#include <engine/widget/scrollbar.h>

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

const AG_WidgetOps scrollbar_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		AG_WidgetDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_ScrollbarDraw,
	AG_ScrollbarScale
};

enum ag_button_which {
	AG_BUTTON_NONE,
	AG_BUTTON_UP,
	AG_BUTTON_DOWN,
	AG_BUTTON_SCROLL
};

static void scrollbar_mousebuttonup(int, union evarg *);
static void scrollbar_mousebuttondown(int, union evarg *);
static void scrollbar_mousemotion(int, union evarg *);

AG_Scrollbar *
AG_ScrollbarNew(void *parent, enum ag_scrollbar_type type)
{
	AG_Scrollbar *sb;

	sb = Malloc(sizeof(AG_Scrollbar), M_WIDGET);
	AG_ScrollbarInit(sb, type);
	AG_ObjectAttach(parent, sb);
	return (sb);
}

void
AG_ScrollbarInit(AG_Scrollbar *sb, enum ag_scrollbar_type type)
{
	AG_WidgetInit(sb, "scrollbar", &scrollbar_ops,
		AG_WIDGET_FOCUSABLE|AG_WIDGET_UNFOCUSED_BUTTONUP);
	AG_WidgetBind(sb, "value", AG_WIDGET_INT, &sb->value);
	AG_WidgetBind(sb, "min", AG_WIDGET_INT, &sb->min);
	AG_WidgetBind(sb, "max", AG_WIDGET_INT, &sb->max);

	sb->value = 0;
	sb->min = 0;
	sb->max = 0;
	sb->type = type;
	sb->curbutton = AG_BUTTON_NONE;
	sb->bar_size = 30;
	sb->button_size = agTextFontHeight;

	AG_SetEvent(sb, "window-mousebuttondown", scrollbar_mousebuttondown,
		NULL);
	AG_SetEvent(sb, "window-mousebuttonup", scrollbar_mousebuttonup, NULL);
	AG_SetEvent(sb, "window-mousemotion", scrollbar_mousemotion, NULL);
}

static void
scrollbar_mousebuttonup(int argc, union evarg *argv)
{
	AG_Scrollbar *sb = argv[0].p;

	sb->curbutton = AG_BUTTON_NONE;
}

/* Clicked or dragged mouse to coord, so adjust value */
static void
scrollbar_mouse_select(AG_Scrollbar *sb, int coord, int totalsize)
{
	const int scrolling_area = totalsize - (sb->button_size*2);
	int min, max;

	if (sb->curbutton != AG_BUTTON_SCROLL)
		return;
	if (sb->bar_size == -1)
		return;
		
	min = AG_WidgetInt(sb, "min");
	max = AG_WidgetInt(sb, "max");
	
	if (max < min)
		return;
	
	/* mouse below min */
	if (coord <= sb->button_size) {
		AG_WidgetSetInt(sb, "value", min);
	}
	/* mouse above max */
	else if (coord >= sb->button_size + scrolling_area) {
		AG_WidgetSetInt(sb, "value", max);
	}
	/* mouse between */
	else {
		int ncoord = coord - sb->button_size;

		AG_WidgetSetInt(sb, "value", ncoord*(max-min+1)/scrolling_area);
	}
	
	/* generate an event */
	AG_PostEvent(NULL, sb, "scrollbar-changed", "%i",
		AG_WidgetInt(sb, "value"));
}

static void
scrollbar_mousebuttondown(int argc, union evarg *argv)
{
	AG_Scrollbar *sb = argv[0].p;
	int button = argv[1].i;
	int coord = (sb->type == AG_SCROLLBAR_HORIZ) ?
		argv[2].i : argv[3].i;
	int totalsize = (sb->type == AG_SCROLLBAR_HORIZ) ?
		AGWIDGET(sb)->w : AGWIDGET(sb)->h;
	int min, value, max, nvalue;

	if (button != SDL_BUTTON_LEFT)
		return;

	min = AG_WidgetInt(sb, "min");
	max = AG_WidgetInt(sb, "max");
	value = AG_WidgetInt(sb, "value");

	if (max < min)
		return;
	
	AG_WidgetFocus(sb);
	
	/* click on the up button */
	if (coord <= sb->button_size) {
		sb->curbutton = AG_BUTTON_UP;
		if (value > min) 
			AG_WidgetSetInt(sb, "value", value - 1);
	}
	/* click on the down button */
	else if (coord >= totalsize - sb->button_size) {
		sb->curbutton = AG_BUTTON_DOWN;
		if (value < max)
			AG_WidgetSetInt(sb, "value", value + 1);
	}
	/* click in between */
	else {
		sb->curbutton = AG_BUTTON_SCROLL;
		scrollbar_mouse_select(sb, coord, totalsize);
	}
	
	/* generate an event if value changed */
	if (value != (nvalue = AG_WidgetInt(sb, "value")))
		AG_PostEvent(NULL, sb, "scrollbar-changed", "%i", nvalue);
}

static void
scrollbar_mousemotion(int argc, union evarg *argv)
{
	AG_Scrollbar *sb = argv[0].p;
	int coord = (sb->type == AG_SCROLLBAR_HORIZ) ?
		argv[1].i : argv[2].i;
	int state = argv[5].i;
	int totalsize = (sb->type == AG_SCROLLBAR_HORIZ) ?
		AGWIDGET(sb)->w : AGWIDGET(sb)->h;

	if (state & SDL_BUTTON_LMASK)
		scrollbar_mouse_select(sb, coord, totalsize);
}

void
AG_ScrollbarScale(void *p, int rw, int rh)
{
	AG_Scrollbar *sb = p;

	switch (sb->type) {
	case AG_SCROLLBAR_HORIZ:
		if (rw == -1) {
			AGWIDGET(sb)->w = sb->button_size*4;
		}
		if (rh == -1) {
			AGWIDGET(sb)->h = sb->button_size;
		} else {
			sb->button_size = AGWIDGET(sb)->h;	/* Square */
		}
		break;
	case AG_SCROLLBAR_VERT:
		if (rw == -1) {
			AGWIDGET(sb)->w = sb->button_size;
		} else {
			sb->button_size = AGWIDGET(sb)->w;	/* Square */
		}
		if (rh == -1) {
			AGWIDGET(sb)->h = sb->button_size*4;
		}
		break;
	}
}

void
AG_ScrollbarDraw(void *p)
{
	AG_Scrollbar *sb = p;
	int value, min, max;
	int w, h, x, y;
	int maxcoord;

	if (AGWIDGET(sb)->w < sb->button_size ||
	    AGWIDGET(sb)->w < sb->button_size)
		return;

	value = AG_WidgetInt(sb, "value");
	min = AG_WidgetInt(sb, "min");
	max = AG_WidgetInt(sb, "max");
	
	if (max < min || max == 0)
		return;

#ifdef DEBUG
	if (value < min || value > max) {
		dprintf("invalid value: min=%d, value=%d, max=%d\n", min,
		    value, max);
		return;
	}
#endif

	agPrim.box(sb, 0, 0, AGWIDGET(sb)->w, AGWIDGET(sb)->h, -1,
	    AG_COLOR(SCROLLBAR_COLOR));

	switch (sb->type) {
	case AG_SCROLLBAR_VERT:
		if (AGWIDGET(sb)->h < sb->button_size*2 + 6) {
			return;
		}
		maxcoord = AGWIDGET(sb)->h - sb->button_size * 2 - sb->bar_size;
		
		/* Draw the up and down buttons. */
		agPrim.box(sb,
		    0, 0,
		    sb->button_size, sb->button_size,
		    (sb->curbutton == AG_BUTTON_UP) ? -1 : 1,
		    AG_COLOR(SCROLLBAR_BTN_COLOR));
		agPrim.box(sb,
		    0, AGWIDGET(sb)->h - sb->button_size,
		    sb->button_size,
		    sb->button_size,
		    (sb->curbutton == AG_BUTTON_DOWN) ? -1 : 1,
		    AG_COLOR(SCROLLBAR_BTN_COLOR));
		
		/* Calculate disabled bar */
		if (sb->bar_size == -1) {
			y = 0;
			h = AGWIDGET(sb)->h - sb->button_size*2;
		}
		/* Calculate active bar */
		else {
			y = value * maxcoord / (max-min);
			h = sb->bar_size;
			if (sb->button_size + y + h >
			    AGWIDGET(sb)->h - sb->button_size)
				y = AGWIDGET(sb)->h - sb->button_size*2 - h;
		}
		/* Draw bar */
		agPrim.box(sb,
		    0, sb->button_size + y,
		    sb->button_size,
		    h,
		    (sb->curbutton == AG_BUTTON_SCROLL) ? -1 : 1,
		    AG_COLOR(SCROLLBAR_BTN_COLOR));
		break;
	case AG_SCROLLBAR_HORIZ:
		if (AGWIDGET(sb)->w < sb->button_size*2 + 6) {
			return;
		}
		maxcoord = AGWIDGET(sb)->w - sb->button_size*2 - sb->bar_size;

		/* Draw the up and down buttons */
		agPrim.box(sb,
		    0, 0,
		    sb->button_size, sb->button_size,
		    (sb->curbutton == AG_BUTTON_UP) ? -1 : 1,
		    AG_COLOR(SCROLLBAR_BTN_COLOR));
		agPrim.box(sb,
		    AGWIDGET(sb)->w - sb->button_size, 0,
		    sb->button_size, sb->button_size,
		    (sb->curbutton == AG_BUTTON_DOWN) ? -1 : 1,
		    AG_COLOR(SCROLLBAR_BTN_COLOR));
		
		/* Calculate disabled bar */
		if (sb->bar_size == -1) {
			x = 0;
			w = AGWIDGET(sb)->w - sb->button_size*2;
		}
		/* Calculate active bar */
		else {
			x = value * maxcoord / (max-min);
			w = sb->bar_size;
			if (sb->button_size + x + w >
			    AGWIDGET(sb)->w - sb->button_size)
				x = AGWIDGET(sb)->w - sb->button_size*2 - w;
		}
		
		/* Draw bar */
		agPrim.box(sb,
		    sb->button_size + x,
		    0,
		    w,
		    sb->button_size,
		    (sb->curbutton == AG_BUTTON_SCROLL) ? -1 : 1,
		    AG_COLOR(SCROLLBAR_BTN_COLOR));
		break;
	}
#if 0
	{
		SDL_Surface *txt;
		char label[32];

		snprintf(label, sizeof(label), "%d\n%d\n%d\n",
		    value, min, max);
		txt = AG_TextRender(NULL, -1, AG_COLOR(TEXT_COLOR), label);
		AG_WidgetBlit(sb, txt, 0, 0);
		SDL_FreeSurface(txt);
		    
	}
#endif
}

void
AG_ScrollbarSetBarSize(AG_Scrollbar *sb, int bsize)
{
	sb->bar_size = (bsize > 10 || bsize == -1) ? bsize : 10;
}

void
AG_ScrollbarGetBarSize(AG_Scrollbar *sb, int *bsize)
{
	*bsize = sb->bar_size;
}
