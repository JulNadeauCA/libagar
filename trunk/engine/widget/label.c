/*	$Csoft: label.c,v 1.13 2002/05/15 07:28:13 vedge Exp $	*/

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

static const struct widget_ops label_ops = {
	{
		label_destroy,
		NULL,	/* load */
		NULL,	/* save */
		NULL,	/* on attach */
		NULL,	/* on detach */
		NULL,	/* attach */
		NULL	/* detach */
	},
	label_draw,
	NULL		/* widget event */
};

struct label *
label_new(struct region *reg, char *caption, int flags)
{
	struct label *label;

	label = emalloc(sizeof(struct label));
	label_init(label, caption, flags);

	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, label);
	pthread_mutex_unlock(&reg->win->lock);

	return (label);
}

void
label_init(struct label *label, char *caption, int flags)
{
	static SDL_Color white = { 255, 255, 255 }; /* XXX fgcolor */
	SDL_Surface *s;

	label->caption = strdup(caption);

	s = TTF_RenderText_Solid(font, label->caption, white);
	if (s == NULL) {
		fatal("TTF_RenderTextSolid: %s\n", SDL_GetError());
	}

	widget_init(&label->wid, "label", "widget", &label_ops, s->w, s->h);
	label->label_s = s;
	label->flags = flags;
}

void
label_draw(void *p)
{
	struct label *l = p;

	WIDGET_DRAW(l, l->label_s, 0, 0);
}

void
label_destroy(void *p)
{
	struct label *l = p;

	free(l->caption);
	SDL_FreeSurface(l->label_s);
}
