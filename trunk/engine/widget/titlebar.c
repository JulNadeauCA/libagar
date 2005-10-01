/*	$Csoft: titlebar.c,v 1.26 2005/09/27 00:25:24 vedge Exp $	*/

/*
 * Copyright (c) 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#include <engine/engine.h>
#include <engine/view.h>

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>

#include <string.h>

#include "titlebar.h"

const AG_WidgetOps agTitlebarOps = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		AG_BoxDestroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	AG_TitlebarDraw,
	AG_BoxScale
};

static void titlebar_mousebuttondown(int, union evarg *);
static void titlebar_mousebuttonup(int, union evarg *);

AG_Titlebar *
AG_TitlebarNew(void *parent, int flags)
{
	AG_Titlebar *tbar;

	tbar = Malloc(sizeof(AG_Titlebar), M_OBJECT);
	AG_TitlebarInit(tbar, flags);
	AG_ObjectAttach(parent, tbar);
	tbar->win = (AG_Window *)parent;
	return (tbar);
}

static void
maximize_window(int argc, union evarg *argv)
{
	AG_Titlebar *tbar = argv[1].p;
	AG_Window *win = tbar->win;

	if (win->flags & AG_WINDOW_MAXIMIZED) {
		AG_WindowSetGeometry(win, win->savx, win->savy, win->savw,
		    win->savh);
		win->flags &= ~(AG_WINDOW_MAXIMIZED);
		if (!agView->opengl) {
			SDL_FillRect(agView->v, NULL, AG_COLOR(BG_COLOR));
			SDL_UpdateRect(agView->v, 0, 0, agView->v->w,
			    agView->v->h);
		}
	} else {
		win->savx = AGWIDGET(win)->x;
		win->savy = AGWIDGET(win)->y;
		win->savw = AGWIDGET(win)->w;
		win->savh = AGWIDGET(win)->h;
		AG_WindowSetGeometry(win, 0, 0, agView->w, agView->h);
		win->flags |= AG_WINDOW_MAXIMIZED;
	}
}

static void
minimize_window(int argc, union evarg *argv)
{
	AG_Titlebar *tbar = argv[1].p;
	AG_Window *win = tbar->win;

	win->flags |= AG_WINDOW_ICONIFIED;
	AG_WindowHide(win);
}

static void
close_window(int argc, union evarg *argv)
{
	AG_Titlebar *tbar = argv[1].p;

	AG_PostEvent(NULL, tbar->win, "window-close", NULL);
}

void
AG_TitlebarInit(AG_Titlebar *tbar, int flags)
{
	AG_BoxInit(&tbar->hb, AG_BOX_HORIZ, AG_BOX_WFILL);
	AG_ObjectSetOps(tbar, &agTitlebarOps);
	AG_WireGfx(tbar, "/engine/widget/pixmaps");

	AG_BoxSetPadding(&tbar->hb, 5);
	AG_BoxSetSpacing(&tbar->hb, 0);

	AG_WidgetSetType(tbar, "titlebar");
	AGWIDGET(tbar)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP;

	tbar->flags = flags;
	tbar->pressed = 0;
	tbar->win = NULL;
	tbar->label = AG_LabelNew(tbar, AG_LABEL_STATIC, _("Untitled"));
	AGWIDGET(tbar->label)->flags |= AG_WIDGET_WFILL;
	
	if ((flags & AG_TITLEBAR_NO_MAXIMIZE) == 0) {
		tbar->maximize_btn = AG_ButtonNew(tbar, NULL);
		AG_ButtonSetFocusable(tbar->maximize_btn, 0);
		AG_ButtonSetSurface(tbar->maximize_btn,
		    AG_SPRITE(tbar,AG_TITLEBAR_MAXIMIZE_ICON).su);
		AG_ButtonSetPadding(tbar->maximize_btn, 1);
		AG_SetEvent(tbar->maximize_btn, "button-pushed",
		    maximize_window, "%p", tbar);
	} else {
		tbar->maximize_btn = NULL;
	}

	if ((flags & AG_TITLEBAR_NO_MINIMIZE) == 0) {
		tbar->minimize_btn = AG_ButtonNew(tbar, NULL);
		AG_ButtonSetFocusable(tbar->minimize_btn, 0);
		AG_ButtonSetSurface(tbar->minimize_btn,
		    AG_SPRITE(tbar,AG_TITLEBAR_MINIMIZE_ICON).su);
		AG_ButtonSetPadding(tbar->minimize_btn, 1);
		AG_SetEvent(tbar->minimize_btn, "button-pushed",
		    minimize_window, "%p", tbar);
	} else {
		tbar->minimize_btn = NULL;
	}

	if ((flags & AG_TITLEBAR_NO_CLOSE) == 0) {
		tbar->close_btn = AG_ButtonNew(tbar, NULL);
		AG_ButtonSetFocusable(tbar->close_btn, 0);
		AG_ButtonSetSurface(tbar->close_btn,
		    AG_SPRITE(tbar,AG_TITLEBAR_CLOSE_ICON).su);
		AG_ButtonSetPadding(tbar->close_btn, 1);
		AG_SetEvent(tbar->close_btn, "button-pushed", close_window,
		    "%p", tbar);
	} else {
		tbar->close_btn = NULL;
	}

	AG_SetEvent(tbar, "window-mousebuttondown", titlebar_mousebuttondown,
	    NULL);
	AG_SetEvent(tbar, "window-mousebuttonup", titlebar_mousebuttonup, NULL);
}

void
AG_TitlebarDraw(void *p)
{
	AG_Titlebar *tbar = p;

	agPrim.box(tbar,
	    0,
	    0,
	    AGWIDGET(tbar)->w,
	    AGWIDGET(tbar)->h,
	    tbar->pressed ? -1 : 1,
	    AG_WINDOW_FOCUSED(tbar->win) ? AG_COLOR(TITLEBAR_FOCUSED_COLOR) :
	                                AG_COLOR(TITLEBAR_UNFOCUSED_COLOR));
}

static void
titlebar_mousebuttondown(int argc, union evarg *argv)
{
	AG_Titlebar *tbar = argv[0].p;

	tbar->pressed = 1;

	pthread_mutex_lock(&agView->lock);
	agView->winop = AG_WINOP_MOVE;
	agView->focus_win = tbar->win;
	agView->wop_win = tbar->win;
	pthread_mutex_unlock(&agView->lock);
}

static void
titlebar_mousebuttonup(int argc, union evarg *argv)
{
	AG_Titlebar *tbar = argv[0].p;
	
	tbar->pressed = 0;
	
	pthread_mutex_lock(&agView->lock);
	agView->winop = AG_WINOP_NONE;
	agView->wop_win = NULL;
	pthread_mutex_unlock(&agView->lock);
}

void
AG_TitlebarSetCaption(AG_Titlebar *tbar, const char *caption)
{
	AG_LabelSetSurface(tbar->label, (caption == NULL) ? NULL :
	    AG_TextRender(NULL, -1, AG_COLOR(TITLEBAR_CAPTION_COLOR), caption));
}
