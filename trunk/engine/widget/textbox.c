/*	$Csoft: textbox.c,v 1.103 2005/09/27 00:25:23 vedge Exp $	*/

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
#include <engine/config.h>

#include <config/have_freetype.h>
#include <config/utf8.h>

#ifdef HAVE_FREETYPE
#include <engine/loader/ttf.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#endif

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/textbox.h>
#include <engine/widget/keycodes.h>
#include <engine/widget/primitive.h>

#include <string.h>
#include <stdarg.h>
#include <errno.h>

const AG_WidgetOps agTextboxOps = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		AG_WidgetDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_TextboxDraw,
	AG_TextboxScale
};

static void mousebuttondown(int, union evarg *);
static void mousemotion(int, union evarg *);
static void keydown(int, union evarg *);
static void keyup(int, union evarg *);

AG_Textbox *
AG_TextboxNew(void *parent, const char *label)
{
	AG_Textbox *textbox;

	textbox = Malloc(sizeof(AG_Textbox), M_OBJECT);
	AG_TextboxInit(textbox, label);
	AG_ObjectAttach(parent, textbox);
	return (textbox);
}

static int
process_key(AG_Textbox *tb, SDLKey keysym, SDLMod keymod, Uint32 unicode)
{
	int i;
	int rv = 0;

	if (keysym == SDLK_RETURN)
		return (0);

	for (i = 0;; i++) {
		const struct ag_keycode *kcode = &agKeyCodes[i];
		
		if (kcode->key != SDLK_LAST &&
		   (kcode->key != keysym || kcode->func == NULL))
			continue;
		
		if (kcode->key == SDLK_LAST ||
		    kcode->modmask == 0 || (keymod & kcode->modmask)) {
		  	if (kcode->clr_compo) {
				tb->compose = 0;
			}
			AG_PostEvent(NULL, tb, "textbox-prechg", NULL);
			rv = kcode->func(tb, keysym, keymod, kcode->arg,
			    unicode);
			AG_PostEvent(NULL, tb, "textbox-postchg", NULL);
			break;
		}
	}
	return (rv);
}

static Uint32
repeat_expire(void *obj, Uint32 ival, void *arg)
{
	AG_Textbox *tb = obj;

	if (process_key(tb, tb->repeat.key, tb->repeat.mod,
	    tb->repeat.unicode) == 0) {
		return (0);
	}
	return (agKbdRepeat);
}

static Uint32
delay_expire(void *obj, Uint32 ival, void *arg)
{
	AG_Textbox *tb = obj;

	AG_ReplaceTimeout(tb, &tb->repeat_to, agKbdRepeat);
	AG_DelTimeout(tb, &tb->cblink_to);
	tb->flags |= AG_TEXTBOX_BLINK_ON;
	return (0);
}

static Uint32
blink_expire(void *obj, Uint32 ival, void *arg)
{
	AG_Textbox *tb = obj;

	if (tb->flags & AG_TEXTBOX_BLINK_ON) {
		tb->flags &= ~(AG_TEXTBOX_BLINK_ON);
	} else {
		tb->flags |= AG_TEXTBOX_BLINK_ON;
	}
	return (ival);
}

static void
gained_focus(int argc, union evarg *argv)
{
	AG_Textbox *tb = argv[0].p;

	AG_DelTimeout(tb, &tb->delay_to);
	AG_DelTimeout(tb, &tb->repeat_to);
	AG_ReplaceTimeout(tb, &tb->cblink_to, agTextBlinkRate);
}

static void
lost_focus(int argc, union evarg *argv)
{
	AG_Textbox *tb = argv[0].p;

	AG_LockTimeouts(tb);
	AG_DelTimeout(tb, &tb->delay_to);
	AG_DelTimeout(tb, &tb->repeat_to);
	AG_DelTimeout(tb, &tb->cblink_to);
	AG_UnlockTimeouts(tb);
}

