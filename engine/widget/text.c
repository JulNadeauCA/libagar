/*	$Csoft: text.c,v 1.106 2005/09/20 10:22:13 vedge Exp $	*/

/*
 * Copyright (c) 2001, 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#include <config/have_freetype.h>

#include <engine/engine.h>
#include <engine/view.h>
#include <engine/config.h>

#ifdef HAVE_FREETYPE
#include <engine/loader/ttf.h>
#endif

#include <engine/widget/window.h>
#include <engine/widget/vbox.h>
#include <engine/widget/box.h>
#include <engine/widget/label.h>
#include <engine/widget/bitmap.h>
#include <engine/widget/button.h>
#include <engine/widget/fspinbutton.h>
#include <engine/widget/textbox.h>
#include <engine/widget/keycodes.h>

#include <string.h>
#include <stdarg.h>
#include <errno.h>

int agTextComposition = 1;		/* Built-in input composition */
int agTextBidi = 0;			/* Bidirectionnal text display */
int agTextFontHeight = 0;		/* Default font height (px) */
int agTextFontAscent = 0;		/* Default font ascent (px) */
int agTextFontDescent = 0;		/* Default font descent (px) */
int agTextFontLineSkip = 0;		/* Default font line skip (px) */
int agTextTabWidth = 40;		/* Tab width (px) */
int agTextBlinkRate = 250;		/* Cursor blink rate (ms) */

#define GLYPH_NBUCKETS 1024

static const char *agTextMsgTitles[] = {
	N_("Error"),
	N_("Warning"),
	N_("Information")
};

pthread_mutex_t agTextLock = PTHREAD_MUTEX_INITIALIZER;
static SLIST_HEAD(ag_fontq, ag_font) fonts = SLIST_HEAD_INITIALIZER(&fonts);
AG_Font *agDefaultFont = NULL;

static struct {
	SLIST_HEAD(, ag_glyph) glyphs;
} agGlyphCache[GLYPH_NBUCKETS+1];

static AG_Timeout text_timeout;		/* Timer for AG_TextTmsg() */

AG_Font *
AG_FetchFont(const char *name, int size, int style)
{
	char path[MAXPATHLEN];
	AG_Font *font;
	
	pthread_mutex_lock(&agTextLock);
	SLIST_FOREACH(font, &fonts, fonts) {
		if (font->size == size &&
		    font->style == style &&
		    strcmp(font->name, name) == 0)
			break;
	}
	if (font != NULL)
		goto out;

	if (AG_ConfigFile("font-path", name, NULL, path, sizeof(path)) == -1)
		fatal("%s", AG_GetError());
	
	font = Malloc(sizeof(AG_Font), M_TEXT);
	strlcpy(font->name, name, sizeof(font->name));
	font->size = size;
	font->style = style;

#ifdef HAVE_FREETYPE
	dprintf("%s (%d pts)\n", path, size);
	if ((font->p = AG_TTFOpenFont(path, size)) == NULL) {
		fatal("%s: %s", path, AG_GetError());
	}
	AG_TTFSetFontStyle(font->p, style);
#else
	/* TODO */
#endif

	SLIST_INSERT_HEAD(&fonts, font, fonts);
out:
	pthread_mutex_unlock(&agTextLock);
	return (font);
}

static Uint32
expire_tmsg(void *obj, Uint32 ival, void *arg)
{
	AG_Window *win = arg;

	AG_ViewDetach(win);
	return (0);
}

/* Initialize the font engine and configure the default font. */
int
AG_TextInit(void)
{
	int i;

	if (AG_Bool(agConfig, "font-engine") == 0)
		return (0);

#ifdef HAVE_FREETYPE
	if (AG_TTFInit() == -1) {
		AG_SetError("AG_TTFInit: %s", SDL_GetError());
		return (-1);
	}
	agDefaultFont = AG_FetchFont(
	    AG_String(agConfig, "font-engine.default-font"),
	    AG_Int(agConfig, "font-engine.default-size"),
	    AG_Int(agConfig, "font-engine.default-style"));
	agTextFontHeight = AG_TTFHeight(agDefaultFont->p);
	agTextFontAscent = AG_TTFAscent(agDefaultFont->p);
	agTextFontDescent = AG_TTFDescent(agDefaultFont->p);
	agTextFontLineSkip = AG_TTFLineSkip(agDefaultFont->p);
#endif

	for (i = 0; i < GLYPH_NBUCKETS; i++) {
		SLIST_INIT(&agGlyphCache[i].glyphs);
	}
	return (0);
}

