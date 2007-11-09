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

extern int agFreetype;

static void
EnableMultiline(AG_Textbox *tbox)
{
	tbox->hBar = AG_ScrollbarNew(tbox, AG_SCROLLBAR_HORIZ, 0);
	tbox->vBar = AG_ScrollbarNew(tbox, AG_SCROLLBAR_VERT, 0);

	AG_WidgetBindInt(tbox->hBar, "min", &tbox->xMin);
	AG_WidgetBindInt(tbox->hBar, "value", &tbox->x);
	AG_WidgetBindInt(tbox->hBar, "max", &tbox->xMax);
	AG_WidgetBindInt(tbox->hBar, "visible", &WIDTH(tbox));

	AG_WidgetBindInt(tbox->vBar, "min",   &tbox->yMin);
	AG_WidgetBindInt(tbox->vBar, "value", &tbox->y);
	AG_WidgetBindInt(tbox->vBar, "max",   &tbox->yMax);
	AG_WidgetBindInt(tbox->vBar, "visible", &tbox->yVis);
}

AG_Textbox *
AG_TextboxNew(void *parent, Uint flags, const char *label)
{
	AG_Textbox *tb;

	tb = Malloc(sizeof(AG_Textbox));
	AG_ObjectInit(tb, &agTextboxOps);

	if (flags & AG_TEXTBOX_HFILL) { AG_ExpandHoriz(tb); }
	if (flags & AG_TEXTBOX_VFILL) { AG_ExpandVert(tb); }
	if (flags & AG_TEXTBOX_READONLY) { AG_WidgetDisable(tb); }
	if (tb->flags & AG_TEXTBOX_MULTILINE) { EnableMultiline(tb); }

	if (flags & AG_TEXTBOX_CATCH_TAB) {
		WIDGET(tb)->flags |= AG_WIDGET_CATCH_TAB;
	}
	if (label != NULL) {
		tb->labelText = Strdup(label);
	}
	AG_ObjectAttach(parent, tb);
	return (tb);
}

static int
ProcessKey(AG_Textbox *tb, SDLKey keysym, SDLMod keymod, Uint32 unicode)
{
	AG_WidgetBinding *stringb;
	char *s;
	int i, rv = 0;

	if (keysym == SDLK_ESCAPE) {
		return (0);
	}
	if (keysym == SDLK_RETURN &&
	   (tb->flags & AG_TEXTBOX_MULTILINE) == 0)
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
RepeatTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_Textbox *tb = obj;

	if (ProcessKey(tb, tb->repeat.key, tb->repeat.mod, tb->repeat.unicode)
	    == 0) {
		return (0);
	}
	return (agKbdRepeat);
}

static Uint32
DelayTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_Textbox *tb = obj;

	AG_ReplaceTimeout(tb, &tb->repeat_to, agKbdRepeat);
	AG_DelTimeout(tb, &tb->cblink_to);
	tb->flags |= AG_TEXTBOX_BLINK_ON;
	return (0);
}

static Uint32
BlinkTimeout(void *obj, Uint32 ival, void *arg)
{
	AG_Textbox *tb = obj;

	if ((tb->flags & AG_TEXTBOX_CURSOR_MOVING) == 0) {
		if (tb->flags & AG_TEXTBOX_BLINK_ON) {
			tb->flags &= ~(AG_TEXTBOX_BLINK_ON);
		} else {
			tb->flags |= AG_TEXTBOX_BLINK_ON;
		}
	}
	return (ival);
}

static void
GainedFocus(AG_Event *event)
{
	AG_Textbox *tb = AG_SELF();

	AG_DelTimeout(tb, &tb->delay_to);
	AG_DelTimeout(tb, &tb->repeat_to);
	AG_ReplaceTimeout(tb, &tb->cblink_to, agTextBlinkRate);
	tb->flags |= AG_TEXTBOX_BLINK_ON;
}

static void
LostFocus(AG_Event *event)
{
	AG_Textbox *tb = AG_SELF();

	AG_LockTimeouts(tb);
	AG_DelTimeout(tb, &tb->delay_to);
	AG_DelTimeout(tb, &tb->repeat_to);
	AG_DelTimeout(tb, &tb->cblink_to);
	tb->flags &= ~(AG_TEXTBOX_BLINK_ON|AG_TEXTBOX_CURSOR_MOVING);
	AG_UnlockTimeouts(tb);
}

