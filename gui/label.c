/*
 * Copyright (c) 2002-2020 Julien Nadeau Carriere <vedge@csoft.net>
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

/*
 * Text label widget. Displays single- or multi-line text. Implements
 * static as well as dynamically-updated labels.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/gui.h>
#include <agar/gui/window.h>
#include <agar/gui/label.h>
#include <agar/gui/primitive.h>
#include <agar/gui/text_cache.h>
#include <agar/gui/box.h>
#include <agar/gui/checkbox.h>
#include <agar/gui/radio.h>
#include <agar/gui/numerical.h>
#include <agar/gui/separator.h>

#include <string.h>
#include <stdarg.h>

static void DrawStatic(AG_Label *_Nonnull);
static void DrawPolled(AG_Label *_Nonnull);

/* Create a new polled (dynamically updated) label. */
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

	if (flags & AG_LABEL_HFILL) { WIDGET(lbl)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_LABEL_VFILL) { WIDGET(lbl)->flags |= AG_WIDGET_VFILL; }
	lbl->flags |= flags;

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

	AG_RedrawOnTick(lbl, (flags & AG_LABEL_SLOW) ? 2000 : 500);
	AG_ObjectAttach(parent, lbl);
	return (lbl);
}

/* Create a new polled label which requires acquiring a given mutex. */
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

	if (flags & AG_LABEL_HFILL) { WIDGET(lbl)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_LABEL_VFILL) { WIDGET(lbl)->flags |= AG_WIDGET_VFILL; }
	lbl->flags |= flags;

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

	AG_RedrawOnTick(lbl, (flags & AG_LABEL_SLOW) ? 2000 : 500);
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

	if (flags & AG_LABEL_HFILL) { WIDGET(lbl)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_LABEL_VFILL) { WIDGET(lbl)->flags |= AG_WIDGET_VFILL; }
	lbl->flags |= flags;

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

	if (flags & AG_LABEL_HFILL) { WIDGET(lbl)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_LABEL_VFILL) { WIDGET(lbl)->flags |= AG_WIDGET_VFILL; }
	lbl->flags |= flags;

	lbl->text = (text != NULL) ? Strdup(text) : NULL;

	AG_ObjectAttach(parent, lbl);
	return (lbl);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Label *lbl = obj;
	
	switch (lbl->type) {
	case AG_LABEL_STATIC:
		AG_TextSize(lbl->text, &r->w, &r->h);
		break;
	case AG_LABEL_POLLED:
		if (lbl->fmt->s && lbl->fmt->s[0] != '\0') {     /* Auto-size */
			int sCached;

			for (;;) {
				AG_Size rv;

				rv = AG_ProcessFmtString(lbl->fmt, lbl->pollBuf,
				    lbl->pollBufSize);
				if (rv >= lbl->pollBufSize) {
					char *pbNew;
					const AG_Size sizeNew = (rv +
					    AG_FMTSTRING_BUFFER_GROW);

					if ((pbNew = TryRealloc(lbl->pollBuf,
					    sizeNew)) == NULL) {
						break;
					}
					lbl->pollBuf = pbNew;
					lbl->pollBufSize = sizeNew;
				} else {
					break;
				}
			}
			if ((sCached = AG_TextCacheGet(lbl->tCache, lbl->pollBuf)) != -1) {
				const AG_Surface *S = WSURFACE(lbl,sCached);

				r->w = S->w;
				r->h = S->h;
			}
		} else {                                /* Explicit size hint */
			r->w =  lbl->wPre;
			r->h = (lbl->hPre * WFONT(lbl)->lineskip);
		}
		break;
	default:
		break;
	}
	r->w += WIDGET(lbl)->paddingLeft + WIDGET(lbl)->paddingRight;
	r->h += WIDGET(lbl)->paddingTop + WIDGET(lbl)->paddingBottom;
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_Label *lbl = obj;
	int wLbl, hLbl, sCached;
	
	if (a->w < 1 || a->h < 1)
		return (-1);

	switch (lbl->type) {
	case AG_LABEL_STATIC:
		if (lbl->text == NULL) {
			lbl->flags &= ~(AG_LABEL_PARTIAL);
			break;
		}
		AG_TextSize(lbl->text, &wLbl, &hLbl);
		if (wLbl > a->w || hLbl > a->h) {
			lbl->flags |=   AG_LABEL_PARTIAL;
		} else {
			lbl->flags &= ~(AG_LABEL_PARTIAL);
		}
		break;
	case AG_LABEL_POLLED:
		if (lbl->fmt == NULL || lbl->fmt->s[0] == '\0') {
			lbl->flags &= ~(AG_LABEL_PARTIAL);
			break;
		}
		for (;;) {
			AG_Size rv;

			rv = AG_ProcessFmtString(lbl->fmt, lbl->pollBuf,
			    lbl->pollBufSize);
			if (rv >= lbl->pollBufSize) {
				char *pbNew;
				const AG_Size sizeNew = (rv + AG_FMTSTRING_BUFFER_GROW);

				if ((pbNew = TryRealloc(lbl->pollBuf, sizeNew)) == NULL) {
					return (0);
				}
				lbl->pollBuf = pbNew;
				lbl->pollBufSize = sizeNew;
			} else {
				break;
			}
		}
		if ((sCached = AG_TextCacheGet(lbl->tCache, lbl->pollBuf)) != -1) {
			const AG_Surface *S = WSURFACE(lbl,sCached);

			if (S->w > a->w || S->h > a->h) {
				lbl->flags |=   AG_LABEL_PARTIAL;
			} else {
				lbl->flags &= ~(AG_LABEL_PARTIAL);
			}
		} else {
			lbl->flags &= ~(AG_LABEL_PARTIAL);
		}
		break;
	default:
		break;
	}
	return (0);
}

