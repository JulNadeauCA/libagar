/*	$Csoft: label.c,v 1.63 2003/05/16 01:26:55 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003 CubeSoft Communications, Inc.
 * <http://www.csoft.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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

#include <engine/compat/snprintf.h>
#include <engine/compat/vasprintf.h>
#include <engine/compat/strlcat.h>
#include <engine/compat/strlcpy.h>

#include <engine/engine.h>
#include <engine/view.h>

#include <engine/widget/text.h>
#include <engine/widget/region.h>

#include "label.h"

#include <string.h>
#include <stdarg.h>
#include <errno.h>

const struct widget_ops label_ops = {
	{
		NULL,		/* init */
		label_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	label_draw,
	NULL		/* update */
};

enum {
	TEXT_COLOR
};

/* Create and attach a new static label. */
struct label *
label_new(struct region *reg, int w, int h, const char *fmt, ...)
{
	struct label *label;
	va_list args;
	char *buf;

	va_start(args, fmt);
	Vasprintf(&buf, fmt, args);
	va_end(args);

	label = Malloc(sizeof(struct label));
	label_init(label, LABEL_STATIC, buf, w, h);
	free(buf);
	region_attach(reg, label);
	return (label);
}

static void
label_scaled(int argc, union evarg *argv)
{
	struct label *lab = argv[0].p;

	switch (lab->type) {
	case LABEL_STATIC:
		pthread_mutex_lock(&lab->text.lock);
		if (lab->text.surface != NULL) {
			if (WIDGET(lab)->rw == -1)
				WIDGET(lab)->w = lab->text.surface->w;
			if (WIDGET(lab)->rh == -1)
				WIDGET(lab)->h = lab->text.surface->h;
		}
		pthread_mutex_unlock(&lab->text.lock);
		break;
	case LABEL_POLLED:
		if (WIDGET(lab)->rh == -1)
			WIDGET(lab)->h = text_font_height(font);
		break;
	}
}

/* Initialize a static or polled label. */
void
label_init(struct label *label, enum label_type type, const char *s,
    int rw, int rh)
{
	widget_init(&label->wid, "label", &label_ops, rw, rh);
	
	widget_map_color(label, TEXT_COLOR, "text", 250, 250, 250);

	label->type = type;
	switch (type) {
	case LABEL_STATIC:
		if (s != NULL) {
			label->text.caption = Strdup(s);
			label->text.surface = text_render(NULL, -1,
			    WIDGET_COLOR(label, TEXT_COLOR),
			    label->text.caption);
		} else {
			label->text.caption = NULL;
			label->text.surface = NULL;
		}
		pthread_mutex_init(&label->text.lock, NULL);
		break;
	case LABEL_POLLED:
		label->text.caption = NULL;
		label->text.surface = NULL;
		label->poll.fmt = Strdup(s);

		memset(label->poll.ptrs, 0,
		    sizeof(void *) * LABEL_MAX_POLLITEMS);
		label->poll.nptrs = 0;
		break;
	}

	event_new(label, "widget-scaled", label_scaled, NULL);
}