static void
free_glyph(AG_Glyph *gl)
{
	SDL_FreeSurface(gl->su);
#ifdef HAVE_OPENGL
	if (agView->opengl)
		glDeleteTextures(1, &gl->texture);
#endif
	Free(gl, M_TEXT);
}

void
AG_TextDestroy(void)
{
	AG_Font *fon, *nextfon;
#ifdef DEBUG
	int maxbucketsz = 0;
#endif
	int i;
	
	for (i = 0; i < GLYPH_NBUCKETS; i++) {
		AG_Glyph *gl, *ngl;
		int bucketsz = 0;

		for (gl = SLIST_FIRST(&agGlyphCache[i].glyphs);
		     gl != SLIST_END(&agGlyphCache[i].glyphs);
		     gl = ngl) {
			ngl = SLIST_NEXT(gl, glyphs);
			free_glyph(gl);
#ifdef DEBUG
			bucketsz++;
#endif
		}
		SLIST_INIT(&agGlyphCache[i].glyphs);
#ifdef DEBUG
		if (bucketsz > maxbucketsz)
			maxbucketsz = bucketsz;
#endif
	}
	
	for (fon = SLIST_FIRST(&fonts);
	     fon != SLIST_END(&fonts);
	     fon = nextfon) {
		nextfon = SLIST_NEXT(fon, fonts);
#ifdef HAVE_FREETYPE
		AG_TTFCloseFont(fon->p);
#endif
		Free(fon, M_TEXT);
	}
#ifdef HAVE_FREETYPE
	AG_TTFDestroy();
#endif
}

static __inline__ u_int
hash_glyph(Uint32 ch)
{
	return (ch % GLYPH_NBUCKETS);
}

/* Lookup/insert a glyph in the glyph cache. */
AG_Glyph *
AG_TextRenderGlyph(const char *fontname, int fontsize, Uint32 color,
    Uint32 ch)
{
	AG_Glyph *gl;
	u_int h;

	h = hash_glyph(ch);
	SLIST_FOREACH(gl, &agGlyphCache[h].glyphs, glyphs) {
		if (fontsize == gl->fontsize &&
		    color == gl->color &&
		    ((fontname == NULL && gl->fontname[0] == '\0') ||
		     (strcmp(fontname, gl->fontname) == 0)) &&
		    ch == gl->ch)
			break;
	}
	if (gl == NULL) {
		Uint32 ucs[2];
		SDL_Color c;

		gl = Malloc(sizeof(AG_Glyph), M_TEXT);
		if (fontname == NULL) {
			gl->fontname[0] = '\0';
		} else {
			strlcpy(gl->fontname, fontname, sizeof(gl->fontname));
		}
		gl->fontsize = fontsize;
		gl->color = color;
		gl->ch = ch;

		SDL_GetRGB(color, agVideoFmt, &c.r, &c.g, &c.b);
		ucs[0] = ch;
		ucs[1] = '\0';
		gl->su = AG_TextRenderUnicode(fontname, fontsize, c, ucs);
		gl->texture = AG_SurfaceTexture(gl->su, gl->texcoord);
	
		gl->nrefs = 1;
		SLIST_INSERT_HEAD(&agGlyphCache[h].glyphs, gl, glyphs);
	} else {
		gl->nrefs++;
	}
	return (gl);
}

void
AG_TextUnusedGlyph(AG_Glyph *gl)
{
	if (--gl->nrefs == 0) {
		u_int h;

		h = hash_glyph(gl->ch);
		SLIST_REMOVE(&agGlyphCache[h].glyphs, gl, ag_glyph, glyphs);
		free_glyph(gl);
	}
}

/* Render UTF-8 text onto a newly allocated transparent surface. */
/* XXX use state variables for font spec */
SDL_Surface *
AG_TextRender(const char *fontname, int fontsize, Uint32 color, const char *text)
{
	SDL_Color c;
	Uint32 *ucs;
	SDL_Surface *su;

	ucs = AG_ImportUnicode(AG_UNICODE_FROM_UTF8, text);
	SDL_GetRGB(color, agVideoFmt, &c.r, &c.g, &c.b);
	su = AG_TextRenderUnicode(fontname, fontsize, c, ucs);
	free(ucs);
	return (su);
}

#ifdef HAVE_FREETYPE

