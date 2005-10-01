/*	$Csoft: combo.c,v 1.27 2005/09/27 00:25:22 vedge Exp $	*/

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

#include <engine/engine.h>
#include <engine/view.h>

#include "combo.h"

#include <engine/widget/window.h>
#include <engine/widget/primitive.h>
#include <engine/widget/label.h>

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
AG_ComboNew(void *parent, int flags, const char *fmt, ...)
{
	char label[AG_LABEL_MAX];
	AG_Combo *com;
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(label, sizeof(label), fmt, ap);
	va_end(ap);

	com = Malloc(sizeof(AG_Combo), M_OBJECT);
	AG_ComboInit(com, label, flags);
	AG_ObjectAttach(parent, com);
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
combo_expand(int argc, union evarg *argv)
{
	AG_Combo *com = argv[1].p;
	int expand = argv[2].i;

	if (expand) {						/* Expand */
		AG_Widget *panel;

		com->panel = AG_WindowNew(AG_WINDOW_NO_TITLEBAR|
				        AG_WINDOW_NO_DECORATIONS, NULL);
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

	pthread_mutex_lock(&com->list->lock);
	if ((it = AG_TlistSelectPtr(com->list, p)) != NULL) {
		AG_TextboxPrintf(com->tbox, "%s", it->text);
	}
	pthread_mutex_unlock(&com->list->lock);
	return (it);
}

/* Select a combo item based on its text. */
AG_TlistItem *
AG_ComboSelectText(AG_Combo *com, const char *text)
{
	AG_TlistItem *it;

	pthread_mutex_lock(&com->list->lock);
	if ((it = AG_TlistSelectText(com->list, text)) != NULL) {
		AG_TextboxPrintf(com->tbox, "%s", it->text);
	}
	pthread_mutex_unlock(&com->list->lock);
	return (it);
}

void
combo_select(AG_Combo *com, AG_TlistItem *it)
{
	pthread_mutex_lock(&com->list->lock);
	AG_TextboxPrintf(com->tbox, "%s", it->text);
	AG_TlistSelect(com->list, it);
	pthread_mutex_unlock(&com->list->lock);
}

static void
combo_selected(int argc, union evarg *argv)
{
	AG_Tlist *tl = argv[0].p;
	AG_Combo *com = argv[1].p;
	AG_TlistItem *ti;

	pthread_mutex_lock(&tl->lock);
	if ((ti = AG_TlistSelectedItem(tl)) != NULL) {
		AG_TextboxPrintf(com->tbox, "%s", ti->text);
		AG_PostEvent(NULL, com, "combo-selected", "%p", ti);
	}
	pthread_mutex_unlock(&tl->lock);
	combo_collapse(com);
}

static void
combo_return(int argc, union evarg *argv)
{
	char text[AG_TEXTBOX_STRING_MAX];
	AG_Textbox *tbox = argv[0].p;
	AG_Combo *com = argv[1].p;
	
	pthread_mutex_lock(&com->list->lock);

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

	pthread_mutex_unlock(&com->list->lock);
}

#if 0
static void
combo_mousebuttonup(int argc, union evarg *argv)
{
	AG_Combo *com = argv[0].p;
/*	int button = argv[1].i; */
	int x = AGWIDGET(com)->cx + argv[2].i;
	int y = AGWIDGET(com)->cy + argv[3].i;

	if (com->panel != NULL && !AG_WidgetArea(com->panel, x, y))
		combo_collapse(com);
}
#endif

void
AG_ComboInit(AG_Combo *com, const char *label, int flags)
{
	AG_WidgetInit(com, "combo", &agComboOps,
	    AG_WIDGET_FOCUSABLE|AG_WIDGET_WFILL|AG_WIDGET_UNFOCUSED_BUTTONUP);
	com->panel = NULL;
	com->flags = flags;
	com->saved_h = 0;

	com->tbox = AG_TextboxNew(com, label);
	com->button = AG_ButtonNew(com, " ... ");
	AG_ButtonSetSticky(com->button, 1);
	AG_ButtonSetPadding(com->button, 1);

	if ((flags & AG_COMBO_ANY_TEXT) == 0) {
		com->tbox->flags &= ~(AG_TEXTBOX_WRITEABLE);
		AGWIDGET(com->tbox)->flags &= ~(AG_WIDGET_FOCUSABLE);
	}

	com->list = Malloc(sizeof(AG_Tlist), M_OBJECT);
	AG_TlistInit(com->list, 0);
	
	if (flags & AG_COMBO_TREE)	
		com->list->flags |= AG_TLIST_TREE;
	if (flags & AG_COMBO_POLL)
		com->list->flags |= AG_TLIST_POLL;

	AG_SetEvent(com->button, "button-pushed", combo_expand, "%p", com);
	AG_SetEvent(com->list, "tlist-changed", combo_selected, "%p", com);
	AG_SetEvent(com->tbox, "textbox-return", combo_return, "%p", com);
#if 0
	AG_SetEvent(com, "window-mousebuttonup", combo_mousebuttonup, NULL);
#endif
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