void
AG_TextboxInit(AG_Textbox *tbox, const char *label)
{
	AG_WidgetInit(tbox, "textbox", &agTextboxOps,
	    AG_WIDGET_FOCUSABLE|AG_WIDGET_WFILL);
	AG_WidgetBind(tbox, "string", AG_WIDGET_STRING, tbox->string,
	    sizeof(tbox->string));

	tbox->string[0] = '\0';
	tbox->xpadding = 4;
	tbox->ypadding = 3;
	tbox->flags = AG_TEXTBOX_WRITEABLE|AG_TEXTBOX_BLINK_ON;
	tbox->prew = tbox->xpadding*2 + 90;			/* XXX */
	tbox->preh = tbox->ypadding*2;
	tbox->pos = 0;
	tbox->offs = 0;
	tbox->sel_x1 = 0;
	tbox->sel_x2 = 0;
	tbox->sel_edit = 0;
	tbox->compose = 0;

	if (label != NULL) {
		tbox->label_su = AG_TextRender(NULL, -1,
		    AG_COLOR(TEXTBOX_TXT_COLOR), (char *)label);
		tbox->label_id = AG_WidgetMapSurface(tbox, tbox->label_su);
	
		tbox->prew += tbox->label_su->w;
		tbox->preh += MAX(tbox->label_su->h, agTextFontHeight);
	} else {
		tbox->label_su = NULL;
		tbox->label_id = -1;
		tbox->preh += agTextFontHeight;
	}
	
	AG_SetTimeout(&tbox->repeat_to, repeat_expire, NULL, 0);
	AG_SetTimeout(&tbox->delay_to, delay_expire, NULL, 0);
	AG_SetTimeout(&tbox->cblink_to, blink_expire, NULL, 0);

	AG_SetEvent(tbox, "window-keydown", keydown, NULL);
	AG_SetEvent(tbox, "window-keyup", keyup, NULL);
	AG_SetEvent(tbox, "window-mousebuttondown", mousebuttondown, NULL);
	AG_SetEvent(tbox, "window-mousemotion", mousemotion, NULL);
	AG_SetEvent(tbox, "widget-gainfocus", gained_focus, NULL);
	AG_SetEvent(tbox, "widget-lostfocus", lost_focus, NULL);
	AG_SetEvent(tbox, "widget-hidden", lost_focus, NULL);
}

void
AG_TextboxDraw(void *p)
{
	AG_Textbox *tbox = p;
	AG_WidgetBinding *stringb;
	AG_Font *font;
	int i, x, y;
	size_t len;
	char *s;
#ifdef UTF8
	Uint32 *ucs;
#endif

	if (tbox->label_id >= 0) {
		if (AGWIDGET(tbox)->w < tbox->label_su->w+(tbox->xpadding<<1) ||
		    AGWIDGET(tbox)->h < tbox->label_su->h+(tbox->ypadding<<1)) 
			return;

		AG_WidgetBlitSurface(tbox, tbox->label_id,
		    0, AGWIDGET(tbox)->h/2 - tbox->label_su->h/2);
	} else {
		if (AGWIDGET(tbox)->w < (tbox->xpadding<<1) ||
		    AGWIDGET(tbox)->h < (tbox->ypadding<<1)) 
			return;
	}

	font = AG_FetchFont(
	    AG_String(agConfig, "font-engine.default-font"),
	    AG_Int(agConfig, "font-engine.default-size"),
	    0);

	stringb = AG_WidgetGetBinding(tbox, "string", &s);
#ifdef UTF8
	ucs = AG_ImportUnicode(AG_UNICODE_FROM_UTF8, s);
	len = AG_UCS4Len(ucs);
#else
	len = strlen(s);
#endif

	x = ((tbox->label_id >= 0) ? tbox->label_su->w : 0) + tbox->xpadding;
	y = tbox->ypadding;

	agPrim.box(tbox,
	    x,
	    0,
	    AGWIDGET(tbox)->w - x - 1,
	    AGWIDGET(tbox)->h,
	    (AGWIDGET(tbox)->flags & AG_WIDGET_FOCUSED) ? -1 : 1,
	    (tbox->flags & AG_TEXTBOX_WRITEABLE) ? AG_COLOR(TEXTBOX_RW_COLOR) :
	                                        AG_COLOR(TEXTBOX_RO_COLOR));

	x += tbox->xpadding;
	if (AGWIDGET(tbox)->flags & AG_WIDGET_FOCUSED) {
		x++;
		y++;
	}

#ifdef HAVE_OPENGL
	if (agView->opengl)  {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
		    GL_REPLACE);
	}
