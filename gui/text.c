/*	$Csoft: text.c,v 1.109 2005/10/04 17:34:56 vedge Exp $	*/

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
#include <config/have_opengl.h>

#include <core/core.h>
#include <core/view.h>
#include <core/config.h>

#ifdef HAVE_FREETYPE
#include <core/loaders/ttf.h>
#endif
#include <core/loaders/xcf.h>

#include <gui/window.h>
#include <gui/vbox.h>
#include <gui/box.h>
#include <gui/label.h>
#include <gui/button.h>
#include <gui/fspinbutton.h>
#include <gui/textbox.h>
#include <gui/keycodes.h>
#include <gui/unicode.h>

#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>

const AG_ObjectOps agFontOps = {
	"AG_Font",
	sizeof(AG_Font),
	{ 0, 0 },
	NULL,		/* init */
	NULL,		/* reinit */
	AG_FontDestroy,
	NULL,		/* load */
	NULL,		/* save */
	NULL,		/* edit */
};

int agTextComposition = 1;		/* Built-in input composition */
int agTextBidi = 0;			/* Bidirectionnal text display */
int agTextFontHeight = 0;		/* Default font height (px) */
int agTextFontAscent = 0;		/* Default font ascent (px) */
int agTextFontDescent = 0;		/* Default font descent (px) */
int agTextFontLineSkip = 0;		/* Default font line skip (px) */
int agTextTabWidth = 40;		/* Tab width (px) */
int agTextBlinkRate = 250;		/* Cursor blink rate (ms) */
int agFreetype = 0;			/* Use Freetype font engine */

#define GLYPH_NBUCKETS 1024

static const char *agTextMsgTitles[] = {
	N_("Error"),
	N_("Warning"),
	N_("Information")
};

AG_Mutex agTextLock = AG_MUTEX_INITIALIZER;
static SLIST_HEAD(ag_fontq, ag_font) fonts = SLIST_HEAD_INITIALIZER(&fonts);
AG_Font *agDefaultFont = NULL;

static struct {
	SLIST_HEAD(, ag_glyph) glyphs;
} agGlyphCache[GLYPH_NBUCKETS+1];

static AG_Timeout text_timeout;		/* Timer for AG_TextTmsg() */

/* Load an individual glyph from a bitmap font file. */
static void
AG_LoadBitmapGlyph(SDL_Surface *su, const char *lbl, void *p)
{
	AG_Font *font = p;

	if (font->nglyphs == 0) {
		strlcpy(font->bspec, lbl, sizeof(font->bspec));
	}
	font->bglyphs = Realloc(font->bglyphs,
	                        (font->nglyphs+1)*sizeof(SDL_Surface *));
	font->bglyphs[font->nglyphs++] = su;
}

void
AG_FontDestroy(void *p)
{
	AG_Font *font = p;
	int i;

	if (!agFreetype) {
		for (i = 0; i < font->nglyphs; i++) {
			SDL_FreeSurface(font->bglyphs[i]);
		}
		Free(font->bglyphs, M_TEXT);
	}
}

