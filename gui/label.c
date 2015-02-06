/*
 * Copyright (c) 2002-2012 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/core/core.h>
#include <agar/gui/gui.h>
#include <agar/gui/window.h>
#include <agar/gui/label.h>
#include <agar/gui/primitive.h>
#include <agar/gui/text_cache.h>

#include <string.h>
#include <stdarg.h>

/*
 * Create a new polled label (AG_LEGACY: API predates the generalization of
 * the formatting engine, in 2.0 this will take an AG_FmtString argument).
 */
AG_Label *
AG_LabelNewPolled(void *parent, Uint flags, const char *fmt, ...)
{
	AG_Label *lbl;
	AG_FmtString *fs;
	va_list ap;
	const char *p;
	
	lbl = Malloc(sizeof(AG_Label));
	AG_ObjectInit(lbl, &agLabelClass);

	lbl->type = AG_LABEL_POLLED;
	lbl->tCache = AG_TextCacheNew(lbl, 32, 4);
	lbl->pollBufSize = AG_FMTSTRING_BUFFER_INIT;
	lbl->pollBuf = Malloc(lbl->pollBufSize);

	lbl->flags |= flags;
	if (flags & AG_LABEL_HFILL) { AG_ExpandHoriz(lbl); }
	if (flags & AG_LABEL_VFILL) { AG_ExpandVert(lbl); }

	/* AG_LEGACY */
	/* Build the format string */
	fs = lbl->fmt = Malloc(sizeof(AG_FmtString));
	fs->s = Strdup(fmt);
	fs->n = 0;
	va_start(ap, fmt);
	for (p = fmt; *p != '\0'; p++) {
		if (*p != '%') {
			continue;
		}
		switch (p[1]) {
		case '%':
		case ' ':
			p++;
			break;
		case '\0':
			break;
		default:
			if (fs->n+1 >= AG_STRING_POINTERS_MAX) {
				AG_FatalError("Too many arguments");
			}
			fs->p[fs->n] = va_arg(ap, void *);
			fs->mu[fs->n] = NULL;
			fs->n++;
			break;
		}
	}
	va_end(ap);
	/* AG_LEGACY */

	AG_RedrawOnTick(lbl, 500);
	AG_ObjectAttach(parent, lbl);
	return (lbl);
}

/*
 * Create a new polled label (AG_LEGACY: API predates the generalization of
 * the formatting engine, in 2.0 this will take an AG_FmtString argument).
 */