static void
Destroy(void *p)
{
	AG_Textbox *tbox = p;

	Free(tbox->labelText);
	AG_MutexDestroy(&tbox->lock);
}

static void
Draw(void *p)
{
	AG_Textbox *tbox = p;
	AG_WidgetBinding *stringb;
	int i, x, xStart, y, dx, dy;
	size_t len;
	char *s;
#ifdef UTF8
	Uint32 *ucs;
#endif
#ifdef HAVE_OPENGL
	GLboolean blend_sv;
	GLint blend_sfactor, blend_dfactor;
	GLfloat texenvmode;
#endif

	AG_PushTextState();
	AG_TextColor(TEXTBOX_TXT_COLOR);

	if (tbox->labelText != NULL &&
	    tbox->label == -1) {
		AG_PushTextState();
		AG_TextColor(TEXTBOX_TXT_COLOR);
		tbox->label = AG_WidgetMapSurface(tbox,
		    AG_TextRender(tbox->labelText));
		AG_PopTextState();
	}
	if (tbox->label != -1) {
		SDL_Surface *lblSu = WSURFACE(tbox,tbox->label);
	
		AG_WidgetPushClipRect(tbox, 0, 0, tbox->wLbl, WIDTH(tbox));
		AG_WidgetBlitSurface(tbox, tbox->label, tbox->lblPadL,
		    HEIGHT(tbox)/2 - lblSu->h/2);
		AG_WidgetPopClipRect(tbox);

		xStart = tbox->lblPadL + tbox->wLbl + tbox->lblPadR +
		         tbox->boxPadX;
		x = xStart - tbox->boxPadX;
	} else {
		if (WIDTH(tbox) < (tbox->boxPadX*2 + tbox->lblPadL +
		                   tbox->lblPadR) ||
		    HEIGHT(tbox) < (tbox->boxPadY*2))  {
			goto out;
		}
		x = 0;
		xStart = tbox->boxPadX;
	}
	y = tbox->boxPadY;

	stringb = AG_WidgetGetBinding(tbox, "string", &s);
#ifdef UTF8
	ucs = AG_ImportUnicode(AG_UNICODE_FROM_UTF8, s);
	len = AG_UCS4Len(ucs);
#else
	len = strlen(s);
#endif
	STYLE(tbox)->TextboxBackground(tbox,
	    AG_RECT(x, 0, WIDTH(tbox)-x-1, HEIGHT(tbox)),
	    (tbox->flags & AG_TEXTBOX_COMBO));
#ifdef HAVE_OPENGL
	if (agView->opengl)  {
		glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &texenvmode);
		glGetBooleanv(GL_BLEND, &blend_sv);
		glGetIntegerv(GL_BLEND_SRC, &blend_sfactor);
		glGetIntegerv(GL_BLEND_DST, &blend_dfactor);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
#endif
	AG_WidgetPushClipRect(tbox, tbox->boxPadX, tbox->boxPadY,
	    WIDTH(tbox) - tbox->boxPadX*2,
	    HEIGHT(tbox) - tbox->boxPadY*2);

	x = xStart;
	tbox->xMax = 0;
	tbox->yMax = 1;
	tbox->yVis = WIDTH(tbox)/agTextFontLineSkip;
	
	for (i = 0; i <= len; i++) {
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
			AG_DrawLineV(tbox,
			    x - tbox->x, (y + 1),
			    (y + agTextFontHeight - 2),
			    AG_COLOR(TEXTBOX_CURSOR_COLOR));
		}
		if (i == len)
			break;

		if (c == '\n') {
			if ((tbox->yMax - 1) >= tbox->y) {
				y += agTextFontLineSkip;
			}
			tbox->xMax = MAX(tbox->xMax, x - xStart +
			                 tbox->boxPadX*2);
			x = xStart;
			tbox->yMax++;
			continue;
		} else if (c == '\t') {
			x += agTextTabWidth;
			continue;
		}
		if ((tbox->yMax - 1) < tbox->y)
			continue;

		c = (tbox->flags & AG_TEXTBOX_PASSWORD) ? '*' : c;
		dx = WIDGET(tbox)->cx + x - tbox->x;
		dy = WIDGET(tbox)->cy + y;
		gl = AG_TextRenderGlyph(c);

		if (!agView->opengl) {
			SDL_Rect rd;

			rd.x = dx;
			rd.y = dy;
			x += gl->su->w;
			SDL_BlitSurface(gl->su, NULL, agView->v, &rd);
		} else {
#ifdef HAVE_OPENGL
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
		AG_TextUnusedGlyph(gl);

		if (y >= HEIGHT(tbox))
			break;
	}
	if (tbox->yMax == 1) {
		tbox->xMax = x - xStart + tbox->boxPadX*2;
	}
	if (tbox->flags & AG_TEXTBOX_MULTILINE) {
		int bw;

#if 0
		AG_ScrollbarSetBarSize(tbox->vBar,
		    ((HEIGHT(tbox)-agTextFontLineSkip)/agTextFontLineSkip)* 
		     (HEIGHT(tbox) - tbox->vBar->bw*2)/tbox->yMax);
#endif

		bw = WIDTH(tbox) - HEIGHT(tbox->hBar)*2 - WIDTH(tbox->vBar);
		AG_ScrollbarSetBarSize(tbox->hBar,
		    tbox->xMax < WIDTH(tbox) ? WIDTH(tbox) :
		    bw - abs(tbox->xMax - bw));
	}
	AG_WidgetUnlockBinding(stringb);

	AG_WidgetPopClipRect(tbox);
#ifdef HAVE_OPENGL
	if (agView->opengl) {
		if (blend_sv) {
			glEnable(GL_BLEND);
		} else {
			glDisable(GL_BLEND);
		}
		glBlendFunc(blend_sfactor, blend_dfactor);
	}
#endif
out:
	if (tbox->flags & AG_TEXTBOX_MULTILINE) {
		if (tbox->vBar != NULL && AG_ScrollbarVisible(tbox->vBar)) {
			int d = WIDTH(tbox->vBar);

			AG_DrawBox(tbox,
			    AG_RECT(WIDTH(tbox)-d, HEIGHT(tbox)-d, d, d), -1,
			    AG_COLOR(TEXTBOX_COLOR));
		} else if (tbox->hBar != NULL &&
		           AG_ScrollbarVisible(tbox->hBar)) {
			int d = HEIGHT(tbox->hBar);

			AG_DrawBox(tbox,
			    AG_RECT(WIDTH(tbox)-d, HEIGHT(tbox)-d, d, d), -1,
			    AG_COLOR(TEXTBOX_COLOR));
		}
	}
	    
	AG_PopTextState();
}