AG_Font *
AG_FetchFont(const char *pname, int psize, int pflags)
{
	char path[MAXPATHLEN];
	char name[AG_OBJECT_NAME_MAX];
	char name_obj[AG_OBJECT_NAME_MAX];
	AG_Font *font;
	int size = (psize >= 0) ? psize : AG_Int(agConfig, "font.size");
	Uint flags = (pflags >= 0) ? pflags : AG_Uint(agConfig, "font.flags");
	char *c;

	if (pname != NULL) {
		strlcpy(name, pname, sizeof(name));
	} else {
		AG_StringCopy(agConfig, "font.face", name, sizeof(name));
	}
	memcpy(name_obj, name, sizeof(name_obj));
	for (c = &name_obj[0]; *c != '\0'; c++) {
		if (*c == '.')
			*c = '_';
	}

	AG_MutexLock(&agTextLock);
	SLIST_FOREACH(font, &fonts, fonts) {
#if 0
		printf("font size: %d == %d\n", font->size, size);
		printf("font flags: 0x%x == 0x%x\n", font->flags, flags);
		printf("font name: `%s' == `%s'\n", AGOBJECT(font)->name, name);
#endif
		if (font->size == size &&
		    font->flags == flags &&
		    strcmp(AGOBJECT(font)->name, name_obj) == 0)
			break;
	}
	if (font != NULL)
		goto out;

	font = Malloc(sizeof(AG_Font), M_TEXT);
	AG_ObjectInit(font, name, &agFontOps);
	font->size = size;
	font->flags = flags;
	font->c0 = 0;
	font->c1 = 0;
	font->height = 0;
	font->ascent = 0;
	font->descent = 0;
	font->lineskip = 0;
	
	if (AG_ConfigFile("font-path", name, NULL, path, sizeof(path)) == -1)
		goto fail;

	dprintf("Loading font: %s\n", path);

#ifdef HAVE_FREETYPE
	if (agFreetype) {
		int tflags = 0;

		dprintf("<%s>: Vector (%d pts)\n", name, size);
		if ((font->ttf = AG_TTFOpenFont(path, size)) == NULL) {
			goto fail;
		}
		if (flags & AG_FONT_BOLD)	{ tflags|=TTF_STYLE_BOLD; }
		if (flags & AG_FONT_ITALIC)	{ tflags|=TTF_STYLE_ITALIC; }
		if (flags & AG_FONT_UNDERLINE)	{ tflags|=TTF_STYLE_UNDERLINE; }
		AG_TTFSetFontStyle(font->ttf, tflags);

		font->type = AG_FONT_VECTOR;
		font->height = AG_TTFHeight(font->ttf);
		font->ascent = AG_TTFAscent(font->ttf);
		font->descent = AG_TTFDescent(font->ttf);
		font->lineskip = AG_TTFLineSkip(font->ttf);
	} else
#endif
	{
		char *s;
		char *msig, *c0, *c1, *flags;
		AG_Netbuf *buf;
		
		if ((buf = AG_NetbufOpen(path, "rb", AG_NETBUF_BIG_ENDIAN))
		    == NULL) {
			goto fail;
		}

		font->type = AG_FONT_BITMAP;
		font->bglyphs = Malloc(32*sizeof(SDL_Surface *), M_TEXT);
		font->nglyphs = 0;

		if (AG_XCFLoad(buf, 0, AG_LoadBitmapGlyph, font) == -1) {
			goto fail;
		}
		AG_NetbufClose(buf);

		/* Get the range of characters from the "MAP:x-y" string. */
		s = font->bspec;
		msig = AG_Strsep(&s, ":");
		c0 = AG_Strsep(&s, "-");
		c1 = AG_Strsep(&s, "-");
		if (font->nglyphs < 1 ||
		    msig == NULL || strcmp(msig, "MAP") != 0 ||
		    c0 == NULL || c1 == NULL ||
		    c0[0] == '\0' || c1[0] == '\0') {
			AG_SetError("Missing bitmap fontspec");
			goto fail;
		}
		font->c0 = (Uint32)strtol(c0, NULL, 10);
		font->c1 = (Uint32)strtol(c1, NULL, 10);
		if (font->nglyphs < (font->c1 - font->c0)) {
			AG_SetError("Inconsistent bitmap fontspec");
			goto fail;
		}

		font->height = font->bglyphs[0]->h;
		font->ascent = font->height;
		font->descent = 0;
		font->lineskip = font->height+2;
	
		dprintf("<%s>: Bitmap '%c'-'%c'\n", name,
		    (char)font->c0, (char)font->c1);
	}

	SLIST_INSERT_HEAD(&fonts, font, fonts);
out:
	AG_MutexUnlock(&agTextLock);
	return (font);
fail:
	AG_MutexUnlock(&agTextLock);
	AG_ObjectDestroy(font);
	Free(font, M_TEXT);
	return (NULL);
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

#ifdef HAVE_FREETYPE
	if (AG_Bool(agConfig, "font.freetype")) {
		if (strcmp(AG_String(agConfig, "font.face"),"?") == 0) {
			AG_SetString(agConfig, "font.face", "Vera.ttf");
			AG_SetInt(agConfig, "font.size", 11);
			AG_SetUint(agConfig, "font.flags", 0);
		}
		if (AG_TTFInit() == -1) {
			AG_SetError("AG_TTFInit: %s", SDL_GetError());
			return (-1);
		}
		agFreetype = 1;
		if ((agDefaultFont = AG_FetchFont(NULL, -1, -1)) == NULL)
			fatal("%s", AG_GetError());
	} else
#endif
	{
		if (strcmp(AG_String(agConfig, "font.face"),"?") == 0) {
			AG_SetString(agConfig, "font.face", "minimal.xcf");
			AG_SetInt(agConfig, "font.size", -1);
			AG_SetUint(agConfig, "font.flags", 0);
		}
		agFreetype = 0;
		if ((agDefaultFont = AG_FetchFont(NULL, -1, -1)) == NULL)
			fatal("%s", AG_GetError());
	}
	agTextFontHeight = agDefaultFont->height;
	agTextFontAscent = agDefaultFont->ascent;
	agTextFontDescent = agDefaultFont->descent;
	agTextFontLineSkip = agDefaultFont->lineskip;

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
	AG_Font *font, *nextfont;
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
	
	for (font = SLIST_FIRST(&fonts);
	     font != SLIST_END(&fonts);
	     font = nextfont) {
		nextfont = SLIST_NEXT(font, fonts);
#ifdef HAVE_FREETYPE
		if (font->type == AG_FONT_VECTOR)
			AG_TTFCloseFont(font->ttf);
#endif
		AG_ObjectDestroy(font);
		Free(font, M_TEXT);
	}
#ifdef HAVE_FREETYPE
	if (agFreetype)
		AG_TTFDestroy();
#endif
}