static void
StyleChanged(AG_Event *_Nonnull event)
{
	AG_Label *lbl = AG_LABEL_SELF();

	if (lbl->tCache) {
		AG_TextCacheClear(lbl->tCache);
	}
	if (lbl->surfaceCtd != -1) {
		AG_WidgetUnmapSurface(lbl, lbl->surfaceCtd);
		lbl->surfaceCtd = -1;
	}
	lbl->flags |= AG_LABEL_REGEN;
}

static void
Init(void *_Nonnull obj)
{
	AG_Label *lbl = obj;

	WIDGET(lbl)->flags |= AG_WIDGET_USE_TEXT;

	lbl->type = AG_LABEL_STATIC;
	lbl->flags = 0;
	lbl->text = NULL;
	lbl->surface = -1;
	lbl->surfaceCtd = -1;
	lbl->wPre = -1;
	lbl->hPre = 1;
	lbl->justify = AG_TEXT_LEFT;
	lbl->valign = AG_TEXT_TOP;
	lbl->tCache = NULL;
	lbl->fmt = NULL;
	lbl->pollBuf = NULL;
	lbl->pollBufSize = 0;

	AG_SetEvent(lbl, "font-changed",    StyleChanged, NULL);
	AG_SetEvent(lbl, "palette-changed", StyleChanged, NULL);
}

/* Size the widget to accomodate the given text (with the current font). */
void
AG_LabelSizeHint(AG_Label *lbl, Uint nLines, const char *text)
{
	AG_OBJECT_ISA(lbl, "AG_Widget:AG_Label:*");
	AG_TextSize(text, &lbl->wPre, NULL);
	lbl->hPre = (nLines > 0) ? nLines : 1;
}

/* Justify the text in the specified way. */
void
AG_LabelJustify(AG_Label *lbl, enum ag_text_justify justify)
{
	AG_OBJECT_ISA(lbl, "AG_Widget:AG_Label:*");
	lbl->justify = justify;
	AG_Redraw(lbl);
}

/* Vertically align the text in the specified way. */
void
AG_LabelValign(AG_Label *lbl, enum ag_text_valign valign)
{
	AG_OBJECT_ISA(lbl, "AG_Widget:AG_Label:*");
	lbl->valign = valign;
	AG_Redraw(lbl);
}

