/*	$Csoft: label.c,v 1.83 2005/03/09 06:39:20 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#include <engine/engine.h>
#include <engine/view.h>

#include "label.h"

#include <string.h>
#include <stdarg.h>
#include <errno.h>

const struct widget_ops label_ops = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		label_destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	label_draw,
	label_scale
};

struct label *
label_new(void *parent, enum label_type type, const char *fmt, ...)
{
	char buf[LABEL_MAX];
	struct label *label;
	va_list ap;
	const char *p;
	
	label = Malloc(sizeof(struct label), M_OBJECT);

	va_start(ap, fmt);
	switch (type) {
	case LABEL_STATIC:
		vsnprintf(buf, sizeof(buf), fmt, ap);
		label_init(label, LABEL_STATIC, buf);
		break;
	case LABEL_POLLED:
		label_init(label, LABEL_POLLED, fmt);
		label->poll.lock = NULL;
		break;
	case LABEL_POLLED_MT:
		label_init(label, LABEL_POLLED_MT, fmt);
		label->poll.lock = va_arg(ap, pthread_mutex_t *);
		break;
	}
	if (type == LABEL_POLLED || type == LABEL_POLLED_MT) {
		for (p = fmt; *p != '\0'; p++) {
			if (*p == '%' && *(p+1) != '\0') {
				switch (*(p+1)) {
				case ' ':
				case '(':
				case ')':
				case '%':
					break;
				default:
					if (label->poll.nptrs+1 <
					    LABEL_MAX_POLLPTRS) {
						label->poll.ptrs
						    [label->poll.nptrs++] =
						    va_arg(ap, void *);
					}
					break;
				}
			}
		}
	}
	va_end(ap);

	object_attach(parent, label);
	return (label);
}

void
label_scale(void *p, int rw, int rh)
{
	struct label *lab = p;

	switch (lab->type) {
	case LABEL_STATIC:
		pthread_mutex_lock(&lab->lock);
		if (rw == -1 && rh == -1 && lab->surface != -1) {
			WIDGET(lab)->w = WIDGET_SURFACE(lab,lab->surface)->w;
			WIDGET(lab)->h = WIDGET_SURFACE(lab,lab->surface)->h;
		}
		pthread_mutex_unlock(&lab->lock);
		break;
	case LABEL_POLLED:
	case LABEL_POLLED_MT:
		if (rh == -1 && rh == -1) {
			WIDGET(lab)->w = lab->prew;
			WIDGET(lab)->h = lab->preh;
		}
		break;
	}
}

void
label_init(struct label *label, enum label_type type, const char *s)
{
	widget_init(label, "label", &label_ops, 0);
	label->type = type;

	switch (type) {
	case LABEL_STATIC:
		label->surface = (s==NULL) ? -1 : widget_map_surface(label,
		    text_render(NULL, -1, COLOR(TEXT_COLOR), s));
		pthread_mutex_init(&label->lock, NULL);
		break;
	case LABEL_POLLED:
	case LABEL_POLLED_MT:
		label_prescale(label, "XXXXXXXXXXXXXXXXX");
		WIDGET(label)->flags |= WIDGET_WFILL;
		label->surface = -1;
		strlcpy(label->poll.fmt, s, sizeof(label->poll.fmt));
		memset(label->poll.ptrs, 0, sizeof(void *)*LABEL_MAX_POLLPTRS);
		label->poll.nptrs = 0;
		break;
	}
}

void
label_prescale(struct label *lab, const char *text)
{
	text_prescale(text, &lab->prew, &lab->preh);
}

void
label_set_surface(struct label *label, SDL_Surface *su)
{
#ifdef DEBUG
	if (label->type != LABEL_STATIC)
		fatal("label is not static");
#endif
	pthread_mutex_lock(&label->lock);
	widget_replace_surface(label, 0, su);
	pthread_mutex_unlock(&label->lock);
}

void
label_printf(struct label *label, const char *fmt, ...)
{
	char s[LABEL_MAX];
	va_list args;

#ifdef DEBUG
	if (label->type != LABEL_STATIC)
		fatal("label is not static");
#endif
	va_start(args, fmt);
	vsnprintf(s, sizeof(s), fmt, args);
	va_end(args);
	
	label_set_surface(label, (s[0]=='\0') ? NULL :
	    text_render(NULL, -1, COLOR(TEXT_COLOR), s));
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

	snprintf(s, len, "%s", ob != NULL ? ob->name : "(null)");
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

static void
label_int_bool(struct label *label, char *s, size_t len, int ri)
{
	int *flag = &LABEL_ARG(int);

	snprintf(s, len, "%s", *flag ? _("yes") : _("no"));
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
	{ "ibool", sizeof("ibool"),	label_int_bool },
};
static const int nfmts = sizeof(fmts) / sizeof(fmts[0]);

/* Display a polled label. */
static void
label_draw_polled(struct label *label)
{
	char s[LABEL_MAX];
	char s2[32];
	SDL_Surface *ts;
	char *fmtp;
	int i, ri = 0;

	s[0] = '\0';
	s2[0] = '\0';

	for (fmtp = label->poll.fmt; *fmtp != '\0'; fmtp++) {
		if (*fmtp == '%' && *(fmtp+1) != '\0') {
			switch (*(fmtp+1)) {
			case 'd':
			case 'i':
				snprintf(s2, sizeof(s2), "%d", LABEL_ARG(int));
				strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'D':
			case 'I':
				if (LABEL_ARG(int) < 0) {
					snprintf(s2, sizeof(s2), "%d",
					    LABEL_ARG(int));
					strlcat(s, s2, sizeof(s));
					ri++;
				} else if (LABEL_ARG(int) > 0) {
					snprintf(s2, sizeof(s2), "+%d",
					    LABEL_ARG(int));
					strlcat(s, s2, sizeof(s));
					ri++;
				}
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
				strlcat(s, &LABEL_ARG(char), sizeof(s));
				ri++;
				break;
			case 'p':
				snprintf(s2, sizeof(s2), "%p",
				    LABEL_ARG(void *));
				strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'f':
				snprintf(s2, sizeof(s2), "%.2f",
				    LABEL_ARG(double));
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

	/* TODO use widget_update_surface */
	ts = text_render(NULL, -1, COLOR(TEXT_COLOR), s);
	widget_blit(label, ts, 0, 0);
	SDL_FreeSurface(ts);
}

void
label_draw(void *p)
{
	struct label *label = p;
	
	switch (label->type) {
	case LABEL_STATIC:
		pthread_mutex_lock(&label->lock);
		widget_blit_surface(label, label->surface, 0, 0);
		pthread_mutex_unlock(&label->lock);
		break;
	case LABEL_POLLED:
		label_draw_polled(label);
		break;
	case LABEL_POLLED_MT:
		pthread_mutex_lock(label->poll.lock);
		label_draw_polled(label);
		pthread_mutex_unlock(label->poll.lock);
		break;
	}
}

void
label_destroy(void *p)
{
	struct label *label = p;

	if (label->type == LABEL_STATIC) {
		pthread_mutex_destroy(&label->lock);
	}
	widget_destroy(label);
}

