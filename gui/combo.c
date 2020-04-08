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
 * Combo box widget. It embeds an AG_Textbox(3) and a button which activates
 * a drop-down menu (an AG_Tlist(3) displayed in a separate window).
 */

#include <agar/core/core.h>
#ifdef AG_WIDGETS

#include <agar/gui/combo.h>
#include <agar/gui/primitive.h>

static int agComboCounter = 0;

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

static void
PanelWindowClose(AG_Event *_Nonnull event)
{
	AG_Window *win = AG_WINDOW_SELF();
	AG_Combo *com = AG_COMBO_PTR(1);

	com->wSaved = WIDTH(win);
	com->hSaved = HEIGHT(win);
	AG_WindowHide(win);
	AG_SetInt(com->button, "state", 0);
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

	if ((x < 0 || y < 0 || x > WIDTH(win) || y > HEIGHT(win))) {
		com->wSaved = WIDTH(win);
		com->hSaved = HEIGHT(win);
		AG_WindowHide(win);
		AG_SetInt(com->button, "state", 0);
	}
}

/* User pressed or released the expand ("...") button. */
static void
ExpandButtonPushed(AG_Event *_Nonnull event)
{
	AG_Combo *com = AG_COMBO_PTR(1);
	AG_Driver *drv = WIDGET(com)->drv;
	AG_Window *panel, *winParent = WIDGET(com)->window;
	AG_SizeReq rList;
	const int button_state = AG_INT(2);
	int x, y, w, h;
	Uint wView, hView;

	if (button_state) {                                       /* Expand */
		if (com->panel) {
			panel = com->panel;
		} else {
			if ((panel = AG_WindowNew(AG_WINDOW_MODAL |
			                          AG_WINDOW_NOTITLE)) == NULL) {
				return;
			}
			panel->wmType = AG_WINDOW_WM_COMBO;
			AG_SetStyle(panel, "padding", "0");
			AG_ObjectSetName(panel, "_combo%u", agComboCounter++);
			com->panel = panel;
			AG_ObjectAttach(panel, com->list);

			if (winParent) {
				AG_WindowAttach(winParent, panel);
				AG_WindowMakeTransient(winParent, panel);
/*				AG_WindowPin(winParent, panel); */
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
			Debug(com, "Too small size panel; detaching\n");
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
}

/* Select a combo item based on its pointer. */
AG_TlistItem *
AG_ComboSelectPointer(AG_Combo *com, void *p)
{
	AG_Tlist *tl;
	AG_TlistItem *it;

	AG_OBJECT_ISA(com, "AG_Widget:AG_Combo:*");
	tl = com->list;
	AG_ObjectLock(tl);

	if ((it = AG_TlistSelectPtr(tl, p)) != NULL)
		AG_TextboxSetString(com->tbox, it->text);

	AG_ObjectUnlock(tl);
	return (it);
}

/* Select a combo item based on its text. */
AG_TlistItem *
AG_ComboSelectText(AG_Combo *com, const char *text)
{
	AG_Tlist *tl;
	AG_TlistItem *it;

	AG_OBJECT_ISA(com, "AG_Widget:AG_Combo:*");
	tl = com->list;
	AG_ObjectLock(tl);

	if ((it = AG_TlistSelectText(tl, text)) != NULL)
		AG_TextboxSetString(com->tbox, it->text);

	AG_ObjectUnlock(tl);
	return (it);
}

void
AG_ComboSelect(AG_Combo *com, AG_TlistItem *it)
{
	AG_Tlist *tl;

	AG_OBJECT_ISA(com, "AG_Widget:AG_Combo:*");
	tl = com->list;
	AG_ObjectLock(tl);

	AG_TextboxSetString(com->tbox, it->text);
	AG_TlistSelect(tl, it);

	AG_ObjectUnlock(tl);
}

static void
SelectedItem(AG_Event *_Nonnull event)
{
	AG_Tlist *tl = AG_TLIST_SELF();
	AG_Combo *com = AG_COMBO_PTR(1);
	AG_TlistItem *ti;
	AG_Window *panel;

	AG_ObjectLock(com);

	AG_ObjectLock(tl);
	if ((ti = AG_TlistSelectedItem(tl)) != NULL) {
		AG_TextboxSetString(com->tbox, ti->text);
		AG_PostEvent(com, "combo-selected", "%p", ti);
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
Return(AG_Event *_Nonnull event)
{
	AG_Textbox *tbox = AG_TEXTBOX_SELF();
	AG_Combo *com = AG_COMBO_PTR(1);
	AG_Tlist *tl = com->list;
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
Init(void *_Nonnull obj)
{
	AG_Combo *com = obj;
	AG_Textbox *tb;
	AG_Button *btn;
	AG_Tlist *tl;

	com->flags = 0;
	
	tb = com->tbox = AG_TextboxNewS(com, AG_TEXTBOX_COMBO | AG_TEXTBOX_EXCL, NULL);
	AG_SetEvent(tb, "textbox-return", Return, "%p", com);
	AG_SetString(tb, "padding", "inherit");
	AG_WidgetForwardFocus(com, tb);

	btn = com->button = AG_ButtonNewS(com, AG_BUTTON_STICKY |
	                                       AG_BUTTON_NO_FOCUS, _(" ... "));
	AG_SetStyle(btn, "padding", "2");
	AG_SetEvent(btn, "button-pushed", ExpandButtonPushed, "%p", com);

	tl = com->list = Malloc(sizeof(AG_Tlist));
	AG_ObjectInit(tl, &agTlistClass);
	WIDGET(tl)->flags |= AG_WIDGET_EXPAND;
	AG_SetEvent(tl, "tlist-changed", SelectedItem, "%p", com);
	
	com->panel = NULL;
	com->wSaved = 0;
	com->hSaved = 0;
	com->wPreList = -1;
	com->hPreList = -1;
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
