/*	$Csoft: button.c,v 1.26 2002/07/20 18:56:31 vedge Exp $	*/

/*
 * Copyright (c) 2002 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistribution of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of CubeSoft Communications, nor the names of its
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
#include <engine/queue.h>
#include <engine/version.h>

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
	NULL		/* animate */
};

enum {
	BUTTON_FRAME	= 0,
	BUTTON_TEXT	= 1
};

static void	button_event(int, union evarg *);

struct button *
button_new(struct region *reg, char *caption, SDL_Surface *image, int flags,
    int rw, int rh)
{
	struct button *button;

	button = emalloc(sizeof(struct button));
	button_init(button, caption, image, flags, rw, rh);

	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, button);
	pthread_mutex_unlock(&reg->win->lock);

	return (button);
}

void
button_init(struct button *b, char *caption, SDL_Surface *image, int flags,
    int rw, int rh)
{
	widget_init(&b->wid, "button", "widget", &button_ops, rw, rh);
	WIDGET(b)->flags |= WIDGET_MOUSEOUT;

	widget_map_color(b, BUTTON_FRAME, "button-frame", 100, 100, 100);
	widget_map_color(b, BUTTON_TEXT, "button-text", 240, 240, 240);

	b->flags = flags;
	b->justify = BUTTON_CENTER;
	b->xmargin = 6;
	b->ymargin = 6;

	if (caption != NULL) {
		b->caption = strdup(caption);
		/* XXX recolor */
		b->label_s = text_render(NULL, -1,
		    WIDGET(b)->color[BUTTON_TEXT], caption);
	} else if (image != NULL) {
		b->caption = NULL;
		b->label_s = image;
	}
	
	if (rw == -1)
		WIDGET(b)->w = b->label_s->w + b->xmargin;
	if (rh == -1)
		WIDGET(b)->h = b->label_s->h + b->ymargin;

	event_new(b, "window-mousebuttonup", 0,
	    button_event, "%i", WINDOW_MOUSEBUTTONUP);
	event_new(b, "window-mousebuttondown", 0,
	    button_event, "%i", WINDOW_MOUSEBUTTONDOWN);
	event_new(b, "window-keyup", 0,
	    button_event, "%i", WINDOW_KEYUP);
	event_new(b, "window-keydown", 0,
	    button_event, "%i", WINDOW_KEYDOWN);
	event_new(b, "window-mouseout", 0,
	    button_event, "%i", WINDOW_MOUSEOUT);
}

void
button_destroy(void *p)
{
	struct button *b = p;

	OBJECT_ASSERT(p, "widget");

	free(b->caption);
}

void
button_draw(void *p)
{
	struct button *b = p;
	SDL_Surface *label = b->label_s;
	int x = 0, y = 0;

	OBJECT_ASSERT(p, "widget");

	/* Button */
	primitives.box(b, 0, 0, WIDGET(b)->w, WIDGET(b)->h,
	    (b->flags & BUTTON_PRESSED) ? -1 : 1,
	    WIDGET(b)->color[BUTTON_FRAME]);

	/* Label */
	switch (b->justify) {
	case BUTTON_LEFT:
		x = b->xmargin;
		break;
	case BUTTON_CENTER:
		x = ((WIDGET(b)->w - label->w) >> 1) + (b->xmargin>>2) - 1;
		break;
	case BUTTON_RIGHT:
		x = WIDGET(b)->w - label->w - b->xmargin;
		break;
	}
	y = ((WIDGET(b)->h - label->h) >> 1) - 1;

	if (b->flags & BUTTON_PRESSED) {
		x++;
		y++;
	}
	WIDGET_DRAW(b, b->label_s, x, y);
}

static void
button_event(int argc, union evarg *argv)
{
	struct button *b = argv[0].p;
	int type = argv[1].i;
	int button, keysym;
	int pushed = 0;
	
	WIDGET_ASSERT(b, "button");

	switch (type) {
	case WINDOW_MOUSEOUT:
		b->flags &= ~(BUTTON_PRESSED);
		break;
	case WINDOW_MOUSEBUTTONDOWN:
		button = argv[2].i;
		WIDGET_FOCUS(b);
		if (button == 1) {
			b->flags |= BUTTON_PRESSED;
		}
		break;
	case WINDOW_MOUSEBUTTONUP:
		button = argv[2].i;
		if (button == 1) {
			b->flags &= ~(BUTTON_PRESSED);
			pushed++;
		}
		break;
	case WINDOW_KEYDOWN:
		keysym = argv[2].i;
		if (keysym == SDLK_RETURN || keysym == SDLK_SPACE) {
			b->flags |= BUTTON_PRESSED;
		}
		break;
	case WINDOW_KEYUP:
		keysym = argv[2].i;
		if (keysym == SDLK_RETURN || keysym == SDLK_SPACE) {
			b->flags &= ~(BUTTON_PRESSED);
			pushed++;
		}
		break;
	}
	
	if (pushed) {
		event_post(b, "button-pushed", NULL);
	}
}

