/*
 * Copyright (c) 2001-2023 Julien Nadeau Carriere <vedge@csoft.net>
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
 * The Agar text rendering interface. It provides text rendering, sizing
 * and access to font metrics. It handles UTF-8 input (internally UCS-4).
 * The input may also contain ANSI SGR sequences with alternate color and
 * typography attributes.
 *
 * Text is rendered using an underlying font engine. The following base font
 * engine classes are provided:
 *
 *  - AG_Font   - Base class for all fonts.
 *  - AG_FontFt - FreeType 2.x (.ttf, .otf, .ttc, .woff2, .woff, .dfont, .fnt).
 *  - AG_FontBf - Agar Bitmap Font format (.agbf, .bmp, .png).
 */

#include <agar/config/have_freetype.h>
#include <agar/config/have_fontconfig.h>
#include <agar/config/ttfdir.h>

#include <agar/core/core.h>
#include <agar/core/config.h>
#include <agar/core/win32.h>

#include <agar/gui/surface.h>
#include <agar/gui/font.h>

#ifdef HAVE_FONTCONFIG
# include <fontconfig/fontconfig.h>
#endif

#include <agar/gui/text.h>
#include <agar/gui/window.h>
#ifdef AG_WIDGETS
# include <agar/gui/box.h>
# include <agar/gui/label.h>
# include <agar/gui/button.h>
# include <agar/gui/checkbox.h>
#endif

#include <string.h>
#include <stdarg.h>

/* #define DEBUG_FONTS */
/* #define DEBUG_ANSI */

#ifndef AG_DEFAULT_FT_FONT_FACE
#define AG_DEFAULT_FT_FONT_FACE "_agFontAlgue"
#endif
#ifndef AG_DEFAULT_FT_FONT_SIZE
#define AG_DEFAULT_FT_FONT_SIZE 14.0f
#endif
#ifndef AG_DEFAULT_BF_FONT_FACE
#define AG_DEFAULT_BF_FONT_FACE "agar-minimal.agbf"
#endif
#ifndef AG_DEFAULT_BF_FONT_SIZE
#define AG_DEFAULT_BF_FONT_SIZE 12.0f
#endif

int agTextFontHeight = 0;             /* Default font height (px) */
int agTextFontAscent = 0;             /* Default font ascent (px) */
int agTextFontLineSkip = 0;           /* Default font line skip (px) */
int agFontconfigInited = 0;           /* Initialized Fontconfig library */

_Nonnull_Mutex AG_Mutex agTextLock;   /* Lock on FontCache and TextState */

AG_FontQ agFontCache;                 /* Open fonts */

AG_TextState agTextStateStack[AG_TEXT_STATES_MAX];  /* Text state stack */
int          agTextStateCur = 0;                    /* Height of stack */

/* ANSI color scheme (may be overridden by AG_TextState) */
AG_Color agTextColorANSI[] = {
#if AG_MODEL == AG_LARGE
	{ 0x0000, 0x0000, 0x0000, 0xffff },  /* Black      (AGSI_BLK)  */
	{ 0xffff, 0x6b6b, 0x6b6b, 0xffff },  /* Red        (AGSI_RED)  */
	{ 0x0000, 0xcdcd, 0x0000, 0xffff },  /* Green      (AGSI_GRN)  */
	{ 0xcdcd, 0xcdcd, 0x0000, 0xffff },  /* Yellow     (AGSI_YEL)  */
	{ 0x8a8a, 0x9696, 0xffff, 0xffff },  /* Blue       (AGSI_BLU)  */
	{ 0xcdcd, 0x0000, 0xcdcd, 0xffff },  /* Magenta    (AGSI_MAG)  */
	{ 0x0000, 0xcdcd, 0xcdcd, 0xffff },  /* Cyan       (AGSI_CYAN) */
	{ 0xfafa, 0xebeb, 0xebeb, 0xffff },  /* White      (AGSI_WHT)  */

	{ 0x7f7f, 0x7f7f, 0x7f7f, 0xffff },  /* Gray       (AGSI_BR_BLK | AGSI_GRAY) */
	{ 0xffff, 0x1e1e, 0x1e1e, 0xffff },  /* Br.Red     (AGSI_BR_RED)  */
	{ 0x0000, 0xffff, 0x0000, 0xffff },  /* Br.Green   (AGSI_BR_GRN)  */
	{ 0xffff, 0xffff, 0x0000, 0xffff },  /* Br.Yellow  (AGSI_BR_YEL)  */
	{ 0x5c5c, 0x5c5c, 0xffff, 0xffff },  /* Br.Blue    (AGSI_BR_BLU)  */
	{ 0xffff, 0x0000, 0xffff, 0xffff },  /* Br.Magenta (AGSI_BR_MAG)  */
	{ 0x0000, 0xffff, 0xffff, 0xffff },  /* Br.Cyan    (AGSI_BR_CYAN) */
	{ 0xffff, 0xffff, 0xffff, 0xffff },  /* Br.White   (AGSI_BR_WHT)  */
#else
	{ 0,   0,     0, 255 },              /* Black      (AGSI_BLK)  */
	{ 255, 107, 107, 255 },              /* Red        (AGSI_RED)  */
	{ 0,   205,   0, 255 },              /* Green      (AGSI_GRN)  */
	{ 205, 205,   0, 255 },              /* Yellow     (AGSI_YEL)  */
	{ 138, 150, 255, 255 },              /* Blue       (AGSI_BLU)  */
	{ 205, 0,   205, 255 },              /* Magenta    (AGSI_MAG)  */
	{ 0,   205, 205, 255 },              /* Cyan       (AGSI_CYAN) */
	{ 250, 235, 235, 255 },              /* White      (AGSI_WHT)  */

	{ 127, 127, 127, 255 },              /* Gray       (AGSI_BR_BLK | AGSI_GRAY) */
	{ 255, 30,   30, 255 },              /* Br.Red     (AGSI_BR_RED)  */
	{ 0,   255,   0, 255 },              /* Br.Green   (AGSI_BR_GRN)  */
	{ 255, 255,   0, 255 },              /* Br.Yellow  (AGSI_BR_YEL)  */
	{ 92,  92,  255, 255 },              /* Br.Blue    (AGSI_BR_BLU)  */
	{ 255, 0,   255, 255 },              /* Br.Magenta (AGSI_BR_MAG)  */
	{ 0,   255, 255, 255 },              /* Br.Cyan    (AGSI_BR_CYAN) */
	{ 255, 255, 255, 255 },              /* Br.White   (AGSI_BR_WHT)  */
#endif
};	

