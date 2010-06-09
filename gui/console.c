/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "console.h"
#include "primitive.h"
#include "window.h"

#include <stdarg.h>
#include <string.h>

AG_Console *
AG_ConsoleNew(void *parent, Uint flags)
{
	AG_Console *cons;

	cons = Malloc(sizeof(AG_Console));
	AG_ObjectInit(cons, &agConsoleClass);
	cons->flags |= flags;

	if (flags & AG_CONSOLE_HFILL) { AG_ExpandHoriz(cons); }
	if (flags & AG_CONSOLE_VFILL) { AG_ExpandVert(cons); }

	AG_ObjectAttach(parent, cons);
	return (cons);
}

static __inline__ void
ScrollUp(AG_Event *event)
{
	AG_Console *cons = AG_SELF();
	int n = AG_INT(1);

	if ((cons->rOffs - n) > 0) {
		cons->rOffs -= n;
	} else {
		cons->rOffs = 0;
	}
	AG_Redraw(cons);
}

static __inline__ void
ScrollDown(AG_Event *event)
{
	AG_Console *cons = AG_SELF();
	int n = AG_INT(1);

	if ((cons->rOffs + n) < cons->nLines - cons->rVisible) {
		cons->rOffs += n;
	} else {
		cons->rOffs = cons->nLines - cons->rVisible;
	}
	AG_Redraw(cons);
}

static __inline__ void
PageUp(AG_Event *event)
{
	AG_Console *cons = AG_SELF();

	if ((cons->rOffs - cons->rVisible) > 0) {
		cons->rOffs -= cons->rVisible;
	} else {
		cons->rOffs = 0;
	}
	AG_Redraw(cons);
}

static __inline__ void
PageDown(AG_Event *event)
{
	AG_Console *cons = AG_SELF();

	if ((cons->rOffs + cons->rVisible) < cons->nLines - cons->rVisible) {
		cons->rOffs += cons->rVisible;
	} else {
		cons->rOffs = cons->nLines - cons->rVisible;
	}
	AG_Redraw(cons);
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Console *cons = AG_SELF();
	int btn = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);

	AG_WidgetFocus(cons);
	AG_ExecMouseAction(cons, AG_ACTION_ON_BUTTONDOWN, btn, x, y);
}

static void
MouseButtonUp(AG_Event *event)
{
	AG_Console *cons = AG_SELF();
	int btn = AG_INT(1);
	int x = AG_INT(2);
	int y = AG_INT(3);

	AG_ExecMouseAction(cons, AG_ACTION_ON_BUTTONUP, btn, x, y);
}

static void
KeyDown(AG_Event *event)
{
	AG_Console *cons = AG_SELF();
	int sym = AG_INT(1);
	int mod = AG_INT(2);

	AG_ExecKeyAction(cons, AG_ACTION_ON_KEYDOWN, sym, mod);
}

static void
KeyUp(AG_Event *event)
{
	AG_Console *cons = AG_SELF();
	int sym = AG_INT(1);
	int mod = AG_INT(2);

	AG_ExecKeyAction(cons, AG_ACTION_ON_KEYUP, sym, mod);
}

