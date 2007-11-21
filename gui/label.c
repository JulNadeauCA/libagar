/*
 * Copyright (c) 2002-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "label.h"
#include "primitive.h"

#include <string.h>
#include <stdarg.h>

/* Create a new polled label. */
AG_Label *
AG_LabelNewPolled(void *parent, Uint flags, const char *fmt, ...)
{
	AG_Label *label;
	va_list ap;
	const char *p;
	
	label = Malloc(sizeof(AG_Label));
	AG_ObjectInit(label, &agLabelClass);
	label->type = AG_LABEL_POLLED;
	label->text = Strdup(fmt);
	label->flags |= flags;
	if (flags & AG_LABEL_HFILL) { AG_ExpandHoriz(label); }
	if (flags & AG_LABEL_VFILL) { AG_ExpandVert(label); }
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
	AG_Label *label;
	va_list ap;
	const char *p;
	
	label = Malloc(sizeof(AG_Label));
	AG_ObjectInit(label, &agLabelClass);
	label->type = AG_LABEL_POLLED_MT;
	label->text = Strdup(fmt);
	label->flags |= flags;
	if (flags & AG_LABEL_HFILL) { AG_ExpandHoriz(label); }
	if (flags & AG_LABEL_VFILL) { AG_ExpandVert(label); }
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
	AG_Label *label;
	va_list ap;

	label = Malloc(sizeof(AG_Label));
	AG_ObjectInit(label, &agLabelClass);
	label->type = AG_LABEL_STATIC;
	label->flags |= flags;
	if (flags & AG_LABEL_HFILL) { AG_ExpandHoriz(label); }
	if (flags & AG_LABEL_VFILL) { AG_ExpandVert(label); }
	if (fmt != NULL) {
		va_start(ap, fmt);
		Vasprintf(&label->text, fmt, ap);
		va_end(ap);
	} else {
		label->text = NULL;
	}

	AG_ObjectAttach(parent, label);
	return (label);
}

/* LabelNewStatic() variant without format string. */
AG_Label *
AG_LabelNewStaticString(void *parent, Uint flags, const char *text)
{
	AG_Label *label;
	
	label = Malloc(sizeof(AG_Label));
	AG_ObjectInit(label, &agLabelClass);
	label->type = AG_LABEL_STATIC;
	label->flags |= flags;
	if (flags & AG_LABEL_HFILL) { AG_ExpandHoriz(label); }
	if (flags & AG_LABEL_VFILL) { AG_ExpandVert(label); }
	label->text = (text != NULL) ? Strdup(text) : NULL;

	AG_ObjectAttach(parent, label);
	return (label);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Label *lbl = p;
	
	AG_MutexLock(&lbl->lock);
	if (lbl->flags & AG_LABEL_NOMINSIZE) {
		r->w = lbl->lPad + lbl->rPad;
		r->h = agTextFontHeight + lbl->tPad + lbl->bPad;
		goto out;
	}
	switch (lbl->type) {
	case AG_LABEL_STATIC:
		if (lbl->surface != -1) {
			r->w = WSURFACE(lbl,lbl->surface)->w;
			r->h = WSURFACE(lbl,lbl->surface)->h;
		} else {
			AG_TextSize(lbl->text, &r->w, &r->h);
		}
		r->w += lbl->lPad + lbl->rPad;
		r->h += lbl->tPad + lbl->bPad;
		break;
	case AG_LABEL_POLLED:
	case AG_LABEL_POLLED_MT:
		r->w = lbl->wPre + lbl->lPad + lbl->rPad;
		r->h = lbl->hPre + lbl->tPad + lbl->bPad;
		break;
	}
out:
	AG_MutexUnlock(&lbl->lock);
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Label *lbl = p;
	
	if (a->w < 1 || a->h < 1)
		return (-1);
	
	/*
	 * If the widget area is too small to display the complete
	 * string, display a "..." at the end of it.
	 */
	AG_MutexLock(&lbl->lock);
	if (lbl->surface != -1) {
		int clipEnabled;

		if (agTextFontHeight+lbl->tPad+lbl->bPad > a->h) {
			WIDGET(lbl)->flags |= AG_WIDGET_CLIPPING;
			clipEnabled = 1;
		} else {
			WIDGET(lbl)->flags &= ~(AG_WIDGET_CLIPPING);
			clipEnabled = 0;
		}
		if ((WSURFACE(lbl,lbl->surface)->w+lbl->lPad+lbl->rPad) >a->w) {
			lbl->flags |= AG_LABEL_PARTIAL;
			if (lbl->surfaceCont == -1) {
				/* TODO share this between all widgets */
				AG_TextColor(TEXT_COLOR);
				lbl->surfaceCont = AG_WidgetMapSurface(lbl,
				    AG_TextRender(" ... "));
			}
			if (WSURFACE(lbl,lbl->surfaceCont)->w > a->w) {
				if (clipEnabled) {
					WIDGET(lbl)->flags &=
					    ~(AG_WIDGET_CLIPPING);
				}
				return (-1);
			}
		} else {
			lbl->flags &= ~AG_LABEL_PARTIAL;
		}
	}
	AG_MutexUnlock(&lbl->lock);
	return (0);
}

