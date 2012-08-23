/*
 * Copyright (c) 2005-2012 Hypertriton, Inc. <http://hypertriton.com/>
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
#include "menu.h"
#include "editable.h"
#include "file_dlg.h"
#include "gui_math.h"

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
ClampVisible(AG_Console *cons)
{
	int v = (int)cons->nLines - (int)cons->rVisible;

	cons->rOffs = MAX(0,v);
}

static void
ScrollUp(AG_Event *event)
{
	AG_Console *cons = AG_SELF();
	int newOffs = (int)cons->rOffs - 1;

	if (newOffs >= 0) {
		cons->rOffs--;
		AG_Redraw(cons);
	}
}

static void
ScrollDown(AG_Event *event)
{
	AG_Console *cons = AG_SELF();
	int newOffs = (int)cons->rOffs + 1;

	if (cons->nLines > cons->rVisible &&
	    newOffs <= (cons->nLines - cons->rVisible)) {
		cons->rOffs++;
		AG_Redraw(cons);
	}
}

static void
PageUp(AG_Event *event)
{
	AG_Console *cons = AG_SELF();
	int newOffs = (int)cons->rOffs - (int)cons->rVisible;

	if (newOffs < 0) {
		newOffs = 0;
	}
	if (cons->rOffs != newOffs) {
		cons->rOffs = (Uint)newOffs;
		AG_Redraw(cons);
	}
}

static void
PageDown(AG_Event *event)
{
	AG_Console *cons = AG_SELF();
	int newOffs;

	if (cons->nLines < cons->rVisible) {
		return;
	}
	newOffs = (int)cons->rOffs + (int)cons->rVisible;
	if (newOffs > (cons->nLines - cons->rVisible))  {
		newOffs = MAX(0, cons->nLines - cons->rVisible);
	}
	if (cons->rOffs != newOffs) {
		cons->rOffs = (Uint)newOffs;
		AG_Redraw(cons);
	}
}

static void
MapLine(AG_Console *cons, int yMouse, int *nLine)
{
	Uint sel;

	if (yMouse < cons->padding) {
		*nLine = cons->rOffs;
	} else if (yMouse > WIDGET(cons)->h) {
		*nLine = (int)cons->nLines - 1;
	} else {
		sel = (yMouse - cons->padding) / cons->lineskip;
		if ((cons->rOffs + sel) >= cons->nLines) {
			*nLine = (int)cons->nLines - 1;
		} else {
			*nLine = (int)(cons->rOffs + sel);
		}
	}
}

/* Export selected lines to a single C string. */
char *
AG_ConsoleExportText(AG_Console *cons, int nativeNL)
{
	char *s, *ps;
	size_t sizeReq = 1;
	int i, dir;

	if (cons->pos == -1) {
		return (NULL);
	}
	dir = (cons->sel < 0) ? -1 : +1;
	for (i = cons->pos;
	     (i >= 0 && i < cons->nLines);
	     i += dir) {
		AG_ConsoleLine *ln = &cons->lines[i];
		sizeReq += ln->len + 2; /* \r\n */
		if (i == cons->pos+cons->sel)
			break;
	}
	if ((s = TryMalloc(sizeReq)) == NULL) {
		return (NULL);
	}
	ps = &s[0];
	*ps = '\0';
	for (i = cons->pos;
	     (i >= 0 && i < cons->nLines);
	     i += dir) {
		AG_ConsoleLine *ln = &cons->lines[i];
		memcpy(ps, ln->text, ln->len);
#ifdef _WIN32
		if (nativeNL) {
			ps[ln->len] = '\r';
			ps[ln->len+1] = '\n';
			ps[ln->len+2] = '\0';
			ps += ln->len + 2;
		} else
#endif
		{
			ps[ln->len] = '\n';
			ps[ln->len+1] = '\0';
			ps += ln->len + 1;
		}
		if (i == cons->pos+cons->sel)
			break;
	}
	return (s);
}
static void
MenuCopy(AG_Event *event)
{
	AG_Console *cons = AG_PTR(1);
	AG_EditableClipboard *cb = &agEditableClipbrd;
	char *s;

	if ((s = AG_ConsoleExportText(cons, 0)) == NULL) {
		return;
	}
	Strlcpy(cb->encoding, "UTF-8", sizeof(cb->encoding));
	Free(cb->s);
	if ((cb->s = AG_ImportUnicode("UTF-8", s, &cb->len, NULL)) == NULL) {
		cb->len = 0;
	}
	free(s);
}
static void
MenuExportToFileTXT(AG_Event *event)
{
	AG_Console *cons = AG_PTR(1);
	char *path = AG_STRING(2);
	char *s;
	FILE *f;

	if ((s = AG_ConsoleExportText(cons, 1)) == NULL) {
		return;
	}
	if ((f = fopen(path, "wb")) == NULL) {
		AG_TextMsg(AG_MSG_ERROR, _("Unable to open %s"), path);
		goto out;
	}
	if (fwrite(s, strlen(s), 1, f) != 1) {
		AG_TextMsg(AG_MSG_ERROR, _("Write error"));
	} else {
		AG_TextTmsg(AG_MSG_INFO, 1000, _("Successfully saved to %s"),
		    path);
	}
	fclose(f);
out:
	free(s);
}
static void
MenuExportToFileDlg(AG_Event *event)
{
	char path[AG_PATHNAME_MAX];
	AG_Console *cons = AG_PTR(1);
	AG_Window *win;
	AG_FileDlg *fd;

	if ((win = AG_WindowNew(0)) == NULL) {
		return;
	}
	AG_WindowSetCaptionS(win, _("Export to text file..."));

	fd = AG_FileDlgNew(win, AG_FILEDLG_SAVE|AG_FILEDLG_CLOSEWIN|
	                        AG_FILEDLG_EXPAND);
	AG_FileDlgSetOptionContainer(fd, AG_BoxNewVert(win, AG_BOX_HFILL));
	AG_GetString(agConfig, "save-path", path, sizeof(path));
	AG_FileDlgSetDirectoryMRU(fd, "agar.console.export-dir", path);
	
	AG_FileDlgAddType(fd, _("Text file"), "*.txt",
	    MenuExportToFileTXT, "%p", cons);
	AG_WindowShow(win);
}