static __inline__ Uint
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
	Uint h;

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
#ifdef HAVE_OPENGL
		gl->texture = AG_SurfaceTexture(gl->su, gl->texcoord);
#endif
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
		Uint h;

		h = hash_glyph(gl->ch);
		SLIST_REMOVE(&agGlyphCache[h].glyphs, gl, ag_glyph, glyphs);
		free_glyph(gl);
	}
}

/* Render UTF-8 text onto a newly allocated transparent surface. */
/* XXX use state variables for font spec */
/* XXX multiline? */
SDL_Surface *
AG_TextRender(const char *fontname, int fontsize, Uint32 color,
    const char *text)
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

SDL_Surface *
AG_TextFormat(const char *fontname, int fontsize, Uint32 color,
    const char *fmt, ...)
{
	char *text;
	va_list args;

	va_start(args, fmt);
	AG_Vasprintf(&text, fmt, args);
	va_end(args);
	return (AG_TextRender(fontname, fontsize, color, text));
}

static __inline__ SDL_Surface *
AG_TextBitmapGlyph(AG_Font *font, Uint32 c)
{
	if ((font->flags & AG_FONT_UPPERCASE) &&
	    (isalpha(c) && islower(c))) {
		c = toupper(c);
	}
	if (c < font->c0 || c > font->c1) {
		return (font->bglyphs[0]);
	}
	return (font->bglyphs[c - font->c0 + 1]);
}

/* Render an UCS-4 text string onto a newly allocated surface. */
SDL_Surface *
AG_TextRenderUnicode(const char *fontname, int fontsize, SDL_Color cFg,
    const Uint32 *text)
{
	AG_Font *font;
	SDL_Surface *su;
	
	if ((font = AG_FetchFont(NULL, -1, -1)) == NULL) {
		fatal("%s", AG_GetError());
	}
	switch (font->type) {
#ifdef HAVE_FREETYPE
	case AG_FONT_VECTOR:
		if ((su = AG_TTFRenderUnicodeSolid(font->ttf, text, NULL, cFg))
		    != NULL) {
			return (su);
		} else {
			fprintf(stderr, "FreeType: %s\n", AG_GetError());
			return (SDL_CreateRGBSurface(SDL_SWSURFACE,0,0,8,
			                             0,0,0,0));
		}
		break;
#endif
	case AG_FONT_BITMAP:
		{
			size_t i, text_len;
			SDL_Surface *gsu;
			SDL_Rect rd;
			Uint w = 0, h = 0;
			Uint32 c;

			/* Figure out the required surface dimensions. */
			text_len = AG_UCS4Len(text);
			for (i = 0; i < text_len; i++) {
				c = text[i];
				gsu = AG_TextBitmapGlyph(font, text[i]);
				w += gsu->w;
				h = MAX(h,gsu->h);
			}

			/* Allocate the final surface. */
			su = SDL_CreateRGBSurface(SDL_SWSURFACE,
			    w, h, 32,
			    agSurfaceFmt->Rmask,
			    agSurfaceFmt->Gmask,
			    agSurfaceFmt->Bmask,
			    0);
			if (su == NULL)
				fatal("SDL_CreateRGBSurface: %s",
				    SDL_GetError());

			/* Blit the glyphs. */
			rd.x = 0;
			rd.y = 0;
			for (i = 0; i < text_len; i++) {
				gsu = AG_TextBitmapGlyph(font, text[i]);
				rd.w = gsu->w;
				rd.h = gsu->h;
				SDL_BlitSurface(gsu, NULL, su, &rd);
				rd.x += gsu->w;
			}
		
			SDL_SetColorKey(su, SDL_SRCCOLORKEY|SDL_RLEACCEL,
			    0);
			SDL_SetAlpha(su,
			    font->bglyphs[0]->flags&(SDL_SRCALPHA|SDL_RLEACCEL),
			    font->bglyphs[0]->format->alpha);

			return (su);
		}
		break;
	default:
		fprintf(stderr, "Cannot render font: %s\n",
		    AGOBJECT(font)->name);
		return (SDL_CreateRGBSurface(SDL_SWSURFACE,0,0,8,0,0,0,0));
	}
}

