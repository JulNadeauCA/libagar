/*
 * Copyright (c) 2002-2012 Hypertriton, Inc. <http://hypertriton.com/>
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
#include <agar/gui/combo.h>
#include <agar/gui/primitive.h>

AG_Combo *
AG_ComboNew(void *parent, Uint flags, const char *fmt, ...)
{
	char s[AG_LABEL_MAX];
	va_list ap;

	if (fmt != NULL) {
		va_start(ap, fmt);
		Vsnprintf(s, sizeof(s), fmt, ap);
		va_end(ap);
		return AG_ComboNewS(parent, flags, s);
	} else {
		return AG_ComboNewS(parent, flags, NULL);
	}
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
	if (flags & AG_COMBO_ANY_TEXT) { AG_WidgetDisable(com->tbox); }
	if (flags & AG_COMBO_TREE) { com->list->flags |= AG_TLIST_TREE; }
	if (flags & AG_COMBO_POLL) { com->list->flags |= AG_TLIST_POLL; }
	if (flags & AG_COMBO_SCROLLTOSEL) {
		com->list->flags |= AG_TLIST_SCROLLTOSEL;
	}
	if (flags & AG_COMBO_HFILL) { AG_ExpandHoriz(com); }
	if (flags & AG_COMBO_VFILL) { AG_ExpandVert(com); }
	
	AG_ObjectAttach(parent, com);
	return (com);
}

static void
Collapse(AG_Combo *com)
{
	if (com->panel == NULL) {
		return;
	}
	com->wSaved = WIDTH(com->panel);
	com->hSaved = HEIGHT(com->panel);

	AG_ObjectDetach(com->list);
	AG_ObjectDetach(com->panel);
	com->panel = NULL;

	AG_SetInt(com->button, "state", 0);
}

static void
ModalClose(AG_Event *event)
{
	AG_Combo *com = AG_PTR(1);

	if (com->panel != NULL)
		Collapse(com);
}

static void
Expand(AG_Event *event)
{
	AG_Combo *com = AG_PTR(1);
	AG_Driver *drv = WIDGET(com)->drv;
	int expand = AG_INT(2);
	AG_SizeReq rList;
	int x, y, w, h;
	Uint wView, hView;

	if (expand) {
		com->panel = AG_WindowNew(
		    AG_WINDOW_NOTITLE|AG_WINDOW_DENYFOCUS|AG_WINDOW_KEEPABOVE|
		    AG_WINDOW_MODAL);
		com->panel->wmType = AG_WINDOW_WM_COMBO;
		AG_WindowSetPadding(com->panel, 0,0,0,0);
		AG_ObjectSetName(com->panel, "_ComboPopup");
		AG_ObjectAttach(com->panel, com->list);
		if (WIDGET(com)->window != NULL) {
			AG_WindowAttach(WIDGET(com)->window, com->panel);
			AG_WindowMakeTransient(WIDGET(com)->window, com->panel);
			AG_WindowPin(WIDGET(com)->window, com->panel);
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
			w = rList.w + com->panel->wBorderSide*2;
			h = rList.h + com->panel->wBorderBot;
 		}
		x = WIDGET(com)->rView.x2 - w;
		y = WIDGET(com)->rView.y1;

		AG_GetDisplaySize(WIDGET(com)->drv, &wView, &hView);
		if (x+w > wView) { w = wView - x; }
		if (y+h > hView) { h = hView - y; }

		if (AGDRIVER_CLASS(drv)->wm == AG_WM_MULTIPLE &&
		    WIDGET(com)->window != NULL) {
			x += WIDGET(WIDGET(com)->window)->x;
			y += WIDGET(WIDGET(com)->window)->y;
		}
		if (x < 0) { x = 0; }
		if (y < 0) { y = 0; }
		if (w < 4 || h < 4) {
			Collapse(com);
			return;
		}
		AG_SetEvent(com->panel, "window-modal-close",
		    ModalClose, "%p", com);
		AG_WindowSetGeometry(com->panel, x,y, w,h);
		AG_WindowShow(com->panel);
	} else {
		Collapse(com);
	}
}

/* Select a combo item based on its pointer. */
AG_TlistItem *
AG_ComboSelectPointer(AG_Combo *com, void *p)
{
	AG_TlistItem *it;

	AG_ObjectLock(com->list);
	if ((it = AG_TlistSelectPtr(com->list, p)) != NULL) {
		AG_TextboxSetString(com->tbox, it->text);
	}
	AG_ObjectUnlock(com->list);
	return (it);
}

/* Select a combo item based on its text. */
AG_TlistItem *
AG_ComboSelectText(AG_Combo *com, const char *text)
{
	AG_TlistItem *it;

	AG_ObjectLock(com->list);
	if ((it = AG_TlistSelectText(com->list, text)) != NULL) {
		AG_TextboxSetString(com->tbox, it->text);
	}
	AG_ObjectUnlock(com->list);
	return (it);
}

void
AG_ComboSelect(AG_Combo *com, AG_TlistItem *it)
{
	AG_ObjectLock(com->list);
	AG_TextboxSetString(com->tbox, it->text);
	AG_TlistSelect(com->list, it);
	AG_ObjectUnlock(com->list);
}

