/*
 * Copyright (c) 2002-2023 Julien Nadeau Carriere <vedge@csoft.net>
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
 * (an AG_Tlist(3) displayed in a separate window). Unlike AG_Combo(3),
 * it does not allow arbitrary text input.
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/ucombo.h>
#include <agar/gui/primitive.h>

static int agUcomboCounter = 0;

static void PanelSepPressed(AG_Event *_Nonnull);

AG_UCombo *
AG_UComboNew(void *parent, Uint flags)
{
	AG_UCombo *com;

	com = Malloc(sizeof(AG_UCombo));
	AG_ObjectInit(com, &agUComboClass);

	if (flags & AG_UCOMBO_HFILL) { WIDGET(com)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_UCOMBO_VFILL) { WIDGET(com)->flags |= AG_WIDGET_VFILL; }

	com->flags |= flags;

	AG_ObjectAttach(parent, com);
	return (com);
}

static void
ClosePanel(AG_UCombo *com, AG_Window *win)
{
	com->wSaved = WIDTH(win);
	com->hSaved = HEIGHT(win);
	AG_WindowHide(win);
	AG_SetInt(com->button, "state", 0);
}

static void
PanelWindowClose(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_SELF();
	AG_UCombo *com = AG_UCOMBO_PTR(1);

	ClosePanel(com, win);
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

	if ((x < 0 || y < 0 || x > WIDTH(win) || y > HEIGHT(win)))
		ClosePanel(com, win);
}

/* An item has been selected from the drop-down menu. */
static void
SelectedItem(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_UCombo *com = AG_UCOMBO_PTR(1);
	AG_TlistItem *it;
	AG_Window *win;

	AG_ObjectLock(com);

	AG_ObjectLock(tl);
	if ((it = AG_TlistSelectedItem(tl)) != NULL) {
		it->selected++;
		AG_ButtonTextS(com->button, it->text);
		AG_PostEvent(com, "ucombo-selected", "%p", it);
	}
	AG_ObjectUnlock(tl);

	if ((win = com->panel) != NULL)
		ClosePanel(com, win);

	AG_ObjectUnlock(com);
}

static void
PanelCancelPressed(AG_Event *event)
{
	AG_UCombo *com = AG_UCOMBO_PTR(1);
	AG_Window *panel;

	if ((panel = com->panel) != NULL)
		ClosePanel(com, panel);
}

static AG_Window *_Nullable
CreatePanel(AG_UCombo *com, Uint winFlags)
{
	AG_Window *win, *winParent = WIDGET(com)->window;
	AG_Button *btn;
	AG_Tlist *tl;

	if ((win = AG_WindowNew(winFlags)) == NULL) {
		return (NULL);
	}
	if (winFlags & AG_WINDOW_NOTITLE)
		win->wmType = AG_WINDOW_WM_COMBO;

	AG_ObjectSetName(win, "_ucombo%u", agUcomboCounter++);

	AG_SetPadding(win, "0"); /* TODO style */

	com->panel = win;

	if (winFlags & AG_WINDOW_NOTITLE) {
		btn = AG_ButtonNewFn(win, AG_BUTTON_HFILL, NULL,
		    PanelSepPressed, "%p", com);
		AG_SetPadding(btn, "5 0 5 0");  /* TODO style */
	}

	com->list = tl = AG_TlistNew(win, AG_TLIST_EXPAND |
	                                  AG_TLIST_NO_KEYREPEAT); 

	btn = AG_ButtonNewFn(win, AG_BUTTON_HFILL, _("Cancel"),
	    PanelCancelPressed, "%p", com);
	AG_SetPadding(btn, "0 4 3 4");  /* TODO style */

	if (com->flags & AG_UCOMBO_POLL)        { tl->flags |= AG_TLIST_POLL; }
	if (com->flags & AG_UCOMBO_SCROLLTOSEL) { tl->flags |= AG_TLIST_SCROLLTOSEL; }

	AG_SetEvent(tl, "tlist-changed", SelectedItem,"%p",com);

	AG_PostEvent(com, "ucombo-expanded", NULL);

	if (winParent) {
		AG_WindowAttach(winParent, win);
		AG_WindowMakeTransient(winParent, win);
/*		AG_WindowPin(winParent, win); */
	}

	WIDGET(win)->flags |= AG_WIDGET_UNFOCUSED_BUTTONDOWN;

	AG_SetEvent(win, "window-close", PanelWindowClose,"%p",com);
	AG_AddEvent(win, "mouse-button-down", PanelMouseButtonDown,"%p",com);

	return (win);
}

