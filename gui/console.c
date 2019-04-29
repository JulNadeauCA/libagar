/*
 * Copyright (c) 2005-2019 Julien Nadeau Carriere <vedge@csoft.net>
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

#include <agar/core/core.h>
#include <agar/core/config.h>
#include <agar/gui/console.h>
#include <agar/gui/primitive.h>
#include <agar/gui/window.h>
#include <agar/gui/menu.h>
#include <agar/gui/editable.h>
#include <agar/gui/file_dlg.h>
#include <agar/gui/gui_math.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>

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
AdjustXoffs(AG_Console *_Nonnull cons)
{
	Uint i;
	int wCons = WIDTH(cons);
	int wBar = WIDTH(cons->vBar) + cons->padding;

	for (i = 0, cons->wMax = 0;
	     i < cons->nLines;
	     i++) {
		AG_ConsoleLine *ln = cons->lines[i];
		int w;

		if (ln->surface[0] != -1) {
			w = WSURFACE(cons,ln->surface[0])->w;
		} else if (ln->surface[1] != -1) {
			w = WSURFACE(cons,ln->surface[1])->w;
		} else {
			AG_TextSize(ln->text, &w, NULL);
		}
		if (w+wBar > cons->wMax)
			cons->wMax = w+wBar;
	}
	if ((cons->wMax - wCons - cons->xOffs) < 0)
		cons->xOffs = MAX(0, cons->wMax - wCons);
}

static __inline__ void
ClampVisible(AG_Console *_Nonnull cons)
{
	int v = (int)cons->nLines - (int)cons->rVisible;

	cons->rOffs = MAX(0,v);
}

static void
ScrollUp(AG_Event *_Nonnull event)
{
	AG_Console *cons = AG_SELF();
	int newOffs = (int)cons->rOffs - 1;

	if (newOffs >= 0) {
		cons->rOffs--;
		AG_Redraw(cons);
	}
}

static void
ScrollDown(AG_Event *_Nonnull event)
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
PageUp(AG_Event *_Nonnull event)
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
PageDown(AG_Event *_Nonnull event)
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
MapLine(AG_Console *_Nonnull cons, int yMouse, int *_Nonnull nLine)
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
	AG_Size sizeReq = 1;
	int i, dir;

	if (cons->pos == -1) {
		return (NULL);
	}
	dir = (cons->sel < 0) ? -1 : +1;
	for (i = cons->pos;
	     (i >= 0 && i < cons->nLines);
	     i += dir) {
		AG_ConsoleLine *ln = cons->lines[i];
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
		AG_ConsoleLine *ln = cons->lines[i];

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
MenuCopy(AG_Event *_Nonnull event)
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
static int
MenuCopyActive(AG_Event *_Nonnull event)
{
	AG_Console *cons = AG_PTR(1);
	return (cons->pos != -1) ? 1 : 0;
}

static int
MenuExportToFileTXT(AG_Event *_Nonnull event)
{
	AG_Console *cons = AG_PTR(1);
	char *path = AG_STRING(2);
	char *s;
	FILE *f;

	if ((s = AG_ConsoleExportText(cons, 1)) == NULL) {
		return (-1);
	}
	if ((f = fopen(path, "wb")) == NULL) {
		AG_SetError("%s: %s", path, AG_Strerror(errno));
		free(s);
		return (-1);
	}
	fwrite(s, strlen(s), 1, f);
	fclose(f);
	free(s);
	return (0);
}
static void
MenuExportToFileDlg(AG_Event *_Nonnull event)
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
	AG_GetString(AG_ConfigObject(), "save-path", path, sizeof(path));
	AG_FileDlgSetDirectoryMRU(fd, "agar.console.export-dir", path);
	
	AG_FileDlgAddType(fd, _("Text file"), "*.txt",
	    MenuExportToFileTXT, "%p", cons);

	AG_WindowShow(win);
}

static void
MenuSelectAll(AG_Event *_Nonnull event)
{
	AG_Console *cons = AG_PTR(1);

	cons->pos = 0;
	cons->sel = cons->nLines-1;
	AG_Redraw(cons);
}

static void
BeginSelect(AG_Event *_Nonnull event)
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
		AG_PopupHide(cons->pm);
	}
	if (cons->nLines > 0) {
		MapLine(cons, y, &cons->pos);
		cons->sel = 0;
		AG_Redraw(cons);
		cons->flags |= AG_CONSOLE_SELECTING;
	}
}

static void
CloseSelect(AG_Event *_Nonnull event)
{
	AG_Console *cons = AG_SELF();
	cons->flags &= ~(AG_CONSOLE_SELECTING);
}

static void
PopupMenu(AG_Event *_Nonnull event)
{
	AG_Console *cons = AG_SELF();
	int x = AG_INT(2);
	int y = AG_INT(3);
	AG_PopupMenu *pm;
	AG_MenuItem *mi;
		
	if (cons->flags & AG_CONSOLE_NOPOPUP)
		return;

	if (cons->pm != NULL) {
		AG_PopupShowAt(cons->pm, x, y);
		return;
	}
	if ((pm = cons->pm = AG_PopupNew(cons)) == NULL) {
		return;
	}
	mi = AG_MenuAction(pm->root, _("Copy"), NULL, MenuCopy, "%p", cons);
	mi->stateFn = AG_SetIntFn(pm->menu, NULL, MenuCopyActive, "%p", cons);

	AG_MenuAction(pm->root, _("Export to file..."), NULL,
	    MenuExportToFileDlg, "%p", cons);

	AG_MenuSeparator(pm->root);

	AG_MenuAction(pm->root, _("Select All"), NULL,
	    MenuSelectAll, "%p", cons);

	AG_PopupShowAt(pm, x, y);
}

static void
MouseMotion(AG_Event *_Nonnull event)
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
ComputeVisible(AG_Console *_Nonnull cons)
{
	cons->rVisible = (int)AG_Floor((float)(cons->r.h - (cons->padding << 1)) /
	                               (float)cons->lineskip);
	if (cons->rVisible > 0)
		cons->rVisible--;
}

static void
OnFontChange(AG_Event *_Nonnull event)
{
	AG_Console *cons = AG_SELF();
	Uint i;
	int j;

	cons->lineskip = WIDGET(cons)->font->lineskip + 1;
	cons->rOffs = 0;
	ComputeVisible(cons);

	for (i = 0; i < cons->nLines; i++) {
		AG_ConsoleLine *ln = cons->lines[i];

		for (j = 0; j < 2; j++) {
			if (ln->surface[j] != -1) {
				AG_WidgetUnmapSurface(cons, ln->surface[j]);
				ln->surface[j] = -1;
			}
		}
	}
}

static void
Init(void *_Nonnull obj)
{
	AG_Console *cons = obj;

	WIDGET(cons)->flags |= AG_WIDGET_FOCUSABLE|AG_WIDGET_USE_TEXT;

	cons->flags = 0;
	cons->padding = 4;
	cons->lines = NULL;
	cons->lineskip = 0;
	cons->nLines = 0;
	cons->xOffs = 0;
	cons->wMax = 0;
	cons->rOffs = 0;
	cons->rVisible = 0;
	cons->pm = NULL;
	cons->pos = -1;
	cons->sel = 0;
	cons->r.x = 0;
	cons->r.y = 0;
	cons->r.w = 0;
	cons->r.h = 0;
	cons->scrollTo = NULL;

	cons->vBar = AG_ScrollbarNew(cons, AG_SCROLLBAR_VERT,
	    AG_SCROLLBAR_NOAUTOHIDE|AG_SCROLLBAR_EXCL);
	AG_WidgetSetFocusable(cons->vBar, 0);
	AG_SetUint(cons->vBar, "min", 0);
	AG_BindUint(cons->vBar, "max", &cons->nLines);
	AG_BindUint(cons->vBar, "visible", &cons->rVisible);
	AG_BindUint(cons->vBar, "value", &cons->rOffs);

	cons->hBar = AG_ScrollbarNew(cons, AG_SCROLLBAR_HORIZ,
	    AG_SCROLLBAR_NOAUTOHIDE|AG_SCROLLBAR_EXCL);
	AG_WidgetSetFocusable(cons->hBar, 0);
	AG_SetInt(cons->hBar, "min", 0);
	AG_BindInt(cons->hBar, "max", &cons->wMax);
	AG_BindInt(cons->hBar, "visible", &WIDGET(cons)->w);
	AG_BindInt(cons->hBar, "value", &cons->xOffs);

	AG_ActionFn(cons, "BeginSelect", BeginSelect, NULL);
	AG_ActionFn(cons, "CloseSelect", CloseSelect, NULL);
	AG_ActionFn(cons, "PopupMenu",	PopupMenu, NULL);
	AG_ActionFn(cons, "ScrollUp",	ScrollUp, NULL);
	AG_ActionFn(cons, "ScrollDown",	ScrollDown, NULL);
	AG_ActionFn(cons, "PageUp",	PageUp, NULL);
	AG_ActionFn(cons, "PageDown",   PageDown, NULL);
	AG_ActionOnButtonDown(cons, AG_MOUSE_LEFT, "BeginSelect");
	AG_ActionOnButtonUp(cons, AG_MOUSE_LEFT, "CloseSelect");

	AG_ActionOnButtonDown(cons, AG_MOUSE_RIGHT, "PopupMenu");
	AG_ActionOnButtonDown(cons, AG_MOUSE_WHEELUP, "ScrollUp");
	AG_ActionOnButtonDown(cons, AG_MOUSE_WHEELDOWN, "ScrollDown");

	AG_ActionOnKey(cons, AG_KEY_UP, AG_KEYMOD_ANY, "ScrollUp");
	AG_ActionOnKey(cons, AG_KEY_DOWN, AG_KEYMOD_ANY, "ScrollDown");
	AG_ActionOnKey(cons, AG_KEY_PAGEUP, AG_KEYMOD_ANY, "PageUp");
	AG_ActionOnKey(cons, AG_KEY_PAGEDOWN, AG_KEYMOD_ANY, "PageDown");

	AG_SetEvent(cons, "mouse-motion", MouseMotion, NULL);
	AG_AddEvent(cons, "font-changed", OnFontChange, NULL);
	AG_AddEvent(cons, "widget-shown", OnFontChange, NULL);
#if 0
	AG_BindUint(cons, "nLines", &cons->nLines);
	AG_BindUint(cons, "rOffs", &cons->rOffs);
	AG_BindUint(cons, "rVisible", &cons->rVisible);
	AG_BindInt(cons, "pos", &cons->pos);
	AG_BindInt(cons, "sel", &cons->sel);
#endif
}

static void
SizeRequest(void *_Nonnull p, AG_SizeReq *_Nonnull r)
{
	AG_TextSize("XXXXXXXXXXXXXXXXXXXXXXXXX", &r->w, &r->h);
	r->h *= 2;
}

static int
SizeAllocate(void *_Nonnull p, const AG_SizeAlloc *_Nonnull a)
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
	
	cons->r.x = 0;
	cons->r.y = 0;
	cons->r.w = (a->w - aBar.w);
	cons->r.h = a->h;
	
	AG_WidgetSizeReq(cons->hBar, &rBar);
	aBar.x = 0;
	aBar.y = a->h - rBar.h;
	aBar.w = a->w - rBar.h;
	aBar.h = rBar.h;
	AG_WidgetSizeAlloc(cons->hBar, &aBar);
	
	ComputeVisible(cons);
	ClampVisible(cons);
	AdjustXoffs(cons);
	return (0);
}

static void
Draw(void *_Nonnull p)
{
	AG_Console *cons = p;
	AG_Surface *su;
	AG_Rect r;
	Uint lnIdx;
	int pos, sel;

	r.x = 0;
	r.y = 0;
	r.w = WIDTH(cons);
	r.h = HEIGHT(cons);
	AG_DrawRect(cons, &r, &WCOLOR(cons,AG_COLOR));

	if (cons->nLines == 0)
		goto out;

	if (cons->scrollTo != NULL) {
		cons->rOffs = *cons->scrollTo;
		cons->scrollTo = NULL;
		ClampVisible(cons);
	}
	AG_PushClipRect(cons, &cons->r);
	r.x = cons->padding - cons->xOffs;
	r.y = cons->padding;
	r.w = WIDGET(cons)->w - (cons->padding << 1);
	r.h = cons->lineskip + 1;
	pos = cons->pos;
	sel = cons->sel;

	for (lnIdx = cons->rOffs;
	     lnIdx < cons->nLines && r.y < WIDGET(cons)->h;
	     lnIdx++) {
		AG_ConsoleLine *ln = cons->lines[lnIdx];
		AG_Color *cTxt = &WCOLOR(cons,AG_TEXT_COLOR);
		int suIdx = 0;

		if (pos != -1) {
			if ((lnIdx == pos) ||
			    ((sel > 0 && lnIdx > pos && lnIdx < pos+sel+1) ||
			     (sel < 0 && lnIdx < pos && lnIdx > pos+sel-1))) {
				AG_DrawRectFilled(cons, &r,
				    &WCOLOR_SEL(cons,AG_COLOR));
				cTxt = &WCOLOR_SEL(cons,AG_TEXT_COLOR);
				suIdx = 1;
			}
		}
		if (ln->surface[suIdx] == -1) {
			AG_TextColor((ln->c.a != 0) ? &ln->c : cTxt);
			if ((su = AG_TextRender(ln->text)) == NULL) {
				continue;
			}
			ln->surface[suIdx] = AG_WidgetMapSurface(cons, su);
		}
		AG_WidgetBlitSurface(cons, ln->surface[suIdx], r.x, r.y);
		r.y += cons->lineskip;
	}
	AG_PopClipRect(cons);
out:
	AG_WidgetDraw(cons->vBar);
	AG_WidgetDraw(cons->hBar);
}

static void
FreeLines(AG_Console *_Nonnull cons)
{
	Uint i;
	
	for (i = 0; i < cons->nLines; i++) {
		AG_ConsoleLine *ln = cons->lines[i];
		free(ln->text);
		free(ln);
	}
	Free(cons->lines);
	cons->lines = NULL;
	cons->nLines = 0;
}

static void
Destroy(void *_Nonnull p)
{
	AG_Console *cons = p;

	if (cons->pm != NULL) {
		AG_PopupDestroy(cons->pm);
	}
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

/* Append a line to the console; backend to AG_ConsoleMsg(). */
AG_ConsoleLine *
AG_ConsoleAppendLine(AG_Console *cons, const char *s)
{
	AG_ConsoleLine *ln;
	
	ln = Malloc(sizeof(AG_ConsoleLine));

	AG_ObjectLock(cons);

	cons->lines = Realloc(cons->lines, (cons->nLines+1) *
	                                   sizeof(AG_ConsoleLine *));
	cons->lines[cons->nLines++] = ln;

	if (s != NULL) {
		ln->text = Strdup(s);
		ln->len = strlen(s);
	} else {
		ln->text = NULL;
		ln->len = 0;
	}
	ln->cons = cons;
	ln->p = NULL;
	ln->surface[0] = -1;
	ln->surface[1] = -1;
	AG_ColorNone(&ln->c);			/* Inherit default */

	if ((cons->flags & AG_CONSOLE_NOAUTOSCROLL) == 0) {
		cons->scrollTo = &cons->nLines;
	}
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

	AG_ObjectLock(cons);
	if ((ln = AG_ConsoleAppendLine(cons, NULL)) != NULL) {
		va_start(args, fmt);
		AG_Vasprintf(&ln->text, fmt, args);
		va_end(args);
		ln->len = strlen(ln->text);
		if (ln->len > 1 && ln->text[ln->len - 1] == '\n') {
			ln->text[ln->len - 1] = '\0';
			ln->len--;
		}
	}
	AG_ObjectUnlock(cons);
	return (ln);
}