static void
MenuSelectAll(AG_Event *event)
{
	AG_Console *cons = AG_PTR(1);

	cons->pos = 0;
	cons->sel = cons->nLines-1;
}

static AG_PopupMenu *
PopupMenu(AG_Console *cons)
{
	AG_PopupMenu *pm;
	AG_MenuItem *m;

	if ((pm = AG_PopupNew(cons)) == NULL) {
		return (NULL);
	}
	m = AG_MenuAction(pm->item, _("Copy"), NULL,
	    MenuCopy, "%p", cons);
	m->state = (cons->pos != -1);
	m = AG_MenuAction(pm->item, _("Export to file..."), NULL,
	    MenuExportToFileDlg, "%p", cons);
	m->state = (cons->pos != -1);
	AG_MenuSeparator(pm->item);
	AG_MenuAction(pm->item, _("Select All"), NULL,
	    MenuSelectAll, "%p", cons);
	return (pm);
}

static void
BeginSelect(AG_Event *event)
{
	AG_Console *cons = AG_SELF();
	int x = AG_INT(2);
	int y = AG_INT(3);

	if (x < cons->r.x || x > cons->r.x+cons->r.w) {
		return;
	}
	if (!AG_WidgetIsFocused(cons)) {
		AG_WidgetFocus(cons);
	}
	if (cons->pm != NULL) {
		AG_PopupDestroy(cons, cons->pm);
		cons->pm = NULL;
	}
	if (cons->nLines > 0) {
		MapLine(cons, y, &cons->pos);
		cons->sel = 0;
		AG_Redraw(cons);
		cons->flags |= AG_CONSOLE_SELECTING;
	}
}

static void
CloseSelect(AG_Event *event)
{
	AG_Console *cons = AG_SELF();

	cons->flags &= ~(AG_CONSOLE_SELECTING);
}

static void
ShowPopup(AG_Event *event)
{
	AG_Console *cons = AG_SELF();
	int x = AG_INT(2);
	int y = AG_INT(3);
		
	if ((cons->flags & AG_CONSOLE_NOPOPUP) == 0) {
		if (cons->pm != NULL) {
			AG_PopupDestroy(cons, cons->pm);
		}
		if ((cons->pm = PopupMenu(cons)) != NULL)
			AG_PopupShowAt(cons->pm, x, y);
	}
}