/* Render (possibly multi-line) UCS-4 text onto a newly allocated surface. */
SDL_Surface *
AG_TextRenderUnicode(const char *fontname, int fontsize, SDL_Color cFg,
    const Uint32 *text)
{
	SDL_Rect rd;
	SDL_Surface *su;
	AG_Font *font;
	Uint32 *ucs, *ucsd, *ucsp;
	int nlines, maxw, font_h;
	Uint8 r, g, b, a;

	if (text == NULL || text[0] == '\0') {
		SDL_Surface *su;
	
		su = SDL_CreateRGBSurface(SDL_SWSURFACE, 0, 0, 8, 0, 0, 0, 0);
		if (su == NULL) {
			fatal("SDL_CreateRGBSurface: %s", SDL_GetError());
		}
		return (su);
	}

	font = AG_FetchFont(fontname != NULL ? fontname :
	    AG_String(agConfig, "font-engine.default-font"),
	    fontsize >= 0 ? fontsize :
	    AG_Int(agConfig, "font-engine.default-size"), 0);
	font_h = AG_TTFHeight(font->p);

	/* Find out the line count. */
	ucsd = ucs = AG_UCS4Dup(text);
	for (ucsp = ucs, nlines = 0; *ucsp != '\0'; ucsp++) {
		if (*ucsp == '\n')
			nlines++;
	}

	if (nlines == 0) {					/* One line */
		su = AG_TTFRenderUnicodeSolid(font->p, ucs, NULL, cFg);
		if (su == NULL) {
			fatal("AG_TTFRenderTextSolid: %s", AG_GetError());
		}
	} else {						/* Multiline */
		SDL_Surface **lines;
		int lineskip, i;
		const Uint32 sep[2] = { '\n', '\0' };
		Uint32 colorkey;

		/*
		 * Render the text to an array of surfaces, since we cannot
		 * predict the width of the final surface.
		 * XXX move to AG_TTFRenderUnicodeSolid().
		 */
		lineskip = AG_TTFLineSkip(font->p);
		lines = Malloc(sizeof(SDL_Surface *) * nlines, M_TEXT);
		for (i = 0, maxw = 0;
		    (ucsp = AG_UCS4Sep(&ucs, sep)) != NULL && ucsp[0] != '\0';
		    i++) {
			lines[i] = AG_TTFRenderUnicodeSolid(font->p, ucsp,
			    NULL, cFg);
			if (lines[i] == NULL) {
				fatal("AG_TTFRenderUnicodeSolid: %s",
				    AG_GetError());
			}
			if (lines[i]->w > maxw)
				maxw = lines[i]->w;	/* Grow width */
		}

		rd.x = 0;
		rd.y = 0;
		rd.w = 0;
		rd.h = font_h;

		/* Generate the final surface. */
		su = SDL_CreateRGBSurface(SDL_SWSURFACE, maxw,
		    lineskip*(nlines+1),
		    agVideoFmt->BitsPerPixel,
		    agVideoFmt->Rmask, agVideoFmt->Gmask, agVideoFmt->Bmask, 0);
		if (su == NULL)
			fatal("SDL_CreateRGBSurface: %s", SDL_GetError());

		colorkey = SDL_MapRGB(su->format, 15, 15, 15);
		SDL_FillRect(su, NULL, colorkey);

		for (i = 0, rd.y = lineskip/2;
		     i < nlines;
		     i++, rd.y += lineskip) {
			rd.w = lines[i]->w;
			SDL_SetColorKey(lines[i], 0, 0);
			SDL_BlitSurface(lines[i], NULL, su, &rd);
			SDL_FreeSurface(lines[i]);
		}
		Free(lines, M_TEXT);

		SDL_SetColorKey(su, SDL_SRCCOLORKEY, colorkey);
	}

	free(ucsd);
	return (su);
}

#else /* !HAVE_FREETYPE */

SDL_Surface *
AG_TextRenderUnicode(const char *fontname, int fontsize, SDL_Color cFg,
    const Uint32 *text)
{
	/* TODO bitmap version */
	return (NULL);
}

#endif /* HAVE_FREETYPE */

/* Return the expected size of an Unicode text element. */
void
AG_TextPrescaleUnicode(const Uint32 *ucs, int *w, int *h)
{
	SDL_Surface *su;
	SDL_Color c;

	c.r = 0;
	c.g = 0;
	c.b = 0;

	/* XXX get the bounding box instead */
	su = AG_TextRenderUnicode(NULL, -1, c, ucs);
	if (w != NULL)
		*w = (int)su->w;
	if (h != NULL)
		*h = (int)su->h;
	SDL_FreeSurface(su);
}

/* Return the expected surface size for a UTF-8 string. */
void
AG_TextPrescale(const char *text, int *w, int *h)
{
	Uint32 *ucs;

	ucs = AG_ImportUnicode(AG_UNICODE_FROM_UTF8, text);
	AG_TextPrescaleUnicode(ucs, w, h);
	free(ucs);
}

