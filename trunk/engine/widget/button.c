/*	$Csoft: button.c,v 1.70 2003/06/08 00:21:04 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
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

#include "button.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

const struct widget_ops button_ops = {
	{
		NULL,		/* init */
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

	button = Malloc(sizeof(struct button));
	button_init(button, caption);
	object_attach(parent, button);
	return (button);
}

void
button_init(struct button *b, const char *caption)
{
	widget_init(b, "button", &button_ops,
	    WIDGET_FOCUSABLE|WIDGET_UNFOCUSED_MOTION);
	widget_bind(b, "state", WIDGET_BOOL, NULL, &b->state);

	widget_map_color(b, FRAME_COLOR, "frame", 100, 100, 100, 255);
	widget_map_color(b, TEXT_COLOR, "text", 240, 240, 240, 255);
	widget_map_color(b, DISABLED_COLOR, "disabled", 110, 110, 110, 255);

	b->state = 0;
	b->sensitive = 1;
	b->sticky = 0;
	b->justify = BUTTON_CENTER;
	b->padding = 2;

	b->label_s = (caption != NULL) ?
	    text_render(NULL, -1, WIDGET_COLOR(b, TEXT_COLOR), caption) :
	    NULL;
	b->slabel_s = NULL;

	event_new(b, "window-mousebuttonup", button_mousebuttonup, NULL);
	event_new(b, "window-mousebuttondown", button_mousebuttondown, NULL);
	event_new(b, "window-mousemotion", button_mousemotion, NULL);
	event_new(b, "window-keyup", button_keyup, NULL);
	event_new(b, "window-keydown", button_keydown, NULL);
}

void
button_destroy(void *p)
{
	struct button *b = p;

	if (b->slabel_s != NULL) {
		SDL_FreeSurface(b->label_s);
	}
	widget_destroy(b);
}

void
button_scale(void *p, int w, int h)
{
	struct button *b = p;
	int nw, nh;

	if (w == -1 && h == -1) {
		WIDGET(b)->w = b->label_s->w + b->padding*2;
		WIDGET(b)->h = b->label_s->h + b->padding*2;
	}

	if (b->padding == 0) {
		nw = WIDGET(b)->w;
		nh = WIDGET(b)->h;
	} else {
		/* Scale the label to a reasonable size. */
		nw = b->label_s->w * WIDGET(b)->h / b->label_s->h;
		if (nw > WIDGET(b)->w - 1)
			nw = WIDGET(b)->w - 1;
		nh = b->label_s->h * WIDGET(b)->w / b->label_s->h;
		if (nh > WIDGET(b)->h - 1)
			nh = WIDGET(b)->h - 1;
		if (nw > b->label_s->w*2)
			nw = b->label_s->w*2;
		if (nh > b->label_s->h*2)
			nh = b->label_s->h*2;
	}

	if (b->slabel_s != NULL) {
		SDL_FreeSurface(b->slabel_s);
	}
	if (nw < 6 || nh < 6) {
		b->slabel_s = view_surface(SDL_SWSURFACE, 0, 0);
	} else {
		b->slabel_s = view_scale_surface(b->label_s, nw, nh);
	}
}

void
button_draw(void *p)
{
	const int xspace = 2;
	struct button *b = p;
	SDL_Surface *label = b->slabel_s;
	int x = 0, y = 0;
	int pressed;
	
	if (WIDGET(b)->w < b->padding*2 ||
	    WIDGET(b)->h < b->padding*2)
		return;

	pressed = widget_get_bool(b, "state");
	if (!b->sensitive) {
		primitives.box(b,
		    0, 0,
		    WIDGET(b)->w, WIDGET(b)->h,
		    -1,
		    DISABLED_COLOR);
	} else {
		primitives.box(b,
		    0, 0,
		    WIDGET(b)->w, WIDGET(b)->h,
		    pressed ? -1 : 1,
		    FRAME_COLOR);
	}

	switch (b->justify) {
	case BUTTON_LEFT:
		x = xspace;
		break;
	case BUTTON_CENTER:
		x = (WIDGET(b)->w - label->w) / 2;
		break;
	case BUTTON_RIGHT:
		x = WIDGET(b)->w - label->w - xspace;
		break;
	}
	y = ((WIDGET(b)->h - label->h) / 2) - 1;

	if (pressed) {
		x++;
		y++;
	}
	widget_blit(b, label, x, y);
}

static void
button_mousemotion(int argc, union evarg *argv)
{
	struct button *b = argv[0].p;
	struct widget_binding *stateb;
	int x = argv[1].i;
	int y = argv[2].i;
	int *pressed;

	if (!b->sensitive)
		return;

	stateb = widget_binding_get_locked(b, "state", &pressed);
	if (!widget_relative_area(b, x, y) &&
	    !b->sticky && *pressed == 1) {
		*pressed = 0;
		widget_binding_modified(stateb);
	}
	widget_binding_unlock(stateb);
}

static void
button_mousebuttondown(int argc, union evarg *argv)
{
	struct button *b = argv[0].p;
	int button = argv[1].i;
	struct widget_binding *stateb;
	int *pushed;
	
	if (!b->sensitive)
		return;

	widget_focus(b);

	if (button != 1)
		return;
	
	stateb = widget_binding_get_locked(b, "state", &pushed);
	if (!b->sticky) {
		*pushed = 1;
	} else {
		*pushed = !(*pushed);
		event_post(b, "button-pushed", "%i", *pushed);
	}
	widget_binding_modified(stateb);
	widget_binding_unlock(stateb);
}

static void
button_mousebuttonup(int argc, union evarg *argv)
{
	struct button *b = argv[0].p;
	int button = argv[1].i;
	struct widget_binding *stateb;
	int *pushed;
	int x = argv[2].i;
	int y = argv[3].i;
	
	if (!b->sensitive ||
	    x < 0 || y < 0 || x > WIDGET(b)->w || y > WIDGET(b)->h)
		return;
	
	stateb = widget_binding_get_locked(b, "state", &pushed);
	if (*pushed && button == 1 && !b->sticky) {
	    	*pushed = 0;
		event_post(b, "button-pushed", "%i", *pushed);
		widget_binding_modified(stateb);
	}
	widget_binding_unlock(stateb);
}

static void
button_keydown(int argc, union evarg *argv)
{
	struct button *b = argv[0].p;
	int keysym = argv[1].i;
	
	if (!b->sensitive)
		return;
	
	if (keysym == SDLK_RETURN || keysym == SDLK_SPACE) {
		widget_set_bool(b, "state", 1);
		event_post(b, "button-pushed", "%i", 1);
	}
}

static void
button_keyup(int argc, union evarg *argv)
{
	struct button *b = argv[0].p;
	int keysym = argv[1].i;
	
	if (!b->sensitive)
		return;

	if (keysym == SDLK_RETURN || keysym == SDLK_SPACE) {
		widget_set_bool(b, "state", 0);
		event_post(b, "button-pushed", "%i", 0);
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
	if (bu->label_s != NULL) {
		SDL_FreeSurface(bu->label_s);
	}
	bu->label_s = view_copy_surface(su);
}