static void
Init(void *obj)
{
	AG_Console *cons = obj;

	WIDGET(cons)->flags |= AG_WIDGET_FOCUSABLE;

	cons->flags = 0;
	cons->padding = 4;
	cons->lines = NULL;
	cons->lineskip = agTextFontLineSkip - agTextFontHeight;
	cons->nLines = 0;
	cons->rOffs = 0;
	cons->rVisible = 0;
	cons->cBg = AG_ColorRGB(0,0,0);
	cons->vBar = AG_ScrollbarNew(cons, AG_SCROLLBAR_VERT, AG_SCROLLBAR_AUTOSIZE);
	cons->r = AG_RECT(0,0,0,0);
	cons->font = NULL;

	AG_BindInt(cons->vBar, "value", &cons->rOffs);
	AG_BindInt(cons->vBar, "max", &cons->nLines);
	AG_BindInt(cons->vBar, "visible", &cons->rVisible);

	AG_ActionFn(cons, "ScrollUp",	ScrollUp, "%i", 1);
	AG_ActionFn(cons, "ScrollDown",	ScrollDown, "%i", 1);
	AG_ActionFn(cons, "PageUp",	PageUp, NULL);
	AG_ActionFn(cons, "PageDown",   PageDown, NULL);

	AG_ActionOnButtonDown(cons, AG_MOUSE_WHEELUP,		"ScrollUp");
	AG_ActionOnButtonDown(cons, AG_MOUSE_WHEELDOWN,		"ScrollDown");

	AG_ActionOnKey(cons, AG_KEY_UP,		AG_KEYMOD_ANY,	"ScrollUp");
	AG_ActionOnKey(cons, AG_KEY_DOWN,	AG_KEYMOD_ANY,	"ScrollDown");
	AG_ActionOnKey(cons, AG_KEY_PAGEUP,	AG_KEYMOD_ANY,	"PageUp");
	AG_ActionOnKey(cons, AG_KEY_PAGEDOWN,	AG_KEYMOD_ANY,	"PageDown");

	AG_SetEvent(cons, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(cons, "mouse-button-up", MouseButtonUp, NULL);
	AG_SetEvent(cons, "key-down", KeyDown, NULL);
	AG_SetEvent(cons, "key-up", KeyUp, NULL);

#ifdef AG_DEBUG
	AG_BindUint(cons, "flags", &cons->flags);
	AG_BindInt(cons, "padding", &cons->padding);
	AG_BindInt(cons, "lineskip", &cons->lineskip);
	AG_BindInt(cons, "rOffs", &cons->rOffs);
#endif
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_TextSize("XXXXXXXXXXXXXXXXXXXXXXXXX", &r->w, &r->h);
	r->h *= 2;
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Console *cons = p;
	AG_SizeReq rBar;
	AG_SizeAlloc aBar;
	
	if (a->w < 8 || a->h < 8)
		return (-1);

	AG_WidgetSizeReq(cons->vBar, &rBar);
	aBar.x = a->w - rBar.w;
	aBar.y = 0;
	aBar.w = rBar.w;
	aBar.h = a->h;
	AG_WidgetSizeAlloc(cons->vBar, &aBar);
	
	cons->r = AG_RECT(0, 0, (a->w - aBar.w), a->h);
	cons->rVisible = a->h / ((cons->font != NULL) ? cons->font->height :
	                                                agTextFontHeight);
	if (cons->rVisible > 1) {
		cons->rVisible--;
	}
	if (cons->rOffs+cons->rVisible >= cons->nLines) {
		cons->rOffs = MAX(0, cons->nLines - cons->rVisible);
	}
	return (0);
}

static void
Draw(void *p)
{
	AG_Console *cons = p;
	AG_Surface *su;
	AG_Rect rDst;
	Uint r;

	STYLE(cons)->ConsoleBackground(cons, cons->cBg);

	AG_PushTextState();

	if (cons->font != NULL) {
		AG_TextFont(cons->font);
	}
	if (cons->nLines > 0) {
		AG_PushClipRect(cons, cons->r);
		rDst = cons->r;
		for (r = cons->rOffs, rDst.y = cons->padding;
		     r < cons->nLines && rDst.y < WIDGET(cons)->h;
		     r++) {
			AG_ConsoleLine *ln = &cons->lines[r];
	
			if (ln->surface == -1) {
				AG_TextColor(ln->cFg);
				AG_TextBGColor(ln->cBg);
				su = AG_TextRender(ln->text);
				ln->surface = AG_WidgetMapSurface(cons, su);
			}
			AG_WidgetBlitSurface(cons, ln->surface, cons->padding,
			    rDst.y);
			rDst.y += WSURFACE(cons,ln->surface)->h + cons->lineskip;
		}
		AG_PopClipRect(cons);
	}
	
	AG_PopTextState();

	/* Trigger autoscrolling */
	if (cons->rVisible < cons->nLines+1) {
//		printf("autoscroll enable\n");
//		cons->flags |= AG_CONSOLE_AUTOSCROLL;
	} else {
//		printf("autoscroll disable\n");
//		cons->flags &= ~(AG_CONSOLE_AUTOSCROLL);
	}

	AG_WidgetDraw(cons->vBar);
}

static void
FreeLines(AG_Console *cons)
{
	int i;
	
	for (i = 0; i < cons->nLines; i++) {
		Free(cons->lines[i].text);
	}
	Free(cons->lines);
	cons->lines = NULL;
	cons->nLines = 0;
}

static void
Destroy(void *p)
{
	AG_Console *cons = p;

	FreeLines(cons);
}

/* Configure padding in pixels */
void
AG_ConsoleSetPadding(AG_Console *cons, int padding)
{
	AG_ObjectLock(cons);
	cons->padding = padding;
	AG_ObjectUnlock(cons);
}

/* Configure an alternate font */
void
AG_ConsoleSetFont(AG_Console *cons, AG_Font *font)
{
	int i;

	AG_ObjectLock(cons);
	cons->font = font;
	cons->rVisible = HEIGHT(cons) / font->height;

	for (i = 0; i < cons->nLines; i++) {
		AG_ConsoleLine *ln = &cons->lines[i];

		AG_WidgetUnmapSurface(cons, ln->surface);
		ln->surface = -1;
	}
	AG_Redraw(cons);
	AG_ObjectUnlock(cons);
}

/* Append a line to the log */
AG_ConsoleLine *
AG_ConsoleAppendLine(AG_Console *cons, const char *s)
{
	AG_ConsoleLine *ln;

	AG_ObjectLock(cons);

	cons->lines = Realloc(cons->lines, (cons->nLines+1) *
				           sizeof(AG_ConsoleLine));
	ln = &cons->lines[cons->nLines++];
	if (s != NULL) {
		ln->text = Strdup(s);
		ln->len = strlen(s);
	} else {
		ln->text = NULL;
		ln->len = 0;
	}
	ln->cons = cons;
	ln->selected = 0;
	ln->p = NULL;
	ln->surface = -1;
	ln->cBg = AG_ColorRGBA(0,0,0,0);
	ln->cFg = AG_ColorRGB(250,250,230);

	if (cons->flags & AG_CONSOLE_AUTOSCROLL)
		cons->rOffs++;

	AG_Redraw(cons);
	AG_ObjectUnlock(cons);
	return (ln);
}

/* Append a message to the console (format string). */
AG_ConsoleLine *
AG_ConsoleMsg(AG_Console *cons, const char *fmt, ...)
{
	AG_ConsoleLine *ln;
	va_list args;
	size_t len;
	char *s;

	if (cons == NULL) {
		va_start(args, fmt);
		if (TryVasprintf(&s, fmt, args) == -1) {
			return (NULL);
		}
		va_end(args);
		Verbose("%s", s);
		if ((len = strlen(s)) > 1 && s[len - 1] != '\n') {
			Verbose("\n");
		}
		Free(s);
		return (NULL);
	}

	AG_ObjectLock(cons);

	if ((ln = AG_ConsoleAppendLine(cons, NULL)) != NULL) {
		va_start(args, fmt);
		if (TryVasprintf(&ln->text, fmt, args) == -1) {
			va_end(args);
			FreeLines(cons);
			ln = NULL;
			goto out;
		}
		va_end(args);
		ln->len = strlen(ln->text);
		if (ln->len > 1 && ln->text[ln->len - 1] == '\n') {
			ln->text[ln->len - 1] = '\0';
			ln->len--;
		}
	}
out:
	AG_ObjectUnlock(cons);
	return (ln);
}

/* Append a message to the console (C string). */
AG_ConsoleLine *
AG_ConsoleMsgS(AG_Console *cons, const char *s)
{
	AG_ConsoleLine *ln;
	size_t len;
	
	if (cons == NULL) {
		Verbose("%s", s);
		if ((len = strlen(s)) > 1 && s[len - 1] != '\n') {
			Verbose("\n");
		}
		return (NULL);
	}

	AG_ObjectLock(cons);
	if ((ln = AG_ConsoleAppendLine(cons, NULL)) != NULL) {
		if ((ln->text = TryStrdup(s)) == NULL) {
			FreeLines(cons);
			ln = NULL;
			goto out;
		}
		ln->len = strlen(ln->text);
		if (ln->len > 1 && ln->text[ln->len - 1] == '\n') {
			ln->text[ln->len - 1] = '\0';
			ln->len--;
		}
	}
out:
	AG_ObjectUnlock(cons);
	return (ln);
}

void
AG_ConsoleMsgPtr(AG_ConsoleLine *ln, void *p)
{
	AG_ObjectLock(ln->cons);
	ln->p = p;
	AG_ObjectUnlock(ln->cons);
}

void
AG_ConsoleMsgIcon(AG_ConsoleLine *ln, int icon)
{
	AG_ObjectLock(ln->cons);
	ln->icon = icon;
	AG_ObjectUnlock(ln->cons);
}

void
AG_ConsoleClear(AG_Console *cons)
{
	FreeLines(cons);
	cons->rOffs = 0;
	AG_Redraw(cons);
}

AG_WidgetClass agConsoleClass = {
	{
		"Agar(Widget:Console)",
		sizeof(AG_Console),
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