void
AG_TextboxSizeHint(AG_Textbox *tbox, const char *text)
{
	AG_TextSize(text, &tbox->wPre, &tbox->hPre);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_Textbox *tbox = p;
	int wLbl, hLbl;

	r->w = tbox->boxPadX*2 + tbox->wPre;
	r->h = tbox->boxPadY*2 + tbox->hPre;

	if (tbox->labelText != NULL) {
		if (tbox->label != -1) {
			wLbl = WSURFACE(tbox,tbox->label)->w;
			hLbl = WSURFACE(tbox,tbox->label)->h;
		} else {
			AG_TextSize(tbox->labelText, &wLbl, &hLbl);
		}
		r->w += tbox->lblPadL + wLbl + tbox->lblPadR;
		r->h = MAX(r->h, hLbl);
	} else {
		r->h = MAX(r->h, agTextFontHeight);
	}
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Textbox *tbox = p;
	int wLbl, hLbl;
	int boxPadW = tbox->boxPadX*2;
	int boxPadH = tbox->boxPadY*2;
	int lblPadW = tbox->lblPadL + tbox->lblPadR;

	if (tbox->labelText == NULL) {
		if (a->w < boxPadW ||
		    a->h < boxPadH)
			return (-1);
	}
	if (a->w < boxPadW + lblPadW ||
	    a->h < boxPadH)
		return (-1);
		
	if (tbox->label != -1) {
		wLbl = WSURFACE(tbox,tbox->label)->w;
		hLbl = WSURFACE(tbox,tbox->label)->h;
	} else {
		AG_TextSize(tbox->labelText, &wLbl, &hLbl);
	}
	if (a->w < boxPadW + lblPadW + wLbl + tbox->wPre) {
		tbox->wLbl = a->w - boxPadW - lblPadW - tbox->wPre;
		if (tbox->wLbl <= 0) {
			if (a->w > boxPadW + lblPadW) {
				tbox->wLbl = 0;
			} else {
				return (-1);
			}
		}
	} else {
		tbox->wLbl = wLbl;
	}

	if (tbox->flags & AG_TEXTBOX_MULTILINE) {
		AG_SizeReq rBar;
		AG_SizeAlloc aBar;
		int d;

		AG_WidgetSizeReq(tbox->hBar, &rBar);
		d = MIN(rBar.h, a->h);
		aBar.x = 0;
		aBar.y = a->h - d;
		aBar.w = a->w - d;
		aBar.h = d;
		AG_WidgetSizeAlloc(tbox->hBar, &aBar);
		
		AG_WidgetSizeReq(tbox->vBar, &rBar);
		d = MIN(rBar.w, a->w);
		aBar.x = a->w - d;
		aBar.y = 0;
		aBar.w = d;
		aBar.h = a->h - d;
		AG_WidgetSizeAlloc(tbox->vBar, &aBar);
	}
	return (0);
}

