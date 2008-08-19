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

#include "console.h"
#include "primitive.h"

#include <stdarg.h>
#include <string.h>

static void MouseButtonDown(AG_Event *);
static void KeyUp(AG_Event *);
static void KeyDown(AG_Event *);

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

static void
Init(void *obj)
{
	AG_Console *cons = obj;

	WIDGET(cons)->flags |= AG_WIDGET_FOCUSABLE|AG_WIDGET_CLIPPING;

	cons->flags = 0;
	cons->padding = 4;
	cons->lines = NULL;
	cons->lineskip = agTextFontLineSkip - agTextFontHeight;
	cons->nLines = 0;
	cons->rOffs = 0;
	cons->cBg = AG_MapRGB(agVideoFmt, 0,0,0);
	cons->vBar = AG_ScrollbarNew(cons, AG_SCROLLBAR_VERT, 0);
	AG_WidgetBindInt(cons->vBar, "value", &cons->rOffs);
	AG_WidgetBindInt(cons->vBar, "max", &cons->nLines);

	AG_SetEvent(cons, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(cons, "window-keyup", KeyUp, NULL);
	AG_SetEvent(cons, "window-keydown", KeyDown, NULL);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_TextSize("XXXXXXXXXXXXXXXXXXXXXXXXX", &r->w, &r->h);
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Console *cons = p;
	AG_SizeAlloc aBar;
	
	if (a->w < 8 || a->h < 8)
		return (-1);

	aBar.w = 20;					/* XXX */
	aBar.x = a->w - aBar.w;
	aBar.y = 0;
	aBar.h = a->h;
	AG_WidgetSizeAlloc(cons->vBar, &aBar);

	cons->vBar->visible = a->h / (agTextFontHeight + cons->lineskip);
	return (0);
}

static void
Draw(void *p)
{
	AG_Console *cons = p;
	AG_Surface *su;
	Uint r;
	int y;

	STYLE(cons)->ConsoleBackground(cons, cons->cBg);

	if (cons->rOffs >= cons->nLines) {
		cons->rOffs = cons->nLines-1;
	}
	if (cons->nLines == 0) {
		return;
	}
	for (r = cons->rOffs, y = cons->padding;
	     r < cons->nLines && y < WIDGET(cons)->h;
	     r++) {
		AG_ConsoleLine *ln = &cons->lines[r];

		if (ln->surface == -1) {
			if (ln->font != NULL) {
				AG_TextFont(ln->font);
			}
			AG_TextColor32(ln->cFg);
			su = AG_TextRender(ln->text);
			ln->surface = AG_WidgetMapSurface(cons, su);
		}
		AG_WidgetBlitSurface(cons, ln->surface, cons->padding, y);
		y += WSURFACE(cons,ln->surface)->h + cons->lineskip;
	}
}

static void
Destroy(void *p)
{
	AG_Console *cons = p;
	int i;

	for (i = 0; i < cons->nLines; i++) {
		AG_ConsoleLine *ln = &cons->lines[i];
		Free(ln->text);
	}
	Free(cons->lines);
	cons->lines = NULL;
}

static void
MouseButtonDown(AG_Event *event)
{
#if 0
	AG_Console *bu = AG_SELF();
	int button = AG_INT(1);
#endif
}

static void
KeyDown(AG_Event *event)
{
#if 0
	AG_Console *bu = AG_SELF();
	int keysym = AG_INT(1);
#endif
}

static void
KeyUp(AG_Event *event)
{
#if 0
	AG_Console *bu = AG_SELF();
	int keysym = AG_INT(1);
#endif
}

void
AG_ConsoleSetPadding(AG_Console *cons, int padding)
{
	AG_ObjectLock(cons);
	cons->padding = padding;
	AG_ObjectUnlock(cons);
}

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
	ln->font = NULL;
	ln->surface = -1;
	ln->cBg = AG_MapRGBA(agVideoFmt, 0,0,0,0);
	ln->cFg = AG_MapRGB(agVideoFmt, 250,250,230);
	
	AG_ObjectUnlock(cons);
	return (ln);
}

AG_ConsoleLine *
AG_ConsoleMsg(AG_Console *cons, const char *fmt, ...)
{
	AG_ConsoleLine *ln;
	va_list args;

	AG_ObjectLock(cons);

	ln = AG_ConsoleAppendLine(cons, NULL);
	va_start(args, fmt);
	Vasprintf(&ln->text, fmt, args);
	va_end(args);
	ln->len = strlen(ln->text);
	
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
