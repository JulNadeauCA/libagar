/*	$Csoft: dummy.c,v 1.2 2002/08/20 09:17:10 vedge Exp $	*/

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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <engine/engine.h>
#include <engine/queue.h>
#include <engine/version.h>

#include <engine/compat/vasprintf.h>

#include "text.h"
#include "widget.h"
#include "window.h"
#include "dummy.h"

static const struct widget_ops dummy_ops = {
	{
		dummy_destroy,
		NULL,	/* load */
		NULL	/* save */
	},
	dummy_draw,
	NULL		/* animate */
};

static SDL_Color white = { 255, 255, 255 }; /* XXX fgcolor */

struct dummy *
dummy_new(struct region *reg, const char *caption, int flags)
{
	struct dummy *dummy;

	dummy = emalloc(sizeof(struct dummy));
	dummy_init(dummy, caption, flags);

	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, dummy);
	pthread_mutex_unlock(&reg->win->lock);

	return (dummy);
}

/* Window must be locked. */
void
dummy_printf(struct dummy *dummy, const char *fmt, ...)
{
	va_list args;
	char *buf;
	SDL_Surface *s;

	va_start(args, fmt);
	vasprintf(&buf, fmt, args);
	va_end(args);

	dummy->caption = erealloc(dummy->caption, strlen(buf));
	sprintf(dummy->caption, buf);
	free(buf);

	SDL_FreeSurface(dummy->dummy_s);
	s = TTF_RenderText_Solid(font, dummy->caption, white);
	if (s == NULL) {
		fatal("TTF_RenderTextSolid: %s\n", SDL_GetError());
	}
	dummy->dummy_s = s;
	WIDGET(dummy)->w = s->w;
	WIDGET(dummy)->h = s->h;
	WIDGET(dummy)->win->redraw++;
}

void
dummy_init(struct dummy *dummy, const char *caption, int flags)
{
	SDL_Surface *s;

	dummy->caption = strdup(caption);
	s = TTF_RenderText_Solid(font, dummy->caption, white);
	if (s == NULL) {
		fatal("TTF_RenderTextSolid: %s\n", SDL_GetError());
	}

	widget_init(&dummy->wid, "dummy", "widget", &dummy_ops, s->w, s->h);
	dummy->dummy_s = s;
	dummy->flags = flags;
}

void
dummy_draw(void *p)
{
	struct dummy *l = p;

	primitives.box(l, 0, 0, WIDGET(dum)->w, WIDGET(dum)->h, 0);
}

void
dummy_destroy(void *p)
{
	struct dummy *l = p;

	free(l->caption);
}