/* Return the expected size of an Unicode text element. */
void
AG_TextPrescaleUnicode(const Uint32 *ucs, int *w, int *h)
{
	SDL_Surface *su;
	SDL_Color c;

	c.r = 0;
	c.g = 0;
	c.b = 0;

	/* TODO use the bounding box in freetype mode */
	su = AG_TextRenderUnicode(NULL, -1, c, ucs);
	if (w != NULL) { *w = (int)su->w; }
	if (h != NULL) { *h = (int)su->h; }
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
	va_list args;

	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);

	win = AG_WindowNew(AG_WINDOW_NORESIZE|AG_WINDOW_NOCLOSE|
	    AG_WINDOW_NOMINIMIZE|AG_WINDOW_NOMAXIMIZE|AG_WINDOW_NOBORDERS);
	AG_WindowSetCaption(win, "%s", _(agTextMsgTitles[title]));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, 0);
	AG_LabelNew(vb, AG_LABEL_STATIC, msg);

	vb = AG_VBoxNew(win, AG_VBOX_HOMOGENOUS|AG_VBOX_HFILL|AG_VBOX_VFILL);
	AG_ButtonAct(vb, AG_BUTTON_FOCUS, _("Ok"), AGWINDETACH(win));

	AG_WindowShow(win);
}

/* Display a message for a given period of time. */
void
AG_TextTmsg(enum ag_text_msg_title title, Uint32 expire, const char *format,
    ...)
{
	char msg[AG_LABEL_MAX];
	AG_Window *win;
	AG_VBox *vb;
	va_list args;

	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);

	win = AG_WindowNew(AG_WINDOW_NORESIZE|AG_WINDOW_NOCLOSE|
	    AG_WINDOW_NOMINIMIZE|AG_WINDOW_NOMAXIMIZE|AG_WINDOW_NOBORDERS);
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
AG_TextPromptOptions(AG_Button **bOpts, Uint nbOpts, const char *fmt, ...)
{
	char text[AG_LABEL_MAX];
	AG_Window *win;
	AG_Box *bo;
	va_list ap;
	Uint i;

	va_start(ap, fmt);
	vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);

	win = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_NORESIZE|
	    AG_WINDOW_NOTITLE);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_WindowSetSpacing(win, 8);

	AG_LabelNewStatic(win, text);

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	for (i = 0; i < nbOpts; i++) {
		bOpts[i] = AG_ButtonNew(bo, 0, "XXXXXXXXXXX");
	}
	AG_WindowShow(win);
	return (win);
}

/* Prompt the user for a floating-point value. */
void
AG_TextEditFloat(double *fp, double min, double max, const char *unit,
    const char *format, ...)
{
	char msg[AG_LABEL_MAX];
	AG_Window *win;
	AG_VBox *vb;
	va_list args;
	AG_FSpinbutton *fsb;

	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);

	win = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_NOVRESIZE);
	AG_WindowSetCaption(win, "%s", _("Enter real number"));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, AG_VBOX_HFILL);
	AG_LabelNew(vb, AG_LABEL_STATIC, msg);
	
	vb = AG_VBoxNew(win, AG_VBOX_HFILL);
	{
		fsb = AG_FSpinbuttonNew(vb, 0, unit, _("Number: "));
		AG_WidgetBind(fsb, "value", AG_WIDGET_DOUBLE, fp);
		AG_FSpinbuttonSetRange(fsb, min, max);
		AG_SetEvent(fsb, "fspinbutton-return", AGWINDETACH(win));
	}
	
	vb = AG_VBoxNew(win, AG_VBOX_HOMOGENOUS|AG_VBOX_HFILL|AG_VBOX_VFILL);
	AG_ButtonAct(vb, 0, _("Ok"), AGWINDETACH(win));

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
	AG_Textbox *tb;

	va_start(args, msgfmt);
	vsnprintf(msg, sizeof(msg), msgfmt, args);
	va_end(args);

	win = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_NOVRESIZE);
	AG_WindowSetCaption(win, "%s", _("Edit string"));
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 1);

	vb = AG_VBoxNew(win, AG_VBOX_HFILL);
	AG_LabelNew(vb, AG_LABEL_STATIC, msg);
	
	vb = AG_VBoxNew(win, AG_VBOX_HFILL);
	{
		tb = AG_TextboxNew(vb, AG_TEXTBOX_HFILL|AG_TEXTBOX_FOCUS, NULL);
		AG_WidgetBind(tb, "string", AG_WIDGET_STRING, sp, len);
		AG_SetEvent(tb, "textbox-return", AGWINDETACH(win));
	}
	vb = AG_VBoxNew(win, AG_VBOX_HOMOGENOUS|AG_VBOX_HFILL|AG_VBOX_VFILL);
	AG_ButtonAct(vb, 0, _("Ok"), AGWINDETACH(win));
	AG_WindowShow(win);
}