/* Core fonts provided with Agar */
const char *agCoreFonts[] = {
	"algue",             /*  #1 Algue (default font; built-in) */
	"unialgue",          /*  #2 Unialgue (default font w/ extended Unicode) */
	"agar-minimal",      /*  #3 Agar Minimal (our bitmap font) */
	"agar-ideograms",    /*  #4 Agar Ideograms (our graphical icons) */
	"monoalgue",         /*  #5 Monoalgue (a monospace sans-serif) */
	"charter",           /*  #6 Bitstream Charter (a transitional serif font) */
	"Noto Serif CJK SC", /*  #7 Noto Serif CJK SC (a pan-CJK serif font) */
	"Noto Sans CJK SC",  /*  #8 Noto Sans CJK SC (a pan-CJK sans font) */
	"league-spartan",    /*  #9 League Spartan (a geometric sans-serif) */
	"league-gothic",     /* #10 League Gothic (a condensable Gothic font) */
	"fraktur",           /* #11 Unifraktur Maguntia (a Fraktur font) */
	"algue",             /* #12 (unused) */
	"algue",             /* #13 (unused) */
	"algue",             /* #14 (unused) */
	"algue",             /* #15 (unused) */
	"algue",             /* #16 (unused) */
	"algue",             /* #17 (unused) */
	NULL
};

/* Canned dialog titles (window captions). */
const char *agTextMsgTitles[] = {            /* For enum ag_text_msg_title */
	N_("Error"),
	N_("Warning"),
	N_("Information"),
	NULL
};

/* Names of justification modes. */
const char *agTextJustifyNames[] = {         /* For enum ag_text_justify */
	N_("Left"),
	N_("Center"),
	N_("Right"),
	NULL
};

/* Names of vertical alignment modes. */
const char *agTextValignNames[] = {          /* For enum ag_text_valign */
	N_("Top"),
	N_("Middle"),
	N_("Bottom"),
	NULL
};

AG_Font *_Nullable agDefaultFont = NULL;     /* Default font */
static int agTextInitedSubsystem = 0;        /* AG_Text is initialized */

/*
 * Save the current text rendering state to the AG_TextState stack
 * and increment the stack height by one unit. The text rendering state
 * includes:
 *  - Font Face (reference to an open font of given family, size & style).
 *  - Colors (Background / Foreground colors and ANSI color scheme).
 *  - Justification mode.
 *  - Vertical alignment mode.
 *  - Width of tabs (\t) in pixels.
 *
 * Must be called from Widget draw(), size_alloc(), size_request() or
 * event-handling context. Widget must have the USE_TEXT flag set.
 */
void
AG_PushTextState(void)
{
	const AG_TextState *tsPrev = AG_TEXT_STATE_CUR();
	AG_Font *fontPrev;
	AG_TextState *ts;

	if ((agTextStateCur + 1) >= AG_TEXT_STATES_MAX)
		AG_FatalError("PushTextState Overflow");

	ts = &agTextStateStack[++agTextStateCur];

	memcpy(ts, tsPrev, sizeof(AG_TextState));

	if (tsPrev->colorANSI != NULL) {
		const AG_Size size = AG_ANSI_COLOR_LAST * sizeof(AG_Color);

		ts->colorANSI = Malloc(size);
		memcpy(ts->colorANSI, tsPrev->colorANSI, size);
	}

	fontPrev = tsPrev->font;
	ts->font = AG_FetchFont(fontPrev->name, fontPrev->spec.size,
	    fontPrev->flags);
}

/* Create a copy of the current text state in dst. */
void
AG_CopyTextState(AG_TextState *dst)
{
	const AG_TextState *ts = AG_TEXT_STATE_CUR();

	memcpy(dst, ts, sizeof(AG_TextState));
}

/*
 * Override an entry in the 16-color (4-bit) ANSI color palette.
 * Duplicate the default palette into the current state if necessary.
 */
void
AG_TextColorANSI(enum ag_ansi_color colorANSI, const AG_Color *c)
{
	AG_TextState *ts = AG_TEXT_STATE_CUR();
	AG_Color *tgt;

	if (colorANSI >= AG_ANSI_COLOR_LAST) {
		AG_FatalError("colorANSI");
	}
	if (ts->colorANSI == NULL) {
		const AG_Size size = AG_ANSI_COLOR_LAST * sizeof(AG_Color);

		ts->colorANSI = Malloc(size);
		memcpy(ts->colorANSI, agTextColorANSI, size);
	}
	tgt = &ts->colorANSI[colorANSI];
	memcpy(tgt, c, sizeof(AG_Color));
}

/*
 * Select the font face to use in rendering text.
 *
 * Must be called from Widget draw(), size_alloc(), size_request() or
 * event-handling context. Widget must have the USE_TEXT flag set.
 */
AG_Font *
AG_TextFontLookup(const char *face, float size, Uint flags)
{
	AG_Font *newFont;
	AG_TextState *ts;

	if ((newFont = AG_FetchFont(face, size, flags)) == NULL) {
		return (NULL);
	}
	ts = AG_TEXT_STATE_CUR();
	ts->font = newFont;
	return (newFont);
}

/*
 * Set font size in points.
 *
 * Must be called from Widget draw(), size_alloc(), size_request() or
 * event-handling context. Widget must have the USE_TEXT flag set.
 */
AG_Font *
AG_TextFontPts(float size)
{
	AG_TextState *ts = AG_TEXT_STATE_CUR();
	const AG_Font *fontCur = ts->font;
	AG_Font *font;

	if (!(font = AG_FetchFont(fontCur->name, size, fontCur->flags))) {
		return (NULL);
	}
	ts->font = font;
	return (font);
}

/*
 * Set font size as % of the current font size.
 *
 * Must be called from Widget draw(), size_alloc(), size_request() or
 * event-handling context. Widget must have the USE_TEXT flag set.
 */
AG_Font *
AG_TextFontPct(int pct)
{
	AG_TextState *ts = AG_TEXT_STATE_CUR();
	const AG_Font *font = ts->font;

	return AG_FetchFont(font->name,
	    (font->spec.size * pct / 100.0f),
	    font->flags);
}
AG_Font *
AG_TextFontPctFlags(int pct, Uint flags)
{
	AG_TextState *ts = AG_TEXT_STATE_CUR();
	const AG_Font *font = ts->font;

	return AG_FetchFont(font->name,
	    (font->spec.size * pct / 100.0f),
	    flags);
}

/*
 * Restore the previous font-engine rendering state.
 *
 * Must be called from Widget draw(), size_alloc(), size_request() or
 * event-handling context. Widget must have the USE_TEXT flag set.
 */
void
AG_PopTextState(void)
{
	if (agTextStateCur == 0) {
#ifdef AG_DEBUG
		AG_Verbose("AG_PopTextState() without Push\n");
#endif
		return;
	}
	--agTextStateCur;
}