/* Display a message. */
void
AG_TextMsg(enum ag_text_msg_title title, const char *format, ...)
{
	char msg[AG_LABEL_MAX];
	AG_Window *win;
	AG_VBox *vb;
	AG_Button *bu;
	va_list args;

	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);

	win = AG_WindowNew(AG_WINDOW_NO_RESIZE|AG_WINDOW_NO_CLOSE|
	    AG_WINDOW_NO_MINIMIZE|AG_WINDOW_NO_MAXIMIZE|
	    AG_WINDOW_NO_DECORATIONS, NULL);
	AG_WindowSetCaption(win, "%s", _(agTextMsgTitles[title]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, 0);
	AG_LabelNew(vb, AG_LABEL_STATIC, msg);

	vb = AG_VBoxNew(win, AG_VBOX_HOMOGENOUS|AG_VBOX_WFILL|AG_VBOX_HFILL);
	bu = AG_ButtonNew(vb, _("Ok"));
	AG_SetEvent(bu, "button-pushed", AGWINDETACH(win));

	AG_WidgetFocus(bu);
	AG_WindowShow(win);
}

/* Display a message for a given period of time. */
void
AG_TextTmsg(enum ag_text_msg_title title, Uint32 expire, const char *format, ...)
{
	char msg[AG_LABEL_MAX];
	AG_Window *win;
	AG_VBox *vb;
	va_list args;

	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);

	win = AG_WindowNew(AG_WINDOW_NO_RESIZE|AG_WINDOW_NO_CLOSE|
	    AG_WINDOW_NO_MINIMIZE|AG_WINDOW_NO_MAXIMIZE|
	    AG_WINDOW_NO_DECORATIONS, NULL);
	AG_WindowSetCaption(win, "%s", _(agTextMsgTitles[title]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, 0);
	AG_LabelNew(vb, AG_LABEL_STATIC, msg);
	AG_WindowShow(win);

	AG_LockTimeouts(NULL);
	if (AG_TimeoutIsScheduled(NULL, &text_timeout)) {
		AG_ViewDetach((AG_Window *)text_timeout.arg);
		AG_DelTimeout(NULL, &text_timeout);
	}
	AG_UnlockTimeouts(NULL);

	AG_SetTimeout(&text_timeout, expire_tmsg, win, AG_TIMEOUT_LOADABLE);
	AG_AddTimeout(NULL, &text_timeout, expire);
}

/* Prompt the user with a choice of options. */
AG_Window *
AG_TextPromptOptions(AG_Button **bOpts, u_int nbOpts, const char *fmt, ...)
{
	char text[AG_LABEL_MAX];
	AG_Window *win;
	AG_Box *bo;
	va_list ap;
	u_int i;

	va_start(ap, fmt);
	vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);

	win = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_NO_RESIZE|
	    AG_WINDOW_NO_TITLEBAR, NULL);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_WindowSetSpacing(win, 8);

	AG_LabelStatic(win, text);

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_WFILL);
	for (i = 0; i < nbOpts; i++) {
		bOpts[i] = AG_ButtonNew(bo, "XXXXXXXXXXX");
	}
	AG_WindowShow(win);
	return (win);
}

/* Prompt the user for a floating-point value. */
void
AG_TextPromptFloat(double *fp, double min, double max, const char *unit,
    const char *format, ...)
{
	char msg[AG_LABEL_MAX];
	AG_Window *win;
	AG_VBox *vb;
	va_list args;
	AG_Button *button;
	AG_FSpinbutton *fsb;

	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);

	win = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_NO_VRESIZE, NULL);
	AG_WindowSetCaption(win, "%s", _("Enter real number"));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, AG_VBOX_WFILL);
	AG_LabelNew(vb, AG_LABEL_STATIC, msg);
	
	vb = AG_VBoxNew(win, AG_VBOX_WFILL);
	fsb = AG_FSpinbuttonNew(vb, unit, _("Number: "));
	AGWIDGET(fsb)->flags |= AG_WIDGET_WFILL;
	AG_WidgetBind(fsb, "value", AG_WIDGET_DOUBLE, fp);
	AG_FSpinbuttonSetRange(fsb, min, max);
	AG_SetEvent(fsb, "fspinbutton-return", AGWINDETACH(win));
	
	vb = AG_VBoxNew(win, AG_VBOX_HOMOGENOUS|AG_VBOX_WFILL|AG_VBOX_HFILL);
	button = AG_ButtonNew(vb, _("Ok"));
	AG_SetEvent(button, "button-pushed", AGWINDETACH(win));

	/* TODO test type */

	AG_WindowShow(win);
	AG_WidgetFocus(fsb->input);
}

