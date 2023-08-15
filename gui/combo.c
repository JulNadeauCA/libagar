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
 * Combo box widget. It embeds an AG_Textbox(3) and a button which activates
 * a drop-down menu (an AG_Tlist(3) displayed in a separate window).
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/combo.h>
#include <agar/gui/primitive.h>

static int agComboCounter = 0;

static void PanelSepPressed(AG_Event *_Nonnull);

AG_Combo *
AG_ComboNew(void *parent, Uint flags, const char *fmt, ...)
{
	AG_Combo *com;
	char *s;
	va_list ap;

	if (fmt != NULL) {
		va_start(ap, fmt);
		Vasprintf(&s, fmt, ap);
		va_end(ap);
		com = AG_ComboNewS(parent, flags, s);
		free(s);
	} else {
		com = AG_ComboNewS(parent, flags, NULL);
	}
	return (com);
}

AG_Combo *
AG_ComboNewFn(void *parent, Uint flags, const char *label, AG_EventFn fn,
    const char *fmt, ...)
{
	AG_Combo *com;
	AG_Event *ev;
	va_list ap;

	com = AG_ComboNewS(parent, flags, label);

	ev = AG_SetEvent(com, "combo-expanded", fn, NULL);
	va_start(ap, fmt);
	AG_EventGetArgs(ev, fmt, ap);
	va_end(ap);

	return (com);
}

AG_Combo *
AG_ComboNewS(void *parent, Uint flags, const char *label)
{
	AG_Combo *com;

	com = Malloc(sizeof(AG_Combo));
	AG_ObjectInit(com, &agComboClass);
	com->flags |= flags;

	if (label != NULL) {
		AG_TextboxSetLabelS(com->tbox, label);
	}
	if (flags & AG_COMBO_POLL)        { com->list->flags |= AG_TLIST_POLL; }
	if (flags & AG_COMBO_SCROLLTOSEL) { com->list->flags |= AG_TLIST_SCROLLTOSEL; }
	if (flags & AG_COMBO_HFILL)       { WIDGET(com)->flags |= AG_WIDGET_HFILL; }
	if (flags & AG_COMBO_VFILL)       { WIDGET(com)->flags |= AG_WIDGET_VFILL; }
	
	AG_ObjectAttach(parent, com);
	return (com);
}

static __inline__ void
ClosePanel(AG_Combo *com, AG_Window *win)
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
	AG_Combo *com = AG_COMBO_PTR(1);

	ClosePanel(com, win);
}

static void
PanelMouseButtonDown(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_SELF();
	AG_Combo *com = AG_COMBO_PTR(1);
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
	AG_Combo *com = AG_COMBO_PTR(1);
	AG_TlistItem *ti = AG_TLISTITEM_PTR(2);
	AG_Window *panel;

	AG_ObjectLock(com);

	AG_ObjectLock(tl);
	AG_TextboxSetString(com->tbox, ti->text);
	AG_PostEvent(com, "combo-selected", "%p", ti);
	AG_ObjectUnlock(tl);

	if ((panel = com->panel) != NULL)
		ClosePanel(com, panel);

	AG_ObjectUnlock(com);
}

static void
PanelCancelPressed(AG_Event *event)
{
	AG_Combo *com = AG_COMBO_PTR(1);
	AG_Window *panel;

	if ((panel = com->panel) != NULL)
		ClosePanel(com, panel);
}

