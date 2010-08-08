/*
 * Copyright (c) 2002-2010 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "gui.h"
#include "label.h"
#include "primitive.h"
#include "text_cache.h"

#include <string.h>
#include <stdarg.h>

#include <core/snprintf.h>

static AG_LabelFormatSpec *fmts = NULL;		/* Extended format specifiers */
static int nFmts = 0;
#ifdef AG_THREADS
static AG_Mutex fmtsLock;
#endif

/* Create a new polled label. */
AG_Label *
AG_LabelNewPolled(void *parent, Uint flags, const char *fmt, ...)
{
	AG_Label *lbl;
	va_list ap;
	const char *p;
	
	lbl = Malloc(sizeof(AG_Label));
	AG_ObjectInit(lbl, &agLabelClass);

	lbl->type = AG_LABEL_POLLED;
	lbl->text = Strdup(fmt);
	lbl->flags |= flags;
	if (flags & AG_LABEL_HFILL) { AG_ExpandHoriz(lbl); }
	if (flags & AG_LABEL_VFILL) { AG_ExpandVert(lbl); }
#ifdef AG_THREADS
	lbl->poll.lock = NULL;
#endif
	lbl->tCache = agTextCache ? AG_TextCacheNew(lbl, 64, 16) : NULL;

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
			if (lbl->poll.nptrs+1 < AG_LABEL_MAX_POLLPTRS) {
				lbl->poll.ptrs[lbl->poll.nptrs++] =
				    va_arg(ap, void *);
			}
			break;
		}
	}
	va_end(ap);

	AG_RedrawOnTick(lbl, 500);
	AG_ObjectAttach(parent, lbl);
	return (lbl);
}

/* Create a new polled label associated with a mutex. */
AG_Label *
AG_LabelNewPolledMT(void *parent, Uint flags, AG_Mutex *mutex,
    const char *fmt, ...)
{
	AG_Label *lbl;
	va_list ap;
	const char *p;
	
	lbl = Malloc(sizeof(AG_Label));
	AG_ObjectInit(lbl, &agLabelClass);

	lbl->type = AG_LABEL_POLLED_MT;
	lbl->text = Strdup(fmt);
	lbl->flags |= flags;
	if (flags & AG_LABEL_HFILL) { AG_ExpandHoriz(lbl); }
	if (flags & AG_LABEL_VFILL) { AG_ExpandVert(lbl); }
#ifdef AG_THREADS
	lbl->poll.lock = mutex;
#endif
	lbl->tCache = agTextCache ? AG_TextCacheNew(lbl, 64, 16) : NULL;

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
			if (lbl->poll.nptrs+1 < AG_LABEL_MAX_POLLPTRS) {
				lbl->poll.ptrs[lbl->poll.nptrs++] =
				    va_arg(ap, void *);
			}
			break;
		}
	}
	va_end(ap);

	AG_RedrawOnTick(lbl, 500);
	AG_ObjectAttach(parent, lbl);
	return (lbl);
}

/* Create a static label (format string). */
AG_Label *
AG_LabelNew(void *parent, Uint flags, const char *fmt, ...)
{
	AG_Label *lbl;
	va_list ap;

	lbl = Malloc(sizeof(AG_Label));
	AG_ObjectInit(lbl, &agLabelClass);

	lbl->type = AG_LABEL_STATIC;
	lbl->flags |= flags;
	if (flags & AG_LABEL_HFILL) { AG_ExpandHoriz(lbl); }
	if (flags & AG_LABEL_VFILL) { AG_ExpandVert(lbl); }
	if (fmt != NULL) {
		va_start(ap, fmt);
		Vasprintf(&lbl->text, fmt, ap);
		va_end(ap);
	} else {
		lbl->text = NULL;
	}

	AG_ObjectAttach(parent, lbl);
	return (lbl);
}

