/*	$Csoft: label.c,v 1.31 2002/11/14 07:18:33 vedge Exp $	*/

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
#include <errno.h>

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
	NULL		/* update */
};

enum {
	TEXT_COLOR
};

struct label *
label_new(struct region *reg, int w, int h, const char *fmt, ...)
{
	struct label *label;
	va_list args;
	char *buf;

	va_start(args, fmt);
	if (vasprintf(&buf, fmt, args) == -1) {
		fatal("vasprintf: %s\n", strerror(errno));
	}
	va_end(args);

	label = emalloc(sizeof(struct label));
	label_init(label, buf, w, h);

	free(buf);

	region_attach(reg, label);

	return (label);
}

void
label_init(struct label *label, const char *caption, int rw, int rh)
{
	widget_init(&label->wid, "label", "widget", &label_ops, rw, rh);
	widget_map_color(label, TEXT_COLOR, "label-text", 250, 250, 250);

	label->flags = 0;
	label->text.caption = strdup(caption);
	label->text.surface = text_render(NULL, -1,
	    WIDGET_COLOR(label, TEXT_COLOR), label->text.caption);
	pthread_mutex_init(&label->text.lock, NULL);
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

	pthread_mutex_lock(&label->text.lock);

	label->text.caption = erealloc(label->text.caption, strlen(buf));
	sprintf(label->text.caption, buf);
	free(buf);

	SDL_FreeSurface(label->text.surface);
	label->text.surface = text_render(NULL, -1,
	    WIDGET_COLOR(label, TEXT_COLOR), label->text.caption);

	WIDGET(label)->w = label->text.surface->w;
	WIDGET(label)->h = label->text.surface->h;

	pthread_mutex_unlock(&label->text.lock);
}

void
label_draw(void *p)
{
	struct label *label = p;

	/* XXX justify, etc */
	pthread_mutex_lock(&label->text.lock);
	WIDGET_DRAW(label, label->text.surface, 0, 0);
	pthread_mutex_unlock(&label->text.lock);
}

void
label_destroy(void *p)
{
	struct label *label = p;

	free(label->text.caption);
	SDL_FreeSurface(label->text.surface);
	pthread_mutex_destroy(&label->text.lock);
}