/* Clear the glyph cache. */
void
AG_TextClearGlyphCache(AG_Driver *drv)
{
	int i;
	AG_Glyph *G, *Gnext;

	for (i = 0; i < AG_GLYPH_NBUCKETS; i++) {
		for (G = SLIST_FIRST(&drv->glyphCache[i].glyphs);
		     G != SLIST_END(&drv->glyphCache[i].glyphs);
		     G = Gnext) {
			Gnext = SLIST_NEXT(G, glyphs);
			AG_SurfaceFree(G->su);
			free(G);
		}
		SLIST_INIT(&drv->glyphCache[i].glyphs);
	}
}

static __inline__ void
InitMetrics(AG_TextMetrics *_Nonnull Tm)
{
	Tm->w = 0;
	Tm->h = 0;
	Tm->wLines = NULL;
	Tm->nLines = 0;
}

static __inline__ void
FreeMetrics(AG_TextMetrics *_Nonnull Tm)
{
	Free(Tm->wLines);
}

/*
 * Return the expected size in pixels of a rendered C string.
 * If Unicode is supported, the string may contain UTF-8.
 */
void
AG_TextSize(const char *text, int *w, int *h)
{
#ifdef AG_UNICODE
	AG_Char *s;
#endif
	if (text == NULL || text[0] == '\0') {
		if (w != NULL) { *w = 0; }
		if (h != NULL) { *h = 0; }
		return;
	}
#ifdef AG_UNICODE
	if ((s = AG_ImportUnicode("UTF-8", text, NULL, NULL)) != NULL) {
		AG_TextSizeInternal(s, w,h);
		free(s);
	}
#else
	AG_TextSizeInternal((const Uint8 *)text, w,h);
#endif
}

/*
 * Return the rendered size in pixels of a string of internal AG_Char (which
 * may be internal UCS-4 or ASCII).
 */
void
AG_TextSizeInternal(const AG_Char *s, int *w, int *h)
{
	AG_TextMetrics Tm;

	InitMetrics(&Tm);

	if (s != NULL && (char)(s[0]) != '\0') {
		const AG_TextState *ts = AG_TEXT_STATE_CUR();
		const AG_Font *font = ts->font;

		AG_OBJECT_ISA(font, "AG_Font:*");
		AGFONT_OPS(font)->size(font, s, &Tm, 0);
	}

	if (w != NULL) { *w = Tm.w; }
	if (h != NULL) { *h = Tm.h; }

	FreeMetrics(&Tm);
}

/*
 * Return the rendered size in pixels of a C string (which may contain UTF-8).
 * Return a line count and an array of line widths.
 */
void
AG_TextSizeMulti(const char *text, int *w, int *h, Uint **wLines, Uint *nLines)
{
#ifdef AG_UNICODE
	AG_Char *s;

	if ((s = AG_ImportUnicode("UTF-8", text, NULL, NULL)) != NULL) {
		AG_TextSizeMultiInternal(s, w, h, wLines, nLines);
		free(s);
		return;
	}
#else
	AG_TextSizeMultiInternal((const Uint8 *)text, w, h, wLines, nLines);
#endif
}

/*
 * Compute the rendered size of a string of internal AG_Char (which may
 * be UCS-4 or ASCII). Return a line count and an array of line widths.
 */
void
AG_TextSizeMultiInternal(const AG_Char *s, int *w, int *h, Uint **wLines,
    Uint *nLines)
{
	AG_TextMetrics Tm;

	InitMetrics(&Tm);

	if (s != NULL && (char)(s[0]) != '\0') {
		const AG_TextState *ts = AG_TEXT_STATE_CUR();
		AG_Font *font = ts->font;

		AG_OBJECT_ISA(font, "AG_Font:*");
		AGFONT_OPS(font)->size(font, s, &Tm, 1);
	}

	if (w != NULL) { *w = Tm.w; }
	if (h != NULL) { *h = Tm.h; }

	if (Tm.nLines == 1) {
		Tm.wLines = Realloc(Tm.wLines, sizeof(Uint));
		Tm.wLines[0] = Tm.w;
	}
	if (wLines != NULL) { *wLines = Tm.wLines; }
	if (nLines != NULL) { *nLines = Tm.nLines; }
}

/*
 * Canned notification and dialog windows.
 * TODO move this elsewhere
 */
#ifdef AG_WIDGETS

/* Display an informational message window. */
void
AG_TextMsg(enum ag_text_msg_title title, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_TextMsgS(title, s);
	free(s);
}
void
AG_TextMsgS(enum ag_text_msg_title title, const char *s)
{
	AG_Window *win;
	AG_Box *vb;

	win = AG_WindowNew(AG_WINDOW_NORESIZE | AG_WINDOW_NOCLOSE |
	                   AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE);
	if (win == NULL)
		return;

	win->wmType = AG_WINDOW_WM_NOTIFICATION;
	AG_WindowSetCaptionS(win, _(agTextMsgTitles[title]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_BoxNewVert(win, 0);
	AG_SetFontSize(vb, "120%");
	AG_LabelNewS(vb, 0, s);

	vb = AG_BoxNewVert(win, AG_BOX_HOMOGENOUS | AG_BOX_EXPAND);
	AG_WidgetFocus( AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win)) );
	AG_WindowShow(win);
}

/* Display the last error message from AG_GetError(). */
void
AG_TextMsgFromError(void)
{
	AG_TextMsgS(AG_MSG_ERROR, AG_GetError());
}

static Uint32
TextTmsgExpire(AG_Timer *_Nonnull to, AG_Event *_Nonnull event)
{
	AG_Window *win = AGWINDOW(event->argv[1].data.p);

	if (!AG_OBJECT_VALID(win) || !AG_OfClass(win, "AG_Widget:AG_Window:*")) {
		AG_Verbose("Ignoring a TextTmsg() expiration\n");
		return (0);
	}
	AG_ObjectDetach(win);
	return (0);
}