/* Create a dialog to edit a string value. */
void
AG_TextEditString(char **sp, size_t len, const char *msgfmt, ...)
{
	char msg[AG_LABEL_MAX];
	AG_Window *win;
	AG_VBox *vb;
	va_list args;
	AG_Button *button;
	AG_Textbox *tb;

	va_start(args, msgfmt);
	vsnprintf(msg, sizeof(msg), msgfmt, args);
	va_end(args);

	win = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_NO_VRESIZE, NULL);
	AG_WindowSetCaption(win, "%s", _("Edit string"));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, AG_VBOX_WFILL);
	{
		AG_LabelNew(vb, AG_LABEL_STATIC, msg);
	}
	
	vb = AG_VBoxNew(win, AG_VBOX_WFILL);
	{
		tb = AG_TextboxNew(vb, NULL);
		AGWIDGET(tb)->flags |= AG_WIDGET_WFILL;
		AG_WidgetBind(tb, "string", AG_WIDGET_STRING, sp, len);
		AG_SetEvent(tb, "textbox-return", AGWINDETACH(win));
	}

	vb = AG_VBoxNew(win, AG_VBOX_HOMOGENOUS|AG_VBOX_WFILL|AG_VBOX_HFILL);
	{
		button = AG_ButtonNew(vb, _("Ok"));
		AG_SetEvent(button, "button-pushed", AGWINDETACH(win));
	}
	AG_WindowShow(win);
	AG_WidgetFocus(tb);
}

/* Prompt the user for a string. */
void
AG_TextPromptString(const char *prompt, void (*ok_fn)(int, union evarg *),
    const char *fmt, ...)
{
	AG_Window *win;
	AG_Box *bo;
	va_list args;
	AG_Button *btn;
	AG_Textbox *tb;
	AG_Event *ev;
	const char *fmtp;
	va_list ap;

	win = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_NO_VRESIZE|
	    AG_WINDOW_NO_TITLEBAR, NULL);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_WindowSetSpacing(win, 8);

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_WFILL);
	AG_LabelNew(bo, AG_LABEL_STATIC, prompt);
	
	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_WFILL);
	{
		tb = AG_TextboxNew(bo, NULL);
		AGWIDGET(tb)->flags |= AG_WIDGET_WFILL;
		AG_WidgetFocus(tb);

		ev = AG_SetEvent(tb, "textbox-return", ok_fn, NULL);
		if (fmt != NULL) {
			va_start(ap, fmt);
			for (fmtp = fmt; *fmtp != '\0'; fmtp++) {
				AG_EVENT_PUSH_ARG(ap, *fmtp, ev);
			}
			va_end(ap);
		}
		AG_EVENT_INSERT_VAL(ev, AG_EVARG_STRING, s, &tb->string[0]);
		AG_AddEvent(tb, "textbox-return", AGWINDETACH(win));
	}

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_WFILL);
	{
		btn = AG_ButtonNew(bo, _("Ok"));
		ev = AG_SetEvent(btn, "button-pushed", ok_fn, NULL);
		if (fmt != NULL) {
			va_start(ap, fmt);
			for (fmtp = fmt; *fmtp != '\0'; fmtp++) {
				AG_EVENT_PUSH_ARG(ap, *fmtp, ev);
			}
			va_end(ap);
		}
		AG_EVENT_INSERT_VAL(ev, AG_EVARG_STRING, s, &tb->string[0]);
		AG_AddEvent(btn, "button-pushed", AGWINDETACH(win));

		btn = AG_ButtonNew(bo, _("Cancel"));
		AG_SetEvent(btn, "button-pushed", AGWINDETACH(win));
	}

	AG_WindowShow(win);
}

/*
 * Parse a command-line font specification and set the default font.
 * The format is <face>,<size>,<style>.
 */
void
AG_TextParseFontSpec(char *fontspec)
{
	char *s;

	if ((s = strsep(&fontspec, ":,/")) != NULL &&
	    s[0] != '\0') {
		AG_SetString(agConfig, "font-engine.default-font", s);
	}
	if ((s = strsep(&fontspec, ":,/")) != NULL &&
	    s[0] != '\0') {
		AG_SetInt(agConfig, "font-engine.default-size", atoi(s));
	}
	if ((s = strsep(&fontspec, ":,/")) != NULL &&
	    s[0] != '\0') {
		AG_SetInt(agConfig, "font-engine.default-style", atoi(s));
	}
}