/* Create a static label (C string). */
AG_Label *
AG_LabelNewS(void *parent, Uint flags, const char *text)
{
	AG_Label *lbl;
	
	lbl = Malloc(sizeof(AG_Label));
	AG_ObjectInit(lbl, &agLabelClass);

	lbl->type = AG_LABEL_STATIC;
	lbl->flags |= flags;
	if (flags & AG_LABEL_HFILL) { AG_ExpandHoriz(lbl); }
	if (flags & AG_LABEL_VFILL) { AG_ExpandVert(lbl); }
	lbl->text = (text != NULL) ? Strdup(text) : NULL;

	AG_ObjectAttach(parent, lbl);
	return (lbl);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Label *lbl = obj;
	
	if (lbl->flags & AG_LABEL_NOMINSIZE) {
		r->w = lbl->lPad + lbl->rPad;
		r->h = agTextFontHeight + lbl->tPad + lbl->bPad;
		return;
	}
	switch (lbl->type) {
	case AG_LABEL_STATIC:
		AG_TextSize(lbl->text, &r->w, &r->h);
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
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Label *lbl = obj;
	int wLbl, hLbl;
	
	if (a->w < 1 || a->h < 1) {
		return (-1);
	}
	lbl->rClip.x = lbl->lPad;
	lbl->rClip.y = lbl->tPad;
	lbl->rClip.w = a->w - lbl->rPad;
	lbl->rClip.h = a->h - lbl->bPad;

	if (lbl->text == NULL)
		return (0);

	/*
	 * If the widget area is too small to display the complete
	 * string, draw a "..." at the end.
	 */
	AG_TextSize(lbl->text, &wLbl, &hLbl);

	if ((wLbl + lbl->lPad + lbl->rPad) > a->w) {
		lbl->flags |= AG_LABEL_PARTIAL;
		if (lbl->surfaceCont == -1) {
			/* TODO share this between all widgets */
			AG_PushTextState();
			AG_TextColor(agColors[TEXT_COLOR]);
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
	lbl->valign = AG_TEXT_TOP;
	lbl->tCache = NULL;
	lbl->rClip = AG_RECT(0,0,0,0);		/* Initialized in SizeAlloc() */
	SLIST_INIT(&lbl->lflags);
	
	memset(lbl->poll.ptrs, 0, sizeof(void *)*AG_LABEL_MAX_POLLPTRS);
	lbl->poll.nptrs = 0;

#ifdef AG_DEBUG
	AG_BindUint(lbl, "flags", &lbl->flags);
	AG_BindPointer(lbl, "text", (void *)&lbl->text);
	AG_BindInt(lbl, "surface", &lbl->surface);
	AG_BindInt(lbl, "surfaceCont", &lbl->surfaceCont);
	AG_BindInt(lbl, "wPre", &lbl->wPre);
	AG_BindInt(lbl, "hPre", &lbl->hPre);
	AG_BindInt(lbl, "lPad", &lbl->lPad);
	AG_BindInt(lbl, "rPad", &lbl->rPad);
	AG_BindInt(lbl, "tPad", &lbl->tPad);
	AG_BindInt(lbl, "bPad", &lbl->bPad);
	AG_BindUint(lbl, "justify", &lbl->justify);
	AG_BindUint(lbl, "valign", &lbl->valign);
#endif /* AG_DEBUG */
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
	AG_Redraw(lbl);
}

/* Justify the text in the specified way. */
void
AG_LabelJustify(AG_Label *lbl, enum ag_text_justify justify)
{
	AG_ObjectLock(lbl);
	lbl->justify = justify;
	AG_ObjectUnlock(lbl);
	AG_Redraw(lbl);
}

/* Vertically align the text in the specified way. */
void
AG_LabelValign(AG_Label *lbl, enum ag_text_valign valign)
{
	AG_ObjectLock(lbl);
	lbl->valign = valign;
	AG_ObjectUnlock(lbl);
	AG_Redraw(lbl);
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
	AG_Redraw(lbl);
}

/* Change the text displayed by the label (C string). */
void
AG_LabelTextS(AG_Label *lbl, const char *s)
{
	AG_ObjectLock(lbl);
	Free(lbl->text);
	lbl->text = Strdup(s);
	lbl->flags |= AG_LABEL_REGEN;
	AG_ObjectUnlock(lbl);
	AG_Redraw(lbl);
}

/*
 * Built-in extended format specifiers (table follows).
 */
static void PrintU8(AG_Label *lbl, char *s, size_t len, int fPos) {
	StrlcpyUint(s, (unsigned int)AG_LABEL_ARG(lbl,Uint8), len);
}
static void PrintS8(AG_Label *lbl, char *s, size_t len, int fPos) {
	StrlcpyInt(s, (int)AG_LABEL_ARG(lbl,Sint8), len);
}
static void PrintU16(AG_Label *lbl, char *s, size_t len, int fPos) {
	StrlcpyUint(s, (unsigned int)AG_LABEL_ARG(lbl,Uint16), len);
}
static void PrintS16(AG_Label *lbl, char *s, size_t len, int fPos) {
	StrlcpyInt(s, (int)AG_LABEL_ARG(lbl,Sint16), len);
}
static void PrintU32(AG_Label *lbl, char *s, size_t len, int fPos) {
	StrlcpyUint(s, (unsigned int)AG_LABEL_ARG(lbl,Uint32), len);
}
static void PrintS32(AG_Label *lbl, char *s, size_t len, int fPos) {
	StrlcpyInt(s, (int)AG_LABEL_ARG(lbl,Sint32), len);
}
static void
PrintOBJNAME(AG_Label *lbl, char *s, size_t len, int fPos)
{
	AG_Object *ob = AG_LABEL_ARG(lbl,AG_Object *);

	if (ob != NULL) {
		Strlcpy(s, ob->name, len);
	} else {
		Strlcpy(s, "(null)", len);
	}
}
static void
PrintOBJTYPE(AG_Label *lbl, char *s, size_t len, int fPos)
{
	AG_Object *ob = AG_LABEL_ARG(lbl,AG_Object *);

	Strlcpy(s, ob->cls->name, len);
}
static void
PrintIBOOL(AG_Label *lbl, char *s, size_t len, int fPos)
{
	int *flag = &AG_LABEL_ARG(lbl,int);

	Strlcpy(s, *flag ? _("yes") : _("no"), len);
}
static void
PrintFLAGS(AG_Label *lbl, char *s, size_t len, int fPos)
{
	Uint *flags = &AG_LABEL_ARG(lbl,Uint);
	struct ag_label_flag *lfl;

	s[0] = '\0';
	SLIST_FOREACH(lfl, &lbl->lflags, lflags) {
		if (lfl->idx == fPos && *flags & (Uint)lfl->v) {
			if (s[0] != '\0') { Strlcat(s, ", ", len); }
			Strlcat(s, lfl->text, len);
		}
	}
}
static void
PrintFLAGS8(AG_Label *lbl, char *s, size_t len, int fPos)
{
	Uint8 *flags = &AG_LABEL_ARG(lbl,Uint8);
	struct ag_label_flag *lfl;

	s[0] = '\0';
	SLIST_FOREACH(lfl, &lbl->lflags, lflags) {
		if (lfl->idx == fPos && *flags & (Uint8)lfl->v) {
			if (s[0] != '\0') { Strlcat(s, ", ", len); }
			Strlcat(s, lfl->text, len);
		}
	}
}
static void
PrintFLAGS16(AG_Label *lbl, char *s, size_t len, int fPos)
{
	Uint16 *flags = &AG_LABEL_ARG(lbl,Uint16);
	struct ag_label_flag *lfl;

	s[0] = '\0';
	SLIST_FOREACH(lfl, &lbl->lflags, lflags) {
		if (lfl->idx == fPos && *flags & (Uint16)lfl->v) {
			if (s[0] != '\0') { Strlcat(s, ", ", len); }
			Strlcat(s, lfl->text, len);
		}
	}
}
static void
PrintFLAGS32(AG_Label *lbl, char *s, size_t len, int fPos)
{
	Uint32 *flags = &AG_LABEL_ARG(lbl,Uint32);
	struct ag_label_flag *lfl;

	s[0] = '\0';
	SLIST_FOREACH(lfl, &lbl->lflags, lflags) {
		if (lfl->idx == fPos && *flags & (Uint32)lfl->v) {
			if (s[0] != '\0') { Strlcat(s, ", ", len); }
			Strlcat(s, lfl->text, len);
		}
	}
}
static const struct {
	const char *name;
	AG_LabelFormatFn fn;
} builtinFmts[] = {
	{ "u8",		PrintU8 },
	{ "s8",		PrintS8 },
	{ "u16",	PrintU16 },
	{ "s16",	PrintS16 },
	{ "u32",	PrintU32 },
	{ "s32",	PrintS32 },
	{ "objname",	PrintOBJNAME },
	{ "objtype",	PrintOBJTYPE },
	{ "ibool",	PrintIBOOL },
	{ "flags",	PrintFLAGS },
	{ "flags8",	PrintFLAGS8 },
	{ "flags16",	PrintFLAGS16 },
	{ "flags32",	PrintFLAGS32 }
};
static const int nBuiltinFmts = sizeof(builtinFmts) / sizeof(builtinFmts[0]);

/* Register built-in format specifiers. */
void
AG_LabelInitFormats(void)
{
	int i;

	AG_MutexInit(&fmtsLock);
	for (i = 0; i < nBuiltinFmts; i++)
		AG_RegisterLabelFormat(builtinFmts[i].name, builtinFmts[i].fn);
}

/* Destroy built-in format specifiers. */
void
AG_LabelDestroyFormats(void)
{
	int i;

	for (i = 0; i < nFmts; i++) {
		Free(fmts[i].fmt);
	}
	Free(fmts);
	fmts = NULL;
	nFmts = 0;
	
	AG_MutexDestroy(&fmtsLock);
}

/* Register a new format specifier. */
void
AG_RegisterLabelFormat(const char *fmt, AG_LabelFormatFn fn)
{
	AG_LabelFormatSpec *fs;

	if (!agGUI) {
		return;
	}
	AG_MutexLock(&fmtsLock);
	fmts = Realloc(fmts, (nFmts+1)*sizeof(AG_LabelFormatSpec));
	fs = &fmts[nFmts++];
	fs->fmt = Strdup(fmt);
	fs->fmtLen = strlen(fmt);
	fs->fn = fn;
	AG_MutexUnlock(&fmtsLock);
}

/* Unregister a format specifier. */
void
AG_UnregisterLabelFormat(const char *fmt)
{
	int i;

	if (!agGUI)
		return;

	AG_MutexLock(&fmtsLock);

	for (i = 0; i < nFmts; i++) {
		if (strcmp(fmts[i].fmt, fmt) == 0)
			break;
	}
	if (i < nFmts) {
		Free(fmts[i].fmt);
		if (i < nFmts-1) {
			memmove(&fmts[i], &fmts[i+1],
			    (nFmts-1)*sizeof(AG_LabelFormatSpec));
		}
		nFmts--;
	}
	AG_MutexUnlock(&fmtsLock);
}

#define PINT(arg)   StrlcatInt(s, (arg), AG_LABEL_MAX); fPos++
#define PUINT(arg)  StrlcatUint(s, (arg), AG_LABEL_MAX); fPos++
#define PSTRING(ps) Strlcat(s, ps, AG_LABEL_MAX); fPos++
#define PF(fmt,arg) \
	Snprintf(s2, AG_LABEL_MAX, (fmt), (arg)); \
	Strlcat(s, s2, AG_LABEL_MAX); \
	fPos++

#ifdef HAVE_64BIT
/* Print a polled 64-bit value (%ll*). */
static int
PrintPolled64(AG_Label *lbl, const char *f, char *s, char *s2)
{
	int fPos = 0;

	switch (*f) {
# ifdef HAVE_LONG_DOUBLE
	case 'f':
		PF("%.2Lf", AG_LABEL_ARG(lbl,long double));
		break;
	case 'g':
		PF("%Lg", AG_LABEL_ARG(lbl,long double));
		break;
# endif
	case 'd':
	case 'i':
		PF("%lld", (long long)AG_LABEL_ARG(lbl,Sint64));
		break;
	case 'o':
		PF("%llo", (unsigned long long)AG_LABEL_ARG(lbl,Uint64));
		break;
	case 'u':
		PF("%llu", (unsigned long long)AG_LABEL_ARG(lbl,Uint64));
		break;
	case 'x':
		PF("%llx", (unsigned long long)AG_LABEL_ARG(lbl,Uint64));
		break;
	case 'X':
		PF("%llX", (unsigned long long)AG_LABEL_ARG(lbl,Uint64));
		break;
	}
	return (fPos);
}
#endif /* HAVE_64BIT */

static __inline__ void
GetPosition(AG_Label *lbl, AG_Surface *su, int *x, int *y)
{
	*x = lbl->lPad +
	     AG_TextJustifyOffset(WIDTH(lbl) - (lbl->lPad+lbl->rPad), su->w);
	*y = lbl->tPad +
	     AG_TextValignOffset(HEIGHT(lbl) - (lbl->tPad+lbl->bPad), su->h);
}

/* Display a polled label. */
static void
DrawPolled(AG_Label *lbl)
{
	char s[AG_LABEL_MAX];
	char s2[AG_LABEL_MAX];
	char *f;
	int i, fPos = 0;
	int x, y;

	if (lbl->text == NULL || lbl->text[0] == '\0') {
		return;
	}
	s[0] = '\0';
	s2[0] = '\0';

	for (f = lbl->text; *f != '\0'; f++) {
		if (f[0] == '%' && f[1] != '\0') {
			switch (f[1]) {
			case 'l':
				switch (f[2]) {
				case 'f':
					PF("%.2f", AG_LABEL_ARG(lbl,double));
					f++;
					break;
				case 'g':
					PF("%g", AG_LABEL_ARG(lbl,double));
					f++;
					break;
#ifdef HAVE_64BIT
				case 'l':
					fPos += PrintPolled64(lbl, &f[3], s,
					    s2);
					f++;
					break;
#endif /* HAVE_64BIT */
				}
				break;
			case 'd':
			case 'i':
				PINT(AG_LABEL_ARG(lbl,int));
				break;
			case 'u':
				PUINT(AG_LABEL_ARG(lbl,unsigned int));
				break;
			case 'o':
				PF("%o", AG_LABEL_ARG(lbl,unsigned int));
				break;
			case 'x':
				PF("%x", AG_LABEL_ARG(lbl,unsigned int));
				break;
			case 'X':
				PF("%X", AG_LABEL_ARG(lbl,unsigned int));
				break;
			case 'c':
				s2[0] = AG_LABEL_ARG(lbl,char);
				s2[1] = '\0';
				PSTRING(s2);
				break;
			case 's':
				PSTRING(&AG_LABEL_ARG(lbl,char));
				break;
			case 'p':
				PF("%p", AG_LABEL_ARG(lbl,void *));
				break;
			case 'f':
				PF("%.2f", AG_LABEL_ARG(lbl,float));
				break;
			case 'g':
				PF("%g", AG_LABEL_ARG(lbl,float));
				break;
			case '[':
				for (i = 0; i < nFmts; i++) {
					if (strncmp(fmts[i].fmt, &f[2],
					    fmts[i].fmtLen) != 0) {
						continue;
					}
					fmts[i].fn(lbl, s2, sizeof(s2), fPos);
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

	if (agTextCache) {
		int su = AG_TextCacheGet(lbl->tCache,s);
		GetPosition(lbl, WSURFACE(lbl,su), &x, &y);
		AG_WidgetBlitSurface(lbl, su, x, y);
	} else {
		AG_Surface *su = AG_TextRender(s);
		GetPosition(lbl, su, &x, &y);
		AG_WidgetBlit(lbl, su, x, y);
		AG_SurfaceFree(su);
	}
}

#undef PINT
#undef PUINT
#undef PSTRING
#undef PF

static void
Draw(void *obj)
{
	AG_Label *lbl = obj;
	int x, y, cw = 0;			/* make compiler happy */

	if (lbl->flags & AG_LABEL_FRAME)
		AG_DrawFrame(lbl,
		    AG_RECT(0, 0, WIDTH(lbl), HEIGHT(lbl)), -1,
		    agColors[FRAME_COLOR]);
	
	if (lbl->flags & AG_LABEL_PARTIAL) {
		cw = WSURFACE(lbl,lbl->surfaceCont)->w;
		if (WIDTH(lbl) <= cw) {
			AG_PushClipRect(lbl,
			    AG_RECT(0, 0, WIDTH(lbl), HEIGHT(lbl)));
			AG_WidgetBlitSurface(lbl, lbl->surfaceCont,
			    0, lbl->tPad);
			AG_PopClipRect(lbl);
			return;
		}
		AG_PushClipRect(lbl,
		    AG_RECT(0, 0, WIDTH(lbl)-cw, HEIGHT(lbl)));
	} else {
		AG_PushClipRect(lbl, lbl->rClip);
	}
	
	AG_PushTextState();
	AG_TextJustify(lbl->justify);
	AG_TextValign(lbl->valign);
	AG_TextColor(agColors[TEXT_COLOR]);

	switch (lbl->type) {
	case AG_LABEL_STATIC:
		if (lbl->surface == -1) {
			lbl->surface = (lbl->text == NULL) ? -1 :
			    AG_WidgetMapSurface(lbl, AG_TextRender(lbl->text));
		} else if (lbl->flags & AG_LABEL_REGEN) {
			if (lbl->text != NULL) {
				AG_WidgetReplaceSurface(lbl, 0,
				    AG_TextRender(lbl->text));
			} else {
				lbl->surface = -1;
			}
		}
		lbl->flags &= ~(AG_LABEL_REGEN);
		if (lbl->surface != -1) {
			GetPosition(lbl, WSURFACE(lbl,lbl->surface), &x, &y);
			AG_WidgetBlitSurface(lbl, lbl->surface, x, y);
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
	
	AG_PopClipRect(lbl);
	
	if (lbl->flags & AG_LABEL_PARTIAL) {
		GetPosition(lbl, WSURFACE(lbl,lbl->surfaceCont), &x, &y);
		AG_WidgetBlitSurface(lbl, lbl->surfaceCont,
		    WIDTH(lbl) - cw,
		    y);
	}
	AG_PopTextState();
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
AG_LabelFlagNew(AG_Label *lbl, Uint idx, const char *text, AG_VariableType type,
    Uint32 v)
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
	AG_Redraw(lbl);
}

AG_WidgetClass agLabelClass = {
	{
		"Agar(Widget:Label)",
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