static void
MouseMotion(AG_Event *event)
{
	AG_Console *cons = AG_SELF();
	int x = AG_INT(1);
	int y = AG_INT(2);
	int newPos, newSel;

	if (!(cons->flags & AG_CONSOLE_SELECTING) ||
	    x < cons->r.x || x > cons->r.x+cons->r.w) {
		return;
	}
	if (cons->nLines > 0) {
		MapLine(cons, y, &newPos);
		newSel = newPos - cons->pos;
		if ((cons->pos + newSel) == 0) {
			cons->pos = 0;
			if (cons->sel != cons->nLines-1) {
				cons->sel = cons->nLines-1;
				AG_Redraw(cons);
			}
		} else if (newSel != cons->sel) {
			cons->sel = newSel;
			AG_Redraw(cons);
		}
	}
}

static void
Init(void *obj)
{
	AG_Console *cons = obj;

	WIDGET(cons)->flags |= AG_WIDGET_FOCUSABLE;

	cons->flags = 0;
	cons->padding = 4;
	cons->lines = NULL;
	cons->lineskip = agTextFontLineSkip + 1;
	cons->nLines = 0;
	cons->rOffs = 0;
	cons->rVisible = 0;
	cons->cBg = AG_ColorRGB(20,20,20);
	cons->pm = NULL;
	cons->pos = -1;
	cons->sel = 0;
	cons->r = AG_RECT(0,0,0,0);
	cons->font = agDefaultFont;
	cons->scrollTo = NULL;

	cons->vBar = AG_ScrollbarNew(cons, AG_SCROLLBAR_VERT,
	    AG_SCROLLBAR_AUTOSIZE);
/*	cons->vBar->maxOffs = +1; */
	AG_WidgetSetFocusable(cons->vBar, 0);

	AG_BindUint(cons->vBar, "value", &cons->rOffs);
	AG_BindUint(cons->vBar, "max", &cons->nLines);
	AG_BindUint(cons->vBar, "visible", &cons->rVisible);

	AG_ActionFn(cons, "BeginSelect", BeginSelect, NULL);
	AG_ActionFn(cons, "CloseSelect", CloseSelect, NULL);
	AG_ActionFn(cons, "ShowPopup",	ShowPopup, NULL);
	AG_ActionFn(cons, "ScrollUp",	ScrollUp, NULL);
	AG_ActionFn(cons, "ScrollDown",	ScrollDown, NULL);
	AG_ActionFn(cons, "PageUp",	PageUp, NULL);
	AG_ActionFn(cons, "PageDown",   PageDown, NULL);
	AG_ActionOnButtonDown(cons, AG_MOUSE_LEFT, "BeginSelect");
	AG_ActionOnButtonUp(cons, AG_MOUSE_LEFT, "CloseSelect");

	AG_ActionOnButtonDown(cons, AG_MOUSE_RIGHT, "ShowPopup");
	AG_ActionOnButtonDown(cons, AG_MOUSE_WHEELUP, "ScrollUp");
	AG_ActionOnButtonDown(cons, AG_MOUSE_WHEELDOWN, "ScrollDown");

	AG_ActionOnKey(cons, AG_KEY_UP, AG_KEYMOD_ANY, "ScrollUp");
	AG_ActionOnKey(cons, AG_KEY_DOWN, AG_KEYMOD_ANY, "ScrollDown");
	AG_ActionOnKey(cons, AG_KEY_PAGEUP, AG_KEYMOD_ANY, "PageUp");
	AG_ActionOnKey(cons, AG_KEY_PAGEDOWN, AG_KEYMOD_ANY, "PageDown");
	AG_SetEvent(cons, "mouse-motion", MouseMotion, NULL);

#ifdef AG_DEBUG
	AG_BindUint(cons, "nLines", &cons->nLines);
	AG_BindUint(cons, "rOffs", &cons->rOffs);
	AG_BindUint(cons, "rVisible", &cons->rVisible);
	AG_BindInt(cons, "pos", &cons->pos);
	AG_BindInt(cons, "sel", &cons->sel);
#endif
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_TextSize("XXXXXXXXXXXXXXXXXXXXXXXXX", &r->w, &r->h);
	r->h *= 2;
}

