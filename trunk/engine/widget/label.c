/*	$Csoft: label.c,v 1.39 2002/11/28 07:19:45 vedge Exp $	*/

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

#include <engine/compat/asprintf.h>
#include <engine/compat/vasprintf.h>
#include <engine/compat/strlcat.h>
#include <engine/engine.h>

#include <engine/view.h>

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
	label_init(label, LABEL_STATIC, buf, w, h);

	free(buf);

	region_attach(reg, label);

	return (label);
}

void
label_init(struct label *label, enum label_type type, const char *s,
    int rw, int rh)
{
	widget_init(&label->wid, "label", "widget", &label_ops, rw, rh);
	widget_map_color(label, TEXT_COLOR, "label-text", 250, 250, 250);

	label->type = type;
	switch (type) {
	case LABEL_STATIC:
		label->text.caption = strdup(s);
		label->text.surface = text_render(NULL, -1,
		    WIDGET_COLOR(label, TEXT_COLOR), label->text.caption);
		pthread_mutex_init(&label->text.lock, NULL);
		break;
	case LABEL_POLLED:
		label->text.caption = NULL;
		label->text.surface = NULL;
		label->poll.fmt = strdup(s);

		memset(label->poll.ptrs, 0,
		    sizeof(void *) * LABEL_MAX_POLLITEMS);
		label->poll.nptrs = 0;
		break;
	}
}

struct label *
label_polled_new(struct region *reg, int w, int h, pthread_mutex_t *mutex,
    const char *fmt, ...)
{
	struct label *label;
	va_list args;
	const char *p;

	label = emalloc(sizeof(struct label));
	label_init(label, LABEL_POLLED, fmt, w, h);

	label->poll.lock = mutex;

	va_start(args, fmt);
	for (p = fmt; *p != '\0'; p++) {
		if (*p == '%' && *(p+1) != '\0') {
			switch (*(p+1)) {
			case ' ':
			case '(':
			case ')':
			case '%':
				break;
			default:
				label->poll.ptrs[label->poll.nptrs++] =
				    va_arg(args, void *);
				break;
			}
		}
	}
	va_end(args);

	region_attach(reg, label);

	return (label);
}

/* Window must be locked. XXX */
void
label_printf(struct label *label, const char *fmt, ...)
{
	va_list args;
	char *buf;

#ifdef DEBUG
	if (label->type != LABEL_STATIC) {
		fatal("not a static label\n");
	}
#endif

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
	
	/* XXX justify */

	switch (label->type) {
	case LABEL_STATIC:
		pthread_mutex_lock(&label->text.lock);
		WIDGET_DRAW(label, label->text.surface, 0, 0);
		pthread_mutex_unlock(&label->text.lock);
		break;
	case LABEL_POLLED:
		{
			SDL_Surface *ts;
			char s[LABEL_MAX_LENGTH], *s2, *fmtp, *sp;
			int ri = 0;
			
			s[0] = '\0';

			if (label->poll.lock != NULL)
				pthread_mutex_lock(label->poll.lock);

			for (sp = s, fmtp = label->poll.fmt;
			     *fmtp != '\0';
			     fmtp++) {

#define LABEL_ARG(fmt, _la_type)					\
	asprintf(&s2, (fmt), *(_la_type *)label->poll.ptrs[ri++])

				if (*fmtp == '%' && *(fmtp+1) != '\0') {
					switch (*(fmtp+1)) {
					case 'd':
					case 'i':
						LABEL_ARG("%d", int);
						break;
					case 'o':
						LABEL_ARG("%o", unsigned int);
						break;
					case 'u':
						LABEL_ARG("%u", unsigned int);
						break;
					case 'x':
						LABEL_ARG("%x", int);
						break;
					case 'X':
						LABEL_ARG("%X", unsigned int);
						break;
					case 'c':
						LABEL_ARG("%c", char);
						break;
					case 's':
						LABEL_ARG("%s", char *);
						break;
					case 'p':
						LABEL_ARG("%p", void *);
						break;
					case '[':
						if (strncmp("u8", fmtp+2, 2)
						    == 0) {
							LABEL_ARG("%d", Uint8);
							fmtp += 3;
						} else if (strncmp("s8",
						           fmtp+2, 2) == 0) {
							LABEL_ARG("%d", Sint8);
							fmtp += 3;
						} else if (strncmp("u16",
						           fmtp+2, 3) == 0) {
							LABEL_ARG("%d", Uint16);
							fmtp += 4;
						} else if (strncmp("s16",
						           fmtp+2, 3) == 0) {
							LABEL_ARG("%d", Sint16);
							fmtp += 4;
						} else if (strncmp("u32",
						           fmtp+2, 3) == 0) {
							LABEL_ARG("%d", Uint32);
							fmtp += 4;
						} else if (strncmp("s32",
						           fmtp+2, 3) == 0) {
							LABEL_ARG("%d", Sint32);
							fmtp += 4;
						} else if (strncmp("wxh",
						           fmtp+2, 3) == 0) {
							SDL_Rect *rd =
							    label->poll.ptrs
							    [ri++];

							asprintf(&s2,
							    "%dx%d",
							    rd->w, rd->h);
							fmtp += 4;
						} else if (strncmp("x,y",
						           fmtp+2, 3) == 0) {
							SDL_Rect *rd =
							    label->poll.ptrs
							    [ri++];

							asprintf(&s2,
							    "[%d,%d]",
							    rd->x, rd->y);
							fmtp += 4;
						} else if (strncmp("rect",
						           fmtp+2, 4) == 0) {
							SDL_Rect *rd =
							    label->poll.ptrs
							    [ri++];

							asprintf(&s2,
							    "%dx%d at [%d,%d]",
							    rd->w, rd->h,
							    rd->x, rd->y);
							fmtp += 5;
						} else {
							fatal("bad cast\n");
						}
						break;
					case '%':
						asprintf(&s2, "%%");
						break;
					default:
						fatal("bad format: %c\n",
						    *(fmtp+1));
					}
					fmtp++;
				} else {
					asprintf(&s2, "%c", *fmtp);
				}
				
				if (s2 != NULL) {
					size_t scl;

					scl = strlcat(sp, s2, LABEL_MAX_LENGTH);
					if (scl > LABEL_MAX_LENGTH) {
						fatal("overflow\n");
					}
					sp += scl;
					free(s2);
				}
			}
			*sp = '\0';

			if (label->poll.lock != NULL)
				pthread_mutex_unlock(label->poll.lock);

			ts = text_render(NULL, -1,
			    WIDGET_COLOR(label, TEXT_COLOR), s);
			WIDGET_DRAW(label, ts, 0, 0);
			SDL_FreeSurface(ts);
		}
		break;
	}
}

void
label_destroy(void *p)
{
	struct label *label = p;

	switch (label->type) {
	case LABEL_STATIC:
		free(label->text.caption);
		SDL_FreeSurface(label->text.surface);
		pthread_mutex_destroy(&label->text.lock);
		break;
	case LABEL_POLLED:
		free(label->poll.fmt);
		break;
	}
}
