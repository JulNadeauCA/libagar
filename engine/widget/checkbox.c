/*	$Csoft: checkbox.c,v 1.2 2002/04/30 00:57:36 vedge Exp $	*/

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

#include "text.h"
#include "widget.h"
#include "window.h"
#include "checkbox.h"

static struct widget_ops checkbox_ops = {
	{
		checkbox_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL,
		NULL
	},
	checkbox_draw,
	checkbox_event,
	NULL,		/* widget link */
	NULL		/* widget unlink */
};

struct checkbox *
checkbox_new(struct window *win, char *caption, Uint32 flags, Sint16 x,
    Sint16 y)
{
	struct checkbox *cb;

	cb = emalloc(sizeof(struct checkbox));
	checkbox_init(cb, caption, flags, x, y);
	
	pthread_mutex_lock(&win->lock);
	widget_link(cb, win);
	pthread_mutex_unlock(&win->lock);

	return (cb);
}

void
checkbox_init(struct checkbox *b, char *caption, Uint32 flags, Sint16 x,
    Sint16 y)
{
	widget_init(&b->wid, "checkbox", &checkbox_ops, x, y, 0, 0);

	b->caption = strdup(caption);
	b->flags = flags;
	b->justify = CHECKBOX_LEFT;
	b->xspacing = 6;
}

void
checkbox_destroy(void *ob)
{
	struct checkbox *b = (struct checkbox *)ob;

	free(b->caption);
}

void
checkbox_draw(void *p)
{
	static SDL_Color white = { 255, 255, 255 }; /* XXX fgcolor */
	struct checkbox *b = (struct checkbox *)p;
	SDL_Surface *label, *cbox;
	Sint16 x = 0, y = 0;

	/* Checkbox */
	cbox = SPRITE(b, (b->flags & CHECKBOX_PRESSED) ?
	    CHECKBOX_DOWN : CHECKBOX_UP);

	/* Define checkbox geometry accordingly. */
	WIDGET(b)->w = cbox->w;
	WIDGET(b)->h = cbox->h;

	/* Label */
	label = TTF_RenderText_Solid(font, b->caption, white);
	if (label == NULL) {
		fatal("TTF_RenderTextSolid: %s\n", SDL_GetError());
	}

	switch (b->justify) {
	case CHECKBOX_LEFT:
	case CHECKBOX_RIGHT:
		WIDGET_DRAW(b, cbox, 0, 0);
		x = cbox->w + b->xspacing;
		y = 0;
		break;
	}
	WIDGET_DRAW(b, label, x, y);
	SDL_FreeSurface(label);
}

void
checkbox_event(void *p, SDL_Event *ev, Uint32 flags)
{
	struct checkbox *b = (struct checkbox *)p;
	
	if (ev->button.button != 1) {
		return;
	}

	switch (ev->type) {
	case SDL_MOUSEBUTTONDOWN:
		b->flags |= CHECKBOX_PRESSED;
		WIDGET(b)->win->redraw++;
		break;
	case SDL_MOUSEBUTTONUP:
		b->flags &= ~(CHECKBOX_PRESSED);
		WIDGET(b)->win->redraw++;
		if (b->push != NULL)
			b->push(b);
		break;
	}
}

