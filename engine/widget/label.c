/*	$Csoft: label.c,v 1.27 2002/09/06 01:28:47 vedge Exp $	*/

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

#include <engine/compat/vasprintf.h>

#include "text.h"
#include "widget.h"
#include "window.h"
#include "label.h"

static const struct widget_ops label_ops = {
	{
		label_destroy,
		NULL,	/* load */
		NULL	/* save */
	},
	label_draw,
	NULL		/* animate */
};

enum {
	LABEL_TEXT = 0
};

struct label *
label_new(struct region *reg, const char *caption, int flags)
{
	struct label *label;

	label = emalloc(sizeof(struct label));
	label_init(label, caption, flags);

	pthread_mutex_lock(&reg->win->lock);
	region_attach(reg, label);
	pthread_mutex_unlock(&reg->win->lock);

	return (label);
}

/* Window must be locked. */
void
label_printf(struct label *label, const char *fmt, ...)
{
	va_list args;
	char *buf;

	va_start(args, fmt);
	if (vasprintf(&buf, fmt, args) == -1) {
		fatal("vasprintf: %s\n", strerror(errno));
	}
	va_end(args);

	label->caption = erealloc(label->caption, strlen(buf));
	sprintf(label->caption, buf);
	free(buf);

	SDL_FreeSurface(label->label_s);
	label->label_s = text_render(NULL, -1,
	    WIDGET_COLOR(label, LABEL_TEXT), label->caption);

	WIDGET(label)->w = label->label_s->w;
	WIDGET(label)->h = label->label_s->h;
}

void
label_init(struct label *label, const char *caption, int flags)
{
	widget_init(&label->wid, "label", "widget", &label_ops, -1, -1);
	widget_map_color(label, LABEL_TEXT, "label-text", 250, 250, 250);

	label->flags = flags;
	label->caption = strdup(caption);
	label->label_s = text_render(NULL, -1,
	    WIDGET_COLOR(label, LABEL_TEXT), label->caption);

	WIDGET(label)->w = label->label_s->w;
	WIDGET(label)->h = label->label_s->h;
}

void
label_draw(void *p)
{
	struct label *l = p;

	/* XXX justify, etc */
	WIDGET_DRAW(l, l->label_s, 0, 0);
}

void
label_destroy(void *p)
{
	struct label *l = p;

	free(l->caption);
	SDL_FreeSurface(l->label_s);
}
