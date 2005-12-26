/*	$Csoft: button.c,v 1.95 2005/10/01 14:15:38 vedge Exp $	*/

/*
 * Copyright (c) 2005 CubeSoft Communications, Inc.
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

#include <core/core.h>
#include <core/view.h>

#include <stdarg.h>
#include <string.h>

#include "button.h"

#include <gui/primitive.h>
#include <gui/text.h>
#include <gui/console.h>

const AG_WidgetOps agConsoleOps = {
	{
		NULL,		/* init */
		NULL,		/* reinit */
		AG_ConsoleDestroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	AG_ConsoleDraw,
	AG_ConsoleScale
};

static void MouseButtonUp(AG_Event *);
static void MouseButtonDown(AG_Event *);
static void KeyUp(AG_Event *);
static void KeyDown(AG_Event *);

AG_Console *
AG_ConsoleNew(void *parent, Uint flags)
{
	AG_Console *cons;

	cons = Malloc(sizeof(AG_Console), M_OBJECT);
	AG_ConsoleInit(cons, flags);
	AG_ObjectAttach(parent, cons);

	if (flags & AG_CONSOLE_FOCUS) {
		AG_WidgetFocus(cons);
	}
	return (cons);
}

void
AG_ConsoleInit(AG_Console *cons, Uint flags)
{
	Uint wflags = AG_WIDGET_FOCUSABLE|AG_WIDGET_CLIPPING;

	if (flags & AG_CONSOLE_HFILL) { wflags |= AG_WIDGET_HFILL; }
	if (flags & AG_CONSOLE_VFILL) { wflags |= AG_WIDGET_VFILL; }

	AG_WidgetInit(cons, "console", &agConsoleOps, wflags);
	cons->flags = flags;
	cons->padding = 4;
	cons->lines = NULL;
	cons->lineskip = agTextFontLineSkip - agTextFontHeight;
	cons->nLines = 0;
	cons->rOffs = 0;
	cons->cBg = SDL_MapRGB(agVideoFmt, 0, 0, 0);
	cons->vBar = AG_ScrollbarNew(cons, AG_SCROLLBAR_VERT, 0);
	AG_WidgetBindInt(cons->vBar, "value", &cons->rOffs);
	AG_WidgetBindInt(cons->vBar, "max", &cons->nLines);

	AG_SetEvent(cons, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(cons, "window-keyup", KeyUp, NULL);
	AG_SetEvent(cons, "window-keydown", KeyDown, NULL);
}

void
AG_ConsoleScale(void *p, int w, int h)
{
	AG_Console *cons = p;

	if (w == -1 && h == -1) {
		AGWIDGET(cons)->w = 1;
		AGWIDGET(cons)->h = 1;
		return;
	}
	AGWIDGET(cons->vBar)->x = AGWIDGET(cons)->w - 20;
	AGWIDGET(cons->vBar)->y = 0;
	AGWIDGET(cons->vBar)->w = 20;
	AGWIDGET(cons->vBar)->h = AGWIDGET(cons)->h;
	cons->vBar->visible = AGWIDGET(cons)->h /
	    (agTextFontHeight + cons->lineskip);
}

void
AG_ConsoleDraw(void *p)
{
	AG_Console *cons = p;
	Uint r;
	int y;
	
	if (AGWIDGET(cons)->w < 8 || AGWIDGET(cons)->h < 8) { return; }
	if (cons->rOffs >= cons->nLines) { cons->rOffs = cons->nLines-1; }

	agPrim.box(cons, 0, 0, AGWIDGET(cons)->w, AGWIDGET(cons)->h, -1,
	    cons->cBg);

	if (cons->nLines == 0)
		return;

	for (r = cons->rOffs, y = cons->padding;
	     r < cons->nLines && y < AGWIDGET(cons)->h;
	     r++) {
		AG_ConsoleLine *ln = &cons->lines[r];

		if (ln->surface == -1) {
			SDL_Surface *su;

			su = AG_TextRender(ln->fontFace, ln->fontSize, ln->cFg,
			    ln->text);
			ln->surface = AG_WidgetMapSurface(cons, su);
		}
		AG_WidgetBlitSurface(cons, ln->surface, cons->padding, y);
		y += AGWIDGET_SURFACE(cons,ln->surface)->h + cons->lineskip;
	}
}

void
AG_ConsoleDestroy(void *p)
{
	AG_Console *cons = p;
	int i;

	for (i = 0; i < cons->nLines; i++) {
		AG_ConsoleLine *ln = &cons->lines[i];

		Free(ln->text, M_WIDGET);
	}
	Free(cons->lines, M_WIDGET);
	cons->lines = NULL;
	AG_WidgetDestroy(cons);
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Console *bu = AG_SELF();
	int button = AG_INT(1);
	
}

static void
KeyDown(AG_Event *event)
{
	AG_Console *bu = AG_SELF();
	int keysym = AG_INT(1);
}

static void
KeyUp(AG_Event *event)
{
	AG_Console *bu = AG_SELF();
	int keysym = AG_INT(1);
}

void
AG_ConsoleSetPadding(AG_Console *bu, int padding)
{
	bu->padding = padding;
}

AG_ConsoleLine *
AG_ConsoleAppendLine(AG_Console *cons, const char *s)
{
	AG_ConsoleLine *ln;

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
	ln->selected = 0;
	ln->p = NULL;
	ln->fontFace = NULL;
	ln->fontSize = 0;
	ln->fontFlags = 0;
	ln->surface = -1;
	ln->cBg = SDL_MapRGBA(agVideoFmt, 0, 0, 0, 0);
	ln->cFg = SDL_MapRGB(agVideoFmt, 250, 250, 230);
	return (ln);
}

AG_ConsoleLine *
AG_ConsoleMsg(AG_Console *cons, const char *fmt, ...)
{
	AG_ConsoleLine *ln;
	va_list args;

	ln = AG_ConsoleAppendLine(cons, NULL);
	va_start(args, fmt);
	AG_Vasprintf(&ln->text, fmt, args);
	va_end(args);
	ln->len = strlen(ln->text);
	return (ln);
}

void
AG_ConsoleMsgPtr(AG_ConsoleLine *ln, void *p)
{
	ln->p = p;
}

void
AG_ConsoleMsgIcon(AG_ConsoleLine *ln, int icon)
{
	ln->icon = icon;
}

