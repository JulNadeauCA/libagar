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

#include "combo.h"
#include "primitive.h"

AG_Combo *
AG_ComboNew(void *parent, Uint flags, const char *label)
{
	AG_Combo *com;

	com = Malloc(sizeof(AG_Combo));
	AG_ObjectInit(com, &agComboOps);
	com->flags |= flags;

	if (label != NULL) {
		AG_TextboxSetLabel(com->tbox, "%s", label);
	}
	if (flags & AG_COMBO_FOCUS) { AG_WidgetFocus(com); }
	if (flags & AG_COMBO_ANY_TEXT) { AG_WidgetDisable(com->tbox); }
	if (flags & AG_COMBO_TREE) { com->list->flags |= AG_TLIST_TREE; }
	if (flags & AG_COMBO_POLL) { com->list->flags |= AG_TLIST_POLL; }
	if (flags & AG_COMBO_HFILL) { AG_ExpandHoriz(com); }
	if (flags & AG_COMBO_VFILL) { AG_ExpandVert(com); }
	
	AG_ObjectAttach(parent, com);
	return (com);
}

static void
Collapse(AG_Combo *com)
{
	AG_WidgetBinding *stateb;
	int *state;

	if (com->panel == NULL) {
		return;
	}
	com->wSaved = WIDGET(com->panel)->w;
	com->hSaved = WIDGET(com->panel)->h;

	AG_WindowHide(com->panel);
	AG_ObjectDetach(com->list);
	AG_ViewDetach(com->panel);
	com->panel = NULL;
	
	stateb = AG_WidgetGetBinding(com->button, "state", &state);
	*state = 0;
	AG_WidgetBindingChanged(stateb);
	AG_WidgetUnlockBinding(stateb);
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
	int expand = AG_INT(2);
	AG_SizeReq rList;
	int x, y, w, h;

	if (expand) {						/* Expand */
		com->panel = AG_WindowNew(AG_WINDOW_MODAL|AG_WINDOW_NOTITLE);
		AG_WindowSetPadding(com->panel, 0, 0, 0, 0);
		AG_ObjectAttach(com->panel, com->list);
		
		if (com->wSaved > 0) {
			w = com->wSaved;
			h = com->hSaved;
		} else {
			if (com->wPreList != -1 && com->hPreList != -1) {
				AG_TlistSizeHintPixels(com->list,
				    com->wPreList, com->hPreList);
			}
			AG_WidgetSizeReq(com->list, &rList);
			w = rList.w + agColorsBorderSize*2;
			h = rList.h + agColorsBorderSize*2;
 		}
		x = WIDGET(com)->cx + WIDGET(com)->w - w;
		y = WIDGET(com)->cy;

		if (x+w > agView->w) { w = agView->w - x; }
		if (y+h > agView->h) { h = agView->h - y; }
		if (w < 4 || h < 4) {
			Collapse(com);
			return;
		}
		AG_SetEvent(com->panel, "window-modal-close",
		    ModalClose, "%p", com);
		AG_WindowSetGeometry(com->panel, x, y, w, h);
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

	AG_MutexLock(&com->list->lock);
	if ((it = AG_TlistSelectPtr(com->list, p)) != NULL) {
		AG_TextboxPrintf(com->tbox, "%s", it->text);
	}
	AG_MutexUnlock(&com->list->lock);
	return (it);
}

/* Select a combo item based on its text. */
AG_TlistItem *
AG_ComboSelectText(AG_Combo *com, const char *text)
{
	AG_TlistItem *it;

	AG_MutexLock(&com->list->lock);
	if ((it = AG_TlistSelectText(com->list, text)) != NULL) {
		AG_TextboxPrintf(com->tbox, "%s", it->text);
	}
	AG_MutexUnlock(&com->list->lock);
	return (it);
}

void
AG_ComboSelect(AG_Combo *com, AG_TlistItem *it)
{
	AG_MutexLock(&com->list->lock);
	AG_TextboxPrintf(com->tbox, "%s", it->text);
	AG_TlistSelect(com->list, it);
	AG_MutexUnlock(&com->list->lock);
}

static void
SelectedItem(AG_Event *event)
{
	AG_Tlist *tl = AG_SELF();
	AG_Combo *com = AG_PTR(1);
	AG_TlistItem *ti;

	AG_MutexLock(&tl->lock);
	if ((ti = AG_TlistSelectedItem(tl)) != NULL) {
		AG_TextboxPrintf(com->tbox, "%s", ti->text);
		AG_PostEvent(NULL, com, "combo-selected", "%p", ti);
	}
	AG_MutexUnlock(&tl->lock);
	Collapse(com);
}

static void
Return(AG_Event *event)
{
	char text[AG_TEXTBOX_STRING_MAX];
	AG_Textbox *tbox = AG_SELF();
	AG_Combo *com = AG_PTR(1);
	
	AG_MutexLock(&com->list->lock);

	AG_TextboxCopyString(tbox, text, sizeof(text));

	if ((com->flags & AG_COMBO_ANY_TEXT) == 0) {
		AG_TlistItem *it;
	
		if (text[0] != '\0' &&
		    (it = AG_TlistSelectText(com->list, text)) != NULL) {
			AG_TextboxPrintf(com->tbox, "%s", it->text);
			AG_PostEvent(NULL, com, "combo-selected", "%p", it);
		} else {
			AG_TlistDeselectAll(com->list);
			AG_TextboxPrintf(com->tbox, "");
			AG_PostEvent(NULL, com, "combo-text-unknown", "%s",
			    text);
		}
	} else {
		AG_TlistDeselectAll(com->list);
		AG_PostEvent(NULL, com, "combo-text-entry", "%s", text);
	}

	AG_MutexUnlock(&com->list->lock);
}

static void
Init(void *obj)
{
	AG_Combo *com = obj;

	com->flags = 0;
	com->panel = NULL;
	com->wSaved = 0;
	com->hSaved = 0;
	com->wPreList = -1;
	com->hPreList = -1;
	
	com->tbox = AG_TextboxNew(com, AG_TEXTBOX_COMBO, NULL);
	com->button = AG_ButtonNew(com, AG_BUTTON_STICKY, _(" ... "));
	AG_ButtonSetPadding(com->button, 1,1,1,1);
	AG_WidgetSetFocusable(com->button, 0);

	com->list = Malloc(sizeof(AG_Tlist));
	AG_ObjectInit(com->list, &agTlistOps);
	AG_Expand(com->list);
	
	AG_SetEvent(com->button, "button-pushed", Expand, "%p", com);
	AG_SetEvent(com->list, "tlist-changed", SelectedItem, "%p", com);
	AG_SetEvent(com->tbox, "textbox-return", Return, "%p", com);
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
	AG_ButtonText(com->button, "%s", text);
}

void
AG_ComboSetButtonTextNODUP(AG_Combo *com, char *text)
{
	AG_ButtonTextNODUP(com->button, text);
}

void
AG_ComboSetButtonSurface(AG_Combo *com, SDL_Surface *su)
{
	AG_ButtonSurface(com->button, su);
}

void
AG_ComboSetButtonSurfaceNODUP(AG_Combo *com, SDL_Surface *su)
{
	AG_ButtonSurfaceNODUP(com->button, su);
}

static void
Destroy(void *p)
{
	AG_Combo *com = p;

	if (com->panel != NULL) {
		AG_WindowHide(com->panel);
		AG_ObjectDetach(com->list);
		AG_ViewDetach(com->panel);
	}
	AG_ObjectDestroy(com->list);
	Free(com->list);
}

void
AG_ComboSizeRequest(void *p, AG_SizeReq *r)
{
	AG_Combo *com = p;
	AG_SizeReq rChld;

	AG_WidgetSizeReq(com->tbox, &rChld);
	r->w = rChld.w;
	r->h = rChld.h;
	AG_WidgetSizeReq(com->button, &rChld);
	r->w += rChld.w;
	if (r->h < rChld.h) { r->h = rChld.h; }
}

int
AG_ComboSizeAllocate(void *p, const AG_SizeAlloc *a)
{
	AG_Combo *com = p;
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

const AG_WidgetOps agComboOps = {
	{
		"AG_Widget:AG_Combo",
		sizeof(AG_Combo),
		{ 0,0 },
		Init,
		NULL,			/* free */
		Destroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	NULL,				/* draw */
	AG_ComboSizeRequest,
	AG_ComboSizeAllocate
};
