/*	$Csoft: checkbox.c,v 1.31 2002/12/24 14:21:54 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc. <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of CubeSoft Communications, nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
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

#include <engine/compat/vasprintf.h>

#include <engine/engine.h>

#include <engine/view.h>

#include "primitive.h"
#include "text.h"
#include "widget.h"
#include "window.h"
#include "checkbox.h"

static struct widget_ops checkbox_ops = {
	{
		checkbox_destroy,
		NULL,		/* load */
		NULL		/* save */
	},
	checkbox_draw,
	NULL		/* update */
};

enum {
	BOX_COLOR,
	TEXT_COLOR
};

static void	checkbox_event(int, union evarg *);

struct checkbox *
checkbox_new(struct region *reg, int rh, const char *caption_fmt, ...)
{
	va_list ap;
	struct checkbox *cb;
	char *caption;

	va_start(ap, caption_fmt);
	Vasprintf(&caption, caption_fmt, ap);
	va_end(ap);

	cb = emalloc(sizeof(struct checkbox));
	checkbox_init(cb, rh, caption);
	region_attach(reg, cb);

	free(caption);
	return (cb);
}

void
checkbox_init(struct checkbox *cbox, int rh, char *caption)
{
	widget_init(&cbox->wid, "checkbox", &checkbox_ops, -1, rh);
	widget_map_color(cbox, TEXT_COLOR, "text", 250, 250, 250);
	widget_map_color(cbox, BOX_COLOR, "box", 100, 100, 100);

	widget_bind(cbox, "state", WIDGET_BOOL, NULL, &cbox->def.state);
	cbox->def.state = 0;
	
	cbox->caption = Strdup(caption);
	cbox->label_s = text_render(NULL, -1, WIDGET_COLOR(cbox, TEXT_COLOR),
	    caption);
	
	/* Adjust size. */
	if (rh < 0) {
		WIDGET(cbox)->h = cbox->label_s->h;
	}
	cbox->cbox_w = cbox->label_s->h;			/* Square */
	WIDGET(cbox)->w = cbox->cbox_w + 6 + cbox->label_s->w;

	event_new(cbox, "window-mousebuttonup",
	    checkbox_event, "%i", WINDOW_MOUSEBUTTONUP);
	event_new(cbox, "window-mousebuttondown",
	    checkbox_event, "%i", WINDOW_MOUSEBUTTONDOWN);
	event_new(cbox, "window-keyup",
	    checkbox_event, "%i", WINDOW_KEYUP);
	event_new(cbox, "window-keydown",
	    checkbox_event, "%i", WINDOW_KEYDOWN);
}

void
checkbox_destroy(void *p)
{
	struct checkbox *cbox = p;

	SDL_FreeSurface(cbox->label_s);
	free(cbox->caption);

	widget_destroy(cbox);
}

void
checkbox_draw(void *p)
{
	struct checkbox *cbox = p;
	int x = 0, y = 0;

	/* Draw the checkbox. */
	primitives.box(cbox, 0, 0, cbox->cbox_w, cbox->label_s->h,
	    widget_get_bool(cbox, "state") ? -1 : 1,
	    WIDGET_COLOR(cbox, BOX_COLOR));

	/* Draw the label. */
	WIDGET_DRAW(cbox, cbox->label_s, cbox->cbox_w + 6 + x, y);
}

static void
checkbox_event(int argc, union evarg *argv)
{
	struct checkbox *cbox = argv[0].p;
	int type = argv[1].i;
	int button, keysym;
	int pushed = 0;

	OBJECT_ASSERT(argv[0].p, "widget");

	switch (type) {
	case WINDOW_MOUSEBUTTONDOWN:
		button = argv[2].i;
		if (button == 1) {
			pushed++;
		} else {
			WIDGET_FOCUS(cbox);
		}
		break;
	case WINDOW_KEYDOWN:
		keysym = argv[2].i;
		if (keysym == SDLK_RETURN || keysym == SDLK_SPACE) {
			pushed++;
		}
		break;
	}

	if (pushed) {
		struct widget_binding *stateb;
		int *state;

		stateb = widget_binding_get_locked(cbox, "state", &state);
		*state = !(*state);
		
		event_post(cbox, "checkbox-changed", "%i", *state);

		widget_binding_modified(stateb);
		widget_binding_unlock(stateb);
	}
}

