/*	$Csoft: combo.c,v 1.29 2005/10/04 17:34:56 vedge Exp $	*/

/*
 * Copyright (c) 2002, 2003, 2004, 2005 CubeSoft Communications, Inc.
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

#include <core/core.h>
#include <core/view.h>

#include "combo.h"

#include <gui/window.h>
#include <gui/primitive.h>
#include <gui/label.h>

#include <stdarg.h>
#include <string.h>
#include <errno.h>

static AG_WidgetOps agComboOps = {
	{
		NULL,			/* init */
		NULL,			/* reinit */
		AG_ComboDestroy,
		NULL,			/* load */
		NULL,			/* save */
		NULL			/* edit */
	},
	NULL,			/* draw */
	AG_ComboScale
};

AG_Combo *
AG_ComboNew(void *parent, Uint flags, const char *label)
{
	AG_Combo *com;

	com = Malloc(sizeof(AG_Combo), M_OBJECT);
	AG_ComboInit(com, flags, label);
	AG_ObjectAttach(parent, com);
	if (flags & AG_COMBO_FOCUS) {
		AG_WidgetFocus(com);
	}
	return (com);
}

static void
combo_collapse(AG_Combo *com)
{
	AG_WidgetBinding *stateb;
	int *state;

	if (com->panel == NULL)
		return;

	com->saved_h = AGWIDGET(com->panel)->h;
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
combo_modal_close(AG_Event *event)
{
	AG_Combo *com = AG_PTR(1);

	if (com->panel != NULL)
		combo_collapse(com);
}

static void
combo_expand(AG_Event *event)
{
	AG_Combo *com = AG_PTR(1);
	int expand = AG_INT(2);

	if (expand) {						/* Expand */
		AG_Widget *panel;

		com->panel = AG_WindowNew(AG_WINDOW_NOTITLE|AG_WINDOW_MODAL);
		panel = AGWIDGET(com->panel);
		AG_ObjectAttach(com->panel, com->list);

		panel->w = AGWIDGET(com)->w - AGWIDGET(com->button)->w;
		panel->h = com->saved_h > 0 ? com->saved_h : AGWIDGET(com)->h*5;
		panel->x = AGWIDGET(com)->cx;
		panel->y = AGWIDGET(com)->cy;
		
		/* XXX redundant? */
		if (panel->x+panel->w > agView->w)
			panel->w = agView->w - panel->x;
		if (panel->y+panel->h > agView->h)
			panel->h = agView->h - panel->y;
		
		AG_SetEvent(panel, "window-modal-close", combo_modal_close,
		    "%p", com);

		AG_WINDOW_UPDATE(panel);
		AG_WindowShow(com->panel);
	} else {
		combo_collapse(com);
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
combo_selected(AG_Event *event)
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
	combo_collapse(com);
}

static void
combo_return(AG_Event *event)
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

void
AG_ComboInit(AG_Combo *com, Uint flags, const char *label)
{
	Uint wflags = AG_WIDGET_FOCUSABLE;

	if (flags & AG_COMBO_HFILL) { wflags |= AG_WIDGET_HFILL; }
	if (flags & AG_COMBO_VFILL) { wflags |= AG_WIDGET_VFILL; }

	AG_WidgetInit(com, "combo", &agComboOps, wflags);
	com->panel = NULL;
	com->flags = flags;
	com->saved_h = 0;

	com->tbox = AG_TextboxNew(com, AG_TEXTBOX_COMBO, label);
	com->button = AG_ButtonNew(com, AG_BUTTON_STICKY, _(" ... "));
	AG_ButtonSetPadding(com->button, 1);

	if ((flags & AG_COMBO_ANY_TEXT) == 0)
		com->tbox->flags &= ~(AG_TEXTBOX_WRITEABLE);

	com->list = Malloc(sizeof(AG_Tlist), M_OBJECT);
	AG_TlistInit(com->list, AG_TLIST_EXPAND);
	
	if (flags & AG_COMBO_TREE) { com->list->flags |= AG_TLIST_TREE; }
	if (flags & AG_COMBO_POLL) { com->list->flags |= AG_TLIST_POLL; }

	AG_SetEvent(com->button, "button-pushed", combo_expand, "%p", com);
	AG_SetEvent(com->list, "tlist-changed", combo_selected, "%p", com);
	AG_SetEvent(com->tbox, "textbox-return", combo_return, "%p", com);
}

void
AG_ComboSetButtonText(AG_Combo *com, const char *text)
{
	AG_ButtonPrintf(com->button, "%s", text);
}

void
AG_ComboSetButtonSurface(AG_Combo *com, SDL_Surface *su)
{
	AG_ButtonSetSurface(com->button, su);
}

void
AG_ComboDestroy(void *p)
{
	AG_Combo *com = p;

	if (com->panel != NULL) {
		AG_WindowHide(com->panel);
		AG_ObjectDetach(com->list);
		AG_ViewDetach(com->panel);
	}
	AG_ObjectDestroy(com->list);
	Free(com->list, M_OBJECT);
	AG_WidgetDestroy(com);
}

void
AG_ComboScale(void *p, int w, int h)
{
	AG_Combo *com = p;

	if (w == -1 && h == -1) {
		AGWIDGET_SCALE(com->tbox, -1, -1);
		AGWIDGET(com)->w = AGWIDGET(com->tbox)->w;
		AGWIDGET(com)->h = AGWIDGET(com->tbox)->h;

		AGWIDGET_SCALE(com->button, -1, -1);
		AGWIDGET(com)->w += AGWIDGET(com->button)->w;
		if (AGWIDGET(com->button)->h > AGWIDGET(com)->h) {
			AGWIDGET(com)->h = AGWIDGET(com->button)->h;
		}
		return;
	}
	
	AG_WidgetScale(com->button, -1, -1);
	AG_WidgetScale(com->tbox, w - AGWIDGET(com->button)->w - 1, h);
	AG_WidgetScale(com->button, AGWIDGET(com->button)->w, h);

	AGWIDGET(com->tbox)->x = 0;
	AGWIDGET(com->tbox)->y = 0;
	AGWIDGET(com->button)->x = w - AGWIDGET(com->button)->w - 1;
	AGWIDGET(com->button)->y = 0;
}