static void
ComputeVisible(AG_Console *cons)
{
	cons->rVisible = (int)AG_Floor((float)(cons->r.h - cons->padding*2) /
	                               (float)cons->lineskip);
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
	ComputeVisible(cons);
	ClampVisible(cons);
	return (0);
}

static void
Draw(void *p)
{
	AG_Console *cons = p;
	AG_Surface *su;
	AG_Rect rDst;
	Uint lnIdx;

	STYLE(cons)->ConsoleBackground(cons, cons->cBg);
	
	if (cons->nLines == 0)
		goto out;

	AG_PushTextState();
	AG_TextFont(cons->font);

	if (cons->scrollTo != NULL) {
		cons->rOffs = *cons->scrollTo;
		cons->scrollTo = NULL;
		ClampVisible(cons);
	}
	AG_PushClipRect(cons, cons->r);
	rDst = cons->r;
	rDst.x = cons->padding;
	rDst.y = cons->padding;
	for (lnIdx = cons->rOffs;
	     lnIdx < cons->nLines && rDst.y < WIDGET(cons)->h;
	     lnIdx++) {
		AG_ConsoleLine *ln = &cons->lines[lnIdx];

		if (ln->surface == -1) {
			AG_TextColor(ln->cFg);
			AG_TextBGColor(ln->cBg);
			su = AG_TextRender(ln->text);
			ln->surface = AG_WidgetMapSurface(cons, su);
		}
		if (cons->pos != -1) {
			if ((lnIdx == cons->pos) ||
			    ((cons->sel > 0 && lnIdx > cons->pos && lnIdx < cons->pos + cons->sel + 1) ||
			     (cons->sel < 0 && lnIdx < cons->pos && lnIdx > cons->pos + cons->sel - 1))) {
				AG_DrawRectFilled(cons,
				    AG_RECT(rDst.x, rDst.y,
				            WIDGET(cons)->w - cons->padding*2,
				            cons->lineskip+1),
				    agColors[TEXT_SEL_COLOR]);
			}
		}
		AG_WidgetBlitSurface(cons, ln->surface, rDst.x, rDst.y);
		rDst.y += cons->lineskip;
	}
	AG_PopClipRect(cons);
	AG_PopTextState();
out:
	AG_WidgetDraw(cons->vBar);
}

static void
FreeLines(AG_Console *cons)
{
	Uint i;
	
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
	Uint i;

	AG_ObjectLock(cons);
	cons->font = (font != NULL) ? font : agDefaultFont;
	cons->lineskip = cons->font->lineskip + 1;
	cons->rOffs = 0;
	ComputeVisible(cons);

	for (i = 0; i < cons->nLines; i++) {
		AG_ConsoleLine *ln = &cons->lines[i];

		AG_WidgetUnmapSurface(cons, ln->surface);
		ln->surface = -1;
	}
	AG_ObjectUnlock(cons);
	AG_Redraw(cons);
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
	ln->p = NULL;
	ln->surface = -1;
	ln->cBg = AG_ColorRGBA(0,0,0,0);
	ln->cFg = AG_ColorRGB(250,250,230);

	if ((cons->flags & AG_CONSOLE_NOAUTOSCROLL) == 0) {
		cons->scrollTo = &cons->nLines;
	}
	AG_ObjectUnlock(cons);
	AG_Redraw(cons);
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

/* Change the text of an existing line. */
void
AG_ConsoleMsgEdit(AG_ConsoleLine *ln, const char *s)
{
	AG_ObjectLock(ln->cons);
	Free(ln->text);
	ln->text = Strdup(s);
	ln->len = strlen(s);
	if (ln->surface != -1) {
		AG_WidgetUnmapSurface(ln->cons, ln->surface);
		ln->surface = -1;
	}
	AG_ObjectUnlock(ln->cons);
	AG_Redraw(ln->cons);
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
AG_ConsoleMsgColor(AG_ConsoleLine *ln, const AG_Color *cBg, const AG_Color *cFg)
{
	AG_ObjectLock(ln->cons);
	if (cBg != NULL) { ln->cBg = *cBg; }
	if (cFg != NULL) { ln->cFg = *cFg; }
	AG_ObjectUnlock(ln->cons);
}

void
AG_ConsoleClear(AG_Console *cons)
{
	FreeLines(cons);
	cons->rOffs = 0;
	cons->pos = -1;
	cons->sel = 0;
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
