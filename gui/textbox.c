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
#include <core/view.h>
#include <core/config.h>

#include <config/have_freetype.h>
#include <config/utf8.h>

#include "ttf.h"
#include "textbox.h"

#include "keycodes.h"
#include "primitive.h"
#include "unicode.h"

#include <string.h>
#include <stdarg.h>
#include <ctype.h>

const AG_WidgetOps agTextboxOps = {
	{
		"AG_Widget:AG_Textbox",
		sizeof(AG_Textbox),
		{ 0,0 },
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

extern int agFreetype;

static void mousebuttondown(AG_Event *);
static void mousemotion(AG_Event *);
static void keydown(AG_Event *);
static void keyup(AG_Event *);

AG_Textbox *
AG_TextboxNew(void *parent, Uint flags, const char *label)
{
	AG_Textbox *textbox;

	textbox = Malloc(sizeof(AG_Textbox), M_OBJECT);
	AG_TextboxInit(textbox, flags, label);
	AG_ObjectAttach(parent, textbox);
	if (flags & AG_TEXTBOX_FOCUS) {
		AG_WidgetFocus(textbox);
	}
	return (textbox);
}

static int
AG_TextboxProcessKey(AG_Textbox *tb, SDLKey keysym, SDLMod keymod,
    Uint32 unicode)
{
	AG_WidgetBinding *stringb;
	char *s;
	int i, rv = 0;

	if (keysym == SDLK_RETURN)
		return (0);

	if (keymod == KMOD_NONE && isprint((int)keysym)) {
		if ((tb->flags & AG_TEXTBOX_INT_ONLY)) {
			if (keysym != SDLK_MINUS &&
			    keysym != SDLK_PLUS &&
			    !isdigit((int)keysym)) {
				return (0);
			}
		} else if ((tb->flags & AG_TEXTBOX_FLT_ONLY)) {
			if (keysym != SDLK_PLUS &&
			    keysym != SDLK_MINUS &&
			    keysym != SDLK_PERIOD &&
			    keysym != SDLK_e &&
			    keysym != SDLK_i &&
			    keysym != SDLK_n &&
			    keysym != SDLK_f &&
			    keysym != SDLK_a &&
			    !isdigit((int)keysym)) {
				return (0);
			}
		}
	}

	stringb = AG_WidgetGetBinding(tb, "string", &s);
	if (tb->pos > stringb->data.size)
		goto out;

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
out:
	AG_WidgetUnlockBinding(stringb);
	return (rv);
}

static Uint32
repeat_expire(void *obj, Uint32 ival, void *arg)
{
	AG_Textbox *tb = obj;

	if (AG_TextboxProcessKey(tb, tb->repeat.key, tb->repeat.mod,
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
gained_focus(AG_Event *event)
{
	AG_Textbox *tb = AG_SELF();

	AG_DelTimeout(tb, &tb->delay_to);
	AG_DelTimeout(tb, &tb->repeat_to);
	AG_ReplaceTimeout(tb, &tb->cblink_to, agTextBlinkRate);
}

static void
lost_focus(AG_Event *event)
{
	AG_Textbox *tb = AG_SELF();

	AG_LockTimeouts(tb);
	AG_DelTimeout(tb, &tb->delay_to);
	AG_DelTimeout(tb, &tb->repeat_to);
	AG_DelTimeout(tb, &tb->cblink_to);
	AG_UnlockTimeouts(tb);
}

void
AG_TextboxInit(AG_Textbox *tbox, Uint flags, const char *label)
{
	Uint wflags = AG_WIDGET_FOCUSABLE;

	if (flags & AG_TEXTBOX_HFILL) { wflags |= AG_WIDGET_HFILL; }
	if (flags & AG_TEXTBOX_VFILL) { wflags |= AG_WIDGET_VFILL; }

	AG_WidgetInit(tbox, &agTextboxOps, wflags);
	AG_WidgetBind(tbox, "string", AG_WIDGET_STRING, tbox->string,
	    sizeof(tbox->string));

	tbox->string[0] = '\0';
	tbox->boxPadX = 2;
	tbox->boxPadY = 3;
	tbox->lblPadL = 2;
	tbox->lblPadR = 2;

	tbox->flags = flags|AG_TEXTBOX_BLINK_ON;
	if (flags & AG_TEXTBOX_READONLY)
		AG_WidgetDisable(tbox);

	tbox->prew = tbox->boxPadX*2 + 90;			/* XXX */
	tbox->preh = tbox->boxPadY*2;
	tbox->pos = 0;
	tbox->offs = 0;
	tbox->sel_x1 = 0;
	tbox->sel_x2 = 0;
	tbox->sel_edit = 0;
	tbox->compose = 0;

	if (label != NULL) {
		AG_TextColor(TEXTBOX_TXT_COLOR);
		tbox->label_su = AG_TextRender((char *)label);
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
	int i, x, y, offs;
	size_t len;
	char *s;
#ifdef UTF8
	Uint32 *ucs;
#endif

	if (tbox->label_id >= 0) {
		if (AGWIDGET(tbox)->w < tbox->label_su->w+(tbox->boxPadX*2) ||
		    AGWIDGET(tbox)->h < tbox->label_su->h+(tbox->boxPadY*2)) 
			return;

		AG_WidgetBlitSurface(tbox, tbox->label_id,
		    tbox->lblPadL, AGWIDGET(tbox)->h/2 - tbox->label_su->h/2);
	} else {
		if (AGWIDGET(tbox)->w < (tbox->boxPadX*2 + tbox->lblPadL +
		    tbox->lblPadR) ||
		    AGWIDGET(tbox)->h < (tbox->boxPadY*2)) 
			return;
	}

	if ((font = AG_FetchFont(NULL, -1, -1)) == NULL)
		fatal("%s", AG_GetError());

	stringb = AG_WidgetGetBinding(tbox, "string", &s);
#ifdef UTF8
	ucs = AG_ImportUnicode(AG_UNICODE_FROM_UTF8, s);
	len = AG_UCS4Len(ucs);
#else
	len = strlen(s);
#endif

	x = ((tbox->label_id >= 0) ? tbox->label_su->w : 0) + tbox->lblPadR;
	y = tbox->boxPadY;
	
	if (AG_WidgetDisabled(tbox)) {
		agPrim.box_dithered(tbox, x, 0,
		    AGWIDGET(tbox)->w - x - 1,
		    AGWIDGET(tbox)->h,
		    -1,
		    AG_COLOR(TEXTBOX_COLOR),
		    AG_COLOR(DISABLED_COLOR));
	} else {
		if (tbox->flags & AG_TEXTBOX_COMBO) {
			agPrim.box(tbox, x, 0,
			    AGWIDGET(tbox)->w - x - 1,
			    AGWIDGET(tbox)->h,
			    1,
			    AG_COLOR(TEXTBOX_COLOR));
		} else {
			agPrim.box(tbox, x, 0,
			    AGWIDGET(tbox)->w - x - 1,
			    AGWIDGET(tbox)->h,
			    -1,
			    AG_COLOR(TEXTBOX_COLOR));
		}
	}

	x += tbox->boxPadX;

#ifdef HAVE_OPENGL
	if (agView->opengl)  {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}
#endif

	offs = tbox->offs < len-1 ? tbox->offs : len-1;
	for (i = offs; i <= len; i++) {
		AG_Glyph *gl;
#ifdef UTF8
		Uint32 c = ucs[i];
#else
		char c = s[i];
#endif

		if (i == tbox->pos &&
		    (tbox->flags & AG_TEXTBOX_BLINK_ON) &&
		    AG_WidgetEnabled(tbox) &&
		    AG_WidgetFocused(tbox)) {
			agPrim.vline(tbox,
			    x, (y + 1),
			    (y + agTextFontHeight - 2),
			    AG_COLOR(TEXTBOX_CURSOR_COLOR));
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
			SDL_Rect rd;

			AG_TextColor(TEXTBOX_TXT_COLOR);
			gl = AG_TextRenderGlyph(c);
			rd.x = AGWIDGET(tbox)->cx + x;
			rd.y = AGWIDGET(tbox)->cy + y;
			x += gl->su->w;
			SDL_BlitSurface(gl->su, NULL, agView->v, &rd);
			AG_TextUnusedGlyph(gl);
		} else {
#ifdef HAVE_OPENGL
			int dx, dy;

			dx = AGWIDGET(tbox)->cx + x;
			dy = AGWIDGET(tbox)->cy + y;

			AG_TextColor(TEXTBOX_TXT_COLOR);
			gl = AG_TextRenderGlyph(c);

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
			
			AG_TextUnusedGlyph(gl);
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
	AG_TextSize(text, &tbox->prew, NULL);
	tbox->prew += (tbox->label_id >= 0) ? tbox->label_su->w : 0;
	tbox->prew += tbox->boxPadX*2;
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
keydown(AG_Event *event)
{
	AG_Textbox *tbox = AG_SELF();
	SDLKey keysym = AG_SDLKEY(1);
	int keymod = AG_INT(2);
	Uint32 unicode = (Uint32)AG_INT(3);		/* XXX use AG_UINT32 */

	if (AG_WidgetDisabled(tbox))
		return;

	if (keysym == SDLK_ESCAPE || keysym == SDLK_TAB) {
		AG_PostEvent(NULL, tbox, "textbox-done", NULL);
		return;
	}
	
	tbox->repeat.key = keysym;
	tbox->repeat.mod = keymod;
	tbox->repeat.unicode = unicode;
	tbox->flags |= AG_TEXTBOX_BLINK_ON;

	AG_LockTimeouts(tbox);
	AG_DelTimeout(tbox, &tbox->repeat_to);
	if (AG_TextboxProcessKey(tbox, keysym, keymod, unicode) == 1) {
		AG_ReplaceTimeout(tbox, &tbox->delay_to, agKbdDelay);
	} else {
		AG_DelTimeout(tbox, &tbox->delay_to);
	}
	AG_UnlockTimeouts(tbox);
}

static void
keyup(AG_Event *event)
{
	AG_Textbox *tb = AG_SELF();
	SDLKey keysym = AG_SDLKEY(1);
	int keymod = AG_INT(2);
	Uint32 unicode = (Uint32)AG_INT(3);		/* XXX use AG_UINT32 */

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
AG_TextboxCursorPosition(AG_Textbox *tbox, int mx, int my, int *pos)
{
	AG_WidgetBinding *stringb;
	AG_Font *font;
	int tstart = 0;
	int i, x, x1, y;
	size_t len;
	char *s;
	Uint32 ch;

	x = ((tbox->label_id >= 0) ? tbox->label_su->w : 0) + tbox->boxPadX;
	if (mx <= x) {
		return (-1);
	}
	x += tbox->boxPadX + (AG_WidgetFocused(tbox) ? 1 : 0);
	y = tbox->boxPadY;

	stringb = AG_WidgetGetBinding(tbox, "string", &s);
	len = strlen(s);
	if ((font = AG_FetchFont(NULL, -1, -1)) == NULL)
		fatal("%s", AG_GetError());

	for (i = tstart; i < len; i++) {
		if (s[i] == '\n') {
			y += agTextFontLineSkip;
			continue;
		} else if (s[i] == '\t') {
			x += agTextTabWidth;
			continue;
		}
		
		ch = (Uint32)s[i];
		switch (font->type) {
#ifdef HAVE_FREETYPE
		case AG_FONT_VECTOR:
			{
				AG_TTFFont *ttf = font->ttf;
				FT_Bitmap *ftbmp;
				AG_TTFGlyph *glyph;

				if (AG_TTFFindGlyph(ttf, ch,
				    TTF_CACHED_METRICS|TTF_CACHED_BITMAP)
				    != 0) {
					continue;
				}
				glyph = ttf->current;
				ftbmp = &glyph->bitmap;
				x1 = x + glyph->minx + ftbmp->width;
				if (i == 0 && glyph->minx < 0) {
					x -= glyph->minx;
				}
				if (x1 >= AGWIDGET(tbox)->w) {
					continue;
				}
				if (mx >= x && mx < x1) {
					*pos = i;
					goto in;
				}
				x += glyph->advance;
			}
			break;
#endif /* HAVE_FREETYPE */
		case AG_FONT_BITMAP:
			{
				AG_Glyph *gl;
			
				gl = AG_TextRenderGlyph(ch);
				x1 = x + gl->su->w;
				if (x1 >= AGWIDGET(tbox)->w) { continue; }
				if (mx >= x && mx < x1) { *pos = i; goto in; }
				x += gl->su->w;
				AG_TextUnusedGlyph(gl);
			}
			break;
		default:
			fatal("Unknown font format");
		}
	}
	AG_WidgetUnlockBinding(stringb);
	return (1);
in:
	AG_WidgetUnlockBinding(stringb);
	return (0);
}

static void
AG_TextboxMoveCursor(AG_Textbox *tbox, int mx, int my)
{
	int rv;

	rv = AG_TextboxCursorPosition(tbox, mx, my, &tbox->pos);
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
mousebuttondown(AG_Event *event)
{
	AG_Textbox *tbox = AG_SELF();
	int btn = AG_INT(1);
	int mx = AG_INT(2);
	int my = AG_INT(3);
	int rv;

	if (tbox->label_id < 0 || mx > tbox->label_su->w)
		AG_WidgetFocus(tbox);

	if (btn == SDL_BUTTON_LEFT)
		AG_TextboxMoveCursor(tbox, mx, my);
}

static void
mousemotion(AG_Event *event)
{
	AG_Textbox *tbox = AG_SELF();
	int mx = AG_INT(1);
	int my = AG_INT(2);
	int state = AG_INT(5);

	if (state & SDL_BUTTON_LEFT)
		AG_TextboxMoveCursor(tbox, mx, my);
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
		vsnprintf(text, stringb->data.size, fmt, args);
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
AG_TextboxSetPassword(AG_Textbox *tbox, int pw)
{
	if (pw)
		tbox->flags |= AG_TEXTBOX_PASSWORD;
	else
		tbox->flags &= ~(AG_TEXTBOX_PASSWORD);
}
