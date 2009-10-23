/*
 * Copyright (c) 2003-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "titlebar.h"
#include "window.h"
#include "primitive.h"
#include "icons.h"

static void MouseButtonDown(AG_Event *);
static void MouseButtonUp(AG_Event *);
static void MaximizeWindow(AG_Event *);
static void MinimizeWindow(AG_Event *);
static void CloseWindow(AG_Event *);

static void
CreateMaximizeButton(AG_Titlebar *tbar)
{
	tbar->maximize_btn = AG_ButtonNew(tbar, 0, NULL);
	AG_ButtonSetJustification(tbar->maximize_btn, AG_TEXT_LEFT);
	AG_ButtonSetFocusable(tbar->maximize_btn, 0);
	AG_ButtonSurfaceNODUP(tbar->maximize_btn, agIconWinMaximize.s);
	AG_ButtonSetPadding(tbar->maximize_btn, 0,0,0,0);
	AG_SetEvent(tbar->maximize_btn, "button-pushed",
	    MaximizeWindow, "%p", tbar);
}

static void
CreateMinimizeButton(AG_Titlebar *tbar)
{
	tbar->minimize_btn = AG_ButtonNew(tbar, 0, NULL);
	AG_ButtonSetJustification(tbar->minimize_btn, AG_TEXT_LEFT);
	AG_ButtonSetFocusable(tbar->minimize_btn, 0);
	AG_ButtonSurfaceNODUP(tbar->minimize_btn, agIconWinMinimize.s);
	AG_ButtonSetPadding(tbar->minimize_btn, 0,0,0,0);
	AG_SetEvent(tbar->minimize_btn, "button-pushed",
	    MinimizeWindow, "%p", tbar);
}

static void
CreateCloseButton(AG_Titlebar *tbar)
{
	tbar->close_btn = AG_ButtonNew(tbar, 0, NULL);
	AG_ButtonSetJustification(tbar->close_btn, AG_TEXT_LEFT);
	AG_ButtonSetFocusable(tbar->close_btn, 0);
	AG_ButtonSurfaceNODUP(tbar->close_btn, agIconWinClose.s);
	AG_ButtonSetPadding(tbar->close_btn, 0,0,0,0);
	AG_SetEvent(tbar->close_btn, "button-pushed",
	    CloseWindow, "%p", tbar);
}

AG_Titlebar *
AG_TitlebarNew(void *parent, Uint flags)
{
	AG_Titlebar *tbar;

	tbar = Malloc(sizeof(AG_Titlebar));
	AG_ObjectInit(tbar, &agTitlebarClass);
	tbar->flags |= flags;
	
	if ((flags & AG_TITLEBAR_NO_MAXIMIZE) == 0)
		CreateMaximizeButton(tbar);
	if ((flags & AG_TITLEBAR_NO_MINIMIZE) == 0)
		CreateMinimizeButton(tbar);
	if ((flags & AG_TITLEBAR_NO_CLOSE) == 0)
		CreateCloseButton(tbar);

	AG_ObjectAttach(parent, tbar);
	AG_ObjectLock(tbar);
	tbar->win = (AG_Window *)parent;
	AG_ObjectUnlock(tbar);
	return (tbar);
}

static void
MaximizeWindow(AG_Event *event)
{
	AG_Titlebar *tbar = AG_PTR(1);
	AG_Window *win = tbar->win;

	AG_ObjectLock(win);
	if (win->flags & AG_WINDOW_MAXIMIZED) {
		AG_WindowUnmaximize(win);
	} else {
		AG_WindowMaximize(win);
	}
	AG_ObjectUnlock(win);
}

static void
MinimizeWindow(AG_Event *event)
{
	AG_Titlebar *tbar = AG_PTR(1);

	AG_WindowMinimize(tbar->win);
}

static void
CloseWindow(AG_Event *event)
{
	AG_Titlebar *tbar = AG_PTR(1);

	AG_PostEvent(NULL, tbar->win, "window-close", NULL);
}

static void
Init(void *obj)
{
	AG_Titlebar *tbar = obj;
	AG_Box *box = obj;

	WIDGET(tbar)->flags |= AG_WIDGET_HFILL|
	                       AG_WIDGET_UNFOCUSED_BUTTONUP;

	AG_BoxSetType(box, AG_BOX_HORIZ);
	AG_BoxSetPadding(&tbar->hb, 3);
	AG_BoxSetSpacing(&tbar->hb, 1);

	tbar->flags = 0;
	tbar->pressed = 0;
	tbar->win = NULL;
	tbar->maximize_btn = NULL;
	tbar->minimize_btn = NULL;
	tbar->close_btn = NULL;
	
	tbar->label = AG_LabelNewStaticString(tbar,
	    AG_LABEL_HFILL|AG_LABEL_NOMINSIZE, _("Untitled"));
	AG_LabelSizeHint(tbar->label, 1, "X");
	AG_LabelSetPadding(tbar->label, 5,5,2,2);

	AG_SetEvent(tbar, "window-mousebuttondown", MouseButtonDown, NULL);
	AG_SetEvent(tbar, "window-mousebuttonup", MouseButtonUp, NULL);
}

static void
Draw(void *obj)
{
	AG_Titlebar *tbar = obj;

	STYLE(tbar)->TitlebarBackground(tbar, tbar->pressed,
	    AG_WINDOW_FOCUSED(tbar->win));
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Titlebar *tbar = AG_SELF();

	tbar->pressed = 1;

	agView->winop = AG_WINOP_MOVE;
	agView->winToFocus = tbar->win;
	agView->winSelected = tbar->win;
}

static void
MouseButtonUp(AG_Event *event)
{
	AG_Titlebar *tbar = AG_SELF();
	
	tbar->pressed = 0;
	
	agView->winop = AG_WINOP_NONE;
	agView->winSelected = NULL;
}

void
AG_TitlebarSetCaption(AG_Titlebar *tbar, const char *caption)
{
	AG_ObjectLock(tbar);
	AG_LabelText(tbar->label, (caption == NULL) ? "" : caption);
	AG_ObjectUnlock(tbar);
}

AG_WidgetClass agTitlebarClass = {
	{
		"AG_Widget:AG_Box:AG_Titlebar",
		sizeof(AG_Titlebar),
		{ 0,0 },
		Init,
		NULL,		/* free */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	AG_BoxSizeRequest,
	AG_BoxSizeAllocate
};