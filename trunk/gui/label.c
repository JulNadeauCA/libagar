/*
 * Copyright (c) 2002-2008 Hypertriton, Inc. <http://hypertriton.com/>
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
#include <core/config.h>

#include "label.h"
#include "primitive.h"
#include "text_cache.h"

#include <string.h>
#include <stdarg.h>

static AG_LabelFormatSpec *fmts = NULL;	/* Format specifiers */
static int nFmts = 0;
static AG_Mutex fmtsLock;

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
	if (!(flags & AG_LABEL_NO_HFILL)) { AG_ExpandHoriz(label); }
	if (flags & AG_LABEL_VFILL) { AG_ExpandVert(label); }
#ifdef THREADS
	label->poll.lock = NULL;
#endif
	label->tCache = agTextCache ? AG_TextCacheNew(label, 64, 16) : NULL;

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
	if (!(flags & AG_LABEL_NO_HFILL)) { AG_ExpandHoriz(label); }
	if (flags & AG_LABEL_VFILL) { AG_ExpandVert(label); }
#ifdef THREADS
	label->poll.lock = mutex;
#endif
	label->tCache = agTextCache ? AG_TextCacheNew(label, 64, 16) : NULL;

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
	
	if (lbl->flags & AG_LABEL_NOMINSIZE) {
		r->w = lbl->lPad + lbl->rPad;
		r->h = agTextFontHeight + lbl->tPad + lbl->bPad;
		return;
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
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Label *lbl = p;
	int wLbl, hLbl;
	
	if (a->w < 1 || a->h < 1) {
		return (-1);
	}
	if (lbl->type == AG_LABEL_POLLED) {
		WIDGET(lbl)->flags |= AG_WIDGET_CLIPPING;
		return (0);
	}
	if (lbl->text == NULL)
		return (0);

	/*
	 * If the widget area is too small to display the complete
	 * string, draw a "..." at the end.
	 */
	AG_TextSize(lbl->text, &wLbl, &hLbl);

	if (hLbl+lbl->tPad+lbl->bPad > a->h) {
		WIDGET(lbl)->flags |= AG_WIDGET_CLIPPING;
	} else {
		WIDGET(lbl)->flags &= ~(AG_WIDGET_CLIPPING);
	}
	if ((wLbl + lbl->lPad + lbl->rPad) > a->w) {
		lbl->flags |= AG_LABEL_PARTIAL;
		WIDGET(lbl)->flags &= ~(AG_WIDGET_CLIPPING);
		if (lbl->surfaceCont == -1) {
			/* TODO share this between all widgets */
			AG_PushTextState();
			AG_TextColor(TEXT_COLOR);
			lbl->surfaceCont = AG_WidgetMapSurface(lbl,
			    AG_TextRender(" ... "));
			AG_PopTextState();
		}
	} else {
		lbl->flags &= ~AG_LABEL_PARTIAL;
	}
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
	lbl->tCache = NULL;
	SLIST_INIT(&lbl->lflags);
	
	memset(lbl->poll.ptrs, 0, sizeof(void *)*AG_LABEL_MAX_POLLPTRS);
	lbl->poll.nptrs = 0;
}

/* Size the widget to accomodate the given text. */
void
AG_LabelSizeHint(AG_Label *lbl, Uint nlines, const char *text)
{
	int hLbl;

	AG_ObjectLock(lbl);
	if (nlines > 0) {
		AG_TextSize(text, &lbl->wPre, &hLbl);
		lbl->hPre = nlines*hLbl + (nlines-1)*agTextFontLineSkip;
	} else {
		AG_TextSize(text, &lbl->wPre, &lbl->hPre);
	}
	AG_ObjectUnlock(lbl);
}

/* Set the padding around the label in pixels. */
void
AG_LabelSetPadding(AG_Label *lbl, int lPad, int rPad, int tPad, int bPad)
{
	AG_ObjectLock(lbl);
	if (lPad != -1) { lbl->lPad = lPad; }
	if (rPad != -1) { lbl->rPad = rPad; }
	if (tPad != -1) { lbl->tPad = tPad; }
	if (bPad != -1) { lbl->bPad = bPad; }
	AG_ObjectUnlock(lbl);
}

