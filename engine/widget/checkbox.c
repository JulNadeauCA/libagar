/*	$Csoft: checkbox.c,v 1.20 2002/07/30 22:23:57 vedge Exp $	*/

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

#include <sys/types.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <engine/engine.h>
#include <engine/version.h>

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
	NULL		/* animate */
};

enum {
	FRAME_COLOR,
	TEXT_COLOR
};

static void	checkbox_event(int, union evarg *);

struct checkbox *
checkbox_new(struct region *reg, char *caption, int rh, int flags)
{
	struct checkbox *cb;

	cb = emalloc(sizeof(struct checkbox));
	checkbox_init(cb, caption, rh, flags);

	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, cb);
	pthread_mutex_unlock(&reg->win->lock);

	return (cb);
}

void
checkbox_init(struct checkbox *cbox, char *caption, int rh, int flags)
{
	SDL_Surface *s;

	cbox->cbox_w = 16;	/* XXX */
	cbox->xspacing = 6;

	widget_init(&cbox->wid, "checkbox", "widget", &checkbox_ops, -1, rh);
	
	widget_map_color(cbox, FRAME_COLOR, "checkbox-frame", 100, 100, 100);
	widget_map_color(cbox, TEXT_COLOR, "checkbox-text", 250, 250, 250);
	
	s = text_render(NULL, -1, WIDGET_COLOR(cbox, TEXT_COLOR), caption);
	if (rh < 0) {
		WIDGET(cbox)->h = s->h;
	}
	WIDGET(cbox)->w = cbox->cbox_w + cbox->xspacing + s->w;

	cbox->caption = strdup(caption);
	cbox->flags = flags;
	cbox->justify = CHECKBOX_LEFT;
	cbox->label_s = s;
	
	event_new(cbox, "window-mousebuttonup", 0,
	    checkbox_event, "%i", WINDOW_MOUSEBUTTONUP);
	event_new(cbox, "window-mousebuttondown", 0,
	    checkbox_event, "%i", WINDOW_MOUSEBUTTONDOWN);
	event_new(cbox, "window-keyup", 0,
	    checkbox_event, "%i", WINDOW_KEYUP);
	event_new(cbox, "window-keydown", 0,
	    checkbox_event, "%i", WINDOW_KEYDOWN);
}

void
checkbox_destroy(void *p)
{
	struct checkbox *b = p;

	SDL_FreeSurface(b->label_s);
	free(b->caption);
}

void
checkbox_draw(void *p)
{
	struct checkbox *cbox = p;
	int x = 0, y = 0;

	/* Checkbox */
	primitives.box(cbox, 0, 0, cbox->cbox_w, cbox->label_s->h,
	    (cbox->flags & CHECKBOX_PRESSED) ? -1 : 1,
	    WIDGET_COLOR(cbox, FRAME_COLOR));

	/* Label (cached) */
	switch (cbox->justify) {
	case CHECKBOX_LEFT:
	case CHECKBOX_RIGHT:
		x = cbox->cbox_w + cbox->xspacing;
		y = 0;
		break;
	}
	WIDGET_DRAW(cbox, cbox->label_s, x, y);
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
		WIDGET(cbox)->win->redraw++;

		if (cbox->flags & CHECKBOX_PRESSED) {
			cbox->flags &= ~(CHECKBOX_PRESSED);
		} else {
			cbox->flags |= CHECKBOX_PRESSED;
		}
		event_post(cbox, "checkbox-changed", "%i",
		    (cbox->flags & CHECKBOX_PRESSED));
	}
}

