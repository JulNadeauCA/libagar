/*	$Csoft: button.c,v 1.88 2005/02/22 04:18:44 vedge Exp $	*/

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

const struct widget_ops button_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	button_draw,
	button_scale
};

enum {
	FRAME_COLOR,
	TEXT_COLOR,
	DISABLED_COLOR
};

static void	button_mousemotion(int, union evarg *);
static void	button_mousebuttonup(int, union evarg *);
static void	button_mousebuttondown(int, union evarg *);
static void	button_keyup(int, union evarg *);
static void	button_keydown(int, union evarg *);

struct button *
button_new(void *parent, const char *caption)
{
	struct button *button;

	button = Malloc(sizeof(struct button), M_OBJECT);
	button_init(button, caption);
	object_attach(parent, button);
	return (button);
}

static Uint32
repeat_expire(void *obj, Uint32 ival, void *arg)
{
	event_post(NULL, obj, "button-pushed", "%i", 1);
	return (mouse_spin_ival);
}

static Uint32
delay_expire(void *obj, Uint32 ival, void *arg)
{
	struct button *bu = obj;

	timeout_replace(bu, &bu->repeat_to, mouse_spin_ival);
	return (0);
}

void
button_init(struct button *bu, const char *caption)
{
	SDL_Surface *label;

	widget_init(bu, "button", &button_ops, WIDGET_FOCUSABLE |
	    WIDGET_UNFOCUSED_MOTION);
	widget_bind(bu, "state", WIDGET_BOOL, &bu->state);
	
	widget_map_color(bu, FRAME_COLOR, "frame", 100, 100, 100, 255);
	widget_map_color(bu, TEXT_COLOR, "text", 240, 240, 240, 255);
	widget_map_color(bu, DISABLED_COLOR, "disabled", 110, 110, 110, 255);

	label = (caption == NULL) ? NULL :
	    text_render(NULL, -1, WIDGET_COLOR(bu, TEXT_COLOR), caption);
	widget_map_surface(bu, label);

	bu->flags = 0;
	bu->state = 0;
	bu->justify = BUTTON_CENTER;
	bu->padding = 4;

	timeout_set(&bu->repeat_to, repeat_expire, NULL, 0);
	timeout_set(&bu->delay_to, delay_expire, NULL, 0);

	event_new(bu, "window-mousebuttonup", button_mousebuttonup, NULL);
	event_new(bu, "window-mousebuttondown", button_mousebuttondown, NULL);
	event_new(bu, "window-mousemotion", button_mousemotion, NULL);
	event_new(bu, "window-keyup", button_keyup, NULL);
	event_new(bu, "window-keydown", button_keydown, NULL);
}

void
button_scale(void *p, int w, int h)
{
	struct button *bu = p;
	SDL_Surface *label = WIDGET_SURFACE(bu,0);

	if (w == -1 && h == -1) {
		if (label != NULL) {
			WIDGET(bu)->w = label->w + bu->padding*2;
			WIDGET(bu)->h = label->h + bu->padding*2;
		} else {
			WIDGET(bu)->w = 1;
			WIDGET(bu)->h = 1;
		}
	}
}

void
button_draw(void *p)
{
	struct button *bu = p;
	SDL_Surface *label = WIDGET_SURFACE(bu,0);
	int x = 0, y = 0;
	int pressed;
	
	if (WIDGET(bu)->w < bu->padding*2 ||
	    WIDGET(bu)->h < bu->padding*2)
		return;

	pressed = widget_get_bool(bu, "state");
	if (bu->flags & BUTTON_INSENSITIVE) {
		primitives.box(bu,
		    0, 0,
		    WIDGET(bu)->w, WIDGET(bu)->h,
		    -1,
		    DISABLED_COLOR);
	} else {
		primitives.box(bu,
		    0, 0,
		    WIDGET(bu)->w, WIDGET(bu)->h,
		    pressed ? -1 : 1,
		    FRAME_COLOR);
	}

	if (label != NULL) {
		switch (bu->justify) {
		case BUTTON_LEFT:
			x = bu->padding;
			break;
		case BUTTON_CENTER:
			x = WIDGET(bu)->w/2 - label->w/2;
			break;
		case BUTTON_RIGHT:
			x = WIDGET(bu)->w - label->w - bu->padding;
			break;
		}
		y = ((WIDGET(bu)->h - label->h)/2) - 1;		/* Middle */

		if (pressed) {
			x++;
			y++;
		}
		widget_blit_surface(bu, 0, x, y);
	}
}

static void
button_mousemotion(int argc, union evarg *argv)
{
	struct button *bu = argv[0].p;
	struct widget_binding *stateb;
	int x = argv[1].i;
	int y = argv[2].i;
	int *pressed;

	if (bu->flags & BUTTON_INSENSITIVE)
		return;

	stateb = widget_get_binding(bu, "state", &pressed);
	if (!widget_relative_area(bu, x, y)) {
		if ((bu->flags & BUTTON_STICKY) == 0
		    && *pressed == 1) {
			*pressed = 0;
			widget_binding_modified(stateb);
		}
		if (bu->flags & BUTTON_MOUSEOVER) {
			bu->flags &= ~(BUTTON_MOUSEOVER);
			event_post(NULL, bu, "button-mouseoverlap", "%i", 0);
		}
	} else {
		bu->flags |= BUTTON_MOUSEOVER;
		event_post(NULL, bu, "button-mouseoverlap", "%i", 1);
	}
	widget_binding_unlock(stateb);
}

