/*
 * Copyright (c) 2002-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

#include "ucombo.h"

#include "primitive.h"

AG_UCombo *
AG_UComboNew(void *parent, Uint flags)
{
	AG_UCombo *com;

	com = Malloc(sizeof(AG_UCombo));
	AG_ObjectInit(com, &agUComboClass);
	com->flags |= flags;

	if (flags & AG_UCOMBO_HFILL) { AG_ExpandHoriz(com); }
	if (flags & AG_UCOMBO_VFILL) { AG_ExpandVert(com); }

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
	AG_EVENT_GET_ARGS(ev, fmt);
	AG_ObjectUnlock(com);
	return (com);
}

/* The UCombo must be locked. */
static void
Collapse(AG_UCombo *com)
{
	if (com->panel == NULL) {
		return;
	}
	com->wSaved = WIDGET(com->panel)->w;
	com->hSaved = WIDGET(com->panel)->h;
	
	AG_WindowHide(com->panel);
	AG_ObjectDetach(com->list);
	AG_ViewDetach(com->panel);
	com->panel = NULL;

	AG_WidgetSetBool(com->button, "state", 0);
}

static void
ModalClose(AG_Event *event)
{
	AG_UCombo *com = AG_PTR(1);

	AG_ObjectLock(com);
	if (com->panel != NULL) {
		Collapse(com);
	}
	AG_ObjectUnlock(com);
}

static void
Expand(AG_Event *event)
{
	AG_UCombo *com = AG_PTR(1);
	int expand = AG_INT(2);
	AG_SizeReq rList;
	int x, y, w, h;

	AG_ObjectLock(com);
	if (expand) {
		com->panel = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_NOTITLE);
		AG_WindowSetPadding(com->panel, 0, 0, 0, 0);
		AG_ObjectAttach(com->panel, com->list);
	
		if (com->wSaved > 0) {
			w = com->wSaved;
			h = com->hSaved;
		} else {
			AG_TlistSizeHintPixels(com->list,
			    com->wPreList, com->hPreList);
			AG_WidgetSizeReq(com->list, &rList);
			w = rList.w + agColorsBorderSize*2;
			h = rList.h + agColorsBorderSize*2;
		}
		x = WIDGET(com)->cx + WIDGET(com)->w - w;
		y = WIDGET(com)->cy;

		AG_SetEvent(com->panel, "window-modal-close",
		    ModalClose, "%p", com);
		AG_WindowSetGeometry(com->panel, x, y, w, h);
		AG_WindowShow(com->panel);
	} else {
		Collapse(com);
	}
	AG_ObjectUnlock(com);
}

static void
SelectedItem(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_UCombo *com = AG_PTR(1);
	AG_TlistItem *it;

	AG_ObjectLock(com);
	AG_ObjectLock(tl);
	if ((it = AG_TlistSelectedItem(tl)) != NULL) {
		it->selected++;
		AG_ButtonText(com->button, "%s", it->text);
		AG_PostEvent(NULL, com, "ucombo-selected", "%p", it);
	}
	Collapse(com);
	AG_ObjectUnlock(tl);
	AG_ObjectUnlock(com);
}

static void
Init(void *obj)
{
	AG_UCombo *com = obj;

	WIDGET(com)->flags |= AG_WIDGET_UNFOCUSED_BUTTONUP;

	com->flags = 0;
	com->panel = NULL;
	com->wSaved = 0;
	com->hSaved = 0;
	com->hPreList = 4;
	AG_TextSize("XXXXXXXX", &com->wPreList, NULL);

	com->button = AG_ButtonNew(com, AG_BUTTON_STICKY, _("..."));
	AG_ButtonSetPadding(com->button, 1,1,1,1);
	AG_WidgetSetFocusable(com->button, 0);
	AG_SetEvent(com->button, "button-pushed", Expand, "%p", com);
	
	com->list = Malloc(sizeof(AG_Tlist));
	AG_ObjectInit(com->list, &agTlistClass);
	WIDGET(com->list)->flags |= AG_WIDGET_EXPAND;

	AG_SetEvent(com->list, "tlist-changed", SelectedItem, "%p", com);
}

void
AG_UComboSizeHint(AG_UCombo *com, const char *text, int h)
{
	AG_ObjectLock(com);
	AG_TextSize(text, &com->wPreList, NULL);
	com->hPreList = h;
	AG_ObjectUnlock(com);
}

void
AG_UComboSizeHintPixels(AG_UCombo *com, int w, int h)
{
	AG_ObjectLock(com);
	com->wPreList = w;
	com->hPreList = h;
	AG_ObjectUnlock(com);
}

static void
Destroy(void *p)
{
	AG_UCombo *com = p;

	if (com->panel != NULL) {
		AG_WindowHide(com->panel);
		AG_ObjectDetach(com->list);
		AG_ViewDetach(com->panel);
	}
	AG_ObjectDestroy(com->list);
}

static void
SizeRequest(void *p, AG_SizeReq *r)
{
	AG_UCombo *com = p;
	AG_SizeReq rButton;

	AG_WidgetSizeReq(com->button, &rButton);
	r->w = rButton.w;
	r->h = rButton.h;
}

static int
SizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_UCombo *com = p;
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
		NULL,			/* free */
		Destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	NULL,				/* draw */
	SizeRequest,
	SizeAllocate
};