static void
KeyDown(AG_Event *event)
{
	AG_Textbox *tbox = AG_SELF();
	SDLKey keysym = AG_SDLKEY(1);
	int keymod = AG_INT(2);
	Uint32 unicode = (Uint32)AG_INT(3);		/* XXX use AG_UINT32 */

	if (AG_WidgetDisabled(tbox))
		return;

	if ((tbox->flags & AG_TEXTBOX_CATCH_TAB) == 0 &&
	    keysym == SDLK_TAB) {
		return;
	}

	AG_MutexLock(&tbox->lock);

	tbox->repeat.key = keysym;
	tbox->repeat.mod = keymod;
	tbox->repeat.unicode = unicode;
	tbox->flags |= AG_TEXTBOX_BLINK_ON;

	AG_LockTimeouts(tbox);
	AG_DelTimeout(tbox, &tbox->repeat_to);
	if (ProcessKey(tbox, keysym, keymod, unicode) == 1) {
		AG_ReplaceTimeout(tbox, &tbox->delay_to, agKbdDelay);
	} else {
		AG_DelTimeout(tbox, &tbox->delay_to);
	}
	AG_UnlockTimeouts(tbox);
	
	AG_MutexUnlock(&tbox->lock);
}

static void
KeyUp(AG_Event *event)
{
	AG_Textbox *tb = AG_SELF();
	SDLKey keysym = AG_SDLKEY(1);
	
	AG_MutexLock(&tb->lock);

	AG_LockTimeouts(tb);
	AG_DelTimeout(tb, &tb->repeat_to);
	AG_DelTimeout(tb, &tb->delay_to);
	AG_ReplaceTimeout(tb, &tb->cblink_to, agTextBlinkRate);
	AG_UnlockTimeouts(tb);

	if (keysym == SDLK_RETURN &&
	   (tb->flags & AG_TEXTBOX_MULTILINE) == 0) {
		if (tb->flags & AG_TEXTBOX_ABANDON_FOCUS) {
			AG_WidgetUnfocus(tb);
		}
		AG_PostEvent(NULL, tb, "textbox-return", NULL);
	}
	AG_MutexUnlock(&tb->lock);
}

#define ON_LINE(my,y) \
	( ((my) >= (y) && (my) <= (y)+agTextFontLineSkip) || \
	  ((my) <= yStart && line == 0) || \
	  ((my) > (nLines*agTextFontLineSkip) && (line == nLines-1)) )
#define ON_CHAR(mx,x,glyph) \
	((mx) >= (x) && (mx) <= (x)+(glyph)->advance)