static void
SelectedItem(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_Combo *com = AG_PTR(1);
	AG_TlistItem *ti;

	AG_ObjectLock(tl);
	if ((ti = AG_TlistSelectedItem(tl)) != NULL) {
		AG_TextboxSetString(com->tbox, ti->text);
		AG_PostEvent(NULL, com, "combo-selected", "%p", ti);
	}
	AG_ObjectUnlock(tl);
	Collapse(com);
}

static void
Return(AG_Event *event)
{
	AG_Textbox *tbox = AG_SELF();
	AG_Combo *com = AG_PTR(1);
	char *text;
	
	AG_ObjectLock(com->list);
	text = tbox->text->ent[0].buf;

	if ((com->flags & AG_COMBO_ANY_TEXT) == 0) {
		AG_TlistItem *it;
	
		if (text[0] != '\0' &&
		    (it = AG_TlistSelectText(com->list, text)) != NULL) {
			AG_TextboxSetString(com->tbox, it->text);
			AG_PostEvent(NULL, com, "combo-selected", "%p", it);
		} else {
			AG_TlistDeselectAll(com->list);
			AG_TextboxSetString(com->tbox, "");
			AG_PostEvent(NULL, com, "combo-text-unknown", "%s",
			    text);
		}
	} else {
		AG_TlistDeselectAll(com->list);
		AG_PostEvent(NULL, com, "combo-text-entry", "%s", text);
	}

	AG_ObjectUnlock(com->list);
}

static void
OnDetach(AG_Event *event)
{
	AG_Combo *com = AG_SELF();

	if (com->panel != NULL) {
		AG_ObjectDetach(com->list);
		AG_ObjectDetach(com->panel);
		com->panel = NULL;
	}
}

static void
Init(void *obj)
{
	AG_Combo *com = obj;

	WIDGET(com)->flags |= AG_WIDGET_TABLE_EMBEDDABLE;

	com->flags = 0;
	com->panel = NULL;
	com->wSaved = 0;
	com->hSaved = 0;
	com->wPreList = -1;
	com->hPreList = -1;
	
	com->tbox = AG_TextboxNewS(com, AG_TEXTBOX_COMBO|AG_TEXTBOX_EXCL, NULL);
	com->button = AG_ButtonNewS(com, AG_BUTTON_STICKY, _(" ... "));
	AG_ButtonSetPadding(com->button, 0,0,0,0);
	AG_LabelSetPadding(com->button->lbl, 0,0,0,0);
	AG_WidgetSetFocusable(com->button, 0);

	com->list = Malloc(sizeof(AG_Tlist));
	AG_ObjectInit(com->list, &agTlistClass);
	AG_Expand(com->list);
	
	AG_SetEvent(com, "detached", OnDetach, NULL);
	AG_SetEvent(com->button, "button-pushed", Expand, "%p", com);
	AG_SetEvent(com->list, "tlist-changed", SelectedItem, "%p", com);
	AG_SetEvent(com->tbox, "textbox-return", Return, "%p", com);
	AG_WidgetForwardFocus(com, com->tbox);
}

void
AG_ComboSizeHint(AG_Combo *com, const char *text, int h)
{
	AG_TextSize(text, &com->wPreList, NULL);
	com->hPreList = h;
}

void
AG_ComboSizeHintPixels(AG_Combo *com, int w, int h)
{
	com->wPreList = w;
	com->hPreList = h;
}

void
AG_ComboSetButtonText(AG_Combo *com, const char *text)
{
	AG_ButtonTextS(com->button, text);
}

void
AG_ComboSetButtonSurface(AG_Combo *com, AG_Surface *su)
{
	AG_ButtonSurface(com->button, su);
}

void
AG_ComboSetButtonSurfaceNODUP(AG_Combo *com, AG_Surface *su)
{
	AG_ButtonSurfaceNODUP(com->button, su);
}

static void
Destroy(void *p)
{
	AG_Combo *com = p;

	AG_ObjectDestroy(com->list);
}

static void
Draw(void *obj)
{
	AG_Combo *com = obj;

	AG_WidgetDraw(com->tbox);
	AG_WidgetDraw(com->button);
}

static void
SizeRequest(void *obj, AG_SizeReq *r)
{
	AG_Combo *com = obj;
	AG_SizeReq rChld;

	AG_WidgetSizeReq(com->tbox, &rChld);
	r->w = rChld.w;
	r->h = rChld.h;
	AG_WidgetSizeReq(com->button, &rChld);
	r->w += rChld.w;
	if (r->h < rChld.h) { r->h = rChld.h; }
}

static int
SizeAllocate(void *obj, const AG_SizeAlloc *a)
{
	AG_Combo *com = obj;
	AG_SizeReq rBtn;
	AG_SizeAlloc aChld;

	AG_WidgetSizeReq(com->button, &rBtn);
	if (a->w < rBtn.w) {
		return (-1);
	}
	aChld.x = 0;
	aChld.y = 0;
	aChld.w = a->w - rBtn.w - 1;
	aChld.h = a->h;
	AG_WidgetSizeAlloc(com->tbox, &aChld);
	aChld.x = aChld.w + 1;
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
		NULL,			/* free */
		Destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	Draw,
	SizeRequest,
	SizeAllocate
};
