/*	$Csoft: button.c,v 1.80 2004/03/18 21:27:48 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004 CubeSoft Communications, Inc.
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
		button_destroy,
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

void
button_init(struct button *bu, const char *caption)
{
	widget_init(bu, "button", &button_ops, WIDGET_FOCUSABLE |
	    WIDGET_UNFOCUSED_MOTION);
	widget_bind(bu, "state", WIDGET_BOOL, &bu->state);

	widget_map_color(bu, FRAME_COLOR, "frame", 100, 100, 100, 255);
	widget_map_color(bu, TEXT_COLOR, "text", 240, 240, 240, 255);
	widget_map_color(bu, DISABLED_COLOR, "disabled", 110, 110, 110, 255);

	bu->state = 0;
	bu->sensitive = 1;
	bu->sticky = 0;
	bu->justify = BUTTON_CENTER;
	bu->padding = 4;
	bu->moverlap = 0;

	if (caption != NULL) {
		bu->label = text_render(NULL, -1, WIDGET_COLOR(bu, TEXT_COLOR),
		    caption);
	} else {
		bu->label = NULL;
	}

	event_new(bu, "window-mousebuttonup", button_mousebuttonup, NULL);
	event_new(bu, "window-mousebuttondown", button_mousebuttondown, NULL);
	event_new(bu, "window-mousemotion", button_mousemotion, NULL);
	event_new(bu, "window-keyup", button_keyup, NULL);
	event_new(bu, "window-keydown", button_keydown, NULL);
}

void
button_destroy(void *p)
{
	struct button *bu = p;

	if (bu->label != NULL)
		SDL_FreeSurface(bu->label);

	widget_destroy(bu);
}

void
button_scale(void *p, int w, int h)
{
	struct button *bu = p;

	if (w == -1 && h == -1) {
		if (bu->label != NULL) {
			WIDGET(bu)->w = bu->label->w + bu->padding*2;
			WIDGET(bu)->h = bu->label->h + bu->padding;
		} else {
			WIDGET(bu)->w = 5;
			WIDGET(bu)->h = 5;
		}
	}
}

void
button_draw(void *p)
{
	struct button *bu = p;
	int x = 0, y = 0;
	int pressed;
	
	if (WIDGET(bu)->w < bu->padding*2 ||
	    WIDGET(bu)->h < bu->padding)
		return;

	pressed = widget_get_bool(bu, "state");
	if (!bu->sensitive) {
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

	if (bu->label != NULL) {
		switch (bu->justify) {
		case BUTTON_LEFT:
			x = bu->padding;
			break;
		case BUTTON_CENTER:
			x = WIDGET(bu)->w/2 - bu->label->w/2;
			break;
		case BUTTON_RIGHT:
			x = WIDGET(bu)->w - bu->label->w - bu->padding;
			break;
		}
		y = ((WIDGET(bu)->h - bu->label->h) / 2) - 1;

		if (pressed) {
			x++;
			y++;
		}
		widget_blit(bu, bu->label, x, y);
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

	if (!bu->sensitive)
		return;

	stateb = widget_get_binding(bu, "state", &pressed);
	if (!widget_relative_area(bu, x, y)) {
		if (!bu->sticky && *pressed == 1) {
			*pressed = 0;
			widget_binding_modified(stateb);
		}
		if (bu->moverlap) {
			bu->moverlap = 0;
			event_post(NULL, bu, "button-mouseoverlap", "%i", 0);
		}
	} else {
		bu->moverlap = 1;
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
	
	if (!bu->sensitive)
		return;

	widget_focus(bu);

	if (button != 1)
		return;
	
	stateb = widget_get_binding(bu, "state", &pushed);
	if (!bu->sticky) {
		*pushed = 1;
	} else {
		*pushed = !(*pushed);
		event_post(NULL, bu, "button-pushed", "%i", *pushed);
	}
	widget_binding_modified(stateb);
	widget_binding_unlock(stateb);
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
	
	if (!bu->sensitive ||
	    x < 0 || y < 0 ||
	    x > WIDGET(bu)->w || y > WIDGET(bu)->h)
		return;
	
	stateb = widget_get_binding(bu, "state", &pushed);
	if (*pushed && button == 1 && !bu->sticky) {
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
	
	if (!bu->sensitive)
		return;
	
	if (keysym == SDLK_RETURN || keysym == SDLK_SPACE) {
		widget_set_bool(bu, "state", 1);
		event_post(NULL, bu, "button-pushed", "%i", 1);
	}
}

static void
button_keyup(int argc, union evarg *argv)
{
	struct button *bu = argv[0].p;
	int keysym = argv[1].i;
	
	if (!bu->sensitive)
		return;

	if (keysym == SDLK_RETURN || keysym == SDLK_SPACE) {
		widget_set_bool(bu, "state", 0);
		event_post(NULL, bu, "button-pushed", "%i", 0);
	}
}

void
button_enable(struct button *bu)
{
	bu->sensitive++;
}

void
button_disable(struct button *bu)
{
	bu->sensitive = 0;
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
	bu->sticky = sticky;
}

void
button_set_justify(struct button *bu, enum button_justify jus)
{
	bu->justify = jus;
}

void
button_set_label(struct button *bu, SDL_Surface *su)
{
	if (bu->label != NULL) {
		SDL_FreeSurface(bu->label);
		bu->label = NULL;
	}
	if (su != NULL) {
		bu->label = view_copy_surface(su);
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

	if (bu->label != NULL) {
		SDL_FreeSurface(bu->label);
	}
	bu->label = text_render(NULL, -1, WIDGET_COLOR(bu, TEXT_COLOR), buf);
}

