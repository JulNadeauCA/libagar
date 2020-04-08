/*
 * Copyright (c) 2003-2020 Julien Nadeau Carriere <vedge@csoft.net>
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

/*
 * Window titlebar widget. This is a simple subclass of AG_Box(3) which embeds
 * an AG_Label(3) and a set of AG_Button(3) for maximizing, minimizing and
 * closing windows. It is only used in single-window (built-in WM) mode.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/titlebar.h>
#include <agar/gui/window.h>
#include <agar/gui/primitive.h>
#include <agar/gui/icons.h>

static void
MaximizeWindow(AG_Event *_Nonnull event)
{
	const AG_Titlebar *tbar = AG_CONST_TITLEBAR_PTR(1);
	AG_Window *win = tbar->win;

	AG_ObjectLock(win);
	if (win->flags & AG_WINDOW_MAXIMIZED) {
		AG_ClearBackground();
		AG_WindowUnmaximize(win);
	} else {
		AG_WindowMaximize(win);
	}
	AG_ObjectUnlock(win);
}

static void
MinimizeWindow(AG_Event *_Nonnull event)
{
	const AG_Titlebar *tbar = AG_CONST_TITLEBAR_PTR(1);

	AG_WindowMinimize(tbar->win);
}

static void
CloseWindow(AG_Event *_Nonnull event)
{
	const AG_Titlebar *tbar = AG_CONST_TITLEBAR_PTR(1);
	AG_Window *win = tbar->win;

	AG_PostEvent(win, "window-close", NULL);
}

AG_Titlebar *
AG_TitlebarNew(void *parent, Uint flags)
{
	AG_Titlebar *tbar;
	AG_Button *btn;
	const Uint btnFlags = (AG_BUTTON_VFILL | AG_BUTTON_NO_FOCUS);

	tbar = Malloc(sizeof(AG_Titlebar));
	AG_ObjectInit(tbar, &agTitlebarClass);
	tbar->flags |= (flags & AG_TITLEBAR_SAVED_FLAGS);
	
	AG_ObjectAttach(parent, tbar);

	AG_ObjectLock(tbar);
	/*
	 * Manually update the window/driver pointers since AG_TitlebarNew()
	 * is called from the Window attach routine.
	 */
	tbar->win = AGWINDOW(parent);

	WIDGET(tbar)->window = tbar->win;
	WIDGET(tbar)->drv = WIDGET(parent)->drv;
	WIDGET(tbar)->drvOps = AGDRIVER_CLASS(WIDGET(tbar)->drv);
	
	if ((flags & AG_TITLEBAR_NO_MINIMIZE) == 0) {
		btn = AG_ButtonNewS(tbar, btnFlags, " _ ");
		AG_SetStyle(btn, "font-size", "80%");
		AG_ObjectSetNameS(btn, "minimize");
		AG_SetEvent(btn, "button-pushed", MinimizeWindow, "%Cp", tbar);
	}
	if ((flags & AG_TITLEBAR_NO_MAXIMIZE) == 0) {
		btn = AG_ButtonNewS(tbar, btnFlags, "\xE2\x96\xA2"); /* U+25A2 */
		AG_SetStyle(btn, "font-size", "80%");
		AG_ObjectSetNameS(btn, "maximize");
		AG_SetEvent(btn, "button-pushed", MaximizeWindow, "%Cp", tbar);
	}
	if ((flags & AG_TITLEBAR_NO_CLOSE) == 0) {
		btn = AG_ButtonNewS(tbar, btnFlags, "\xE2\x9C\x95"); /* U+2715 */
		AG_SetStyle(btn, "font-size", "80%");
		AG_ObjectSetNameS(btn, "close");
		AG_SetEvent(btn, "button-pushed", CloseWindow, "%Cp", tbar);
	}

	if ((flags & AG_TITLEBAR_NO_BUTTONS) == 0)
		AG_SetStyle(tbar->label, "padding", "2 0 0 2");

	AG_ObjectUnlock(tbar);
	return (tbar);
}

static void
MouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Titlebar *tbar = AG_TITLEBAR_SELF();

	tbar->flags |= AG_TITLEBAR_PRESSED;

	if (AGDRIVER_SINGLE(WIDGET(tbar)->drv))
		AG_WM_MoveBegin(tbar->win);

	AG_Redraw(tbar);
}

static void
MouseButtonUp(AG_Event *_Nonnull event)
{
	AG_Titlebar *tbar = AG_TITLEBAR_SELF();
	
	tbar->flags &= ~(AG_TITLEBAR_PRESSED);
	
	if (AGDRIVER_SINGLE(WIDGET(tbar)->drv))
		AG_WM_MoveEnd(tbar->win);
}

static void
Init(void *_Nonnull obj)
{
	AG_Titlebar *tbar = obj;

	WIDGET(tbar)->flags |= AG_WIDGET_HFILL |
	                       AG_WIDGET_UNFOCUSED_BUTTONUP;

	AG_BoxSetType(AGBOX(tbar), AG_BOX_HORIZ);

	tbar->flags = 0;
	tbar->win = NULL;
	tbar->label = AG_LabelNewS(tbar, AG_LABEL_HFILL, _("Untitled"));

	AG_SetEvent(tbar, "mouse-button-down", MouseButtonDown, NULL);
	AG_SetEvent(tbar, "mouse-button-up", MouseButtonUp, NULL);
}

static void
Draw(void *_Nonnull obj)
{
	AG_Titlebar *tbar = obj;
	AG_Window *win = tbar->win;
	const AG_Color *cFg;
	AG_Rect r = WIDGET(tbar)->r;
	
	cFg = AG_WindowIsFocused(win) ? &WCOLOR(tbar, FG_COLOR) :
	                                &WCOLOR_DISABLED(tbar, FG_COLOR);

	r.w++;

	if (tbar->flags & AG_TITLEBAR_PRESSED) {
		AG_DrawBoxSunk(win, &r, cFg);
	} else {
		AG_DrawBoxRaised(win, &r, cFg);
	}

	WIDGET_SUPER_OPS(tbar)->draw(tbar);
}

AG_WidgetClass agTitlebarClass = {
	{
		"Agar(Widget:Box:Titlebar)",
		sizeof(AG_Titlebar),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	NULL,			/* size_request */
	NULL,			/* size_allocate */
};

#endif /* AG_WIDGETS */