static void
button_mousebuttondown(int argc, union evarg *argv)
{
	struct button *bu = argv[0].p;
	int button = argv[1].i;
	struct widget_binding *stateb;
	int *pushed;
	
	if (bu->flags & BUTTON_INSENSITIVE)
		return;

	widget_focus(bu);

	if (button != SDL_BUTTON_LEFT)
		return;
	
	stateb = widget_get_binding(bu, "state", &pushed);
	if (!(bu->flags & BUTTON_STICKY)) {
		*pushed = 1;
	} else {
		*pushed = !(*pushed);
		event_post(NULL, bu, "button-pushed", "%i", *pushed);
	}
	widget_binding_modified(stateb);
	widget_binding_unlock(stateb);

	if (bu->flags & BUTTON_REPEAT) {
		timeout_del(bu, &bu->repeat_to);
		timeout_replace(bu, &bu->delay_to, mouse_spin_delay);
	}
}

static void
button_mousebuttonup(int argc, union evarg *argv)
{
	struct button *bu = argv[0].p;
	int button = argv[1].i;
	struct widget_binding *stateb;
	int *pushed;
	int x = argv[2].i;
	int y = argv[3].i;
		
	if (bu->flags & BUTTON_REPEAT) {
		timeout_del(bu, &bu->repeat_to);
		timeout_del(bu, &bu->delay_to);
	}
	
	if ((bu->flags & BUTTON_INSENSITIVE) ||
	    x < 0 || y < 0 ||
	    x > WIDGET(bu)->w || y > WIDGET(bu)->h) {
		return;
	}
	
	stateb = widget_get_binding(bu, "state", &pushed);
	if (*pushed &&
	    button == SDL_BUTTON_LEFT &&
	    !(bu->flags & BUTTON_STICKY)) {
	    	*pushed = 0;
		event_post(NULL, bu, "button-pushed", "%i", *pushed);
		widget_binding_modified(stateb);
	}
	widget_binding_unlock(stateb);
}

static void
button_keydown(int argc, union evarg *argv)
{
	struct button *bu = argv[0].p;
	int keysym = argv[1].i;
	
	if (bu->flags & BUTTON_INSENSITIVE)
		return;

	if (keysym == SDLK_RETURN || keysym == SDLK_SPACE) {
		widget_set_bool(bu, "state", 1);
		event_post(NULL, bu, "button-pushed", "%i", 1);

		if (bu->flags & BUTTON_REPEAT) {
			timeout_del(bu, &bu->repeat_to);
			timeout_replace(bu, &bu->delay_to, 800);
		}
	}
}

static void
button_keyup(int argc, union evarg *argv)
{
	struct button *bu = argv[0].p;
	int keysym = argv[1].i;
	
	if (bu->flags & BUTTON_INSENSITIVE)
		return;
	
	if (bu->flags & BUTTON_REPEAT) {
		timeout_del(bu, &bu->delay_to);
		timeout_del(bu, &bu->repeat_to);
	}

	if (keysym == SDLK_RETURN || keysym == SDLK_SPACE) {
		widget_set_bool(bu, "state", 0);
		event_post(NULL, bu, "button-pushed", "%i", 0);
	}
}

void
button_enable(struct button *bu)
{
	bu->flags &= ~(BUTTON_INSENSITIVE);
}

void
button_disable(struct button *bu)
{
	bu->flags |= (BUTTON_INSENSITIVE);
}

void
button_set_padding(struct button *bu, int padding)
{
	bu->padding = padding;
}

void
button_set_focusable(struct button *bu, int focusable)
{
	if (focusable) {
		WIDGET(bu)->flags |= WIDGET_FOCUSABLE;
		WIDGET(bu)->flags &= ~(WIDGET_UNFOCUSED_BUTTONUP);
	} else {
		WIDGET(bu)->flags &= ~(WIDGET_FOCUSABLE);
		WIDGET(bu)->flags |= WIDGET_UNFOCUSED_BUTTONUP;
	}
}

void
button_set_sticky(struct button *bu, int sticky)
{
	if (sticky) {
		bu->flags |= (BUTTON_STICKY);
	} else {
		bu->flags &= ~(BUTTON_STICKY);
	}
}

void
button_set_justify(struct button *bu, enum button_justify jus)
{
	bu->justify = jus;
}

void
button_set_label(struct button *bu, SDL_Surface *su)
{
	widget_replace_surface(bu, 0, su);
}

void
button_set_repeat(struct button *bu, int repeat)
{
	if (repeat) {
		bu->flags |= (BUTTON_REPEAT);
	} else {
		timeout_del(bu, &bu->repeat_to);
		timeout_del(bu, &bu->delay_to);
		bu->flags &= ~(BUTTON_REPEAT);
	}
}

void
button_printf(struct button *bu, const char *fmt, ...)
{
	char buf[LABEL_MAX];
	va_list args;

	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	widget_replace_surface(bu, 0,
	    text_render(NULL, -1, WIDGET_COLOR(bu, TEXT_COLOR), buf));
}

