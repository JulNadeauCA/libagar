/*	$Csoft: button.c,v 1.1 2002/04/20 11:23:52 vedge Exp $	*/

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
#include "widget_offs.h"
#include "window.h"
#include "button.h"

extern TTF_Font *font;		/* text */

static struct widvec button_vec = {
	{
		button_destroy,
		NULL,		/* load */
		NULL,		/* save */
		button_link,	/* link */
		button_unlink	/* unlink */
	},
	button_draw,
	button_event
};

struct button *
button_create(struct window *win, char *name, char *caption, Uint32 flags,
    Uint32 x, Uint32 y, Uint32 w, Uint32 h)
{
	struct button *b;
	struct fobj *fob;
	SDL_Rect rd;

	rd.x = x;
	rd.y = y;
	rd.w = w;	/* variable */
	rd.h = h;	/* variable */

	b = (struct button *)emalloc(sizeof(struct button));
	widget_init(&b->wid, name, 0, &button_vec, win, rd);

	b->caption = strdup(caption);
	b->flags = flags;
	b->justify = BUTTON_CENTER;
	b->xmargin = 6;
	b->ymargin = 6;

	fob = fobj_load(savepath("widget", "fob"));
	if (fob == NULL) {
		return (NULL);
	}
	xcf_load(fob, BUTTON_XCF, OBJECT(b));

	return (b);
}

int
button_destroy(void *ob)
{
	struct button *b = (struct button*)ob;

	free(b->caption);

	return (0);
}

int
button_link(void *p)
{
	return (widget_link(p));
}

int
button_unlink(void *p)
{
	return (widget_unlink(p));
}

void
button_draw(void *p)
{
	static SDL_Color white = { 255, 255, 255 }; /* XXX fgcolor */
	struct button *b = (struct button *)p;
	SDL_Surface *s, *bg;
	Sint16 x = 0, y = 0;

	/* Button */
	bg = SPRITE(b, (b->flags & BUTTON_PRESSED) ? BUTTON_DOWN : BUTTON_UP);
	WIDGET_DRAW(b, bg, 0, 0);

	/* Label */
	s = TTF_RenderText_Solid(font, b->caption, white);
	if (s == NULL) {
		fatal("TTF_RenderTextSolid: %s\n", SDL_GetError());
	}

	switch (b->justify) {
	case BUTTON_LEFT:
		x = b->xmargin;
		y = b->ymargin;
		break;
	case BUTTON_CENTER:
		x = ((bg->w - s->w) >> 1) + (b->xmargin>>1);
		y = ((bg->h - s->h) >> 1);
		break;
	case BUTTON_RIGHT:
		x = bg->w - s->w - b->xmargin;
		y = bg->h - s->h - b->ymargin;
		break;
	}
	if (b->flags & BUTTON_PRESSED) {
		x++;
		y++;
	}

	WIDGET_DRAW(b, s, x, y);
	SDL_FreeSurface(s);
}

void
button_event(void *p, SDL_Event *ev, Uint32 flags)
{
	dprintf("button event\n");
}