/* Change the text displayed by the label (format string). */
void
AG_LabelText(AG_Label *lbl, const char *fmt, ...)
{
	va_list ap;

	AG_OBJECT_ISA(lbl, "AG_Widget:AG_Label:*");
	AG_ObjectLock(lbl);

	Free(lbl->text);

	va_start(ap, fmt);
	Vasprintf(&lbl->text, fmt, ap);
	va_end(ap);

	lbl->flags |= AG_LABEL_REGEN;

	AG_Redraw(lbl);
	AG_ObjectUnlock(lbl);
}

/* Change the text displayed by the label (C string). */
void
AG_LabelTextS(AG_Label *lbl, const char *s)
{
	AG_OBJECT_ISA(lbl, "AG_Widget:AG_Label:*");
	AG_ObjectLock(lbl);

	Free(lbl->text);

	lbl->text = Strdup(s);
	lbl->flags |= AG_LABEL_REGEN;

	AG_Redraw(lbl);
	AG_ObjectUnlock(lbl);
}

/* Calculate offset for horizontal justify */
static __inline__ int
JustifyOffset(const AG_Label *_Nonnull lbl, int w, int wLine)
{
	switch (lbl->justify) {
	case AG_TEXT_LEFT:
	default:
		return (0);
	case AG_TEXT_CENTER:
		return ((w >> 1) - (wLine >> 1));
	case AG_TEXT_RIGHT:
		return (w - wLine);
	}
}

/* Calculate offset for vertical alignment */
static __inline__ int
ValignOffset(const AG_Label *_Nonnull lbl, int h, int hLine)
{
	switch (lbl->valign) {
	case AG_TEXT_TOP:	return (0);
	case AG_TEXT_MIDDLE:	return ((h >> 1) - (hLine >> 1));
	case AG_TEXT_BOTTOM:	return (h - hLine);
	}
	return (0);
}

static void
Draw(void *_Nonnull obj)
{
	static void (*pfDraw[])(AG_Label *) = {
		DrawStatic,        /* STATIC */
		DrawPolled         /* POLLED */
	};
	AG_Label *lbl = obj;
	AG_Surface *Sctd;
	const AG_Color *cBg = &WCOLOR(lbl, BG_COLOR);
	AG_Rect r;

	if (lbl->flags & AG_LABEL_FRAME)
		AG_DrawFrameSunk(lbl, &WIDGET(lbl)->r);

	if (cBg->a < AG_OPAQUE)
		AG_PushBlendingMode(lbl, AG_ALPHA_SRC, AG_ALPHA_ONE_MINUS_SRC);

	if (lbl->flags & AG_LABEL_PARTIAL) {                   /* Truncated */
		if (lbl->surfaceCtd == -1) {
#ifdef AG_UNICODE
			Sctd = AG_TextRender("\xE2\x80\xA6 "); /* U+2026 */
#else
			Sctd = AG_TextRender("... ");
#endif
			lbl->surfaceCtd = AG_WidgetMapSurface(lbl, Sctd);
		} else {
			Sctd = WSURFACE(lbl,lbl->surfaceCtd);
		}
		r.x = 0;
		r.y = 0;
		r.w = WIDTH(lbl) - Sctd->w;
		r.h = HEIGHT(lbl);
		AG_PushClipRect(lbl, &r);
	} else {
		Sctd = NULL;
	}
	
	AG_TextColor(&WCOLOR(lbl, TEXT_COLOR));
	AG_TextBGColor(cBg);

#ifdef AG_DEBUG
	if (lbl->type >= AG_LABEL_TYPE_LAST)
		AG_FatalError("type");
#endif
	pfDraw[lbl->type](lbl);
	
	if (Sctd != NULL) {
		AG_PopClipRect(lbl);

		if (WIDTH(lbl)  > Sctd->w &&
		    HEIGHT(lbl) > Sctd->h) {
			AG_WidgetBlitSurface(lbl, lbl->surfaceCtd,
			    WIDTH(lbl) - Sctd->w,
			    ValignOffset(lbl, HEIGHT(lbl), Sctd->h));
		}
	}

	if (cBg->a < AG_OPAQUE)
		AG_PopBlendingMode(lbl);
}

