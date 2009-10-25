/*
 * Copyright (c) 2003-2009 Hypertriton, Inc. <http://hypertriton.com/>
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
CreateMaximizeButton(AG_Titlebar *tbar)
{
	tbar->maximize_btn = AG_ButtonNewS(tbar, 0, NULL);
	AG_ButtonJustify(tbar->maximize_btn, AG_TEXT_LEFT);
	AG_ButtonSetFocusable(tbar->maximize_btn, 0);
	AG_ButtonSurfaceNODUP(tbar->maximize_btn, agIconWinMaximize.s);
	AG_ButtonSetPadding(tbar->maximize_btn, 0,0,0,0);
	AG_SetEvent(tbar->maximize_btn, "button-pushed",
	    MaximizeWindow, "%p", tbar);
}

static void
MinimizeWindow(AG_Event *event)
{
	AG_Titlebar *tbar = AG_PTR(1);

	AG_WindowMinimize(tbar->win);
}

static void
CreateMinimizeButton(AG_Titlebar *tbar)
{
	tbar->minimize_btn = AG_ButtonNewS(tbar, 0, NULL);
	AG_ButtonJustify(tbar->minimize_btn, AG_TEXT_LEFT);
	AG_ButtonSetFocusable(tbar->minimize_btn, 0);
	AG_ButtonSurfaceNODUP(tbar->minimize_btn, agIconWinMinimize.s);
	AG_ButtonSetPadding(tbar->minimize_btn, 0,0,0,0);
	AG_SetEvent(tbar->minimize_btn, "button-pushed",
	    MinimizeWindow, "%p", tbar);
}

static void
CloseWindow(AG_Event *event)
{
	AG_Titlebar *tbar = AG_PTR(1);

	AG_PostEvent(NULL, tbar->win, "window-close", NULL);
}

static void
CreateCloseButton(AG_Titlebar *tbar)
{
	tbar->close_btn = AG_ButtonNewS(tbar, 0, NULL);
	AG_ButtonJustify(tbar->close_btn, AG_TEXT_LEFT);
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
	
	AG_ObjectAttach(parent, tbar);

	/*
	 * Manually update the window/driver pointers since AG_TitlebarNew()
	 * is called from the Window attach routine.
	 */
	AG_ObjectLock(tbar);
	tbar->win = (AG_Window *)parent;
	WIDGET(tbar)->window = tbar->win;
	WIDGET(tbar)->drv = WIDGET(parent)->drv;
	WIDGET(tbar)->drvOps = AGDRIVER_CLASS(WIDGET(tbar)->drv);
	AG_ObjectUnlock(tbar);
	
	if ((flags & AG_TITLEBAR_NO_MAXIMIZE) == 0)
		CreateMaximizeButton(tbar);
	if ((flags & AG_TITLEBAR_NO_MINIMIZE) == 0)
		CreateMinimizeButton(tbar);
	if ((flags & AG_TITLEBAR_NO_CLOSE) == 0)
		CreateCloseButton(tbar);

	return (tbar);
}

static void
MouseButtonDown(AG_Event *event)
{
	AG_Titlebar *tbar = AG_SELF();
	
	tbar->pressed = 1;

	if (AGDRIVER_SINGLE(WIDGET(tbar)->drv)) {
		AG_DriverSw *dsw = (AG_DriverSw *)WIDGET(tbar)->drv;
		agWindowToFocus = tbar->win;
		dsw->winSelected = tbar->win;
		if (!(tbar->win->flags & AG_WINDOW_NOMOVE))
			dsw->winop = AG_WINOP_MOVE;
	}
}

static void
MouseButtonUp(AG_Event *event)
{
	AG_Titlebar *tbar = AG_SELF();
	AG_DriverSw *dsw = (AG_DriverSw *)WIDGET(tbar)->drv;
	
	tbar->pressed = 0;
	
	if (AGDRIVER_SINGLE(dsw)) {
		dsw->winop = AG_WINOP_NONE;
		dsw->winSelected = NULL;
	}
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
	
	tbar->label = AG_LabelNewS(tbar, AG_LABEL_HFILL|AG_LABEL_NOMINSIZE,
	    _("Untitled"));
	AG_LabelSizeHint(tbar->label, 1, "X");
	AG_LabelSetPadding(tbar->label, 5,5,2,2);

	AG_SetEvent(tbar, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(tbar, "mouse-button-up", MouseButtonUp, NULL);

#ifdef AG_DEBUG
	AG_BindUint(tbar, "flags", &tbar->flags);
	AG_BindInt(tbar, "pressed", &tbar->pressed);
#endif
}

static void
Draw(void *obj)
{
	AG_Titlebar *tbar = obj;

	STYLE(tbar)->TitlebarBackground(tbar, tbar->pressed,
	    AG_WindowIsFocused(tbar->win));
	WIDGET_SUPER_OPS(tbar)->draw(tbar);
}

AG_WidgetClass agTitlebarClass = {
	{
		"Agar(Widget:Box:Titlebar)",
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
	AG_WidgetInheritSizeRequest,
	AG_WidgetInheritSizeAllocate
};