AG_Label *
AG_LabelNewPolledMT(void *parent, Uint flags, AG_Mutex *mu, const char *fmt, ...)
{
	AG_Label *lbl;
	AG_FmtString *fs;
	va_list ap;
	const char *p;
	
	lbl = Malloc(sizeof(AG_Label));
	AG_ObjectInit(lbl, &agLabelClass);

	lbl->type = AG_LABEL_POLLED;
	lbl->tCache = AG_TextCacheNew(lbl, 32, 4);
	lbl->pollBufSize = AG_FMTSTRING_BUFFER_INIT;
	lbl->pollBuf = Malloc(lbl->pollBufSize);

	lbl->flags |= flags;
	if (flags & AG_LABEL_HFILL) { AG_ExpandHoriz(lbl); }
	if (flags & AG_LABEL_VFILL) { AG_ExpandVert(lbl); }

	/* Build the format string (legacy style) */
	if ((fs = lbl->fmt = TryMalloc(sizeof(AG_FmtString))) == NULL) {
		AG_FatalError(NULL);
	}
	fs->s = Strdup(fmt);
	fs->n = 0;
	va_start(ap, fmt);
	for (p = fmt; *p != '\0'; p++) {
		if (*p != '%') {
			continue;
		}
		switch (p[1]) {
		case '%':
		case ' ':
			p++;
			break;
		case '\0':
			break;
		default:
			if (fs->n+1 >= AG_STRING_POINTERS_MAX) {
				AG_FatalError("Too many arguments");
			}
			fs->p[fs->n] = va_arg(ap, void *);
			fs->mu[fs->n] = mu;
			fs->n++;
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
	AG_Font *font = WIDGET(lbl)->font;
	
	if (lbl->flags & AG_LABEL_NOMINSIZE) {
		r->w = lbl->lPad + lbl->rPad;
		r->h = font->height + lbl->tPad + lbl->bPad;
		return;
	}
	switch (lbl->type) {
	case AG_LABEL_STATIC:
		AG_TextSize(lbl->text, &r->w, &r->h);
		r->w += lbl->lPad + lbl->rPad;
		r->h += lbl->tPad + lbl->bPad;
		break;
	case AG_LABEL_POLLED:
		r->w = lbl->wPre + lbl->lPad + lbl->rPad;
		r->h = lbl->hPre*font->lineskip + lbl->tPad + lbl->bPad;
		break;
	}
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Label *lbl = obj;
	int wLbl, hLbl;
	AG_Surface *s;
	
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
	 * string, render the label partially.
	 */
	AG_TextSize(lbl->text, &wLbl, &hLbl);

	if ((wLbl + lbl->lPad + lbl->rPad) > a->w) {
		lbl->flags |= AG_LABEL_PARTIAL;
		if (lbl->surfaceCont == -1 &&
		    (s = AG_TextRender("... ")) != NULL) {
			lbl->surfaceCont = AG_WidgetMapSurface(lbl, s);
		}
	} else {
		lbl->flags &= ~AG_LABEL_PARTIAL;
	}
	return (0);
}

static void
OnFontChange(AG_Event *event)
{
	AG_Label *lbl = AG_SELF();

	if (lbl->tCache != NULL) {
		AG_TextCacheClear(lbl->tCache);
	}
	if (lbl->surfaceCont != -1) {
		AG_WidgetUnmapSurface(lbl, lbl->surfaceCont);
		lbl->surfaceCont = -1;
	}
	lbl->flags |= AG_LABEL_REGEN;
}

static void
Init(void *obj)
{
	AG_Label *lbl = obj;

	WIDGET(lbl)->flags |= AG_WIDGET_USE_TEXT|AG_WIDGET_TABLE_EMBEDDABLE;

	lbl->type = AG_LABEL_STATIC;
	lbl->flags = 0;
	lbl->fmt = NULL;
	lbl->text = NULL;
	lbl->surface = -1;
	lbl->surfaceCont = -1;
	lbl->lPad = 2;
	lbl->rPad = 2;
	lbl->tPad = 0;
	lbl->bPad = 1;
	lbl->wPre = -1;
	lbl->hPre = 1;
	lbl->justify = AG_TEXT_LEFT;
	lbl->valign = AG_TEXT_TOP;
	lbl->tCache = NULL;
	lbl->rClip = AG_RECT(0,0,0,0);		/* Initialized in SizeAlloc() */
	lbl->pollBuf = NULL;
	lbl->pollBufSize = 0;

	AG_SetEvent(lbl, "font-changed", OnFontChange, NULL);
}

/* Size the widget to accomodate the given text (with the current font). */
void
AG_LabelSizeHint(AG_Label *lbl, Uint nlines, const char *text)
{
	AG_ObjectLock(lbl);
	AG_TextSize(text, &lbl->wPre, NULL);
	lbl->hPre = (nlines > 0) ? nlines : 1;
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

static __inline__ void
GetPosition(AG_Label *lbl, AG_Surface *su, int *x, int *y)
{
	*x = lbl->lPad +
	     AG_TextJustifyOffset(WIDTH(lbl) - (lbl->lPad+lbl->rPad), su->w);
	*y = lbl->tPad +
	     AG_TextValignOffset(HEIGHT(lbl) - (lbl->tPad+lbl->bPad), su->h);
}

/* Render a polled label. */
static void
DrawPolled(AG_Label *lbl)
{
	size_t rv;
	char *pollBufNew;
	int x, y;
	int su;

	if (lbl->fmt == NULL || lbl->fmt->s[0] == '\0') {
		return;
	}
	for (;;) {
		rv = AG_ProcessFmtString(lbl->fmt, lbl->pollBuf, lbl->pollBufSize);
		if (rv >= lbl->pollBufSize) {
			if ((pollBufNew = TryRealloc(lbl->pollBuf,
			    (rv+AG_FMTSTRING_BUFFER_GROW))) == NULL) {
				return;
			}
			lbl->pollBuf = pollBufNew;
			lbl->pollBufSize = (rv+AG_FMTSTRING_BUFFER_GROW);
		} else {
			break;
		}
	}

	if ((su = AG_TextCacheGet(lbl->tCache,lbl->pollBuf)) != -1) {
		GetPosition(lbl, WSURFACE(lbl,su), &x, &y);
		AG_WidgetBlitSurface(lbl, su, x, y);
	}
}

static void
Draw(void *obj)
{
	AG_Label *lbl = obj;
	int x, y, cw = 0;			/* make compiler happy */
	AG_Surface *suNew;

	if (lbl->flags & AG_LABEL_FRAME) {
		AG_DrawFrame(lbl,
		    AG_RECT(0, 0, WIDTH(lbl), HEIGHT(lbl)), -1,
		    WCOLOR(lbl,0));
	}
	if ((lbl->flags & AG_LABEL_PARTIAL) && lbl->surfaceCont != -1) {
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
	
	AG_TextJustify(lbl->justify);
	AG_TextValign(lbl->valign);

	switch (lbl->type) {
	case AG_LABEL_STATIC:
		if (lbl->surface == -1 && lbl->text != NULL) {
			if ((suNew = AG_TextRender(lbl->text)) != NULL) {
				lbl->surface = AG_WidgetMapSurface(lbl, suNew);
			}
		} else if (lbl->flags & AG_LABEL_REGEN) {
			if (lbl->text != NULL) {
				if ((suNew = AG_TextRender(lbl->text)) != NULL)
					AG_WidgetReplaceSurface(lbl, 0, suNew);
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
	}
	
	AG_PopClipRect(lbl);
	
	if ((lbl->flags & AG_LABEL_PARTIAL) && lbl->surfaceCont != -1) {
		GetPosition(lbl, WSURFACE(lbl,lbl->surfaceCont), &x, &y);
		AG_WidgetBlitSurface(lbl, lbl->surfaceCont,
		    WIDTH(lbl) - cw,
		    y);
	}
}

static void
Destroy(void *p)
{
	AG_Label *lbl = p;

	if (lbl->fmt != NULL) {
		AG_FreeFmtString(lbl->fmt);
	}
	Free(lbl->text);
	Free(lbl->pollBuf);

	if (lbl->tCache != NULL)
		AG_TextCacheDestroy(lbl->tCache);
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
