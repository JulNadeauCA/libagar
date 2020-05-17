/*
 * Copyright (c) 2002-2020 Julien Nadeau Carriere <vedge@csoft.net>
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
 * Button-style combo box widget. The button activates a drop-down menu
 * (an AG_Tlist(3) displayed in a separate window).
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/ucombo.h>
#include <agar/gui/primitive.h>

static int agUcomboCounter = 0;

AG_UCombo *
AG_UComboNew(void *parent, Uint flags)
{
	AG_UCombo *com;

	com = Malloc(sizeof(AG_UCombo));
	AG_ObjectInit(com, &agUComboClass);

	if (flags & AG_UCOMBO_POLL)        { com->list->flags |= AG_TLIST_POLL; }
	if (flags & AG_UCOMBO_SCROLLTOSEL) { com->list->flags |= AG_TLIST_SCROLLTOSEL; }
	if (flags & AG_UCOMBO_HFILL)       { WIDGET(com)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_UCOMBO_VFILL)       { WIDGET(com)->flags |= AG_WIDGET_VFILL; }

	com->flags |= flags;

	AG_ObjectAttach(parent, com);
	return (com);
}

AG_UCombo *
AG_UComboNewPolled(void *parent, Uint flags, AG_EventFn fn, const char *fmt,
    ...)
{
	AG_UCombo *com;
	AG_Event *ev;

	com = AG_UComboNew(parent, flags);
	AG_ObjectLock(com);
	com->list->flags |= AG_TLIST_POLL;
	ev = AG_SetEvent(com->list, "tlist-poll", fn, NULL);
	if (fmt) {
		va_list ap;

		va_start(ap, fmt);
		AG_EventGetArgs(ev, fmt, ap);
		va_end(ap);
	}
	AG_ObjectUnlock(com);
	return (com);
}

static void
PanelWindowClose(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_SELF();
	AG_UCombo *com = AG_UCOMBO_PTR(1);

	com->wSaved = WIDTH(win);
	com->hSaved = HEIGHT(win);
	AG_WindowHide(win);
	AG_SetInt(com->button, "state", 0);
}

static void
PanelMouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_SELF();
	AG_UCombo *com = AG_UCOMBO_PTR(1);
	const int x = AG_INT(2);
	const int y = AG_INT(3);

	if (com->panel == NULL)
		return;

	if ((x < 0 || y < 0 || x > WIDTH(win) || y > HEIGHT(win))) {
		com->wSaved = WIDTH(win);
		com->hSaved = HEIGHT(win);
		AG_WindowHide(win);
		AG_SetInt(com->button, "state", 0);
	}
}

static void
ExpandButtonPushed(AG_Event *_Nonnull event)
{
	AG_UCombo *com = AG_UCOMBO_PTR(1);
	AG_Driver *drv = WIDGET(com)->drv;
	AG_Window *panel, *winParent = WIDGET(com)->window;
	const int button_state = AG_INT(2);
	AG_SizeReq rList;
	int x, y, w, h;
	Uint wView, hView;

	AG_ObjectLock(com);
	if (button_state) {                                       /* Expand */
		if (com->panel) {
			panel = com->panel;
		} else {
			if ((panel = AG_WindowNew(AG_WINDOW_MODAL |
			                          AG_WINDOW_NOTITLE)) == NULL) {
				return;
			}
			panel->wmType = AG_WINDOW_WM_COMBO;

			AG_ObjectSetName(panel, "_ucombo%u", agUcomboCounter++);
			AG_SetStyle(panel, "padding", "0");
			com->panel = panel;
			AG_ObjectAttach(panel, com->list);

			if (winParent) {
				AG_WindowAttach(WIDGET(com)->window, panel);
				AG_WindowMakeTransient(WIDGET(com)->window, panel);
/*				AG_WindowPin(WIDGET(com)->window, panel); */
			}

			WIDGET(panel)->flags |= AG_WIDGET_UNFOCUSED_BUTTONDOWN;

			AG_SetEvent(panel, "window-close", PanelWindowClose, "%p", com);
			AG_AddEvent(panel, "mouse-button-down", PanelMouseButtonDown, "%p", com);
		}
	
		if (com->wSaved > 0) {
			w = com->wSaved;
			h = com->hSaved;
		} else {
			if (com->wPreList != -1 && com->hPreList != -1) {
				AG_TlistSizeHintPixels(com->list,
				    com->wPreList, com->hPreList);
			}
			AG_WidgetSizeReq(com->list, &rList);
			w = rList.w + (panel->wBorderSide << 1);
			h = rList.h + panel->wBorderBot;
		}
		x = WIDGET(com)->rView.x2 - w;
		y = WIDGET(com)->rView.y1;
		AG_GetDisplaySize(WIDGET(com)->drv, &wView, &hView);
		if (x+w > wView) { w = wView - x; }
		if (y+h > hView) { h = hView - y; }
		if (winParent && AGDRIVER_CLASS(drv)->wm == AG_WM_MULTIPLE) {
			x += WIDGET(winParent)->x;
			y += WIDGET(winParent)->y;
		}
		if (x < 0) { x = 0; }
		if (y < 0) { y = 0; }
		if (w < 4 || h < 4) {
			AG_ObjectDetach(panel);
			com->panel = NULL;
			return;
		}
		com->wSaved = w;
		com->hSaved = h;
		AG_WindowSetGeometry(panel, x,y, w,h);
		AG_WindowShow(panel);
	} else {                                                /* Collapse */
		if ((panel = com->panel) != NULL) {
			com->wSaved = WIDTH(panel);
			com->hSaved = HEIGHT(panel);
			AG_WindowHide(panel);
			AG_SetInt(com->button, "state", 0);
		}
	}
	AG_ObjectUnlock(com);
}