/* Prompt the user for a string. */
void
AG_TextPromptString(const char *prompt, void (*ok_fn)(AG_Event *),
    const char *fmt, ...)
{
	AG_Window *win;
	AG_Box *bo;
	va_list args;
	AG_Button *btn;
	AG_Textbox *tb;
	AG_Event *ev;

	win = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_NOVRESIZE|
	    AG_WINDOW_NOTITLE);
	AG_WindowSetPosition(win, AG_WINDOW_CENTER, 0);
	AG_WindowSetSpacing(win, 8);

	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);
	AG_LabelNew(bo, AG_LABEL_STATIC, prompt);
	
	bo = AG_BoxNew(win, AG_BOX_VERT, AG_BOX_HFILL);
	{
		tb = AG_TextboxNew(bo, AG_TEXTBOX_HFILL|AG_TEXTBOX_FOCUS, NULL);
		ev = AG_SetEvent(tb, "textbox-return", ok_fn, NULL);
		AG_EVENT_GET_ARGS(ev, fmt)
		AG_EVENT_INS_VAL(ev, AG_EVARG_STRING, "string", s,
		    &tb->string[0]);
		AG_AddEvent(tb, "textbox-return", AGWINDETACH(win));
	}

	bo = AG_BoxNew(win, AG_BOX_HORIZ, AG_BOX_HOMOGENOUS|AG_BOX_HFILL);
	{
		btn = AG_ButtonNew(bo, 0, _("Ok"));
		ev = AG_SetEvent(btn, "button-pushed", ok_fn, NULL);
		AG_EVENT_GET_ARGS(ev, fmt);
		AG_EVENT_INS_VAL(ev, AG_EVARG_STRING, "string", s,
		    &tb->string[0]);
		AG_AddEvent(btn, "button-pushed", AGWINDETACH(win));

		AG_ButtonAct(bo, 0, _("Cancel"), AGWINDETACH(win));
	}

	AG_WindowShow(win);
}

/*
 * Parse a command-line font specification and set the default font.
 * The format is <face>,<size>,<flags>. Acceptable flags include 'b'
 * (bold), 'i' (italic) and 'U' (uppercase).
 */
void
AG_TextParseFontSpec(const char *fontspec)
{
	char buf[128];
	char *fs, *s, *c;

	strlcpy(buf, fontspec, sizeof(buf));
	fs = &buf[0];

	if ((s = AG_Strsep(&fs, ":,/")) != NULL &&
	    s[0] != '\0') {
		AG_SetString(agConfig, "font.face", s);
		dprintf("set: %s\n", AG_String(agConfig, "font.face"));
	}
	if ((s = AG_Strsep(&fs, ":,/")) != NULL &&
	    s[0] != '\0') {
		AG_SetInt(agConfig, "font.size", atoi(s));
	}
	if ((s = AG_Strsep(&fs, ":,/")) != NULL &&
	    s[0] != '\0') {
		Uint flags = 0;

		for (c = &s[0]; *c != '\0'; c++) {
			switch (*c) {
			case 'b': flags |= AG_FONT_BOLD;	break;
			case 'i': flags |= AG_FONT_ITALIC;	break;
			case 'U': flags |= AG_FONT_UPPERCASE;	break;
			}
		}
		AG_SetUint(agConfig, "font.flags", flags);
	}
}