/* Justify the text in the specified way. */
void
AG_LabelJustify(AG_Label *lbl, enum ag_text_justify justify)
{
	AG_ObjectLock(lbl);
	lbl->justify = justify;
	AG_ObjectUnlock(lbl);
}

/* Change the text displayed by the label (format string). */
void
AG_LabelText(AG_Label *lbl, const char *fmt, ...)
{
	va_list ap;

	AG_ObjectLock(lbl);
	Free(lbl->text);
	va_start(ap, fmt);
	Vasprintf(&lbl->text, fmt, ap);
	va_end(ap);
	lbl->flags |= AG_LABEL_REGEN;
	AG_ObjectUnlock(lbl);
}

/* Change the text displayed by the label. */
void
AG_LabelString(AG_Label *lbl, const char *s)
{
	AG_ObjectLock(lbl);
	Free(lbl->text);
	lbl->text = Strdup(s);
	lbl->flags |= AG_LABEL_REGEN;
	AG_ObjectUnlock(lbl);
}

/*
 * Built-in extended format specifiers.
 */
static void PrintU8(AG_Label *label, char *s, size_t len, int fPos) {
	Snprintf(s, len, "%u", AG_LABEL_ARG(Uint8));
}
static void PrintS8(AG_Label *label, char *s, size_t len, int fPos) {
	Snprintf(s, len, "%d", AG_LABEL_ARG(Sint8));
}
static void PrintU16(AG_Label *label, char *s, size_t len, int fPos) {
	Snprintf(s, len, "%u", AG_LABEL_ARG(Uint16));
}
static void PrintS16(AG_Label *label, char *s, size_t len, int fPos) {
	Snprintf(s, len, "%d", AG_LABEL_ARG(Sint16));
}
static void PrintU32(AG_Label *label, char *s, size_t len, int fPos) {
	Snprintf(s, len, "%u", AG_LABEL_ARG(Uint32));
}
static void PrintS32(AG_Label *label, char *s, size_t len, int fPos) {
	Snprintf(s, len, "%d", AG_LABEL_ARG(Sint32));
}
static void
PrintOBJNAME(AG_Label *label, char *s, size_t len, int fPos)
{
	AG_Object *ob = AG_LABEL_ARG(AG_Object *);
	Snprintf(s, len, "%s", ob != NULL ? ob->name : "(null)");
}
static void
PrintOBJTYPE(AG_Label *label, char *s, size_t len, int fPos)
{
	AG_Object *ob = AG_LABEL_ARG(AG_Object *);
	Snprintf(s, len, "%s", ob->cls->name);
}
static void
PrintIBOOL(AG_Label *label, char *s, size_t len, int fPos)
{
	int *flag = &AG_LABEL_ARG(int);
	Snprintf(s, len, "%s", *flag ? _("yes") : _("no"));
}
static void
PrintFLAGS(AG_Label *label, char *s, size_t len, int fPos)
{
	Uint *flags = &AG_LABEL_ARG(Uint);
	struct ag_label_flag *lfl;
	s[0] = '\0';
	SLIST_FOREACH(lfl, &label->lflags, lflags) {
		if (lfl->idx == fPos && *flags & (Uint)lfl->v) {
			if (s[0] != '\0') { Strlcat(s, ", ", len); }
			Strlcat(s, lfl->text, len);
		}
	}
}
static void
PrintFLAGS8(AG_Label *label, char *s, size_t len, int fPos)
{
	Uint8 *flags = &AG_LABEL_ARG(Uint8);
	struct ag_label_flag *lfl;
	s[0] = '\0';
	SLIST_FOREACH(lfl, &label->lflags, lflags) {
		if (lfl->idx == fPos && *flags & (Uint8)lfl->v) {
			if (s[0] != '\0') { Strlcat(s, ", ", len); }
			Strlcat(s, lfl->text, len);
		}
	}
}
static void
PrintFLAGS16(AG_Label *label, char *s, size_t len, int fPos)
{
	Uint16 *flags = &AG_LABEL_ARG(Uint16);
	struct ag_label_flag *lfl;
	s[0] = '\0';
	SLIST_FOREACH(lfl, &label->lflags, lflags) {
		if (lfl->idx == fPos && *flags & (Uint16)lfl->v) {
			if (s[0] != '\0') { Strlcat(s, ", ", len); }
			Strlcat(s, lfl->text, len);
		}
	}
}
static void
PrintFLAGS32(AG_Label *label, char *s, size_t len, int fPos)
{
	Uint32 *flags = &AG_LABEL_ARG(Uint32);
	struct ag_label_flag *lfl;
	s[0] = '\0';
	SLIST_FOREACH(lfl, &label->lflags, lflags) {
		if (lfl->idx == fPos && *flags & (Uint32)lfl->v) {
			if (s[0] != '\0') { Strlcat(s, ", ", len); }
			Strlcat(s, lfl->text, len);
		}
	}
}