/* Render a static label. */
static void
DrawStatic(AG_Label *_Nonnull lbl)
{
	AG_Surface *S;

	if (lbl->surface == -1 && lbl->text != NULL) {
		if ((S = AG_TextRender(lbl->text)) != NULL) {
			lbl->surface = AG_WidgetMapSurface(lbl, S);
		}
	} else if (lbl->flags & AG_LABEL_REGEN) {
		if (lbl->text != NULL) {
			if ((S = AG_TextRender(lbl->text)) != NULL)
				AG_WidgetReplaceSurface(lbl, 0, S);
		} else {
			lbl->surface = -1;
		}
	}
	lbl->flags &= ~(AG_LABEL_REGEN);

	if (lbl->surface != -1) {
		const AG_Surface *S = WSURFACE(lbl,lbl->surface);
		const int x = JustifyOffset(lbl, WIDTH(lbl), S->w);
		const int y = ValignOffset(lbl, HEIGHT(lbl), S->h);

		AG_WidgetBlitSurface(lbl, lbl->surface,
		    WIDGET(lbl)->paddingLeft + x,
		    WIDGET(lbl)->paddingTop + y);
	}
}

/* Render a dynamically updated label. */
static void
DrawPolled(AG_Label *_Nonnull lbl)
{
	char *pollBufNew;
	AG_Size rv;
	int sCached;

	if (lbl->fmt == NULL || lbl->fmt->s[0] == '\0') {
		return;
	}
	for (;;) {
		rv = AG_ProcessFmtString(lbl->fmt, lbl->pollBuf, lbl->pollBufSize);
		if (rv >= lbl->pollBufSize) {
			const AG_Size sizeNew = (rv + AG_FMTSTRING_BUFFER_GROW);

			if ((pollBufNew = TryRealloc(lbl->pollBuf, sizeNew)) == NULL) {
				return;
			}
			lbl->pollBuf = pollBufNew;
			lbl->pollBufSize = sizeNew;
		} else {
			break;
		}
	}

	if ((sCached = AG_TextCacheGet(lbl->tCache,lbl->pollBuf)) != -1) {
		const AG_Surface *S = WSURFACE(lbl,sCached);
		const int x = JustifyOffset(lbl, WIDTH(lbl), S->w);
		const int y = ValignOffset(lbl, HEIGHT(lbl), S->h);

		AG_WidgetBlitSurface(lbl, sCached, x,y);
	}
}

static void
Destroy(void *_Nonnull p)
{
	AG_Label *lbl = p;

	Free(lbl->text);

	if (lbl->fmt != NULL) {
		AG_FreeFmtString(lbl->fmt);
	}
	Free(lbl->pollBuf);

	if (lbl->tCache)
		AG_TextCacheDestroy(lbl->tCache);
}

static void *_Nullable
Edit(void *_Nonnull p)
{
	static const AG_FlagDescr flagDescr[] = {
	    { AG_LABEL_PARTIAL, _("Partially visible"), 0 },
	    { AG_LABEL_REGEN,   _("Regenerate"),        0 },
	    { AG_LABEL_FRAME,   _("Display a border"),  1 },
	    { 0,                NULL,                   0 }
	};
	AG_Label *lbl = p;
	AG_Box *box;

	box = AG_BoxNewVert(NULL, AG_BOX_EXPAND);
	
	AG_LabelNew(box, 0, _("Label Type: %s"),
	    (lbl->type == AG_LABEL_STATIC) ? _("Static") :
	                                     _("Polled"));

	AG_CheckboxSetFromFlags(box, 0, &lbl->flags, flagDescr);

	AG_SeparatorNewHoriz(box);

	AG_LabelNewS(box, 0, _("Justify:"));
	AG_RadioNewUint(box, 0, agTextJustifyNames, (Uint *)&lbl->justify);

	AG_SeparatorNewHoriz(box);
	AG_LabelNewS(box, 0, _("Vertical Alignment:"));
	AG_RadioNewUint(box, 0, agTextValignNames, (Uint *)&lbl->valign);

	return (box);
}

AG_WidgetClass agLabelClass = {
	{
		"Agar(Widget:Label)",
		sizeof(AG_Label),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		Edit
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

#endif /* AG_WIDGETS */
