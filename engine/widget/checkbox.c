/*	$Csoft: checkbox.c,v 1.10 2002/05/28 05:59:39 vedge Exp $	*/

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
		NULL,		/* save */
		NULL,		/* on attach */
		NULL,		/* on detach */
		NULL,		/* attach */
		NULL		/* detach */
	},
	checkbox_draw,
	checkbox_event
};

struct checkbox *
checkbox_new(struct region *reg, char *caption, int flags)
{
	struct checkbox *cb;

	cb = emalloc(sizeof(struct checkbox));
	checkbox_init(cb, caption, flags);

	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, cb);
	pthread_mutex_unlock(&reg->win->lock);

	return (cb);
}

void
checkbox_init(struct checkbox *cbox, char *caption, int flags)
{
	static SDL_Color white = { 255, 255, 255 }; /* XXX fgcolor */
	SDL_Surface *s;
	
	s = TTF_RenderText_Solid(font, caption, white);
	if (s == NULL) {
		fatal("TTF_RenderTextSolid: %s\n", SDL_GetError());
	}

	cbox->cbox_w = 16;	/* XXX */
	cbox->xspacing = 6;

	widget_init(&cbox->wid, "checkbox", "widget", &checkbox_ops,
	    cbox->cbox_w + cbox->xspacing + s->w, s->h);
	cbox->caption = strdup(caption);
	cbox->flags = flags;
	cbox->justify = CHECKBOX_LEFT;
	cbox->label_s = s;
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
	SDL_Surface *box_s;
	int x = 0, y = 0;

	/* Checkbox */
	box_s = primitive_box(cbox, cbox->cbox_w, cbox->label_s->h,
	    (cbox->flags & CHECKBOX_PRESSED) ? -1 : 1);

	/* Label (cached) */
	switch (cbox->justify) {
	case CHECKBOX_LEFT:
	case CHECKBOX_RIGHT:
		WIDGET_DRAW(cbox, box_s, 0, 0);
		x = box_s->w + cbox->xspacing;
		y = 0;
		break;
	}
	SDL_FreeSurface(box_s);

	WIDGET_DRAW(cbox, cbox->label_s, x, y);
}

void
checkbox_event(void *p, SDL_Event *ev, int flags)
{
	struct checkbox *cbox = p;
	int pushed = 0;

	switch (ev->type) {
	case SDL_MOUSEBUTTONDOWN:
		if (ev->button.button == 1) {
			pushed++;
		} else {
			WIDGET_FOCUS(cbox);
		}
		break;
	case SDL_KEYDOWN:
		if (ev->key.keysym.sym == SDLK_RETURN ||
		    ev->key.keysym.sym == SDLK_SPACE) {
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
		if (cbox->push != NULL) {
			cbox->push(cbox);
		}
	}
}

