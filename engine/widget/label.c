/*	$Csoft: label.c,v 1.2 2002/04/20 06:21:19 vedge Exp $	*/

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
#include "label.h"

extern TTF_Font *font;		/* text */

static struct widvec label_vec = {
	{
		label_destroy,
		NULL,		/* load */
		NULL,		/* save */
		label_link,	/* link */
		label_unlink	/* unlink */
	},
	label_draw
};

struct label *
label_create(struct window *win, char *name, char *caption, Uint32 flags,
    Sint16 x, Sint16 y)
{
	struct label *l;
	SDL_Rect rd;

	rd.x = x;
	rd.y = y;
	rd.w = 0;	/* variable */
	rd.h = 0;	/* variable */

	l = (struct label *)emalloc(sizeof(struct label));
	widget_init(&l->wid, name, 0, &label_vec, win, rd);

	l->caption = strdup(caption);
	l->flags = flags;

	return (l);
}

int
label_destroy(void *ob)
{
	struct label *l = (struct label*)ob;

	free(l->caption);

	return (0);
}

int
label_link(void *p)
{
	return (widget_link(p));
}

int
label_unlink(void *p)
{
	return (widget_unlink(p));
}

void
label_draw(void *p)
{
	static SDL_Color white = { 255, 255, 255 }; /* XXX fgcolor */
	struct label *l = (struct label *)p;
	SDL_Surface *s;

	s = TTF_RenderText_Solid(font, l->caption, white);
	if (s == NULL) {
		fatal("TTF_RenderTextSolid: %s\n", SDL_GetError());
	}
	WIDGET_DRAW(l, s);
	SDL_FreeSurface(s);
}