static void
SelectedItem(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_UCombo *com = AG_UCOMBO_PTR(1);
	AG_TlistItem *it;
	AG_Window *panel;

	AG_ObjectLock(com);

	AG_ObjectLock(tl);
	if ((it = AG_TlistSelectedItem(tl)) != NULL) {
		it->selected++;
		AG_ButtonTextS(com->button, it->text);
		AG_PostEvent(com, "ucombo-selected", "%p", it);
	}
	AG_ObjectUnlock(tl);

	if ((panel = com->panel) != NULL) {
		com->wSaved = WIDTH(panel);
		com->hSaved = HEIGHT(panel);
		AG_WindowHide(panel);
		AG_SetInt(com->button, "state", 0);
	}

	AG_ObjectUnlock(com);
}

static void
Init(void *_Nonnull obj)
{
	AG_UCombo *com = obj;
	AG_Tlist *tl;

	WIDGET(com)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP;

	com->flags = 0;
	com->panel = NULL;
	com->wSaved = 0;
	com->hSaved = 0;
	com->wPreList = -1;
	com->hPreList = -1;

	com->button = AG_ButtonNewS(com, AG_BUTTON_STICKY |
	                                 AG_BUTTON_NO_FOCUS, _("..."));
	AG_SetStyle(com->button, "padding", "1");
	AG_WidgetForwardFocus(com, com->button);
	AG_SetEvent(com->button, "button-pushed", ExpandButtonPushed, "%p", com);
	
	tl = com->list = Malloc(sizeof(AG_Tlist));
	AG_ObjectInit(tl, &agTlistClass);
	WIDGET(tl)->flags |= AG_WIDGET_EXPAND;
	AG_SetEvent(tl, "tlist-changed", SelectedItem, "%p", com);
}

void
AG_UComboSizeHint(AG_UCombo *com, const char *text, int h)
{
	AG_OBJECT_ISA(com, "AG_Widget:AG_UCombo:*");
	AG_ObjectLock(com);

	AG_TextSize(text, &com->wPreList, NULL);
	com->hPreList = h;

	AG_ObjectUnlock(com);
}

void
AG_UComboSizeHintPixels(AG_UCombo *com, int w, int h)
{
	AG_OBJECT_ISA(com, "AG_Widget:AG_UCombo:*");
	com->wPreList = w;
	com->hPreList = h;
}

static void
Draw(void *_Nonnull obj)
{
	AG_UCombo *com = obj;

	AG_WidgetDraw(com->button);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_UCombo *com = obj;
	AG_SizeReq rButton;

	AG_WidgetSizeReq(com->button, &rButton);
	r->w = WIDGET(com)->paddingLeft + rButton.w + WIDGET(com)->paddingRight;
	r->h = WIDGET(com)->paddingTop + rButton.h + WIDGET(com)->paddingBottom;
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_UCombo *com = obj;
	AG_SizeAlloc aButton;

	aButton.x = 0;
	aButton.y = 0;
	aButton.w = a->w;
	aButton.h = a->h;
	AG_WidgetSizeAlloc(com->button, &aButton);
	return (0);
}

AG_WidgetClass agUComboClass = {
	{
		"Agar(Widget:UCombo)",
		sizeof(AG_UCombo),
		{ 0,0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};

#endif /* AG_WIDGETS */