/* Register built-in format specifiers. */
void
AG_RegisterBuiltinLabelFormats(void)
{
	AG_MutexInit(&fmtsLock);
	AG_RegisterLabelFormat("u8", PrintU8);
	AG_RegisterLabelFormat("s8", PrintS8);
	AG_RegisterLabelFormat("u16", PrintU16);
	AG_RegisterLabelFormat("s16", PrintS16);
	AG_RegisterLabelFormat("u32", PrintU32);
	AG_RegisterLabelFormat("s32", PrintS32);
	AG_RegisterLabelFormat("objname", PrintOBJNAME);
	AG_RegisterLabelFormat("objtype", PrintOBJTYPE);
	AG_RegisterLabelFormat("ibool", PrintIBOOL);
	AG_RegisterLabelFormat("flags", PrintFLAGS);
	AG_RegisterLabelFormat("flags8", PrintFLAGS8);
	AG_RegisterLabelFormat("flags16", PrintFLAGS16);
	AG_RegisterLabelFormat("flags32", PrintFLAGS32);
}

/* Register a new format specifier. */
void
AG_RegisterLabelFormat(const char *fmt, AG_LabelFormatFn fn)
{
	AG_LabelFormatSpec *fs;

	AG_MutexLock(&fmtsLock);
	fmts = Realloc(fmts, (nFmts+1)*sizeof(AG_LabelFormatSpec));
	fs = &fmts[nFmts++];
	fs->fmt = Strdup(fmt);
	fs->fmtLen = strlen(fmt);
	fs->fn = fn;
	AG_MutexUnlock(&fmtsLock);
}

#define PF(fmt,arg) \
	Snprintf(s2, AG_LABEL_MAX, (fmt), (arg)); \
	Strlcat(s, s2, AG_LABEL_MAX); \
	fPos++
#define PSTRING(ps) \
	Strlcat(s, ps, AG_LABEL_MAX); \
	fPos++

#ifdef HAVE_64BIT
/* Print a polled 64-bit value (%ll*). */
static int
PrintPolled64(AG_Label *label, const char *f, char *s, char *s2)
{
	int fPos = 0;

	switch (*f) {
# ifdef HAVE_LONG_DOUBLE
	case 'f':
		PF("%.2Lf", AG_LABEL_ARG(long double));
		break;
	case 'g':
		PF("%Lg", AG_LABEL_ARG(long double));
		break;
# endif
	case 'd':
	case 'i':
		PF("%lld", (long long)AG_LABEL_ARG(Sint64));
		break;
	case 'o':
		PF("%llo", (unsigned long long)AG_LABEL_ARG(Uint64));
		break;
	case 'u':
		PF("%llu", (unsigned long long)AG_LABEL_ARG(Uint64));
		break;
	case 'x':
		PF("%llx", (unsigned long long)AG_LABEL_ARG(Uint64));
		break;
	case 'X':
		PF("%llX", (unsigned long long)AG_LABEL_ARG(Uint64));
		break;
	}
	return (fPos);
}
#endif /* HAVE_64BIT */