#endif

	for (i = 0; i <= len; i++) {
		AG_Glyph *gl;
		int invert = 0;
#ifdef UTF8
		Uint32 c = ucs[i];
#else
		char c = s[i];
#endif

		if ((AGWIDGET(tbox)->flags & AG_WIDGET_FOCUSED) &&
		    tbox->flags & AG_TEXTBOX_BLINK_ON) {
			if (i == tbox->pos) {
				agPrim.vline(tbox,
				    x,
				    y + 1,
				    y + agTextFontHeight - 2,
				    AG_COLOR(TEXTBOX_CURSOR_COLOR));
			}
		}
		if (i == len)
			break;

		if (c == '\n') {
			y += agTextFontLineSkip;
			continue;
		} else if (c == '\t') {
			x += agTextTabWidth;
			continue;
		}

		c = (tbox->flags & AG_TEXTBOX_PASSWORD) ? '*' : c;

		if (!agView->opengl) {
			FT_Bitmap *ftbmp;
			AG_TTFFont *ttf = font->p;
			AG_TTFGlyph *glyph;
			int xglyph, yglyph;
			Uint8 *src;

			if (AG_TTFFindGlyph(ttf, c,
			    TTF_CACHED_METRICS|TTF_CACHED_BITMAP) != 0) {
				continue;
			}
			glyph = ttf->current;
			ftbmp = &glyph->bitmap;
			src = ftbmp->buffer;

			if (i == 0 && glyph->minx < 0) {
				x -= glyph->minx;
			}
			if ((x + glyph->minx + ftbmp->width + glyph->advance)
			    >= AGWIDGET(tbox)->w)
				continue;

			for (yglyph = 0; yglyph < ftbmp->rows; yglyph++) {
				/* Work around FreeType 9.3.3 bug. */
				if (glyph->yoffset < 0)
					glyph->yoffset = 0;

				for (xglyph = 0; xglyph < ftbmp->width;
				     xglyph++) {
					if ((invert && src[xglyph]) ||
					   (!invert && !src[xglyph])) {
						continue;
					}
					AG_WidgetPutPixel(tbox,
					    x + glyph->minx + xglyph,
					    y + glyph->yoffset + yglyph,
					    AG_COLOR(TEXTBOX_TXT_COLOR));
				}
				src += ftbmp->pitch;
			}
			x += glyph->advance;
		} else {
#ifdef HAVE_OPENGL
			int dx, dy;

			dx = AGWIDGET(tbox)->cx + x;
			dy = AGWIDGET(tbox)->cy + y;

			gl = AG_TextRenderGlyph(NULL, -1,
			    AG_COLOR(TEXTBOX_TXT_COLOR), c);

			glBindTexture(GL_TEXTURE_2D, gl->texture);
			glBegin(GL_TRIANGLE_STRIP);
			{
				glTexCoord2f(gl->texcoord[0], gl->texcoord[1]);
				glVertex2i(dx, dy);
				glTexCoord2f(gl->texcoord[2], gl->texcoord[1]);
				glVertex2i(dx + gl->su->w, dy);
				glTexCoord2f(gl->texcoord[0], gl->texcoord[3]);
				glVertex2i(dx, dy + gl->su->h);
				glTexCoord2f(gl->texcoord[2], gl->texcoord[3]);
				glVertex2i(dx + gl->su->w, dy + gl->su->h);
			}
			glEnd();
			glBindTexture(GL_TEXTURE_2D, 0);
			x += gl->su->w;
#endif /* HAVE_OPENGL */
		}
		if (x >= AGWIDGET(tbox)->w - 1)
			break;
	}
	AG_WidgetUnlockBinding(stringb);

#ifdef HAVE_OPENGL
	if (agView->opengl)
		glDisable(GL_BLEND);