static AG_Window *_Nullable
CreatePanel(AG_Combo *com, Uint winFlags)
{
	AG_Window *win, *winParent = WIDGET(com)->window;
	AG_Button *btn;
	AG_Tlist *tl;

	if ((win = AG_WindowNew(winFlags)) == NULL) {
		return (NULL);
	}
	if (winFlags & AG_WINDOW_NOTITLE)
		win->wmType = AG_WINDOW_WM_COMBO;

	AG_ObjectSetName(win, "_combo%u", agComboCounter++);

	AG_SetPadding(win, "0"); /* TODO style */

	com->panel = win;

	if (winFlags & AG_WINDOW_NOTITLE) {
		btn = AG_ButtonNewFn(win, AG_BUTTON_HFILL, NULL,
		    PanelSepPressed, "%p", com);
		AG_SetPadding(btn, "5 0 5 0");  /* TODO style */
	}

	com->listExp = tl = AG_TlistNew(win, AG_TLIST_EXPAND |
	                                     AG_TLIST_NO_KEYREPEAT); 
	if (com->flags & AG_COMBO_POLL)        { tl->flags |= AG_TLIST_POLL; }
	if (com->flags & AG_COMBO_SCROLLTOSEL) { tl->flags |= AG_TLIST_SCROLLTOSEL; }
	AG_SetEvent(tl, "tlist-selected", SelectedItem, "%p", com);
	
	AG_TlistCopy(tl, com->list);
	AG_TlistSizeHintLargest(tl, com->nVisItems);

	btn = AG_ButtonNewFn(win, AG_BUTTON_HFILL, _("Cancel"),
	    PanelCancelPressed, "%p", com);
	AG_SetPadding(btn, "0 4 3 4");  /* TODO style */

	AG_TlistClear(com->list);
	AG_PostEvent(com, "combo-expanded", NULL);

	if (winParent != NULL) {
		AG_WindowAttach(winParent, win);
		AG_WindowMakeTransient(winParent, win);
/*		AG_WindowPin(winParent, win); */
	}

	WIDGET(win)->flags |= AG_WIDGET_UNFOCUSED_BUTTONDOWN;

	AG_SetEvent(win, "window-close", PanelWindowClose, "%p", com);
	AG_AddEvent(win, "mouse-button-down", PanelMouseButtonDown, "%p", com);

	return (win);
}