/* Display a polled label. */
static void
DrawPolled(AG_Label *label)
{
	char s[AG_LABEL_MAX];
	char s2[AG_LABEL_MAX];
	char *f;
	int i, fPos = 0;

	if (label->text == NULL || label->text[0] == '\0') {
		return;
	}
	s[0] = '\0';
	s2[0] = '\0';

	for (f = label->text; *f != '\0'; f++) {
		if (f[0] == '%' && f[1] != '\0') {
			switch (f[1]) {
			case 'l':
				switch (f[2]) {
				case 'f':
					PF("%.2f", AG_LABEL_ARG(double));
					break;
				case 'g':
					PF("%g", AG_LABEL_ARG(double));
					break;
#ifdef HAVE_64BIT
				case 'l':
					fPos += PrintPolled64(label, &f[3],
					    s, s2);
					f += 2;
					break;
#endif /* HAVE_64BIT */
				}
				break;
			case 'd':
			case 'i':
				PF("%d", AG_LABEL_ARG(int));
				break;
			case 'o':
				PF("%o", AG_LABEL_ARG(unsigned int));
				break;
			case 'u':
				PF("%u", AG_LABEL_ARG(unsigned int));
				break;
			case 'x':
				PF("%x", AG_LABEL_ARG(unsigned int));
				break;
			case 'X':
				PF("%X", AG_LABEL_ARG(unsigned int));
				break;
			case 'c':
				s2[0] = AG_LABEL_ARG(char);
				s2[1] = '\0';
				PSTRING(s2);
				break;
			case 's':
				PSTRING(&AG_LABEL_ARG(char));
				break;
			case 'p':
				PF("%p", AG_LABEL_ARG(void *));
				break;
			case 'f':
				PF("%.2f", AG_LABEL_ARG(float));
				break;
			case 'g':
				PF("%g", AG_LABEL_ARG(float));
				break;
			case '[':
				for (i = 0; i < nFmts; i++) {
					if (strncmp(fmts[i].fmt, &f[2],
					    fmts[i].fmtLen) != 0) {
						continue;
					}
					fmts[i].fn(label, s2, sizeof(s2), fPos);
					f += fmts[i].fmtLen+1;
					PSTRING(s2);
					break;
				}
				break;
			case '%':
				s2[0] = '%';
				s2[1] = '\0';
				Strlcat(s, s2, sizeof(s));
				break;
			}
			f++;
		} else {
			s2[0] = *f;
			s2[1] = '\0';
			Strlcat(s, s2, sizeof(s));
		}
	}

	AG_PushTextState();
	AG_TextJustify(label->justify);
	AG_TextColor(TEXT_COLOR);
	if (agTextCache) {
		AG_WidgetBlitSurface(label,
		    AG_TextCacheInsLookup(label->tCache,s),
		    label->lPad,
		    label->tPad);
	} else {
		AG_Surface *suTmp;
		suTmp = AG_TextRender(s);
		AG_WidgetBlit(label, suTmp, label->lPad, label->tPad);
		AG_SurfaceFree(suTmp);
	}
	AG_PopTextState();
}

static void
Draw(void *p)
{
	AG_Label *lbl = p;

	if (lbl->flags & AG_LABEL_PARTIAL) {
		AG_WidgetPushClipRect(lbl,
		    AG_RECT(0, 0,
		            WIDGET(lbl)->w - WSURFACE(lbl,lbl->surfaceCont)->w,
		            WIDGET(lbl)->h));
	}

	switch (lbl->type) {
	case AG_LABEL_STATIC:
		AG_PushTextState();
		if (lbl->surface == -1) {
			AG_TextJustify(lbl->justify);
			AG_TextColor(TEXT_COLOR);
			lbl->surface = (lbl->text == NULL) ? -1 :
			    AG_WidgetMapSurface(lbl, AG_TextRender(lbl->text));
		} else if (lbl->flags & AG_LABEL_REGEN) {
			AG_TextJustify(lbl->justify);
			AG_TextColor(TEXT_COLOR);
			if (lbl->text != NULL) {
				AG_WidgetReplaceSurface(lbl, 0,
				    AG_TextRender(lbl->text));
			} else {
				lbl->surface = -1;
			}
		}
		AG_PopTextState();

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

	if (lbl->tCache != NULL)
		AG_TextCacheDestroy(lbl->tCache);
}

/* Register a flag description text. */
void
AG_LabelFlagNew(AG_Label *lbl, Uint idx, const char *text,
    enum ag_widget_binding_type type, Uint32 v)
{
	struct ag_label_flag *lfl;

	AG_ObjectLock(lbl);
	lfl = Malloc(sizeof(struct ag_label_flag));
	lfl->idx = idx;
	lfl->text = text;
	lfl->type = type;
	lfl->v = v;
	SLIST_INSERT_HEAD(&lbl->lflags, lfl, lflags);
	AG_ObjectUnlock(lbl);
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