/* Create a new polled label. */
struct label *
label_polled_new(struct region *reg, int w, int h, pthread_mutex_t *mutex,
    const char *fmt, ...)
{
	struct label *label;
	va_list args;
	const char *p;

	label = Malloc(sizeof(struct label));
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

/* Update the text of a static label. */
void
label_printf(struct label *label, const char *fmt, ...)
{
	va_list args;

#ifdef DEBUG
	if (label->type != LABEL_STATIC)
		fatal("not a static label");
#endif
	
	pthread_mutex_lock(&label->text.lock);

	free(label->text.caption);
	va_start(args, fmt);
	Vasprintf(&label->text.caption, fmt, args);
	va_end(args);

	if (label->text.surface != NULL)
		SDL_FreeSurface(label->text.surface);
	if (label->text.caption[0] != '\0') {
		label->text.surface = text_render(NULL, -1,
		    WIDGET_COLOR(label, TEXT_COLOR), label->text.caption);
		WIDGET(label)->w = label->text.surface->w;
		WIDGET(label)->h = label->text.surface->h;
	} else {
		label->text.surface = NULL;
		WIDGET(label)->w = 0;
		WIDGET(label)->h = 0;
	}
	pthread_mutex_unlock(&label->text.lock);
}

#define LABEL_ARG(_type)	(*(_type *)label->poll.ptrs[ri])

static void
label_uint8(struct label *label, char *s, size_t len, int ri)
{
	snprintf(s, len, "%u", LABEL_ARG(Uint8));
}

static void
label_sint8(struct label *label, char *s, size_t len, int ri)
{
	snprintf(s, len, "%d", LABEL_ARG(Sint8));
}

static void
label_uint16(struct label *label, char *s, size_t len, int ri)
{
	snprintf(s, len, "%u", LABEL_ARG(Uint16));
}

static void
label_sint16(struct label *label, char *s, size_t len, int ri)
{
	snprintf(s, len, "%d", LABEL_ARG(Sint16));
}

static void
label_uint32(struct label *label, char *s, size_t len, int ri)
{
	snprintf(s, len, "%u", LABEL_ARG(Uint32));
}

static void
label_sint32(struct label *label, char *s, size_t len, int ri)
{
	snprintf(s, len, "%d", LABEL_ARG(Sint32));
}

static void
label_obj(struct label *label, char *s, size_t len, int ri)
{
	struct object *ob = LABEL_ARG(struct object *);

	snprintf(s, len, "%s", ob->name);
}

static void
label_objt(struct label *label, char *s, size_t len, int ri)
{
	struct object *ob = LABEL_ARG(struct object *);

	snprintf(s, len, "%s", ob->type);
}

static void
label_wxh(struct label *label, char *s, size_t len, int ri)
{
	SDL_Rect *rd = &LABEL_ARG(SDL_Rect);

	snprintf(s, len, "%ux%u", rd->w, rd->h);
}

static void
label_xy(struct label *label, char *s, size_t len, int ri)
{
	SDL_Rect *rd = &LABEL_ARG(SDL_Rect);

	snprintf(s, len, "%d,%d", rd->x, rd->y);
}

static void
label_rect(struct label *label, char *s, size_t len, int ri)
{
	SDL_Rect *rd = &LABEL_ARG(SDL_Rect);

	snprintf(s, len, "%ux%u at %d,%d", rd->w, rd->h, rd->x, rd->y);
}

static const struct {
	char	 *fmt;
	size_t	  fmt_len;
	void	(*func)(struct label *, char *, size_t, int);
} fmts[] = {
	{ "u8", sizeof("u8"),		label_uint8 },
	{ "s8", sizeof("s8"),		label_sint8 },
	{ "u16", sizeof("u16"),		label_uint16 },
	{ "s16", sizeof("s16"),		label_sint16 },
	{ "u32", sizeof("u32"),		label_uint32 },
	{ "s32", sizeof("s32"),		label_sint32 },
	{ "obj", sizeof("obj"),		label_obj },
	{ "objt", sizeof("objt"),	label_objt },
	{ "wxh", sizeof("wxh"),		label_wxh },
	{ "x,y", sizeof("x,y"),		label_xy },
	{ "rect", sizeof("rect"),	label_rect },
};
static const int nfmts = sizeof(fmts) / sizeof(fmts[0]);

/* Display a polled label. */
static __inline__ void
label_draw_polled(struct label *label)
{
	char s[LABEL_MAX_LENGTH];
	char s2[32];
	SDL_Surface *ts;
	char *fmtp;
	int i, ri = 0;

	s[0] = '\0';
	s2[0] = '\0';

	if (label->poll.lock != NULL)
		pthread_mutex_lock(label->poll.lock);

	for (fmtp = label->poll.fmt; *fmtp != '\0'; fmtp++) {
		if (*fmtp == '%' && *(fmtp+1) != '\0') {
			switch (*(fmtp+1)) {
			case 'd':
			case 'i':
				snprintf(s2, sizeof(s2), "%d", LABEL_ARG(int));
				strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'o':
				snprintf(s2, sizeof(s2), "%o",
				    LABEL_ARG(unsigned int));
				strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'u':
				snprintf(s2, sizeof(s2), "%u",
				    LABEL_ARG(unsigned int));
				strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'x':
				snprintf(s2, sizeof(s2), "%x",
				    LABEL_ARG(unsigned int));
				strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'X':
				snprintf(s2, sizeof(s2), "%X",
				    LABEL_ARG(unsigned int));
				strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'c':
				s2[0] = LABEL_ARG(char);
				s2[1] = '\0';
				strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 's':
				strlcat(s, LABEL_ARG(char *), sizeof(s));
				ri++;
				break;
			case 'p':
				snprintf(s2, sizeof(s2),
				    "%p", LABEL_ARG(void *));
				strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case '[':
				for (i = 0; i < nfmts; i++) {
					if (strncmp(fmts[i].fmt, fmtp+2,
					    fmts[i].fmt_len-1) != 0)
						continue;
					fmts[i].func(label, s2, sizeof(s2), ri);
					fmtp += fmts[i].fmt_len;
					strlcat(s, s2, sizeof(s));
					ri++;
					break;
				}
				break;
			case '%':
				s2[0] = '%';
				s2[1] = '\0';
				strlcat(s, s2, sizeof(s));
				break;
			}
			fmtp++;
		} else {
			s2[0] = *fmtp;
			s2[1] = '\0';
			strlcat(s, s2, sizeof(s));
		}
	}

	if (label->poll.lock != NULL)
		pthread_mutex_unlock(label->poll.lock);

	ts = text_render(NULL, -1, WIDGET_COLOR(label, TEXT_COLOR), s);
	widget_blit(label, ts, 0, 0);
	SDL_FreeSurface(ts);
}

void
label_draw(void *p)
{
	struct label *label = p;

	/* XXX justify */

	switch (label->type) {
	case LABEL_STATIC:
		pthread_mutex_lock(&label->text.lock);
		if (label->text.surface != NULL) {
			widget_blit(label, label->text.surface, 0, 0);
		}
		pthread_mutex_unlock(&label->text.lock);
		break;
	case LABEL_POLLED:
		label_draw_polled(label);
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
		if (label->text.surface != NULL) {
			SDL_FreeSurface(label->text.surface);
		}
		pthread_mutex_destroy(&label->text.lock);
		break;
	case LABEL_POLLED:
		free(label->poll.fmt);
		break;
	}

	widget_destroy(label);
}
