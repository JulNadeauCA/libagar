/*	$Csoft: checkbox.c,v 1.50 2004/03/18 21:27:48 vedge Exp $	*/

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

#include "checkbox.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>

static struct widget_ops checkbox_ops = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		checkbox_destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	checkbox_draw,
	checkbox_scale
};

enum {
	XSPACING =	6
};

enum {
	BOX_COLOR,
	TEXT_COLOR
};

static void	checkbox_mousebutton(int , union evarg *);
static void	checkbox_keydown(int , union evarg *);

struct checkbox *
checkbox_new(void *parent, const char *fmt, ...)
{
	char caption[CHECKBOX_CAPTION_MAX];
	struct checkbox *cb;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(caption, sizeof(caption), fmt, ap);
	va_end(ap);

	cb = Malloc(sizeof(struct checkbox), M_OBJECT);
	checkbox_init(cb, caption);
	object_attach(parent, cb);
	return (cb);
}

void
checkbox_init(struct checkbox *cbox, char *caption)
{
	widget_init(cbox, "checkbox", &checkbox_ops, WIDGET_FOCUSABLE);
	widget_bind(cbox, "state", WIDGET_BOOL, &cbox->state);

	widget_map_color(cbox, TEXT_COLOR, "text", 250, 250, 250, 255);
	widget_map_color(cbox, BOX_COLOR, "box", 100, 100, 100, 255);

	cbox->state = 0;
	cbox->label_s = text_render(NULL, -1, WIDGET_COLOR(cbox, TEXT_COLOR),
	    caption);
	
	event_new(cbox, "window-mousebuttondown", checkbox_mousebutton, NULL);
	event_new(cbox, "window-keydown", checkbox_keydown, NULL);
}

void
checkbox_destroy(void *p)
{
	struct checkbox *cbox = p;

	SDL_FreeSurface(cbox->label_s);
	widget_destroy(cbox);
}

void
checkbox_draw(void *p)
{
	struct checkbox *cbox = p;

	primitives.box(cbox,
	    0, 0,
	    WIDGET(cbox)->h, WIDGET(cbox)->h,
	    widget_get_bool(cbox, "state") ? -1 : 1,
	    BOX_COLOR);

	widget_blit(cbox, cbox->label_s, WIDGET(cbox)->h + XSPACING, 0);
}

static void
checkbox_mousebutton(int argc, union evarg *argv)
{
	struct checkbox *cbox = argv[0].p;
	int button = argv[1].i;

	widget_focus(cbox);

	if (button == SDL_BUTTON(1))
		checkbox_toggle(cbox);
}

static void
checkbox_keydown(int argc, union evarg *argv)
{
	struct checkbox *cbox = argv[0].p;
	SDLKey key = argv[1].i;

	switch (key) {
	case SDLK_RETURN:
	case SDLK_SPACE:
		checkbox_toggle(cbox);
		break;
	default:
		break;
	}
}

void
checkbox_scale(void *p, int rw, int rh)
{
	struct checkbox *cb = p;

	if (rh == -1)
		WIDGET(cb)->h = cb->label_s->h;
	if (rw == -1)
		WIDGET(cb)->w = WIDGET(cb)->h + XSPACING + cb->label_s->w;
}

/* Toggle the checkbox state. */
void
checkbox_toggle(struct checkbox *cbox)
{
	struct widget_binding *stateb;
	int *state;

	stateb = widget_get_binding(cbox, "state", &state);
	*state = !(*state);
	event_post(NULL, cbox, "checkbox-changed", "%i", *state);
	widget_binding_modified(stateb);
	widget_binding_unlock(stateb);
}