#endif
}

void
AG_TextboxPrescale(AG_Textbox *tbox, const char *text)
{
	AG_TextPrescale(text, &tbox->prew, NULL);
	tbox->prew += (tbox->label_id >= 0) ? tbox->label_su->w : 0;
	tbox->prew += tbox->xpadding*2;
}

void
AG_TextboxScale(void *p, int rw, int rh)
{
	AG_Textbox *tbox = p;

	if (rw == -1 && rh == -1) {
		AGWIDGET(tbox)->w = tbox->prew;
		AGWIDGET(tbox)->h = tbox->preh;
	}
}

static void
keydown(int argc, union evarg *argv)
{
	AG_Textbox *tbox = argv[0].p;
	SDLKey keysym = (SDLKey)argv[1].i;
	int keymod = argv[2].i;
	Uint32 unicode = (Uint32)argv[3].i;

	if ((tbox->flags & AG_TEXTBOX_WRITEABLE) == 0)
		return;

	if (keysym == SDLK_ESCAPE || keysym == SDLK_TAB) {
		return;
	}
	
	tbox->repeat.key = keysym;
	tbox->repeat.mod = keymod;
	tbox->repeat.unicode = unicode;
	tbox->flags |= AG_TEXTBOX_BLINK_ON;

	AG_LockTimeouts(tbox);
	AG_DelTimeout(tbox, &tbox->repeat_to);
	if (process_key(tbox, keysym, keymod, unicode) == 1) {
		AG_ReplaceTimeout(tbox, &tbox->delay_to, agKbdDelay);
	} else {
		AG_DelTimeout(tbox, &tbox->delay_to);
	}
	AG_UnlockTimeouts(tbox);
}

static void
keyup(int argc, union evarg *argv)
{
	AG_Textbox *tb = argv[0].p;
	SDLKey keysym = (SDLKey)argv[1].i;
	int keymod = argv[2].i;
	Uint32 unicode = (Uint32)argv[3].i;

	AG_LockTimeouts(tb);
	AG_DelTimeout(tb, &tb->repeat_to);
	AG_DelTimeout(tb, &tb->delay_to);
	AG_ReplaceTimeout(tb, &tb->cblink_to, agTextBlinkRate);
	AG_UnlockTimeouts(tb);
	
	if (keysym == SDLK_RETURN) {
		if (tb->flags & AG_TEXTBOX_ABANDON_FOCUS) {
			AG_WidgetUnfocus(tb);
		}
		AG_PostEvent(NULL, tb, "textbox-return", NULL);
		return;
	}
}

/* Map mouse coordinates to a position within the string. */
static int
cursor_position(AG_Textbox *tbox, int mx, int my, int *pos)
{
	AG_WidgetBinding *stringb;
	AG_Font *font;
	int tstart = 0;
	int i, x, y;
	size_t len;
	char *s;

	x = ((tbox->label_id >= 0) ? tbox->label_su->w : 0) + tbox->xpadding;
	if (mx <= x) {
		return (-1);
	}
	x += tbox->xpadding + (AGWIDGET(tbox)->flags & AG_WIDGET_FOCUSED) ? 1:0;
	y = tbox->ypadding;

	stringb = AG_WidgetGetBinding(tbox, "string", &s);
	len = strlen(s);
	font = AG_FetchFont(
	    AG_String(agConfig, "font-engine.default-font"),
	    AG_Int(agConfig, "font-engine.default-size"), 0);

	for (i = tstart; i < len; i++) {
		if (s[i] == '\n') {
			y += agTextFontLineSkip;
			continue;
		} else if (s[i] == '\t') {
			x += agTextTabWidth;
			continue;
		}

		{
			Uint32 ch = (Uint32)s[i];
			AG_TTFFont *ttf = font->p;
			FT_Bitmap *ftbmp;
			AG_TTFGlyph *glyph;

			if (AG_TTFFindGlyph(ttf, ch,
			    TTF_CACHED_METRICS|TTF_CACHED_BITMAP) != 0) {
				continue;
			}
			glyph = ttf->current;
			ftbmp = &glyph->bitmap;

			if (i == 0 && glyph->minx < 0)
				x -= glyph->minx;
			if ((x + glyph->minx+ftbmp->width)
			    >= AGWIDGET(tbox)->w)
				continue;
		
			if (mx >= x &&
			    mx < x+glyph->minx+ftbmp->width) {
				*pos = i;
				goto in;
			}
			x += glyph->advance;
		}
	}
	AG_WidgetUnlockBinding(stringb);
	return (1);
in:
	AG_WidgetUnlockBinding(stringb);
	return (0);
}

