/*	$Csoft: button.c,v 1.52 2003/01/21 03:41:22 vedge Exp $	*/

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

#include "primitive.h"
#include "text.h"
#include "widget.h"
#include "window.h"
#include "button.h"

extern TTF_Font *font;		/* text */

static const struct widget_ops button_ops = {
	{
		button_destroy,
		NULL,		/* load */
		NULL		/* save */
	},
	button_draw,
	NULL		/* update */
};

enum {
	FRAME_COLOR,
	TEXT_COLOR
};

static void	button_mousemotion(int, union evarg *);
static void	button_mousebuttonup(int, union evarg *);
static void	button_mousebuttondown(int, union evarg *);
static void	button_keyup(int, union evarg *);
static void	button_keydown(int, union evarg *);
static void	button_scaled(int, union evarg *);

struct button *
button_new(struct region *reg, char *caption, SDL_Surface *image, int flags,
    int rw, int rh)
{
	struct button *button;

	button = emalloc(sizeof(struct button));
	button_init(button, caption, image, flags, rw, rh);

	region_attach(reg, button);

	return (button);
}

void
button_init(struct button *b, char *caption, SDL_Surface *image, int flags,
    int rw, int rh)
{
	widget_init(&b->wid, "button", &button_ops, rw, rh);
	WIDGET(b)->flags |= WIDGET_UNFOCUSED_MOTION;

	widget_map_color(b, FRAME_COLOR, "frame", 100, 100, 100);
	widget_map_color(b, TEXT_COLOR, "text", 240, 240, 240);

	widget_bind(b, "state", WIDGET_BOOL, NULL, &b->def.state);

	b->flags = flags;
	b->justify = BUTTON_CENTER;
	b->def.state = 0;

	if (caption != NULL) {
		b->caption = Strdup(caption);
		b->label_s = text_render(NULL, -1,
		    WIDGET_COLOR(b, TEXT_COLOR), caption);
	} else if (image != NULL) {
		SDL_Surface *is;

		/* Copy the original surface. XXX */
		is = SDL_ConvertSurface(image, image->format,
		    SDL_SWSURFACE|SDL_SRCALPHA);
		if (is == NULL) {
			fatal("SDL_ConvertSurface: %s\n", SDL_GetError());
		}
		b->caption = NULL;
		b->label_s = is;
	}
	b->slabel_s = NULL;
	
	if (rw == -1)
		WIDGET(b)->w = b->label_s->w + 6;
	if (rh == -1)
		WIDGET(b)->h = b->label_s->h + 6;

	event_new(b, "window-mousebuttonup", button_mousebuttonup, NULL);
	event_new(b, "window-mousebuttondown", button_mousebuttondown, NULL);
	event_new(b, "window-mousemotion", button_mousemotion, NULL);
	event_new(b, "window-keyup", button_keyup, NULL);
	event_new(b, "window-keydown", button_keydown, NULL);
	event_new(b, "widget-scaled", button_scaled, NULL);
}

void
button_destroy(void *p)
{
	struct button *b = p;

	free(b->caption);
	if (b->slabel_s != NULL) {
		SDL_FreeSurface(b->label_s);
	}

	widget_destroy(b);
}

static void
button_scaled(int argc, union evarg *argv)
{
	struct button *b = argv[0].p;
	int x, y, nw, nh;
	Uint32 col = 0;
	Uint8 *src, *dst, r1, g1, b1, a1;

	/* Auto-size the button. */
	if (WIDGET(b)->rw == -1)
		WIDGET(b)->w = b->label_s->w;
	if (WIDGET(b)->rh == -1)
		WIDGET(b)->h = b->label_s->h;

	/* Scale the label. */
	nw = b->label_s->w * WIDGET(b)->h / b->label_s->h;
	if (nw > WIDGET(b)->w - 6)
		nw = WIDGET(b)->w - 6;
	nh = b->label_s->h * WIDGET(b)->w / b->label_s->h;
	if (nh > WIDGET(b)->h - 6)
		nh = WIDGET(b)->h - 6;

	/* Limit to a reasonable size. */
	if (nw > b->label_s->w*2)
		nw = b->label_s->w*2;
	else if (nw < b->label_s->w)
		nw = WIDGET(b)->w;
	if (nh > b->label_s->h*2)
		nh = b->label_s->h*2;
	else if (nh < b->label_s->h)
		nh = WIDGET(b)->h;

	if (b->slabel_s != NULL) {
		SDL_FreeSurface(b->slabel_s);
	}
	b->slabel_s = view_scale_surface(b->label_s, nw, nh);
}

void
button_draw(void *p)
{
	struct button *b = p;
	SDL_Surface *label = b->slabel_s;
	int x = 0, y = 0;
	int pressed;

	pressed = widget_get_bool(b, "state");

	/* Button */
	primitives.box(b, 0, 0, WIDGET(b)->w, WIDGET(b)->h, pressed ? -1 : 1,
	    WIDGET_COLOR(b, FRAME_COLOR));
	
	if (WIDGET(b)->w < 6 || WIDGET(b)->h < 6)
		return;

	/* Label */
	switch (b->justify) {
	case BUTTON_LEFT:
		x = 6;
		break;
	case BUTTON_CENTER:
		x = (WIDGET(b)->w - label->w) / 2;
		break;
	case BUTTON_RIGHT:
		x = WIDGET(b)->w - label->w - 6;
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

	stateb = widget_binding_get_locked(b, "state", &pressed);
	if (!WIDGET_INSIDE_RELATIVE(b, x, y) &&
	    !(b->flags & BUTTON_STICKY) &&
	    *pressed == 1) {
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

	WIDGET_FOCUS(b);

	if (button != 1)
		return;
	
	stateb = widget_binding_get_locked(b, "state", &pushed);
	if ((b->flags & BUTTON_STICKY) == 0) {
		*pushed = 1;
	} else {
		*pushed = !(*pushed);
		event_post(b, "button-pushed", NULL);
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
	
	if (!WIDGET_INSIDE_RELATIVE(b, x, y))
		return;
	
	stateb = widget_binding_get_locked(b, "state", &pushed);
	if (*pushed &&
	    button == 1 &&
	    (b->flags & BUTTON_STICKY) == 0) {
	    	*pushed = 0;
		widget_binding_modified(stateb);

		event_post(b, "button-pushed", NULL);
	}
	widget_binding_unlock(stateb);
}

static void
button_keydown(int argc, union evarg *argv)
{
	struct button *b = argv[0].p;
	int keysym = argv[1].i;
	
	if (keysym == SDLK_RETURN || keysym == SDLK_SPACE) {
		widget_set_bool(b, "state", 1);
	}
}

static void
button_keyup(int argc, union evarg *argv)
{
	struct button *b = argv[0].p;
	int keysym = argv[1].i;

	if (keysym == SDLK_RETURN || keysym == SDLK_SPACE) {
		widget_set_bool(b, "state", 0);
		event_post(b, "button-pushed", NULL);
	}
}

