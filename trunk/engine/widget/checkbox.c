/*	$Csoft: checkbox.c,v 1.7 2002/05/19 14:30:24 vedge Exp $	*/

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

	widget_init(&cbox->wid, "checkbox", "widget", &checkbox_ops, -1, -1);
	cbox->caption = strdup(caption);
	cbox->flags = flags;
	cbox->justify = CHECKBOX_LEFT;
	cbox->xspacing = 6;
	cbox->cbox_w = 16;
	
	/* Label */
	cbox->label_s = TTF_RenderText_Solid(font, cbox->caption, white);
	if (cbox->label_s == NULL) {
		fatal("TTF_RenderTextSolid: %s\n", SDL_GetError());
	}

	WIDGET(cbox)->w = cbox->cbox_w + cbox->xspacing + cbox->label_s->w;
	WIDGET(cbox)->h = cbox->label_s->h;
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
	SDL_Rect rd;
	SDL_Surface *box_s;
	int x = 0, y = 0;

	/* Checkbox */
	/* Checkbox */
	box_s = view_surface(SDL_SWSURFACE, cbox->cbox_w, cbox->label_s->h);
	SDL_FillRect(box_s, NULL, SDL_MapRGBA(box_s->format,
	    255, 255, 255, 255));
	if (cbox->flags & CHECKBOX_PRESSED) {
		rd.x = 2;
		rd.y = 2;
		rd.w = box_s->w - 2;
		rd.h = box_s->h - 2;
		SDL_FillRect(box_s, NULL,
		    SDL_MapRGBA(box_s->format, 50, 50, 50, 120));
		SDL_FillRect(box_s, &rd,
		    SDL_MapRGBA(box_s->format, 96, 96, 128, 120));
	} else {
		rd.x = 1;
		rd.y = 1;
		rd.w = box_s->w - 1;
		rd.h = box_s->h - 1;
		SDL_FillRect(box_s, NULL,
		    SDL_MapRGBA(box_s->format, 180, 180, 180, 120));
		SDL_FillRect(box_s, &rd,
		    SDL_MapRGBA(box_s->format, 96, 96, 128, 120));
	}

	switch (cbox->justify) {
	case CHECKBOX_LEFT:
	case CHECKBOX_RIGHT:
		WIDGET_DRAW(cbox, box_s, 0, 0);
		x = box_s->w + cbox->xspacing;
		y = 0;
		break;
	}
	WIDGET_DRAW(cbox, cbox->label_s, x, y);
}

void
checkbox_event(void *p, SDL_Event *ev, int flags)
{
	struct checkbox *cbox = p;

	dprintf("%s\n", OBJECT(cbox)->name);

	if (ev->button.button != 1) {
		return;
	}

	switch (ev->type) {
	case SDL_MOUSEBUTTONDOWN:
		cbox->flags |= CHECKBOX_PRESSED;
		WIDGET(cbox)->win->redraw++;
		if (cbox->push != NULL) {
			cbox->push(cbox);
		}
		break;
	case SDL_MOUSEBUTTONUP:
		cbox->flags &= ~(CHECKBOX_PRESSED);
		WIDGET(cbox)->win->redraw++;
		break;
	}
}

