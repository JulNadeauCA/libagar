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

#include <core/core.h>
#include <core/view.h>

#include "label.h"

#include <string.h>
#include <stdarg.h>

const AG_WidgetOps agLabelOps = {
	{
		"AG_Widget:AG_Label",
		sizeof(AG_Label),
		{ 0,0 },
		NULL,		/* init */
		NULL,		/* reinit */
		AG_LabelDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_LabelDraw,
	AG_LabelScale
};

/* Create a new polled label. */
AG_Label *
AG_LabelNewPolled(void *parent, Uint flags, const char *fmt, ...)
{
	char buf[AG_LABEL_MAX];
	AG_Label *label;
	va_list ap;
	const char *p;
	
	label = Malloc(sizeof(AG_Label), M_OBJECT);
	AG_LabelInit(label, AG_LABEL_POLLED, flags, fmt);
#ifdef THREADS
	label->poll.lock = NULL;
#endif
	va_start(ap, fmt);
	for (p = fmt; *p != '\0'; p++) {
		if (*p != '%' || *(p+1) == '\0') {
			continue;
		}
		switch (*(p+1)) {
		case ' ':
		case '(':
		case ')':
		case '%':
			break;
		default:
			if (label->poll.nptrs+1 < AG_LABEL_MAX_POLLPTRS) {
				label->poll.ptrs[label->poll.nptrs++] =
				    va_arg(ap, void *);
			}
			break;
		}
	}
	va_end(ap);

	AG_ObjectAttach(parent, label);
	return (label);
}

/* Create a new polled label associated with a mutex. */
AG_Label *
AG_LabelNewPolledMT(void *parent, Uint flags, AG_Mutex *mutex,
    const char *fmt, ...)
{
	char buf[AG_LABEL_MAX];
	AG_Label *label;
	va_list ap;
	const char *p;
	
	label = Malloc(sizeof(AG_Label), M_OBJECT);
	AG_LabelInit(label, AG_LABEL_POLLED_MT, flags, fmt);
#ifdef THREADS
	label->poll.lock = mutex;
#endif
	va_start(ap, fmt);
	for (p = fmt; *p != '\0'; p++) {
		if (*p != '%' || *(p+1) == '\0') {
			continue;
		}
		switch (*(p+1)) {
		case ' ':
		case '(':
		case ')':
		case '%':
			break;
		default:
			if (label->poll.nptrs+1 < AG_LABEL_MAX_POLLPTRS) {
				label->poll.ptrs[label->poll.nptrs++] =
				    va_arg(ap, void *);
			}
			break;
		}
	}
	va_end(ap);

	AG_ObjectAttach(parent, label);
	return (label);
}

/* Create a new static label using the given formatted text. */
AG_Label *
AG_LabelNewStatic(void *parent, Uint flags, const char *fmt, ...)
{
	char *s;
	AG_Label *label;
	va_list ap;

	if (fmt != NULL) {
		va_start(ap, fmt);
		Vasprintf(&s, fmt, ap);
		va_end(ap);
	} else {
		s = NULL;
	}

	label = Malloc(sizeof(AG_Label), M_OBJECT);
	AG_LabelInit(label, AG_LABEL_STATIC, flags, s);
	AG_ObjectAttach(parent, label);
	Free(s, 0);
	return (label);
}

/* LabelNewStatic() variant without format string. */
AG_Label *
AG_LabelNewStaticString(void *parent, Uint flags, const char *text)
{
	AG_Label *lbl;
	
	lbl = Malloc(sizeof(AG_Label), M_OBJECT);
	AG_LabelInit(lbl, AG_LABEL_STATIC, flags, text);
	AG_ObjectAttach(parent, lbl);
	return (lbl);
}

void
AG_LabelScale(void *p, int rw, int rh)
{
	AG_Label *lbl = p;

	if (rw == -1 && rh == -1) {
		if (lbl->flags & AG_LABEL_NOMINSIZE) {
			AGWIDGET(lbl)->w = lbl->lPad + lbl->rPad;
			AGWIDGET(lbl)->h = agTextFontHeight +
			                   lbl->tPad + lbl->bPad;
			return;
		}
		switch (lbl->type) {
		case AG_LABEL_STATIC:
			AG_MutexLock(&lbl->lock);
			if (lbl->surface != -1) {
				AGWIDGET(lbl)->w =
				    AGWIDGET_SURFACE(lbl,lbl->surface)->w +
				    lbl->lPad + lbl->rPad;
				AGWIDGET(lbl)->h =
				    AGWIDGET_SURFACE(lbl,lbl->surface)->h +
				    lbl->tPad + lbl->bPad;
			}
			AG_MutexUnlock(&lbl->lock);
			break;
		case AG_LABEL_POLLED:
		case AG_LABEL_POLLED_MT:
			if (rh == -1 && rh == -1) {
				AGWIDGET(lbl)->w = lbl->wPre + lbl->lPad +
				                   lbl->rPad;
				AGWIDGET(lbl)->h = lbl->hPre + lbl->tPad +
				                   lbl->bPad;
			}
			break;
		}
	}
	if ((lbl->flags & AG_LABEL_NOMINSIZE) && lbl->surface != -1) {
		SDL_Surface *su = AGWIDGET_SURFACE(lbl,lbl->surface);

		if ((su->w + lbl->lPad+lbl->rPad) > rw ||
		    (su->h + lbl->tPad+lbl->bPad) > rh) {
			lbl->flags |= AG_LABEL_PARTIAL;
			if (lbl->surfaceCont == -1) {
				/* XXX this should be shared between widgets */
				lbl->surfaceCont = AG_WidgetMapSurface(lbl,
				    AG_TextRender(NULL, -1,
				    AG_COLOR(TEXT_COLOR), " ... "));
			}
		} else {
			lbl->flags &= ~AG_LABEL_PARTIAL;
		}
		AGWIDGET(lbl)->w = rw;
		AGWIDGET(lbl)->h = rh;
	}
}

void
AG_LabelInit(AG_Label *lbl, enum ag_label_type type, Uint flags,
    const char *s)
{
	int wflags = 0;

	if (flags & AG_LABEL_HFILL) { wflags |= AG_WIDGET_HFILL; }
	if (flags & AG_LABEL_VFILL) { wflags |= AG_WIDGET_VFILL; }

	AG_WidgetInit(lbl, &agLabelOps, wflags);
	lbl->type = type;
	lbl->flags = flags;
	lbl->lPad = 2;
	lbl->rPad = 2;
	lbl->tPad = 0;
	lbl->bPad = 1;
	lbl->wPre = -1;
	lbl->hPre = -1;
	lbl->surfaceCont = -1;
	SLIST_INIT(&lbl->lflags);

	switch (type) {
	case AG_LABEL_STATIC:
		lbl->surface = (s == NULL) ? -1 : AG_WidgetMapSurface(lbl,
		    AG_TextRender(NULL, -1, AG_COLOR(TEXT_COLOR), s));
		AG_MutexInit(&lbl->lock);
		break;
	case AG_LABEL_POLLED:
	case AG_LABEL_POLLED_MT:
		AG_LabelPrescale(lbl, 1, "XXXXXXXXXXXXXXXXX");
		lbl->surface = -1;
		strlcpy(lbl->poll.fmt, s, sizeof(lbl->poll.fmt));
		memset(lbl->poll.ptrs, 0, sizeof(void *)*AG_LABEL_MAX_POLLPTRS);
		lbl->poll.nptrs = 0;
		break;
	}
}

void
AG_LabelPrescale(AG_Label *lab, Uint nlines, const char *text)
{
	if (nlines > 0) {
		AG_TextPrescale(text, &lab->wPre, NULL);
		lab->hPre = nlines*agTextFontHeight +
		            (nlines-1)*agTextFontLineSkip;
	} else {
		AG_TextPrescale(text, &lab->wPre, &lab->hPre);
	}
}

void
AG_LabelSetSurface(AG_Label *lbl, SDL_Surface *su)
{
#ifdef DEBUG
	if (lbl->type != AG_LABEL_STATIC)
		fatal("label is not static");
#endif
	AG_MutexLock(&lbl->lock);
	AG_WidgetReplaceSurface(lbl, 0, su);
	AG_MutexUnlock(&lbl->lock);
}

void
AG_LabelSetPadding(AG_Label *lbl, int lPad, int rPad, int tPad, int bPad)
{
	if (lPad != -1) { lbl->lPad = lPad; }
	if (rPad != -1) { lbl->rPad = rPad; }
	if (tPad != -1) { lbl->tPad = tPad; }
	if (bPad != -1) { lbl->bPad = bPad; }
}

void
AG_LabelPrintf(AG_Label *lbl, const char *fmt, ...)
{
	char s[AG_LABEL_MAX];
	va_list args;

#ifdef DEBUG
	if (lbl->type != AG_LABEL_STATIC)
		fatal("label is not static");
#endif
	va_start(args, fmt);
	vsnprintf(s, sizeof(s), fmt, args);
	va_end(args);
	
	AG_LabelSetSurface(lbl, (s[0]=='\0') ? NULL :
	    AG_TextRender(NULL, -1, AG_COLOR(TEXT_COLOR), s));
}

#define LABEL_ARG(_type)	(*(_type *)label->poll.ptrs[ri])

static void
label_uint8(AG_Label *label, char *s, size_t len, int ri)
{
	snprintf(s, len, "%u", LABEL_ARG(Uint8));
}

static void
label_sint8(AG_Label *label, char *s, size_t len, int ri)
{
	snprintf(s, len, "%d", LABEL_ARG(Sint8));
}

static void
label_uint16(AG_Label *label, char *s, size_t len, int ri)
{
	snprintf(s, len, "%u", LABEL_ARG(Uint16));
}

static void
label_sint16(AG_Label *label, char *s, size_t len, int ri)
{
	snprintf(s, len, "%d", LABEL_ARG(Sint16));
}

static void
label_uint32(AG_Label *label, char *s, size_t len, int ri)
{
	snprintf(s, len, "%u", LABEL_ARG(Uint32));
}

static void
label_sint32(AG_Label *label, char *s, size_t len, int ri)
{
	snprintf(s, len, "%d", LABEL_ARG(Sint32));
}

static void
label_obj(AG_Label *label, char *s, size_t len, int ri)
{
	AG_Object *ob = LABEL_ARG(AG_Object *);

	snprintf(s, len, "%s", ob != NULL ? ob->name : "(null)");
}

static void
label_objt(AG_Label *label, char *s, size_t len, int ri)
{
	AG_Object *ob = LABEL_ARG(AG_Object *);
	snprintf(s, len, "%s", ob->ops->type);
}

static void
label_wxh(AG_Label *label, char *s, size_t len, int ri)
{
	SDL_Rect *rd = &LABEL_ARG(SDL_Rect);
	snprintf(s, len, "%ux%u", rd->w, rd->h);
}

static void
label_xy(AG_Label *label, char *s, size_t len, int ri)
{
	SDL_Rect *rd = &LABEL_ARG(SDL_Rect);
	snprintf(s, len, "%d,%d", rd->x, rd->y);
}

static void
label_rect(AG_Label *label, char *s, size_t len, int ri)
{
	SDL_Rect *rd = &LABEL_ARG(SDL_Rect);
	snprintf(s, len, "%ux%u at %d,%d", rd->w, rd->h, rd->x, rd->y);
}

static void
label_int_bool(AG_Label *label, char *s, size_t len, int ri)
{
	int *flag = &LABEL_ARG(int);
	snprintf(s, len, "%s", *flag ? _("yes") : _("no"));
}

static void
label_flags(AG_Label *label, char *s, size_t len, int ri)
{
	Uint *flags = &LABEL_ARG(Uint);
	struct ag_label_flag *lfl;

	s[0] = '\0';
	SLIST_FOREACH(lfl, &label->lflags, lflags) {
		if (lfl->idx == ri && *flags & (Uint)lfl->v) {
			if (s[0] != '\0') { strlcat(s, ", ", len); }
			strlcat(s, lfl->text, len);
		}
	}
}


static void
label_flags8(AG_Label *label, char *s, size_t len, int ri)
{
	Uint8 *flags = &LABEL_ARG(Uint8);
	struct ag_label_flag *lfl;

	s[0] = '\0';
	SLIST_FOREACH(lfl, &label->lflags, lflags) {
		if (lfl->idx == ri && *flags & (Uint8)lfl->v) {
			if (s[0] != '\0') { strlcat(s, ", ", len); }
			strlcat(s, lfl->text, len);
		}
	}
}

static void
label_flags16(AG_Label *label, char *s, size_t len, int ri)
{
	Uint16 *flags = &LABEL_ARG(Uint16);
	struct ag_label_flag *lfl;

	s[0] = '\0';
	SLIST_FOREACH(lfl, &label->lflags, lflags) {
		if (lfl->idx == ri && *flags & (Uint16)lfl->v) {
			if (s[0] != '\0') { strlcat(s, ", ", len); }
			strlcat(s, lfl->text, len);
		}
	}
}

static void
label_flags32(AG_Label *label, char *s, size_t len, int ri)
{
	Uint32 *flags = &LABEL_ARG(Uint32);
	struct ag_label_flag *lfl;

	s[0] = '\0';
	SLIST_FOREACH(lfl, &label->lflags, lflags) {
		if (lfl->idx == ri && *flags & (Uint32)lfl->v) {
			if (s[0] != '\0') { strlcat(s, ", ", len); }
			strlcat(s, lfl->text, len);
		}
	}
}

static const struct {
	char	 *fmt;
	size_t	  fmt_len;
	void	(*func)(AG_Label *, char *, size_t, int);
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
	{ "flags", sizeof("flags"),	label_flags },
	{ "flags8", sizeof("flags8"),	label_flags8 },
	{ "flags16", sizeof("flags16"),	label_flags16 },
	{ "flags32", sizeof("flags32"),	label_flags32 },
};
static const int nfmts = sizeof(fmts) / sizeof(fmts[0]);

/* Display a polled label. */
static void
AG_LabelDrawPolled(AG_Label *label)
{
	char s[AG_LABEL_MAX];
	char s2[AG_LABEL_MAX];
	SDL_Surface *ts;
	char *fmtp;
	int i, ri = 0;

	s[0] = '\0';
	s2[0] = '\0';

	for (fmtp = label->poll.fmt; *fmtp != '\0'; fmtp++) {
		if (*fmtp == '%' && *(fmtp+1) != '\0') {
			switch (*(fmtp+1)) {
#ifdef SDL_HAS_64BIT_TYPE
			case 'l':
				if (*(fmtp+2) == 'l') {
					switch (*(fmtp+3)) {
					case 'd':
					case 'i':
						snprintf(s2, sizeof(s2),
						    "%lld", (long long)
						    LABEL_ARG(Sint64));
						strlcat(s, s2, sizeof(s));
						ri++;
						break;
					case 'o':
						snprintf(s2, sizeof(s2),
						    "%llo", (unsigned long long)
						    LABEL_ARG(Uint64));
						strlcat(s, s2, sizeof(s));
						ri++;
						break;
					case 'u':
						snprintf(s2, sizeof(s2),
						    "%llu", (unsigned long long)
						    LABEL_ARG(Uint64));
						strlcat(s, s2, sizeof(s));
						ri++;
						break;
					case 'x':
						snprintf(s2, sizeof(s2),
						    "%llx", (unsigned long long)
						    LABEL_ARG(Uint64));
						strlcat(s, s2, sizeof(s));
						ri++;
						break;
					case 'X':
						snprintf(s2, sizeof(s2),
						    "%llX", (unsigned long long)
						    LABEL_ARG(Uint64));
						strlcat(s, s2, sizeof(s));
						ri++;
						break;
					}
					fmtp += 2;
				}
				break;
#endif /* SDL_HAS_64BIT_TYPE */
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
				    LABEL_ARG(float));
				strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'F':
				snprintf(s2, sizeof(s2), "%.2f",
				    LABEL_ARG(double));
				strlcat(s, s2, sizeof(s));
				ri++;
				break;
#if 0
			case 'G':
				snprintf(s2, sizeof(s2), "%.2f",
				    LABEL_ARG(long double));
				strlcat(s, s2, sizeof(s));
				ri++;
				break;
#endif
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

	/* TODO use AG_WidgetUpdateSurface */
	ts = AG_TextRender(NULL, -1, AG_COLOR(TEXT_COLOR), s);
	AG_WidgetBlit(label, ts, label->lPad, label->tPad);
	SDL_FreeSurface(ts);
}

void
AG_LabelDraw(void *p)
{
	AG_Label *lbl = p;
	int wClip;

	if (AGWIDGET(lbl)->w == 0 || AGWIDGET(lbl)->h == 0)
		return;

	wClip = AGWIDGET(lbl)->w;
	if (lbl->flags & AG_LABEL_PARTIAL) {
		wClip -= AGWIDGET_SURFACE(lbl,lbl->surfaceCont)->w;
	}
	AG_WidgetPushClipRect(lbl, 0, 0, wClip, AGWIDGET(lbl)->h);

	switch (lbl->type) {
	case AG_LABEL_STATIC:
		AG_MutexLock(&lbl->lock);
		AG_WidgetBlitSurface(lbl, lbl->surface, lbl->lPad, lbl->tPad);
		AG_MutexUnlock(&lbl->lock);
		break;
	case AG_LABEL_POLLED:
		AG_LabelDrawPolled(lbl);
		break;
	case AG_LABEL_POLLED_MT:
		AG_MutexLock(lbl->poll.lock);
		AG_LabelDrawPolled(lbl);
		AG_MutexUnlock(lbl->poll.lock);
		break;
	}

	AG_WidgetPopClipRect(lbl);
	if (lbl->flags & AG_LABEL_PARTIAL)
		AG_WidgetBlitSurface(lbl, lbl->surfaceCont, wClip, lbl->tPad);
}

void
AG_LabelDestroy(void *p)
{
	AG_Label *lbl = p;
	struct ag_label_flag *lfl, *lflNext;

	if (lbl->type == AG_LABEL_STATIC) {
		AG_MutexDestroy(&lbl->lock);
	}
	for (lfl = SLIST_FIRST(&lbl->lflags);
	     lfl != SLIST_END(&lbl->lflags);
	     lfl = lflNext) {
		lflNext = SLIST_NEXT(lfl, lflags);
		Free(lfl, M_WIDGET);
	}
	AG_WidgetDestroy(lbl);
}

/* Register a flag description text. */
void
AG_LabelFlagNew(AG_Label *lbl, Uint idx, const char *text,
    enum ag_widget_binding_type type, Uint32 v)
{
	struct ag_label_flag *lfl;

	lfl = Malloc(sizeof(struct ag_label_flag), M_WIDGET);
	lfl->idx = idx;
	lfl->text = text;
	lfl->type = type;
	lfl->v = v;
	SLIST_INSERT_HEAD(&lbl->lflags, lfl, lflags);
}