/* Display a message for a given period of time (format string). */
void
AG_TextTmsg(enum ag_text_msg_title title, Uint32 expire, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_TextTmsgS(title, expire, s);
	free(s);
}
void
AG_TextTmsgS(enum ag_text_msg_title title, Uint32 ticks, const char *s)
{
	AG_Window *win;
	AG_Box *vb;
	AG_Timer *to;

	win = AG_WindowNew(AG_WINDOW_NORESIZE | AG_WINDOW_NOCLOSE |
	                   AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE);
	if (win == NULL)
		return;

	win->wmType = AG_WINDOW_WM_NOTIFICATION;
	AG_WindowSetCaptionS(win, _(agTextMsgTitles[title]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_BoxNewVert(win, 0);
	AG_SetFontSize(vb, "120%");
	AG_LabelNewS(vb, 0, s);
	AG_WindowShow(win);

	to = AG_AddTimerAuto(NULL, ticks, TextTmsgExpire, "%p", win);
	if (to != NULL)
		Strlcpy(to->name, "textTmsg", sizeof(to->name));
}

/*
 * Display an informational message with a "Don't tell me again" option.
 * The user preference is preserved in a persistent table. Unlike warnings,
 * the dialog window is not modal (format string).
 */
void
AG_TextInfo(const char *key, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_TextInfoS(key, s);
	free(s);
}
void
AG_TextInfoS(const char *key, const char *s)
{
	char disableSw[64];
	AG_Variable *Vdisable;
	AG_Window *win;
	AG_Box *vb;
	AG_Checkbox *cb;
	
	if (key != NULL) {
		Strlcpy(disableSw, "info.", sizeof(disableSw));
		Strlcat(disableSw, key, sizeof(disableSw));
		AG_ObjectLock(agConfig);
		if (AG_Defined(agConfig, disableSw) &&
		    AG_GetInt(agConfig, disableSw) == 1)
			goto out;
	}
	win = AG_WindowNew(AG_WINDOW_NORESIZE | AG_WINDOW_NOCLOSE |
	                   AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE);
	if (win == NULL)
		return;

	win->wmType = AG_WINDOW_WM_DIALOG;

	AG_WindowSetCaptionS(win, _(agTextMsgTitles[AG_MSG_INFO]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_BoxNewVert(win, 0);
	AG_SetFontSize(vb, "120%");
	AG_LabelNewS(vb, 0, s);

	vb = AG_BoxNewVert(win, AG_BOX_HOMOGENOUS | AG_BOX_EXPAND);
	AG_WidgetFocus( AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win)) );
	if (key != NULL) {
		cb = AG_CheckboxNewS(win, AG_CHECKBOX_HFILL,
		    _("Don't tell me again"));
		Vdisable = AG_SetInt(agConfig, disableSw, 0);
		AG_BindInt(cb, "state", &Vdisable->data.i);
	}
	AG_WindowShow(win);
out:
	if (key != NULL)
		AG_ObjectUnlock(agConfig);
}

/*
 * Display a warning message with a "Don't tell me again" option.
 * The user preference is preserved in a persistent table.
 */
void
AG_TextWarning(const char *key, const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_TextWarningS(key, s);
	free(s);
}
void
AG_TextWarningS(const char *key, const char *s)
{
	char disableSw[64];
	AG_Variable *Vdisable;
	AG_Window *win;
	AG_Box *vb;
	AG_Checkbox *cb;

	if (key != NULL) {
		Strlcpy(disableSw, "warn.", sizeof(disableSw));
		Strlcat(disableSw, key, sizeof(disableSw));
		AG_ObjectLock(agConfig);
		if (AG_Defined(agConfig, disableSw) &&
		    AG_GetInt(agConfig, disableSw) == 1)
			goto out;
	}
	win = AG_WindowNew(AG_WINDOW_NORESIZE | AG_WINDOW_NOCLOSE |
	                   AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE);
	if (win == NULL)
		return;

	win->wmType = AG_WINDOW_WM_DIALOG;

	AG_WindowSetCaptionS(win, _(agTextMsgTitles[AG_MSG_WARNING]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_BoxNewVert(win, 0);
	AG_SetFontSize(vb, "120%");
	AG_LabelNewS(vb, 0, s);

	vb = AG_BoxNewVert(win, AG_BOX_HOMOGENOUS | AG_BOX_EXPAND);
	AG_WidgetFocus( AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win)) );

	if (key != NULL) {
		cb = AG_CheckboxNewS(win, AG_CHECKBOX_HFILL,
		    _("Don't tell me again"));
		Vdisable = AG_SetInt(agConfig, disableSw, 0);
		AG_BindInt(cb, "state", &Vdisable->data.i);
	}
	AG_WindowShow(win);

out:
	if (key != NULL)
		AG_ObjectUnlock(agConfig);
}

/* Display an error message. */
void
AG_TextError(const char *fmt, ...)
{
	va_list ap;
	char *s;

	va_start(ap, fmt);
	Vasprintf(&s, fmt, ap);
	va_end(ap);

	AG_TextErrorS(s);
	free(s);
}
void
AG_TextErrorS(const char *s)
{
	AG_Window *win;
	AG_Box *vb;

	win = AG_WindowNew(AG_WINDOW_NORESIZE | AG_WINDOW_NOCLOSE |
	                   AG_WINDOW_NOMINIMIZE | AG_WINDOW_NOMAXIMIZE);
	if (win == NULL)
		return;

	win->wmType = AG_WINDOW_WM_DIALOG;
	AG_WindowSetCaptionS(win, _(agTextMsgTitles[AG_MSG_ERROR]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_BoxNewVert(win, 0);
	AG_SetFontSize(vb, "120%");
	AG_LabelNewS(vb, 0, s);

	vb = AG_BoxNewVert(win, AG_BOX_HOMOGENOUS | AG_BOX_EXPAND);
	AG_WidgetFocus( AG_ButtonNewFn(vb, 0, _("Ok"), AGWINDETACH(win)) );
	AG_WindowShow(win);
}
#endif /* AG_WIDGETS */

/* Align a text surface inside a given space. */
void
AG_TextAlign(int *x, int *y, int wArea, int hArea, int wText, int hText,
    int lPad, int rPad, int tPad, int bPad, enum ag_text_justify justify,
    enum ag_text_valign valign)
{
	switch (justify) {
	case AG_TEXT_LEFT:
		*x = lPad;
		break;
	case AG_TEXT_CENTER:
		*x = ((wArea + lPad + rPad) >> 1) - (wText >> 1);
		break;
	case AG_TEXT_RIGHT:
		*x = wArea - rPad - wText;
		break;
	}
	switch (valign) {
	case AG_TEXT_TOP:
		*y = tPad;
		break;
	case AG_TEXT_MIDDLE:
		*y = ((hArea + tPad + bPad) >> 1) - (hText >> 1);
		break;
	case AG_TEXT_BOTTOM:
		*y = hArea - bPad - wText;
		break;
	}
}

/*
 * Render text (UTF-8 encoded) onto a newly-allocated surface.
 * Inherit font, FG and BG colors from current text state.
 */
AG_Surface *
AG_TextRenderF(const char *fmt, ...)
{
	AG_Surface *S;
	char *text;
	va_list args;

	va_start(args, fmt);
	Vasprintf(&text, fmt, args);
	va_end(args);
	S = AG_TextRender(text);
	free(text);
	return (S);
}

/*
 * Render text (UTF-8 encoded) left-to-right onto a newly-allocated surface.
 * Inherit font, FG and BG colors from current text state.
 */
AG_Surface *
AG_TextRender(const char *text)
{
	AG_TextState *ts = AG_TEXT_STATE_CUR();
	AG_Surface *S;
#ifdef AG_UNICODE
	AG_Char *us;

	if ((us = AG_ImportUnicode("UTF-8", text, NULL, NULL)) == NULL) {
		AG_FatalError(NULL);
	}
	S = AG_TextRenderInternal(us, ts->font, &ts->colorBG, &ts->color);
	free(us);
#else
	S = AG_TextRenderInternal((const Uint8 *)text, ts->font,
	                          &ts->colorBG, &ts->color);
#endif
	return (S);
}

/*
 * Render text (UTF-8 encoded) right-to-left onto a newly-allocated surface.
 * Inherit font, FG and BG colors from current text state.
 */
AG_Surface *
AG_TextRenderRTL(const char *text)
{
	AG_TextState *ts = AG_TEXT_STATE_CUR();
	AG_Surface *S;
#ifdef AG_UNICODE
	AG_Char *us, *usReversed, *c, *cReversed;
	AG_Size len;

	if ((us = AG_ImportUnicode("UTF-8", text, NULL, NULL)) == NULL) {
		AG_FatalError(NULL);
	}
	len = AG_LengthUCS4(us);
	usReversed = Malloc((len + 1) * sizeof(Uint32));
	for (c = &us[0], cReversed = &usReversed[len-1];
	    *c != '\0';
	     c++, --cReversed) {
		*cReversed = *c;
	}
	usReversed[len] = '\0';

	S = AG_TextRenderInternal(usReversed, ts->font,
	                          &ts->colorBG, &ts->color);

	free(usReversed);
	free(us);
#else
	S = AG_TextRenderInternal((const Uint8 *)text, ts->font,
	                          &ts->colorBG, &ts->color);
#endif /* !AG_UNICODE */

	return (S);
}

/*
 * Render text in native internal (AG_Char; UCS-4) format onto a newly
 * allocated surface.
 */
AG_Surface *
AG_TextRenderInternal(const AG_Char *text, AG_Font *font, const AG_Color *cBg,
    const AG_Color *cFg)
{
	AG_TextMetrics Tm;
	AG_Surface *S;

	InitMetrics(&Tm);

	AGFONT_OPS(font)->size(font, text, &Tm, 1);

	/* TODO AG_SURFACE_GL_TEXTURE? */
	S = AG_SurfaceNew(agSurfaceFmt, Tm.w, Tm.h, 0);
	AG_FillRect(S, NULL, cBg);

	if (cBg->a == AG_OPAQUE)
		S->format.Amask = 0;                  /* No blending needed */
#if 0
	else if (cBg->a == AG_TRANSPARENT)
		AG_SurfaceSetColorKey(S,           /* Colorkey transparency */
		    AG_SURFACE_COLORKEY,                       /* (useful?) */
		    AG_MapPixel(&S->format, cBg));
#endif
	if (Tm.w > 0 && Tm.h > 0)
		AGFONT_OPS(font)->render(text, S, &Tm, font, cBg,cFg);

	FreeMetrics(&Tm);
	return (S);
}

/*
 * Interpret a possible ANSI escape sequence within a native string and write
 * a normalized description into *ansi. Return 0 on success, -1 on failure.
 */
int
AG_TextParseANSI(const AG_TextState *ts, AG_TextANSI *_Nonnull ansi,
    const AG_Char *_Nonnull s)
{
	const AG_Char *c = &s[0];
	int len;

	switch (*c) {
	case '[':						/* CSI */
		break;
	case 'N':						/* SS2 */
	case 'O':						/* SS3 */
	case 'P':						/* DCS */
		ansi->ctrl = AG_ANSI_SS2 + (*c - (AG_Char)'N');
		ansi->len = 1;
		return (0);
	case '\\':						/* ST */
	case ']':						/* OSC */
	case '^':						/* PM */
	case '_':						/* APC */
		ansi->ctrl = AG_ANSI_ST + (*c - (AG_Char)'\\');
		ansi->len = 1;
		return (0);
	case 'X':						/* SOS */
		ansi->ctrl = AG_ANSI_SOS;
		ansi->len = 1;
		return (0);
	case 'c':						/* RIS */
		ansi->ctrl = AG_ANSI_RIS;
		ansi->len = 1;
		return (0);
	default:
		goto fail;
	}

	for (c = &s[1]; (len = (int)(c-s)) <= AG_TEXT_ANSI_SEQ_MAX; c++) {
		char buf[AG_TEXT_ANSI_PARAM_MAX], *pBuf;
		const AG_Char *pSrc;
		char *tok;
		int sgr, i;

		switch (*c) {
		case 'm':					/* SGR */
			ansi->ctrl = AG_ANSI_CSI_SGR;
			if (len >= sizeof(buf)) {
				goto fail;
			} else if (len == 0) {
				break;
			}
			for (i=0, pSrc=&s[1], pBuf=buf;
			     i < len;
			     i++) {
				*pBuf = (char)(*pSrc);    /* AG_Char -> ASCII */
				pBuf++;
				pSrc++;
			}
			pBuf[-1] = '\0';
#ifdef DEBUG_ANSI
			Debug(NULL, "SGR (%d bytes): \"%s\"\n", len, buf);
#endif
			pBuf = buf;
			if ((tok = Strsep(&pBuf, ":;")) == NULL) {
				break;
			}
			if ((sgr = atoi(tok)) >= AG_SGR_LAST || sgr < 0) {
				goto fail;
			}
			ansi->sgr = (enum ag_text_sgr_parameter) sgr;
#ifdef DEBUG_ANSI
			Debug(NULL, "SGR parameter %d\n", ansi->sgr);
#endif
			if (sgr == 0) {
				break;
			}
			/*
			 * 3/4-bit color.
			 */
			if (sgr >= 30 && sgr <= 37) {          /* Normal FG */
				ansi->sgr = AG_SGR_FG;
				memcpy(&ansi->color, (ts->colorANSI) ?
				                     &ts->colorANSI[sgr-30] :
				                   &agTextColorANSI[sgr-30],
				       sizeof(AG_Color));
				break;
			} else if (sgr >= 40 && sgr <= 47) {    /* Normal BG */
				ansi->sgr = AG_SGR_BG;
				memcpy(&ansi->color, (ts->colorANSI) ?
				                     &ts->colorANSI[sgr-40] :
				                   &agTextColorANSI[sgr-40],
				       sizeof(AG_Color));
				break;
			} else if (sgr >= 90 && sgr <= 97) {    /* Bright FG */
				ansi->sgr = AG_SGR_FG;
				memcpy(&ansi->color, (ts->colorANSI) ?
				                     &ts->colorANSI[8+sgr-90] :
				                   &agTextColorANSI[8+sgr-90],
				       sizeof(AG_Color));
				break;
			} else if (sgr >= 100 && sgr <= 107) {  /* Bright BG */
				ansi->sgr = AG_SGR_BG;
				memcpy(&ansi->color, (ts->colorANSI) ?
				                     &ts->colorANSI[8+sgr-100] :
				                   &agTextColorANSI[8+sgr-100],
				       sizeof(AG_Color));
				break;
			}
			switch (ansi->sgr) {
			case AG_SGR_FG:
			case AG_SGR_BG:
				if (!(tok = Strsep(&pBuf, ":;")) ||
				    tok[0] == '\0' || tok[1] != '\0') {
					goto fail_param;
				}
				switch (tok[0]) {
				case '2':                      /* 24-bit RGB */
					ansi->color.r = (tok = Strsep(&pBuf, ":;")) ? AG_8toH(atoi(tok)) : 0;
					ansi->color.g = (tok = Strsep(&pBuf, ":;")) ? AG_8toH(atoi(tok)) : 0;
					ansi->color.b = (tok = Strsep(&pBuf, ":;")) ? AG_8toH(atoi(tok)) : 0;
					ansi->color.a = AG_OPAQUE;
					break;
				case '5':                   /* 8-bit indexed */
					if (!(tok = Strsep(&pBuf, ":;")) || tok[0] == '\0') {
						goto fail_param;
					}
					if ((i = atoi(tok)) >= 0 && i <= 15) {
						memcpy(&ansi->color,
						    (ts->colorANSI) ?
						    &ts->colorANSI[i] :
						    &agTextColorANSI[i],
						    sizeof(AG_Color));
					}
					break;
				}
				break;
			case AG_SGR_NO_FG:
				if ((tok = Strsep(&pBuf, ":;")) &&
				    tok[0] == '4' && tok[1] == '9' &&
				    tok[2] == '\0') {
					ansi->sgr = AG_SGR_NO_FG_NO_BG;
				}
				break;
			case AG_SGR_NO_BG:
				if ((tok = Strsep(&pBuf, ":;")) &&
				    tok[0] == '3' && tok[1] == '9' &&
				    tok[2] == '\0') {
					ansi->sgr = AG_SGR_NO_FG_NO_BG;
				}
				break;
			default:
				break;
			}
			break;
		case 'A':					/* CUU */
		case 'B':					/* CUD */
		case 'C':					/* CUF */
		case 'D':					/* CUB */
		case 'E':					/* CNL */
		case 'F':					/* CPL */
		case 'G':					/* CHA */
			ansi->ctrl = AG_ANSI_CSI_CUU + (*c - (AG_Char)'A');
			break;
		case 'H':					/* CUP */
		case 'f':
			ansi->ctrl = AG_ANSI_CSI_CUP;
			break;
		case 'J':					/* ED */
		case 'K':					/* EL */
			ansi->ctrl = AG_ANSI_CSI_ED + (*c - (AG_Char)'J');
			break;
		case 'S':					/* SU */
		case 'T':					/* SD */
			ansi->ctrl = AG_ANSI_CSI_SU + (*c - (AG_Char)'S');
			break;
		case 'h':					/* Private */
		case 'i':
		case 'l':
			ansi->ctrl = AG_ANSI_PRIVATE;
			break;
		case 'n':
			ansi->ctrl = AG_ANSI_CSI_DSR;
			break;
		case 's':
			ansi->ctrl = AG_ANSI_CSI_SCP;
			break;
		case 'u':
			ansi->ctrl = AG_ANSI_CSI_RCP;
			break;
		case '\0':
			goto fail_param;
		default:
			if ((*c >= 0x70 && *c < 0x7e) || /* Private final byte */
			    strchr("<=>?", *c)) {        /* Private parameter */
				ansi->ctrl = AG_ANSI_PRIVATE;
				break;
			}
			if ((*c >= 0x30 && *c <= 0x3f) || /* Parameter byte */
			    (*c >= 0x20 && *c <= 0x2f)) { /* Intermediate byte */
				continue;
			}
			goto fail_param;
		}
		break;
	}
	ansi->len = len+1;
	return (0);
fail:
	AG_SetError("Bad ANSI code (before `%c')", *c);
	return (-1);
fail_param:
	AG_SetError("Bad ANSI parameter (before `%c')", *c);
	return (-1);
}

/*
 * Convert an internal UCS-4 string to a fixed-size buffer using the specified
 * encoding. Strip ANSI sequences. At most dstSize-1 bytes will be copied.
 * The string is always NUL-terminated.
 */
int
AG_TextExportUnicode_StripANSI(const char *encoding, char *dst, const AG_Char *ucs,
    AG_Size dstSize)
{
	AG_TextANSI ansi;
	AG_Size len;

	if (strcmp(encoding, "UTF-8") == 0) {
		for (len = 0; *ucs != '\0' && len < dstSize; ucs++) {
			AG_Char uch = *ucs;
			int chlen, ch1, i;
			
			if (ucs[0] == 0x1b &&
			    ucs[1] >= 0x40 &&
			    ucs[1] <= 0x5f &&
			    ucs[2] != '\0') {
				if (AG_TextParseANSI(AG_TEXT_STATE_CUR(),
				                     &ansi, &ucs[1]) == 0) {
					ucs += ansi.len;
					continue;
				}
			}
			if (uch < 0x80) {
				chlen = 1;
				ch1 = 0;
			} else if (uch < 0x800) {	
				chlen = 2;
				ch1 = 0xc0;
			} else if (uch < 0x10000) {
				chlen = 3;
				ch1 = 0xe0;
			} else if (uch < 0x200000) {
				chlen = 4;
				ch1 = 0xf0;
			} else if (uch < 0x4000000) {
				chlen = 5;
				ch1 = 0xf8;
			} else if (uch <= 0x7fffffff) {
				chlen = 6;
				ch1 = 0xfc;
			} else {
				AG_SetErrorS("Bad UTF-8 sequence");
				return (-1);
			}
			if (len+chlen+1 > dstSize) {
				AG_SetErrorS("Out of space");
				return (-1);
			}
			for (i = chlen - 1; i > 0; i--) {
				dst[i] = (uch & 0x3f) | 0x80;
				uch >>= 6;
			}
			dst[0] = uch | ch1;
			dst += chlen;
			len += chlen;
		}
		*dst = '\0';
		return (0);
	} else if (strcmp(encoding, "US-ASCII") == 0) {
		for (len = 0; *ucs != '\0' && len < dstSize; ucs++) {
			if (ucs[0] == 0x1b &&
			    ucs[1] >= 0x40 &&
			    ucs[1] <= 0x5f &&
			    ucs[2] != '\0') {
				if (AG_TextParseANSI(AG_TEXT_STATE_CUR(),
				                     &ansi, &ucs[1]) == 0) {
					ucs += ansi.len;
					continue;
				}
			}
			if ((*ucs) & ~0x7f) {
				AG_SetErrorS("Non-ASCII character");
				return (-1);
			}
			*dst = (char)*ucs;
			dst++;
			len++;
		}
		*dst = '\0';
		return (0);
	} else {
# ifdef HAVE_ICONV
		return ExportUnicodeICONV(encoding, dst, ucs, dstSize);
# else
		AG_SetError("No such encoding: \"%s\"", encoding);
		return (-1);
# endif
	}
}

static AG_Glyph *_Nonnull
TextRenderGlyph_Miss(AG_Driver *_Nonnull drv, AG_Font *_Nonnull font,
    const AG_Color *_Nonnull cBg, const AG_Color *_Nonnull cFg, AG_Char ch)
{
	AG_Glyph *G;
	AG_Char s[2];

	G = Malloc(sizeof(AG_Glyph));
	G->font = (AG_Font *)font;
	G->colorBG = *cBg;
	G->color = *cFg;
	G->ch = ch;
	s[0] = ch;
	s[1] = '\0';
	G->su = AG_TextRenderInternal(s, font, cBg,cFg);    /* Render glyph */
	AGFONT_OPS(font)->get_glyph_metrics(font, G);    /* Get the advance */
	AGDRIVER_CLASS(drv)->updateGlyph(drv, G);   /* Prepare GPU transfer */
	return (G);
}

/*
 * Render character ch using the given font and BG/FG colors. Cache the
 * result in the per-driver glyph cache. The glyph cache is stored in
 * AG_Driver(3) instances because glyph renderings may be associated with
 * driver-specific hardware textures.
 *
 * Must be called from GUI rendering context.
 */
AG_Glyph *
AG_TextRenderGlyph(AG_Driver *drv, AG_Font *font,
    const AG_Color *cBg, const AG_Color *cFg, AG_Char ch)
{
	AG_Glyph *G;
	const Uint h = (Uint)(ch % AG_GLYPH_NBUCKETS);

	SLIST_FOREACH(G, &drv->glyphCache[h].glyphs, glyphs) {
		if (ch == G->ch &&
		    font == G->font &&
		    AG_ColorCompare(cBg, &G->colorBG) == 0 &&
		    AG_ColorCompare(cFg, &G->color) == 0)
			break;
	}
	if (G == NULL) {
		G = TextRenderGlyph_Miss(drv, font, cBg, cFg, ch);
		SLIST_INSERT_HEAD(&drv->glyphCache[h].glyphs, G, glyphs);
	}
	return (G);
}

/* Set active text color. */
void
AG_TextColor(const AG_Color *c)
{
	memcpy(&AG_TEXT_STATE_CUR()->color, c, sizeof(AG_Color));
}
void
AG_TextColorRGB(Uint8 r, Uint8 g, Uint8 b)
{
	AG_ColorRGB_8(&AG_TEXT_STATE_CUR()->color, r,g,b);
}
void
AG_TextColorRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	AG_ColorRGBA_8(&AG_TEXT_STATE_CUR()->color, r,g,b,a);
}

/* Set text color from 0xRRGGBBAA format. */
void
AG_TextColorHex(Uint32 c)
{
	AG_ColorHex32(&AG_TEXT_STATE_CUR()->color, c);
}

/* Set active text background color. */
void
AG_TextBGColor(const AG_Color *c)
{
	memcpy(&AG_TEXT_STATE_CUR()->colorBG, c, sizeof(AG_Color));
}
void
AG_TextBGColorRGB(Uint8 r, Uint8 g, Uint8 b)
{
	AG_ColorRGB_8(&AG_TEXT_STATE_CUR()->colorBG, r,g,b);
}
void
AG_TextBGColorRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
	AG_ColorRGBA_8(&AG_TEXT_STATE_CUR()->colorBG, r,g,b,a);
}

/* Set text BG color from 0xRRGGBBAA format. */
void
AG_TextBGColorHex(Uint32 c)
{
	AG_ColorHex(&AG_TEXT_STATE_CUR()->colorBG, c);
}

/* Select a specific font face to use in rendering text. */
void
AG_TextFont(AG_Font *font)
{
	AG_TEXT_STATE_CUR()->font = font;
}

/* Select the justification mode to use in rendering text. */
void
AG_TextJustify(enum ag_text_justify mode)
{
	AG_TEXT_STATE_CUR()->justify = mode;
}

/* Select the vertical alignment mode to use in rendering text. */
void
AG_TextValign(enum ag_text_valign mode)
{
	AG_TEXT_STATE_CUR()->valign = mode;
}

/* Select the tab width in pixels for rendering text. */
void
AG_TextTabWidth(int px)
{
	AG_TEXT_STATE_CUR()->tabWd = px;
}

static void
SetDefaultFontAll(AG_Widget *wid, AG_Font *defaultFontPrev,
    AG_Font *defaultFontNew)
{
	AG_Widget *chld;

	if (wid->font == defaultFontPrev) {
		wid->font = defaultFontNew;
	}
	AG_PostEvent(wid, "font-changed", NULL);
	AG_Redraw(wid);

	AGOBJECT_FOREACH_CHILD(chld, wid, ag_widget)
		SetDefaultFontAll(chld, defaultFontPrev, defaultFontNew);
}


/*
 * Set the default font to the specified font. Return a pointer to
 * the previous default font.
 *
 * Updates the default font settings in AG_Config(3).
 * Updates agDefaultFont and agDefaultFont{Height,Ascent,LineSkip}.
 */
AG_Font *
AG_SetDefaultFont(AG_Font *font)
{
	AG_Font *prevDefaultFont;
	AG_Driver *drv;
	AG_Window *win;
	int i;

	AG_MutexLock(&agTextLock);

	prevDefaultFont = agDefaultFont;

	if (font != NULL) {
		agDefaultFont = font;
	} else {
		agDefaultFont = font = AG_FetchFont(NULL, 0.0f, 0);
	}

	agTextFontHeight = font->height;
	agTextFontAscent = font->ascent;
	agTextFontLineSkip = font->lineskip;

	/* Update the rendering state. */
	for (i = 0; i <= agTextStateCur; i++) {
		AG_TextState *ts = &agTextStateStack[i];

		if (ts->font == prevDefaultFont)
			ts->font = font;
	}

	/* Update the AG_Config(3) font settings. */
	Strlcpy(agConfig->fontFace, OBJECT(font)->name, sizeof(agConfig->fontFace));
	agConfig->fontSize = font->spec.size;
	agConfig->fontFlags = font->flags;

	AG_MutexUnlock(&agTextLock);

	/* Update all font references in the style engine. */
	AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		AG_FOREACH_WINDOW(win, drv) {
			SetDefaultFontAll(WIDGET(win),
			    prevDefaultFont, font);
			AG_WindowUpdate(win);
		}
	}

	return (prevDefaultFont);
}

/*
 * Set the default font from a string "<Face>,<Size>,<Style>".
 *
 * Size is in points (fractional point sizes are allowed).
 * Style is a space-separated list of style / weight / width variant
 * attribute names (as returned by AG_FontGetStyleName(3)).
 *
 * Exceptionally, this call may be invoked before AG_InitGraphics()
 * (in order to simplify setting the default font from command-line).
 */
void
AG_TextParseFontSpec(const char *spec)
{
	char fontspecBuf[AG_TEXT_FONTSPEC_MAX];
	char *pFontspec, *s;

	if (spec == NULL) {
		fontspecBuf[0] = '\0';
	} else {
		Strlcpy(fontspecBuf, spec, sizeof(fontspecBuf));
	}
	pFontspec = &fontspecBuf[0];

	if ((s = AG_Strsep(&pFontspec, ":,/")) != NULL && s[0] != '\0') {
		Strlcpy(agConfig->fontFace, s, sizeof(agConfig->fontFace));
#ifdef DEBUG_FONTS
		Debug(NULL, "Default font face = %s\n", agConfig->fontFace);
#endif
	}
	if ((s = AG_Strsep(&pFontspec, ":,/")) != NULL && s[0] != '\0') {
		agConfig->fontSize = (float)strtod(s, NULL);
#ifdef DEBUG_FONTS
		Debug(NULL, "Default font size = %.2f pts\n", agConfig->fontSize);
#endif
	}
	if ((s = AG_Strsep(&pFontspec, ":,/")) != NULL && s[0] != '\0') {
		char styleBuf[64], *pStyleBuf, *styleTok;
		Uint flags = 0;

		Strlcpy(styleBuf, s, sizeof(styleBuf));
		pStyleBuf = styleBuf;
		while ((styleTok = Strsep(&pStyleBuf, " ")) != NULL) {
			const AG_FontStyleName *fsn;

			for (fsn = &agFontStyleNames[0]; fsn->name != NULL;
			     fsn++) {
				if (Strcasecmp(fsn->name, styleTok) == 0) {
					 flags |= fsn->flag;
					 break;
				}
			}

		}
#ifdef DEBUG_FONTS
		AG_FontGetStyleName(styleBuf, sizeof(styleBuf), flags);
		Debug(NULL, "Default font style = %s\n", styleBuf);
#endif
		agConfig->fontFlags = flags;
	}
}

/* Initialize the font engine and configure the default font. */
int
AG_InitTextSubsystem(void)
{
	AG_User *sysUser;
	AG_TextState *ts;

	if (agTextInitedSubsystem++ > 0)
		return (0);

	AG_MutexInitRecursive(&agTextLock);
	TAILQ_INIT(&agFontCache);

	AG_ObjectLock(agConfig);

	/*
	 * Configure PATH_FONTS (the search path for loading fonts).
	 */
	if (strcmp(TTFDIR, "NONE") != 0)
		AG_ConfigAddPathS(AG_CONFIG_PATH_FONTS, TTFDIR);

	sysUser = AG_GetRealUser();
#if defined(__APPLE__)
	if (sysUser != NULL &&
	    sysUser->home != NULL) {
		AG_ConfigAddPath(AG_CONFIG_PATH_FONTS, "%s/Library/Fonts",
		    sysUser->home);
	}
	AG_ConfigAddPathS(AG_CONFIG_PATH_FONTS, "/Library/Fonts");
	AG_ConfigAddPathS(AG_CONFIG_PATH_FONTS, "/System/Library/Fonts");
#elif defined(_WIN32)
	{
		char windir[AG_PATHNAME_MAX];

		if (sysUser != NULL && sysUser->home != NULL) {
			AG_ConfigAddPath(AG_CONFIG_PATH_FONTS, "%s\\Fonts",
			    sysUser->home);
		}
		if (GetWindowsDirectoryA(windir, sizeof(windir)) > 0) {
			AG_ConfigAddPath(AG_CONFIG_PATH_FONTS, "%s\\Fonts",
			    windir);
		}
		AG_ConfigAddPathS(AG_CONFIG_PATH_FONTS, ".");
		AG_ConfigAddPathS(AG_CONFIG_PATH_FONTS, "..\\share\\agar\\fonts");
	}
#else /* !WIN32 & !APPLE */
	if (sysUser != NULL && sysUser->home != NULL) {
		AG_ConfigAddPath(AG_CONFIG_PATH_FONTS, "%s%s.fonts",
		    sysUser->home, AG_PATHSEP);
	}
#endif
	if (sysUser != NULL)
		AG_UserFree(sysUser);

#ifdef HAVE_FONTCONFIG
	/* Set up fontconfig if available. */
	if (FcInit()) {
		agFontconfigInited = 1;
	} else {
		AG_Verbose("Failed to initialize fontconfig; ignoring\n");
	}
#endif

	/* Set default font face and size according to FreeType availability. */
#ifdef HAVE_FREETYPE
	if (agConfig->fontFace[0] == '\0') {
		Strlcpy(agConfig->fontFace, AG_DEFAULT_FT_FONT_FACE,
		    sizeof(agConfig->fontFace));
	}
	if (agConfig->fontSize == 0.0f)
		agConfig->fontSize = AG_DEFAULT_FT_FONT_SIZE;
#else
	if (agConfig->fontFace[0] == '\0') {
		Strlcpy(agConfig->fontFace, AG_DEFAULT_BF_FONT_FACE,
		    sizeof(agConfig->fontFace));
	}
	if (agConfig->fontSize == 0.0f)
		agConfig->fontSize = AG_DEFAULT_BF_FONT_SIZE;
#endif /* !HAVE_FREETYPE */

	AG_ObjectUnlock(agConfig);

	/* Set the default font; see AG_SetDefaultFont(). */
	agDefaultFont = AG_FetchFont(agConfig->fontFace,
	                             agConfig->fontSize,
	                             agConfig->fontFlags);
	if (agDefaultFont == NULL) {
		return (-1);
	}
	agTextFontHeight = agDefaultFont->height;
	agTextFontAscent = agDefaultFont->ascent;
	agTextFontLineSkip = agDefaultFont->lineskip;

	/* Initialize the rendering state. */
	agTextStateCur = 0;
	ts = &agTextStateStack[0];
	ts->font = agDefaultFont;
	AG_ColorWhite(&ts->color);
	AG_ColorNone(&ts->colorBG);
	ts->colorANSI = NULL;
	ts->justify = AG_TEXT_LEFT;
	ts->valign = AG_TEXT_TOP;
	ts->tabWd = agTextTabWidth;
	return (0);
}

void
AG_DestroyTextSubsystem(void)
{
	AG_Font *font, *fontNext;
	
	if (--agTextInitedSubsystem > 0) {
		return;
	}
	for (font = TAILQ_FIRST(&agFontCache);
	     font != TAILQ_END(&agFontCache);
	     font = fontNext) {
		fontNext = TAILQ_NEXT(font, fonts);
		AG_ObjectDestroy(font);
	}
#ifdef HAVE_FONTCONFIG
	if (agFontconfigInited) {
		FcFini();
		agFontconfigInited = 0;
	}
#endif
	AG_MutexDestroy(&agTextLock);
}