/* Map mouse coordinates to a position within the string. */
static int
GetCursorPosition(AG_Textbox *tbox, int mx, int my, int *pos)
{
	AG_WidgetBinding *stringb;
	AG_Font *font;
	size_t len;
	char *s;
	Uint32 ch;
	int xStart = tbox->boxPadX;
	int yStart = tbox->boxPadY;
	int i, x, y, line = 0;
	int nLines = 1;

	if (tbox->label != -1)
		xStart += tbox->lblPadL + tbox->wLbl + tbox->lblPadR;
	if (AG_WidgetFocused(tbox))
		xStart++;

	x = xStart;
	y = yStart;

	stringb = AG_WidgetGetBinding(tbox, "string", &s);
	len = strlen(s);
	if ((font = AG_FetchFont(NULL, -1, -1)) == NULL)
		fatal("%s", AG_GetError());

	for (i = 0; i < len; i++) {
		if (s[i] == '\n')
			nLines++;
	}
	for (i = 0; i < len; i++) {
		if (mx <= xStart && ON_LINE(my,y)) {
			*pos = i;
			goto in;
		}
		if (s[i] == '\n') {
			if (ON_LINE(my,y) &&
			    mx > x) {
				*pos = i;
				goto in;
			}
			y += agTextFontLineSkip;
			x = xStart;
			line++;
			continue;
		} else if (s[i] == '\t') {
			if (ON_LINE(my,y) &&
			    mx >= x && mx <= x+agTextTabWidth) {
				*pos = (mx < x + agTextTabWidth/2) ? i : i+1;
				goto in;
			}
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
			    TTF_CACHED_METRICS|TTF_CACHED_BITMAP) != 0) {
				continue;
			}
			glyph = ttf->current;
			ftbmp = &glyph->bitmap;

			if (ON_LINE(my,y) && ON_CHAR(mx,x,glyph)) {
				*pos = (mx < x+glyph->advance/2) ? i : i+1;
				goto in;
			}
			x += glyph->advance;
			break;
		}
#endif /* HAVE_FREETYPE */
		case AG_FONT_BITMAP:
		{
			AG_Glyph *gl;
			
			gl = AG_TextRenderGlyph(ch);
			if (ON_LINE(my,y) &&
			    mx >= x && mx <= x+gl->su->w) {
				*pos = i;
				goto in;
			}
			x += gl->su->w;
			AG_TextUnusedGlyph(gl);
			break;
		}
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
#undef ON_LINE
#undef ON_CHAR

static void
MoveCursorToCoords(AG_Textbox *tbox, int mx, int my)
{
	int rv;
	AG_WidgetBinding *stringb;
	char *s;

	AG_MutexLock(&tbox->lock);
	rv = GetCursorPosition(tbox, mx, my, &tbox->pos);
	if (rv == -1) {
		tbox->pos = 0;
	} else if (rv == 1) {
		stringb = AG_WidgetGetBinding(tbox, "string", &s);
		tbox->pos = strlen(s);
		AG_WidgetUnlockBinding(stringb);
	}
	AG_MutexUnlock(&tbox->lock);
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Textbox *tbox = AG_SELF();
	int btn = AG_INT(1);
	int mx = AG_INT(2);
	int my = AG_INT(3);

	AG_WidgetFocus(tbox);

	switch (btn) {
	case SDL_BUTTON_LEFT:
		if (tbox->flags & AG_TEXTBOX_MULTILINE) {
			if ( (AG_ScrollbarVisible(tbox->hBar) &&
			      my >= WIDGET(tbox->hBar)->y) ||
			     (AG_ScrollbarVisible(tbox->vBar) &&
			      mx >= WIDGET(tbox->vBar)->x))
				return;
		}
		tbox->flags |= AG_TEXTBOX_CURSOR_MOVING|AG_TEXTBOX_BLINK_ON;
		mx += tbox->x;
		MoveCursorToCoords(tbox, mx, my);
		break;
	default:
		break;
	}
}

static void
MouseButtonUp(AG_Event *event)
{
	AG_Textbox *tbox = AG_SELF();
	int btn = AG_INT(1);

	switch (btn) {
	case SDL_BUTTON_LEFT:
		tbox->flags &= ~(AG_TEXTBOX_CURSOR_MOVING);
		break;
	default:
		break;
	}
}