static void
ShowPanel(AG_UCombo *com, AG_Window *win)
{
	AG_Driver *drv = WIDGET(com)->drv;
	AG_Window *winParent = WIDGET(com)->window;
	AG_Tlist *tl;
	Uint wView, hView;
	int x,y, w,h;

	if (com->wSaved > 0) {
		w = com->wSaved;
		h = com->hSaved;
	} else if ((tl = com->list) != NULL) {
		AG_SizeReq rList;

		if (com->wPreList != -1 && com->hPreList != -1) {
			AG_TlistSizeHintPixels(tl, com->wPreList, com->hPreList);
		}
		AG_WidgetSizeReq(tl, &rList);
		w = rList.w + WIDGET(win)->paddingLeft + WIDGET(win)->paddingRight + (win->wBorderSide << 1);
		h = rList.h + WIDGET(win)->paddingTop + WIDGET(win)->paddingBottom +  win->wBorderBot;
	} else {
		return;
	}
	x = WIDGET(com)->rView.x2 - w;
	y = WIDGET(com)->rView.y1;
	AG_GetDisplaySize(drv, &wView, &hView);
	if (x + w > wView) { w = wView - x; }
	if (y + h > hView) { h = hView - y; }

	if (winParent != NULL && AGDRIVER_CLASS(drv)->wm == AG_WM_MULTIPLE) {
		x += WIDGET(winParent)->x;
		y += WIDGET(winParent)->y;
	}
	if (x < 0) { x = 0; }
	if (y < 0) { y = 0; }
	if (w < 4 || h < 4) {                                    /* Minimum */
		AG_PostEvent(com, "ucombo-collapsed", NULL);
		AG_ObjectDetach(win);
		com->list = NULL;
		com->panel = NULL;
		return;
	}
	com->wSaved = w;
	com->hSaved = h;

	AG_WindowSetMinSize(win, 50,50);     /* TODO style */
	AG_WindowSetGeometry(win, x,y, w,h);
	AG_WindowShow(win);
}

static void
PanelSepPressed(AG_Event *_Nonnull event)
{
	AG_UCombo *com = AG_UCOMBO_PTR(1);
	AG_Window *win;

	AG_ObjectLock(com);

	if ((win = com->panel) != NULL) {
		ClosePanel(com, win);
		AG_PostEvent(com, "ucombo-collapsed", NULL);
	}
	if ((win = CreatePanel(com, AG_WINDOW_KEEPABOVE | AG_WINDOW_NOMAXIMIZE |
	                            AG_WINDOW_NOMINIMIZE)) != NULL) {
		AG_WindowSetCaptionS(win, _("Menu"));
		AG_TlistClear(com->list);
		AG_PostEvent(com, "ucombo-expanded", NULL);
		ShowPanel(com, win);
	}

	AG_ObjectUnlock(com);
}

static void
ExpandButtonPushed(AG_Event *_Nonnull event)
{
	AG_UCombo *com = AG_UCOMBO_PTR(1);
	const int button_state = AG_INT(2);
	AG_Window *win;

	AG_ObjectLock(com);

	if (button_state == 0) {                                /* Collapse */
		if ((win = com->panel) != NULL) {
			ClosePanel(com, win);
			AG_PostEvent(com, "ucombo-collapsed", NULL);
		}
		goto out;
	}

	if (com->panel != NULL) {                          /* Cached window */
		win = com->panel;
		if (win->flags & AG_WINDOW_NOTITLE) {
			AG_TlistClear(com->list);
			AG_PostEvent(com, "ucombo-expanded", NULL);
			ShowPanel(com, win);
		} else {
			ClosePanel(com, win);
			AG_PostEvent(com, "ucombo-collapsed", NULL);

			win = CreatePanel(com, AG_WINDOW_MODAL |
			                       AG_WINDOW_KEEPABOVE |
			                       AG_WINDOW_NOTITLE);
			if (win != NULL) {
				AG_TlistClear(com->list);
				AG_PostEvent(com, "ucombo-expanded", NULL);
				ShowPanel(com, win);
			}
		}
	} else {                                              /* New window */
		win = CreatePanel(com, AG_WINDOW_MODAL | AG_WINDOW_KEEPABOVE |
		                       AG_WINDOW_NOTITLE);
		if (win != NULL) {
			AG_TlistClear(com->list);
			AG_PostEvent(com, "ucombo-expanded", NULL);
			ShowPanel(com, win);
		}

	}
out:
	AG_ObjectUnlock(com);
}

static void
Init(void *_Nonnull obj)
{
	AG_UCombo *com = obj;

	WIDGET(com)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP;

	com->flags = 0;
	com->wSaved = 0;
	com->hSaved = 0;
	com->wPreList = -1;
	com->hPreList = -1;

	com->button = AG_ButtonNewS(com, AG_BUTTON_STICKY |
	                                 AG_BUTTON_NO_FOCUS, _("..."));
	AG_ObjectSetNameS(com->button, "trigger");
	AG_SetPadding(com->button, "1");
	AG_WidgetForwardFocus(com, com->button);
	AG_SetEvent(com->button, "button-pushed", ExpandButtonPushed, "%p", com);

	com->list = NULL;
	com->panel = NULL;
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
		{ 1,0, AGC_UCOMBO, 0 },
		Init,
		NULL,		/* reset */
		NULL,		/* destroy */
		NULL,		/* load */
		NULL,		/* save */
		NULL		/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate,
	NULL,			/* mouse_button_down */
	NULL,			/* mouse_button_up */
	NULL,			/* mouse_motion */
	NULL,			/* key_down */
	NULL,			/* key_up */
	NULL,			/* touch */
	NULL,			/* ctrl */
	NULL			/* joy */
};

#endif /* AG_WIDGETS */