/* Append a message to the console (C string). */
AG_ConsoleLine *
AG_ConsoleMsgS(AG_Console *cons, const char *s)
{
	AG_ConsoleLine *ln;
	
	AG_ObjectLock(cons);
	if ((ln = AG_ConsoleAppendLine(cons, s)) != NULL) {
		if (ln->len > 1 && ln->text[ln->len - 1] == '\n') {
			ln->text[ln->len - 1] = '\0';
			ln->len--;
		}
	}
	AG_ObjectUnlock(cons);
	return (ln);
}

/* Change the text of an existing line. */
void
AG_ConsoleMsgEdit(AG_ConsoleLine *ln, const char *s)
{
	AG_Console *cons = ln->cons;
	int i;

	AG_ObjectLock(cons);
	free(ln->text);
	ln->text = Strdup(s);
	ln->len = strlen(s);
	for (i = 0; i < 2; i++) {
		if (ln->surface[i] != -1) {
			AG_WidgetUnmapSurface(cons, ln->surface[i]);
			ln->surface[i] = -1;
		}
	}
	AG_Redraw(cons);
	AG_ObjectUnlock(cons);
}

void
AG_ConsoleMsgPtr(AG_ConsoleLine *ln, void *p)
{
	AG_ObjectLock(ln->cons);
	ln->p = p;
	AG_ObjectUnlock(ln->cons);
}

void
AG_ConsoleMsgColor(AG_ConsoleLine *ln, const AG_Color *c)
{
	AG_ObjectLock(ln->cons);
	memcpy(&ln->c, c, sizeof(AG_Color));
	AG_ObjectUnlock(ln->cons);
}

void
AG_ConsoleClear(AG_Console *cons)
{
	AG_ObjectLock(cons);
	FreeLines(cons);
	cons->rOffs = 0;
	cons->pos = -1;
	cons->sel = 0;
	AG_Redraw(cons);
	AG_ObjectUnlock(cons);
}

AG_WidgetClass agConsoleClass = {
	{
		"Agar(Widget:Console)",
		sizeof(AG_Console),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		Destroy,
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