static void
move_cursor(AG_Textbox *tbox, int mx, int my)
{
	int rv;

	rv = cursor_position(tbox, mx, my, &tbox->pos);
	if (rv == -1) {
		tbox->pos = 0;
	} else if (rv == 1) {
		AG_WidgetBinding *stringb;
		char *s;
		
		stringb = AG_WidgetGetBinding(tbox, "string", &s);
		tbox->pos = strlen(s);
		AG_WidgetUnlockBinding(stringb);
	}
}

static void
mousebuttondown(int argc, union evarg *argv)
{
	AG_Textbox *tbox = argv[0].p;
	int btn = argv[1].i;
	int mx = argv[2].i;
	int my = argv[3].i;
	int rv;

	if (tbox->label_id < 0 || mx > tbox->label_su->w)
		AG_WidgetFocus(tbox);

	if (btn == SDL_BUTTON_LEFT)
		move_cursor(tbox, mx, my);
}

static void
mousemotion(int argc, union evarg *argv)
{
	AG_Textbox *tbox = argv[0].p;
	int mx = argv[1].i;
	int my = argv[2].i;
	int state = argv[5].i;

	if (state & SDL_BUTTON_LEFT)
		move_cursor(tbox, mx, my);
}

void
AG_TextboxPrintf(AG_Textbox *tbox, const char *fmt, ...)
{
	AG_WidgetBinding *stringb;
	va_list args;
	char *text;

	stringb = AG_WidgetGetBinding(tbox, "string", &text);
	if (fmt != NULL && fmt[0] != '\0') {
		va_start(args, fmt);
		vsnprintf(text, stringb->size, fmt, args);
		va_end(args);
		tbox->pos = strlen(text);
	} else {
		text[0] = '\0';
		tbox->pos = 0;
	}
	AG_WidgetUnlockBinding(stringb);
}

char *
AG_TextboxDupString(AG_Textbox *tbox)
{
	AG_WidgetBinding *stringb;
	char *s, *sd;

	stringb = AG_WidgetGetBinding(tbox, "string", &s);
	sd = Strdup(s);
	AG_WidgetUnlockBinding(stringb);
	return (sd);
}

/* Copy text to a fixed-size buffer and always NUL-terminate. */
size_t
AG_TextboxCopyString(AG_Textbox *tbox, char *dst, size_t dst_size)
{
	AG_WidgetBinding *stringb;
	size_t rv;
	char *text;

	stringb = AG_WidgetGetBinding(tbox, "string", &text);
	rv = strlcpy(dst, text, dst_size);
	AG_WidgetUnlockBinding(stringb);
	return (rv);
}

/* Perform trivial conversion from string to int. */
int
AG_TextboxInt(AG_Textbox *tbox)
{
	AG_WidgetBinding *stringb;
	char *text;
	int i;

	stringb = AG_WidgetGetBinding(tbox, "string", &text);
	i = atoi(text);
	AG_WidgetUnlockBinding(stringb);
	return (i);
}

void
AG_TextboxSetWriteable(AG_Textbox *tbox, int wr)
{
	if (wr)
		tbox->flags |= AG_TEXTBOX_WRITEABLE;
	else
		tbox->flags &= ~(AG_TEXTBOX_WRITEABLE);
}

void
AG_TextboxSetPassword(AG_Textbox *tbox, int pw)
{
	if (pw)
		tbox->flags |= AG_TEXTBOX_PASSWORD;
	else
		tbox->flags &= ~(AG_TEXTBOX_PASSWORD);
}