static void
ShowPanel(AG_Combo *com, AG_Window *win)
{
	AG_Driver *drv = WIDGET(com)->drv;
	AG_Window *winParent = WIDGET(com)->window;
	AG_Tlist *tl;
	Uint wView, hView;
	int x,y, w,h;

	if (com->wSaved > 0) {
		w = com->wSaved;
		h = com->hSaved;
	} else if ((tl = com->listExp) != NULL) {
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
		AG_PostEvent(com, "combo-collapsed", NULL);
		AG_ObjectDetach(win);
		com->listExp = NULL;
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
	AG_Combo *com = AG_COMBO_PTR(1);
	AG_Window *win;

	AG_ObjectLock(com);

	if ((win = com->panel) != NULL) {
		ClosePanel(com, win);
		AG_PostEvent(com, "combo-collapsed", NULL);
	}
	if ((win = CreatePanel(com, AG_WINDOW_KEEPABOVE | AG_WINDOW_NOMAXIMIZE |
	                            AG_WINDOW_NOMINIMIZE)) != NULL) {
		AG_WindowSetCaptionS(win, _("Menu"));
		ShowPanel(com, win);
	}

	AG_ObjectUnlock(com);
}


static void
ExpandButtonPushed(AG_Event *_Nonnull event)
{
	AG_Combo *com = AG_COMBO_PTR(1);
	const int button_state = AG_INT(2);
	AG_Window *win;

	AG_ObjectLock(com);

	if (button_state == 0) {                                /* Collapse */
		if ((win = com->panel) != NULL) {
			ClosePanel(com, win);
			AG_PostEvent(com, "combo-collapsed", NULL);
		}
		goto out;
	}

	if (com->panel != NULL) {                          /* Cached window */
		win = com->panel;
		if (win->flags & AG_WINDOW_NOTITLE) {            /* Refresh */
			AG_TlistClear(com->list);
			AG_PostEvent(com, "combo-expanded", NULL);
			ShowPanel(com, win);
		} else {
			ClosePanel(com, win);
			AG_PostEvent(com, "combo-collapsed", NULL);

			win = CreatePanel(com, AG_WINDOW_MODAL |
			                       AG_WINDOW_KEEPABOVE |
			                       AG_WINDOW_NOTITLE);
			if (win != NULL)
				ShowPanel(com, win);
		}
	} else {                                              /* New window */
		win = CreatePanel(com, AG_WINDOW_MODAL | AG_WINDOW_KEEPABOVE |
		                       AG_WINDOW_NOTITLE);
		if (win != NULL)
			ShowPanel(com, win);
	}
out:
	AG_ObjectUnlock(com);
}

/* Select a combo item based on its pointer. */
AG_TlistItem *
AG_ComboSelectPointer(AG_Combo *com, void *p)
{
	AG_Tlist *tl;
	AG_TlistItem *it;

	AG_OBJECT_ISA(com, "AG_Widget:AG_Combo:*");
	AG_ObjectLock(com);

	if ((tl = com->listExp) != NULL) {
		AG_ObjectLock(tl);
		AG_TlistSelectPtr(tl, p);
		AG_ObjectUnlock(tl);
	}
	if ((it = AG_TlistSelectPtr(com->list, p)) != NULL)
		AG_TextboxSetString(com->tbox, it->text);

	AG_ObjectUnlock(com);

	return (it);
}

/* Select a combo item based on its text. */
AG_TlistItem *
AG_ComboSelectText(AG_Combo *com, const char *text)
{
	AG_Tlist *tl;
	AG_TlistItem *it;

	AG_OBJECT_ISA(com, "AG_Widget:AG_Combo:*");
	AG_ObjectLock(com);

	if ((tl = com->listExp) != NULL) {
		AG_ObjectLock(tl);
		AG_TlistSelectText(tl, text);
		AG_ObjectUnlock(tl);
	}
	if ((it = AG_TlistSelectText(com->list, text)) != NULL)
		AG_TextboxSetString(com->tbox, it->text);

	AG_ObjectUnlock(com);
	return (it);
}

/* Select a combo item by reference. */
void
AG_ComboSelect(AG_Combo *com, AG_TlistItem *it)
{
	AG_OBJECT_ISA(com, "AG_Widget:AG_Combo:*");
	AG_ObjectLock(com);

	AG_TlistSelect(com->list, it);
	AG_TextboxSetString(com->tbox, it->text);

	AG_ObjectUnlock(com);
}

static void
Return(AG_Event *_Nonnull event)
{
	AG_Textbox *tbox = AG_TEXTBOX_SELF();
	AG_Combo *com = AG_COMBO_PTR(1);
	AG_Tlist *tl = com->listExp;
	const char *text = tbox->text;
	
	AG_ObjectLock(tl);

	if ((com->flags & AG_COMBO_ANY_TEXT) == 0) {
		AG_TlistItem *it;
	
		if (text[0] != '\0' &&
		    (it = AG_TlistSelectText(tl, text)) != NULL) {
			AG_TextboxSetString(com->tbox, it->text);
			AG_PostEvent(com, "combo-selected", "%p", it);
		} else {
			AG_TlistDeselectAll(tl);
			AG_TextboxSetString(com->tbox, "");
			AG_PostEvent(com, "combo-text-unknown", "%s", text);
		}
	} else {
		AG_TlistDeselectAll(tl);
		AG_PostEvent(com, "combo-text-entry", "%s", text);
	}

	AG_ObjectUnlock(tl);
}

static void
OnShow(AG_Event *_Nonnull event)
{
	AG_Combo *com = AG_COMBO_SELF();
	AG_TlistItem *it;

	AG_TlistClear(com->list);
	AG_PostEvent(com, "combo-expanded", NULL);

	if ((it = AG_TlistSelectedItem(com->list)) != NULL)
		AG_TextboxSetString(com->tbox, it->text);
}

static void
Init(void *_Nonnull obj)
{
	AG_Combo *com = obj;
	AG_Textbox *tb;
	AG_Button *btn;

	com->flags = 0;
	com->nVisItems = 10;
	com->wSaved = 0;
	com->hSaved = 0;
	com->wPreList = -1;
	com->hPreList = -1;
	
	tb = com->tbox = AG_TextboxNewS(com, AG_TEXTBOX_COMBO | AG_TEXTBOX_EXCL, NULL);
	AG_ObjectSetNameS(tb, "input");
	AG_SetEvent(tb, "textbox-return", Return, "%p", com);
	AG_WidgetForwardFocus(com, tb);

	btn = com->button = AG_ButtonNewS(com, AG_BUTTON_STICKY |
	                                       AG_BUTTON_NO_FOCUS, _(" ... "));
	AG_ObjectSetNameS(btn, "trigger");
	AG_SetEvent(btn, "button-pushed", ExpandButtonPushed, "%p", com);

	com->list = AG_TlistNew(NULL, 0);
	com->listExp = NULL;
	com->panel = NULL;

	AG_AddEvent(com, "widget-shown", OnShow, NULL);
}

void
AG_ComboSizeHint(AG_Combo *com, const char *text, int h)
{
	AG_OBJECT_ISA(com, "AG_Widget:AG_Combo:*");
	AG_TextSize(text, &com->wPreList, NULL);
	com->hPreList = h;
}

void
AG_ComboSizeHintPixels(AG_Combo *com, int w, int h)
{
	AG_OBJECT_ISA(com, "AG_Widget:AG_Combo:*");
	com->wPreList = w;
	com->hPreList = h;
}

void
AG_ComboSetButtonText(AG_Combo *com, const char *text)
{
	AG_OBJECT_ISA(com, "AG_Widget:AG_Combo:*");
	AG_ButtonTextS(com->button, text);
}

void
AG_ComboSetButtonSurface(AG_Combo *com, AG_Surface *su)
{
	AG_OBJECT_ISA(com, "AG_Widget:AG_Combo:*");
	AG_ButtonSurface(com->button, su);
}

void
AG_ComboSetButtonSurfaceNODUP(AG_Combo *com, AG_Surface *su)
{
	AG_OBJECT_ISA(com, "AG_Widget:AG_Combo:*");
	AG_ButtonSurfaceNODUP(com->button, su);
}

static void
Draw(void *_Nonnull obj)
{
	AG_Combo *com = obj;
	
	AG_WidgetDraw(com->tbox);
	AG_WidgetDraw(com->button);
}

static void
SizeRequest(void *_Nonnull obj, AG_SizeReq *_Nonnull r)
{
	AG_Combo *com = obj;
	AG_SizeReq rChld;

	AG_WidgetSizeReq(com->tbox, &rChld);
	r->w = rChld.w;
	r->h = rChld.h;

	AG_WidgetSizeReq(com->button, &rChld);
	r->w += WIDGET(com)->spacingHoriz + rChld.w;
}

static int
SizeAllocate(void *_Nonnull obj, const AG_SizeAlloc *_Nonnull a)
{
	AG_Combo *com = obj;
	AG_SizeReq rBtn;
	AG_SizeAlloc aChld;

	AG_WidgetSizeReq(com->button, &rBtn);
	if (a->w < rBtn.w) {
		return (-1);
	}
	aChld.x = 0;                                       /* Input textbox */
	aChld.y = 0;
	aChld.w = a->w - rBtn.w - WIDGET(com)->spacingHoriz -
	                          WIDGET(com)->paddingRight;
	aChld.h = a->h;
	AG_WidgetSizeAlloc(com->tbox, &aChld);
	
	aChld.x = aChld.w + WIDGET(com)->spacingHoriz;      /* [...] Button */
	aChld.w = rBtn.w;
	AG_WidgetSizeAlloc(com->button, &aChld);
	return (0);
}

AG_WidgetClass agComboClass = {
	{
		"Agar(Widget:Combo)",
		sizeof(AG_Combo),
		{ 1,0, AGC_COMBO, 0xE031 },
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