static void
MouseMotion(AG_Event *event)
{
	AG_Textbox *tbox = AG_SELF();
	int mx = AG_INT(1);
	int my = AG_INT(2);
	
	if ((tbox->flags & AG_TEXTBOX_CURSOR_MOVING) == 0) {
		return;
	}
	mx += tbox->x;
	if (tbox->flags & AG_TEXTBOX_MULTILINE) {
		if ( (AG_ScrollbarVisible(tbox->hBar) &&
		      my >= WIDGET(tbox->hBar)->y) ||
		     (AG_ScrollbarVisible(tbox->vBar) &&
		      mx >= WIDGET(tbox->vBar)->x) )
			return;
	}
	MoveCursorToCoords(tbox, mx, my);
}

void
AG_TextboxPrintf(AG_Textbox *tbox, const char *fmt, ...)
{
	AG_WidgetBinding *stringb;
	va_list args;
	char *text;

	AG_MutexLock(&tbox->lock);
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
	AG_MutexUnlock(&tbox->lock);
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

/* Enable or disable password mode. */
void
AG_TextboxSetPassword(AG_Textbox *tbox, int pw)
{
	AG_MutexLock(&tbox->lock);
	if (pw) {
		tbox->flags |= AG_TEXTBOX_PASSWORD;
	} else {
		tbox->flags &= ~(AG_TEXTBOX_PASSWORD);
	}
	AG_MutexUnlock(&tbox->lock);
}

/* Change the label text. */
void
AG_TextboxSetLabel(AG_Textbox *tbox, const char *fmt, ...)
{
	va_list ap;
	
	AG_MutexLock(&tbox->lock);

	va_start(ap, fmt);
	Free(tbox->labelText);
	Vasprintf(&tbox->labelText, fmt, ap);
	va_end(ap);

	if (tbox->label != -1) {
		AG_WidgetUnmapSurface(tbox, tbox->label);
		tbox->label = -1;
	}
	AG_MutexUnlock(&tbox->lock);
}

static void
Init(void *obj)
{
	AG_Textbox *tbox = obj;

	WIDGET(tbox)->flags |= AG_WIDGET_FOCUSABLE;

	AG_WidgetBind(tbox, "string", AG_WIDGET_STRING, tbox->string,
	    sizeof(tbox->string));

	tbox->string[0] = '\0';
	tbox->boxPadX = 2;
	tbox->boxPadY = 3;
	tbox->lblPadL = 2;
	tbox->lblPadR = 2;
	tbox->wLbl = 0;
	tbox->flags = AG_TEXTBOX_BLINK_ON;
	tbox->offs = 0;
	tbox->pos = 0;
	tbox->sel_x1 = 0;
	tbox->sel_x2 = 0;
	tbox->sel_edit = 0;
	tbox->compose = 0;
	tbox->wPre = 0;
	tbox->hPre = agTextFontHeight;
	tbox->label = -1;
	tbox->labelText = NULL;
	AG_MutexInitRecursive(&tbox->lock);
	
	tbox->hBar = NULL;
	tbox->vBar = NULL;
	tbox->xMin = 0;
	tbox->x = 0;
	tbox->xMax = 10;
	tbox->yMin = 0;
	tbox->y = 0;
	tbox->yMax = 10;
	tbox->yVis = 1;

	AG_SetEvent(tbox, "window-keydown", KeyDown, NULL);
	AG_SetEvent(tbox, "window-keyup", KeyUp, NULL);
	AG_SetEvent(tbox, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(tbox, "window-mousebuttonup", MouseButtonUp, NULL);
	AG_SetEvent(tbox, "window-mousemotion", MouseMotion, NULL);
	AG_SetEvent(tbox, "widget-gainfocus", GainedFocus, NULL);
	AG_SetEvent(tbox, "widget-lostfocus", LostFocus, NULL);
	AG_SetEvent(tbox, "widget-hidden", LostFocus, NULL);
	AG_SetTimeout(&tbox->repeat_to, RepeatTimeout, NULL, 0);
	AG_SetTimeout(&tbox->delay_to, DelayTimeout, NULL, 0);
	AG_SetTimeout(&tbox->cblink_to, BlinkTimeout, NULL, 0);
}

const AG_WidgetOps agTextboxOps = {
	{
		"AG_Widget:AG_Textbox",
		sizeof(AG_Textbox),
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