static void
Init(void *obj)
{
	AG_Label *lbl = obj;

	lbl->type = AG_LABEL_STATIC;
	lbl->flags = 0;
	lbl->text = NULL;
	lbl->surface = -1;
	lbl->surfaceCont = -1;
	lbl->lPad = 2;
	lbl->rPad = 2;
	lbl->tPad = 0;
	lbl->bPad = 1;
	lbl->wPre = -1;
	lbl->hPre = agTextFontHeight;
	lbl->justify = AG_TEXT_LEFT;
	SLIST_INIT(&lbl->lflags);
	AG_MutexInitRecursive(&lbl->lock);
	
	memset(lbl->poll.ptrs, 0, sizeof(void *)*AG_LABEL_MAX_POLLPTRS);
	lbl->poll.nptrs = 0;
}

void
AG_LabelSizeHint(AG_Label *lab, Uint nlines, const char *text)
{
	if (nlines > 0) {
		AG_TextSize(text, &lab->wPre, NULL);
		lab->hPre = nlines*agTextFontHeight +
		            (nlines-1)*agTextFontLineSkip;
	} else {
		AG_TextSize(text, &lab->wPre, &lab->hPre);
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
	if (su == NULL) {
		lbl->surface = -1;
	}
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
AG_LabelJustify(AG_Label *lbl, enum ag_text_justify justify)
{
	AG_MutexLock(&lbl->lock);
	lbl->justify = justify;
	AG_MutexUnlock(&lbl->lock);
}

void
AG_LabelText(AG_Label *lbl, const char *fmt, ...)
{
	va_list ap;

	AG_MutexLock(&lbl->lock);
	Free(lbl->text);
	va_start(ap, fmt);
	Vasprintf(&lbl->text, fmt, ap);
	va_end(ap);
	lbl->flags |= AG_LABEL_REGEN;
	AG_MutexUnlock(&lbl->lock);
}

void
AG_LabelString(AG_Label *lbl, const char *s)
{
	AG_MutexLock(&lbl->lock);
	Free(lbl->text);
	lbl->text = Strdup(s);
	lbl->flags |= AG_LABEL_REGEN;
	AG_MutexUnlock(&lbl->lock);
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
	snprintf(s, len, "%s", ob->cls->name);
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
			if (s[0] != '\0') { Strlcat(s, ", ", len); }
			Strlcat(s, lfl->text, len);
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
			if (s[0] != '\0') { Strlcat(s, ", ", len); }
			Strlcat(s, lfl->text, len);
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
			if (s[0] != '\0') { Strlcat(s, ", ", len); }
			Strlcat(s, lfl->text, len);
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
			if (s[0] != '\0') { Strlcat(s, ", ", len); }
			Strlcat(s, lfl->text, len);
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
DrawPolled(AG_Label *label)
{
	char s[AG_LABEL_MAX];
	char s2[AG_LABEL_MAX];
	SDL_Surface *ts;
	char *fmtp;
	int i, ri = 0;

	if (label->text == NULL || label->text[0] == '\0') {
		return;
	}
	s[0] = '\0';
	s2[0] = '\0';

	for (fmtp = label->text; *fmtp != '\0'; fmtp++) {
		if (*fmtp == '%' && *(fmtp+1) != '\0') {
			switch (*(fmtp+1)) {
#ifdef HAVE_64BIT
			case 'l':
				if (*(fmtp+2) == 'l') {
					switch (*(fmtp+3)) {
					case 'd':
					case 'i':
						snprintf(s2, sizeof(s2),
						    "%lld", (long long)
						    LABEL_ARG(Sint64));
						Strlcat(s, s2, sizeof(s));
						ri++;
						break;
					case 'o':
						snprintf(s2, sizeof(s2),
						    "%llo", (unsigned long long)
						    LABEL_ARG(Uint64));
						Strlcat(s, s2, sizeof(s));
						ri++;
						break;
					case 'u':
						snprintf(s2, sizeof(s2),
						    "%llu", (unsigned long long)
						    LABEL_ARG(Uint64));
						Strlcat(s, s2, sizeof(s));
						ri++;
						break;
					case 'x':
						snprintf(s2, sizeof(s2),
						    "%llx", (unsigned long long)
						    LABEL_ARG(Uint64));
						Strlcat(s, s2, sizeof(s));
						ri++;
						break;
					case 'X':
						snprintf(s2, sizeof(s2),
						    "%llX", (unsigned long long)
						    LABEL_ARG(Uint64));
						Strlcat(s, s2, sizeof(s));
						ri++;
						break;
					}
					fmtp += 2;
				}
				break;
#endif /* HAVE_64BIT */
			case 'd':
			case 'i':
				snprintf(s2, sizeof(s2), "%d", LABEL_ARG(int));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'D':
			case 'I':
				if (LABEL_ARG(int) < 0) {
					snprintf(s2, sizeof(s2), "%d",
					    LABEL_ARG(int));
					Strlcat(s, s2, sizeof(s));
					ri++;
				} else if (LABEL_ARG(int) > 0) {
					snprintf(s2, sizeof(s2), "+%d",
					    LABEL_ARG(int));
					Strlcat(s, s2, sizeof(s));
					ri++;
				}
				break;
			case 'o':
				snprintf(s2, sizeof(s2), "%o",
				    LABEL_ARG(unsigned int));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'u':
				snprintf(s2, sizeof(s2), "%u",
				    LABEL_ARG(unsigned int));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'x':
				snprintf(s2, sizeof(s2), "%x",
				    LABEL_ARG(unsigned int));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'X':
				snprintf(s2, sizeof(s2), "%X",
				    LABEL_ARG(unsigned int));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'c':
				s2[0] = LABEL_ARG(char);
				s2[1] = '\0';
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 's':
				Strlcat(s, &LABEL_ARG(char), sizeof(s));
				ri++;
				break;
			case 'p':
				snprintf(s2, sizeof(s2), "%p",
				    LABEL_ARG(void *));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'f':
				snprintf(s2, sizeof(s2), "%.2f",
				    LABEL_ARG(float));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
			case 'F':
				snprintf(s2, sizeof(s2), "%.2f",
				    LABEL_ARG(double));
				Strlcat(s, s2, sizeof(s));
				ri++;
				break;
#if 0
			case 'G':
				snprintf(s2, sizeof(s2), "%.2f",
				    LABEL_ARG(long double));
				Strlcat(s, s2, sizeof(s));
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
					Strlcat(s, s2, sizeof(s));
					ri++;
					break;
				}
				break;
			case '%':
				s2[0] = '%';
				s2[1] = '\0';
				Strlcat(s, s2, sizeof(s));
				break;
			}
			fmtp++;
		} else {
			s2[0] = *fmtp;
			s2[1] = '\0';
			Strlcat(s, s2, sizeof(s));
		}
	}

	/* XXX TODO render directly! */
	AG_TextJustify(label->justify);
	AG_TextColor(TEXT_COLOR);
	ts = AG_TextRender(s);
	AG_WidgetBlit(label, ts, label->lPad, label->tPad);
	SDL_FreeSurface(ts);
}

static void
Draw(void *p)
{
	AG_Label *lbl = p;

	AG_MutexLock(&lbl->lock);

	if (lbl->flags & AG_LABEL_PARTIAL)
		AG_WidgetPushClipRect(lbl, 0, 0,
		    WIDGET(lbl)->w - WSURFACE(lbl,lbl->surfaceCont)->w,
		    WIDGET(lbl)->h);

	switch (lbl->type) {
	case AG_LABEL_STATIC:
		if (lbl->surface == -1) {
			AG_TextJustify(lbl->justify);
			AG_TextColor(TEXT_COLOR);
			lbl->surface = (lbl->text == NULL) ? -1 :
			    AG_WidgetMapSurface(lbl, AG_TextRender(lbl->text));
		} else if (lbl->flags & AG_LABEL_REGEN) {
			AG_TextJustify(lbl->justify);
			AG_TextColor(TEXT_COLOR);
			AG_LabelSetSurface(lbl, (lbl->text == NULL) ? NULL :
			    AG_TextRender(lbl->text));
		}
		lbl->flags &= ~(AG_LABEL_REGEN);
		if (lbl->surface != -1) {
			AG_WidgetBlitSurface(lbl, lbl->surface,
			    lbl->lPad, lbl->tPad);
		}
		break;
	case AG_LABEL_POLLED:
		DrawPolled(lbl);
		break;
	case AG_LABEL_POLLED_MT:
		AG_MutexLock(lbl->poll.lock);
		DrawPolled(lbl);
		AG_MutexUnlock(lbl->poll.lock);
		break;
	}

	if (lbl->flags & AG_LABEL_PARTIAL) {
		AG_WidgetPopClipRect(lbl);
		AG_WidgetBlitSurface(lbl, lbl->surfaceCont,
		    WIDGET(lbl)->w - WSURFACE(lbl,lbl->surfaceCont)->w,
		    lbl->tPad);
	}
	AG_MutexUnlock(&lbl->lock);
}

static void
Destroy(void *p)
{
	AG_Label *lbl = p;
	struct ag_label_flag *lfl, *lflNext;

	for (lfl = SLIST_FIRST(&lbl->lflags);
	     lfl != SLIST_END(&lbl->lflags);
	     lfl = lflNext) {
		lflNext = SLIST_NEXT(lfl, lflags);
		Free(lfl);
	}
	Free(lbl->text);
	AG_MutexDestroy(&lbl->lock);
}

/* Register a flag description text. */
void
AG_LabelFlagNew(AG_Label *lbl, Uint idx, const char *text,
    enum ag_widget_binding_type type, Uint32 v)
{
	struct ag_label_flag *lfl;

	lfl = Malloc(sizeof(struct ag_label_flag));
	lfl->idx = idx;
	lfl->text = text;
	lfl->type = type;
	lfl->v = v;
	SLIST_INSERT_HEAD(&lbl->lflags, lfl, lflags);
}

AG_WidgetClass agLabelClass = {
	{
		"AG_Widget:AG_Label",
		sizeof(AG_Label),
		{ 0,0 },
		Init,
		NULL,		/* free */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
